#!/usr/bin/env bash
# 重新生成 EXAMPLES.md：所有代码块里的输出都由当前二进制实跑得到。
# 用法: scripts/gen-examples.sh [BIN] [OUT]
set -euo pipefail
BIN="${1:-./build/polyno_err_ana.exe}"
OUT="${2:-EXAMPLES.md}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

block() {  # 命令（原样写进 sh 代码块） + 实际执行的参数
    local cmd="$1"; shift
    printf '```sh\n%s\n```\n\n' "$cmd"
    printf '```text\n'
    "$BIN" "$@"
    printf '```\n\n'
}

{
cat <<'HDR'
# 示例与教程

本页所有输出均由仓库当前版本实际运行得到（`--lang zh`，默认内核 `numextend`）。
在 `mpfr` 内核下数值一致（回归与快照会校验）。可用 `scripts/gen-examples.sh` 重新生成。

HDR

echo '## 1. 最小例子：`(1+x)^4`'
echo
block "./polyno_err_ana --real --absolute x 1e-4 -f '(1+x)^4' --vars" \
      --real --absolute x 1e-4 -f '(1+x)^4' --vars
cat <<'TXT'
要点：

- `δf = 4(1+x)^3·δx`，把 `δx=1e-4` 折进系数后显示 `0.0004(1+x)^3`；
- `δf/f` 的分母是 **f**（不是 x），所以约分成 `0.0004/(1+x)`（需求 R-1）；
- 边界块只变动当前变量，其余固定在估计点。

TXT

echo '## 2. 估计点：`x ≈ 0.6`'
echo
block "./polyno_err_ana --real --absolute x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars" \
      --real --absolute x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars

echo '## 3. 相对误差模式'
echo
echo '相对模式下实际误差量 `δx = |x|·ERR`，直接折进系数（与 `0.0004x(1+x)^3` 等价）：'
echo
block "./polyno_err_ana --real --relative x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars" \
      --real --relative x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars

echo '## 4. 复数与分支号'
echo
echo '`-C` 声明复数变量的实/虚部误差，`δz = RE_ERR + i·IM_ERR`；分类只做模（`c0`/`cinf`）。'
echo '`-k 2` 让幂走分支 `exp(w·(log z + 2πik))`：'
echo
block "./polyno_err_ana -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+z)^(1+i)' -k 2 --vars" \
      -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+z)^(1+i)' -k 2 --vars
cat <<'TXT'
注意 `δf/f = (1+i)δz/(1+z)`：符号层把 `(1+z)^(1+i)` 当作原子幂，指数 `-(1+i)` 与 `i` 相加后约分。

TXT

echo '## 5. 高阶消失：`(1+x)^2` 在 `x = -1`'
echo
echo '线性化 `δf = 0`，但分类必须用精确差 `g(δx) = δx²`（`k=2`）：'
echo
block "./polyno_err_ana --real --absolute x 1e-4 --estimated x=-1 -f '(1+x)^2' --lim all" \
      --real --absolute x 1e-4 --estimated x=-1 -f '(1+x)^2' --lim all
echo '换成 `(1+x)^3`（奇次幂）四组结果各不相同；`(1+x)^(1/2)` 则因 `k=1/2` 在 `δx→-0` 给出 `undef`：'
echo
block "./polyno_err_ana --real --absolute x 1e-4 --estimated x=-1 -f '(1+x)^(1/2)' --lim all" \
      --real --absolute x 1e-4 --estimated x=-1 -f '(1+x)^(1/2)' --lim all

echo '## 6. JSON 输出与 schema'
echo
echo '```sh'
echo "./polyno_err_ana -R x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars --json | jq ."
echo '```'
echo
printf '```json\n'
"$BIN" -R x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars --json
printf '```\n\n'
cat <<'TXT'
schema 随二进制发布（`--json-schema`），可用任意 draft 2020-12 校验器检查：

```sh
./polyno_err_ana --json-schema > schema.json
# 例：python 端 pip install jsonschema 后
# jsonschema -i out.json schema.json
```

`symbolic.available=false` 时（如 `--max-terms` 触发退化），符号字段为空串、`partials` 为空数组，
`boundary`/`limits` 仍然有效。

TXT

echo '## 7. 负实底数与实根'
echo
echo '默认走复主值（`arg(-8) = +π`）；`--real-root` 按有理指数取实根（与 `-k` 互斥）：'
echo
printf '```sh\n'
printf "./polyno_err_ana -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/3)' --vars\n"
printf "./polyno_err_ana -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/3)' --vars --real-root\n"
printf '```\n\n```text\n'
"$BIN" -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/3)' --vars | sed -n '6,9p'
"$BIN" -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/3)' --vars --real-root | sed -n '6,9p'
printf '```\n\n'

cat <<'TXT'
## 8. 常用开关速查

| 目的 | 命令片段 |
| --- | --- |
| 逐变量 4 状态 | `--lim all` |
| 只看向 ±∞ | `--lim x:infs` |
| 区间（含两端） | `--lim x:0..inf`（等价全部 4 状态）、`--lim x:p0..pinf`（3 组） |
| 多变量多规格 | `--lim x:p0,y:ninf` 或重复 `--lim` |
| 低/高精度 | `-p 2` … `-p 4096`（`-R`/`--estimated` 字面量同样按该精度解析） |
| 机器可读 | `--json`；schema 用 `--json-schema` |
| 消息语言 | `--lang ja|ko|es|pt`（默认 zh） |
| 爆项保护 | `--max-terms N`（超限退化为数值探测，退出码仍为 0） |

退出码：`0` 成功、`1` 解析错误（含列号）、`2` 命令行错误、`3` 数值异常。
TXT
} > "$OUT"

echo "已生成 $OUT"
