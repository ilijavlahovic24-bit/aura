# Aura

**A SIMD-accelerated backtesting engine with a JIT-compiled strategy language.**

Aura lets you describe a trading strategy in a small statically-typed DSL, compiles it to native
code through LLVM at runtime, and evaluates it over hundreds of millions of bars using
vectorized (AVX2) kernels and a deterministic portfolio simulator.

# Status: In Progress
---

## Why Aura

- **Fast by construction.** Columnar storage, zero-copy `mmap`, series-at-a-time evaluation,
  hand-written AVX2 kernels with scalar reference implementations.
- **Strategies as code, not config.** A tiny language with series as a first-class type,
  compiled via LLVM ORC JIT. Change a parameter without recompiling.
- **No look-ahead by design.** The language has no way to reference future bars, and signals
  fill at the next bar's open.
- **Deterministic.** Same input, same output, bit for bit, independent of thread count.
- **Verifiable.** Every JIT result is checked against a reference IR interpreter.

---

## Example

```
strategy MeanReversion {
    param lookback : int   = 20;
    param z_entry  : float = 2.0;

    let m = sma(close, lookback);
    let s = stdev(close, lookback);
    let z = (close - m) / s;

    enter long  when z < -z_entry;
    exit  long  when z > 0.0;
    enter short when z > z_entry;
    exit  short when z < 0.0;

    size = 1.0;
}
```

```bash
aura import  prices.csv -o prices.aurd --compress
aura run     mean_reversion.aura --data prices.aurd --fee-bps 1 --slippage-bps 2
aura sweep   mean_reversion.aura --data prices.aurd \
             --grid lookback=10:60:5 --grid z_entry=1.0:3.0:0.25 --sort-by sharpe
aura serve   --data prices.aurd --port 8080
```

---

## Architecture

```
.aura source
   │  Flex / Bison
   ▼
  AST ──► semantic analysis (types, broadcast, window constraints)
   │
   ▼
  IR ──► LLVM IR ──► optimization passes ──► ORC JIT ──► kernel(ctx)
   │                                                        │
   └──► reference interpreter  ◄── differential tests ──────┘

 .aurd (columnar, Gorilla-compressed)
   │  mmap / block decompression
   ▼
 aligned SoA buffers ──► signals (vectorized, parallel)
                              │
                              ▼
                      portfolio simulator (sequential)
                              │
                              ▼
                    metrics · trades · equity curve
                              │
                       CLI  /  HTTP API  /  dashboard
```

| Component | Directory | Description |
|---|---|---|
| Data engine | `src/data/` | `.aurd` format, CSV import, Gorilla compression, `mmap` |
| Frontend | `src/lang/` | `lexer.l`, `parser.y`, AST, semantic analysis |
| Codegen | `src/codegen/` | IR, LLVM IR emission, ORC JIT, reference interpreter |
| Runtime | `src/rt/` | AVX2 kernels (`sma`, `ema`, `stdev`, `highest/lowest`, …) + scalar references |
| Simulator | `src/sim/` | Order execution model, positions, fees, metrics |
| CLI / server | `src/cli/`, `src/server/` | Commands, JSON output, REST API |
| Dashboard | `web/` | React UI: equity, drawdown, metrics, trades |

---

## The `.aurd` format

Little-endian columnar format with a 64-byte header, split into blocks of 4096 bars.
Columns: `ts` (`i64`), `open`, `high`, `low`, `close`, `volume` (`f64`).

- **Uncompressed:** 32-byte aligned columns, used directly from `mmap` with no copying.
- **Compressed:** Gorilla-style delta-of-delta for timestamps and XOR encoding for prices;
  lossless, bit-identical round trip.

---

## Language overview

**Types:** `int`, `float`, `bool`, `series<float>`, `series<bool>`. Scalars broadcast into series.

**Built-ins:** `sma`, `ema`, `stdev`, `rsi`, `atr`, `highest`, `lowest`, `shift`, `crossover`,
`crossunder`, `abs`, `sqrt`, `log`, `min`, `max`, `where`.

