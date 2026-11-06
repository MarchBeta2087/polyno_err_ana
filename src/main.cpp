// polyno_err_ana —— 多项式/有理式误差分析器（里程碑 1：实数、常整数指数）
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>

#include "ast.hpp"
#include "num.hpp"
#include "sym.hpp"

using namespace pna;

namespace {

constexpr const char* kVersion = "0.1.0-m1";

struct Var {
    std::string name;
    Real err{Real::zero()};
    bool complex_var = false;
    Real im_err{Real::zero()};
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
    bool to_zero;   // δv → 0（否则 → ∞）
    bool plus;      // + 方向
};

const char* state_label(const State& s) {
    if (s.to_zero) return s.plus ? "+0" : "-0";
    return s.plus ? "+inf" : "-inf";
}

// 幂律判定：g ~ a·δ^k 时在给定状态下的极限类型（k 可为非整数）。
LimType power_limit(int sgn_a, double k, const State& s) {
    if (k == 0.0) return LimType::Finite;
    const bool odd_int = std::fabs(k - std::round(k)) < 1e-9 &&
                         (static_cast<long>(std::llround(k)) % 2 != 0);
    if (!s.plus && !odd_int && std::fabs(k - std::round(k)) >= 1e-9)
        return LimType::Undefined;  // δ<0 且指数非整数：实数域无定义
    bool goes_zero = (k > 0) ? s.to_zero : !s.to_zero;
    int sg = sgn_a;
    if (!s.plus && odd_int) sg = -sg;
    if (goes_zero) return sg > 0 ? LimType::PosZero : LimType::NegZero;
    return sg > 0 ? LimType::PosInf : LimType::NegInf;
}

// 符号层不可用（非整数指数等）时的数值幂律探测。
struct Probe {
    bool ok = false;
    double k = 0;
    int sgn = 0;
};

Probe probe_power(const Expr& f, const Env& env, const std::string& nm, const State& st) {
    const double h1 = st.to_zero ? 1e-2 : 1e2;
    const double h2 = st.to_zero ? 1e-4 : 1e4;
    auto g = [&](double h) -> Real {
        Env e = env;
        for (auto& kv : e)
            if (kv.first == nm) kv.second = kv.second + Real::from_double(st.plus ? h : -h);
        return eval(f, e) - eval(f, env);
    };
    Real a = g(h1), b = g(h2);
    if (!is_finite(a) || !is_finite(b) || is_zero(a) || is_zero(b)) return {};
    const double da = std::fabs(to_double(a)), db = std::fabs(to_double(b));
    if (!(da > 0) || !(db > 0)) return {};
    Probe p;
    p.k = std::log(da / db) / std::log(h1 / h2);
    if (!std::isfinite(p.k)) return {};
    if (std::fabs(p.k) < 1e-3) p.k = 0.0;
    if (std::fabs(p.k) >= 1e-9) p.k = std::round(p.k * 2.0) / 2.0;  // 吸附到 0.5 的倍数
    p.sgn = sign(b);
    p.ok = true;
    return p;
}

struct Taylor {
    std::vector<Real> c;   // c[n] = f^(n)(v)/n!
    bool nonfinite = false;
};

Taylor taylor(const Sym& f, const Env& env, const std::string& var, int n_max) {
    Taylor t;
    Sym cur = f;
    Real fact = Real::one();
    for (int n = 0; n <= n_max; ++n) {
        Real v = sym_value(cur, env);
        if (!is_finite(v)) t.nonfinite = true;
        t.c.push_back(v / fact);
        cur = diff(cur, var);
        fact = fact * Real::from_double(static_cast<double>(n + 1));
        if (sym_is_zero(cur)) break;
    }
    return t;
}

int first_nonzero(const Taylor& t, int from) {
    for (int n = from; n < static_cast<int>(t.c.size()); ++n)
        if (!is_zero(t.c[static_cast<size_t>(n)])) return n;
    return -1;
}

struct VarView {
    const Var* decl;
    Real dv;          // 实际误差量 δv
    bool degraded{};  // 相对模式下估计点为 0（N-5）
};

struct Opts {
    std::string formula;
    std::vector<Var> vars;
    std::vector<std::pair<std::string, Real>> est;
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

Real parse_num(const std::string& s) {
    try {
        return Real::from_str(s.c_str());
    } catch (const NumError&) {
        throw std::runtime_error("命令行错误: 无法解析数值: " + s);
    }
}

void parse_lim_state(const std::string& s, State& st) {
    if (s == "p0" || s == "+0") st = {true, true};
    else if (s == "n0" || s == "-0") st = {true, false};
    else if (s == "pinf" || s == "+inf") st = {false, true};
    else if (s == "ninf" || s == "-inf") st = {false, false};
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
    // 位置参数按“当前声明类别”收集：-R 后 2 个一组，-C 后 3 个一组；
    // 允许 -R 与其变量之间夹杂 -a/-r 等开关（§3.1 顺序无关）。
    std::vector<std::string> pend_real, pend_comp;
    bool comp_pending = false;
    auto need = [&](int& i, const char* name) -> std::string {
        if (i + 1 >= argc) usage_error(std::string(name) + " 缺少参数");
        return argv[++i];
    };
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a.size() && a[0] != '-' ) {
            (comp_pending ? pend_comp : pend_real).push_back(a);
            continue;
        }
        if (is_number(a.c_str())) {  // 负误差量，如 -1e-4
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
            o.est.emplace_back(kv.substr(0, eq), parse_num(kv.substr(eq + 1)));
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
            if (k != "numextend") usage_error("当前构建仅支持 --kernel numextend");
        } else if (a == "-h" || a == "--help") {
            std::printf(
                "用法: polyno_err_ana [OPTIONS] -f FORMULA\n"
                "  -R, --real VARNAME ERR [VARNAME ERR ...]   实数变量与误差\n"
                "  -C, --comp VARNAME RE_ERR IM_ERR           复数变量（里程碑 1 未实现）\n"
                "  -a, --absolute | -r, --relative            误差模式（默认绝对）\n"
                "      --vars                                 显示符号推导\n"
                "      --estimated VARNAME=VALUE              估计点（可多次）\n"
                "  -p, --precision INT                        精度 bit（默认 256）\n"
                "      --lim all | VAR:p0|n0|pinf|ninf        极限分类规格\n"
                "  -V, --version   -G, --gpl   -h, --help\n");
            std::exit(0);
        } else if (a == "-V" || a == "--version") {
            std::printf("polyno_err_ana %s (kernel: numextend, %d bit)\n", kVersion, o.precision);
            std::exit(0);
        } else if (a == "-G" || a == "--gpl") {
            std::printf("GPL-3.0-or-later: <https://www.gnu.org/licenses/gpl-3.0.txt>\n");
            std::exit(0);
        } else {
            usage_error("未知选项: " + a);
        }
    }
    if (pend_real.size() % 2 != 0) usage_error("-R 需要成对的 VARNAME ERR");
    for (size_t k = 0; k + 1 < pend_real.size(); k += 2) {
        Var v;
        v.name = pend_real[k];
        v.err = parse_num(pend_real[k + 1]);
        o.vars.push_back(std::move(v));
    }
    if (pend_comp.size() % 3 != 0) usage_error("-C 需要成组的 VARNAME RE_ERR IM_ERR");
    for (size_t k = 0; k + 2 < pend_comp.size(); k += 3) {
        Var v;
        v.name = pend_comp[k];
        v.err = parse_num(pend_comp[k + 1]);
        v.im_err = parse_num(pend_comp[k + 2]);
        v.complex_var = true;
        o.vars.push_back(std::move(v));
    }

