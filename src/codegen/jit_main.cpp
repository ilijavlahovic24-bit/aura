#include "lang/parser.hpp"
#include "lang/sema.hpp"
#include "lang/ir_builder.hpp"
#include "codegen/llvm_codegen.hpp"
#include "codegen/jit.hpp"
#include "codegen/context.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>

// Runtime implementacije (privremeno, dok ne pređemo na Fazu 3)

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " file.aura\n";
        return 1;
    }
    std::ifstream f(argv[1]);
    if (!f) { std::cerr << "cannot open " << argv[1] << "\n"; return 1; }

    // 1. Frontend
    aura::lang::Parser parser(f);
    auto strat = parser.parse();
    if (!parser.errors().empty()) {
        for (const auto& e : parser.errors()) std::cerr << e << "\n";
        return 2;
    }
    aura::lang::SemanticAnalyzer sema;
    if (!sema.analyze(*strat, argv[1])) {
        for (const auto& e : sema.errors()) std::cerr << e << "\n";
        return 2;
    }

    // 2. IR
    aura::lang::IRBuilder builder;
    auto ir = builder.build(*strat);

    // 3. LLVM IR
    aura::codegen::LLVMCodegen codegen;
    auto mod = codegen.generate(ir);

    // 4. JIT
    aura::codegen::JITEngine jit;
    KernelFn kernel = jit.compile(std::move(mod));

    // 5. Test podaci
    const int64_t n = 1000;
    std::vector<double> open(n, 100.0), high(n, 101.0), low(n, 99.0);
    std::vector<double> close(n), volume(n, 1000.0);
    for (int64_t i = 0; i < n; ++i) {
        close[i] = 100.0 + std::sin(i * 0.1) * 5.0;
    }

    std::vector<uint8_t> enter_long(n, 0), exit_long(n, 0);
    std::vector<uint8_t> enter_short(n, 0), exit_short(n, 0);
    std::vector<double> size_out(n, 0.0);

    // Povećano na 64 da pokrije sve privremene serije (za mean_reversion treba 13)
    const int64_t num_scratch = 64;
    std::vector<double> scratch(num_scratch * n, 0.0);

    std::vector<double> params = {20.0, 2.0};

    // 6. KernelCtx
    KernelCtx ctx;
    ctx.open = open.data();
    ctx.high = high.data();
    ctx.low = low.data();
    ctx.close = close.data();
    ctx.volume = volume.data();
    ctx.params = params.data();
    ctx.enter_long = enter_long.data();
    ctx.exit_long = exit_long.data();
    ctx.enter_short = enter_short.data();
    ctx.exit_short = exit_short.data();
    ctx.size = size_out.data();
    ctx.scratch = scratch.data();
    ctx.n = n;
    ctx.num_scratch = num_scratch;

    // 7. Poziv
    kernel(&ctx);

    // 8. Validacija
    int enters = 0, exits = 0;
    for (int64_t i = 0; i < n; ++i) {
        if (enter_long[i]) ++enters;
        if (exit_long[i]) ++exits;
    }
    std::cout << "Kernel executed. enter_long signals: " << enters
              << ", exit_long signals: " << exits << "\n";

    bool size_set = false;
    for (int64_t i = 0; i < n; ++i) {
        if (size_out[i] != 0.0) { size_set = true; break; }
    }
    std::cout << "Size set: " << (size_set ? "yes" : "no") << "\n";

    return 0;
}