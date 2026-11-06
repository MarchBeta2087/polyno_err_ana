#include "sym.hpp"

#include <algorithm>
#include <cmath>

namespace pna {
namespace {

bool atom_eq(const Expr& a, const Expr& b) { return same(a, b); }

void normalize(Term& t) {
    std::vector<Factor> out;
    for (auto& f : t.fac) {
        if (c_is_zero(f.exp)) continue;
        bool merged = false;
        for (auto& g : out) {
            if (atom_eq(g.atom, f.atom)) {
                g.exp = g.exp + f.exp;
                merged = true;
                break;
            }
        }
        if (!merged) out.push_back(f);
    }
    out.erase(std::remove_if(out.begin(), out.end(), [](const Factor& f) { return c_is_zero(f.exp); }),
              out.end());
    std::sort(out.begin(), out.end(), [](const Factor& x, const Factor& y) {
        return to_string(x.atom) < to_string(y.atom);
    });
    t.fac = std::move(out);
}

bool same_factors(const std::vector<Factor>& a, const std::vector<Factor>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (!c_is_zero(a[i].exp - b[i].exp) || !atom_eq(a[i].atom, b[i].atom)) return false;
    return true;
}

void cleanup(Sym& s) {
    std::vector<Term> out;
    for (auto& t : s.terms) {
        if (c_is_zero(t.coeff)) continue;
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
    out.erase(std::remove_if(out.begin(), out.end(), [](const Term& t) { return c_is_zero(t.coeff); }),
              out.end());
    s.terms = std::move(out);
}

Sym monomial(const Cplx& c, std::vector<Factor> fac) {
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

Sym single_factor(const Expr& atom, const Cplx& e) { return monomial(c_one(), {{atom, e}}); }

Sym reciprocal(const Expr& b) {
    switch (b->k) {
        case NK::Num: return sym_const(c_one() / b->num);
        case NK::Var: return single_factor(b, c_from_double(-1.0));
        case NK::Pow: {
            Cplx ex = eval(b->b, {});
            if (c_is_finite(ex)) return single_factor(b->a, c_neg(ex));
            break;
        }
        case NK::Mul: return sym_mul(reciprocal(b->a), reciprocal(b->b));
        case NK::Div: return sym_mul(to_sym(b->a), reciprocal(b->b));
        default: break;
    }
    return single_factor(b, c_from_double(-1.0));
}

}  // namespace

Sym sym_const(const Cplx& c) { return monomial(c, {}); }

Sym sym_var(const std::string& name) { return single_factor(mk_var(name), c_one()); }

Sym to_sym(const Expr& e) {
    switch (e->k) {
        case NK::Num: return sym_const(e->num);
        case NK::Var: return sym_var(e->var);
        case NK::Add: return sym_add(to_sym(e->a), to_sym(e->b));
        case NK::Sub: return sym_sub(to_sym(e->a), to_sym(e->b));
        case NK::Mul: return sym_mul(to_sym(e->a), to_sym(e->b));
        case NK::Div: return sym_mul(to_sym(e->a), reciprocal(e->b));
        case NK::Pow: {
            Cplx ex = eval(e->b, {});
            if (!c_is_finite(ex)) throw EvalError("non-finite exponent: symbolic form unsupported");
            if (c_is_zero(ex)) return sym_const(c_one());
            return single_factor(e->a, ex);  // 任意常指数：保留为原子幂，见 §4.1
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

Sym sym_neg(const Sym& a) { return sym_scale(a, c_from_double(-1.0)); }

Sym sym_sub(const Sym& a, const Sym& b) { return sym_add(a, sym_neg(b)); }

Sym sym_scale(const Sym& a, const Cplx& c) {
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
                da = sym_const(c_one());
            } else if (f.atom->k == NK::Num) {
                continue;
            } else {
                da = diff(to_sym(f.atom), var);
            }
            if (sym_is_zero(da)) continue;

            Term nt;
            nt.coeff = t.coeff * f.exp;
            for (size_t j = 0; j < t.fac.size(); ++j) {
                if (j == i) {
                    Cplx em1 = f.exp - c_one();
                    if (!c_is_zero(em1)) nt.fac.push_back({f.atom, em1});
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

Cplx sym_value(const Sym& a, const Env& env) {
    Cplx acc = c_zero();
    for (const auto& t : a.terms) {
        Cplx v = t.coeff;
        for (const auto& f : t.fac) {
            Cplx base = eval(f.atom, env);
            v = v * c_pow(base, f.exp);
        }
        acc = acc + v;
    }
    return acc;
}

bool sym_is_zero(const Sym& a) {
    for (const auto& t : a.terms)
        if (!c_is_zero(t.coeff)) return false;
    return true;
}

namespace {
std::string exp_text(const Cplx& e) {
    if (c_is_real(e) && is_integer(e.re)) {
        const long n = static_cast<long>(to_double(e.re));
        if (n == 1) return "";
        return "^" + std::to_string(n);
    }
    return "^" + c_fmt(e);
}

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
        const bool real_coeff = c_is_real(t.coeff);
        const bool negt = real_coeff && is_neg(t.coeff.re);
        Real c = real_coeff ? abs(t.coeff.re) : c_abs(t.coeff);
        if (!first) out += negt ? " - " : " + ";
        else if (negt) out += "-";
        first = false;

        std::vector<Factor> num, den;
        for (const auto& f : t.fac) {
            const bool pos = sign(f.exp.re) >= 0;
            Factor g{f.atom, pos ? f.exp : c_neg(f.exp)};
            (pos ? num : den).push_back(g);
        }

        const bool unit = real_coeff && is_zero(c - Real::one());
        if (!(unit && !num.empty())) {
            if (real_coeff) out += fmt(c);
            else out += c_fmt(t.coeff);
        }

        std::string prev;
        for (const auto& f : num) {
            std::string s = catom(f.atom);
            if (f.atom->k == NK::Var && !prev.empty()) out += "*";
            out += s;
            out += exp_text(f.exp);
            prev = f.atom->k == NK::Var ? "v" : "e";
        }
        if (!den.empty()) {
            out += "/";
            if (den.size() == 1) {
                out += catom(den[0].atom);
                out += exp_text(den[0].exp);
            } else {
                out += "(";
                for (size_t i = 0; i < den.size(); ++i) {
                    if (i) out += "*";
                    out += catom(den[i].atom);
                    out += exp_text(den[i].exp);
                }
                out += ")";
            }
        }
    }
    return out;
}

}  // namespace pna
