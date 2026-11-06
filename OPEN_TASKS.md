# 开放任务（Open Tasks / Help Wanted）

本文件列出**社区更容易完成、维护者短期难以覆盖**的任务。与 `TODO.md` 的分工：

- `TODO.md`：维护者自己的工程待办（有明确实现路径）；
- `OPEN_TASKS.md`：需要外部帮助、或难度/资源门槛较高的任务（母语审校、跨平台打包、大型特性）。

认领方式：在对应 Issue 下留言「我认领」，先描述计划再动手；改动请遵循
`CONTRIBUTING.md`（入站=出站，GPL-3.0-or-later，无 CLA）。

| # | 任务 | 难度 | 需要的人 | Issue |
| --- | --- | --- | --- | --- |
| 1 | 日语（ja）文案母语审校 | 低 | 日语母语者 | [#9](https://github.com/MarchBeta2087/polyno_err_ana/issues/9) |
| 2 | 韩语（ko）文案母语审校 | 低 | 韩语母语者 | [#10](https://github.com/MarchBeta2087/polyno_err_ana/issues/10) |
| 3 | 西班牙语（es）文案母语审校 | 低 | 西语母语者 | [#11](https://github.com/MarchBeta2087/polyno_err_ana/issues/11) |
| 4 | 葡萄牙语（pt）文案母语审校 | 低 | 葡语母语者 | [#12](https://github.com/MarchBeta2087/polyno_err_ana/issues/12) |
| 5 | macOS x86_64 发布产物 | 低 | **部分完成**（arm64 已发；Intel 由 `attach-macos-x86` 自动补挂，等 runner） | [#13](https://github.com/MarchBeta2087/polyno_err_ana/issues/13) |
| 6 | ~~MPFR 内核预编译包~~ | — | 已完成（#14，v0.15.1：linux/windows 两个 `-mpfr` 包） | [#14](https://github.com/MarchBeta2087/polyno_err_ana/issues/14) |
| 7 | 函数节点 `sin/exp/log` | 高 | C++23 + 符号计算经验 | [#15](https://github.com/MarchBeta2087/polyno_err_ana/issues/15) |
| 8 | 变量指数 `a^b` | 高 | 同上（依赖 #15） | [#16](https://github.com/MarchBeta2087/polyno_err_ana/issues/16) |
| 9 | 多变量联合极限（需先定路径语义） | 高 | 数值分析 + 需求设计经验 | [#17](https://github.com/MarchBeta2087/polyno_err_ana/issues/17) |

## 为什么这些任务难

**母语审校（1–4）**：需要母语者判断语域（敬体/常体）、术语一致性与自然度。
维护者只能保证技术正确，无法保证地道；机器翻译又容易在“极限分类”“估计点”等术语上出错。
改动只涉及 `src/i18n.cpp` 一个文件，但需要重新生成快照（`tests/snapshots/expected.txt`）。

**跨平台打包（5–6）**：需要在对应平台验证链接方式（静态运行库、LGPL 依赖、UCRT64/arm64），
维护者手上只有一台 Windows 机器，无法本地验证 macOS/发行版差异。

**联合极限（9）**：难点在**语义**而非编码。`δx`、`δy` 以不同速率趋近时极限可以不同
（`f = x - y`：`δx=δy=t` → 0，`δx=t², δy=t` → −t → 0 或 ∞ 取决于更高阶），
所以必须先定义“路径”（同速率 / `|δv| ~ t^p` 参数化 / 逐次极限），修改需求 §3.6，
再实现并用“同一公式不同路径给出不同结果”的用例证明实现真的按路径区分。
若无法给出自洽定义，正确做法是维持“不支持”并在 FAQ 说明原因。

**大型特性（7–8）**：`sin/exp/log` 会牵动解析器、数值层（两个内核）、符号层（函数原子与导数）、
首项提取与分类、需求文档五处；`a^b` 还要求 `log/exp` 先落地并解决分支一致性。
这类改动需要先写设计草案（可放 `docs/history/`）再动代码。

## 我可以提供的支持

- 在 Issue 里给出实现位置指引与验收标准（已在各 Issue 写明）
- 审阅 PR：关注双内核一致性、快照更新、需求文档同步
- 若任务属于 `TODO.md` 中维护者自己的范围，会明确回复“由维护者处理”

## 不建议认领的情况

- 只想改一处措辞但不跑测试 / 不更新快照 —— 会让 CI 变红；
- 想顺带重构（如改 `Sym` 数据结构）—— 请先开 Issue 讨论，避免与既有里程碑冲突。
