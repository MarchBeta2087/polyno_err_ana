#!/usr/bin/env bash
# 打印最近 N 条基准并标注相对上一条的变化（正数 = 变慢）。
set -u
CSV="$(cd "$(dirname "$0")/.." && pwd)/bench/history.csv"
N="${1:-5}"
[ -f "$CSV" ] || { echo "还没有历史记录，先跑 scripts/bench.sh <bin> 20 --save"; exit 1; }
echo "utc                      rev      kernel     bits n    case1   Δ      case2   Δ      case3   Δ      case4   Δ"
{ tail -n +2 "$CSV" | tail -n "$N"; } | awk -F, '
{
    if (nr > 0) {
        printf "%-24s %-8s %-10s %-4s %-4s", $1, $2, $3, $4, $5;
        for (i = 6; i <= 9; i++) {
            d = $i - prev[i];
            printf " %6d(%+d)", $i, d;
        }
        printf "\n";
    } else {
        printf "%-24s %-8s %-10s %-4s %-4s %6d     %6d     %6d     %6d\n",
               $1, $2, $3, $4, $5, $6, $7, $8, $9;
    }
    for (i = 6; i <= 9; i++) prev[i] = $i;
    nr++;
}'
