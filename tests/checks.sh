#!/usr/bin/env bash
# 回归检查（T-1 … T-13、复数/分支、real-root、爆项、CLI 错误）。
# 用法: tests/checks.sh ./build/polyno_err_ana.exe
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

# 分类表的行：$2 为 δv 状态，$3 = δf，$4 = δf/f
limit_rows() { printf '%s\n' "$1" | awk '/极限分类/{f=1;next} f && $2 ~ /^(\+0|-0|\+inf|-inf|0|inf)$/'; }
count_rows() { limit_rows "$1" | wc -l; }
field_of() { limit_rows "$1" | awk -v s="$2" -v c="$3" '($2 "")==s {print $c}'; }

# ---- T-1 / T-3 / T-4 ----
out=$(run --real --absolute x 1e-4 -f '(1+x)^4' --vars --lim all)
check_contains "T-1 δf" "$out" '0.0004(1+x)^3'
check_contains "T-1 δf/f" "$out" '0.0004/(1+x)'

out=$(run --real --absolute x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars)
check_contains "T-3 代入值" "$out" '0.0016384'

out=$(run --real --relative x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars)
check_contains "T-4 δf" "$out" '0.00098304'
check_contains "T-4 δf/f" "$out" '0.00015'

# ---- T-5 组数 ----
out=$(run --real --absolute x 1e-4 y 1e-4 --estimated x=0.5 --estimated y=0.5 -f '(1+x*y)^5')
n=$(count_rows "$out")
[ "$n" -eq 2 ] || { printf 'FAIL T-5 默认组数: %s != 2\n' "$n"; fail=$((fail+1)); }

out=$(run --real --absolute x 1e-4 y 1e-4 -f '(1+x*y)^5' --lim all)
n=$(count_rows "$out")
[ "$n" -eq 8 ] || { printf 'FAIL T-5 all 组数: %s != 8\n' "$n"; fail=$((fail+1)); }

# ---- T-7 / T-8 解析错误 ----
out=$(run --real x 1e-4 -f 'a^b'); rc=$?
[ "$rc" -eq 1 ] || { printf 'FAIL T-7 退出码: %s\n' "$rc"; fail=$((fail+1)); }
check_contains "T-7 消息" "$out" 'exponent must be a constant'

out=$(run --real x 1e-4 -f 'sin(a)'); rc=$?
[ "$rc" -eq 1 ] || { printf 'FAIL T-8 退出码: %s\n' "$rc"; fail=$((fail+1)); }
check_contains "T-8 消息" "$out" 'unsupported function'

# ---- T-9 / T-9b / T-9c 高阶首项 ----
out=$(run --real --absolute x 1e-4 --estimated x=-1 -f '(1+x)^2' --lim all)
[ "$(field_of "$out" "+0" 3)" = "+0" ] || { printf 'FAIL T-9 +0\n'; fail=$((fail+1)); }
[ "$(field_of "$out" "+inf" 3)" = "+inf" ] || { printf 'FAIL T-9 +inf\n'; fail=$((fail+1)); }
[ "$(field_of "$out" "+0" 3)" != "fin" ] || { printf 'FAIL T-9 出现 fin\n'; fail=$((fail+1)); }

out=$(run --real --absolute x 1e-4 --estimated x=-1 -f '(1+x)^3' --lim all)
uniq=$(limit_rows "$out" | awk '{print $3}' | sort -u | wc -l)
[ "$uniq" -eq 4 ] || { printf 'FAIL T-9b 类型数: %s != 4\n' "$uniq"; fail=$((fail+1)); }

out=$(run --real --absolute x 1e-4 --estimated x=-1 -f '(1+x)^(1/2)' --lim all)
[ "$(field_of "$out" "+0" 3)" = "+0" ] || { printf 'FAIL T-9c p0\n'; fail=$((fail+1)); }
[ "$(field_of "$out" "-0" 3)" = "undef" ] || { printf 'FAIL T-9c n0\n'; fail=$((fail+1)); }

# ---- T-6 复数 + 分支号 ----
out=$(run -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+z)^(1+i)' -k 2 --vars)
check_contains "T-6 branch" "$out" 'branch k = 2'
check_contains "T-6 复数输出" "$out" '(1+z)^(0+1i)'
check_contains "T-6 δf/f 约分" "$out" '(0+0.0002i)/(1+z)'
[ "$(field_of "$out" "0" 3)" = "c0" ] || { printf 'FAIL T-6 c0\n'; fail=$((fail+1)); }
[ "$(field_of "$out" "inf" 4)" = "cinf" ] || { printf 'FAIL T-6 cinf\n'; fail=$((fail+1)); }

