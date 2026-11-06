#!/usr/bin/env bash
# 回归检查（T-1 … T-9c）。用法: tests/checks.sh ./build/polyno_err_ana.exe
set -u
BIN="${1:?binary path required}"
fail=0
run() { "$BIN" "$@" 2>&1; }

check_contains() {  # desc haystack needle
    case "$2" in
        *"$3"*) return 0 ;;
        *) printf 'FAIL %s: 缺少 "%s"\n' "$1" "$3"; fail=$((fail+1)); return 1 ;;
    esac
}

out=$(run --real --absolute x 1e-4 -f '(1+x)^4' --vars --lim all)
check_contains "T-1 δf" "$out" '0.0004(1+x)^3'
check_contains "T-1 δf/f" "$out" '0.0004/(1+x)'

out=$(run --real --absolute x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars)
check_contains "T-3 代入值" "$out" '0.0016384'

out=$(run --real --relative x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars)
check_contains "T-4 δf" "$out" '0.00098304'
check_contains "T-4 δf/f" "$out" '0.00015'

out=$(run --real --absolute x 1e-4 y 1e-4 --estimated x=0.5 --estimated y=0.5 -f '(1+x*y)^5')
n=$(printf '%s\n' "$out" | grep -c '^  +0 ')
[ "$n" -eq 2 ] || { printf 'FAIL T-5 默认组数: %s != 2\n' "$n"; fail=$((fail+1)); }

out=$(run --real --absolute x 1e-4 y 1e-4 -f '(1+x*y)^5' --lim all)
n=$(printf '%s\n' "$out" | grep -cE '^  (\+0|-0|\+inf|-inf) ')
[ "$n" -eq 8 ] || { printf 'FAIL T-5 all 组数: %s != 8\n' "$n"; fail=$((fail+1)); }

out=$(run --real --absolute x 1e-4 -f 'a^b'); rc=$?
[ "$rc" -eq 1 ] || { printf 'FAIL T-7 退出码: %s\n' "$rc"; fail=$((fail+1)); }
check_contains "T-7 消息" "$out" 'exponent must be a constant'

out=$(run --real --absolute x 1e-4 -f 'sin(a)'); rc=$?
[ "$rc" -eq 1 ] || { printf 'FAIL T-8 退出码: %s\n' "$rc"; fail=$((fail+1)); }
check_contains "T-8 消息" "$out" 'unsupported function'

out=$(run --real --absolute x 1e-4 --estimated x=-1 -f '(1+x)^2' --lim all)
rows=$(printf '%s\n' "$out" | grep -E '^  (\+0|-0|\+inf|-inf) ')
[ "$(printf '%s\n' "$rows" | sed -n '1p' | awk '{print $2}')" = "+0" ] || { printf 'FAIL T-9 +0\n'; fail=$((fail+1)); }
[ "$(printf '%s\n' "$rows" | sed -n '3p' | awk '{print $2}')" = "+inf" ] || { printf 'FAIL T-9 +inf\n'; fail=$((fail+1)); }
nfin=$(printf '%s\n' "$rows" | awk '$2=="fin"' | wc -l)
[ "$nfin" -eq 0 ] || { printf 'FAIL T-9 δf 列出现 fin\n'; fail=$((fail+1)); }

out=$(run --real --absolute x 1e-4 --estimated x=-1 -f '(1+x)^3' --lim all)
uniq=$(printf '%s\n' "$out" | grep -E '^  (\+0|-0|\+inf|-inf) ' | awk '{print $2}' | sort -u | wc -l)
[ "$uniq" -eq 4 ] || { printf 'FAIL T-9b 类型数: %s != 4\n' "$uniq"; fail=$((fail+1)); }

out=$(run --real --absolute x 1e-4 --estimated x=-1 -f '(1+x)^(1/2)' --lim all)
check_contains "T-9c p0" "$out" '+0       +0'
check_contains "T-9c n0" "$out" '-0       undef'

if [ "$fail" -eq 0 ]; then
    echo "ok: 全部回归用例通过"
    exit 0
fi
echo "失败: $fail 项"
exit 1
