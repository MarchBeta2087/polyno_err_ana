#!/usr/bin/env bash
# 性能冒烟：同一公式重复 N 次，报告总耗时（毫秒）。
# 用法: scripts/bench.sh [BIN] [N] [--save]
#   --save 追加一行到 bench/history.csv（可用 BENCH_CSV 改路径，CI 用临时文件）
set -u
BIN="${1:-./build/polyno_err_ana.exe}"
N="${2:-20}"
SAVE="${3:-}"
HERE="$(cd "$(dirname "$0")" && pwd)"
CSV="${BENCH_CSV:-$HERE/../bench/history.csv}"
LAST_MS=0

run() {
    local label="$1"; shift
    local t0 t1
    t0=$(date +%s%N)
    for _ in $(seq "$N"); do "$BIN" "$@" > /dev/null 2>&1; done
    t1=$(date +%s%N)
    LAST_MS=$(( (t1 - t0) / 1000000 ))
    printf '%-42s %8d ms\n' "$label" "$LAST_MS" >&2
}

rev=$(git rev-parse --short HEAD 2>/dev/null || echo none)
kernel=$("$BIN" -V 2>/dev/null | sed -E 's/.*kernel: ([a-z]+).*/\1/')
bits=$("$BIN" -V 2>/dev/null | sed -E 's/.* ([0-9]+) bit.*/\1/')
now=$(date -u +%Y-%m-%dT%H:%M:%SZ)

echo "== bench: N=$N kernel=$kernel bits=$bits rev=$rev =="

run "(1+x)^50*(1+y)^50" -R x 1e-4 y 1e-4 -f '(1+x)^50*(1+y)^50' --vars; c1=$LAST_MS
run "(1+x)^200" -R x 1e-4 -f '(1+x)^200' --vars;                    c2=$LAST_MS
run "多变量 8 项" -R a 1e-4 b 1e-4 c 1e-4 -f '(a+b+c)^12*(1+a*b*c)^6' --vars; c3=$LAST_MS
run "复数指数 (1+z)^(1+i)" -C z 1e-4 1e-4 -f '(1+z)^(1+i)' --vars;   c4=$LAST_MS

if [ "$SAVE" = "--save" ]; then
    [ -f "$CSV" ] || echo "utc,rev,kernel,bits,n,case1_ms,case2_ms,case3_ms,case4_ms" > "$CSV"
    echo "$now,$rev,$kernel,$bits,$N,$c1,$c2,$c3,$c4" >> "$CSV"
    echo "已追加: $CSV" >&2
fi
