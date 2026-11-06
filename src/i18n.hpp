// SPDX-License-Identifier: GPL-3.0-or-later
// 极简 i18n：错误消息与帮助文本二选一；输出标签（δf、+0、c0…）为符号记法，不翻译。
#pragma once

#include <string>

namespace pna {

enum class Lang { Zh, En };

void set_lang(Lang l);
Lang lang();

inline const char* tr(const char* zh, const char* en) { return lang() == Lang::En ? en : zh; }

// 把内置中文消息（含后缀）翻成英文；未命中时原样返回。仅对 En 生效。
std::string tr_message(const std::string& zh);

// 完整 --help 文本（按当前语言）
const char* help_text();

}  // namespace pna
