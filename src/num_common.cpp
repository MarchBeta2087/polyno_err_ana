// SPDX-License-Identifier: GPL-3.0-or-later
#include "num.hpp"

#include <algorithm>
#include <cmath>

namespace pna {
namespace {
int g_branch = 0;
bool g_real_root = false;

// 连续分数逼近：x ≈ p/q（q ≤ 1e6，误差 ≤ 1e-12·max(1,|x|)）。
bool rational_of(double x, long& p, long& q) {
    if (!std::isfinite(x)) return false;
    const double tol = 1e-12 * std::max(1.0, std::fabs(x));
    double v = x;
    long h0 = 0, h1 = 1, k0 = 1, k1 = 0;
    for (int i = 0; i < 64; ++i) {
        const double a = std::floor(v);
        if (!std::isfinite(a) || std::fabs(a) > 1e15) return false;
        const long ai = static_cast<long>(a);
        const long h2 = ai * h1 + h0;
        const long k2 = ai * k1 + k0;
        if (k2 > 1000000 || k2 <= 0) return false;
        h0 = h1; h1 = h2; k0 = k1; k1 = k2;
        if (k1 == 0) return false;
        const double appr = static_cast<double>(h1) / static_cast<double>(k1);
        if (std::fabs(appr - x) <= tol) { p = h1; q = k1; return true; }
        const double frac = v - a;
        if (frac == 0.0) return false;
        v = 1.0 / frac;
    }
    return false;
}
}  // namespace

void set_branch(int k) { g_branch = k; }
int branch_k() { return g_branch; }
void set_real_root(bool on) { g_real_root = on; }
bool real_root_enabled() { return g_real_root; }

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
    // --real-root：负实底数 + 有理指数（分母为奇数）取实根
    if (g_real_root && c_is_real(e) && c_is_real(base) && sign(base.re) < 0) {
        long p = 0, q = 1;
        if (!rational_of(to_double(e.re), p, q) || q % 2 == 0) return c_nan();
        Real m = pow(abs(base.re), e.re);
        const int sg = (p % 2 == 0) ? 1 : -1;
        return Cplx{sg > 0 ? m : neg(m), Real::zero()};
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
    if (is_finite(a.re) && is_finite(a.im)) {
        if (c_is_real(a)) return fmt(a.re);
        std::string im = fmt(abs(a.im));
        return "(" + fmt(a.re) + (is_neg(a.im) ? "-" : "+") + im + "i)";
    }
    // N-3：NaN 一律 Undefined；纯 ±Inf 显示 ±Inf
    if (is_nan(a.re) || is_nan(a.im)) return "Undefined";
    if (!is_finite(a.re) && is_finite(a.im)) return c_is_real(a) ? (is_neg(a.re) ? "-Inf" : "Inf") : "Undefined";
    if (is_finite(a.re) && !is_finite(a.im)) return "Undefined";
    return is_neg(a.re) ? "-Inf" : "Inf";
}

bool is_integer(const Real& a) {
    double d = to_double(a);
    return std::isfinite(d) && d == std::floor(d) && std::fabs(d) <= 1e9;
}

}  // namespace pna
