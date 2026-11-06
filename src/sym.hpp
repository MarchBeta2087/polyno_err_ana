// 符号层：以“原子表达式”为因子的 Laurent 单项式之和（多项式和）。
// 选择原子而非全展开，是为了让 (1+x)^3/(1+x)^4 自然约分为 (1+x)^-1（R-1）。
#pragma once

#include <string>
#include <vector>

#include "ast.hpp"

namespace pna {

struct Factor {
    Expr atom;
    long exp{1};
};

struct Term {
    Real coeff{};
    std::vector<Factor> fac;
};

struct Sym {
    std::vector<Term> terms;
};

Sym to_sym(const Expr& e);
Sym sym_const(const Real& c);
Sym sym_var(const std::string& name);

Sym sym_add(const Sym& a, const Sym& b);
Sym sym_sub(const Sym& a, const Sym& b);
Sym sym_mul(const Sym& a, const Sym& b);
Sym sym_scale(const Sym& a, const Real& c);
// a / e（尽量把 1/e 分配到因子，使 (1+x)^3 / (1+x)^4 约分为 (1+x)^-1）
Sym sym_div_by(const Sym& a, const Expr& e);
Sym sym_neg(const Sym& a);

Sym diff(const Sym& a, const std::string& var);

Real sym_value(const Sym& a, const Env& env);
bool sym_is_zero(const Sym& a);
std::string sym_show(const Sym& a);

}  // namespace pna
