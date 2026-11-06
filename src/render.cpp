// SPDX-License-Identifier: GPL-3.0-or-later
#include "render.hpp"

#include <algorithm>
#include <cstdio>

namespace pna {

// 列宽对齐的小表：值均为单宽字符（含 δ），right[i] 控制右对齐。
void render_table(const std::vector<std::string>& head,
                  const std::vector<std::vector<std::string>>& rows,
                  const std::vector<bool>& right) {
    const std::size_t n = head.size();
    std::vector<std::size_t> w(n);
    for (std::size_t i = 0; i < n; ++i) w[i] = head[i].size();
    for (const auto& r : rows)
        for (std::size_t i = 0; i < n && i < r.size(); ++i) w[i] = std::max(w[i], r[i].size());

    auto emit = [&](const std::vector<std::string>& cells, char fill) {
        std::string line = "  ";
        for (std::size_t i = 0; i < n; ++i) {
            const std::string c = i < cells.size() ? cells[i] : std::string();
            const std::size_t pad = w[i] - c.size();
            line += fill == ' ' ? (right[i] ? std::string(pad, ' ') + c : c + std::string(pad, ' '))
                                : std::string(w[i], '-');
            if (i + 1 < n) line += "  ";
        }
        while (!line.empty() && line.back() == ' ') line.pop_back();
        std::printf("%s\n", line.c_str());
    };
    emit(head, ' ');
    emit(std::vector<std::string>(n, ""), '-');
    for (const auto& r : rows) emit(r, ' ');
}


void print_header(const std::string& version, const std::string& kernel, int bits, int branch,
                  const std::string& formula, const std::vector<VarLine>& vars) {
    std::printf("polyno_err_ana %s  (%s, %d bit)\n", version.c_str(), kernel.c_str(), bits);
    if (branch != 0) std::printf("branch k = %d\n", branch);
    std::printf("\nf    = %s\n", formula.c_str());
    for (const auto& v : vars) {
        std::printf("%s ≈ %s,  δ%s = %s (%s)%s\n", v.name.c_str(), v.est.c_str(),
                    v.name.c_str(), v.dv.c_str(), v.mode.c_str(), v.note.c_str());
    }
}

void print_symbolic(const std::string& df, const std::string& df_value, const std::string& rel,
                    const std::string& rel_value, const std::vector<PartialLine>& partials) {
    std::printf("\n");
    std::printf("δf    = %s\n", df.c_str());
    std::printf("      = %s\n", df_value.c_str());
    std::printf("δf/f  = %s\n", rel.c_str());
    std::printf("      = %s\n", rel_value.c_str());
    std::printf("\n");
    for (const auto& p : partials) {
        std::printf("∂f/∂%s  = %s\n", p.name.c_str(), p.expr.c_str());
        std::printf("δ%s     = %s\n", p.name.c_str(), p.dv.c_str());
        if (p.ratio.empty())
            std::printf("δ%s/%s   = N/A（估计点为 0）\n", p.name.c_str(), p.name.c_str());
        else
            std::printf("δ%s/%s   = %s\n", p.name.c_str(), p.name.c_str(), p.ratio.c_str());
        std::printf("\n");
    }
}

void print_symbolic_unavailable(const std::vector<std::string>& names) {
    std::printf("\n");
    std::printf("%-9s= N/A（符号展开不可用：非整数指数或项数超限）\n", "δf");
    std::printf("%-9s= N/A\n", "δf/f");
    for (const auto& n : names) std::printf("∂f/∂%s = N/A\n", n.c_str());
    std::printf("\n");
}

void print_boundary(const std::vector<std::vector<std::string>>& rows) {
    std::printf("  精确边界（单变量变，其余固定在估计点）\n\n");
    render_table({"var", "δ_{R+}f", "δ_{R-}f", "δ_{R+}f/f", "δ_{R-}f/f"}, rows,
                 {false, true, true, true, true});
}

void print_limits(const std::vector<std::vector<std::string>>& rows) {
    std::printf("\n  极限分类（单变量变，其余 δ=0）\n\n");
    render_table({"var", "δv →", "δf", "δf/f"}, rows, {false, true, true, true});
}

}  // namespace pna
