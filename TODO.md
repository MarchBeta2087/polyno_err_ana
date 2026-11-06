# TODO / 路线图

唯一待办来源：`设计.md` 的「下一步」只指向本文件，避免两处漂移。
规则：**只放未完成项**；完成后从本文件删除，并在 `CHANGELOG.md` 记一笔。
每项标注关联需求（`需求分析.md` 章节/编号）与验收方式。

## P0 — 需求缺口

（暂无；m9 已闭合：探测路径的相对形式分类、复数相对模式语义、混合量纲分类）

## P1 — 工程


## P2 — 发布/社区


## 已决策（不再是待办）

- 商业双许可：**不做**。维持单许可 GPL-3.0-or-later，理由与影响见
  `docs/history/2026-11-07-license-decision.md`

## 已完成（移出，见 CHANGELOG）

- m14.1：文档站（pandoc + Pages 工作流）、CoC、SECURITY、Issue/PR 模板

## 待办补充

- [ ] 仓库转公开后：启用 Pages（Source = GitHub Actions）+ 设置变量 `ENABLE_PAGES=true`

- m14：`--lang` 六语言（zh/en/ja/ko/es/pt），单表实现 + 6 份帮助文本

- m13.4：移除 `mpfr/` submodule，迁移到官方 tarball 4.2.1（`docs/history/2026-11-07-mpfr-tarball-migration.md`）

- m11：Release 工作流（linux/windows 静态构建 + SHA256SUMS）、`bench/history.csv` 与 `bench_compare.sh`
- m12：`--json` 输出、`render` 输出块、`deps.lock` + `verify-deps.sh`
- m13：`--json-schema`、`--lang zh|en`、schema 自检、GMP/MPFR 官方 tarball 校验（`fetch-gmp.sh` / `fetch-mpfr.sh`）

- m10：`render` 模块、`sym_mul` 增量合并（上限按结果项数判定）、快照 35 条、低精度字面量对齐

- 结构哈希原子归并、`cleanup()` 哈希分桶、`--max-terms` 爆项保护（m4–m6）
- `--real-root`、`Undefined` 三态呈现（m4）
- 对齐表格输出、多规格 `--lim`、分组/区间语法（m7–m8）
- `-p` 极值用例 T-14、CI 三平台（linux/windows/macos）+ numextend 构建缓存
