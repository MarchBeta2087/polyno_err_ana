# 示例与教程

本页所有输出均由仓库当前版本实际运行得到（`--lang zh`，默认内核 `numextend`）。
在 `mpfr` 内核下数值一致（回归与快照会校验）。

## 1. 最小例子：`(1+x)^4`

```sh
./polyno_err_ana --real --absolute x 1e-4 -f '(1+x)^4' --vars
```

```text
polyno_err_ana 0.14.7-m14  (numextend, 256 bit)

f    = (1+x)^4
x ≈ 0,  δx = 0.0001 (absolute)

δf    = 0.0004(1+x)^3
      = 0.0004
δf/f  = 0.0004/(1+x)
      = 0.0004

∂f/∂x  = 4(1+x)^3
δx     = 0.0001
δx/x   = N/A（估计点为 0）

  精确边界（单变量变，其余固定在估计点）

  var    δ_{R+}f    δ_{R-}f  δ_{R+}f/f  δ_{R-}f/f
  ---  ----------  ----------  ----------  ----------
  x    0.00040006  0.00039994  0.00040006  0.00039994

  极限分类（单变量变，其余 δ=0）

  var  δv →   δf  δf/f
  ---  -------  ----  -----
  x         +0    +0     +0
  x         -0    -0     -0
  x       +inf  +inf   +inf
  x       -inf  -inf   -inf
```

要点：

- `δf = 4(1+x)^3·δx`，把 `δx=1e-4` 折进系数后显示 `0.0004(1+x)^3`；
- `δf/f` 的分母是 **f**（不是 x），所以约分成 `0.0004/(1+x)`（需求 R-1）；
- 边界块只变动当前变量，其余固定在估计点。

## 2. 估计点：`x ≈ 0.6`

```sh
./polyno_err_ana --real --absolute x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars
```

```text
polyno_err_ana 0.14.7-m14  (numextend, 256 bit)

f    = (1+x)^4
x ≈ 0.6,  δx = 0.0001 (absolute)

δf    = 0.0004(1+x)^3
      = 0.0016384
δf/f  = 0.0004/(1+x)
      = 0.00025

∂f/∂x  = 4(1+x)^3
δx     = 0.0001
δx/x   = 0.000166667

  精确边界（单变量变，其余固定在估计点）

  var    δ_{R+}f    δ_{R-}f   δ_{R+}f/f   δ_{R-}f/f
  ---  ----------  ----------  -----------  -----------
  x    0.00163855  0.00163825  0.000250023  0.000249977

  极限分类（单变量变，其余 δ=0）

  var  δv →   δf  δf/f
  ---  -------  ----  -----
  x         +0    +0     +0
  x         -0    -0     -0
  x       +inf  +inf   +inf
  x       -inf  -inf   -inf
```

## 3. 相对误差模式

相对模式下实际误差量 `δx = |x|·ERR`，直接折进系数（与 `0.0004x(1+x)^3` 等价）：

```sh
./polyno_err_ana --real --relative x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars
```

```text
polyno_err_ana 0.14.7-m14  (numextend, 256 bit)

f    = (1+x)^4
x ≈ 0.6,  δx = 6.0000e-05 (relative)

δf    = 0.00024(1+x)^3
      = 0.00098304
δf/f  = 0.00024/(1+x)
      = 0.00015

∂f/∂x  = 4(1+x)^3
δx     = 6.0000e-05
δx/x   = 0.0001

  精确边界（单变量变，其余固定在估计点）

  var     δ_{R+}f     δ_{R-}f   δ_{R+}f/f   δ_{R-}f/f
  ---  -----------  -----------  -----------  -----------
  x    0.000983095  0.000982985  0.000150008  0.000149992

  极限分类（单变量变，其余 δ=0）

  var  δv →   δf  δf/f
  ---  -------  ----  -----
  x         +0    +0     +0
  x         -0    -0     -0
  x       +inf  +inf   +inf
  x       -inf  -inf   -inf
```

## 4. 复数与分支号

`-C` 声明复数变量的实/虚部误差，`δz = RE_ERR + i·IM_ERR`；分类只做模（`c0`/`cinf`）。
`-k 2` 让幂走分支 `exp(w·(log z + 2πik))`：

```sh
./polyno_err_ana -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+z)^(1+i)' -k 2 --vars
```

```text
polyno_err_ana 0.14.7-m14  (numextend, 256 bit)
branch k = 2

f    = (1+z)^(1+i)
z ≈ (1+0.5i),  δz = (0.0001+0.0001i) (absolute)

δf    = (0+0.0002i)(1+z)^(0+1i)
      = (-3.6139e-10+4.0918e-10i)
δf/f  = (0+0.0002i)/(1+z)
      = (2.3529e-05+9.4118e-05i)

∂f/∂z  = (1+1i)(1+z)^(0+1i)
δz     = (0.0001+0.0001i)
δz/z   = (0.00012+4.0000e-05i)

  精确边界（单变量变，其余固定在估计点）

  var    δ_{R+}f    δ_{R-}f  δ_{R+}f/f  δ_{R-}f/f
  ---  ----------  ----------  ----------  ----------
  z    5.4591e-10  5.4593e-10  9.7013e-05  9.7016e-05

  极限分类（单变量变，其余 δ=0）

  var  δv →   δf  δf/f
  ---  -------  ----  -----
  z          0    c0     c0
  z        inf  cinf   cinf
```

