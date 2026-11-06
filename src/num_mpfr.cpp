// SPDX-License-Identifier: GPL-3.0-or-later
// MPFR 后端：与 numextend 版实现同一组 Real 原语（见 num.hpp）。
#include "num.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace pna {
namespace {

int g_bits = 256;

mpfr_prec_t prec() { return static_cast<mpfr_prec_t>(g_bits); }

void check(int rc, const char* what) {
    if (rc != 0) throw NumError(std::string("mpfr error in ") + what);
}

}  // namespace

void set_precision(int bits) {
    if (bits < 2) throw NumError("precision must be >= 2");
    g_bits = bits;
}

const char* kernel_name() { return "mpfr"; }

Real::Real() {
    mpfr_init2(v, prec());
    mpfr_set_zero(v, 1);  // mpfr_init2 留下 NaN，必须显式置 +0
}

Real::Real(const Real& o) {
    mpfr_init2(v, mpfr_get_prec(o.v));
    mpfr_set(v, o.v, MPFR_RNDN);
}

Real::Real(Real&& o) noexcept {
    mpfr_init2(v, prec());
    mpfr_set_zero(v, 1);
    mpfr_swap(v, o.v);
}

Real::~Real() { mpfr_clear(v); }

Real& Real::operator=(const Real& o) {
    if (this != &o) {
        mpfr_set_prec(v, mpfr_get_prec(o.v));
        mpfr_set(v, o.v, MPFR_RNDN);
    }
    return *this;
}

Real& Real::operator=(Real&& o) noexcept {
    if (this != &o) mpfr_swap(v, o.v);
    return *this;
}

Real Real::from_double(double d) {
    Real r;
    mpfr_set_d(r.v, d, MPFR_RNDN);
    return r;
}

Real Real::from_str(const char* s) {
    Real r;
    if (mpfr_set_str(r.v, s, 10, MPFR_RNDN) != 0)
        throw NumError(std::string("cannot parse number: ") + s);
    return r;
}

Real Real::nan() {
    Real r;
    mpfr_set_nan(r.v);
    return r;
}

Real Real::inf(int s) {
    Real r;
    mpfr_set_inf(r.v, s < 0 ? -1 : 1);
    return r;
}

Real Real::one() { return from_double(1.0); }
Real Real::zero() { return Real{}; }

namespace {
template <class F>
Real binop(F f, const Real& a, const Real& b) {
    Real r;
    f(r.v, a.v, b.v, MPFR_RNDN);
    return r;
}
}  // namespace

Real operator+(const Real& a, const Real& b) { return binop(mpfr_add, a, b); }
Real operator-(const Real& a, const Real& b) { return binop(mpfr_sub, a, b); }
Real operator*(const Real& a, const Real& b) { return binop(mpfr_mul, a, b); }
Real operator/(const Real& a, const Real& b) { return binop(mpfr_div, a, b); }

Real neg(const Real& a) {
    Real r(a);
    mpfr_neg(r.v, r.v, MPFR_RNDN);
    return r;
}

Real abs(const Real& a) {
    Real r(a);
    mpfr_abs(r.v, r.v, MPFR_RNDN);
    return r;
}

Real pow(const Real& base, const Real& e) {
    if (is_zero(base) && is_zero(e)) return Real::nan();  // 0^0 → Undefined（N-3）
    Real r;
    mpfr_pow(r.v, base.v, e.v, MPFR_RNDN);
    return r;
}

bool re_eq(const Real& a, const Real& b) { return mpfr_equal_p(a.v, b.v) != 0; }
bool is_zero(const Real& a) { return mpfr_zero_p(a.v) != 0; }
bool is_finite(const Real& a) { return mpfr_number_p(a.v) != 0; }
bool is_nan(const Real& a) { return mpfr_nan_p(a.v) != 0; }
bool is_neg(const Real& a) { return mpfr_signbit(a.v) != 0; }

int sign(const Real& a) {
    if (!is_finite(a) || is_zero(a)) return 0;
    return mpfr_sgn(a.v) < 0 ? -1 : 1;
}

double to_double(const Real& a) { return mpfr_get_d(a.v, MPFR_RNDN); }

std::string to_str(const Real& a, int digits) {
    if (!is_finite(a)) return mpfr_nan_p(a.v) ? "NaN" : (is_neg(a) ? "-Inf" : "Inf");
    char buf[128];
    mpfr_snprintf(buf, sizeof buf, "%.*Rg", digits < 1 ? 6 : digits, a.v);
    return buf;
}

std::string fmt(const Real& a) {
    if (!is_finite(a)) return to_str(a, 6);
    if (is_zero(a)) return "0";
    const double d = to_double(a);
    char buf[128];
    if (d != 0.0 && std::fabs(d) >= 1e-4 && std::fabs(d) < 1e6)
        mpfr_snprintf(buf, sizeof buf, "%.6Rg", a.v);
    else
        mpfr_snprintf(buf, sizeof buf, "%.4Re", a.v);
    return buf;
}

Real r_exp(const Real& a) {
    Real r;
    mpfr_exp(r.v, a.v, MPFR_RNDN);
    return r;
}

Real r_log(const Real& a) {
    Real r;
    mpfr_log(r.v, a.v, MPFR_RNDN);
    return r;
}

Real r_sqrt(const Real& a) {
    Real r;
    mpfr_sqrt(r.v, a.v, MPFR_RNDN);
    return r;
}

Real r_sin(const Real& a) {
    Real r;
    mpfr_sin(r.v, a.v, MPFR_RNDN);
    return r;
}

Real r_cos(const Real& a) {
    Real r;
    mpfr_cos(r.v, a.v, MPFR_RNDN);
    return r;
}

Real r_atan2(const Real& y, const Real& x) {
    Real r;
    mpfr_atan2(r.v, y.v, x.v, MPFR_RNDN);
    return r;
}

}  // namespace pna
