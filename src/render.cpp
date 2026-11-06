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

}  // namespace pna
