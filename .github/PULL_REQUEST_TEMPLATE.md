## 变更内容

<!-- 一句话说明做了什么；关联 Issue 用 "Closes #123" -->

## 为什么

<!-- 动机、取舍；如与需求冲突请说明 -->

## 影响面

- [ ] 需求文档（`需求分析.md`）已同步
- [ ] 输出格式变更 → `tests/snapshots/expected.txt` 已重新生成
- [ ] 双内核：`numextend` 与 `mpfr` 都已构建并通过 `ctest`
- [ ] 新增 CLI/行为 → `tests/checks.sh` 有断言
- [ ] `CHANGELOG.md` 有记录（用户可见变化）

## 自测

```sh
# 本地已执行（贴关键输出）
bash scripts/verify-deps.sh
bash tests/checks.sh ./build/polyno_err_ana.exe
ctest --test-dir build --output-on-failure
ctest --test-dir build-mpfr --output-on-failure
```

## 备注

<!-- 已知限制、后续 TODO、需要 reviewer 注意的点 -->
