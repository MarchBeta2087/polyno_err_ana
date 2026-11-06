// SPDX-License-Identifier: GPL-3.0-or-later
#include "ast.hpp"

#include "i18n.hpp"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <unordered_map>

namespace pna {
namespace {

struct Tok {
    enum K { End, Num, Ident, Plus, Minus, Star, Slash, Caret, LP, RP } k{End};
    std::string text;
    double num{};
    bool text_back_i{false};
    size_t col{};
};

const char* kFuncs[] = {"sin", "cos", "tan", "asin", "acos", "atan", "exp", "log", "ln",
                        "sqrt", "abs", "pow"};

bool is_func(const std::string& s) {
    for (const char* f : kFuncs)
        if (s == f) return true;
    return false;
}

std::vector<Tok> lex(const std::string& s) {
    std::vector<Tok> out;
    size_t i = 0;
    while (i < s.size()) {
        char c = s[i];
        if (std::isspace(static_cast<unsigned char>(c))) {
            ++i;
            continue;
        }
        Tok t;
        t.col = i + 1;
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            size_t b = i;
            while (i < s.size() && (std::isdigit(static_cast<unsigned char>(s[i])) || s[i] == '.')) ++i;
            if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
                size_t save = i;
                ++i;
                if (i < s.size() && (s[i] == '+' || s[i] == '-')) ++i;
                if (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) {
                    while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) ++i;
                } else {
                    i = save;
                }
            }
            t.k = Tok::Num;
            t.text = s.substr(b, i - b);
            t.num = std::strtod(t.text.c_str(), nullptr);
            t.text_back_i = false;
            if (i < s.size() && s[i] == 'i' &&
                (i + 1 == s.size() || (!std::isalnum(static_cast<unsigned char>(s[i + 1])) && s[i + 1] != '_'))) {
                ++i;
                t.text_back_i = true;  // 2i、0.5i
            }
            out.push_back(t);
            continue;
        }
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            size_t b = i;
            while (i < s.size() && (std::isalnum(static_cast<unsigned char>(s[i])) || s[i] == '_')) ++i;
            t.k = Tok::Ident;
            t.text = s.substr(b, i - b);
            out.push_back(t);
            continue;
        }
        ++i;
        switch (c) {
            case '+': t.k = Tok::Plus; break;
            case '-': t.k = Tok::Minus; break;
            case '*': t.k = Tok::Star; break;
            case '/': t.k = Tok::Slash; break;
            case '^': t.k = Tok::Caret; break;
            case '(': t.k = Tok::LP; break;
            case ')': t.k = Tok::RP; break;
            default:
                throw ParseError(std::string(tr("意外字符 '", "unexpected character '")) + c + "'", t.col);
        }
        t.text = std::string(1, c);
        out.push_back(t);
    }
    Tok e;
    e.k = Tok::End;
    e.col = s.size() + 1;
    out.push_back(e);
    return out;
}

struct Parser {
    std::vector<Tok> ts;
    size_t p = 0;
    const Tok& cur() const { return ts[p]; }
    void next() { ++p; }

    Expr expr();     // + -
    Expr term();     // * /
    Expr unary();    // 一元 -
    Expr power();    // ^（右结合）
    Expr atom();

    [[noreturn]] void fail(const std::string& m) { throw ParseError(m, cur().col); }
};

Cplx canon(const Cplx& v);   // -0 虚部归一化，见下方定义
void finish(Node& n);        // 结构哈希，见下方定义

Expr mk(NK k, Expr a, Expr b = nullptr) {
    auto n = std::make_shared<Node>();
    n->k = k;
    n->a = std::move(a);
    n->b = std::move(b);
    n->col = n->a ? n->a->col : 0;
    finish(*n);
    return n;
}

Expr fold(NK k, const Expr& a, const Expr& b) {
    if (a->k == NK::Num && b->k == NK::Num) {
        Cplx r;
        switch (k) {
            case NK::Add: r = a->num + b->num; break;
            case NK::Sub: r = canon(a->num - b->num); break;
            case NK::Mul: r = a->num * b->num; break;
            case NK::Div: r = a->num / b->num; break;
            case NK::Pow: r = c_pow(a->num, b->num); break;
            default: break;
        }
        auto n = std::make_shared<Node>();
        n->k = NK::Num;
        n->num = r;
        n->col = a->col;
        finish(*n);
        return n;
    }
    return mk(k, a, b);
}

