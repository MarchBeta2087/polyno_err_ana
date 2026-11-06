# polyno_err_ana

多项式/有理式误差分析器：给定表达式 `f` 与各变量的误差量，输出符号一阶误差传播、精确误差边界
（`R+` / `R-`），以及误差量在 `δv → ±0 / ±∞` 下的极限类型分类。

- 语言：C++23
- 数值内核：numextend（bigfloat）；MPFR 内核待接入
- 许可证：GPL-3.0-or-later（见 `LICENSE.md`）

需求与验收：`需求分析.md`（v0.2）；实现说明与偏差：`设计.md`。

## 当前状态（里程碑 1）

| 能力 | 状态 |
| --- | --- |
| `+ - * / ^`、括号、一元负号、常数指数（含负数） | ✅ |
| `-R` 实数变量、`-a/-r`、`--estimated`、`-p` | ✅ |
| 符号 `δf`、`δf/f`（原子级约分，R-1/R-2） | ✅ |
| `δ_{R±}f`、`δ_{R±}f/f` | ✅ |
| 极限分类（`--lim all` / `VAR:p0|n0|pinf|ninf`） | ✅ 实数 + 复数（`c0/cinf`） |
| 复数变量 `-C`、`i`、复数指数、`--estimated z=1+2i` | ✅ 里程碑 2 |
| `-k` 分支号（`exp(w·(log z + 2πik))`） | ✅ 里程碑 2 |
| `--kernel mpfr` | ❌ 需先构建 GMP |

## 构建

依赖 `numextend` 子模块：

```sh
git clone --recurse-submodules git@github.com:MarchBeta2087/polyno_err_ana.git
cd polyno_err_ana

# 1) 先构建数值内核
cmake -S numextend -B numextend/build -DCMAKE_BUILD_TYPE=Release
cmake --build numextend/build -j

# 2) 再构建本项目
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# 3) 回归
ctest --test-dir build --output-on-failure
```

若 `numextend/build/libnex.a` 不存在，根 `CMakeLists.txt` 会退化为 `add_subdirectory(numextend)`
（同时会构建 numextend 自带的测试，耗时更长）。

## 用法

```sh
# T-1：符号一阶形式 + 分类
./build/polyno_err_ana --real --absolute x 1e-4 -f '(1+x)^4' --vars --lim all

# T-3：估计点代入
./build/polyno_err_ana --real --absolute x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars

# 多变量：默认每变量 +0 一组，--lim all 得 4n 组
./build/polyno_err_ana --real --absolute x 1e-4 y 1e-4 -f '(1+x*y)^5' --lim all

# 高精度
./build/polyno_err_ana --real --absolute x 1e-4 -f '(1+x)^4' -p 512

# 复数 + 分支号（T-6）
./build/polyno_err_ana -C z 1e-4 1e-4 --estimated z=1+0.5i -f '(1+z)^(1+i)' -k 2 --vars
```

退出码：`0` 成功、`1` 公式解析错误（含列号）、`2` 命令行错误、`3` 数值异常。

## 目录

```text
src/num.hpp|num.cpp    数值层（numextend bigfloat 的 RAII 包装）
src/ast.hpp|ast.cpp    词法 / 语法 / AST / 求值
src/sym.hpp|sym.cpp    符号层（原子因子 + 整数指数的 Laurent 多项式和）
src/main.cpp           命令行与输出
tests/checks.sh        断言式回归（CTest: regression）
mpfr/, numextend/      git submodule（上游依赖）
```

## 第三方

| 依赖 | 用途 | 许可 |
| --- | --- | --- |
| [numextend](https://github.com/MarchBeta2087/numextend) | 当前数值内核 | 见子模块 `LICENSE` |
| [MPFR](https://www.mpfr.org/)（`aixoss/mpfr` 镜像） | 计划中的内核 | LGPL-3.0-or-later |
| [GMP](https://gmplib.org/) | MPFR 依赖，构建时获取（未入库） | LGPL-3.0-or-later / GPL-2.0+ |
