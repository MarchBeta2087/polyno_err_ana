# TODO / 路线图

唯一待办来源。规则：**只放未完成项**；完成后从本文件删除，并在 `CHANGELOG.md` 记一笔。
每项标注关联文件与验收方式。

需要社区帮助的任务（母语审校、跨平台打包、大型特性）见 **`OPEN_TASKS.md`**，
平台等级（Tier 1–4）与升级规则见 **`PLATFORMS.md`**。

## v1.0 检查单

- [x] 需求 `T-1 … T-17` 全部有对应回归/快照（`tests/checks.sh` + `tests/cases.txt`）
- [x] 双内核（numextend / mpfr）回归一致；发布包含 `-mpfr` 变体
- [x] 依赖固定并可校验（`deps.lock` + `fetch-*.sh` 的 sha256）
- [x] JSON 输出 + schema（`--json-schema`，CI 校验两类输出）
- [x] 平台分级文档（Tier 1 三平台 CI；Tier 2 Intel mac 异步）
- [x] 性能门禁、安全策略、CoC、Issue/PR 模板、Pages 文档站
- [ ] **兼容性承诺**：写清 CLI/JSON 的语义化版本与废弃流程（新增 `COMPATIBILITY.md`）
- [ ] **一次无已知 P0/P1 的候选发布**：连续两个 tag 无回归后升 `v1.0.0`

> 不在 v1.0 门槛内：i18n 母语审校（`OPEN_TASKS.md` 的 #9–#12，社区进度不影响发布）、
> Tier 3 实测（`PLATFORMS.md` 已声明为“社区验证”，不阻塞）。

## P1 — 工程


## P2 — 发布/社区


## P3 — 功能

- 函数节点 `sin/exp/log`、变量指数 `a^b`、多变量联合极限
  → 已移交 `OPEN_TASKS.md`（#15 / #16 / #17），本文件不再跟踪

## 已决策（不再是待办）

- 商业双许可：**不做**。维持单许可 GPL-3.0-or-later，见
  `docs/history/2026-11-07-license-decision.md`
- 需求文档位置：`需求分析.md` 保留在仓库根目录作为**现行规格**（不是归档件）；
  历史草案/建议在 `docs/history/`

## 已完成（按里程碑，详见 CHANGELOG）

- m14.8：`EXAMPLES.md`（示例/教程页，含实际输出与 JSON）、`--kernel` 澄清为设计选择（非偏差）
- m14.7：JSON schema 细化（`if/then` + `examples` + 字段说明，校验器支持 `const/maxLength/maxItems`）；
  `scripts/bench_gate.sh` 接入 CI（默认 1200 ms/次上限，`BENCH_CEIL_MS` 可调，`--warn-only` 可选）

- m1 实数符号误差 + 边界 + 分类；m2 复数与 `-k`；m3 双内核与快照基线
- m4 结构哈希、`--real-root`、`Undefined` 三态；m5 哈希分桶、CLI 错误路径
- m6 爆项保护、CI 缓存、Windows MPFR 作业；m7 对齐表格、多规格 `--lim`
- m8 `--lim` 分组/区间、精度极值、macOS CI；m9 P0 闭合（探测路径分类、混合量纲）
- m10 `render` 模块、`sym_mul` 增量合并；m11 Release 工作流与基准历史
- m12 `--json`、`deps.lock`；m13 `--json-schema`、`--lang` 六语言、GMP/MPFR 官方 tarball
- m14 文档站、CoC/SECURITY、Issue/PR 模板、仓库公开与 Pages
