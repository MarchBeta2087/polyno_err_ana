#!/usr/bin/env bash
# 与 tests/snapshots/expected.txt 逐字比对（内核名已归一化）。
set -u
BIN="${1:?binary path required}"
HERE="$(cd "$(dirname "$0")" && pwd)"
TMP="$(mktemp)"
bash "$HERE/snapshot_dump.sh" "$BIN" > "$TMP" 2>&1
if diff -u "$HERE/snapshots/expected.txt" "$TMP"; then
    rm -f "$TMP"
    echo "快照一致"
    exit 0
fi
rm -f "$TMP"
echo "快照不一致"
exit 1
