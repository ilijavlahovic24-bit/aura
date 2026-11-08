#include "lang/parser.hpp"
#include "lang/sema.hpp"
#include "lang/ir_builder.hpp"
#include "codegen/llvm_codegen.hpp"
#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
    if (argc != 2) return 1;
    std::ifstream f(argv[1]);
    
    aura::lang::Parser parser(f);
    auto strat = parser.parse();
    if (!parser.errors().empty()) return 2;

    aura::lang::SemanticAnalyzer sema;
    if (!sema.analyze(*strat, argv[1])) return 2;

    aura::lang::IRBuilder builder;
    auto ir = builder.build(*strat);

    aura::codegen::LLVMCodegen codegen;
    auto mod = codegen.generate(ir);
    
    
    mod->print(llvm::outs(), nullptr);
    
    return 0;
}