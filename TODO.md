# TODO / 路线图

唯一待办来源。规则：**只放未完成项**；完成后从本文件删除，并在 `CHANGELOG.md` 记一笔。
每项标注关联文件与验收方式。

需要社区帮助的任务（母语审校、跨平台打包、大型特性）见 **`OPEN_TASKS.md`**，本文件只维护维护者自己的工程项。

## P1 — 工程

- [ ] **JSON schema 细节**：`--json-schema` 已能输出，但尚未覆盖 `--vars` 关闭时的字段差异说明
      验收：schema 中 `symbolic.available=false` 的分支有 `description` 或 `examples`
- [ ] **性能回归门禁**：`scripts/bench.sh --save` 已记录历史，但 CI 未比较
      验收：`bench_compare.sh` 在 CI 中对比最近一次 main 的耗时，超过阈值告警（非阻塞）

## P2 — 发布/社区

- [ ] **文档站内容**：当前是 Markdown 直出；可加“示例/教程”页与快照diff 展示

## P3 — 功能

- [ ] 多变量联合极限（受“不同趋近速率”限制，可能永远不做；仅记录，不计划实现）
- 函数节点 `sin/exp/log`、变量指数 `a^b` → 已移交 `OPEN_TASKS.md`（#15 / #16）

## 已决策（不再是待办）

- 商业双许可：**不做**。维持单许可 GPL-3.0-or-later，见
  `docs/history/2026-11-07-license-decision.md`
- 需求文档位置：`需求分析.md` 保留在仓库根目录作为**现行规格**（不是归档件）；
  历史草案/建议在 `docs/history/`

## 已完成（按里程碑，详见 CHANGELOG）

- m1 实数符号误差 + 边界 + 分类；m2 复数与 `-k`；m3 双内核与快照基线
- m4 结构哈希、`--real-root`、`Undefined` 三态；m5 哈希分桶、CLI 错误路径
- m6 爆项保护、CI 缓存、Windows MPFR 作业；m7 对齐表格、多规格 `--lim`
- m8 `--lim` 分组/区间、精度极值、macOS CI；m9 P0 闭合（探测路径分类、混合量纲）
- m10 `render` 模块、`sym_mul` 增量合并；m11 Release 工作流与基准历史
- m12 `--json`、`deps.lock`；m13 `--json-schema`、`--lang` 六语言、GMP/MPFR 官方 tarball
- m14 文档站、CoC/SECURITY、Issue/PR 模板、仓库公开与 Pages
