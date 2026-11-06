#!/usr/bin/env bash
# 校验子模块与依赖版本是否与 deps.lock 一致。
# 用法: scripts/verify-deps.sh [--tarball FILE]
#   （MPFR 子模块已移除：只校验 numextend 提交 + GMP/MPFR 源码目录与 tarball）
#   --tarball  额外校验本地 GMP 压缩包的 sha256（离线也可用）
set -u
TARBALL=""
[ "${1:-}" = "--tarball" ] && TARBALL="${2:?--tarball 需要路径}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LOCK="$ROOT/deps.lock"
fail=0
get() { sed -n "s/^$1 *= *//p" "$LOCK" | sed 's/ *#.*//' | tr -d ' '; }

check_commit() {  # path expected label
    local path="$ROOT/$1" want="$2" label="$3" got
    if [ ! -d "$path/.git" ] && [ ! -f "$path/.git" ]; then
        printf 'MISS %s: 子模块未初始化（git submodule update --init）\n' "$label"
        fail=$((fail + 1))
        return
    fi
    got=$(git -C "$path" rev-parse HEAD 2>/dev/null)
    case "$got" in
        "$want"*) printf 'ok   %s: %s\n' "$label" "${got:0:7}" ;;
        *) printf 'DRIFT %s: 期望 %s…，实际 %s…\n' "$label" "${want:0:7}" "${got:0:7}"; fail=$((fail + 1)) ;;
    esac
}

check_commit "$(get numextend_path)" "$(get numextend_commit)" numextend

gmp_dir="$ROOT/$(get gmp_source)"
if [ -d "$gmp_dir" ]; then
    # GMP 不带 VERSION 文件：从 gmp-h.in 读主/次/补丁号（按字段精确匹配，避免 RELEASE 行）
    ver=$(awk '$2=="__GNU_MP_VERSION"{m=$3} $2=="__GNU_MP_VERSION_MINOR"{n=$3} $2=="__GNU_MP_VERSION_PATCHLEVEL"{p=$3} END{if(m!="")printf "%s.%s.%s",m,n,p}' "$gmp_dir/gmp-h.in" 2>/dev/null)
    if [ "$ver" = "$(get gmp_version)" ]; then
        printf 'ok   gmp: %s（源码目录）\n' "$ver"
    else
        printf 'DRIFT gmp: 期望 %s，实际 %s\n' "$(get gmp_version)" "$ver"
        fail=$((fail + 1))
    fi
else
    printf 'SKIP gmp: 源码目录不存在（仅 MPFR 内核需要）\n'
fi

mpfr_ver_dir="$ROOT/$(get mpfr_source)"
if [ -d "$mpfr_ver_dir" ]; then
    ver=$(sed -n '1p' "$mpfr_ver_dir/VERSION" 2>/dev/null | tr -d ' 
')
    if [ "$ver" = "$(get mpfr_version)" ]; then
        printf 'ok   mpfr: %s（官方 tarball 目录）
' "$ver"
    else
        printf 'DRIFT mpfr tarball: 期望 %s，实际 %s
' "$(get mpfr_version)" "$ver"
        fail=$((fail + 1))
    fi
fi

if [ -n "$TARBALL" ]; then
    if [ -f "$TARBALL" ]; then
        want="$(get gmp_sha256)"
        got=$(sha256sum "$TARBALL" | awk '{print $1}')
        if [ "$got" = "$want" ]; then
            printf 'ok   gmp tarball sha256
'
        else
            printf 'DRIFT gmp tarball: 期望 %s…，实际 %s…
' "${want:0:12}" "${got:0:12}"
            fail=$((fail + 1))
        fi
    else
        printf 'MISS gmp tarball: %s
' "$TARBALL"
        fail=$((fail + 1))
    fi
fi

if [ "$fail" -eq 0 ]; then
    echo "依赖一致"
    exit 0
fi
echo "依赖不一致: $fail 项"
exit 1
