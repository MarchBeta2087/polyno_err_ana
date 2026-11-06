// 词法 / 语法 / AST / 数值求值。
#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "num.hpp"

namespace pna {

enum class NK { Num, Var, Add, Sub, Mul, Div, Pow };

struct Node;
using Expr = std::shared_ptr<const Node>;

struct Node {
    NK k{};
    Real num{};
    std::string var;
    Expr a, b;      // Pow: a=底数, b=指数（常数子树）
    size_t col{};
};

struct ParseError : std::runtime_error {
    size_t col;
    ParseError(std::string m, size_t c) : std::runtime_error(std::move(m)), col(c) {}
};

struct EvalError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// 解析完整公式；错误抛 ParseError（含 1-based 列号）。
Expr parse(const std::string& src);

Expr mk_num(double d);
Expr mk_var(std::string name);

bool has_var(const Expr& e);
void collect_vars(const Expr& e, std::vector<std::string>& out);
// 无变量时求值；有变量抛 ParseError。
double const_value(const Expr& e);

using Env = std::vector<std::pair<std::string, Real>>;
const Real* lookup(const Env& env, const std::string& name);
Real eval(const Expr& e, const Env& env);

std::string to_string(const Expr& e);
std::string fmt_num(double d);

// 结构相等（按规范化字符串比较，atom 去重依赖它）。
bool same(const Expr& a, const Expr& b);

}  // namespace pna
