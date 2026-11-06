// SPDX-License-Identifier: GPL-3.0-or-later
#include "json_out.hpp"

#include <cstdio>

namespace pna {
namespace {

std::string jstr(const std::string& s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof buf, "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    out += "\"";
    return out;
}

std::string kv(const char* key, const std::string& value) { return jstr(key) + ": " + jstr(value); }
std::string kvn(const char* key, int value) { return jstr(key) + ": " + std::to_string(value); }
std::string kvb(const char* key, bool value) {
    return jstr(key) + ": " + (value ? "true" : "false");
}

}  // namespace

std::string to_json(const JsonDoc& d) {
    std::string o;
    o += "{\n";
    o += "  " + kv("version", d.version) + ",\n";
    o += "  " + kv("kernel", d.kernel) + ",\n";
    o += "  " + kvn("precision", d.precision) + ",\n";
    o += "  " + kvn("branch", d.branch) + ",\n";
    o += "  " + kv("formula", d.formula) + ",\n";
    o += "  " + kv("mode", d.mode) + ",\n";

    o += "  \"vars\": [";
    for (size_t i = 0; i < d.vars.size(); ++i) {
        const auto& v = d.vars[i];
        o += i ? ",\n" : "\n";
        o += "    {" + kv("name", v.name) + ", " + kv("estimate", v.estimate) + ", " +
             kv("delta", v.delta) + ", " + kv("mode", v.mode) + "}";
    }
    o += d.vars.empty() ? "],\n" : "\n  ],\n";

    o += "  \"symbolic\": {\n";
    o += "    " + kvb("available", d.symbolic_available) + ",\n";
    o += "    " + kv("df", d.df) + ",\n";
    o += "    " + kv("df_value", d.df_value) + ",\n";
    o += "    " + kv("df_f", d.df_f) + ",\n";
    o += "    " + kv("df_f_value", d.df_f_value) + ",\n";
    o += "    \"partials\": [";
    for (size_t i = 0; i < d.partials.size(); ++i) {
        const auto& p = d.partials[i];
        o += i ? ",\n" : "\n";
        o += "      {" + kv("var", p.var) + ", " + kv("expr", p.expr) + ", " + kv("delta", p.delta) +
             "}";
    }
    o += d.partials.empty() ? "]\n" : "\n    ]\n";
    o += "  },\n";

    o += "  \"boundary\": [";
    for (size_t i = 0; i < d.boundary.size(); ++i) {
        const auto& b = d.boundary[i];
        o += i ? ",\n" : "\n";
        o += "    {" + kv("var", b.var) + ", " + kv("r_plus", b.r_plus) + ", " +
             kv("r_minus", b.r_minus) + ", " + kv("r_plus_rel", b.r_plus_rel) + ", " +
             kv("r_minus_rel", b.r_minus_rel) + "}";
    }
    o += d.boundary.empty() ? "],\n" : "\n  ],\n";

    o += "  \"limits\": [";
    for (size_t i = 0; i < d.limits.size(); ++i) {
        const auto& r = d.limits[i];
        o += i ? ",\n" : "\n";
        o += "    {" + kv("var", r.var) + ", " + kv("state", r.state) + ", " + kv("df", r.df) +
             ", " + kv("df_f", r.df_f) + "}";
    }
    o += d.limits.empty() ? "]\n" : "\n  ]\n";
    o += "}\n";
    return o;
}

}  // namespace pna
