// SPDX-License-Identifier: GPL-3.0-or-later
// 输出层：列宽对齐的小表渲染（值均为单宽字符，含 δ）。
#pragma once

#include <string>
#include <vector>

namespace pna {

// 渲染一张表：head 为表头，rows 为数据行，right[i] 控制第 i 列右对齐；
// 自动插入 `---` 分隔行，行首缩进两个空格，行尾不留空白。
void render_table(const std::vector<std::string>& head,
                  const std::vector<std::vector<std::string>>& rows,
                  const std::vector<bool>& right);

}  // namespace pna
