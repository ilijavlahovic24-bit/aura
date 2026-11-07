#include "lang/parser.hpp"
#include "lang/sema.hpp"
#include "ir_builder.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) return 1;
    std::ifstream f(argv[1]);
    
    aura::lang::Parser parser(f);
    auto strat = parser.parse();
    if (!parser.errors().empty()) return 2;

    aura::lang::SemanticAnalyzer sema;
    if (!sema.analyze(*strat, argv[1])) return 2;

    aura::lang::IRBuilder builder;
    auto mod = builder.build(*strat);
    mod.dump(std::cout);
    
    return 0;
}