# 平台支持等级

本项目按 **Tier** 声明支持程度。变更等级需在 Issue 中给出证据（CI 结果或社区实测），
并在 `CHANGELOG.md` 记录。

## Tier 1 —— 每次提交 CI 验证 + 官方预编译包

| 平台 | 工具链 | 验证方式 |
| --- | --- | --- |
| Linux x86_64 | GCC / Clang（glibc ≥ 2.35，如 Ubuntu 22.04+） | `ci.yml` 全量：双内核构建 + `regression` + `snapshot` + 性能门禁；Release 出包（含 `-mpfr`） |
| Windows x86_64 | MSYS2 **UCRT64**（GCC ≥ 13） | 同上（含 MPFR 内核 + `-mpfr` 包） |
| macOS（通用二进制） | AppleClang（macos-latest，`CMAKE_OSX_ARCHITECTURES="arm64;x86_64"`） | `ci.yml` 构建 + `lipo` 校验 + 在 arm64 上跑回归；Release 出 `macos-universal2` 包 |

Tier 1 承诺：

- 构建必须通过，回归/快照必须全绿；失败视为阻断问题（P0）；
- 每个 tag 提供静态运行库单文件包与 `SHA256SUMS.txt`；
- 安全/严重缺陷优先修复。

## Tier 2 —— 尽力支持（无独立 CI，靠通用二进制覆盖）

| 平台 | 说明 |
| --- | --- |
| macOS x86_64（Intel） | **不再单独安排 Intel runner（`macos-13`）**：改为在 arm64 runner 上构建 **universal2**（`arm64;x86_64`）单一二进制，Intel 机器直接运行同一个包。CI 会在构建后 `lipo -info` 校验两个架构都在，并在 arm64 上跑回归；**Intel 实机运行未纳入 CI**。 |

Tier 2 承诺：已知可运行；不做 Intel 实机回归，因此 Intel 专属问题（如特定 CPU/OS 组合）
可能晚发现。若出现**严重缺陷**（崩溃、错误结果），按普通 bug 修复并**发布 PATCH**（`X.Y.Z+1`）。

> 为什么不跑 Intel CI：`macos-13` 排队时间不可控、可能超时或被取消，且不产出额外价值
> （universal2 已覆盖 x86_64）。如果将来需要实机验证，欢迎在 `OPEN_TASKS.md` 提 Issue 征集。

## Tier 3 —— 源码可构建，社区验证（无 CI、无预编译包）

| 平台 | 预期步骤 | 常见坑 |
| --- | --- | --- |
| FreeBSD | `cmake -S numextend -B numextend/build && cmake --build`；本项目同上（Makefiles/Ninja 均可） | MPFR 内核需自行构建 GMP/MPFR（`scripts/fetch-*` + `build-mpfr.sh`，脚本假设 GNU 工具链） |
| OpenBSD | 同上；建议用 `egcc`/`clang` + `gmake` | `sha256sum` 需装 `coreutils`；`bash` 非默认（脚本需 bash） |
| NetBSD | 同上 | 同上；`nproc` 可能不存在，手工 `-j` |

Tier 3 承诺：**不同意为“官方支持”**，但欢迎 PR 修正可移植性问题（`help wanted`）；文档会记录已知限制。
CI 暂不覆盖（可用 `vmactions/freebsd-vm` 之类的 action 后续接入）。

## Tier 4 —— 不保证

其余所有环境，包括但不限于：

- Linux 非 x86_64 架构（arm64、riscv64 等）与 musl（Alpine）；
- MSVC 工具链（当前 CMake/源码未适配 MSVC 的 `-static`/告警差异）；
- 32 位平台、Windows 非 UCRT64 的 MinGW/MSYS 变体；
- 移动端、嵌入式、WASM。

Tier 4 意味着“可能可以编译，但不在回归范围，问题可能被标记为 `wontfix`”。

## 等级变更规则

- **升级**：连续两个 tag 在目标平台构建+回归通过（附 CI 链接或社区报告），并有人愿意维护；
- **降级**：CI 连续失败或 runner 长期不可用（如 macOS Intel），先降到 Tier 2/3 并说明；
- 所有变更同步到本文件、`README.md` 的“平台”小节与 Release 说明。
