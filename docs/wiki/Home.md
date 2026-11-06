# polyno_err_ana Wiki

> 本 Wiki 只放**社区维护的笔记**（用法片段、教学材料、平台实测记录）。
> 规范文档以仓库为准，并发布在 <https://marchbeta2087.github.io/polyno_err_ana/>。

## 入口

| 内容 | 位置 |
| --- | --- |
| 快速开始 / 构建 / 下载 | [README](https://github.com/MarchBeta2087/polyno_err_ana#readme) |
| 示例与教程 | [EXAMPLES.md](https://github.com/MarchBeta2087/polyno_err_ana/blob/main/EXAMPLES.md) |
| 需求与验收 | [需求分析.md](https://github.com/MarchBeta2087/polyno_err_ana/blob/main/需求分析.md) |
| 设计说明 | [设计.md](https://github.com/MarchBeta2087/polyno_err_ana/blob/main/设计.md) |
| 平台支持（Tier） | [PLATFORMS.md](https://github.com/MarchBeta2087/polyno_err_ana/blob/main/PLATFORMS.md) |
| 开放任务 | [OPEN_TASKS.md](https://github.com/MarchBeta2087/polyno_err_ana/blob/main/OPEN_TASKS.md) |
| 讨论区 | [Discussions](https://github.com/MarchBeta2087/polyno_err_ana/discussions) |
| 安全报告 | [Security Advisory](https://github.com/MarchBeta2087/polyno_err_ana/security/advisories/new) |

## 社区笔记（欢迎补充）

- **Tier 3 实测记录**：FreeBSD / OpenBSD / NetBSD 的构建步骤与坑（请附 `cmake` 输出与 `-V` 结果）
- **用法片段**：把 `--json` 接进脚本、和 Python/Julia 对照的小例子
- **教学材料**：误差传播一阶形式 vs 精确边界的讲解

> 新增页面请保持“可复查”：给出命令、版本（`-V`）、平台与完整输出。

## 提问前

1. 先看 [FAQ](https://github.com/MarchBeta2087/polyno_err_ana/blob/main/FAQ.md)
2. 附上 `polyno_err_ana -V` 与完整命令
3. 数值差异请附 `--json` 输出
