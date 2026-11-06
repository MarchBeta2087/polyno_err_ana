#!/usr/bin/env bash
# 与 tests/snapshots/expected.txt 逐字比对（内核名已归一化）。
set -u
BIN="${1:?binary path required}"
HERE="$(cd "$(dirname "$0")" && pwd)"
TMP="$(mktemp)"
bash "$HERE/snapshot_dump.sh" "$BIN" > "$TMP" 2>&1
if ! command -v diff >/dev/null; then
    # 精简 MSYS2 环境可能没有 diff：退化为 cmp
    if cmp -s "$HERE/snapshots/expected.txt" "$TMP"; then
        rm -f "$TMP"; echo "快照一致"; exit 0
    fi
    echo "快照不一致（无 diff，未打印差异）"; rm -f "$TMP"; exit 1
fi
if diff -u "$HERE/snapshots/expected.txt" "$TMP"; then
    rm -f "$TMP"
    echo "快照一致"
    exit 0
fi
rm -f "$TMP"
echo "快照不一致"
exit 1
