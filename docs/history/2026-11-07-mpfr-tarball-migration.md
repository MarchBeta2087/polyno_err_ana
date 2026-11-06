# 迁移说明：MPFR 由 git submodule 改为官方 tarball

- 日期：2026-11-07
- 版本：v0.13.4-m13

## 变更

| 项目 | 之前 | 之后 |
| --- | --- | --- |
| 来源 | submodule `mpfr/`（`aixoss/mpfr`，base 3.1.2） | 官方 tarball `mpfr-4.2.1.tar.xz` |
| 版本 | 3.1.2 base | 4.2.1 |
| 校验 | 仅 git commit | `deps.lock` 记录 URL + sha256 |
| 获取 | `git submodule update --init mpfr` | `scripts/fetch-mpfr.sh` |
| 构建 | `scripts/build-mpfr.sh` | 同左，默认使用 `mpfr-4.2.1/` |

`deps.lock` 现只保留 numextend 的 submodule 信息；MPFR 只记录 version/url/sha256/source。

## 本地迁移步骤

已 clone 过旧版本的仓库，需要：

```sh
git pull
git submodule deinit -f mpfr || true
rm -rf mpfr .git/modules/mpfr
git config -f .gitmodules --remove-section submodule.mpfr 2>/dev/null || true

scripts/fetch-mpfr.sh        # 下载 + 校验 + 解压到 mpfr-4.2.1/
scripts/build-mpfr.sh        # 重建 GMP+MPFR 静态库
```

仍在使用旧 `third_party_prefix`（由 3.1.2 base 构建）的目录建议删除后重建，
避免与 4.2.1 混链：

```sh
rm -rf third_party_prefix build-mpfr
cmake -S . -B build-mpfr -DPOLYNO_ENABLE_MPFR=ON
cmake --build build-mpfr -j && ctest --test-dir build-mpfr
```

## CI

CI 不使用 tarball，直接装发行版包（Linux `libmpfr-dev`、Windows ucrt64 `mpfr`、macOS `brew mpfr`），
版本可能与 `deps.lock` 不同；回归对两者都通过（数值格式化一致）。

## 原因

- 上游镜像停在 base 3.1.2，与官方 4.2.x 有差距；
- tarball + sha256 可审计、无需 git 历史，构建更快；
- 与 GMP 的处理方式统一（`scripts/fetch-gmp.sh`）。
