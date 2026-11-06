# 第三方组件与许可

本项目以 **GPL-3.0-or-later** 发布（见 `LICENSE.md`）。下列组件不随源码分发，
按要求在构建时获取；使用/再分发时须遵守各自许可。

| 组件 | 版本 | 用途 | 引入方式 | 许可 |
| --- | --- | --- | --- | --- |
| [numextend](https://github.com/MarchBeta2087/numextend) | v1.8.1 | 默认数值内核（bigfloat/bigcomplex 语义） | git submodule `numextend/` | MIT（见 `numextend/LICENSE`） |
| [MPFR](https://www.mpfr.org/) | 4.2.x（`aixoss/mpfr` 镜像，base 3.1.2） | 可选数值内核 | git submodule `mpfr/` | LGPL-3.0-or-later（见 `mpfr/COPYING.LESSER`） |
| [GMP](https://gmplib.org/) | 6.3.0 | MPFR 依赖 | 官方 tarball `gmp-6.3.0.tar.xz`，由 `scripts/fetch-gmp.sh` 下载并按 `deps.lock` 的 sha256 校验后解压（未入库） | LGPL-3.0-or-later / GPL-2.0-or-later（双许可） |

## 链接方式说明

- `numextend` 以**静态库**链接，MIT 许可允许该用法，但分发二进制时需保留其版权与许可声明。
- `mpfr` / `gmp` 以**静态库**链接。LGPL-3.0 要求：分发二进制时，
  (a) 附上 LGPL 全文与版权声明；(b) 提供可重新链接的等价方式
  （本项目提供源码与 `scripts/build-mpfr.sh`，构建过程可复现，满足该条）。
  若需更宽松的分发方式，可改用共享库（`--enable-shared`）构建 GMP/MPFR。
- 本仓库未包含 GMP 源码；CI 使用发行版/包管理器提供的二进制包。
- GMP 6.3.0 官方压缩包 sha256（`deps.lock` 记录）：
  `a3c2b80201b89e68616f4ad30bc66aee4927c3ce50e33929ca819d5c43538898`（来源
  `https://gmplib.org/download/gmp/gmp-6.3.0.tar.xz`）。

## 上游文档

- numextend 设计：`numextend/docs/design.md`、`numextend/docs/manual.html`
- MPFR 手册：`mpfr/doc/mpfr.info`（info 格式）
- GMP 手册：随源码 `gmp-6.3.0/doc/`
