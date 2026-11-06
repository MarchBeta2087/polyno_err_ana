# 安全策略

## 支持范围

| 版本 | 安全修复 |
| --- | --- |
| `0.14.x`（当前主线） | ✅ |
| 更早的 `0.1x` / 里程碑 tag | ❌（建议升级到最新 tag） |

预编译包见 GitHub Releases；从源码构建请使用最新 `main`。

## 报告漏洞

**请勿在公开 Issue 中披露未修复的漏洞。**

优先使用 GitHub 的私有漏洞报告：

1. 打开仓库 → **Security** → **Report a vulnerability**（GitHub Security Advisory）
2. 或直接访问 `https://github.com/MarchBeta2087/polyno_err_ana/security/advisories/new`

也可通过 GitHub 私信维护者 [@MarchBeta2087](https://github.com/MarchBeta2087)。

请在报告中尽量包含：

- 受影响的版本 / commit、内核（`numextend` 或 `mpfr`）、平台
- 触发命令与公式（可最小化到最短形式）
- 现象：崩溃、挂起、内存增长、错误结果等
- 如可能，附 `--json` 输出或退出码

## 处理流程

| 阶段 | 目标时间 |
| --- | --- |
| 确认收到 | 3 个工作日内 |
| 初步评估（可利用性/影响） | 7 个工作日内 |
| 修复或缓解方案 | 视严重程度，尽量在下一个 tag |

修复发布后会在此文件与 `CHANGELOG.md` 记录致谢（如你希望匿名，请在报告中说明）。

## 已知的“设计内”行为（非漏洞）

- **资源消耗**：符号展开受 `--max-terms`（默认 200000）限制，超限会退化为数值探测；
  超大精度（如 `-p 1000000`）会显著变慢，属预期行为。
- **`Undefined` 而非异常**：除零、`0^0`、`log` 域外等按需求以 `Undefined` 呈现（N-3）。
- **静态链接 LGPL 库**：发布包静态链接 MPFR/GMP，其许可义务见 `THIRD_PARTY.md`，
  这不是本项目可修复的安全问题。
- **解析器不执行代码**：表达式仅支持 `+ - * / ^` 与常量指数，不存在脚本执行面；
  但极端嵌套仍可能造成栈深度问题——如遇到崩溃请按漏洞流程报告。

## 依赖安全

- `numextend`、MPFR、GMP 的版本与校验和固定在 `deps.lock`；CI 会运行 `scripts/verify-deps.sh`。
- 发现上游漏洞时，请同样通过私有渠道告知，便于尽快升级并发布新版本。
