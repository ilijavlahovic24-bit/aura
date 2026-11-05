#include "lexer.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s file.aura\n", argv[0]);
        return 1;
    }
    std::ifstream f(argv[1]);
    if (!f) { std::perror(argv[1]); return 1; }

    aura::lang::Lexer lex(&f);
    for (;;) {
        aura::lang::Token t = lex.next();
        std::printf("%u:%u\t%s", t.loc.line, t.loc.col, aura::lang::tok_name(t.kind));
        if (t.kind == aura::lang::Tok::IDENT)         std::printf("\t'%s'", t.ident.c_str());
        if (t.kind == aura::lang::Tok::INT_LIT)       std::printf("\t%lld", (long long)t.int_val);
        if (t.kind == aura::lang::Tok::FLOAT_LIT)     std::printf("\t%g", t.float_val);
        std::printf("\n");
        if (t.kind == aura::lang::Tok::END) break;
    }
    return 0;
}