注意 `δf/f = (1+i)δz/(1+z)`：符号层把 `(1+z)^(1+i)` 当作原子幂，指数 `-(1+i)` 与 `i` 相加后约分。

## 5. 高阶消失：`(1+x)^2` 在 `x = -1`

线性化 `δf = 0`，但分类必须用精确差 `g(δx) = δx²`（`k=2`）：

```sh
./polyno_err_ana --real --absolute x 1e-4 --estimated x=-1 -f '(1+x)^2' --lim all
```

```text
polyno_err_ana 0.14.7-m14  (numextend, 256 bit)

f    = (1+x)^2
x ≈ -1,  δx = 0.0001 (absolute)

  精确边界（单变量变，其余固定在估计点）

  var    δ_{R+}f    δ_{R-}f  δ_{R+}f/f  δ_{R-}f/f
  ---  ----------  ----------  ----------  ----------
  x    1.0000e-08  1.0000e-08         N/A         N/A

  极限分类（单变量变，其余 δ=0）

  var  δv →   δf  δf/f
  ---  -------  ----  -----
  x         +0    +0    fin
  x         -0    +0    fin
  x       +inf  +inf    fin
  x       -inf  +inf    fin
```

换成 `(1+x)^3`（奇次幂）四组结果各不相同；`(1+x)^(1/2)` 则因 `k=1/2` 在 `δx→-0` 给出 `undef`：

```sh
./polyno_err_ana --real --absolute x 1e-4 --estimated x=-1 -f '(1+x)^(1/2)' --lim all
```

```text
polyno_err_ana 0.14.7-m14  (numextend, 256 bit)

f    = (1+x)^(1/2)
x ≈ -1,  δx = 0.0001 (absolute)

  精确边界（单变量变，其余固定在估计点）

  var  δ_{R+}f  δ_{R-}f  δ_{R+}f/f  δ_{R-}f/f
  ---  --------  --------  ----------  ----------
  x        0.01      0.01         N/A         N/A

  极限分类（单变量变，其余 δ=0）

  var  δv →    δf  δf/f
  ---  -------  -----  -----
  x         +0     +0    fin
  x         -0  undef    fin
  x       +inf   +inf    fin
  x       -inf  undef    fin
```

## 6. JSON 输出与 schema

```sh
./polyno_err_ana -R x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars --json | jq .
```

```json
  {
    "version": "0.14.7-m14",
    "kernel": "numextend",
    "precision": 256,
    "branch": 0,
    "formula": "(1+x)^4",
    "mode": "absolute",
    "vars": [
      {"name": "x", "estimate": "0.6", "delta": "0.0001", "mode": "absolute"}
    ],
    "symbolic": {
      "available": true,
      "df": "0.0004(1+x)^3",
      "df_value": "0.0016384",
      "df_f": "0.0004/(1+x)",
      "df_f_value": "0.00025",
      "partials": [
        {"var": "x", "expr": "4(1+x)^3", "delta": "0.0001"}
      ]
    },
    "boundary": [
      {"var": "x", "r_plus": "0.00163855", "r_minus": "0.00163825", "r_plus_rel": "0.000250023", "r_minus_rel": "0.000249977"}
    ],
    "limits": [
      {"var": "x", "state": "+0", "df": "+0", "df_f": "+0"},
      {"var": "x", "state": "-0", "df": "-0", "df_f": "-0"},
      {"var": "x", "state": "+inf", "df": "+inf", "df_f": "+inf"},
      {"var": "x", "state": "-inf", "df": "-inf", "df_f": "-inf"}
    ]
  }
```

schema 随二进制发布（`--json-schema`），可用任意 draft 2020-12 校验器检查：

```sh
./polyno_err_ana --json-schema > schema.json
# 例：python 端 pip install jsonschema 后
# jsonschema -i out.json schema.json
```

`symbolic.available=false` 时（如 `--max-terms` 触发退化），符号字段为空串、`partials` 为空数组，
`boundary`/`limits` 仍然有效。

## 7. 负实底数与实根

默认走复主值（`arg(-8) = +π`）；`--real-root` 按有理指数取实根（与 `-k` 互斥）：

```sh
./polyno_err_ana -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/3)' --vars
./polyno_err_ana -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/3)' --vars --real-root
```

```text
δf    = 0.0001
      = 0.0001
δf/f  = 0.0001/(x+(1+1.73205i))
      = (2.8571e-05-3.2991e-05i)
δf    = 0.0001
      = 0.0001
δf/f  = 0.0001/(x+(-2))
      = -6.6667e-05
```

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