    if (o.formula.empty()) usage_error("缺少 -f FORMULA");
    if (o.vars.empty()) usage_error("至少需要一个 -R 变量");
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
        env.emplace_back(v.name, it == o.est.end() ? Real::zero() : it->second);
    }
    return env;
}

Real value_of(const Env& env, const std::string& name) {
    const Real* p = lookup(env, name);
    return p ? *p : Real::zero();
}

std::string var_note(const Opts& o, const Var& v, const Real& est) {
    if (o.mode == Mode::Absolute) return "";
    if (is_zero(est)) return "  [δ" + v.name + " = 0（估计点为 0，相对误差失效）]";
    return "";
}

void print_limits_header() {
    std::printf("\n  极限分类（单变量变，其余 δ=0）\n");
    std::printf("  %-8s %-12s %-12s\n", "δv →", "δf", "δf/f");
}

}  // namespace

int main(int argc, char** argv) {
    try {
        Opts o = parse_args(argc, argv);
        set_precision(o.precision);

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
            sym_ok = false;  // 非整数指数：符号层不可用，退化为数值探测
        }

        // δv 视图
        std::vector<VarView> views;
        for (const auto& v : o.vars) {
            Real est = value_of(env, v.name);
            VarView vv{&v, v.err, false};
            if (o.mode == Mode::Relative) {
                if (is_zero(est)) {
                    vv.dv = Real::zero();
                    vv.degraded = true;
                } else {
                    vv.dv = abs(est) * v.err;
                }
            }
            views.push_back(vv);
        }

        std::printf("polyno_err_ana %s  (numextend, %d bit)\n", kVersion, o.precision);
        if (o.branch != 0) std::printf("branch k = %d\n", o.branch);
        std::printf("\nf    = %s\n", o.formula.c_str());
        for (const auto& vv : views) {
            Real est = value_of(env, vv.decl->name);
            std::printf("%s ≈ %s,  δ%s = %s (%s)%s\n", vv.decl->name.c_str(), fmt(est).c_str(),
                        vv.decl->name.c_str(), fmt(vv.dv).c_str(),
                        o.mode == Mode::Absolute ? "absolute" : "relative",
                        var_note(o, *vv.decl, est).c_str());
        }

        // 逐变量偏导与全微分
        std::vector<Sym> partials;
        Sym delta = sym_const(Real::zero());
        for (const auto& vv : views) {
            Sym p = sym_ok ? diff(fsym, vv.decl->name) : Sym{};
            partials.push_back(p);
            if (sym_ok) delta = sym_add(delta, sym_scale(p, vv.dv));
        }
        Sym rel = sym_ok ? sym_div_by(delta, f) : Sym{};

        if (o.vars_out && !sym_ok) {
            std::printf("\n");
            std::printf("%-9s= N/A\n", "δf");
            std::printf("%-9s= N/A\n", "δf/f");
            for (const auto& vv : views) {
                std::printf("∂f/∂%s = N/A\n", vv.decl->name.c_str());
                std::printf("δ%-8s= %s\n\n", vv.decl->name.c_str(), fmt(vv.dv).c_str());
            }
        }
        if (o.vars_out && sym_ok) {
            std::printf("\n");
            std::printf("δf    = %s\n", sym_show(delta).c_str());
            std::printf("      = %s\n", fmt(sym_value(delta, env)).c_str());
            std::printf("δf/f  = %s\n", sym_show(rel).c_str());
            std::printf("      = %s\n", fmt(sym_value(rel, env)).c_str());
            std::printf("\n");
            for (size_t i = 0; i < views.size(); ++i) {
                const auto& vv = views[i];
                Real est = value_of(env, vv.decl->name);
                Real pv = sym_value(partials[i], env);
                std::printf("∂f/∂%s  = %s\n", vv.decl->name.c_str(), sym_show(partials[i]).c_str());
                std::printf("δ%s     = %s\n", vv.decl->name.c_str(), fmt(vv.dv).c_str());
                if (is_zero(est))
                    std::printf("δ%s/%s   = N/A（估计点为 0）\n", vv.decl->name.c_str(),
                                vv.decl->name.c_str());
                else
                    std::printf("δ%s/%s   = %s\n", vv.decl->name.c_str(), vv.decl->name.c_str(),
                                fmt(vv.dv / est).c_str());
                std::printf("\n");
            }
        }

        // 精确边界（§3.4）
        Real f0 = eval(f, env);
        std::printf("δ_{R+}f   δ_{R-}f   δ_{R+}f/f   δ_{R-}f/f\n");
        for (const auto& vv : views) {
            Env ep = env, em = env;
            for (auto& kv : ep)
                if (kv.first == vv.decl->name) kv.second = kv.second + vv.dv;
            for (auto& kv : em)
                if (kv.first == vv.decl->name) kv.second = kv.second - vv.dv;
            Real fp = eval(f, ep), fm = eval(f, em);
            Real dp = abs(fp - f0), dm = abs(fm - f0);
            auto cell = [](const Real& x) { return is_finite(x) ? fmt(x) : std::string("N/A"); };
            std::printf("%-10s %-10s ", cell(dp).c_str(), cell(dm).c_str());
            if (is_zero(f0))
                std::printf("%-11s %-11s", "N/A", "N/A");
            else
                std::printf("%-11s %-11s", cell(dp / abs(f0)).c_str(), cell(dm / abs(f0)).c_str());
            std::printf(" (%s)", vv.decl->name.c_str());
            std::printf("\n");
        }

        // 极限分类（§3.6）
        print_limits_header();
        for (const auto& vv : views) {
            const std::string& nm = vv.decl->name;
            Taylor t;
            int kg = -1, kf = -1;
            if (sym_ok) {
                t = taylor(fsym, env, nm, 8);
                kg = t.nonfinite ? -2 : first_nonzero(t, 1);
                kf = t.nonfinite ? -2 : first_nonzero(t, 0);
            }

            std::vector<State> states;
            if (views.size() == 1) {
                states = {{true, true}, {true, false}, {false, true}, {false, false}};
            } else if (o.lim_all) {
                states = {{true, true}, {true, false}, {false, true}, {false, false}};
            } else if (o.lim_spec && o.lim_var == nm) {
                states = {o.lim_state};
            } else {
                states = {{true, true}};
            }

            for (const auto& st : states) {
                LimType tg;
                LimType tr;
                if (!sym_ok) {
                    Probe p = probe_power(f, env, nm, st);
                    if (!p.ok) {
                        tg = tr = LimType::Undefined;
                    } else {
                        tg = power_limit(p.sgn, p.k, st);
                        tr = LimType::Undefined;  // 相对形式的非整数阶无法稳定判定
                    }
                } else if (t.nonfinite) {
                    tg = LimType::Undefined;
                    tr = LimType::Undefined;
                } else if (kg < 0) {
                    tg = LimType::Finite;
                    tr = LimType::Finite;
                } else {
                    tg = power_limit(sign(t.c[static_cast<size_t>(kg)]), kg, st);
                    if (kf < 0) {
                        tr = LimType::Undefined;
                    } else if (is_zero(t.c[0])) {
                        long K = static_cast<long>(kg) - static_cast<long>(kf);
                        if (K == 0) {
                            tr = LimType::Finite;
                        } else {
                            Real ratio = t.c[static_cast<size_t>(kg)] / t.c[static_cast<size_t>(kf)];
                            tr = power_limit(sign(ratio), K, st);
                        }
                    } else {
                        tr = power_limit(sign(t.c[static_cast<size_t>(kg)]), kg, st);
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
