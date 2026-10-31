#include "data/aurd.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <fstream>
#include <string>
#include <string_view>

namespace aura::data::aurd {

namespace {

// Howard Hinnant: days since 1970-01-01 for a civil date.
int64_t days_from_civil(int y, unsigned m, unsigned d) noexcept {
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return static_cast<int64_t>(era) * 146097 + static_cast<int64_t>(doe) - 719468;
}

std::string_view trim(std::string_view s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))  s.remove_suffix(1);
    return s;
}

bool parse_int(std::string_view s, int64_t& out) {
    if (s.empty()) return false;
    auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), out);
    return ec == std::errc{} && p == s.data() + s.size();
}

bool parse_double(std::string_view s, double& out) {
    if (s.empty()) return false;
    auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), out);
    return ec == std::errc{} && p == s.data() + s.size();
}

bool parse_iso8601(std::string_view s, int64_t& out) {
    if (!s.empty() && (s.back() == 'Z' || s.back() == 'z')) s.remove_suffix(1);

    auto read_n = [&](size_t pos, int n, int& dst) {
        int v = 0;
        for (int i = 0; i < n; ++i) {
            char c = s[pos + i];
            if (c < '0' || c > '9') return false;
            v = v * 10 + (c - '0');
        }
        dst = v;
        return true;
    };

    // Date-only: YYYY-MM-DD
    if (s.size() == 10) {
        if (s[4] != '-' || s[7] != '-') return false;
        int y, M, d;
        if (!read_n(0, 4, y) || !read_n(5, 2, M) || !read_n(8, 2, d)) return false;
        if (M < 1 || M > 12 || d < 1 || d > 31) return false;
        out = days_from_civil(y, static_cast<unsigned>(M), static_cast<unsigned>(d)) * 86400;
        return true;
    }

    // Full: YYYY-MM-DD[T ]HH:MM:SS
    if (s.size() < 19) return false;
    if (s[4] != '-' || s[7] != '-' || s[13] != ':' || s[16] != ':') return false;
    if (s[10] != 'T' && s[10] != 't' && s[10] != ' ') return false;

    int y, M, d, h, m, sec;
    if (!read_n(0, 4, y) || !read_n(5, 2, M) || !read_n(8, 2, d) ||
        !read_n(11, 2, h) || !read_n(14, 2, m) || !read_n(17, 2, sec)) return false;
    if (M < 1 || M > 12 || d < 1 || d > 31 || h > 23 || m > 59 || sec > 60) return false;

    out = days_from_civil(y, static_cast<unsigned>(M), static_cast<unsigned>(d)) * 86400
        + h * 3600 + m * 60 + sec;
    return true;
}

bool parse_timestamp(std::string_view s, int64_t& out) {
    // Heuristics: if it contains ':' or the 5th character is '-' → ISO-8601. 
// Else → Unix integer.
    if (s.find(':') != std::string_view::npos) return parse_iso8601(s, out);
    if (s.size() >= 10 && s[4] == '-')         return parse_iso8601(s, out);
    return parse_int(s, out);
}

bool split_csv(std::string_view line, std::string_view* fields, size_t n) {
    size_t start = 0;
    size_t idx = 0;
    for (size_t i = 0; i <= line.size(); ++i) {
        if (i == line.size() || line[i] == ',') {
            if (idx >= n) return false;
            fields[idx++] = line.substr(start, i - start);
            start = i + 1;
        }
    }
    return idx == n;
}

std::string derive_symbol(const std::filesystem::path& csv) {
    std::string s = csv.stem().string();
    if (s.empty()) s = "UNKNOWN";
    if (s.size() > kSymbolLen - 1) s.resize(kSymbolLen - 1);
    return s;
}

} // namespace

void import_csv(const std::filesystem::path& csv,
                const std::filesystem::path& out,
                bool compress)
{
    std::ifstream f(csv);
    if (!f) throw DatasetError("cannot open csv: " + csv.string());

    RawBars bars;
    bars.symbol = derive_symbol(csv);

    std::string line;
    size_t line_no = 0;
    bool first_data_line = true;

    auto fail = [&](const std::string& msg) {
        throw DatasetError(csv.string() + ":" + std::to_string(line_no) + ": error: " + msg);
    };

    while (std::getline(f, line)) {
        ++line_no;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        if (first_data_line) {
            first_data_line = false;
            // Header heuristic: first character is not a digit or '-'
            if (!std::isdigit(static_cast<unsigned char>(line[0])) && line[0] != '-') {
                continue;
            }
        }

        std::string_view raw[6];
        if (!split_csv(line, raw, 6)) fail("expected 6 comma-separated fields");

        std::string_view fields[6];
        for (int i = 0; i < 6; ++i) fields[i] = trim(raw[i]);

        int64_t ts;
        if (!parse_timestamp(fields[0], ts))
            fail("invalid timestamp: '" + std::string(fields[0]) + "'");

        double v[5];
        static const char* names[5] = {"open", "high", "low", "close", "volume"};
        for (int i = 0; i < 5; ++i) {
            if (!parse_double(fields[1 + i], v[i]))
                fail(std::string("invalid ") + names[i] + ": '" + std::string(fields[1 + i]) + "'");
            if (std::isnan(v[i]))
                fail(std::string("NaN value not allowed in ") + names[i]);
        }
        const double o = v[0], h = v[1], l = v[2], c = v[3], vol = v[4];

        if (!bars.ts.empty() && ts <= bars.ts.back())
            fail("timestamp not strictly increasing (prev=" + std::to_string(bars.ts.back()) +
                 ", current=" + std::to_string(ts) + ")");
        if (l > std::min(o, c)) fail("low > min(open, close)");
        if (h < std::max(o, c)) fail("high < max(open, close)");

        bars.ts.push_back(ts);
        bars.open.push_back(o);
        bars.high.push_back(h);
        bars.low.push_back(l);
        bars.close.push_back(c);
        bars.volume.push_back(vol);
    }

    if (bars.ts.empty()) throw DatasetError(csv.string() + ": no bars found");
    if (bars.ts.size() >= 2)
        bars.timeframe_sec = static_cast<uint32_t>(bars.ts[1] - bars.ts[0]);

    write_dataset(out, bars, compress);
}

} // namespace aura::data::aurd