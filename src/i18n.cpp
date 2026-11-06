// SPDX-License-Identifier: GPL-3.0-or-later
#include "i18n.hpp"

#include <string>

namespace pna {
namespace {
Lang g_lang = Lang::Zh;

const char* kHelpZh =
    "用法: polyno_err_ana [OPTIONS] -f FORMULA\n"
    "  -R, --real VARNAME ERR [VARNAME ERR ...]   实数变量与误差\n"
    "  -C, --comp VARNAME RE_ERR IM_ERR           复数变量与实/虚部误差\n"
    "  -a, --absolute | -r, --relative            误差模式（默认绝对）\n"
    "      --vars                                 显示符号推导\n"
    "      --estimated VARNAME=VALUE              估计点（可多次，支持 1+2i）\n"
    "  -p, --precision INT                        精度 bit（默认 256）\n"
    "  -k, --branch INT                           复数幂分支号（默认 0）\n"
    "      --real-root                            负实底数取实根（与 -k 互斥）\n"
    "      --max-terms INT                        符号展开项数上限（默认 200000）\n"
    "      --json                                 以 JSON 输出（机器可读）\n"
    "      --json-schema                          打印 --json 的 JSON Schema\n"
    "      --lim all | VAR:p0|n0|pinf|ninf        极限分类规格\n"
    "  -V, --version   -G, --gpl   -h, --help\n";

const char* kHelpEn =
    "Usage: polyno_err_ana [OPTIONS] -f FORMULA\n"
    "  -R, --real VARNAME ERR [VARNAME ERR ...]   real variable and its error\n"
    "  -C, --comp VARNAME RE_ERR IM_ERR           complex variable (real/imag error)\n"
    "  -a, --absolute | -r, --relative            error mode (default: absolute)\n"
    "      --vars                                 show symbolic derivation\n"
    "      --estimated VARNAME=VALUE              evaluation point (repeatable, e.g. 1+2i)\n"
    "  -p, --precision INT                        precision in bits (default 256)\n"
    "  -k, --branch INT                           branch index for complex powers (default 0)\n"
    "      --real-root                            real root for negative base (-k exclusive)\n"
    "      --max-terms INT                        cap on symbolic terms (default 200000)\n"
    "      --json                                 machine-readable JSON output\n"
    "      --json-schema                          print the JSON Schema of --json\n"
    "      --lim all | VAR:p0|n0|pinf|ninf        limit classification spec\n"
    "  -V, --version   -G, --gpl   -h, --help\n";

const char* kHelpZhMore =
    "      --lim VAR:zeros|infs|A..B              分组与区间（0..inf 即全部 4 状态）\n"
    "      --lang zh|en                           消息语言（默认 zh）\n";
const char* kHelpEnMore =
    "      --lim VAR:zeros|infs|A..B              groups and ranges (0..inf = all four)\n"
    "      --lang zh|en                           message language (default zh)\n";

// 拼接：基础用法 + 补充项
std::string build(bool en) {
    std::string s = en ? kHelpEn : kHelpZh;
    s += en ? kHelpEnMore : kHelpZhMore;
    return s;
}
}  // namespace

void set_lang(Lang l) { g_lang = l; }
Lang lang() { return g_lang; }

namespace {
struct MsgPair {
    const char* zh;
    const char* en;
};
// 前缀匹配（长串在前），保留中文消息的方向性后缀
const MsgPair kMsgs[] = {
    {"命令行错误: ", "command line error: "},
    {"无法解析数值: ", "cannot parse number: "},
    {"--estimated 值非法（第 ", "--estimated invalid value (column "},
    {"--estimated 需要 VARNAME=VALUE", "--estimated requires VARNAME=VALUE"},
    {"--lim all 与 --lim VAR:STATE 冲突", "--lim all conflicts with --lim VAR:STATE"},
    {"--lim 需要 all 或 VAR:STATE", "--lim needs all or VAR:STATE"},
    {"--lim 区间端点非法: ", "--lim invalid range endpoint: "},
    {"--lim 引用了未声明的变量: ", "--lim references undeclared variable: "},
    {"--lim 状态非法: ", "--lim invalid state: "},
    {"--lim 缺少状态: ", "--lim missing state: "},
    {"非法的 --lim 状态: ", "invalid --lim state: "},
    {"--max-terms 需要正整数", "--max-terms requires a positive integer"},
    {"--max-terms 必须 >= 1", "--max-terms must be >= 1"},
    {"--real-root 与 -k 互斥", "--real-root conflicts with -k"},
    {"--lang 只支持 zh / en", "--lang supports zh / en only"},
    {"-C 需要成组的 VARNAME RE_ERR IM_ERR", "-C needs VARNAME RE_ERR IM_ERR groups"},
    {"-R 需要成对的 VARNAME ERR", "-R needs VARNAME ERR pairs"},
    {"-a 与 -r 冲突", "-a conflicts with -r"},
    {"-p 需要整数", "-p requires an integer"},
    {"-p 必须 >= 2", "-p must be >= 2"},
    {"公式变量未声明: ", "formula variable not declared: "},
    {"变量重复声明: ", "duplicate variable: "},
    {"未知选项: ", "unknown option: "},
    {"缺少 -f FORMULA", "missing -f FORMULA"},
    {"至少需要一个 -R 或 -C 变量", "at least one -R or -C variable is required"},
    {"当前构建的内核是 ", "this build uses kernel "},
    {"，不支持 --kernel ", ", which does not support --kernel "},
};
}  // namespace

std::string tr_message(const std::string& zh) {
    if (g_lang != Lang::En) return zh;
    std::string rest = zh;
    std::string head;
    // 反复匹配前缀（如“命令行错误: 未知选项: --x”需要两次命中）
    for (int round = 0; round < 4; ++round) {
        bool hit = false;
        for (const auto& m : kMsgs) {
            const std::string from = m.zh;
            if (rest.rfind(from, 0) == 0) {
                head += m.en;
                rest = rest.substr(from.size());
                hit = true;
                break;
            }
        }
        if (!hit) break;
    }
    return head + rest;
}

const char* help_text() {
    static const std::string zh = build(false);
    static const std::string en = build(true);
    return g_lang == Lang::En ? en.c_str() : zh.c_str();
}

}  // namespace pna