**Semantics worth knowing:**
- Windowed functions yield `NaN` for the first `n-1` bars; comparisons with `NaN` are `false`.
- `shift(x, k)` requires `k >= 0`. There is no way to read the future.
- Window arguments must be constant expressions (literal or `param`).
- The language has no loops or recursion by design.

---

## Execution model

- A signal computed at the close of bar `t` fills at the **open of bar `t+1`**.
- Costs: `fee_bps` per side and `slippage_bps` against the trade.
- Position size is a fraction of equity in `[0, 1]`; no leverage, no margin calls.
- Conflicting `enter long` / `enter short` on the same bar are both ignored.
- Any open position is closed at the last bar's close.

**Metrics:** total return, CAGR, Sharpe, Sortino, max drawdown, Calmar, trade count, win rate,
profit factor, exposure.

---

## Performance targets

Measured on a single reference machine (to be documented with CPU, compiler, and flags).

| Benchmark | Target |
|---|---|
| SMA crossover, 10M bars, signals + simulation, 1 thread | < 150 ms |
| `stdev` + `rsi` strategy, 10M bars, 1 thread | < 400 ms |
| 10×10 parameter sweep, 1M bars, 8 threads | < 2 s |
| Open a 100M-bar uncompressed dataset | < 50 ms |

Results will be published in `bench/RESULTS.md` once measured.

---

## Building

**Requirements:** Linux x86-64 with AVX2, GCC 12+ or Clang 16+, CMake ≥ 3.22,
LLVM 17+, Flex 2.6+, Bison 3.8+.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Sanitizer build:

```bash
cmake -B build-asan -DAURA_SANITIZE=ON
cmake --build build-asan -j && ctest --test-dir build-asan
```

---

## CLI reference

```
aura import   <in.csv> -o <out.aurd> [--compress]
aura info     <data.aurd>
aura check    <strategy.aura>
aura run      <strategy.aura> --data <data.aurd>
              [--param name=value ...] [--capital N] [--fee-bps N] [--slippage-bps N]
              [--format json|text] [--interp] [--emit-ir]
aura sweep    <strategy.aura> --data <data.aurd>
              --grid name=start:stop:step ... [--sort-by sharpe] [--threads N]
aura serve    [--port 8080] --data <data.aurd>
```

| Exit code | Meaning |
|---|---|
| 0 | Success |
| 1 | Argument or I/O error |
| 2 | Strategy program error (`file:line:col: error: message`) |
| 3 | Data error |
| 4 | Simulation went bankrupt |

`--format json` follows the schema in `docs/schema.json`.

---

## Testing

- **Unit tests** for the data engine (round trips, block boundaries, `-0.0`, denormals).
- **Golden tests** in `tests/public/`: input dataset, strategy, and `expected.json`.
  Trades must match exactly; numeric metrics within `1e-9` relative tolerance.
- **Differential tests:** JIT output vs. the reference interpreter; AVX2 kernels vs. scalar references.
- **Determinism tests:** identical output across thread counts and across compressed/uncompressed inputs.

---

## Roadmap

- [ ] **Phase 1: Data engine.** `.aurd`, `mmap`, CSV import, Gorilla compression
- [ ] **Phase 2: Language and JIT.** Flex/Bison frontend, semantic analysis, LLVM IR, ORC JIT, reference interpreter
- [ ] **Phase 3: Vectorized execution.** AVX2 kernels, simulator, metrics, parallel sweep
- [ ] **Phase 4: Tooling.** CLI, JSON schema, HTTP API, dashboard
- [ ] Stretch: multi-instrument portfolios, parser fuzzing, AVX-512 path, walk-forward analysis

---

## Repository layout

```
aura/
├── CMakeLists.txt
├── docs/          report, schema.json, grammar.md
├── src/           data/ lang/ codegen/ rt/ sim/ cli/ server/
├── web/           dashboard frontend
├── tests/         public/ (golden tests), unit/
├── bench/         benchmark harnesses
└── examples/      sample strategies
```

---

## Limitations

- Backtests are only as realistic as the execution model: no order book, no partial fills,
  no market impact, no leverage.
- Past performance in a simulation says nothing about future results. Aura is a research tool,
  not financial advice.

## License

MIT