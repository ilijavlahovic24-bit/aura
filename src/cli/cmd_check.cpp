#include "cli/cmd.hpp"
#include "lang/parser.hpp"
#include "lang/sema.hpp"
#include <fstream>
#include <iostream>

namespace aura::cli {

int cmd_check(const Args& a) {
    if (a.positional.empty()) {
        std::cerr << "usage: aura check <strategy.aura>\n";
        return ARG_ERROR;
    }
    std::ifstream f(a.positional[0]);
    if (!f) { std::cerr << "aura: cannot open " << a.positional[0] << "\n"; return ARG_ERROR; }

    lang::Parser parser(f);
    auto strat = parser.parse();
    if (!parser.errors().empty()) {
        for (const auto& e : parser.errors()) std::cerr << a.positional[0] << ":" << e << "\n";
        return STRATEGY_ERROR;
    }
    lang::SemanticAnalyzer sema;
    if (!sema.analyze(*strat, a.positional[0])) {
        for (const auto& e : sema.errors()) std::cerr << e << "\n";
        return STRATEGY_ERROR;
    }
    std::cout << "OK\n";
    return OK;
}

} // namespace aura::cli