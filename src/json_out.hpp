// SPDX-License-Identifier: GPL-3.0-or-later
// JSON 输出（机器可读）：所有数值以字符串保留，避免二次格式化。
#pragma once

#include <string>
#include <vector>

namespace pna {

struct JsonVar {
    std::string name, estimate, delta, mode;
};
struct JsonPartial {
    std::string var, expr, delta;
};
struct JsonBoundary {
    std::string var, r_plus, r_minus, r_plus_rel, r_minus_rel;
};
struct JsonLimit {
    std::string var, state, df, df_f;
};

struct JsonDoc {
    std::string version, kernel, formula, mode;
    int precision = 256;
    int branch = 0;
    bool symbolic_available = false;
    std::string df, df_value, df_f, df_f_value;
    std::vector<JsonVar> vars;
    std::vector<JsonPartial> partials;
    std::vector<JsonBoundary> boundary;
    std::vector<JsonLimit> limits;
};

std::string to_json(const JsonDoc& doc);

}  // namespace pna
