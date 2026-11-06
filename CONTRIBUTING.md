# 贡献指南

## 构建与测试

```sh
# 数值内核（子模块）
cmake -S numextend -B numextend/build -DCMAKE_BUILD_TYPE=Release
cmake --build numextend/build -j

# 本项目（numextend 内核）
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure

# 可选：MPFR 内核（需 GMP/MPFR 静态库，见 README）
scripts/build-mpfr.sh
cmake -S . -B build-mpfr -DCMAKE_BUILD_TYPE=Release \
      -DPOLYNO_ENABLE_MPFR=ON -DPNA_THIRD_PARTY=$PWD/third_party_prefix
cmake --build build-mpfr -j
ctest --test-dir build-mpfr --output-on-failure
```

## 代码约定

- C++23，无第三方符号/表达式库；格式化用仓库内 `fmt`/`c_fmt`，不引 `{fmt}`。
- 两个内核（`src/num.cpp` / `src/num_mpfr.cpp`）必须实现 `src/num.hpp` 的同一组原语；
  改接口时同步改两处，并确认 `ctest` 在两套构建下都通过。
- 符号层（`src/sym.cpp`）的原子更新必须经 `Node::hash`（见 `ast.cpp:finish`），
  禁止在比较/归并路径上新增 `to_string`。
- 新增行为先改 `需求分析.md`，再改代码与 `tests/cases.txt` 快照。

## 测试

- `tests/checks.sh`：断言式回归（布局无关，按列取值）。
- `tests/cases.txt` + `tests/snapshots/expected.txt`：逐字快照，**两个内核必须一致**。
- 修改输出格式后：

```sh
bash tests/snapshot_dump.sh ./build/polyno_err_ana.exe > tests/snapshots/expected.txt
bash tests/snapshot_test.sh ./build/polyno_err_ana.exe
```

## 从哪里开始

- `OPEN_TASKS.md`：社区可认领的任务（标注难度、所需技能、验收标准，均对应 GitHub Issue）
- `TODO.md`：维护者自己的工程待办（通常不与外部 PR 冲突，避免重复投入）
- `good first issue` 标签：低门槛任务（目前是四种语言的母语审校）

## 待办与里程碑

待办只在 `TODO.md` 维护（P0/P1/P2，带验收方式）。完成一项就**从 TODO 删除**，并在
`CHANGELOG.md` 记一笔；不要在 `设计.md` 里另开清单，避免两处漂移。

## 提交

- 信息用中文或英文均可，首行不超过 72 字符，正文说明“为什么”。
- 一个提交只做一件事；跨内核/跨文档的改动在正文列出影响面。
- 提交前本地跑：`bash tests/checks.sh <bin>` + 两套 `ctest`。
- 版本号在 `src/main.cpp:kVersion`，与 `CHANGELOG.md`、git tag 保持一致。

## 许可证

- 本项目只以 **GPL-3.0-or-later** 发布，不提供商业授权（决定记录见
  `docs/history/2026-11-07-license-decision.md`）。
- 采用「入站=出站」：提交 PR 即表示你同意该贡献以 GPL-3.0-or-later 授权，
  无需签署额外 CLA。
- 源文件首行保留 `SPDX-License-Identifier: GPL-3.0-or-later`。
- 注意：将来若要把项目改为其它许可，需要取得**每一位**外部贡献者的同意，
  因此请勿在未沟通的情况下引入与 GPL 不兼容的代码或大段第三方代码。
