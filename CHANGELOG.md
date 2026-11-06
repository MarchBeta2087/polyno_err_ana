# CHANGELOG

版本按里程碑打 tag：`vX.Y.0-mN`。每条只记用户可见的变化；设计细节见 `设计.md`，待办见 `TODO.md`。

## v0.10.0-m10

- 输出渲染抽到 `src/render.{hpp,cpp}`（`render_table`），`main.cpp` 只组装行数据
- `sym_mul` 改为逐对相乘即时合并，项数上限按**结果项数**判定（大量可约分场景不再误报）；
  求导/相乘阶段触发上限也会正确退化为数值探测（退出码 0）
- `from_double` 走 `%.17g` + ctx 解析，低精度（`-p 8`）下 numextend 与 MPFR 结果一致
- 快照扩到 35 条（`-p 8/64`、`--lim x:n0..ninf`、三变量、求导阶段爆项）

## v0.9.0-m9

- 数值探测路径也给出相对形式分类：`k_g` 来自 `g`、`k_f` 来自 `f`（`probe_magnitude`），
  `K = k_g − k_f` 后套同一套幂律规则（此前一律 `Undefined`）
- 复值系数（`f` 含复变量、偏导为复数）一律按模分类（`c0/cinf`），不再输出错误的 `±0/±inf`
- 复数相对模式语义固化：`δz = |z|·(RE_ERR + i·IM_ERR)`，写入需求 §3.1 并加快照
- 新增 3 条快照（复数相对模式、探测路径分类、混合量纲）与对应断言

## v0.8.0-m8

- `--lim` 支持分组与区间：`zeros` / `infs` / `all`、`A..B`（`0..inf` 等价全部 4 状态）、
  逗号分隔或重复出现；复数变量别名 `0` / `inf`；非法状态/区间端点 → 退出码 2
- `-p` 在参数解析前生效，`-R` / `--estimated` 字面量也按目标精度解析
  （`p=2` 时 `δx = 9.1553e-05`、`δf = 0.000366211`）
- 新增 T-14（`-p 2/4096`）、T-15（`--lim` 区间）用例与快照
- CI 增加 macOS 作业（numextend + MPFR 双内核，brew 提供 gmp/mpfr）
- 源文件加 `SPDX-License-Identifier: GPL-3.0-or-later`；新增 `CONTRIBUTING.md`、`THIRD_PARTY.md`、`FAQ.md`
- 待办收敛到 `TODO.md`，`设计.md` 只保留设计约束

## v0.7.0-m7

- 精确边界与极限分类改为对齐表格：`render_table` 按内容算列宽、右对齐、自动分隔行；
  新增 `var` 列；复数状态标签用 `0` / `inf`
- `--lim` 支持多规格（逗号/多次），逐变量校验
- `checks.sh` 改为布局无关的 awk 断言；快照重新生成

## v0.6.0-m6

- 爆项保护：`--max-terms`（默认 200000），`sym_mul` 交叉相乘前预判、超限抛 `EvalError`
  并退化为数值探测（仍输出边界与分类）
- 提示统一为“非整数指数或项数超限”
- CI：numextend 构建缓存；Windows 用 MSYS2 UCRT64 的 gmp/mpfr 包跑 MPFR 作业

## v0.5.0-m5

- 符号层：`Node` 结构哈希（FNV-1a），`cleanup()` 改因子哈希分桶合并
  （bench：`(1+x)^200` 860→602 ms）
- `--lim VAR:STATE` 在 `n == 1` 时优先于默认 4 组
- `-p` 非法值、`--lim` 未声明变量 → 退出码 2；负数右操作数加括号（`x+(-2)`）
- 边界/数值格统一走 `c_fmt`（NaN→`Undefined`，纯 ±Inf 原样）
- 快照用例扩到 14 条；新增 `CHANGELOG.md`

## v0.4.0-m4

- `--real-root`：负实底数 + 有理指数（连续分数 `p/q`）取实根；偶分母/非有理 → `Undefined`；
  与 `-k` 互斥
- `-0` 虚部归一化，修复 numextend / MPFR 在负实底数主值角度上的分叉
- N-3：NaN 统一显示 `Undefined`；新增 T-10…T-13
- `scripts/bench.sh` 性能冒烟

## v0.3.0-m3

- 双内核：`num.hpp` 接口 + `num_common.cpp`（`Cplx`/分支幂）+ `num.cpp`（numextend）/ `num_mpfr.cpp`（MPFR）
- `-DPOLYNO_ENABLE_MPFR=ON` 链接 GMP/MPFR 静态库；`-G` 输出 LICENSE 全文
- 快照基线 `tests/snapshots/expected.txt`（与内核无关）；`scripts/build-mpfr.sh`
- CI：Linux 双内核 + Windows（MSYS2 UCRT64）

## v0.2.0-m2

- 复数：`Cplx` 运算、`i` / `2i` 字面量、`-C`、`--estimated z=1+2i`
- `-k` 分支号生效：`z^w = exp(w·(log z + 2πik))`
- 符号层支持复常数指数（`(1+z)^(1+i)`），分类对复数用模（`c0`/`cinf`）

## v0.1.0-m1

- 实数符号误差：`δf`、`δf/f`（原子级约分，满足 R-1/R-2）
- 精确边界 `δ_{R±}f`；极限分类（Taylor 首项 + 数值探测兜底）
- 解析器（优先级、右结合、常数指数校验、列号报错）、退出码 0/1/2/3
