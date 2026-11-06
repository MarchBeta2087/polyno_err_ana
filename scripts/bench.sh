#!/usr/bin/env bash
# 粗略性能冒烟：同一公式重复 N 次，输出总耗时（毫秒）。
set -u
BIN="${1:-./build/polyno_err_ana.exe}"
N="${2:-20}"
run() {
    local label="$1"; shift
    local t0 t1
    t0=$(date +%s%N)
    for _ in $(seq "$N"); do "$BIN" "$@" > /dev/null 2>&1; done
    t1=$(date +%s%N)
    printf '%-42s %8d ms\n' "$label" $(( (t1 - t0) / 1000000 ))
}
run "(1+x)^50*(1+y)^50 展开+约分" -R x 1e-4 y 1e-4 -f '(1+x)^50*(1+y)^50' --vars
run "(1+x)^200 高次幂" -R x 1e-4 -f '(1+x)^200' --vars
run "多变量 8 项" -R a 1e-4 b 1e-4 c 1e-4 -f '(a+b+c)^12*(1+a*b*c)^6' --vars
run "复数指数（1+z)^(1+i)" -C z 1e-4 1e-4 -f '(1+z)^(1+i)' --vars
