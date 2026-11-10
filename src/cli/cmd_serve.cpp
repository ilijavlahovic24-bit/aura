#include "cli/cmd.hpp"
#include "data/aurd.hpp"
#include "lang/parser.hpp"
#include "lang/sema.hpp"
#include "lang/ir_builder.hpp"
#include "codegen/llvm_codegen.hpp"
#include "codegen/jit.hpp"
#include "codegen/context.hpp"
#include "sim/simulator.hpp"
#include "server/http_server.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace aura::cli {

static const char* kDashboard = R"DASH(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><title>Aura Dashboard</title>
<style>
  * { box-sizing: border-box; }
  body { font-family: ui-monospace, monospace; background:#0e1116; color:#c9d1d9; margin:0; padding:24px; }
  h1 { color:#7cf; font-size:22px; margin:0 0 4px; }
  .sub { color:#8b949e; font-size:12px; margin-bottom:20px; }
  .controls { background:#161b22; border:1px solid #30363d; padding:14px; border-radius:8px; margin-bottom:20px; display:flex; gap:10px; align-items:center; flex-wrap:wrap; }
  label { color:#8b949e; font-size:13px; }
  input { background:#0d1117; color:#c9d1d9; border:1px solid #30363d; border-radius:4px; padding:6px 10px; font-family:inherit; font-size:13px; }
  input.narrow { width:90px; }
  button { background:#238636; color:white; border:0; padding:8px 16px; border-radius:4px; font-family:inherit; font-size:13px; cursor:pointer; }
  button:disabled { background:#30363d; cursor:wait; }
  .metrics { display:grid; grid-template-columns: repeat(auto-fit,minmax(150px,1fr)); gap:12px; margin-bottom:20px; }
  .metric { background:#161b22; border:1px solid #30363d; padding:12px; border-radius:6px; }
  .metric .lbl { color:#8b949e; font-size:11px; text-transform:uppercase; }
  .metric .v { color:#7cf; font-size:20px; margin-top:4px; font-weight:600; }
  .metric .v.neg { color:#f85149; }
  .panel { background:#161b22; border:1px solid #30363d; padding:16px; border-radius:8px; margin-bottom:20px; }
  .panel h2 { font-size:14px; color:#8b949e; margin:0 0 10px; text-transform:uppercase; }
  svg { display:block; width:100%; height:220px; }
  .axis { stroke:#30363d; stroke-width:1; }
  .grid { stroke:#21262d; stroke-width:1; }
  .line { fill:none; stroke:#7cf; stroke-width:2; }
  .area { fill:rgba(124,252,255,0.08); }
  .label { fill:#8b949e; font-size:10px; }
  table { width:100%; border-collapse:collapse; font-size:12px; }
  th, td { text-align:left; padding:6px 8px; border-bottom:1px solid #21262d; }
  th { color:#8b949e; font-weight:normal; text-transform:uppercase; font-size:10px; }
  .err { color:#f85149; background:#2d1b1f; border:1px solid #5d2228; padding:10px; border-radius:6px; }
</style></head>
<body>
<h1>Aura Backtesting Engine</h1>
<div class="sub">JIT-compiled strategy over mmap'd historical data</div>

<div class="controls">
  <label>Strategy</label>
  <input id="strat" value="examples/mean_reversion.aura" size="36">
  <label>Capital</label>
  <input id="capital" class="narrow" value="10000">
  <label>Fee bps</label>
  <input id="fee" class="narrow" value="1">
  <label>Slippage bps</label>
  <input id="slip" class="narrow" value="2">
  <button id="run" onclick="runBacktest()">Run</button>
</div>

<div id="output"></div>

<script>
async function runBacktest() {
  const btn = document.getElementById('run');
  btn.disabled = true; btn.textContent = 'Running...';
  const out = document.getElementById('output');
  try {
    const params = new URLSearchParams({
      strategy: document.getElementById('strat').value,
      capital:  document.getElementById('capital').value,
      fee:      document.getElementById('fee').value,
      slip:     document.getElementById('slip').value,
    });
    const r = await fetch('/api/run?' + params);
    const j = await r.json();
    if (j.error) { out.innerHTML = '<div class="err">' + j.error + '</div>'; return; }
    render(j);
  } catch (e) {
    out.innerHTML = '<div class="err">Request failed: ' + e + '</div>';
  } finally {
    btn.disabled = false; btn.textContent = 'Run';
  }
}

function fmtPct(v) { return (v * 100).toFixed(2) + '%'; }
function fmtNum(v) { return Number(v).toFixed(3); }

function render(j) {
  const m = j.metrics;
  const metrics = [
    ['Total Return', fmtPct(m.total_return), m.total_return < 0],
    ['Sharpe',       fmtNum(m.sharpe),        m.sharpe < 0],
    ['Max Drawdown', fmtPct(m.max_drawdown),  true],
    ['Trades',       m.trade_count,           false],
    ['Win Rate',     fmtPct(m.win_rate),      false],
    ['Profit Factor',fmtNum(m.profit_factor), false],
  ];
  let html = '<div class="metrics">';
  for (const [lbl, v, neg] of metrics) {
    html += '<div class="metric"><div class="lbl">' + lbl + '</div><div class="v ' + (neg?'neg':'') + '">' + v + '</div></div>';
  }
  html += '</div>';

  html += '<div class="panel"><h2>Equity Curve</h2>' + equitySvg(j.equity) + '</div>';
  html += '<div class="panel"><h2>Drawdown</h2>' + ddSvg(j.equity) + '</div>';
  html += '<div class="panel"><h2>Trades (' + j.trades.length + ')</h2>' + tradesTable(j.trades) + '</div>';
  document.getElementById('output').innerHTML = html;
}

function equitySvg(data) {
  const W = 900, H = 220, P = {l:60, r:20, t:20, b:30};
  const min = Math.min(...data), max = Math.max(...data);
  const range = (max - min) || 1;
  const xs = i => P.l + (i / (data.length-1)) * (W-P.l-P.r);
  const ys = v => H - P.b - ((v - min) / range) * (H-P.t-P.b);

  let path = '';
  data.forEach((v, i) => path += (i?'L':'M') + xs(i) + ',' + ys(v) + ' ');
  // area fill
  const area = path + `L${xs(data.length-1)},${H-P.b} L${xs(0)},${H-P.b} Z`;

  let grid = '', labels = '';
  for (let k = 0; k <= 4; ++k) {
    const v = min + (range * k / 4);
    const y = ys(v);
    grid += `<line class="grid" x1="${P.l}" x2="${W-P.r}" y1="${y}" y2="${y}"/>`;
    labels += `<text class="label" x="${P.l-6}" y="${y+3}" text-anchor="end">${v.toFixed(0)}</text>`;
  }
  return `<svg viewBox="0 0 ${W} ${H}">${grid}${labels}
    <path class="area" d="${area}"/><path class="line" d="${path}"/></svg>`;
}

function ddSvg(data) {
  const W = 900, H = 160, P = {l:60, r:20, t:20, b:20};
  let peak = data[0], dd = [];
  for (const v of data) { peak = Math.max(peak, v); dd.push((peak-v)/peak); }
  const maxDd = Math.max(...dd) || 1;
  const xs = i => P.l + (i / (data.length-1)) * (W-P.l-P.r);
  const ys = v => H - P.b - (v/maxDd) * (H-P.t-P.b);
  let path = '';
  dd.forEach((v, i) => path += (i?'L':'M') + xs(i) + ',' + ys(v) + ' ');
  const area = path + `L${xs(data.length-1)},${H-P.b} L${xs(0)},${H-P.b} Z`;
  return `<svg viewBox="0 0 ${W} ${H}">
    <line class="axis" x1="${P.l}" x2="${W-P.r}" y1="${H-P.b}" y2="${H-P.b}"/>
    <path d="${area}" fill="rgba(248,81,73,0.15)"/>
    <path d="${path}" fill="none" stroke="#f85149" stroke-width="2"/></svg>`;
}

function tradesTable(trades) {
  if (!trades.length) return '<div class="sub">no trades</div>';
  let html = '<table><thead><tr><th>Entry</th><th>Exit</th><th>Side</th><th>Entry px</th><th>Exit px</th><th>Size</th><th>PnL</th></tr></thead><tbody>';
  for (const t of trades) {
    const cls = t.pnl < 0 ? 'style="color:#f85149"' : 'style="color:#3fb950"';
    html += `<tr><td>${t.entry}</td><td>${t.exit}</td><td>${t.long?'LONG':'SHORT'}</td>
      <td>${t.entry_price.toFixed(2)}</td><td>${t.exit_price.toFixed(2)}</td>
      <td>${(t.size*100).toFixed(1)}%</td><td ${cls}>${t.pnl.toFixed(2)}</td></tr>`;
  }
  html += '</tbody></table>';
  return html;
}
</script>
</body></html>)DASH";

static std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            auto hex = s.substr(i + 1, 2);
            try { out += static_cast<char>(std::stoi(hex, nullptr, 16)); }
            catch (...) { out += s[i]; }
            i += 2;
        } else if (s[i] == '+') {
            out += ' ';
        } else {
            out += s[i];
        }
    }
    return out;
}

static std::map<std::string, std::string> parse_query(const std::string& qs) {
    std::map<std::string, std::string> out;
    size_t i = 0;
    while (i < qs.size()) {
        auto amp = qs.find('&', i);
        if (amp == std::string::npos) amp = qs.size();
        auto chunk = qs.substr(i, amp - i);
        auto eq = chunk.find('=');
        if (eq != std::string::npos) {
            out[url_decode(chunk.substr(0, eq))] = url_decode(chunk.substr(eq + 1));
        }
        i = amp + 1;
    }
    return out;
}

int cmd_serve(const Args& a) {
    if (!a.has("data")) {
        std::cerr << "usage: aura serve --data <data.aurd> [--port 8080]\n";
        return ARG_ERROR;
    }
    const std::string data_file = a.get("data");
    const int port = static_cast<int>(a.get_int("port", 8080));

    // Otvori dataset jednom
    data::aurd::Dataset ds;
    try { ds = data::aurd::Dataset::open(data_file); }
    catch (const data::aurd::DatasetError& e) {
        std::cerr << "aura: data error: " << e.what() << "\n";
        return DATA_ERROR;
    }
    const int64_t n = static_cast<int64_t>(ds.size());

    auto handler = [&](const std::string& path) -> std::string {
        if (path == "/" || path == "/index.html") return kDashboard;
        if (path.rfind("/api/run", 0) == 0) {
            auto qpos = path.find('?');
            if (qpos == std::string::npos) return R"({"error":"missing query"})";
            auto q = parse_query(path.substr(qpos + 1));
            auto it = q.find("strategy");
            if (it == q.end()) return R"({"error":"missing strategy"})";
            std::string strat_file = it->second;

            std::ifstream f(strat_file);
            if (!f) return R"({"error":"cannot open strategy file"})";

            lang::Parser parser(f);
            auto strat = parser.parse();
            if (!parser.errors().empty())
                return R"({"error":"parse error: )" + json_escape(parser.errors()[0]) + "\"}";
            lang::SemanticAnalyzer sema;
            if (!sema.analyze(*strat, strat_file))
                return R"({"error":"semantic error: )" + json_escape(sema.errors()[0]) + "\"}";

            std::vector<double> params;
            for (const auto& p : strat->params) {
                if (auto fe = dynamic_cast<lang::FloatExpr*>(p.init.get())) params.push_back(fe->value);
                else if (auto ie = dynamic_cast<lang::IntExpr*>(p.init.get())) params.push_back((double)ie->value);
                else params.push_back(0.0);
            }

            lang::IRBuilder irb;
            auto ir = irb.build(*strat);

            codegen::LLVMCodegen cg;
            auto mod = cg.generate(ir);
            const int scratch_slots = cg.used_scratch_slots();

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

            codegen::JITEngine jit;
            KernelFn kernel = jit.compile(std::move(mod));
            kernel(&ctx);

            sim::SimConfig cfg;
            cfg.initial_capital = 10000.0;
            cfg.fee_bps         = 1.0;
            cfg.slippage_bps    = 2.0;
            cfg.timeframe_sec   = ds.timeframe();
            if (q.count("capital")) try { cfg.initial_capital = std::stod(q["capital"]); } catch(...){}
            if (q.count("fee"))     try { cfg.fee_bps = std::stod(q["fee"]); } catch(...){}
            if (q.count("slip"))    try { cfg.slippage_bps = std::stod(q["slip"]); } catch(...){}

            auto r = sim::run_simulation(
                ctx.open, ctx.close, n,
                el.data(), xl.data(), es.data(), xs.data(), sz.data(), cfg);

            std::ostringstream os;
            os << "{\"metrics\":{"
               << "\"total_return\":" << r.metrics.total_return << ","
               << "\"sharpe\":" << r.metrics.sharpe << ","
               << "\"max_drawdown\":" << r.metrics.max_drawdown << ","
               << "\"trade_count\":" << r.metrics.trade_count << ","
               << "\"win_rate\":" << r.metrics.win_rate << ","
               << "\"profit_factor\":" << r.metrics.profit_factor << "},"
               << "\"trades\":[";
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
            os << "],\"equity\":[";
            for (size_t i = 0; i < r.equity_curve.size(); ++i) {
                if (i) os << ",";
                os << r.equity_curve[i];
            }
            os << "]}";
            return os.str();
        }
        return "";
    };

    server::HttpServer srv(port, handler);
    srv.run();
    return OK;
}

} // namespace aura::cli