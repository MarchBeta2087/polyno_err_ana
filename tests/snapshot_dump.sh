#!/usr/bin/env bash
# 输出归一化快照：内核名/位数、版本号替换为占位符，便于跨后端与跨版本比对。
set -u
BIN="${1:?binary path required}"
HERE="$(cd "$(dirname "$0")" && pwd)"
while IFS= read -r line; do
    case "$line" in ''|'#'*) continue ;; esac
    printf '### %s\n' "$line"
    # shellcheck disable=SC2086
    eval "\"\$BIN\" $line" 2>&1 |
        sed -E -e 's/(numextend|mpfr)(, [0-9]+ bit)/KERNEL\2/' \
               -e 's/"version": "[^"]*"/"version": "<v>"/'                -e 's/"kernel": "[a-z]*"/"kernel": "<kernel>"/'
done < "$HERE/cases.txt"
