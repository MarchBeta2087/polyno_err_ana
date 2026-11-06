#!/usr/bin/env bash
# 输出归一化快照：内核名与位数替换为 KERNEL，便于跨后端比对。
set -u
BIN="${1:?binary path required}"
HERE="$(cd "$(dirname "$0")" && pwd)"
while IFS= read -r line; do
    case "$line" in ''|'#'*) continue ;; esac
    printf '### %s\n' "$line"
    # shellcheck disable=SC2086
    eval "\"\$BIN\" $line" 2>&1 | sed -E 's/(numextend|mpfr)(, [0-9]+ bit)/KERNEL\2/'
done < "$HERE/cases.txt"
