// SPDX-License-Identifier: GPL-3.0-or-later
// polyno_err_ana —— 多项式/有理式误差分析器
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "ast.hpp"
#include "num.hpp"
#include "json_out.hpp"
#include "i18n.hpp"
#include "json_schema.hpp"
#include "render.hpp"
#include "license_text.hpp"
#include "sym.hpp"

using namespace pna;

namespace {

constexpr const char* kVersion = "0.15.2-m14";

struct Var {
    std::string name;
    Cplx err{};              // 实部 = 误差（实数变量即全部）
    bool complex_var = false;
};

enum class Mode { Absolute, Relative };

enum class LimType { Finite, PosZero, NegZero, PosInf, NegInf, CZero, CInf, Undefined };

const char* lim_label(LimType t) {
    switch (t) {
        case LimType::Finite: return "fin";
        case LimType::PosZero: return "+0";
        case LimType::NegZero: return "-0";
        case LimType::PosInf: return "+inf";
        case LimType::NegInf: return "-inf";
        case LimType::CZero: return "c0";
        case LimType::CInf: return "cinf";
        case LimType::Undefined: return "undef";
    }
    return "?";
}

struct State {
    bool to_zero;
    bool plus;
};

const char* state_label(const State& s) {
    if (s.to_zero) return s.plus ? "+0" : "-0";
    return s.plus ? "+inf" : "-inf";
}

// 实数幂律：g ~ a·δ^k
LimType power_limit(int sgn_a, double k, const State& s) {
    if (k == 0.0) return LimType::Finite;
    const double r = std::round(k);
    const bool integer = std::fabs(k - r) < 1e-9;
    const bool odd = integer && (static_cast<long>(std::llround(k)) % 2 != 0);
    if (!s.plus && !integer) return LimType::Undefined;  // δ<0 且指数非整数
    bool goes_zero = (k > 0) ? s.to_zero : !s.to_zero;
    int sg = sgn_a;
    if (!s.plus && odd) sg = -sg;
    if (goes_zero) return sg > 0 ? LimType::PosZero : LimType::NegZero;
    return sg > 0 ? LimType::PosInf : LimType::NegInf;
}

// 复数（模）幂律：只区分 CZero / CInf
LimType power_limit_c(double k, const State& s) {
    if (k == 0.0) return LimType::Finite;
    bool goes_zero = (k > 0) ? s.to_zero : !s.to_zero;
    return goes_zero ? LimType::CZero : LimType::CInf;
}

struct Taylor {
    std::vector<Cplx> c;   // c[n] = f^(n)(v)/n!
    bool nonfinite = false;
};

Taylor taylor(const Sym& f, const Env& env, const std::string& var, int n_max) {
    Taylor t;
    Sym cur = f;
    Real fact = Real::one();
    for (int n = 0; n <= n_max; ++n) {
        Cplx v = sym_value(cur, env);
        if (!c_is_finite(v)) t.nonfinite = true;
        t.c.push_back(v);  // 已除以 n!（见下方 fact 更新）
        t.c.back() = t.c.back() / Cplx{fact, Real::zero()};
        cur = diff(cur, var);
        fact = fact * Real::from_double(static_cast<double>(n + 1));
        if (sym_is_zero(cur)) break;
    }
    return t;
}

int first_nonzero(const Taylor& t, int from) {
    for (int n = from; n < static_cast<int>(t.c.size()); ++n)
        if (!c_is_zero(t.c[static_cast<size_t>(n)])) return n;
    return -1;
}

// 数值幂律探测（符号层不可用时）
struct Probe {
    bool ok = false;
    double k = 0;
    int sgn = 0;
};

Probe probe_power(const Expr& f, const Env& env, const std::string& nm, const State& st) {
    const double h1 = st.to_zero ? 1e-2 : 1e2;
    const double h2 = st.to_zero ? 1e-4 : 1e4;
    auto g = [&](double h) -> Cplx {
        Env e = env;
        for (auto& kv : e)
            if (kv.first == nm)
                kv.second = kv.second + Cplx{Real::from_double(st.plus ? h : -h), Real::zero()};
        return eval(f, e) - eval(f, env);
    };
    Cplx a = g(h1), b = g(h2);
    if (!c_is_finite(a) || !c_is_finite(b) || c_is_zero(a) || c_is_zero(b)) return {};
    const double da = to_double(c_abs(a)), db = to_double(c_abs(b));
    if (!(da > 0) || !(db > 0)) return {};
    Probe p;
    p.k = std::log(da / db) / std::log(h1 / h2);
    if (!std::isfinite(p.k)) return {};
    if (std::fabs(p.k) < 1e-3) p.k = 0.0;
    if (std::fabs(p.k) >= 1e-9) p.k = std::round(p.k * 2.0) / 2.0;
    p.sgn = c_is_real(b) ? sign(b.re) : 1;
    p.ok = true;
    return p;
}

struct LimSpec {
    std::string var;
    std::string text;   // zeros / infs / all / p0 / 0..inf / A..B ...
};

// 探测 f 自身在 δ→0/∞ 时的幂律阶（用于 |f(v)|=0 时的相对形式 K = k_g − k_f）。
Probe probe_magnitude(const Expr& f, const Env& env, const std::string& nm, const State& st) {
    const double h1 = st.to_zero ? 1e-2 : 1e2;
    const double h2 = st.to_zero ? 1e-4 : 1e4;
    auto val = [&](double h) -> Cplx {
        Env e = env;
        for (auto& kv : e)
            if (kv.first == nm)
                kv.second = kv.second + Cplx{Real::from_double(st.plus ? h : -h), Real::zero()};
        return eval(f, e);
    };
    Cplx a = val(h1), b = val(h2);
    if (!c_is_finite(a) || !c_is_finite(b) || c_is_zero(a) || c_is_zero(b)) return {};
    const double da = to_double(c_abs(a)), db = to_double(c_abs(b));
    if (!(da > 0) || !(db > 0)) return {};
    Probe p;
    p.k = std::log(da / db) / std::log(h1 / h2);
    if (!std::isfinite(p.k)) return {};
    if (std::fabs(p.k) < 1e-3) p.k = 0.0;
    if (std::fabs(p.k) >= 1e-9) p.k = std::round(p.k * 2.0) / 2.0;
    p.sgn = c_is_real(b) ? sign(b.re) : 1;
    p.ok = true;
    return p;
}

struct VarView {
    const Var* decl;
    Cplx dv;
    bool degraded{};
};

struct Opts {
    std::string formula;
    std::vector<Var> vars;
    std::vector<std::pair<std::string, Cplx>> est;
    Mode mode = Mode::Absolute;
    bool vars_out = false;
    bool json = false;
    bool json_schema = false;
    pna::Lang lang = pna::Lang::Zh;
    int precision = 256;
    int branch = 0;
    bool real_root = false;
    std::size_t max_terms = 200000;
    bool lim_all = false;
    std::vector<LimSpec> lim_specs;   // --lim VAR:STATE（可多次，同一变量可多状态）
};

[[noreturn]] void usage_error(const std::string& m) {
    throw std::runtime_error(tr_message("命令行错误: " + m));
}

Cplx parse_num(const std::string& s) {
    try {
        return c_from_str(s.c_str());
    } catch (const NumError&) {
        throw std::runtime_error(tr_message("命令行错误: 无法解析数值: " + s));
    }
}

// --estimated 的 VALUE 走公式常量语法（支持 1+2i）
Cplx parse_value(const std::string& s) {
    try {
        return const_value(parse(s));
    } catch (const ParseError& e) {
        char buf[512];
        std::snprintf(buf, sizeof buf,
                      tr("命令行错误: --estimated 值非法（列 %zu）: %s",
                         "command line error: --estimated invalid value (column %zu): %s"),
                      e.col, e.what());
        throw std::runtime_error(buf);
    }
}

void parse_lim_state(const std::string& s, State& st) {
    if (s == "p0" || s == "+0") st = {true, true};
    else if (s == "n0" || s == "-0") st = {true, false};
    else if (s == "pinf" || s == "+inf") st = {false, true};
    else if (s == "ninf" || s == "-inf") st = {false, false};
    else if (s == "0") st = {true, true};
    else if (s == "inf") st = {false, true};
    else usage_error("非法的 --lim 状态: " + s);
}

// 展开 --lim 的状态规格：单个状态、zeros/infs/all、以及 A..B 区间。
std::vector<State> expand_lim(const std::string& text, bool cvar) {
    static const State real_all[4] = {{true, true}, {true, false}, {false, true}, {false, false}};
    static const char* real_name[4] = {"p0", "n0", "pinf", "ninf"};
    const std::vector<State> all = cvar ? std::vector<State>{{true, true}, {false, true}}
                                       : std::vector<State>(real_all, real_all + 4);

    auto idx_of = [&](const std::string& t) -> int {
        if (cvar) {
            if (t == "0") return 0;
            if (t == "inf") return 1;
            if (t == "p0" || t == "n0") return 0;      // 复数别名
            if (t == "pinf" || t == "ninf") return 1;
            return -1;
        }
        for (int i = 0; i < 4; ++i)
            if (t == real_name[i]) return i;
        if (t == "0") return 0;      // 区间语境：0 == 零点组起点
        if (t == "inf") return 3;    // 区间语境：inf == 无穷组终点
        return -1;
    };

    if (text == "all") return all;
    if (text == "zeros") return cvar ? std::vector<State>{{true, true}}
                                     : std::vector<State>{{true, true}, {true, false}};
    if (text == "infs") return cvar ? std::vector<State>{{false, true}}
                                    : std::vector<State>{{false, true}, {false, false}};
    auto dots = text.find("..");
    if (dots != std::string::npos) {
        const std::string a = text.substr(0, dots);
        const std::string b = text.substr(dots + 2);
        int ia = idx_of(a), ib = idx_of(b);
        if (ia < 0 || ib < 0) usage_error("--lim 区间端点非法: " + text);
        if (ia > ib) std::swap(ia, ib);
        std::vector<State> out;
        for (int i = ia; i <= ib; ++i) out.push_back(all[static_cast<size_t>(i)]);
        return out;
    }
    int i = idx_of(text);
    if (i < 0) usage_error("--lim 状态非法: " + text);
    return {all[static_cast<size_t>(i)]};
}

bool is_number(const char* s) {
    if (!s || !*s) return false;
    char* end = nullptr;
    std::strtod(s, &end);
    return end && *end == 0;
}

Opts parse_args(int argc, char** argv) {
    Opts o;
    // 先扫一遍 -p：让后续 -R/--estimated 的字面量也按目标精度解析
    for (int i = 1; i + 1 < argc; ++i) {
        const std::string a = argv[i];
        if (a == "-p" || a == "--precision") {
            try {
                const int bits = std::stoi(argv[i + 1]);
                if (bits >= 2) set_precision(bits);
            } catch (const std::exception&) {
            }
        }
    }
    std::vector<std::string> pend_real, pend_comp;
    bool comp_pending = false;
    auto need = [&](int& i, const char* name) -> std::string {
        if (i + 1 >= argc) usage_error(std::string(name) + " 缺少参数");
        return argv[++i];
    };
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a.size() && a[0] != '-') || is_number(a.c_str())) {
            (comp_pending ? pend_comp : pend_real).push_back(a);
            continue;
        }
        if (a == "-f" || a == "--formula") {
            o.formula = need(i, "-f");
        } else if (a == "-R" || a == "--real") {
            comp_pending = false;
        } else if (a == "-C" || a == "--comp") {
            comp_pending = true;
        } else if (a == "-a" || a == "--absolute") {
            if (o.mode == Mode::Relative) usage_error("-a 与 -r 冲突");
            o.mode = Mode::Absolute;
        } else if (a == "-r" || a == "--relative") {
            o.mode = Mode::Relative;
        } else if (a == "--max-terms") {
            try {
                long v = std::stol(need(i, "--max-terms"));
                if (v < 1) usage_error("--max-terms 必须 >= 1");
                o.max_terms = static_cast<std::size_t>(v);
            } catch (const std::exception&) {
                usage_error("--max-terms 需要正整数");
            }
        } else if (a == "--real-root") {
            o.real_root = true;
        } else if (a == "--lang") {
            std::string v = need(i, "--lang");
            if (!parse_lang(v, o.lang)) usage_error("--lang 只支持 zh / en / ja / ko / es / pt");
        } else if (a == "--json-schema") {
            o.json_schema = true;
        } else if (a == "--json") {
            o.json = true;
        } else if (a == "--vars") {
            o.vars_out = true;
        } else if (a == "-p" || a == "--precision") {
            try {
                o.precision = std::stoi(need(i, "-p"));
            } catch (const std::exception&) {
                usage_error("-p 需要整数");
            }
            if (o.precision < 2) usage_error("-p 必须 >= 2");
        } else if (a == "-k" || a == "--branch") {
            o.branch = std::stoi(need(i, "-k"));
        } else if (a == "--estimated") {
            std::string kv = need(i, "--estimated");
            auto eq = kv.find('=');
            if (eq == std::string::npos) usage_error("--estimated 需要 VARNAME=VALUE");
            o.est.emplace_back(kv.substr(0, eq), parse_value(kv.substr(eq + 1)));
        } else if (a == "--lim") {
            std::string v = need(i, "--lim");
            if (v == "all") {
                o.lim_all = true;
            } else {
                // 允许 "x:p0,y:ninf" 或多次 --lim
                std::string rest = v;
                while (!rest.empty()) {
                    auto comma = rest.find(',');
                    std::string item = rest.substr(0, comma);
                    rest = comma == std::string::npos ? std::string() : rest.substr(comma + 1);
                    if (item.empty()) continue;
                    auto c = item.find(':');
                    if (c == std::string::npos) usage_error("--lim 需要 all 或 VAR:STATE");
                    LimSpec sp;
                    sp.var = item.substr(0, c);
                    sp.text = item.substr(c + 1);
                    if (sp.text.empty()) usage_error("--lim 缺少状态: " + item);
                    o.lim_specs.push_back(sp);
                }
            }
        } else if (a == "--kernel") {
            std::string k = need(i, "--kernel");
            if (k != kernel_name())
            {
                char buf[256];
                std::snprintf(buf, sizeof buf,
                              tr("当前构建的内核是 %s，不支持 --kernel %s",
                                 "this build uses kernel %s, which does not support --kernel %s"),
                              kernel_name(), k.c_str());
                usage_error(buf);
            }
        } else if (a == "-h" || a == "--help") {
            std::fputs(pna::help_text(), stdout);
            std::exit(0);
        } else if (a == "-V" || a == "--version") {
            std::printf("polyno_err_ana %s (kernel: %s, %d bit)\n", kVersion, kernel_name(), o.precision);
            std::exit(0);
        } else if (a == "-G" || a == "--gpl") {
            std::fputs(pna::kLicenseText, stdout);
            std::fputs("\n", stdout);
            std::exit(0);
        } else {
            usage_error("未知选项: " + a);
        }
    }
    if (pend_real.size() % 2 != 0) usage_error("-R 需要成对的 VARNAME ERR");
    for (size_t k = 0; k + 1 < pend_real.size(); k += 2) {
        Var v;
        v.name = pend_real[k];
        v.err = Cplx{parse_num(pend_real[k + 1]).re, Real::zero()};
        o.vars.push_back(std::move(v));
    }
    if (pend_comp.size() % 3 != 0) usage_error("-C 需要成组的 VARNAME RE_ERR IM_ERR");
    for (size_t k = 0; k + 2 < pend_comp.size(); k += 3) {
        Var v;
        v.name = pend_comp[k];
        v.err = Cplx{parse_num(pend_comp[k + 1]).re, parse_num(pend_comp[k + 2]).re};
        v.complex_var = true;
        o.vars.push_back(std::move(v));
    }

    if (o.json_schema) return o;   // --json-schema 不需要公式/变量
    if (o.formula.empty()) usage_error("缺少 -f FORMULA");
    if (o.vars.empty()) usage_error("至少需要一个 -R 或 -C 变量");
    for (size_t i = 0; i < o.vars.size(); ++i)
        for (size_t j = i + 1; j < o.vars.size(); ++j)
            if (o.vars[i].name == o.vars[j].name) usage_error("变量重复声明: " + o.vars[i].name);
    if (o.lim_all && !o.lim_specs.empty()) usage_error("--lim all 与 --lim VAR:STATE 冲突");
    if (o.real_root && o.branch != 0) usage_error("--real-root 与 -k 互斥");
    for (const auto& sp : o.lim_specs) {
        bool found = false;
        for (const auto& v : o.vars) found = found || v.name == sp.var;
        if (!found) usage_error("--lim 引用了未声明的变量: " + sp.var);
    }
    return o;
}

