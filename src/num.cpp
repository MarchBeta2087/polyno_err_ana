// SPDX-License-Identifier: GPL-3.0-or-later
#include "num.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace pna {
namespace {
int g_bits = 256;

bigfloat_ctx_ty g_ctx{};
bool g_ready = false;
}  // namespace

void set_precision(int bits) {
    if (bits < 2) throw NumError("precision must be >= 2");
    g_bits = bits;
    bigfloat_ctx_make(&g_ctx, static_cast<size_t>(bits), 20, BIGFLOAT_ROUND_NEAREST_EVEN_E);
    g_ready = true;
}

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
    // bigfloat_from_f64 不带 ctx（按 binary64 精度构造），这里改走十进制字符串，
    // 保证与 MPFR 后端一样按目标精度（-p）舍入。
    char buf[40];
    std::snprintf(buf, sizeof buf, "%.17g", d);
    Real r;
    if (bigfloat_from_str(&r.v, buf, &ctx(), nullptr) != BIGFLOAT_OK_E)
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

bool re_eq(const Real& a, const Real& b) { return bigfloat_cmp(&a.v, &b.v) == 0; }
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
bool is_nan(const Real& a) { return bigfloat_is_nan(&a.v); }
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


Real r_exp(const Real& a) {
    Real r;
    bigfloat_exp(&r.v, &a.v, &ctx());
    return r;
}

Real r_log(const Real& a) {
    Real r;
    bigfloat_log(&r.v, &a.v, &ctx());
    return r;
}

Real r_sqrt(const Real& a) {
    Real r;
    bigfloat_sqrt(&r.v, &a.v, &ctx());
    return r;
}

Real r_sin(const Real& a) {
    Real r;
    bigfloat_sin(&r.v, &a.v, &ctx());
    return r;
}

Real r_cos(const Real& a) {
    Real r;
    bigfloat_cos(&r.v, &a.v, &ctx());
    return r;
}

Real r_atan2(const Real& y, const Real& x) {
    Real r;
    if (bigfloat_atan2(&r.v, &y.v, &x.v, &ctx()) != BIGFLOAT_OK_E) return Real::nan();
    return r;
}

const char* kernel_name() { return "numextend"; }

}  // namespace pna