out=$(run -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+z)^2' --vars)
check_contains "复数幂符号形式" "$out" '(0.0002+0.0002i)(1+z)'
check_contains "复数 i 常量解析" "$out" '(0.0001+0.0001i)'

b0=$("$BIN" -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+z)^(1+i)' 2>&1 | awk '/δ_\{R\+\}f/{getline; print}')
b2=$("$BIN" -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+z)^(1+i)' -k 2 2>&1 | awk '/δ_\{R\+\}f/{getline; print}')
[ "$b0" != "$b2" ] || { printf 'FAIL 分支号未影响求值\n'; fail=$((fail+1)); }

# ---- T-10..T-13 real-root / NaN ----
out=$(run -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/3)' --vars)
check_contains "T-10 复主值" "$out" '(1+1.73205i)'

out=$(run -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/3)' --real-root --vars)
check_contains "T-11 real-root 实根" "$out" '(x+(-2))'
check_contains "T-11 real-root 数值" "$out" '-6.6667e-05'

out=$(run -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/2)' --real-root --vars)
check_contains "T-12 偶分母 Undefined" "$out" 'Undefined'

out=$(run -R x 1e-4 -f 'x+0^0' --lim all)
check_contains "T-13 0^0 Undefined" "$out" 'Undefined'
[ "$(field_of "$out" "+0" 3)" = "undef" ] || { printf 'FAIL T-13 0^0 分类\n'; fail=$((fail+1)); }

out=$(run -R x 1e-4 -f '1/(x-x)' --lim all)
check_contains "T-13 除零 Undefined" "$out" 'Undefined'

"$BIN" -R x 1e-4 -f '(-8)^(1/3)' --real-root -k 2 > /dev/null 2>&1
[ $? -eq 2 ] || { printf 'FAIL --real-root 与 -k 未互斥\n'; fail=$((fail+1)); }

a=$("$BIN" -R x 1e-4 -f '(1+x)^3' --vars | grep '^δf    =')
b=$("$BIN" -R x 1e-4 -f '(1+x)^3' -k 2 --vars | grep '^δf    =')
[ "$a" = "$b" ] || { printf 'FAIL 整数指数时 -k 影响了结果\n'; fail=$((fail+1)); }

# ---- 里程碑 5：单状态 --lim / N-5 / 混合 -R -C ----
out=$(run -R x 1e-4 --lim x:n0 -f '(1+x)^4')
n=$(count_rows "$out")
[ "$n" -eq 1 ] || { printf 'FAIL --lim 单状态组数: %s != 1\n' "$n"; fail=$((fail+1)); }
[ "$(field_of "$out" "-0" 3)" = "-0" ] || { printf 'FAIL --lim 单状态标签\n'; fail=$((fail+1)); }

out=$(run -R x 1e-4 --relative --estimated x=0 -f '(1+x)^4')
check_contains "N-5 提示" "$out" '相对误差失效'

out=$(run -R x 1e-4 -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+x)^2+z' --vars)
check_contains "混合 -R/-C" "$out" '(0.0001+0.0001i)'
nmix=$(limit_rows "$out" | awk '$2=="0" && $4=="c0"' | wc -l)
[ "$nmix" -eq 1 ] || { printf 'FAIL 混合分类行数: %s\n' "$nmix"; fail=$((fail+1)); }

# ---- 多规格 --lim ----
out=$(run -R x 1e-4 -f '(1+x)^4' --lim x:p0,x:ninf)
n=$(count_rows "$out")
[ "$n" -eq 2 ] || { printf 'FAIL 多规格 --lim 组数: %s != 2\n' "$n"; fail=$((fail+1)); }
[ "$(field_of "$out" "-inf" 3)" = "-inf" ] || { printf 'FAIL 多规格 --lim 内容\n'; fail=$((fail+1)); }

# ---- 里程碑 6：爆项保护 ----
out=$(run -R a 1e-4 b 1e-4 c 1e-4 d 1e-4 e 1e-4 f 1e-4 --max-terms 4 -f '(a+b)*(c+d)*(e+f)' --vars)
check_contains "项数上限退化" "$out" '符号展开不可用'
check_contains "退化后仍有分类" "$out" '极限分类'

out=$(run -R a 1e-4 b 1e-4 -f '(a+b)*(a+b)' --max-terms 1)
check_contains "上限 1 仍可算" "$out" '极限分类'

