// 数值层：numextend bigfloat 的 RAII 包装（内核抽象的最小实现）。
#pragma once

extern "C" {
#include "nex/bigfloat/nex_bigfloat.h"
#include "nex/transcend/nex_transcend.h"
}

#include <stdexcept>
#include <string>

namespace pna {

struct NumError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// 全局精度（bit）；默认 256，见 §3.1 -p。
void set_precision(int bits);
const bigfloat_ctx_ty& ctx();

struct Real {
    bigfloat_ty v;

    Real();
    Real(const Real& o);
    Real(Real&& o) noexcept;
    ~Real();
    Real& operator=(const Real& o);
    Real& operator=(Real&& o) noexcept;

    static Real from_double(double d);
    static Real from_str(const char* s);
    static Real nan();
    static Real one();
    static Real zero();
};

Real operator+(const Real& a, const Real& b);
Real operator-(const Real& a, const Real& b);
Real operator*(const Real& a, const Real& b);
Real operator/(const Real& a, const Real& b);
Real neg(const Real& a);
Real abs(const Real& a);
// 幂：整数指数走平方乘（支持负底数）；否则 exp(e·log b)，b ≤ 0 得 NaN。
Real pow(const Real& base, const Real& e);

bool is_zero(const Real& a);
bool is_finite(const Real& a);
bool is_neg(const Real& a);
int sign(const Real& a);          // -1/0/+1，NaN 得 0
double to_double(const Real& a);
std::string to_str(const Real& a, int digits = 6);   // 有效数字位数

// 数值格式化：|x| ∈ [1e-4, 1e6) 用 %g，否则 %e；精确 0 输出 "0"（§3.7）。
std::string fmt(const Real& a);

}  // namespace pna