Expr Parser::atom() {
    switch (cur().k) {
        case Tok::Num: {
            auto n = std::make_shared<Node>();
            n->k = NK::Num;
            n->num = cur().text_back_i ? c_from_double(0.0, cur().num) : c_from_double(cur().num);
            n->col = cur().col;
            finish(*n);
            next();
            return n;
        }
        case Tok::Ident: {
            std::string name = cur().text;
            size_t col = cur().col;
            next();
            if (is_func(name))
                throw ParseError(tr("不支持的函数: ", "unsupported function: ") + name, col);
            if (!name.empty() && name[0] == '_')
                throw ParseError(tr("非法变量名: ", "illegal variable name: ") + name, col);
            if (cur().k == Tok::LP) throw ParseError(tr("不支持的函数: ", "unsupported function: ") + name, col);
            if (name == "i") {
                auto n = std::make_shared<Node>();
                n->k = NK::Num;
                n->num = c_i();
                n->col = col;
                finish(*n);
                return n;
            }
            auto n = std::make_shared<Node>();
            n->k = NK::Var;
            n->var = name;
            n->col = col;
            finish(*n);
            return n;
        }
        case Tok::LP: {
            next();
            Expr e = expr();
            if (cur().k != Tok::RP) fail(tr("缺少 ')'", "expected ')'"));
            next();
            return e;
        }
        case Tok::End: fail(tr("公式意外结束", "unexpected end of formula"));
        default: fail(std::string(tr("意外标记 '", "unexpected token '")) + cur().text + "'");
    }
}

Expr Parser::power() {
    Expr base = atom();
    if (cur().k == Tok::Caret) {
        next();
        Expr e = unary();  // 右结合：2^-1、a^b^c 均由此递归
        if (has_var(e)) throw ParseError(tr("指数必须是常数", "exponent must be a constant"), e->col);
        return fold(NK::Pow, base, e);
    }
    return base;
}

Expr Parser::unary() {
    if (cur().k == Tok::Minus) {
        size_t col = cur().col;
        next();
        Expr inner = unary();
        if (inner->k == NK::Num) {
            auto n = std::make_shared<Node>();
            n->k = NK::Num;
            n->num = canon(c_neg(inner->num));
            n->col = col;
            finish(*n);
            return n;
        }
        return mk(NK::Sub, mk_num(0.0), inner);
    }
    if (cur().k == Tok::Plus) {
        next();
        return unary();
    }
    return power();
}

Expr Parser::term() {
    Expr e = unary();
    for (;;) {
        if (cur().k == Tok::Star) {
            next();
            e = fold(NK::Mul, e, unary());
        } else if (cur().k == Tok::Slash) {
            next();
            e = fold(NK::Div, e, unary());
        } else {
            return e;
        }
    }
}

Expr Parser::expr() {
    Expr e = term();
    for (;;) {
        if (cur().k == Tok::Plus) {
            next();
            e = fold(NK::Add, e, term());
        } else if (cur().k == Tok::Minus) {
            next();
            e = fold(NK::Sub, e, term());
        } else {
            return e;
        }
    }
}

// ---- 结构哈希（FNV-1a）：用于符号层原子归并，避免反复 to_string ----
inline std::uint64_t fnv(std::uint64_t h, std::uint64_t v) {
    h ^= v;
    h *= 1099511628211ULL;
    return h;
}

std::uint64_t dbits(double d) {
    std::uint64_t u = 0;
    std::memcpy(&u, &d, sizeof u);
    return u;
}

// 归一化 -0 虚部：避免 atan2(±0, x) 导致主值角度符号不同（不同后端会分叉）
Cplx canon(const Cplx& v) {
    Cplx r = v;
    if (is_zero(r.im) && is_neg(r.im)) r.im = Real::zero();
    return r;
}

void finish(Node& n) {
    std::uint64_t h = 1469598103934665603ULL;
    h = fnv(h, static_cast<std::uint64_t>(n.k));
    if (n.k == NK::Num) {
        h = fnv(h, dbits(to_double(n.num.re)));
        h = fnv(h, dbits(to_double(n.num.im)));
    } else if (n.k == NK::Var) {
        for (unsigned char c : n.var) h = fnv(h, c);
    }
    if (n.a) h = fnv(h, n.a->hash);
    if (n.b) h = fnv(h, n.b->hash);
    n.hash = h;
}

int prec(const Expr& e) {
    switch (e->k) {
        case NK::Add:
        case NK::Sub: return 1;
        case NK::Mul:
        case NK::Div: return 2;
        case NK::Pow: return 4;
        default: return 5;
    }
}

