# TODO / 路线图

唯一待办来源：`设计.md` 的「下一步」只指向本文件，避免两处漂移。
规则：**只放未完成项**；完成后从本文件删除，并在 `CHANGELOG.md` 记一笔。
每项标注关联需求（`需求分析.md` 章节/编号）与验收方式。

## P0 — 需求缺口

（暂无；m9 已闭合：探测路径的相对形式分类、复数相对模式语义、混合量纲分类）

## P1 — 工程

- [ ] 错误消息英文版（`--lang en`）：需求 N-2 只要求列号，属可选；需先定“输出标签是否也翻译”
- [ ] JSON schema 化（`schema/json_out.schema.json`）并加 `--json-schema` 自检，便于下游校验

## P2 — 发布/社区

- [ ] 许可咨询：是否提供商业双许可（见 `FAQ.md` §5）——**需人工决策**
- [ ] GMP tarball 的 `sha256` 补录（`deps.lock` 里为 `PENDING`）：本机访问 mpfr.org/gmplib.org
      被 TLS 拦截，需在可联网环境执行 `scripts/verify-deps.sh --fill-hash`
- [ ] 把 `mpfr/` 从 `aixoss/mpfr` 镜像切换到官方 tarball（4.2.x）+ 校验和
- [ ] 文档站点化（`docs/` 静态页，含表格渲染示例）

## 已完成（移出，见 CHANGELOG）

- m11：Release 工作流（linux/windows 静态构建 + SHA256SUMS）、`bench/history.csv` 与 `bench_compare.sh`
- m12：`--json` 输出、`render` 输出块、`deps.lock` + `verify-deps.sh`

- m10：`render` 模块、`sym_mul` 增量合并（上限按结果项数判定）、快照 35 条、低精度字面量对齐

- 结构哈希原子归并、`cleanup()` 哈希分桶、`--max-terms` 爆项保护（m4–m6）
- `--real-root`、`Undefined` 三态呈现（m4）
- 对齐表格输出、多规格 `--lim`、分组/区间语法（m7–m8）
- `-p` 极值用例 T-14、CI 三平台（linux/windows/macos）+ numextend 构建缓存
