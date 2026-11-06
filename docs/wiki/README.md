# Wiki 同步说明

GitHub 的 Wiki 是独立 git 仓库（`<repo>.wiki.git`），且**只有先在网页上创建第一页**后才会初始化。
API 不支持创建 Wiki 页面，因此同步流程如下：

1. 打开 <https://github.com/MarchBeta2087/polyno_err_ana/wiki>，点 “Create the first page”，
   标题填 `Home`，内容随意（保存后仓库即存在）；
2. 本地同步本目录内容：

```sh
git clone https://github.com/MarchBeta2087/polyno_err_ana.wiki.git /tmp/wiki
cp docs/wiki/Home.md docs/wiki/_Sidebar.md /tmp/wiki/
cd /tmp/wiki && git add -A && git commit -m "wiki: 初始化首页（指向文档站与仓库）" && git push
```

3. 以后修改：编辑本目录的 Markdown → 重复第 2 步（或直接在 Wiki 网页编辑，再 PR 回本目录保持版本化）。

> 注意：本目录是“版本化的镜像”，Wiki 网页上的临时编辑不会自动回流；以本目录为准。
