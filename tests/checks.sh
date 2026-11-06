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

# --- 里程碑 2：复数与分支号 ---
out=$(run -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+z)^(1+i)' -k 2 --vars)
check_contains "T-6 branch" "$out" 'branch k = 2'
check_contains "T-6 复数输出" "$out" '(1+z)^(0+1i)'
check_contains "T-6 δf/f 约分" "$out" '(0+0.0002i)/(1+z)'
check_contains "T-6 分类 c0" "$out" '+0       c0'
check_contains "T-6 分类 cinf" "$out" '+inf     cinf'

out=$(run -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+z)^2' --vars)
check_contains "复数幂符号形式" "$out" '(0.0002+0.0002i)(1+z)'
check_contains "复数 i 常量解析" "$out" '(0.0001+0.0001i)'

b0=$("$BIN" -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+z)^(1+i)' 2>&1 | grep -A1 'δ_{R+}f' | tail -1)
b2=$("$BIN" -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+z)^(1+i)' -k 2 2>&1 | grep -A1 'δ_{R+}f' | tail -1)
[ "$b0" != "$b2" ] || { printf 'FAIL 分支号未影响求值
'; fail=$((fail+1)); }

# --- 里程碑 4：real-root / NaN / Inf 呈现 ---
out=$(run -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/3)' --vars)
check_contains "复主值" "$out" '(1+1.73205i)'

out=$(run -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/3)' --real-root --vars)
check_contains "real-root 实根" "$out" '(x+(-2))'
check_contains "real-root 数值" "$out" '-6.6667e-05'

out=$(run -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/2)' --real-root --vars)
check_contains "偶分母 → Undefined" "$out" 'Undefined'

out=$(run -R x 1e-4 -f 'x+0^0' --lim all)
check_contains "0^0 边界 Undefined" "$out" 'Undefined  Undefined'
check_contains "0^0 分类 undef" "$out" 'undef'

out=$(run -R x 1e-4 -f '1/(x-x)' --lim all)
check_contains "除零 Undefined" "$out" 'Undefined'

"$BIN" -R x 1e-4 -f '(-8)^(1/3)' --real-root -k 2 > /dev/null 2>&1
[ $? -eq 2 ] || { printf 'FAIL --real-root 与 -k 未互斥
'; fail=$((fail+1)); }

# 整数指数时 -k 不改变 δf 行（只多一行 banner）
a=$("$BIN" -R x 1e-4 -f '(1+x)^3' --vars | grep '^δf    =')
b=$("$BIN" -R x 1e-4 -f '(1+x)^3' -k 2 --vars | grep '^δf    =')
[ "$a" = "$b" ] || { printf 'FAIL 整数指数时 -k 影响了结果
'; fail=$((fail+1)); }

# --- 里程碑 5：--lim 单状态 / N-5 / 混合 -R -C ---
out=$(run -R x 1e-4 --lim x:n0 -f '(1+x)^4')
n=$(printf '%s\n' "$out" | grep -cE '^  (\+0|-0|\+inf|-inf) ')
[ "$n" -eq 1 ] || { printf 'FAIL --lim 单状态组数: %s != 1\n' "$n"; fail=$((fail+1)); }
check_contains "--lim 单状态标签" "$out" "-0"

out=$(run -R x 1e-4 --relative --estimated x=0 -f '(1+x)^4')
check_contains "N-5 提示" "$out" '相对误差失效'

out=$(run -R x 1e-4 -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+x)^2+z' --vars)
check_contains "混合 -R/-C" "$out" '(0.0001+0.0001i)'
check_contains "混合分类行数" "$out" '+0       c0'

# --- CLI 错误路径（退出码 2） ---
expect_rc2() {
    "$BIN" "$@" > /dev/null 2>&1
    local rc=$?
    [ "$rc" -eq 2 ] || { printf 'FAIL 退出码应为 2（实际 %s）: %s
' "$rc" "$*"; fail=$((fail+1)); }
}
expect_rc2 --nope -R x 1e-4 -f '(1+x)'
expect_rc2 -R x 1e-4 -p 1 -f '(1+x)'
expect_rc2 -R x 1e-4 -p abc -f '(1+x)'
expect_rc2 -R x 1e-4 x 1e-4 -f '(1+x)'
expect_rc2 -R x 1e-4 --lim z:p0 -f '(1+x)'
expect_rc2 -R x 1e-4 --limit all -f '(1+x)'
expect_rc2 --real x 1e-4 --kernel nope -f '(1+x)'
"$BIN" > /dev/null 2>&1; [ $? -eq 2 ] || { printf 'FAIL 缺少 -f 应退出 2
'; fail=$((fail+1)); }

if [ "$fail" -eq 0 ]; then
    echo "ok: 全部回归用例通过"
    exit 0
fi
echo "失败: $fail 项"
exit 1
