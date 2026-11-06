#include "parser.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " file.aura\n";
        return 1;
    }
    std::ifstream f(argv[1]);
    if (!f) return 1;

    aura::lang::Parser parser(f);
    auto strat = parser.parse();

    if (!parser.errors().empty()) {
        for (const auto& e : parser.errors()) {
            std::cerr << argv[1] << ":" << e << "\n";
        }
        return 2;
    }

    if (strat) {
        strat->dump(std::cout);
    }
    return 0;
}