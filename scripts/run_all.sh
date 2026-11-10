#!/usr/bin/env bash
# scripts/run_all.sh — kompletan pipeline od build-a do dashboarda
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

echo "=== 1. Build (Release) ==="
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"

echo "=== 2. Unit tests ==="
ctest --test-dir build --output-on-failure

echo "=== 3. Generate synthetic data (OU process, hourly bars) ==="
mkdir -p /tmp/aura
python3 - <<'PY' > /tmp/aura/prices.csv
import random
random.seed(42)
x, theta, mu, sigma = 100.0, 0.05, 100.0, 1.0
print('ts,open,high,low,close,volume')
t = 1700000000
for i in range(50000):
    x = x + theta*(mu-x) + sigma*random.gauss(0,1)
    print(f'{t},{x-0.02:.4f},{x+0.5:.4f},{x-0.5:.4f},{x:.4f},1000')
    t += 3600
PY

echo "=== 4. Import CSV → .aurd ==="
./build/aura_cli import /tmp/aura/prices.csv -o /tmp/aura/prices.aurd

echo "=== 5. Dataset info ==="
./build/aura_cli info /tmp/aura/prices.aurd

echo "=== 6. Strategy check ==="
./build/aura_cli check examples/mean_reversion.aura

echo "=== 7. Single backtest (text) ==="
./build/aura_cli run examples/mean_reversion.aura --data /tmp/aura/prices.aurd \
    --fee-bps 1 --slippage-bps 2

echo "=== 8. Single backtest (JSON) ==="
./build/aura_cli run examples/mean_reversion.aura --data /tmp/aura/prices.aurd \
    --fee-bps 1 --slippage-bps 2 --format json > /tmp/aura/result.json
echo "  (saved to /tmp/aura/result.json, $(wc -c < /tmp/aura/result.json) bytes)"

echo "=== 9. Grid sweep (4 threads) ==="
./build/aura_cli sweep examples/mean_reversion.aura --data /tmp/aura/prices.aurd \
    --grid lookback=10:40:10 --grid z_entry=1.0:3.0:0.5 --threads 4 > /tmp/aura/sweep.txt
head -5 /tmp/aura/sweep.txt

echo "=== 10. Determinism test (1 thread vs 8 threads) ==="
./build/aura_cli sweep examples/mean_reversion.aura --data /tmp/aura/prices.aurd \
    --grid lookback=10:40:10 --grid z_entry=1.0:3.0:0.5 --threads 1 > /tmp/aura/s1.txt
./build/aura_cli sweep examples/mean_reversion.aura --data /tmp/aura/prices.aurd \
    --grid lookback=10:40:10 --grid z_entry=1.0:3.0:0.5 --threads 8 > /tmp/aura/s8.txt
if diff -q /tmp/aura/s1.txt /tmp/aura/s8.txt > /dev/null; then
    echo "  DETERMINIZAM OK: --threads 1 and --threads 8 produce identical output"
else
    echo "  DETERMINIZAM FAILED"
    exit 1
fi

echo "=== 11. Sanitizer build (ASan + UBSan) ==="
cmake -B build-asan -DAURA_SANITIZE=ON
cmake --build build-asan -j"$(nproc)"
./build-asan/aura_cli run examples/mean_reversion.aura --data /tmp/aura/prices.aurd \
    --fee-bps 1 --slippage-bps 2 > /dev/null
./build-asan/aura_cli sweep examples/mean_reversion.aura --data /tmp/aura/prices.aurd \
    --grid lookback=10:20:10 --grid z_entry=1.0:2.0:0.5 --threads 4 > /dev/null
echo "  ASan + UBSan clean"

echo
echo "=========================================="
echo "  ALL CHECKS PASSED"
echo "  Dashboard: ./build/aura_cli serve --data /tmp/aura/prices.aurd --port 8080"
echo "=========================================="