// SPDX-License-Identifier: GPL-3.0-or-later
// 极简 i18n：错误消息、帮助文本与输出块标题多语言；符号标签（δf、+0、c0…）不翻译。
#pragma once

#include <string>

namespace pna {

enum class Lang { Zh, En, Ja, Ko, Es, Pt };

void set_lang(Lang l);
Lang lang();

// 解析语言代码（zh/en/ja/ko/es/pt）。成功返回 true。
bool parse_lang(const std::string& code, Lang& out);
const char* lang_code(Lang l);

// 双参数形式：zh 与 en 都必须出现在 i18n.cpp 的表中，其它语言按 en 查表。
const char* tr(const char* zh, const char* en);

// 把内置中文消息（含动态后缀）翻成当前语言；未命中时原样返回。
std::string tr_message(const std::string& zh);

// 完整 --help 文本（按当前语言）
const char* help_text();

}  // namespace pna
