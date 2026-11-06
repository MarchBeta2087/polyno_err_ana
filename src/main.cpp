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
#include "license_text.hpp"
#include "sym.hpp"

using namespace pna;

namespace {

constexpr const char* kVersion = "0.2.0-m2";

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
    int precision = 256;
    int branch = 0;
    bool lim_all = false;
    bool lim_spec = false;
    std::string lim_var;
    State lim_state{true, true};
};

[[noreturn]] void usage_error(const std::string& m) {
    throw std::runtime_error("命令行错误: " + m);
}

Cplx parse_num(const std::string& s) {
    try {
        return c_from_str(s.c_str());
    } catch (const NumError&) {
        throw std::runtime_error("命令行错误: 无法解析数值: " + s);
    }
}

// --estimated 的 VALUE 走公式常量语法（支持 1+2i）
Cplx parse_value(const std::string& s) {
    try {
        return const_value(parse(s));
    } catch (const ParseError& e) {
        throw std::runtime_error("命令行错误: --estimated 值非法（第 " + std::to_string(e.col) +
                                 " 列）: " + e.what());
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

bool is_number(const char* s) {
    if (!s || !*s) return false;
    char* end = nullptr;
    std::strtod(s, &end);
    return end && *end == 0;
}

Opts parse_args(int argc, char** argv) {
    Opts o;
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
        } else if (a == "--vars") {
            o.vars_out = true;
        } else if (a == "-p" || a == "--precision") {
            o.precision = std::stoi(need(i, "-p"));
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
                auto c = v.find(':');
                if (c == std::string::npos) usage_error("--lim 需要 all 或 VAR:STATE");
                o.lim_var = v.substr(0, c);
                parse_lim_state(v.substr(c + 1), o.lim_state);
                o.lim_spec = true;
            }
        } else if (a == "--kernel") {
            std::string k = need(i, "--kernel");
            if (k != kernel_name())
                usage_error(std::string("当前构建的内核是 ") + kernel_name() + "，不支持 --kernel " + k);
        } else if (a == "-h" || a == "--help") {
            std::printf(
                "用法: polyno_err_ana [OPTIONS] -f FORMULA\n"
                "  -R, --real VARNAME ERR [VARNAME ERR ...]        实数变量与误差\n"
                "  -C, --comp VARNAME RE_ERR IM_ERR [...]          复数变量与实/虚部误差\n"
                "  -a, --absolute | -r, --relative                 误差模式（默认绝对）\n"
                "      --vars                                      显示符号推导\n"
                "      --estimated VARNAME=VALUE                   估计点（可多次，支持 1+2i）\n"
                "  -p, --precision INT                             精度 bit（默认 256）\n"
                "  -k, --branch INT                                复数幂分支号（默认 0）\n"
                "      --lim all | VAR:p0|n0|pinf|ninf              极限分类规格\n"
                "  -V, --version   -G, --gpl   -h, --help\n");
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

    if (o.formula.empty()) usage_error("缺少 -f FORMULA");
    if (o.vars.empty()) usage_error("至少需要一个 -R 或 -C 变量");
    for (size_t i = 0; i < o.vars.size(); ++i)
        for (size_t j = i + 1; j < o.vars.size(); ++j)
            if (o.vars[i].name == o.vars[j].name) usage_error("变量重复声明: " + o.vars[i].name);
    if (o.lim_all && o.lim_spec) usage_error("--lim all 与 --lim VAR:STATE 冲突");
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
        Opts o = parse_args(argc, argv);
        set_precision(o.precision);
        set_branch(o.branch);

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

        std::printf("polyno_err_ana %s  (%s, %d bit)\n", kVersion, kernel_name(), o.precision);
        if (o.branch != 0) std::printf("branch k = %d\n", o.branch);
        std::printf("\nf    = %s\n", o.formula.c_str());
        for (const auto& vv : views) {
            Cplx est = value_of(env, vv.decl->name);
            std::printf("%s ≈ %s,  δ%s = %s (%s)%s\n", vv.decl->name.c_str(), c_fmt(est).c_str(),
                        vv.decl->name.c_str(), c_fmt(vv.dv).c_str(),
                        o.mode == Mode::Absolute ? "absolute" : "relative",
                        vv.degraded ? "  [δv = 0（估计点为 0，相对误差失效）]" : "");
        }

        std::vector<Sym> partials;
        Sym delta = sym_const(c_zero());
        for (const auto& vv : views) {
            Sym p = sym_ok ? diff(fsym, vv.decl->name) : Sym{};
            partials.push_back(p);
            if (sym_ok) delta = sym_add(delta, sym_scale(p, vv.dv));
        }
        Sym rel = sym_ok ? sym_div_by(delta, f) : Sym{};

        if (o.vars_out && !sym_ok) {
            std::printf("\n");
            std::printf("%-9s= N/A（含非整数指数，符号展开不支持）\n", "δf");
            std::printf("%-9s= N/A\n", "δf/f");
            for (const auto& vv : views)
                std::printf("∂f/∂%s = N/A\n", vv.decl->name.c_str());
            std::printf("\n");
        }
        if (o.vars_out && sym_ok) {
            std::printf("\n");
            std::printf("δf    = %s\n", sym_show(delta).c_str());
            std::printf("      = %s\n", c_fmt(sym_value(delta, env)).c_str());
            std::printf("δf/f  = %s\n", sym_show(rel).c_str());
            std::printf("      = %s\n", c_fmt(sym_value(rel, env)).c_str());
            std::printf("\n");
            for (size_t i = 0; i < views.size(); ++i) {
                const auto& vv = views[i];
                Cplx est = value_of(env, vv.decl->name);
                std::printf("∂f/∂%s  = %s\n", vv.decl->name.c_str(), sym_show(partials[i]).c_str());
                std::printf("δ%s     = %s\n", vv.decl->name.c_str(), c_fmt(vv.dv).c_str());
                if (c_is_zero(est))
                    std::printf("δ%s/%s   = N/A（估计点为 0）\n", vv.decl->name.c_str(),
                                vv.decl->name.c_str());
                else
                    std::printf("δ%s/%s   = %s\n", vv.decl->name.c_str(), vv.decl->name.c_str(),
                                c_fmt(vv.dv / est).c_str());
                std::printf("\n");
            }
        }

        // 精确边界（§3.4）
        Cplx f0 = eval(f, env);
        Real f0abs = c_abs(f0);
        std::printf("δ_{R+}f   δ_{R-}f   δ_{R+}f/f   δ_{R-}f/f\n");
        for (const auto& vv : views) {
            Env ep = env, em = env;
            for (auto& kv : ep)
                if (kv.first == vv.decl->name) kv.second = kv.second + vv.dv;
            for (auto& kv : em)
                if (kv.first == vv.decl->name) kv.second = kv.second - vv.dv;
            Real dp = c_abs(eval(f, ep) - f0), dm = c_abs(eval(f, em) - f0);
            auto cell = [](const Real& x) { return is_finite(x) ? fmt(x) : std::string("N/A"); };
            std::printf("%-10s %-10s ", cell(dp).c_str(), cell(dm).c_str());
            if (is_zero(f0abs))
                std::printf("%-11s %-11s", "N/A", "N/A");
            else
                std::printf("%-11s %-11s", cell(dp / f0abs).c_str(), cell(dm / f0abs).c_str());
            std::printf(" (%s)\n", vv.decl->name.c_str());
        }

        // 极限分类（§3.6）
        std::printf("\n  极限分类（单变量变，其余 δ=0）\n");
        std::printf("  %-8s %-12s %-12s\n", "δv →", "δf", "δf/f");
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
            if (cvar) {
                states = {{true, true}, {false, true}};  // 复数：0 / ∞
            } else if (views.size() == 1 || o.lim_all) {
                states = {{true, true}, {true, false}, {false, true}, {false, false}};
            } else if (o.lim_spec && o.lim_var == nm) {
                states = {o.lim_state};
            } else {
                states = {{true, true}};
            }

            for (const auto& st : states) {
                LimType tg, tr;
                if (!sym_ok || t.nonfinite) {
                    Probe p = probe_power(f, env, nm, st);
                    if (!p.ok) {
                        tg = tr = LimType::Undefined;
                    } else {
                        tg = cvar ? power_limit_c(p.k, st) : power_limit(p.sgn, p.k, st);
                        tr = LimType::Undefined;
                    }
                } else if (kg < 0) {
                    tg = tr = LimType::Finite;
                } else {
                    const Cplx ag = t.c[static_cast<size_t>(kg)];
                    if (cvar) {
                        tg = power_limit_c(kg, st);
                    } else {
                        tg = power_limit(sign(ag.re), kg, st);
                    }
                    if (kf < 0) {
                        tr = LimType::Undefined;
                    } else if (c_is_zero(t.c[0])) {
                        const double K = static_cast<double>(kg) - static_cast<double>(kf);
                        const Cplx ar = t.c[static_cast<size_t>(kg)] / t.c[static_cast<size_t>(kf)];
                        tr = cvar ? power_limit_c(K, st) : power_limit(sign(ar.re), K, st);
                    } else {
                        tr = cvar ? power_limit_c(kg, st) : power_limit(sign(ag.re), kg, st);
                    }
                }
                std::printf("  %-8s %-12s %-12s (%s)\n", state_label(st), lim_label(tg), lim_label(tr),
                            nm.c_str());
            }
        }
        return 0;
    } catch (const ParseError& e) {
        std::fprintf(stderr, "解析错误（第 %zu 列）: %s\n", e.col, e.what());
        return 1;
    } catch (const EvalError& e) {
        std::fprintf(stderr, "求值错误: %s\n", e.what());
        return 3;
    } catch (const NumError& e) {
        std::fprintf(stderr, "数值错误: %s\n", e.what());
        return 3;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "%s\n", e.what());
        return 2;
    }
}
