#include "cli/cmd.hpp"
#include "data/aurd.hpp"
#include "lang/parser.hpp"
#include "lang/sema.hpp"
#include "lang/ir_builder.hpp"
#include "codegen/llvm_codegen.hpp"
#include "codegen/jit.hpp"
#include "codegen/context.hpp"
#include "sim/simulator.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <thread>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <sstream>

namespace aura::cli {

struct Grid {
    std::string name;
    std::vector<double> values;
};

struct Result {
    std::vector<double> params;   // u redosledu strat->params
    double sharpe = 0.0;
    double total_return = 0.0;
    int    trade_count = 0;
};

int cmd_sweep(const Args& a) {
    if (a.positional.empty() || !a.has("data") || !a.has("grid")) {
        std::cerr << "usage: aura sweep <strategy.aura> --data <data.aurd> --grid name=start:stop:step ...\n";
        return ARG_ERROR;
    }
    const std::string& strategy_file = a.positional[0];
    const std::string data_file = a.get("data");
    const int threads = static_cast<int>(a.get_int("threads", 4));

    // 1. Parsiraj gridove
    std::vector<Grid> grids;
    for (const auto& g : a.get_all("grid")) {
        auto eq = g.find('=');
        auto c1 = g.find(':', eq + 1);
        auto c2 = g.find(':', c1 + 1);
        if (eq == std::string::npos || c1 == std::string::npos || c2 == std::string::npos) {
            std::cerr << "aura: invalid grid syntax: '" << g << "', expected name=start:stop:step\n";
            return ARG_ERROR;
        }
        Grid grid;
        grid.name = g.substr(0, eq);
        double start = std::stod(g.substr(eq + 1, c1 - eq - 1));
        double stop  = std::stod(g.substr(c1 + 1, c2 - c1 - 1));
        double step  = std::stod(g.substr(c2 + 1));
        for (double v = start; v <= stop + 1e-9; v += step) grid.values.push_back(v);
        grids.push_back(std::move(grid));
    }

    // 2. Dataset
    data::aurd::Dataset ds;
    try { ds = data::aurd::Dataset::open(data_file); }
    catch (const data::aurd::DatasetError& e) {
        std::cerr << "aura: data error: " << e.what() << "\n";
        return DATA_ERROR;
    }
    const int64_t n = static_cast<int64_t>(ds.size());

    // 3. Frontend
    std::ifstream f(strategy_file);
    if (!f) { std::cerr << "aura: cannot open " << strategy_file << "\n"; return ARG_ERROR; }
    lang::Parser parser(f);
    auto strat = parser.parse();
    if (!parser.errors().empty()) {
        for (const auto& e : parser.errors()) std::cerr << strategy_file << ":" << e << "\n";
        return STRATEGY_ERROR;
    }
    lang::SemanticAnalyzer sema;
    if (!sema.analyze(*strat, strategy_file)) {
        for (const auto& e : sema.errors()) std::cerr << e << "\n";
        return STRATEGY_ERROR;
    }

    // 4. IR + LLVM + JIT (once; parameters are not embedded in the code)
    lang::IRBuilder irb;
    auto ir = irb.build(*strat);
    codegen::LLVMCodegen cg;
    auto mod = cg.generate(ir);
    const int scratch_slots = cg.used_scratch_slots();
    codegen::JITEngine jit;
    KernelFn kernel = jit.compile(std::move(mod));

    // 5. Cartesian product of grids
    std::vector<std::vector<double>> combos;
    combos.emplace_back();  // one empty
    for (const auto& g : grids) {
        std::vector<std::vector<double>> next;
        for (const auto& c : combos)
            for (double v : g.values) {
                auto nc = c;
                nc.push_back(v);
                next.push_back(std::move(nc));
            }
        combos = std::move(next);
    }

    // 6. Map combos to parameter vectors (in the order of strat->params)
    std::vector<std::vector<double>> param_combos;
    for (const auto& combo : combos) {
        std::vector<double> pv;
        for (const auto& p : strat->params) {
            double v = 0.0;
            if (auto fe = dynamic_cast<lang::FloatExpr*>(p.init.get())) v = fe->value;
            else if (auto ie = dynamic_cast<lang::IntExpr*>(p.init.get())) v = (double)ie->value;
            for (size_t gi = 0; gi < grids.size(); ++gi)
                if (grids[gi].name == p.name) v = combo[gi];
            pv.push_back(v);
        }
        param_combos.push_back(std::move(pv));
    }
    // 7. parallel execution
    std::vector<Result> results(param_combos.size());
    std::atomic<size_t> next_idx{0};
    std::mutex print_mu;

    sim::SimConfig cfg;
    cfg.initial_capital = a.get_double("capital", 10000.0);
    cfg.fee_bps         = a.get_double("fee-bps", 0.0);
    cfg.slippage_bps    = a.get_double("slippage-bps", 0.0);
    cfg.timeframe_sec   = ds.timeframe();

    auto worker = [&]() {
        while (true) {
            size_t idx = next_idx.fetch_add(1);
            if (idx >= param_combos.size()) break;
            auto params = param_combos[idx];  // copy(small vector)
            std::vector<uint8_t> el(n, 0), xl(n, 0), es(n, 0), xs(n, 0);
            std::vector<double> sz(n, 1.0), scratch((size_t)scratch_slots * n, 0.0);

            KernelCtx ctx{};
            ctx.open   = ds.open(0, n);
            ctx.high   = ds.high(0, n);
            ctx.low    = ds.low(0, n);
            ctx.close  = ds.close(0, n);
            ctx.volume = ds.volume(0, n);
            ctx.params = params.data();
            ctx.enter_long = el.data(); ctx.exit_long  = xl.data();
            ctx.enter_short = es.data(); ctx.exit_short = xs.data();
            ctx.size = sz.data();
            ctx.scratch = scratch.data();
            ctx.n = n;
            ctx.num_scratch = scratch_slots;

            kernel(&ctx);

            auto r = sim::run_simulation(
                ctx.open, ctx.close, n,
                el.data(), xl.data(), es.data(), xs.data(), sz.data(), cfg);

            results[idx].params = std::move(params);
            results[idx].sharpe = r.metrics.sharpe;
            results[idx].total_return = r.metrics.total_return;
            results[idx].trade_count = r.metrics.trade_count;
        }
    };

    std::vector<std::thread> pool;
    for (int i = 0; i < threads; ++i) pool.emplace_back(worker);
    for (auto& t : pool) t.join();
    // 9. Sort results by sharpe (descending), tie-break by index, deterministic
    std::vector<size_t> order(results.size());
    for (size_t i = 0; i < order.size(); ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](size_t x, size_t y) {
        if (results[x].sharpe != results[y].sharpe)
            return results[x].sharpe > results[y].sharpe;
        return x < y;
    });

    // 9. Output
    std::cout << std::fixed << std::setprecision(4);
    std::cout << std::left;
    std::cout << std::setw(10) << "sharpe"
              << std::setw(10) << "return"
              << std::setw(8)  << "trades";
    for (const auto& p : strat->params) std::cout << std::setw(12) << p.name;
    std::cout << "\n";

    for (size_t i : order) {
        const auto& r = results[i];
        std::cout << std::setw(10) << r.sharpe
                  << std::setw(10) << (r.total_return * 100)
                  << std::setw(8)  << r.trade_count;
        for (double v : r.params) std::cout << std::setw(12) << v;
        std::cout << "\n";
    }
    return OK;
}

} // namespace aura::cli