#!/usr/bin/env bash
# 性能门禁：跑 bench 的 4 个用例，单次运行耗时超过阈值则失败（默认 1200 ms/次）。
# 阈值故意宽松（本地约 50–90 ms/次），只拦灾难性回归（如符号归并退化为 O(n²)）。
# 用法: scripts/bench_gate.sh [BIN] [N] [--warn-only]
set -u
BIN="${1:-./build/polyno_err_ana.exe}"
N="${2:-10}"
MODE="${3:-}"
CEIL_MS="${BENCH_CEIL_MS:-1200}"

HERE="$(cd "$(dirname "$0")" && pwd)"
TMP=$(mktemp)
BENCH_CSV="$TMP" bash "$HERE/bench.sh" "$BIN" "$N" --save > /dev/null 2>&1
row=$(tail -1 "$TMP")
rm -f "$TMP"

if [ -z "$row" ]; then
    echo "bench_gate: 未取得结果（bench.sh 失败？）"
    exit 1
fi

echo "bench_gate: N=$N, 阈值=${CEIL_MS} ms/次"
fail=0
IFS=',' read -r utc rev kernel bits n c1 c2 c3 c4 <<< "$row"
i=1
for ms in "$c1" "$c2" "$c3" "$c4"; do
    [ -z "$ms" ] && continue
    per=$(( ms / (n > 0 ? n : 1) ))
    printf '  case%s: %4d ms/次 (%d 次共 %d ms)\n' "$i" "$per" "$n" "$ms"
    if [ "$per" -gt "$CEIL_MS" ]; then
        echo "  超阈值: case$i ${per} ms/次 > ${CEIL_MS}"
        fail=$((fail + 1))
    fi
    i=$((i + 1))
done

if [ "$fail" -eq 0 ]; then
    echo "bench_gate: 通过"
    exit 0
fi
if [ "$MODE" = "--warn-only" ]; then
    echo "bench_gate: 有 $fail 项超阈值（warn-only，不阻塞）"
    exit 0
fi
echo "bench_gate: 失败（$fail 项超阈值）"
exit 1
