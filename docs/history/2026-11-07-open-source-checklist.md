# 开放仓库检查清单

- 日期：2026-11-07
- 状态：**准备就绪，等待维护者把仓库设为 public**

转公开后执行（需要 admin 权限，`gh` 已登录账号 `MarchBeta2087`）：

## 1. 基础设置（一次性）

```sh
R=MarchBeta2087/polyno_err_ana

# 描述 / 主页 / 话题
gh api -X PATCH repos/$R \
  -f description="多项式/有理式误差分析器：符号误差传播、精确边界与极限分类（C++23，GMP/MPFR）" \
  -f homepage="https://marchbeta2087.github.io/polyno_err_ana/" \
  -f has_issues=true -f has_discussions=true

gh api -X PUT repos/$R/topics -f names[]=cpp23 -f names[]=error-analysis \
  -f names[]=interval-arithmetic -f names[]=mpfr -f names[]=gmp \
  -f names[]=symbolic -f names[]=cli
```

## 2. 文档站（Pages）

```sh
# 1) 启用 Pages，来源 = GitHub Actions
gh api -X POST repos/$R/pages -f build_type=workflow

# 2) 打开部署开关（工作流里 deploy 作业的条件）
gh variable set ENABLE_PAGES --body true -R $R

# 3) 触发一次
gh workflow run pages -R $R
```

## 3. 安全

```sh
# 私有漏洞报告（Security Advisory 的“Report a vulnerability”入口）
gh api -X PUT repos/$R/private-vulnerability-reporting

# Dependabot 告警与安全更新
gh api -X PUT repos/$R/vulnerability-alerts
gh api -X PUT repos/$R/automated-security-fixes
```

## 4. 分支保护（main）

```sh
gh api -X PUT repos/$R/branches/main/protection \
  -F required_status_checks.strict=true \
  -f 'required_status_checks.contexts[]=ci / linux' \
  -f 'required_status_checks.contexts[]=ci / windows' \
  -F enforce_admins=false \
  -F required_pull_request_reviews.required_approving_review_count=0 \
  -F restrictions=
```

> 单人维护时 `required_approving_review_count=0` 仍要求走 PR；
> 想直接推 main 可把 `enforce_admins` 设为 `false`（默认允许 admin 绕过）。

## 5. 复核项（公开前已确认）

- [x] 跟踪文件中无绝对路径、无 token/私钥痕迹（`git ls-files | xargs grep` 已扫）
- [x] 许可齐全：`LICENSE.md`（GPL-3.0-or-later）、`THIRD_PARTY.md`（MIT / LGPL 说明）
- [x] 社区文件：`CODE_OF_CONDUCT.md`、`SECURITY.md`、Issue 表单、PR 模板、`CODEOWNERS`
- [x] 文档站源与生成脚本：`site/style.css`、`scripts/build-docs.sh`
- [x] 依赖可复现：`deps.lock` + `fetch-gmp.sh` / `fetch-mpfr.sh`（sha256）
- [x] CI：linux/windows/macos 三平台 + 双内核 + 快照（45 条）
- [ ] 仓库体积：`.git` 约 8.5 MB（曾误提交的 `mpfr-4.2.1/` 已被 force-push 移除；
      如需彻底回收，可在本地 `git gc --prune=now` 后强推，或联系 GitHub 支持）

公开后自动生效的变化：

- `release.yml` 的 macOS 作业（`if: repository.private == false`）开始出 macOS 包；
- Pages 部署在设置 `ENABLE_PAGES=true` 后开始工作；
- Dependabot 会按 `.github/dependabot.yml`每周检查 Actions 版本。
