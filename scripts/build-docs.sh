#!/usr/bin/env bash
# 生成静态文档站（GitHub Pages 用）：把仓库根目录的 Markdown 转成 site/_out/*.html。
# 依赖: pandoc ≥ 2.x。用法: scripts/build-docs.sh [输出目录]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${1:-$ROOT/site/_out}"
SRC="$ROOT/site"

command -v pandoc >/dev/null || { echo "需要 pandoc（https://pandoc.org/）"; exit 1; }

rm -rf "$OUT"
mkdir -p "$OUT"
cp "$SRC/style.css" "$OUT/style.css"
touch "$OUT/.nojekyll"

cat > "$OUT/_nav.html" <<'NAV'
<nav class="site">
  <a href="index.html">首页</a>
  <a href="requirements.html">需求</a>
  <a href="design.html">设计</a>
  <a href="examples.html">示例</a>
  <a href="faq.html">FAQ</a>
  <a href="platforms.html">平台</a>
  <a href="security.html">安全</a>
  <a href="third-party.html">第三方</a>
  <a href="contributing.html">贡献</a>
  <a href="changelog.html">变更</a>
  <a href="todo.html">待办</a>
  <a href="https://github.com/MarchBeta2087/polyno_err_ana">GitHub</a>
</nav>
NAV

cat > "$OUT/_footer.html" <<'FOOT'
<footer>polyno_err_ana · GPL-3.0-or-later · 本页由 <code>scripts/build-docs.sh</code> 从仓库 Markdown 生成</footer>
FOOT

page() {  # src out title
    local src="$1" out="$2" title="$3"
    pandoc "$src" -f gfm -t html5 --standalone \
        --metadata title="$title" --css style.css \
        --include-before-body "$OUT/_nav.html" \
        --include-after-body "$OUT/_footer.html" \
        -o "$OUT/$out"
    echo "  $out"
}

echo "生成到 $OUT"
page "$ROOT/README.md"           index.html        "polyno_err_ana —— 多项式/有理式误差分析器"
page "$ROOT/需求分析.md"         requirements.html "需求分析"
page "$ROOT/设计.md"             design.html       "设计说明"
page "$ROOT/EXAMPLES.md"         examples.html     "示例与教程"
page "$ROOT/FAQ.md"              faq.html          "FAQ"
page "$ROOT/SECURITY.md"         security.html     "安全策略"
page "$ROOT/THIRD_PARTY.md"      third-party.html  "第三方组件与许可"
page "$ROOT/CONTRIBUTING.md"     contributing.html "贡献指南"
page "$ROOT/CODE_OF_CONDUCT.md"  coc.html          "行为准则"
page "$ROOT/CHANGELOG.md"        changelog.html    "变更记录"
page "$ROOT/TODO.md"             todo.html         "维护者待办"
page "$ROOT/OPEN_TASKS.md"       open-tasks.html   "开放任务（欢迎认领）"
page "$ROOT/PLATFORMS.md"        platforms.html    "平台支持等级"

echo "完成: $OUT/index.html"
