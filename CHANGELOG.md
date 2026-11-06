# CHANGELOG

版本按里程碑打 tag：`vX.Y.0-mN`。每条只记用户可见的变化；设计细节见 `设计.md`，待办见 `TODO.md`。

## v0.14.7-m14

- JSON Schema 细化：`symbolic` 增加字段说明与两组 `examples`（可用 / 不可用），
  并用 `allOf + if/then` 约束 `available=false` 时 `df*` 为空串、`partials` 为空数组；
  测试校验器扩展支持 `const / maxLength / maxItems / allOf / if-then`，并同时校验两类输出
- 性能门禁：新增 `scripts/bench_gate.sh`（默认 1200 ms/次上限，`BENCH_CEIL_MS` 可调、`--warn-only`），
  CI linux 作业接入；`bench.sh` 支持 `BENCH_CSV` 指定 CSV 路径（CI 用临时文件，不污染仓库）

- 修复 Release 竞态：`publish` 改为 `needs: [linux, windows, macos]`（私有仓库 macos 被 skip 时不影响发布，
  公开仓库会等 mac 包就绪），避免 macOS 产物偶尔漏传

## v0.14.6-m14

- 修正“GitHub 私信”说法（GitHub 没有 DM）：`SECURITY.md`/`CODE_OF_CONDUCT.md`/`FAQ.md`
  统一改为 **Security Advisory** 作为唯一私有渠道，并说明其也可用于行为举报
- `CONTRIBUTING.md` 增加隐私提示：提交使用 noreply 邮箱
- 之后提交改用 `98508270+MarchBeta2087@users.noreply.github.com`（历史中的旧邮箱保留）

## v0.14.5-m14

- `OPEN_TASKS.md` 增加“多变量联合极限”（#17）：说明难点在语义（路径/速率）而非编码，
  并要求先改需求 §3.6 再实现；`TODO.md` 的 P3 改为只指向开放任务

## v0.14.4-m14

- 新增 `OPEN_TASKS.md`（开放任务清单）+ 8 个 GitHub Issue（4 个 i18n 母语审校、2 个打包、2 个大型特性）
  并打上 `help wanted` / `good first issue` 等标签
- `TODO.md` 去掉社区类任务，改为指向 `OPEN_TASKS.md`；`CONTRIBUTING.md` 增加“从哪里开始”
- 文档站增补 `OPEN_TASKS.md` 页面

## v0.14.3-m14

- 文档刷新：`README.md` 状态升到 v0.14（补 JSON/Schema/多语言/静态发布）、新增「下载」段与目录整理
- `设计.md` 升到「v0.14 实现现状」：行数/文件表刷新、§1 补 JSON/多语言、§4.5 补 schema 与 i18n、
  §6 已知偏差重写（6 条）
- `TODO.md` 重写：P1/P2/P3（母语审校、schema 细节、性能门禁、macOS x86_64、函数节点…），
  去掉重复与已完成条目
- `需求分析.md` 升到 v0.5（现行规格）：N-1 上限 2500→3500、JSON 示例版本更新、新增 T-16/T-17、
  明确“不归档、留在根目录作为现行规格”

## v0.14.2-m14

- 仓库转为公开；启用 Pages（workflow 模式）、私有漏洞报告、Dependabot 告警与自动安全更新、
  `main` 分支保护（必需检查：linux / windows / macos）
- Actions 版本统一升级：checkout v7、cache v6、upload-artifact v7、download-artifact v8、
  configure-pages v6、upload-pages-artifact v5、deploy-pages v5
- 脚本补可执行位；Pages 工作流改用 `bash scripts/build-docs.sh`

## v0.14.1-m14

- 文档站：`scripts/build-docs.sh`（pandoc → `site/_out/`）+ `.github/workflows/pages.yml`
  （构建 artifact；Pages 可用时自动部署）
- 社区文件：`CODE_OF_CONDUCT.md`（Contributor Covenant v2.1 摘要）、`SECURITY.md`（私有报告渠道与处理时限）
- GitHub 模板：`.github/ISSUE_TEMPLATE/{bug_report,feature_request,config}.yml`、`PULL_REQUEST_TEMPLATE.md`
- 注：私有仓库免费计划不支持 Pages，部署作业由仓库变量 `ENABLE_PAGES=true` 控制；转公开后开启即可
- 开放前准备：`.github/dependabot.yml`（Actions 每周）、`.github/CODEOWNERS`、
  `release.yml` 增加 macOS 作业（仅公开仓库执行）、检查清单
  `docs/history/2026-11-07-open-source-checklist.md`

## v0.14.0-m14

