#include "lang/parser.hpp"
#include "lang/sema.hpp"
#include "lang/ir_builder.hpp"
#include "codegen/llvm_codegen.hpp"
#include "codegen/jit.hpp"
#include "codegen/interp.hpp"
#include "codegen/context.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>

int main(int argc, char** argv) {
    std::string strategy_file;
    bool use_interp = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--interp") use_interp = true;
        else if (argv[i][0] != '-') strategy_file = argv[i];
    }
    if (strategy_file.empty()) {
        std::cerr << "usage: " << argv[0] << " file.aura [--interp]\n";
        return 1;
    }

    std::ifstream f(strategy_file);
    if (!f) { std::cerr << "cannot open " << strategy_file << "\n"; return 1; }

    aura::lang::Parser parser(f);
    auto strat = parser.parse();
    if (!parser.errors().empty()) {
        for (auto& e : parser.errors()) std::cerr << e << "\n";
        return 2;
    }
    aura::lang::SemanticAnalyzer sema;
    if (!sema.analyze(*strat, strategy_file)) {
        for (auto& e : sema.errors()) std::cerr << e << "\n";
        return 2;
    }

    aura::lang::IRBuilder irb;
    auto ir = irb.build(*strat);

    aura::codegen::LLVMCodegen cg;
    auto mod = cg.generate(ir);
    const int scratch_slots = cg.used_scratch_slots();

    // Priprema podataka
    const int64_t n = 1000;
    std::vector<double> open(n), high(n), low(n), close(n), volume(n);
    for (int64_t i = 0; i < n; ++i) {
        double p = 100.0 + 5.0 * std::sin(i * 0.1);
        open[i] = p - 0.1; close[i] = p;
        high[i] = p + 0.5; low[i] = p - 0.5;
        volume[i] = 1000.0;
    }
    std::vector<double> params;
    for (const auto& p : strat->params) {
        if (auto fe = dynamic_cast<aura::lang::FloatExpr*>(p.init.get()))
            params.push_back(fe->value);
        else if (auto ie = dynamic_cast<aura::lang::IntExpr*>(p.init.get()))
            params.push_back((double)ie->value);
        else params.push_back(0.0);
    }

    auto make_ctx = [&](std::vector<uint8_t>& el, std::vector<uint8_t>& xl,
                        std::vector<uint8_t>& es, std::vector<uint8_t>& xs,
                        std::vector<double>& sz, std::vector<double>& scratch) -> KernelCtx {
        el.assign(n, 0); xl.assign(n, 0); es.assign(n, 0); xs.assign(n, 0);
        sz.assign(n, 0.0);
        scratch.assign((size_t)scratch_slots * n, 0.0);
        KernelCtx ctx{};
        ctx.open = open.data(); ctx.high = high.data();
        ctx.low = low.data();   ctx.close = close.data();
        ctx.volume = volume.data();
        ctx.params = params.data();
        ctx.enter_long = el.data();  ctx.exit_long = xl.data();
        ctx.enter_short = es.data(); ctx.exit_short = xs.data();
        ctx.size = sz.data();
        ctx.scratch = scratch.data();
        ctx.n = n; ctx.num_scratch = scratch_slots;
        return ctx;
    };

    // --- JIT put ---
    aura::codegen::JITEngine jit;
    KernelFn kernel = jit.compile(std::move(mod));

    std::vector<uint8_t> el_j, xl_j, es_j, xs_j;
    std::vector<double> sz_j, scr_j;
    auto ctx_j = make_ctx(el_j, xl_j, es_j, xs_j, sz_j, scr_j);
    kernel(&ctx_j);

    // --- Interp put ---
    std::vector<uint8_t> el_i, xl_i, es_i, xs_i;
    std::vector<double> sz_i, scr_i;
    auto ctx_i = make_ctx(el_i, xl_i, es_i, xs_i, sz_i, scr_i);
    aura::codegen::interpret(ir, ctx_i);

    // --- Poređenje ---
    auto count = [&](const std::vector<uint8_t>& v) {
        int c = 0; for (auto b : v) if (b) ++c; return c;
    };
    std::cout << "=== JIT ===\n";
    std::cout << "enter_long: " << count(el_j) << ", exit_long: " << count(xl_j) << "\n";
    std::cout << "enter_short: " << count(es_j) << ", exit_short: " << count(xs_j) << "\n";
    std::cout << "=== INTERP ===\n";
    std::cout << "enter_long: " << count(el_i) << ", exit_long: " << count(xl_i) << "\n";
    std::cout << "enter_short: " << count(es_i) << ", exit_short: " << count(xs_i) << "\n";

    // Diff provera
    int diffs = 0;
    for (int64_t i = 0; i < n; ++i) {
        if (el_j[i] != el_i[i]) ++diffs;
        if (xl_j[i] != xl_i[i]) ++diffs;
        if (es_j[i] != es_i[i]) ++diffs;
        if (xs_j[i] != xs_i[i]) ++diffs;
    }
    std::cout << "=== DIFF ===\n";
    std::cout << "signal mismatches: " << diffs << "\n";

    if (use_interp) {
        std::cout << "(Ran interpreter only via --interp; comparison shown above)\n";
    }
    return diffs == 0 ? 0 : 1;
}