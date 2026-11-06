// SPDX-License-Identifier: GPL-3.0-or-later
// 输出层：表格渲染与各输出块（只接收已格式化好的字符串，不依赖符号/数值层）。
#pragma once

#include <string>
#include <vector>

namespace pna {

// 渲染一张表：head 为表头，rows 为数据行，right[i] 控制第 i 列右对齐；
// 自动插入 `---` 分隔行，行首缩进两个空格，行尾不留空白。
void render_table(const std::vector<std::string>& head,
                  const std::vector<std::vector<std::string>>& rows,
                  const std::vector<bool>& right);

struct VarLine {
    std::string name;    // 变量名
    std::string est;     // 估计点（已格式化）
    std::string dv;      // 实际误差量（已格式化）
    std::string mode;    // "absolute" / "relative"
    std::string note;    // 附加提示（可空）
};

struct PartialLine {
    std::string name;    // 变量名
    std::string expr;    // ∂f/∂v
    std::string dv;      // δv
    std::string ratio;   // δv/v，或空表示估计点为 0
};

// 版本/分支/公式/变量估计点
void print_header(const std::string& version, const std::string& kernel, int bits, int branch,
                  const std::string& formula, const std::vector<VarLine>& vars);

// 符号块：δf、δf/f 及其代入值；partials 为空时表示符号层不可用（打印 N/A 与原因）
void print_symbolic(const std::string& df, const std::string& df_value, const std::string& rel,
                    const std::string& rel_value, const std::vector<PartialLine>& partials);

// 符号不可用时的提示块（变量名逐个列出）
void print_symbolic_unavailable(const std::vector<std::string>& names);

// 精确边界块与极限分类块（表体由调用方组装）
void print_boundary(const std::vector<std::vector<std::string>>& rows);
void print_limits(const std::vector<std::vector<std::string>>& rows);

}  // namespace pna
