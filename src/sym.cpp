#include "sym.hpp"

#include <algorithm>
#include <cmath>

namespace pna {
namespace {

bool atom_eq(const Expr& a, const Expr& b) { return same(a, b); }

void normalize(Term& t) {
    std::vector<Factor> out;
    for (auto& f : t.fac) {
        if (f.exp == 0) continue;
        bool merged = false;
        for (auto& g : out) {
            if (atom_eq(g.atom, f.atom)) {
                g.exp += f.exp;
                merged = true;
                break;
            }
        }
        if (!merged) out.push_back(f);
    }
    out.erase(std::remove_if(out.begin(), out.end(), [](const Factor& f) { return f.exp == 0; }),
              out.end());
    std::sort(out.begin(), out.end(), [](const Factor& x, const Factor& y) {
        return to_string(x.atom) < to_string(y.atom);
    });
    t.fac = std::move(out);
}

bool same_factors(const std::vector<Factor>& a, const std::vector<Factor>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].exp != b[i].exp || !atom_eq(a[i].atom, b[i].atom)) return false;
    return true;
}

void cleanup(Sym& s) {
    std::vector<Term> out;
    for (auto& t : s.terms) {
        if (is_zero(t.coeff)) continue;
        normalize(t);
        bool merged = false;
        for (auto& u : out) {
            if (same_factors(u.fac, t.fac)) {
                u.coeff = u.coeff + t.coeff;
                merged = true;
                break;
            }
        }
        if (!merged) out.push_back(std::move(t));
    }
    out.erase(std::remove_if(out.begin(), out.end(), [](const Term& t) { return is_zero(t.coeff); }),
              out.end());
    s.terms = std::move(out);
}

Sym monomial(const Real& c, std::vector<Factor> fac) {
    Sym s;
    Term t;
    t.coeff = c;
    t.fac = std::move(fac);
    s.terms.push_back(std::move(t));
    cleanup(s);
    return s;
}

// 1/b 的符号表示：能分配就分配到因子，否则整体作为 -1 次原子。
Sym reciprocal(const Expr& b);

Sym single_factor(const Expr& atom, long e) { return monomial(Real::one(), {{atom, e}}); }

Sym reciprocal(const Expr& b) {
    switch (b->k) {
        case NK::Num: return sym_const(Real::one() / b->num);
        case NK::Var: return single_factor(b, -1);
        case NK::Pow: {
            double d = to_double(eval(b->b, {}));
            if (std::isfinite(d) && d == std::floor(d) && std::fabs(d) <= 1e6)
                return single_factor(b->a, -static_cast<long>(d));
            break;
        }
        case NK::Mul: return sym_mul(reciprocal(b->a), reciprocal(b->b));
        case NK::Div: return sym_mul(to_sym(b->a), reciprocal(b->b));
        default: break;
    }
    return single_factor(b, -1);
}

}  // namespace

Sym sym_const(const Real& c) { return monomial(c, {}); }

Sym sym_var(const std::string& name) { return single_factor(mk_var(name), 1); }

Sym to_sym(const Expr& e) {
    switch (e->k) {
        case NK::Num: return sym_const(e->num);
        case NK::Var: return sym_var(e->var);
        case NK::Add: return sym_add(to_sym(e->a), to_sym(e->b));
        case NK::Sub: return sym_sub(to_sym(e->a), to_sym(e->b));
        case NK::Mul: return sym_mul(to_sym(e->a), to_sym(e->b));
        case NK::Div: return sym_mul(to_sym(e->a), reciprocal(e->b));
        case NK::Pow: {
            double d = to_double(eval(e->b, {}));
            if (!std::isfinite(d) || d != std::floor(d) || std::fabs(d) > 1e6)
                throw EvalError("non-integer exponent: symbolic form unsupported");
            long n = static_cast<long>(d);
            if (n == 0) return sym_const(Real::one());
            return single_factor(e->a, n);
        }
    }
    throw EvalError("bad node");
}

Sym sym_add(const Sym& a, const Sym& b) {
    Sym s = a;
    s.terms.insert(s.terms.end(), b.terms.begin(), b.terms.end());
    cleanup(s);
    return s;
}

Sym sym_neg(const Sym& a) { return sym_scale(a, Real::from_double(-1.0)); }