void print(const Expr& e, std::string& out, int parent_prec, bool right_side);

void print_child(const Expr& c, std::string& out, int pp, bool right_side) {
    const bool neg_num = c->k == NK::Num && is_neg(c->num.re) && is_zero(c->num.im);
    bool need = neg_num || prec(c) < pp || (right_side && prec(c) == pp && prec(c) <= 2);
    if (need) out += '(';
    print(c, out, pp, right_side);
    if (need) out += ')';
}

void print(const Expr& e, std::string& out, int parent_prec, bool right_side) {
    (void)parent_prec;
    (void)right_side;
    switch (e->k) {
        case NK::Num: out += c_fmt(e->num); return;
        case NK::Var: out += e->var; return;
        case NK::Add:
            print_child(e->a, out, 1, false);
            out += "+";
            print_child(e->b, out, 1, true);
            return;
        case NK::Sub:
            print_child(e->a, out, 1, false);
            out += "-";
            print_child(e->b, out, 1, true);
            return;
        case NK::Mul:
            print_child(e->a, out, 2, false);
            out += "*";
            print_child(e->b, out, 2, true);
            return;
        case NK::Div:
            print_child(e->a, out, 2, false);
            out += "/";
            print_child(e->b, out, 2, true);
            return;
        case NK::Pow:
            print_child(e->a, out, 4, false);
            out += "^";
            print_child(e->b, out, 4, true);
            return;
    }
}

}  // namespace

Expr parse(const std::string& src) {
    Parser ps{lex(src), 0};
    Expr e = ps.expr();
    if (ps.cur().k != Tok::End)
        throw ParseError(std::string(tr("意外的尾部标记 '", "unexpected trailing token '")) + ps.cur().text + "'", ps.cur().col);
    return e;
}

Expr mk_num(double d) {
    auto n = std::make_shared<Node>();
    n->k = NK::Num;
    n->num = c_from_double(d);
    finish(*n);
    return n;
}

Expr mk_var(std::string name) {
    auto n = std::make_shared<Node>();
    n->k = NK::Var;
    n->var = std::move(name);
    finish(*n);
    return n;
}

bool has_var(const Expr& e) {
    if (!e) return false;
    if (e->k == NK::Var) return true;
    return has_var(e->a) || has_var(e->b);
}

void collect_vars(const Expr& e, std::vector<std::string>& out) {
    if (!e) return;
    if (e->k == NK::Var) {
        for (const auto& v : out)
            if (v == e->var) return;
        out.push_back(e->var);
        return;
    }
    collect_vars(e->a, out);
    collect_vars(e->b, out);
}

Cplx const_value(const Expr& e) {
    if (has_var(e)) throw ParseError(tr("指数必须是常数", "exponent must be a constant"), e->col);
    return eval(e, {});
}

const Cplx* lookup(const Env& env, const std::string& name) {
    for (const auto& kv : env)
        if (kv.first == name) return &kv.second;
    return nullptr;
}

Cplx eval(const Expr& e, const Env& env) {
    switch (e->k) {
        case NK::Num: return e->num;
        case NK::Var: {
            const Cplx* v = lookup(env, e->var);
            if (!v) throw EvalError("unbound variable: " + e->var);
            return *v;
        }
        case NK::Add: return eval(e->a, env) + eval(e->b, env);
        case NK::Sub: return eval(e->a, env) - eval(e->b, env);
        case NK::Mul: return eval(e->a, env) * eval(e->b, env);
        case NK::Div: return eval(e->a, env) / eval(e->b, env);
        case NK::Pow: return c_pow(eval(e->a, env), eval(e->b, env));
    }
    throw EvalError("bad node");
}

std::string to_string(const Expr& e) {
    std::string out;
    print(e, out, 0, false);
    return out;
}

std::string fmt_num(double d) {
    if (d == 0.0) return "0";
    char buf[64];
    if (std::fabs(d) >= 1e-4 && std::fabs(d) < 1e6)
        std::snprintf(buf, sizeof buf, "%.6g", d);
    else
        std::snprintf(buf, sizeof buf, "%.4e", d);
    return buf;
}

bool same(const Expr& a, const Expr& b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->k != b->k || a->hash != b->hash) return false;
    if (a->k == NK::Num) return re_eq(a->num.re, b->num.re) && re_eq(a->num.im, b->num.im);
    if (a->k == NK::Var) return a->var == b->var;
    return same(a->a, b->a) && same(a->b, b->b);
}

}  // namespace pna
