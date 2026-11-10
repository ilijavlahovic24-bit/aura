#ifndef AURA_SIM_SIMULATOR_HPP
#define AURA_SIM_SIMULATOR_HPP

#include <cstdint>
#include <vector>

namespace aura::sim {

struct SimConfig {
    double   initial_capital = 10000.0;
    double   fee_bps         = 0.0;    
    double   slippage_bps    = 0.0;    
    uint32_t timeframe_sec   = 0;      
};

struct Trade {
    int64_t entry_bar;
    int64_t exit_bar;
    double  entry_price;
    double  exit_price;
    double  size;       
    bool    is_long;
    double  pnl;
};

struct Metrics {
    double total_return = 0.0;
    double cagr         = 0.0;
    double sharpe       = 0.0;
    double sortino      = 0.0;
    double max_drawdown = 0.0;
    double calmar       = 0.0;
    int    trade_count  = 0;
    double win_rate     = 0.0;
    double profit_factor = 0.0;
    double exposure     = 0.0;
};

struct SimResult {
    std::vector<double> equity_curve;
    std::vector<Trade>  trades;
    Metrics             metrics;
};

// Signal[i] is executed on open[i+1]. Conflicting enter/exit of the same bar are ignored.
// The last open position is closed on close[n-1].
SimResult run_simulation(
    const double* open, const double* close, int64_t n,
    const uint8_t* enter_long, const uint8_t* exit_long,
    const uint8_t* enter_short, const uint8_t* exit_short,
    const double* size,
    const SimConfig& cfg);

} // namespace aura::sim

#endif