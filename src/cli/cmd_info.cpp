#include "cli/cmd.hpp"
#include "data/aurd.hpp"
#include <iostream>
#include <cstdio>

namespace aura::cli {

int cmd_info(const Args& a) {
    if (a.positional.empty()) {
        std::cerr << "usage: aura info <data.aurd>\n";
        return ARG_ERROR;
    }
    try {
        auto ds = data::aurd::Dataset::open(a.positional[0]);
        std::cout << "symbol:    " << ds.symbol() << "\n";
        std::cout << "bars:      " << ds.size() << "\n";
        std::cout << "timeframe: " << ds.timeframe() << "s\n";
        if (ds.size() >= 2) {
            const auto* ts = ds.ts(0, 2);
            if (ts) {
                const auto span = ds.size();
                const auto* t0 = ds.ts(0, 1);
                const auto* tN = ds.ts(span - 1, span);
                std::cout << "first:     " << t0[0] << "\n";
                std::cout << "last:      " << tN[0] << "\n";
            }
        }
        return OK;
    } catch (const data::aurd::DatasetError& e) {
        std::cerr << "aura: data error: " << e.what() << "\n";
        return DATA_ERROR;
    }
}

} // namespace aura::cli