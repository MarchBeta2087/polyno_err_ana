#include "num.hpp"

#include <cmath>

namespace pna {
namespace {
int g_branch = 0;
}  // namespace

void set_branch(int k) { g_branch = k; }
int branch_k() { return g_branch; }

Real pi() {
    static const Real p = r_atan2(Real::zero(), Real::from_double(-1.0));
    return p;
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
    return r_sqrt(t);
}

Real c_arg(const Cplx& a) { return r_atan2(a.im, a.re); }

Cplx c_exp(const Cplx& a) {
    Real e = r_exp(a.re);
    return Cplx{e * r_cos(a.im), e * r_sin(a.im)};
}

Cplx c_log(const Cplx& a) {
    if (c_is_zero(a)) return c_nan();
    return Cplx{r_log(c_abs(a)), c_arg(a)};
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
