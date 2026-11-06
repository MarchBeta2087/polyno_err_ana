#!/usr/bin/env bash
# 获取 GMP 源码：从 deps.lock 的官方 URL 下载 → 校验 sha256 → 解压到仓库根目录。
# 用法:
#   scripts/fetch-gmp.sh                     # 下载并校验（需外网）
#   scripts/fetch-gmp.sh --tarball FILE      # 用本地已有压缩包（离线）
#   scripts/fetch-gmp.sh --dir DIR           # 指定解压目标（默认仓库根目录）
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LOCK="$ROOT/deps.lock"
TARBALL=""
DEST="$ROOT"

while [ $# -gt 0 ]; do
    case "$1" in
        --tarball) TARBALL="${2:?--tarball 需要路径}"; shift 2 ;;
        --dir) DEST="${2:?--dir 需要路径}"; shift 2 ;;
        -h|--help) sed -n '2,8p' "$0"; exit 0 ;;
        *) echo "未知参数: $1"; exit 2 ;;
    esac
done

get() { sed -n "s/^$1 *= *//p" "$LOCK" | sed 's/ *#.*//' | tr -d ' '; }
URL="$(get gmp_url)"
WANT="$(get gmp_sha256)"
VER="$(get gmp_version)"
DIRNAME="gmp-$VER"

command -v sha256sum >/dev/null || { echo "需要 sha256sum"; exit 1; }

if [ -n "$TARBALL" ]; then
    [ -f "$TARBALL" ] || { echo "找不到压缩包: $TARBALL"; exit 1; }
else
    TARBALL="$DEST/$DIRNAME.tar.xz"
    if [ ! -f "$TARBALL" ]; then
        command -v curl >/dev/null || { echo "需要 curl"; exit 1; }
        echo "==> 下载 $URL"
        curl -fSL --retry 3 -o "$TARBALL" "$URL"
    fi
fi

GOT=$(sha256sum "$TARBALL" | awk '{print $1}')
if [ "$GOT" != "$WANT" ]; then
    echo "sha256 不匹配！"
    echo "  期望: $WANT"
    echo "  实际: $GOT"
    exit 1
fi
echo "ok   sha256: $GOT"

if [ -d "$DEST/$DIRNAME" ]; then
    echo "ok   已存在: $DEST/$DIRNAME（跳过解压）"
else
    tar -C "$DEST" -xf "$TARBALL"
    echo "ok   解压: $DEST/$DIRNAME"
fi

echo "下一步: GMP_SRC=$DEST/$DIRNAME scripts/build-mpfr.sh"
