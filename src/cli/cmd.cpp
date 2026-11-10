#include "cli/cmd.hpp"
#include <iostream>
#include <sstream>

namespace aura::cli {

Args parse_args(int argc, char** argv, int start) {
    Args a;
    for (int i = start; i < argc; ++i) {
        std::string s = argv[i];

        // Long flag: --name or --name=value
        if (s.size() >= 2 && s[0] == '-' && s[1] == '-') {
            std::string key = s.substr(2);
            auto eq = key.find('=');
            if (eq != std::string::npos) {
                a.flags.emplace(key.substr(0, eq), key.substr(eq + 1));
            } else {
                bool has_value = false;
                if (i + 1 < argc) {
                    std::string next = argv[i + 1];
                    bool next_is_flag = (next.size() >= 2 && next[0] == '-');
                    if (!next_is_flag) {
                        a.flags.emplace(key, next);
                        ++i;
                        has_value = true;
                    }
                }
                if (!has_value) a.flags.emplace(key, "1");
            }
        }
        // Short flag: -X or -Xvalue
        else if (s.size() >= 2 && s[0] == '-' && s[1] != '-') {
            if (s.size() == 2) {
                std::string key = s.substr(1);
                bool has_value = false;
                if (i + 1 < argc) {
                    std::string next = argv[i + 1];
                    bool next_is_flag = (next.size() >= 2 && next[0] == '-');
                    if (!next_is_flag) {
                        a.flags.emplace(key, next);
                        ++i;
                        has_value = true;
                    }
                }
                if (!has_value) a.flags.emplace(key, "1");
            } else {
                // -Xvalue (npr. -o/tmp/x.aurd)
                a.flags.emplace(s.substr(1, 1), s.substr(2));
            }
        }
        else {
            a.positional.push_back(s);
        }
    }
    return a;
}
std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

} // namespace aura::cli