# TODO / 路线图

唯一待办来源：`设计.md` 的「下一步」只指向本文件，避免两处漂移。
规则：**只放未完成项**；完成后从本文件删除，并在 `CHANGELOG.md` 记一笔。
每项标注关联需求（`需求分析.md` 章节/编号）与验收方式。

## P0 — 需求缺口

- [ ] **非整数/复数指数走 `EvalError` 时的相对形式分类**
      `sym_ok=false` 时 `δf/f` 一律给 `Undefined`（`src/main.cpp` 分类分支）。
      需按 §3.6 规则 8 用 `K = k_g − k_f`，配合数值探测给出 `c0/cinf/fin`。
      验收：`-C z 1e-4 1e-4 -f '(1+z)^(1+i)' -k 2` 的 `δf/f` 列不再是 `undef`。
- [ ] **复数相对模式语义固化**
      `-C` + `--relative` 现按 `|z|·(re_err + i·im_err)` 缩放，需写进 §3.1/§3.2 并补用例。
      验收：`-C z 1e-4 1e-4 --relative --estimated z=1+0.5i` 有快照。
- [ ] **多变量混合分类的组合覆盖**
      `f` 同时含实数与复数变量时，`δf` 的量纲（实数 vs 模）目前按变量逐个判定，
      需明确“δf 是全微分（复值）还是逐变量模”并补 4 组用例（§3.6 “单变量变”）。

## P1 — 工程

- [ ] 快照用例 30+（当前 27；补 `-p 8/64`、`--lim x:n0..ninf`、多变量三类混合）
- [ ] 输出渲染从 `src/main.cpp` 抽到 `src/render.{hpp,cpp}`（当前 `render_table` 与各区块仍在 main）
- [ ] `sym_mul` 的项数预估更精确（按去重后的因子键估算，而非项数乘积）
- [ ] 错误消息英文版（`--lang en`，需求 N-2 只要求列号，可选）
- [ ] `scripts/bench.sh` 记录历史结果（CSV + 简单对比），避免性能回退无感知

## P2 — 发布/社区

- [ ] 二进制发布（GitHub Release 附 Linux/macOS/Windows 构建产物与校验和）
- [ ] 许可咨询：是否提供商业双许可（见 `FAQ.md` §5）
- [ ] 把 `mpfr/` 从 `aixoss/mpfr` 镜像改为官方 tarball 校验（当前 base 3.1.2）
- [ ] 文档站点化（`docs/` 静态页，含表格渲染示例）

## 已完成（移出，见 CHANGELOG）

- 结构哈希原子归并、`cleanup()` 哈希分桶、`--max-terms` 爆项保护（m4–m6）
- `--real-root`、`Undefined` 三态呈现（m4）
- 对齐表格输出、多规格 `--lim`、分组/区间语法（m7–m8）
- `-p` 极值用例 T-14、CI 三平台（linux/windows/macos）+ numextend 构建缓存
