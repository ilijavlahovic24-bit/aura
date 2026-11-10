#include "sim/simulator.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace aura::sim {

static double apply_cost(double price, bool is_buy, double fee_bps, double slip_bps) {
    double c = (fee_bps + slip_bps) / 10000.0;
    return is_buy ? price * (1.0 + c) : price * (1.0 - c);
}

SimResult run_simulation(
    const double* open, const double* close, int64_t n,
    const uint8_t* enter_long, const uint8_t* exit_long,
    const uint8_t* enter_short, const uint8_t* exit_short,
    const double* size_arr, const SimConfig& cfg)
{
    SimResult res;
    res.equity_curve.resize(n, cfg.initial_capital);

    enum class Pos { Flat, Long, Short };
    Pos pos = Pos::Flat;
    double equity = cfg.initial_capital;
    double entry_price = 0.0;
    double pos_size = 0.0;
    int64_t entry_bar = 0;

    auto close_trade = [&](int64_t bar, double raw_price) {
        bool is_buy = (pos == Pos::Short);
        double exec = apply_cost(raw_price, is_buy, cfg.fee_bps, cfg.slippage_bps);
        double pnl = (pos == Pos::Long)
            ? (exec - entry_price) / entry_price * pos_size * equity
            : (entry_price - exec) / entry_price * pos_size * equity;
        equity += pnl;
        Trade t;
        t.entry_bar = entry_bar; t.exit_bar = bar;
        t.entry_price = entry_price; t.exit_price = exec;
        t.size = pos_size; t.is_long = (pos == Pos::Long);
        t.pnl = pnl;
        res.trades.push_back(t);
        pos = Pos::Flat;
    };

    for (int64_t i = 1; i < n; ++i) {
        if (pos == Pos::Long  && exit_long[i-1])       close_trade(i, open[i]);
        else if (pos == Pos::Short && exit_short[i-1]) close_trade(i, open[i]);

        if (pos == Pos::Flat) {
            bool wl = enter_long[i-1]  != 0;
            bool ws = enter_short[i-1] != 0;
            if (wl && ws) {
                // konflikt → ignoriši
            } else if (wl) {
                pos = Pos::Long;
                pos_size = std::clamp(size_arr[i-1], 0.0, 1.0);
                entry_price = apply_cost(open[i], true, cfg.fee_bps, cfg.slippage_bps);
                entry_bar = i;
            } else if (ws) {
                pos = Pos::Short;
                pos_size = std::clamp(size_arr[i-1], 0.0, 1.0);
                entry_price = apply_cost(open[i], false, cfg.fee_bps, cfg.slippage_bps);
                entry_bar = i;
            }
        }

        double mtm = equity;
        if (pos == Pos::Long)
            mtm = equity * (1.0 + (close[i] - entry_price) / entry_price * pos_size);
        else if (pos == Pos::Short)
            mtm = equity * (1.0 + (entry_price - close[i]) / entry_price * pos_size);
        res.equity_curve[i] = mtm;
    }

    if (pos != Pos::Flat) close_trade(n - 1, close[n - 1]);

    auto& m = res.metrics;
    m.trade_count = (int)res.trades.size();
    if (n > 0) m.total_return = res.equity_curve.back() / cfg.initial_capital - 1.0;

    // Dnevni prinosi (bar po bar)
    std::vector<double> rets;
    rets.reserve(n - 1);
    for (int64_t i = 1; i < n; ++i)
        rets.push_back(res.equity_curve[i] / res.equity_curve[i-1] - 1.0);

    if (!rets.empty()) {
        double mean = std::accumulate(rets.begin(), rets.end(), 0.0) / rets.size();
        double var = 0.0, dvar = 0.0;
        for (double r : rets) {
            var  += (r - mean) * (r - mean);
            if (r < 0) dvar += r * r;
        }
        var /= rets.size();
        double sd = std::sqrt(var), dsd = std::sqrt(dvar / rets.size());

        // Annualizacija
        double ann = 1.0;
        if (cfg.timeframe_sec > 0)
            ann = std::sqrt(365.25 * 86400.0 / cfg.timeframe_sec);

        if (sd  > 1e-12) m.sharpe  = mean / sd  * ann;
        if (dsd > 1e-12) m.sortino = mean / dsd * ann;

        if (cfg.timeframe_sec > 0 && n > 0 && cfg.initial_capital > 0) {
            double years = (double)n * cfg.timeframe_sec / (365.25 * 86400.0);
            if (years > 0 && res.equity_curve.back() > 0)
                m.cagr = std::pow(res.equity_curve.back() / cfg.initial_capital, 1.0 / years) - 1.0;
        }
    }

    double peak = cfg.initial_capital, mdd = 0.0;
    for (double e : res.equity_curve) {
        peak = std::max(peak, e);
        if (peak > 0) mdd = std::max(mdd, (peak - e) / peak);
    }
    m.max_drawdown = mdd;
    if (mdd > 1e-12) m.calmar = m.cagr / mdd;

    int wins = 0;
    double gp = 0.0, gl = 0.0;
    for (const auto& t : res.trades) {
        if (t.pnl > 0) { wins++; gp += t.pnl; }
        else gl += -t.pnl;
    }
    if (!res.trades.empty()) m.win_rate = (double)wins / res.trades.size();
    if (gl > 1e-12) m.profit_factor = gp / gl;

    int open_bars = 0;
    for (const auto& t : res.trades) open_bars += (int)(t.exit_bar - t.entry_bar);
    if (n > 0) m.exposure = (double)open_bars / n;

    return res;
}

} // namespace aura::sim