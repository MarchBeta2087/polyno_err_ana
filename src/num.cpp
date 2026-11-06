#include "num.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace pna {
namespace {
int g_bits = 256;

bigfloat_ctx_ty g_ctx{};
bool g_ready = false;
int g_branch = 0;
}  // namespace

void set_precision(int bits) {
    if (bits < 2) throw NumError("precision must be >= 2");
    g_bits = bits;
    bigfloat_ctx_make(&g_ctx, static_cast<size_t>(bits), 20, BIGFLOAT_ROUND_NEAREST_EVEN_E);
    g_ready = true;
}

void set_branch(int k) { g_branch = k; }
int branch_k() { return g_branch; }

const bigfloat_ctx_ty& ctx() {
    if (!g_ready) set_precision(g_bits);
    return g_ctx;
}

Real::Real() { bigfloat_init(&v); }

Real::Real(const Real& o) {
    bigfloat_init(&v);
    if (bigfloat_copy(&v, &o.v) != BIGFLOAT_OK_E) throw NumError("oom");
}

Real::Real(Real&& o) noexcept : v(o.v) { bigfloat_init(&o.v); }

Real::~Real() { bigfloat_free(&v); }

Real& Real::operator=(const Real& o) {
    if (this != &o && bigfloat_copy(&v, &o.v) != BIGFLOAT_OK_E) throw NumError("oom");
    return *this;
}

Real& Real::operator=(Real&& o) noexcept {
    if (this != &o) {
        bigfloat_free(&v);
        v = o.v;
        bigfloat_init(&o.v);
    }
    return *this;
}

Real Real::from_double(double d) {
    Real r;
    bigfloat_from_f64(&r.v, d);
    return r;
}

Real Real::from_str(const char* s) {
    Real r;
    if (bigfloat_from_str(&r.v, s, &ctx(), nullptr) != BIGFLOAT_OK_E)
        throw NumError(std::string("cannot parse number: ") + s);
    return r;
}

Real Real::nan() {
    Real r;
    bigfloat_from_f64(&r.v, std::nan(""));
    return r;
}

Real Real::one() { return from_double(1.0); }
Real Real::zero() { return Real{}; }

namespace {
Real binop(bigfloat_err_ty (*f)(bigfloat_ty*, const bigfloat_ty*, const bigfloat_ty*,
                               const bigfloat_ctx_ty*),
           const Real& a, const Real& b) {
    Real r;
    f(&r.v, &a.v, &b.v, &ctx());
    return r;
}
}  // namespace

Real operator+(const Real& a, const Real& b) { return binop(bigfloat_add, a, b); }
Real operator-(const Real& a, const Real& b) { return binop(bigfloat_sub, a, b); }
Real operator*(const Real& a, const Real& b) { return binop(bigfloat_mul, a, b); }
Real operator/(const Real& a, const Real& b) { return binop(bigfloat_div, a, b); }

Real neg(const Real& a) {
    Real r(a);
    bigfloat_neg(&r.v);
    return r;
}

Real abs(const Real& a) { return is_neg(a) ? neg(a) : a; }

Real pow(const Real& base, const Real& e) {
    double d = to_double(e);
    bool int_ok = std::isfinite(d) && d == std::floor(d) && std::fabs(d) <= 1024.0;
    if (int_ok) {
        long n = static_cast<long>(d);
        if (n == 0) {
            if (is_zero(base)) return Real::nan();  // 0^0 → Undefined（N-3）
            return Real::one();
        }
        bool inv = n < 0;
        unsigned long k = static_cast<unsigned long>(inv ? -n : n);
        Real result = Real::one();
        Real b = base;
        while (k) {
            if (k & 1u) result = result * b;
            b = b * b;
            k >>= 1;
        }
        return inv ? Real::one() / result : result;
    }
    if (is_zero(base)) {
        int se = sign(e);
        if (se > 0) return Real::zero();
        if (se < 0) return Real::from_double(1.0 / 0.0);
        return Real::nan();
    }
    if (sign(base) < 0) return Real::nan();
    Real lg;
    bigfloat_log(&lg.v, &base.v, &ctx());
    Real t = e * lg;
    Real r;
    bigfloat_exp(&r.v, &t.v, &ctx());
    return r;
}

bool is_zero(const Real& a) { return bigfloat_is_zero(&a.v); }
bool is_finite(const Real& a) {
    switch (a.v.flag) {
        case BIGFLOAT_POS_ZERO_E:
        case BIGFLOAT_NEG_ZERO_E:
        case BIGFLOAT_POS_E:
        case BIGFLOAT_NEG_E: return true;
        default: return false;
    }
}
bool is_neg(const Real& a) { return a.v.flag == BIGFLOAT_NEG_E || a.v.flag == BIGFLOAT_NEG_INF_E; }

int sign(const Real& a) {
    if (!is_finite(a)) return 0;
    if (is_zero(a)) return 0;
    return is_neg(a) ? -1 : 1;
}

double to_double(const Real& a) {
    double d = 0;
    bigfloat_to_f64(&a.v, &d);
    return d;
}

std::string to_str(const Real& a, int digits) {
    if (!is_finite(a)) {
        if (bigfloat_is_nan(&a.v)) return "NaN";
        return is_neg(a) ? "-Inf" : "Inf";
    }
    size_t need = 0;
    bigfloat_to_str(&a.v, static_cast<size_t>(digits < 1 ? 6 : digits), nullptr, 0, &need);
    std::string s(need + 1, '\0');
    bigfloat_to_str(&a.v, static_cast<size_t>(digits < 1 ? 6 : digits), s.data(), s.size(), &need);
    s.resize(std::strlen(s.c_str()));
    return s;
}