Sym sym_sub(const Sym& a, const Sym& b) { return sym_add(a, sym_neg(b)); }

Sym sym_scale(const Sym& a, const Real& c) {
    Sym s = a;
    for (auto& t : s.terms) t.coeff = t.coeff * c;
    cleanup(s);
    return s;
}

Sym sym_mul(const Sym& a, const Sym& b) {
    Sym s;
    for (const auto& x : a.terms) {
        for (const auto& y : b.terms) {
            Term t;
            t.coeff = x.coeff * y.coeff;
            t.fac = x.fac;
            t.fac.insert(t.fac.end(), y.fac.begin(), y.fac.end());
            s.terms.push_back(std::move(t));
        }
    }
    cleanup(s);
    return s;
}

Sym sym_div_by(const Sym& a, const Expr& e) { return sym_mul(a, reciprocal(e)); }

Sym diff(const Sym& a, const std::string& var) {
    Sym out;
    for (const auto& t : a.terms) {
        if (t.fac.empty()) continue;  // 常数项
        for (size_t i = 0; i < t.fac.size(); ++i) {
            const Factor& f = t.fac[i];
            Sym da;
            if (f.atom->k == NK::Var) {
                if (f.atom->var != var) continue;
                da = sym_const(Real::one());
            } else if (f.atom->k == NK::Num) {
                continue;
            } else {
                da = diff(to_sym(f.atom), var);
            }
            if (sym_is_zero(da)) continue;

            Term nt;
            nt.coeff = t.coeff * Real::from_double(static_cast<double>(f.exp));
            for (size_t j = 0; j < t.fac.size(); ++j) {
                if (j == i) {
                    if (f.exp != 1) nt.fac.push_back({f.atom, f.exp - 1});
                } else {
                    nt.fac.push_back(t.fac[j]);
                }
            }
            Sym mono;
            mono.terms.push_back(std::move(nt));
            out = sym_add(out, sym_mul(mono, da));
        }
    }
    cleanup(out);
    return out;
}

Real sym_value(const Sym& a, const Env& env) {
    Real acc = Real::zero();
    for (const auto& t : a.terms) {
        Real v = t.coeff;
        for (const auto& f : t.fac) {
            Real base = eval(f.atom, env);
            v = v * pow(base, Real::from_double(static_cast<double>(f.exp)));
        }
        acc = acc + v;
    }
    return acc;
}

bool sym_is_zero(const Sym& a) {
    for (const auto& t : a.terms)
        if (!is_zero(t.coeff)) return false;
    return true;
}

namespace {
std::string catom(const Expr& e) {
    std::string s = to_string(e);
    if (e->k == NK::Var || e->k == NK::Num) return s;
    return "(" + s + ")";
}
}  // namespace

std::string sym_show(const Sym& a) {
    if (a.terms.empty()) return "0";
    std::string out;
    bool first = true;
    for (const auto& t : a.terms) {
        const bool negt = is_neg(t.coeff);
        Real c = abs(t.coeff);
        if (!first) out += negt ? " - " : " + ";
        else if (negt) out += "-";
        first = false;

        std::vector<Factor> num, den;
        for (const auto& f : t.fac) {
            Factor g{f.atom, f.exp > 0 ? f.exp : -f.exp};
            (f.exp > 0 ? num : den).push_back(g);
        }

        bool unit = is_zero(c - Real::one());
        if (!(unit && !num.empty())) out += fmt(c);

        std::string prev;
        for (const auto& f : num) {
            std::string s = catom(f.atom);
            if (f.atom->k == NK::Var && !prev.empty()) out += "*";
            out += s;
            if (f.exp != 1) out += "^" + std::to_string(f.exp);
            prev = f.atom->k == NK::Var ? "v" : "e";
        }
        if (!den.empty()) {
            out += "/";
            if (den.size() == 1) {
                out += catom(den[0].atom);
                if (den[0].exp != 1) out += "^" + std::to_string(den[0].exp);
            } else {
                out += "(";
                for (size_t i = 0; i < den.size(); ++i) {
                    if (i) out += "*";
                    out += catom(den[i].atom);
                    if (den[i].exp != 1) out += "^" + std::to_string(den[i].exp);
                }
                out += ")";
            }
        }
    }
    return out;
}

}  // namespace pna