Env make_env(const Opts& o) {
    Env env;
    for (const auto& v : o.vars) {
        auto it = std::find_if(o.est.begin(), o.est.end(),
                               [&](const auto& kv) { return kv.first == v.name; });
        env.emplace_back(v.name, it == o.est.end() ? c_zero() : it->second);
    }
    return env;
}

Cplx value_of(const Env& env, const std::string& name) {
    const Cplx* p = lookup(env, name);
    return p ? *p : c_zero();
}

}  // namespace

int main(int argc, char** argv) {
    try {
        for (int i = 1; i + 1 < argc; ++i) {
            if (std::string(argv[i]) != "--lang") continue;
            Lang l;
            if (parse_lang(argv[i + 1], l)) set_lang(l);
        }
        Opts o = parse_args(argc, argv);
        set_lang(o.lang);
        if (o.json_schema) {
            std::fputs(pna::kJsonSchema, stdout);
            return 0;
        }
        set_precision(o.precision);
        set_branch(o.branch);
        set_real_root(o.real_root);
        sym_set_max_terms(o.max_terms);

        Expr f = parse(o.formula);
        Env env = make_env(o);

        std::vector<std::string> fvars;
        collect_vars(f, fvars);
        for (const auto& v : fvars) {
            bool declared = false;
            for (const auto& d : o.vars) declared = declared || d.name == v;
            if (!declared) usage_error("公式变量未声明: " + v);
        }

        Sym fsym;
        bool sym_ok = true;
        try {
            fsym = to_sym(f);
        } catch (const EvalError&) {
            sym_ok = false;
        }

        std::vector<VarView> views;
        for (const auto& v : o.vars) {
            Cplx est = value_of(env, v.name);
            VarView vv{&v, v.err, false};
            if (o.mode == Mode::Relative) {
                Real m = c_abs(est);
                if (is_zero(m)) {
                    vv.dv = c_zero();
                    vv.degraded = true;
                } else {
                    vv.dv = c_scale(v.err, m);
                }
            }
            views.push_back(vv);
        }

        std::vector<VarLine> var_lines;
        for (const auto& vv : views) {
            VarLine vl;
            vl.name = vv.decl->name;
            vl.est = c_fmt(value_of(env, vv.decl->name));
            vl.dv = c_fmt(vv.dv);
            vl.mode = o.mode == Mode::Absolute ? "absolute" : "relative";
            vl.note = vv.degraded
                          ? tr("  [δv = 0（估计点为 0，相对误差失效）]",
                               "  [δv = 0: estimate is 0, relative error not applicable]")
                          : "";
            var_lines.push_back(std::move(vl));
        }

        std::vector<Sym> partials;
        Sym delta = sym_const(c_zero());
        Sym rel;
        if (sym_ok) {
            try {
                for (const auto& vv : views) {
                    Sym p = diff(fsym, vv.decl->name);
                    partials.push_back(p);
                    delta = sym_add(delta, sym_scale(p, vv.dv));
                }
                rel = sym_div_by(delta, f);
            } catch (const EvalError&) {
                // 求导/相乘阶段也可能触发项数上限：整体退化为数值探测
                sym_ok = false;
                partials.clear();
                delta = sym_const(c_zero());
                rel = Sym{};
            }
        }

        std::vector<PartialLine> plines;
        std::string df_text, df_value, rel_text, rel_value;
        if (o.vars_out && sym_ok) {
            for (size_t i = 0; i < views.size(); ++i) {
                const auto& vv = views[i];
                Cplx est = value_of(env, vv.decl->name);
                PartialLine pl;
                pl.name = vv.decl->name;
                pl.expr = sym_show(partials[i]);
                pl.dv = c_fmt(vv.dv);
                pl.ratio = c_is_zero(est) ? std::string() : c_fmt(vv.dv / est);
                plines.push_back(std::move(pl));
            }
            df_text = sym_show(delta);
            df_value = c_fmt(sym_value(delta, env));
            rel_text = sym_show(rel);
            rel_value = c_fmt(sym_value(rel, env));
        }

        // 精确边界（§3.4）
        Cplx f0 = eval(f, env);
        Real f0abs = c_abs(f0);
        std::vector<std::vector<std::string>> brow;
        for (const auto& vv : views) {
            Env ep = env, em = env;
            for (auto& kv : ep)
                if (kv.first == vv.decl->name) kv.second = kv.second + vv.dv;
            for (auto& kv : em)
                if (kv.first == vv.decl->name) kv.second = kv.second - vv.dv;
            Real dp = c_abs(eval(f, ep) - f0), dm = c_abs(eval(f, em) - f0);
            auto cell = [](const Real& x) { return c_fmt(Cplx{x, Real::zero()}); };
            std::string rp = "N/A", rm = "N/A";
            if (!is_zero(f0abs)) {
                rp = cell(dp / f0abs);
                rm = cell(dm / f0abs);
            }
            brow.push_back({vv.decl->name, cell(dp), cell(dm), rp, rm});
        }

        // 极限分类（§3.6）
        std::vector<std::vector<std::string>> lrow;
        for (const auto& vv : views) {
            const std::string& nm = vv.decl->name;
            const bool cvar = vv.decl->complex_var;

            Taylor t;
            int kg = -1, kf = -1;
            if (sym_ok) {
                t = taylor(fsym, env, nm, 8);
                kg = t.nonfinite ? -2 : first_nonzero(t, 1);
                kf = t.nonfinite ? -2 : first_nonzero(t, 0);
            }

            std::vector<State> states;
            for (const auto& sp : o.lim_specs) {
                if (sp.var != nm) continue;
                for (const auto& st : expand_lim(sp.text, cvar)) {
                    bool dup = false;
                    for (const auto& e : states)
                        dup = dup || (e.to_zero == st.to_zero && e.plus == st.plus);
                    if (!dup) states.push_back(st);
                }
            }
            if (states.empty()) {
                if (cvar) states = {{true, true}, {false, true}};   // 复数：0 / ∞
                else if (views.size() == 1 || o.lim_all)
                    states = {{true, true}, {true, false}, {false, true}, {false, false}};
                else states = {{true, true}};
            }

            for (const auto& st : states) {
                LimType tg, tr;
                // 复变量或复值系数（如 f 含 z，对实数变量求偏导仍为复）→ 按模分类
                const bool mod_class = cvar || !c_is_real(f0);
                auto classify = [&](const Cplx& a, double k) -> LimType {
                    if (mod_class || !c_is_real(a)) return power_limit_c(k, st);
                    return power_limit(sign(a.re), k, st);
                };
                if (!sym_ok || t.nonfinite) {
                    Probe pg = probe_power(f, env, nm, st);
                    if (!pg.ok) {
                        tg = tr = LimType::Undefined;
                    } else {
                        const Cplx ag{Real::from_double(static_cast<double>(pg.sgn)), Real::zero()};
                        tg = classify(ag, pg.k);
                        if (!c_is_zero(f0)) {
                            tr = classify(ag, pg.k);
                        } else {
                            Probe pf = probe_magnitude(f, env, nm, st);
                            if (!pf.ok) {
                                tr = LimType::Undefined;
                            } else {
                                const double K = pg.k - pf.k;
                                const Cplx ar{Real::from_double(static_cast<double>(pg.sgn * pf.sgn)),
                                              Real::zero()};
                                tr = classify(ar, K);
                            }
                        }
                    }
                } else if (kg < 0) {
                    tg = tr = LimType::Finite;
                } else {
                    const Cplx ag = t.c[static_cast<size_t>(kg)];
                    tg = classify(ag, kg);
                    if (kf < 0) {
                        tr = LimType::Undefined;
                    } else if (c_is_zero(t.c[0])) {
                        const double K = static_cast<double>(kg) - static_cast<double>(kf);
                        const Cplx ar = t.c[static_cast<size_t>(kg)] / t.c[static_cast<size_t>(kf)];
                        tr = classify(ar, K);
                    } else {
                        tr = classify(ag, kg);
                    }
                }
                const char* slabel = cvar ? (st.to_zero ? "0" : "inf") : state_label(st);
                lrow.push_back({nm, slabel, lim_label(tg), lim_label(tr)});
            }
        }
        if (o.json) {
            JsonDoc doc;
            doc.version = kVersion;
            doc.kernel = kernel_name();
            doc.precision = o.precision;
            doc.branch = o.branch;
            doc.formula = o.formula;
            doc.mode = o.mode == Mode::Absolute ? "absolute" : "relative";
            doc.symbolic_available = sym_ok;
            doc.df = df_text;
            doc.df_value = df_value;
            doc.df_f = rel_text;
            doc.df_f_value = rel_value;
            for (const auto& v : var_lines)
                doc.vars.push_back({v.name, v.est, v.dv, v.mode});
            for (const auto& p : plines) doc.partials.push_back({p.name, p.expr, p.dv});
            for (const auto& r : brow)
                doc.boundary.push_back({r[0], r[1], r[2], r[3], r[4]});
            for (const auto& r : lrow) doc.limits.push_back({r[0], r[1], r[2], r[3]});
            std::fputs(to_json(doc).c_str(), stdout);
            return 0;
        }

        print_header(kVersion, kernel_name(), o.precision, o.branch, o.formula, var_lines);
        if (o.vars_out && !sym_ok) {
            std::vector<std::string> names;
            for (const auto& vv : views) names.push_back(vv.decl->name);
            print_symbolic_unavailable(names);
        }
        if (o.vars_out && sym_ok) print_symbolic(df_text, df_value, rel_text, rel_value, plines);
        if (!o.vars_out) std::printf("\n");
        print_boundary(brow);
        print_limits(lrow);
        return 0;
    } catch (const ParseError& e) {
        std::fprintf(stderr, tr("解析错误（第 %zu 列）: %s\n", "parse error (column %zu): %s\n"), e.col,
                     e.what());
        return 1;
    } catch (const EvalError& e) {
        std::fprintf(stderr, tr("求值错误: %s\n", "evaluation error: %s\n"), e.what());
        return 3;
    } catch (const NumError& e) {
        std::fprintf(stderr, tr("数值错误: %s\n", "numeric error: %s\n"), e.what());
        return 3;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "%s\n", e.what());
        return 2;
    }
}
