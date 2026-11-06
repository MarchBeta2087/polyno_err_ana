// SPDX-License-Identifier: GPL-3.0-or-later
// 数值层：numextend bigfloat 的 RAII 包装 + 复数运算（含分支幂）。
#pragma once

#ifdef POLYNO_KERNEL_MPFR
#include <gmp.h>
#include <mpfr.h>
#else
extern "C" {
#include "nex/bigfloat/nex_bigfloat.h"
#include "nex/transcend/nex_transcend.h"
}
#endif

#include <stdexcept>
#include <string>

namespace pna {

struct NumError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// 全局精度（bit）；默认 256，见 §3.1 -p。
void set_precision(int bits);

#ifndef POLYNO_KERNEL_MPFR
const bigfloat_ctx_ty& ctx();
#endif

// 复数幂的分支号（§3.5），全局唯一，求值/符号/边界共用。
void set_branch(int k);
int branch_k();

// --real-root：负实底数 + 非整数有理指数时取实根（§3.2）。
void set_real_root(bool on);
bool real_root_enabled();

struct Real {
#ifdef POLYNO_KERNEL_MPFR
    mpfr_t v;
#else
    bigfloat_ty v;
#endif

    Real();
    Real(const Real& o);
    Real(Real&& o) noexcept;
    ~Real();
    Real& operator=(const Real& o);
    Real& operator=(Real&& o) noexcept;

    static Real from_double(double d);
    static Real from_str(const char* s);
    static Real nan();
    static Real inf(int sign);
    static Real one();
    static Real zero();
};

// 后端名（"numextend" / "mpfr"）与后端原语。
const char* kernel_name();
Real r_exp(const Real& a);
Real r_log(const Real& a);
Real r_sqrt(const Real& a);
Real r_sin(const Real& a);
Real r_cos(const Real& a);
Real r_atan2(const Real& y, const Real& x);

Real operator+(const Real& a, const Real& b);
Real operator-(const Real& a, const Real& b);
Real operator*(const Real& a, const Real& b);
Real operator/(const Real& a, const Real& b);
Real neg(const Real& a);
Real abs(const Real& a);
// 实幂：整数指数走平方乘（支持负底数）；否则 exp(e·log b)，b < 0 得 NaN。
Real pow(const Real& base, const Real& e);

bool re_eq(const Real& a, const Real& b);   // 精确相等（用于符号层原子比较）
bool is_zero(const Real& a);
bool is_finite(const Real& a);
bool is_nan(const Real& a);
bool is_neg(const Real& a);
bool is_integer(const Real& a);
int sign(const Real& a);          // -1/0/+1，NaN 得 0
double to_double(const Real& a);
std::string to_str(const Real& a, int digits = 6);

// 数值格式化：|x| ∈ [1e-3, 1e6) 用 %g，否则 %e；精确 0 输出 "0"（§3.7）。
std::string fmt(const Real& a);

// ------------------------------------------------------------------ //
// 复数：re + im·i，分量均为任意精度 Real。
// ------------------------------------------------------------------ //
struct Cplx {
    Real re, im;
};

Cplx c_from_double(double re, double im = 0.0);
Cplx c_from_str(const char* s);
Cplx c_nan();
Cplx c_zero();
Cplx c_one();
Cplx c_i();

Cplx operator+(const Cplx& a, const Cplx& b);
Cplx operator-(const Cplx& a, const Cplx& b);
Cplx operator*(const Cplx& a, const Cplx& b);
Cplx operator/(const Cplx& a, const Cplx& b);
Cplx c_neg(const Cplx& a);
Real c_abs(const Cplx& a);        // |z|
Real c_arg(const Cplx& a);        // atan2(im, re) ∈ (−π, π]
Cplx c_pow(const Cplx& base, const Cplx& e);   // 尊重 branch_k()
Cplx c_exp(const Cplx& a);
Cplx c_log(const Cplx& a);
Cplx c_scale(const Cplx& a, const Real& k);

bool c_is_zero(const Cplx& a);
bool c_is_real(const Cplx& a);    // im == 0（含 −0）
bool c_is_finite(const Cplx& a);
std::string c_fmt(const Cplx& a);     // 实数则同 fmt；否则 "(a+bi)"

Real pi();

}  // namespace pna
