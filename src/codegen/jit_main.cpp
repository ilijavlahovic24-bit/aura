#include "lang/parser.hpp"
#include "lang/sema.hpp"
#include "lang/ir_builder.hpp"
#include "codegen/llvm_codegen.hpp"
#include "codegen/jit.hpp"
#include "codegen/interp.hpp"
#include "codegen/context.hpp"
#include "sim/simulator.hpp"

#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <random>      // <-- DODATO: potrebno za OU proces

int main(int argc, char** argv) {
    std::string strategy_file;
    bool use_interp = false;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--interp") use_interp = true;
        else if (a[0] != '-') strategy_file = a;
    }
    if (strategy_file.empty()) {
        std::cerr << "usage: " << argv[0] << " <file.aura> [--interp]\n";
        return 1;
    }

    std::ifstream f(strategy_file);
    if (!f) { std::cerr << "cannot open " << strategy_file << "\n"; return 1; }

    // --- 1. Frontend ---
    aura::lang::Parser parser(f);
    auto strat = parser.parse();
    if (!parser.errors().empty()) {
        for (const auto& e : parser.errors()) std::cerr << e << "\n";
        return 2;
    }
    aura::lang::SemanticAnalyzer sema;
    if (!sema.analyze(*strat, strategy_file)) {
        for (const auto& e : sema.errors()) std::cerr << e << "\n";
        return 2;
    }

    // --- 2. IR + LLVM ---
    aura::lang::IRBuilder irb;
    auto ir = irb.build(*strat);

    aura::codegen::LLVMCodegen cg;
    auto mod = cg.generate(ir);
    const int scratch_slots = cg.used_scratch_slots();

    // --- 3. Test podaci (Ornstein-Uhlenbeck proces) ---
    //
    // NOTE ON THE PREVIOUS GENERATOR:
    // The generator used a sine wave: p = 100 + 5*sin(i*0.1).
    //   - A sine wave is deterministic and periodic, containing no stochastic noise.
    //   - A 20-bar SMA window covers only ~1/3 of the sine cycle (~62 bars), 
    //     resulting in a very small rolling stdev.
    //   - Consequently, the z-score rarely exceeds ±1.2. The threshold z_entry = 2.0 
    //     is almost never reached at the true trough of the cycle. Instead, it triggers 
    //     randomly in the middle phases whenever the stdev drops unusually low.
    // Result: the strategy enters at the wrong times, yielding a ~0% win rate.
    //
    // OU PROCESS:
    // dx = theta*(mu - x)*dt + sigma*dW is a stochastic mean-reverting model. 
    // The price has a drift towards the mean (mu) and random noise (sigma).
    // The stationary standard deviation is sigma / sqrt(2*theta) ≈ 3.16. 
    // Therefore, the z-score regularly exceeds ±2, meaning the strategy has 
    // a real statistical edge it can exploit.
    //
    // rng(42) ensures determinism (identical input -> bitwise identical output), 
    // which is required by the spec for reproducible JIT vs. interpreter diff testing.

    const int64_t n = 1000;
    std::vector<double> open(n), high(n), low(n), close(n), volume(n);

    std::mt19937_64 rng(42);
    std::normal_distribution<double> noise(0.0, 1.0);
    double x = 100.0;
    const double theta = 0.05, mu = 100.0, sigma = 1.0;
    for (int64_t i = 0; i < n; ++i) {
        x = x + theta * (mu - x) + sigma * noise(rng);
        close[i]  = x;
        open[i]   = x - 0.02;
        high[i]   = x + 0.5;
        low[i]    = x - 0.5;
        volume[i] = 1000.0;
    }

    // Parametri iz AST-a
    std::vector<double> params;
    for (const auto& p : strat->params) {
        if (auto fe = dynamic_cast<aura::lang::FloatExpr*>(p.init.get()))
            params.push_back(fe->value);
        else if (auto ie = dynamic_cast<aura::lang::IntExpr*>(p.init.get()))
            params.push_back((double)ie->value);
        else
            params.push_back(0.0);
    }

    // Lambda za pripremu KernelCtx (koristi se za JIT i interp)
    auto make_ctx = [&](std::vector<uint8_t>& el, std::vector<uint8_t>& xl,
                        std::vector<uint8_t>& es, std::vector<uint8_t>& xs,
                        std::vector<double>& sz, std::vector<double>& scr) -> KernelCtx {
        el.assign(n, 0); xl.assign(n, 0); es.assign(n, 0); xs.assign(n, 0);
        sz.assign(n, 0.0);
        scr.assign((size_t)scratch_slots * n, 0.0);

        KernelCtx ctx{};
        ctx.open = open.data();   ctx.high = high.data();
        ctx.low = low.data();     ctx.close = close.data();
        ctx.volume = volume.data();
        ctx.params = params.data();
        ctx.enter_long = el.data();  ctx.exit_long = xl.data();
        ctx.enter_short = es.data(); ctx.exit_short = xs.data();
        ctx.size = sz.data();
        ctx.scratch = scr.data();
        ctx.n = n;
        ctx.num_scratch = scratch_slots;
        return ctx;
    };

    // --- 4. Priprema konteksta ---
    std::vector<uint8_t> el_j, xl_j, es_j, xs_j;
    std::vector<double> sz_j, scr_j;
    auto ctx_j = make_ctx(el_j, xl_j, es_j, xs_j, sz_j, scr_j);

    std::vector<uint8_t> el_i, xl_i, es_i, xs_i;
    std::vector<double> sz_i, scr_i;
    auto ctx_i = make_ctx(el_i, xl_i, es_i, xs_i, sz_i, scr_i);

    aura::sim::SimConfig cfg;
    cfg.initial_capital = 10000.0;
    cfg.fee_bps         = 1.0;
    cfg.slippage_bps    = 2.0;
    cfg.timeframe_sec   = 60;

    // --- 5. Izvršavanje ---
    if (use_interp) {
        // Samo interpreter
        aura::codegen::interpret(ir, ctx_i);
        std::cout << "=== INTERP (--interp mode) ===\n";
        auto bt = aura::sim::run_simulation(
            ctx_i.open, ctx_i.close, n,
            el_i.data(), xl_i.data(), es_i.data(), xs_i.data(),
            sz_i.data(), cfg);
        std::cout << "Total Return:  " << (bt.metrics.total_return * 100) << " %\n";
        std::cout << "Sharpe:        " << bt.metrics.sharpe << "\n";
        std::cout << "Max Drawdown:  " << (bt.metrics.max_drawdown * 100) << " %\n";
        std::cout << "Trades:        " << bt.metrics.trade_count << "\n";
        return 0;
    }

    // JIT + interpreter (diferencijalno testiranje)
    aura::codegen::JITEngine jit;
    KernelFn kernel = jit.compile(std::move(mod));
    kernel(&ctx_j);
    aura::codegen::interpret(ir, ctx_i);

    auto count = [](const std::vector<uint8_t>& v) {
        int c = 0; for (auto b : v) if (b) ++c; return c;
    };
    std::cout << "=== JIT ===\n";
    std::cout << "enter_long: " << count(el_j) << ", exit_long: " << count(xl_j) << "\n";
    std::cout << "enter_short: " << count(es_j) << ", exit_short: " << count(xs_j) << "\n";
    std::cout << "=== INTERP ===\n";
    std::cout << "enter_long: " << count(el_i) << ", exit_long: " << count(xl_i) << "\n";
    std::cout << "enter_short: " << count(es_i) << ", exit_short: " << count(xs_i) << "\n";

    int diffs = 0;
    for (int64_t i = 0; i < n; ++i) {
        diffs += (el_j[i] != el_i[i]);
        diffs += (xl_j[i] != xl_i[i]);
        diffs += (es_j[i] != es_i[i]);
        diffs += (xs_j[i] != xs_i[i]);
    }
    std::cout << "=== DIFF ===\n";
    std::cout << "signal mismatches: " << diffs << "\n";

    if (diffs != 0) {
        std::cerr << "ERROR: JIT and interpreter produce different signals!\n";
        return 1;
    }

    // --- 6. Backtest (JIT signali) ---
    auto bt = aura::sim::run_simulation(
        ctx_j.open, ctx_j.close, n,
        el_j.data(), xl_j.data(), es_j.data(), xs_j.data(),
        sz_j.data(), cfg);

    std::cout << "=== BACKTEST ===\n";
    std::cout << "Total Return:  " << (bt.metrics.total_return * 100) << " %\n";
    std::cout << "Sharpe:        " << bt.metrics.sharpe << "\n";
    std::cout << "Max Drawdown:  " << (bt.metrics.max_drawdown * 100) << " %\n";
    std::cout << "Trades:        " << bt.metrics.trade_count << "\n";
    std::cout << "Win Rate:      " << (bt.metrics.win_rate * 100) << " %\n";
    std::cout << "Profit Factor: " << bt.metrics.profit_factor << "\n";
    std::cout << "Exposure:      " << (bt.metrics.exposure * 100) << " %\n";

    return 0;
}