#!/usr/bin/env bash
# 获取 MPFR 源码：官方 tarball → sha256 校验 → 解压（与 fetch-gmp.sh 同一模式）。
# 子模块 mpfr/ 仍保留为备选（不联网时可用）。
# 用法:
#   scripts/fetch-mpfr.sh                    # 下载并校验（需外网）
#   scripts/fetch-mpfr.sh --tarball FILE     # 用本地已有压缩包（离线）
#   scripts/fetch-mpfr.sh --dir DIR          # 解压目标（默认仓库根目录）
#   scripts/fetch-mpfr.sh --record           # 首次落库：下载后把 sha256 写回 deps.lock
#                                            #   （仅维护者使用；会修改 deps.lock）
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LOCK="$ROOT/deps.lock"
TARBALL=""
DEST="$ROOT"
RECORD=0

while [ $# -gt 0 ]; do
    case "$1" in
        --tarball) TARBALL="${2:?--tarball 需要路径}"; shift 2 ;;
        --dir) DEST="${2:?--dir 需要路径}"; shift 2 ;;
        --record) RECORD=1; shift ;;
        -h|--help) sed -n '2,10p' "$0"; exit 0 ;;
        *) echo "未知参数: $1"; exit 2 ;;
    esac
done

get() { sed -n "s/^$1 *= *//p" "$LOCK" | sed 's/ *#.*//' | tr -d ' '; }
# 多值键（空格分隔）保留内部空格
getlist() { sed -n "s/^$1 *= *//p" "$LOCK" | sed 's/ *#.*//'; }
URL="$(get mpfr_url)"
MIRRORS="$(getlist mpfr_mirrors)"
WANT="$(get mpfr_sha256)"
VER="$(get mpfr_version)"
DIRNAME="mpfr-$VER"

command -v sha256sum >/dev/null || { echo "需要 sha256sum"; exit 1; }

if [ -n "$TARBALL" ]; then
    [ -f "$TARBALL" ] || { echo "找不到压缩包: $TARBALL"; exit 1; }
else
    TARBALL="$DEST/$DIRNAME.tar.xz"
    if [ ! -f "$TARBALL" ]; then
        command -v curl >/dev/null || { echo "需要 curl"; exit 1; }
        ok=0
        for u in "$URL" $MIRRORS; do
            echo "==> 下载 $u"
            if curl -fSL --retry 2 --connect-timeout 20 -o "$TARBALL.part" "$u"; then
                mv "$TARBALL.part" "$TARBALL"
                ok=1
                break
            fi
            echo "    失败，尝试下一个源"
        done
        rm -f "$TARBALL.part"
        [ "$ok" -eq 1 ] || { echo "所有下载源均失败（可离线用 --tarball）"; exit 1; }
    fi
fi

GOT=$(sha256sum "$TARBALL" | awk '{print $1}')

if [ "$WANT" = "PENDING" ]; then
    if [ "$RECORD" -ne 1 ]; then
        cat <<EOF
deps.lock 里 mpfr_sha256 仍为 PENDING，无法校验。
确认来源可信（$URL）后可执行：

  scripts/fetch-mpfr.sh --record

它会下载/校验并把 sha256 写回 deps.lock（仅维护者操作）。
本次实际哈希：$GOT
EOF
        exit 1
    fi
    echo "==> 记录 sha256 到 deps.lock: $GOT"
    python - "$LOCK" "$GOT" <<'PY'
import sys
lock, got = sys.argv[1], sys.argv[2]
text = open(lock, encoding="utf-8").read()
out, hit = [], False
for line in text.splitlines():
    if line.startswith("mpfr_sha256"):
        line = f"mpfr_sha256 = {got}"
        hit = True
    out.append(line)
if not hit:
    out.append(f"mpfr_sha256 = {got}")
open(lock, "w", encoding="utf-8", newline="\n").write("\n".join(out) + "\n")
PY
    WANT="$GOT"
fi

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

echo "下一步: MPFR_SRC=$DEST/$DIRNAME scripts/build-mpfr.sh"
