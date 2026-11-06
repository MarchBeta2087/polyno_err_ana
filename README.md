# polyno_err_ana

多项式/有理式误差分析器：给定表达式 `f` 与各变量的误差量，输出符号一阶误差传播、精确误差边界
（`R+` / `R-`），以及误差量在 `δv → ±0 / ±∞` 下的极限类型分类。

- 语言：C++23
- 数值内核：numextend（默认）或 MPFR（`-DPOLYNO_ENABLE_MPFR=ON`）
- 许可证：GPL-3.0-or-later（见 `LICENSE.md`；第三方组件见 `THIRD_PARTY.md`）
- **不提供商业双许可**（决定记录：`docs/history/2026-11-07-license-decision.md`）
- 贡献指南：`CONTRIBUTING.md`；常见问题（含许可边界）：`FAQ.md`；待办与路线图：`TODO.md`

需求与验收：`需求分析.md`（v0.4）；实现说明与偏差：`设计.md`；版本记录：`CHANGELOG.md`。

## 当前状态（v0.8）

| 能力 | 状态 |
| --- | --- |
| `+ - * / ^`、括号、一元负号、常数指数（含负数） | ✅ |
| `-R` 实数变量、`-a/-r`、`--estimated`、`-p` | ✅ |
| 符号 `δf`、`δf/f`（原子级约分，R-1/R-2） | ✅ |
| `δ_{R±}f`、`δ_{R±}f/f` | ✅ |
| 极限分类（`--lim all` / `VAR:p0\|n0\|pinf\|ninf`） | ✅ 实数 + 复数（`c0/cinf`） |
| 复数变量 `-C`、`i`、复数指数、`--estimated z=1+2i` | ✅ 里程碑 2 |
| `-k` 分支号（`exp(w·(log z + 2πik))`） | ✅ 里程碑 2 |
| `--real-root`（负实底数取实根，与 `-k` 互斥） | ✅ 里程碑 4 |
| `--kernel mpfr` | ✅ 需以 `-DPOLYNO_ENABLE_MPFR=ON` 构建 |
| `--lim` 分组/区间（`zeros` / `infs` / `A..B` / 逗号多规格） | ✅ |
| `--max-terms` 爆项保护（默认 200000，超限退化数值探测） | ✅ |
| `-p` 精度极值（`2` … `4096` bit） | ✅ |
| `-G` 许可全文、`--version` 内核/精度 | ✅ |

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

### MPFR 内核

```sh
cmake -S . -B build-mpfr -DCMAKE_BUILD_TYPE=Release -DPOLYNO_ENABLE_MPFR=ON -DPNA_THIRD_PARTY=/path/to/prefix
cmake --build build-mpfr -j
./build-mpfr/polyno_err_ana -V        # kernel: mpfr
```

GMP/MPFR 需自建为静态库（本项目只链接 `.a`）。两者都走官方 tarball 并按 `deps.lock`
校验 sha256（MPFR 4.2.1、GMP 6.3.0）；`mpfr/` submodule 仅作为离线备选：

```sh
scripts/fetch-gmp.sh             # gmp-6.3.0.tar.xz → sha256 → 解压
scripts/fetch-mpfr.sh            # mpfr-4.2.1.tar.xz → sha256 → 解压
scripts/build-mpfr.sh            # 两者静态库 → third_party_prefix/
```

离线时用已有压缩包：`scripts/fetch-gmp.sh --tarball …`、`scripts/fetch-mpfr.sh --tarball …`。
`build-mpfr.sh` 优先用 `mpfr-<version>/`（tarball 目录），不存在时回退到 `mpfr/` submodule。

Windows 需在 MSYS2 MINGW64 环境（`pacman -S --needed mingw-w64-x86_64-gcc make m4`）；
GCC 14+ 默认 C23 会让 GMP 的 long-long 探针失败，脚本已加 `-std=gnu17`。

两个内核共用同一套 `src/num.hpp` 接口，`ast/sym/main` 不改；回归用例对两者一致。

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

# 机器可读输出（JSON）与 schema
./build/polyno_err_ana -R x 1e-4 --estimated x=0.6 -f '(1+x)^4' --vars --json | jq .
./build/polyno_err_ana --json-schema > schema.json

# 英文消息
./build/polyno_err_ana --lang en -R x 1e-4 -f '(1+x)^4' --vars

# 负实底数：默认复主值 vs --real-root 实根
./build/polyno_err_ana -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/3)' --vars
./build/polyno_err_ana -R x 1e-4 --estimated x=0.5 -f 'x+(-8)^(1/3)' --vars --real-root
```

退出码：`0` 成功、`1` 公式解析错误（含列号）、`2` 命令行错误、`3` 数值异常。

## 目录

```text
src/num.hpp            后端无关接口（Real/Cplx/格式化）
src/num.cpp            numextend 后端
src/num_mpfr.cpp       MPFR 后端（-DPOLYNO_ENABLE_MPFR=ON 时编译）
src/num_common.cpp     共用：Cplx 运算、分支幂、pi()
src/ast.hpp|ast.cpp    词法 / 语法 / AST / 结构哈希 / 求值
src/sym.hpp|sym.cpp    符号层（原子因子 + 复指数 Laurent 多项式和、爆项保护）
src/main.cpp           命令行、输出表格、极限分类
tests/checks.sh        断言式回归（CTest: regression）
tests/cases.txt        快照用例（CTest: snapshot）
tests/snapshots/       期望输出（与内核无关）
src/json_out.hpp|cpp  JSON 输出（--json）
src/render.hpp|cpp    表格渲染与输出块
deps.lock             依赖锁定（提交号 + GMP URL/sha256；scripts/verify-deps.sh 校验）
scripts/fetch-gmp.sh  GMP 源码获取与校验（--tarball 支持离线）
scripts/fetch-mpfr.sh MPFR 源码获取与校验（--tarball / --record）
schema/json_out.schema.json  --json 的 JSON Schema（--json-schema 输出）
scripts/bench.sh       性能冒烟（--save 追加 bench/history.csv）
scripts/bench_compare.sh 对比最近 N 次基准
scripts/build-mpfr.sh  GMP/MPFR 构建脚本
docs/history/          历史决策记录（需求修改建议 v0.1）
numextend/             git submodule（数值内核）
mpfr-<version>/        MPFR 官方 tarball 解压目录（不入库）
docs/history/          决策与迁移记录
```

## 许可与使用边界

- 仅运行本工具、使用其输出：**不触发** GPL 义务（无分发、无衍生作品）。
- 把源码/子模块加入自己的工程或与之链接：视为组合作品，分发时须遵循 GPL-3.0-or-later。
- 需要闭源集成时，用独立进程调用 CLI（标准输入输出交互），不要复制源码。
- 商用允许；分发二进制时需提供对应源码与许可文本。
- 本项目只有单一 GPL 许可，**没有商业授权渠道**；详见 `FAQ.md`。

## 第三方

见 `THIRD_PARTY.md`。摘要：

| 依赖 | 用途 | 许可 |
| --- | --- | --- |
| [numextend](https://github.com/MarchBeta2087/numextend) | 默认数值内核 | MIT |
| [MPFR](https://www.mpfr.org/) 4.2.1（官方 tarball） | 可选内核（`-DPOLYNO_ENABLE_MPFR=ON`） | LGPL-3.0-or-later |
| [GMP](https://gmplib.org/) | MPFR 依赖（需自建，未入库） | LGPL-3.0-or-later / GPL-2.0+ |
