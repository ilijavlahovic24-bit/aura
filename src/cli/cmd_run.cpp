#include "cli/cmd.hpp"
#include "data/aurd.hpp"
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
#include <iomanip>
#include <sstream>
#include <vector>

namespace aura::cli {

static void emit_json(std::ostream& os, const sim::SimResult& r,
                      const sim::SimConfig& cfg,
                      const std::string& strategy_name,
                      const std::vector<std::pair<std::string, double>>& params)
{
    auto& m = r.metrics;
    os << "{";
    os << "\"strategy\":\"" << json_escape(strategy_name) << "\",";
    os << "\"params\":{";
    for (size_t i = 0; i < params.size(); ++i) {
        if (i) os << ",";
        os << "\"" << json_escape(params[i].first) << "\":" << params[i].second;
    }
    os << "},";
    os << "\"config\":{\"capital\":" << cfg.initial_capital
       << ",\"fee_bps\":" << cfg.fee_bps
       << ",\"slippage_bps\":" << cfg.slippage_bps << "},";
    os << "\"metrics\":{"
       << "\"total_return\":" << m.total_return << ","
       << "\"cagr\":" << m.cagr << ","
       << "\"sharpe\":" << m.sharpe << ","
       << "\"sortino\":" << m.sortino << ","
       << "\"max_drawdown\":" << m.max_drawdown << ","
       << "\"calmar\":" << m.calmar << ","
       << "\"trade_count\":" << m.trade_count << ","
       << "\"win_rate\":" << m.win_rate << ","
       << "\"profit_factor\":" << m.profit_factor << ","
       << "\"exposure\":" << m.exposure
       << "},";
    os << "\"trades\":[";
    for (size_t i = 0; i < r.trades.size(); ++i) {
        const auto& t = r.trades[i];
        if (i) os << ",";
        os << "{\"entry\":" << t.entry_bar << ",\"exit\":" << t.exit_bar
           << ",\"entry_price\":" << t.entry_price
           << ",\"exit_price\":" << t.exit_price
           << ",\"size\":" << t.size
           << ",\"long\":" << (t.is_long ? "true" : "false")
           << ",\"pnl\":" << t.pnl << "}";
    }
    os << "],";
    os << "\"equity\":[";
    for (size_t i = 0; i < r.equity_curve.size(); ++i) {
        if (i) os << ",";
        os << r.equity_curve[i];
    }
    os << "]}\n";
}

static void emit_text(std::ostream& os, const sim::SimResult& r) {
    auto& m = r.metrics;
    os << std::fixed << std::setprecision(4);
    os << "Total Return:  " << (m.total_return * 100) << " %\n";
    os << "CAGR:          " << (m.cagr * 100) << " %\n";
    os << "Sharpe:        " << m.sharpe << "\n";
    os << "Sortino:       " << m.sortino << "\n";
    os << "Max Drawdown:  " << (m.max_drawdown * 100) << " %\n";
    os << "Calmar:        " << m.calmar << "\n";
    os << "Trades:        " << m.trade_count << "\n";
    os << "Win Rate:      " << (m.win_rate * 100) << " %\n";
    os << "Profit Factor: " << m.profit_factor << "\n";
    os << "Exposure:      " << (m.exposure * 100) << " %\n";
}

int cmd_run(const Args& a) {
    if (a.positional.empty() || !a.has("data")) {
        std::cerr << "usage: aura run <strategy.aura> --data <data.aurd> [--param name=value ...] [--format json|text]\n";
        return ARG_ERROR;
    }
    const std::string& strategy_file = a.positional[0];
    const std::string data_file = a.get("data");
    const bool json_out = (a.get("format") == "json");
    const bool use_interp = a.has("interp");

    // 1. Dataset
    data::aurd::Dataset ds;
    try { ds = data::aurd::Dataset::open(data_file); }
    catch (const data::aurd::DatasetError& e) {
        std::cerr << "aura: data error: " << e.what() << "\n";
        return DATA_ERROR;
    }
    const int64_t n = static_cast<int64_t>(ds.size());

    // 2. Frontend
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

    // 3. Param overrides
    std::vector<double> params;
    std::vector<std::pair<std::string, double>> param_pairs;
    for (const auto& p : strat->params) {
        double val = 0.0;
        if (auto fe = dynamic_cast<lang::FloatExpr*>(p.init.get())) val = fe->value;
        else if (auto ie = dynamic_cast<lang::IntExpr*>(p.init.get())) val = static_cast<double>(ie->value);
        // override iz --param name=value
        for (const auto& ov : a.get_all("param")) {
            auto eq = ov.find('=');
            if (eq == std::string::npos) continue;
            if (ov.substr(0, eq) == p.name) {
                try { val = std::stod(ov.substr(eq + 1)); } catch (...) {}
            }
        }
        params.push_back(val);
        param_pairs.emplace_back(p.name, val);
    }

    // 4. IR + LLVM
    lang::IRBuilder irb;
    auto ir = irb.build(*strat);

    codegen::LLVMCodegen cg;
    auto mod = cg.generate(ir);
    const int scratch_slots = cg.used_scratch_slots();

    if (a.has("emit-ir")) {
        mod->print(llvm::outs(), nullptr);
        return OK;
    }

    // 5. Output buffers
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

    // 6. Execution (JIT or interp)
    if (use_interp) {
        codegen::interpret(ir, ctx);
    } else {
        codegen::JITEngine jit;
        KernelFn kernel = jit.compile(std::move(mod));
        kernel(&ctx);
    }

    // 7. Simulator
    sim::SimConfig cfg;
    cfg.initial_capital = a.get_double("capital", 10000.0);
    cfg.fee_bps         = a.get_double("fee-bps", 0.0);
    cfg.slippage_bps    = a.get_double("slippage-bps", 0.0);
    cfg.timeframe_sec   = ds.timeframe();

    auto result = sim::run_simulation(
        ctx.open, ctx.close, n,
        el.data(), xl.data(), es.data(), xs.data(), sz.data(), cfg);

    if (json_out) emit_json(std::cout, result, cfg, strat->name, param_pairs);
    else          emit_text(std::cout, result);

    return OK;
}

} // namespace aura::cli