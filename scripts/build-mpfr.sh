#!/usr/bin/env bash
# 构建 GMP + MPFR 静态库到 third_party_prefix（供 -DPOLYNO_ENABLE_MPFR=ON 使用）。
# 用法: scripts/build-mpfr.sh [安装前缀] [--dry-run]
# 依赖: gcc / make / m4；Windows 建议在 MSYS2 MINGW64 shell 中运行。
set -euo pipefail
DRY_RUN=0

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PREFIX=""
for a in "$@"; do
    case "$a" in
        --dry-run) DRY_RUN=1 ;;
        -*) echo "未知参数: $a"; exit 2 ;;
        *) PREFIX="$a" ;;
    esac
done
PREFIX="${PREFIX:-$ROOT/third_party_prefix}"
GMP_SRC="${GMP_SRC:-$ROOT/gmp-6.3.0}"
# MPFR 唯一来源：官方 tarball 解压目录（scripts/fetch-mpfr.sh）
MPFR_VER="$(sed -n 's/^mpfr_version *= *//p' "$ROOT/deps.lock" | sed 's/ *#.*//' | tr -d ' ')"
if [ -z "${MPFR_SRC:-}" ]; then
    MPFR_SRC="$ROOT/mpfr-$MPFR_VER"
fi

if [ "$DRY_RUN" -eq 1 ]; then
    echo "GMP_SRC=$GMP_SRC $([ -d "$GMP_SRC" ] || echo '(缺失)')"
    echo "MPFR_SRC=$MPFR_SRC $([ -d "$MPFR_SRC" ] || echo '(缺失)')"
    echo "PREFIX=$PREFIX"
    exit 0
fi

[ -d "$GMP_SRC" ] || {
    echo "找不到 GMP 源码：$GMP_SRC"
    echo "先执行 scripts/fetch-gmp.sh（会按 deps.lock 校验 sha256）"
    exit 1
}
[ -d "$MPFR_SRC" ] || {
    echo "找不到 MPFR 源码：$MPFR_SRC"
    echo "先执行 scripts/fetch-mpfr.sh（官方 tarball + sha256 校验）"
    exit 1
}


command -v make >/dev/null || { echo "需要 make"; exit 1; }
command -v m4 >/dev/null || { echo "需要 m4"; exit 1; }

mkdir -p "$PREFIX"

# GCC 14+ 默认 C23 会让 GMP 的 long-long 探针（void g(){} 后又传参）编译失败。
GMP_CFLAGS="${GMP_CFLAGS:--O2 -std=gnu17}"

echo "==> GMP"
(cd "$GMP_SRC" && ./configure --prefix="$PREFIX" --enable-static --disable-shared \
    CFLAGS="$GMP_CFLAGS" && make -j"$(nproc)" && make install)

echo "==> MPFR"
(cd "$MPFR_SRC" && ./configure --prefix="$PREFIX" --with-gmp="$PREFIX" \
    --enable-static --disable-shared && make -j"$(nproc)" && make install)

echo "==> 完成: $PREFIX/lib/libmpfr.a"
echo "下一步: cmake -S . -B build-mpfr -DPOLYNO_ENABLE_MPFR=ON -DPNA_THIRD_PARTY=$PREFIX"
