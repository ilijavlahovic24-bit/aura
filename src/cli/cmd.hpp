#ifndef AURA_CLI_CMD_HPP
#define AURA_CLI_CMD_HPP

#include <string>
#include <vector>
#include <map>

namespace aura::cli {

// Exit codes
enum ExitCode : int {
    OK             = 0,
    ARG_ERROR      = 1,
    STRATEGY_ERROR = 2,
    DATA_ERROR     = 3,
    BANKRUPT       = 4,
};

struct Args {
    std::vector<std::string> positional;
    std::multimap<std::string, std::string> flags;

    bool has(const std::string& k) const { return flags.count(k) > 0; }
    std::string get(const std::string& k, const std::string& def = "") const {
        auto it = flags.find(k);
        return it == flags.end() ? def : it->second;
    }
    std::vector<std::string> get_all(const std::string& k) const {
        std::vector<std::string> out;
        auto range = flags.equal_range(k);
        for (auto it = range.first; it != range.second; ++it)
            out.push_back(it->second);
        return out;
    }
    double get_double(const std::string& k, double def) const {
        auto it = flags.find(k);
        if (it == flags.end()) return def;
        try { return std::stod(it->second); } catch (...) { return def; }
    }
    int64_t get_int(const std::string& k, int64_t def) const {
        auto it = flags.find(k);
        if (it == flags.end()) return def;
        try { return std::stoll(it->second); } catch (...) { return def; }
    }
};

Args parse_args(int argc, char** argv, int start);

// JSON string escaping
std::string json_escape(const std::string& s);

// Commands
int cmd_import(const Args&);
int cmd_info  (const Args&);
int cmd_check (const Args&);
int cmd_run   (const Args&);
int cmd_sweep (const Args&);
int cmd_serve (const Args&);

} // namespace aura::cli

#endif