- `--lang` 扩展到 **6 种语言**：`zh`（默认）/ `en` / `ja` / `ko` / `es` / `pt`
- i18n 改为单表 `{zh,en,ja,ko,es,pt}`：错误消息、帮助（6 份全文）、输出块标题、N-5 提示
- 中文模式下的解析/求值/数值错误也改为中文（此前恒为英文）
- 快照扩到 45 条（新增 ja/ko/es/pt 输出与 es 的 JSON）；checks 增加 4 种语言的帮助/错误/标题断言

## v0.13.4-m13

- 移除 `mpfr/` submodule：MPFR 只走官方 tarball 4.2.1（`scripts/fetch-mpfr.sh`）
- `build-mpfr.sh` 支持 `--dry-run`、参数解析更严格（未知参数退出 2）
- `verify-deps.sh` 不再检查 MPFR 提交；迁移说明见
  `docs/history/2026-11-07-mpfr-tarball-migration.md`
- README/THIRD_PARTY 更新依赖获取方式与目录结构

## v0.13.3-m13

- **许可政策定为单许可**：不提供商业双许可，只保留 GPL-3.0-or-later；
  更新 `FAQ.md`（新增“有无商业授权”“以后会不会换许可”）、`README.md`、`CONTRIBUTING.md`（入站=出站，无 CLA）
- 决策记录：`docs/history/2026-11-07-license-decision.md`

## v0.13.2-m13

- MPFR 改用官方 tarball：新增 `scripts/fetch-mpfr.sh`（下载 → sha256 → 解压，支持 `--tarball` 离线、
  `--record` 首次落库），`deps.lock` 记录 4.2.1 的 URL 与 sha256
- `build-mpfr.sh` 优先使用 `mpfr-<version>/`，无则回退 `mpfr/` submodule；`verify-deps.sh` 增加 MPFR 目录校验
- 已用官方 4.2.1 重建并跑通双内核回归（`ctest` 2/2）

## v0.13.1-m13

- GMP 改走官方 tarball：新增 `scripts/fetch-gmp.sh`（下载 → sha256 校验 → 解压，`--tarball` 支持离线），
  `deps.lock` 补 `gmp_url` 与 `gmp_sha256`
- `scripts/verify-deps.sh --tarball FILE` 校验本地压缩包；CI 的 linux 作业增加依赖校验步骤
- README/THIRD_PARTY 记录 GMP 来源与校验和

## v0.13.0-m13

- 新增 `--json-schema`：输出 `schema/json_out.schema.json`（draft 2020-12），构建时嵌入二进制
- 新增 `--lang zh|en`：错误信息、`--help`、输出块标题中英切换；符号标签不翻译
- `checks.sh` 增加 schema 校验（不依赖 jsonschema 包）与英文消息断言；快照扩到 40 条（含 2 条英文输出）

## v0.12.0-m12

- 新增 `--json`：一次输出 meta/`symbolic`/`boundary`/`limits`，数值保持字符串格式（字段见需求 §3.9）
- 输出块抽到 `render`（表头/符号块/边界/分类），`main.cpp` 只组装数据、再选择表格或 JSON 输出
- 新增 `deps.lock` + `scripts/verify-deps.sh`：校验 numextend/MPFR 提交与 GMP 版本
  （本机访问 mpfr.org/gmplib.org 被 TLS 拦截，GMP tarball 的 sha256 待联网补录）
- 快照扩到 38 条（含 3 条 JSON、版本号归一化）；checks.sh 增加 JSON 结构断言

## v0.11.3-m11

- Release 的 `publish` 不再等待 macOS 作业（`needs: [linux, windows]`）
- 移除 Release 的 macOS 作业：该账号的 macOS runner 屡次排队 10+ 分钟不启动；
  macOS 可编译性仍由 `ci.yml` 的 macos 作业保证，预编译包暂不提供

## v0.11.2-m11

- Release 的 macOS 作业改用 `macos-13`（Intel）并设为 `continue-on-error`：
  `macos-latest` 曾两次排队 15 分钟后被取消；macOS 产物缺失时仍照常发布 linux/windows

## v0.11.1-m11

- 修复 macOS 静态链接失败：`-static` 仅用于 Linux/MinGW（macOS 无静态 libSystem）

## v0.11.0-m11

- `-DPOLYNO_STATIC_RUNTIME=ON`：静态链接运行库，产出可直接分发的单文件
- 新增 `.github/workflows/release.yml`：tag 触发，构建 linux/windows/macos 三平台产物、
  生成 `SHA256SUMS.txt` 并发布 GitHub Release
- `scripts/bench.sh --save` 追加 `bench/history.csv`；`scripts/bench_compare.sh` 对比最近 N 次
- README 增加“下载”一节

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
