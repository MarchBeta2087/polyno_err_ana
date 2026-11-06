# CHANGELOG

## v0.4.0-m5 — 未发布

**符号层**
- `Node` 增加 64 位 FNV-1a 结构哈希，原子归并只比哈希与结构，不再拼字符串（bench：`(1+x)^200` 860→602 ms，`(1+x)^50*(1+y)^50` 875→714 ms）
- `cleanup()` 换因子哈希分桶合并（平均 O(n)）
- 负实底数的 `-0` 虚部归一化，修复 numextend 与 MPFR 在复主值角度上的分叉

**功能**
- `--real-root`：负实底数 + 有理指数（连续分数 `p/q`，`q ≤ 10⁶`）取实根；偶分母/非有理 → `Undefined`；与 `-k` 互斥
- `-p` 非法值（非整数或 < 2）、`--lim` 引用未声明变量 → 退出码 2
- `--lim VAR:STATE` 在 `n == 1` 时也优先于默认 4 组

**输出（N-3）**
- `NaN` 统一显示 `Undefined`，纯 `±Inf` 保留符号；边界块不再出现 `N/A`/`NaN` 混用
- 负数作为右操作数时加括号（`x+(-2)` 而非 `x+-2`）

**资源保护**
- `sym_mul`/`sym_add` 增加项数上限 `--max-terms`（默认 200000），超限抛 `EvalError` 并退化为数值探测，不再长时间卡死
- 符号展开不可用的提示统一为“非整数指数或项数超限”

**CI**
- numextend 构建缓存（按子模块源码哈希）
- Windows 启用 MPFR 作业（MSYS2 UCRT64 的 `gmp`/`mpfr` 包）

**CLI / 输出**
- `--lim` 增加分组与区间：`zeros`/`infs`/`all`、`A..B`（`0..inf` 等价全部 4 状态）、复数别名 `0`/`inf`
- `-p` 现在在解析参数前生效，`-R`/`--estimated` 的字面量也按目标精度解析（`p=2` 时 `δx = 9.1553e-05`）

- `--lim` 支持多规格：`--lim x:p0,x:ninf`、多次 `--lim`、`--lim all`（互斥）
- 精确边界与极限分类改为对齐表格（`var` 列 + `---` 分隔行），不再手工调宽
- 复变量的状态标签用 `0` / `inf`（不再借用 `+0/-0/+inf/-inf`）

**开源准备**
- 所有源文件与 `CMakeLists.txt` 加 `SPDX-License-Identifier: GPL-3.0-or-later`
- 新增 `CONTRIBUTING.md`、`THIRD_PARTY.md`（numextend MIT / MPFR·GMP LGPL 与静态链接说明）
- 新增 `FAQ.md`（11 条：GPL「传染性」边界、组合作品、结果归属、商用、静态链接 LGPL、功能取舍等），README 增「许可与使用边界」

**测试**
- 快照用例扩到 14 条（`tests/cases.txt`），覆盖单状态 `--lim`、相对模式估计点为 0、`-p 64`、`-R`/`-C` 混合、`0^0`、除零、real-root
- `tests/checks.sh` 增加 CLI 错误路径断言（8 条），CTest 为 `regression` + `snapshot`
- 两个内核（numextend / MPFR）输出逐字一致

## v0.3.0-m3

- 双后端：`num.hpp` 接口 + `num_common.cpp`（`Cplx`、分支幂）+ `num.cpp` / `num_mpfr.cpp`
- `-DPOLYNO_ENABLE_MPFR=ON` 链接 GMP/MPFR；`-G` 输出 LICENSE 全文
- 快照基线（`tests/snapshots/expected.txt`）、`scripts/build-mpfr.sh`
- CI：Linux 双内核 + Windows（MSYS2 UCRT64）

## v0.2.0-m2

- 复数：`Cplx` 运算、`i`/`2i` 字面量、`-C`、`--estimated z=1+2i`
- `-k` 分支号真实生效（`exp(w·(log z + 2πik))`）
- 符号层支持复常数指数（`(1+z)^(1+i)`）

## v0.1.0-m1

- 实数符号误差：`δf`、`δf/f`（原子级约分，R-1/R-2）
- 精确边界 `δ_{R±}f`、极限分类（Taylor 首项 + 数值探测兜底）
- 解析器（优先级、右结合、常数指数校验、列号报错）、退出码 0/1/2/3