std::string fmt(const Real& a) {
    if (!is_finite(a)) return to_str(a, 6);
    if (is_zero(a)) return "0";
    double d = to_double(a);
    char buf[64];
    if (d != 0.0 && std::fabs(d) >= 1e-4 && std::fabs(d) < 1e6)
        std::snprintf(buf, sizeof buf, "%.6g", d);
    else
        std::snprintf(buf, sizeof buf, "%.4e", d);
    return buf;
}


// ------------------------------------------------------------------ //
// 复数
// ------------------------------------------------------------------ //

Cplx c_from_double(double re, double im) { return Cplx{Real::from_double(re), Real::from_double(im)}; }

Cplx c_from_str(const char* s) {
    // 仅支持纯实数文本；复数请走公式解析（"1+2i"）。
    Real r = Real::from_str(s);
    return Cplx{r, Real::zero()};
}
Cplx c_zero() { return Cplx{Real::zero(), Real::zero()}; }
Cplx c_one() { return Cplx{Real::one(), Real::zero()}; }
Cplx c_i() { return Cplx{Real::zero(), Real::one()}; }
Cplx c_nan() { return Cplx{Real::nan(), Real::nan()}; }

Cplx operator+(const Cplx& a, const Cplx& b) { return Cplx{a.re + b.re, a.im + b.im}; }
Cplx operator-(const Cplx& a, const Cplx& b) { return Cplx{a.re - b.re, a.im - b.im}; }
Cplx operator*(const Cplx& a, const Cplx& b) {
    return Cplx{a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re};
}
Cplx operator/(const Cplx& a, const Cplx& b) {
    Real d = b.re * b.re + b.im * b.im;
    return Cplx{(a.re * b.re + a.im * b.im) / d, (a.im * b.re - a.re * b.im) / d};
}
Cplx c_neg(const Cplx& a) { return Cplx{neg(a.re), neg(a.im)}; }
Cplx c_scale(const Cplx& a, const Real& k) { return Cplx{a.re * k, a.im * k}; }

Real c_abs(const Cplx& a) {
    Real t = a.re * a.re + a.im * a.im;
    Real r;
    bigfloat_sqrt(&r.v, &t.v, &ctx());
    return r;
}

Real pi() {
    Real y = Real::zero();
    Real x = Real::from_double(-1.0);
    Real out;
    bigfloat_atan2(&out.v, &y.v, &x.v, &ctx());
    return out;
}

Real c_arg(const Cplx& a) {
    Real out;
    if (bigfloat_atan2(&out.v, &a.im.v, &a.re.v, &ctx()) != BIGFLOAT_OK_E) return Real::nan();
    return out;
}

Cplx c_exp(const Cplx& a) {
    Real e;
    bigfloat_exp(&e.v, &a.re.v, &ctx());
    Real c, s;
    bigfloat_cos(&c.v, &a.im.v, &ctx());
    bigfloat_sin(&s.v, &a.im.v, &ctx());
    return Cplx{e * c, e * s};
}

Cplx c_log(const Cplx& a) {
    if (c_is_zero(a)) return c_nan();
    Real r = c_abs(a);
    Real lr;
    bigfloat_log(&lr.v, &r.v, &ctx());
    return Cplx{lr, c_arg(a)};
}

Cplx c_pow(const Cplx& base, const Cplx& e) {
    // 整数实指数：平方乘，负底数/负指数均适用，与分支号无关。
    if (c_is_real(e) && is_integer(e.re)) {
        long n = static_cast<long>(to_double(e.re));
        if (n == 0) return c_is_zero(base) ? c_nan() : c_one();
        bool inv = n < 0;
        unsigned long k = static_cast<unsigned long>(inv ? -n : n);
        Cplx acc = c_one(), b = base;
        while (k) {
            if (k & 1u) acc = acc * b;
            b = b * b;
            k >>= 1;
        }
        return inv ? c_one() / acc : acc;
    }
    if (c_is_zero(base)) {
        if (c_is_real(e) && sign(e.re) > 0) return c_zero();
        return c_nan();
    }
    // z^w = exp(w·(log z + 2πik))
    Cplx l = c_log(base);
    if (branch_k() != 0) {
        Real twopi = pi() * Real::from_double(2.0);
        l.im = l.im + twopi * Real::from_double(static_cast<double>(branch_k()));
    }
    return c_exp(e * l);
}

bool c_is_zero(const Cplx& a) { return is_zero(a.re) && is_zero(a.im); }
bool c_is_real(const Cplx& a) { return is_zero(a.im); }
bool c_is_finite(const Cplx& a) { return is_finite(a.re) && is_finite(a.im); }

std::string c_fmt(const Cplx& a) {
    if (c_is_real(a)) return fmt(a.re);
    std::string im = fmt(abs(a.im));
    return "(" + fmt(a.re) + (is_neg(a.im) ? "-" : "+") + im + "i)";
}

bool is_integer(const Real& a) {
    double d = to_double(a);
    return std::isfinite(d) && d == std::floor(d) && std::fabs(d) <= 1e9;
}

}  // namespace pna