# ---- 里程碑 9：探测路径相对分类 / 复数相对模式 / 混合量纲 ----
out=$(run -C z 1e-4 1e-4 -f '(1+z)^(1+i)' -k 2 --lim all)
[ "$(field_of "$out" "0" 4)" = "c0" ] || { printf 'FAIL T-6 δf/f c0
'; fail=$((fail+1)); }
[ "$(field_of "$out" "inf" 4)" = "cinf" ] || { printf 'FAIL T-6 δf/f cinf
'; fail=$((fail+1)); }

out=$(run -C z 1e-4 1e-4 --relative --estimated z=1+0.5i -f '(1+z)^2' --vars)
check_contains "复数相对模式 δz" "$out" '(0.000111803+0.000111803i)'

out=$(run -R x 1e-4 -C z 1e-4 1e-4 -f 'z*(1+x)' --estimated z=1+1i --estimated x=0.5 --lim all)
nmix=$(limit_rows "$out" | awk '$3 ~ /^c/ && $4 ~ /^c/' | wc -l)
[ "$nmix" -eq 6 ] || { printf 'FAIL 混合量纲模分类: %s != 6
' "$nmix"; fail=$((fail+1)); }

# ---- 精度极值 ----
out=$(run -R x 1e-4 -f '(1+x)^4' --vars -p 2)
check_contains "p=2 报告精度" "$out" '2 bit'
check_contains "p=2 δf" "$out" '0.000366211'
[ "$(count_rows "$out")" -eq 4 ] || { printf 'FAIL p=2 分类行数
'; fail=$((fail+1)); }

out=$(run -R x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars -p 4096)
check_contains "p=4096 报告精度" "$out" '4096 bit'
check_contains "p=4096 δf 值" "$out" '0.0016384'

# ---- --lim 区间/分组 ----
out=$(run -R x 1e-4 -f '(1+x)^4' --lim x:0..inf)
[ "$(count_rows "$out")" -eq 4 ] || { printf 'FAIL x:0..inf 组数
'; fail=$((fail+1)); }
out=$(run -R x 1e-4 -f '(1+x)^4' --lim x:p0..pinf)
[ "$(count_rows "$out")" -eq 3 ] || { printf 'FAIL x:p0..pinf 组数
'; fail=$((fail+1)); }
out=$(run -R x 1e-4 -f '(1+x)^4' --lim x:zeros)
[ "$(count_rows "$out")" -eq 2 ] || { printf 'FAIL x:zeros 组数
'; fail=$((fail+1)); }
out=$(run -C z 1e-4 1e-4 -f '(1+z)^2' --lim z:zeros)
[ "$(count_rows "$out")" -eq 1 ] || { printf 'FAIL 复数 zeros 组数
'; fail=$((fail+1)); }

# ---- 求导阶段触发项数上限：同样退化为数值探测且退出码 0 ----
out=$(run -R a 1e-4 b 1e-4 --max-terms 3 -f '(a+b)^2*(a+b)^2' --vars --lim all)
rc=$?
[ "$rc" -eq 0 ] || { printf 'FAIL 求导阶段爆项应退出 0（实际 %s）
' "$rc"; fail=$((fail+1)); }
check_contains "求导阶段爆项退化" "$out" '符号展开不可用'
check_contains "求导阶段仍出分类" "$out" '极限分类'

# ---- 区间 n0..ninf ----
out=$(run -R x 1e-4 -f '(1+x)^4' --lim x:n0..ninf)
[ "$(count_rows "$out")" -eq 3 ] || { printf 'FAIL x:n0..ninf 组数
'; fail=$((fail+1)); }

# ---- CLI 错误路径（退出码 2） ----
expect_rc2() {
    "$BIN" "$@" > /dev/null 2>&1
    local rc=$?
    [ "$rc" -eq 2 ] || { printf 'FAIL 退出码应为 2（实际 %s）: %s\n' "$rc" "$*"; fail=$((fail+1)); }
}
expect_rc2 --nope -R x 1e-4 -f '(1+x)'
expect_rc2 -R x 1e-4 -p 1 -f '(1+x)'
expect_rc2 -R x 1e-4 -p abc -f '(1+x)'
expect_rc2 -R x 1e-4 x 1e-4 -f '(1+x)'
expect_rc2 -R x 1e-4 --lim z:p0 -f '(1+x)'
expect_rc2 -R x 1e-4 --lim x:bogus -f '(1+x)'
expect_rc2 -R x 1e-4 --lim x:p0..bogus -f '(1+x)'
expect_rc2 -R x 1e-4 --limit all -f '(1+x)'
expect_rc2 --real x 1e-4 --kernel nope -f '(1+x)'
expect_rc2 -R x 1e-4 --max-terms 0 -f '(1+x)'
"$BIN" > /dev/null 2>&1; [ $? -eq 2 ] || { printf 'FAIL 缺少 -f 应退出 2\n'; fail=$((fail+1)); }

if [ "$fail" -eq 0 ]; then
    echo "ok: 全部回归用例通过"
    exit 0
fi
echo "失败: $fail 项"
exit 1
