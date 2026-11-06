// SPDX-License-Identifier: GPL-3.0-or-later
#include "i18n.hpp"

#include <cstring>

namespace pna {
namespace {

Lang g_lang = Lang::Zh;

struct Row {
    const char* zh;
    const char* en;
    const char* ja;
    const char* ko;
    const char* es;
    const char* pt;
};

const char* col(const Row& r, Lang l) {
    switch (l) {
        case Lang::Zh: return r.zh;
        case Lang::En: return r.en;
        case Lang::Ja: return r.ja;
        case Lang::Ko: return r.ko;
        case Lang::Es: return r.es;
        case Lang::Pt: return r.pt;
    }
    return r.en;
}

// 键：zh 列用于中文模式与前缀匹配，en 列用于其它语言查表。
const Row kRows[] = {
    // ---- 命令行 ----
    {"命令行错误: ", "command line error: ", "コマンドラインエラー: ", "명령줄 오류: ",
     "error de línea de comandos: ", "erro de linha de comando: "},
    {"无法解析数值: ", "cannot parse number: ", "数値を解釈できません: ",
     "숫자를 해석할 수 없습니다: ", "no se puede interpretar el número: ",
     "não foi possível interpretar o número: "},
    {"-R 需要成对的 VARNAME ERR", "-R needs VARNAME ERR pairs",
     "-R には VARNAME ERR の組が必要です", "-R에는 VARNAME ERR 쌍이 필요합니다",
     "-R requiere pares VARNAME ERR", "-R requer pares VARNAME ERR"},
    {"-C 需要成组的 VARNAME RE_ERR IM_ERR", "-C needs VARNAME RE_ERR IM_ERR groups",
     "-C には VARNAME RE_ERR IM_ERR の組が必要です", "-C에는 VARNAME RE_ERR IM_ERR 쌍이 필요합니다",
     "-C requiere grupos VARNAME RE_ERR IM_ERR", "-C requer grupos VARNAME RE_ERR IM_ERR"},
    {"-a 与 -r 冲突", "-a conflicts with -r", "-a と -r は同時に指定できません",
     "-a와 -r은 함께 쓸 수 없습니다", "-a entra en conflicto con -r",
     "-a entra em conflito com -r"},
    {"-p 需要整数", "-p requires an integer", "-p には整数が必要です", "-p에는 정수가 필요합니다",
     "-p requiere un entero", "-p requer um inteiro"},
    {"-p 必须 >= 2", "-p must be >= 2", "-p は 2 以上である必要があります",
     "-p는 2 이상이어야 합니다", "-p debe ser >= 2", "-p deve ser >= 2"},
    {"--max-terms 需要正整数", "--max-terms requires a positive integer",
     "--max-terms には正の整数が必要です", "--max-terms에는 양의 정수가 필요합니다",
     "--max-terms requiere un entero positivo", "--max-terms requer um inteiro positivo"},
    {"--max-terms 必须 >= 1", "--max-terms must be >= 1", "--max-terms は 1 以上である必要があります",
     "--max-terms는 1 이상이어야 합니다", "--max-terms debe ser >= 1", "--max-terms deve ser >= 1"},
    {"--real-root 与 -k 互斥", "--real-root conflicts with -k",
     "--real-root と -k は同時に指定できません", "--real-root와 -k는 함께 쓸 수 없습니다",
     "--real-root entra en conflicto con -k", "--real-root entra em conflito com -k"},
    {"--lang 只支持 zh / en / ja / ko / es / pt", "--lang supports zh / en / ja / ko / es / pt",
     "--lang は zh / en / ja / ko / es / pt に対応しています",
     "--lang은 zh / en / ja / ko / es / pt를 지원합니다", "--lang admite zh / en / ja / ko / es / pt",
     "--lang aceita zh / en / ja / ko / es / pt"},
    {"--estimated 需要 VARNAME=VALUE", "--estimated requires VARNAME=VALUE",
     "--estimated には VARNAME=VALUE が必要です", "--estimated에는 VARNAME=VALUE가 필요합니다",
     "--estimated requiere VARNAME=VALUE", "--estimated requer VARNAME=VALUE"},
    {"--lim all 与 --lim VAR:STATE 冲突", "--lim all conflicts with --lim VAR:STATE",
     "--lim all と --lim VAR:STATE は同時に指定できません",
     "--lim all과 --lim VAR:STATE는 함께 쓸 수 없습니다",
     "--lim all entra en conflicto con --lim VAR:STATE",
     "--lim all entra em conflito com --lim VAR:STATE"},
    {"--lim 需要 all 或 VAR:STATE", "--lim needs all or VAR:STATE",
     "--lim には all または VAR:STATE が必要です", "--lim에는 all 또는 VAR:STATE가 필요합니다",
     "--lim requiere all o VAR:STATE", "--lim requer all ou VAR:STATE"},
    {"--lim 区间端点非法: ", "--lim invalid range endpoint: ", "--lim の区間端点が不正です: ",
     "--lim 범위 끝점이 잘못되었습니다: ", "--lim extremo de rango no válido: ",
     "--lim extremo de intervalo inválido: "},
    {"--lim 引用了未声明的变量: ", "--lim references undeclared variable: ",
     "--lim が未宣言の変数を参照しています: ", "--lim이 선언되지 않은 변수를 참조합니다: ",
     "--lim hace referencia a una variable no declarada: ",
     "--lim faz referência a uma variável não declarada: "},
    {"--lim 状态非法: ", "--lim invalid state: ", "--lim の状態が不正です: ",
     "--lim 상태가 잘못되었습니다: ", "--lim estado no válido: ", "--lim estado inválido: "},
    {"--lim 缺少状态: ", "--lim missing state: ", "--lim に状態がありません: ",
     "--lim에 상태가 없습니다: ", "--lim falta el estado: ", "--lim falta o estado: "},
    {"非法的 --lim 状态: ", "invalid --lim state: ", "--lim の状態が不正です: ",
     "--lim 상태가 잘못되었습니다: ", "estado de --lim no válido: ", "estado de --lim inválido: "},
    {"公式变量未声明: ", "formula variable not declared: ",
     "式中の変数が宣言されていません: ", "수식 변수가 선언되지 않았습니다: ",
     "variable de la fórmula no declarada: ", "variável da fórmula não declarada: "},
    {"变量重复声明: ", "duplicate variable: ", "変数が重複しています: ", "변수가 중복되었습니다: ",
     "variable duplicada: ", "variável duplicada: "},
    {"未知选项: ", "unknown option: ", "不明なオプション: ", "알 수 없는 옵션: ",
     "opción desconocida: ", "opção desconhecida: "},
    {"缺少 -f FORMULA", "missing -f FORMULA", "-f FORMULA がありません",
     "-f FORMULA가 없습니다", "falta -f FORMULA", "falta -f FORMULA"},
    {"至少需要一个 -R 或 -C 变量", "at least one -R or -C variable is required",
     "-R または -C の変数が少なくとも 1 つ必要です", "-R 또는 -C 변수가 최소 하나 필요합니다",
     "se requiere al menos una variable -R o -C", "é necessário pelo menos uma variável -R ou -C"},
    {"当前构建的内核是 %s，不支持 --kernel %s", "this build uses kernel %s, which does not support --kernel %s",
     "このビルドのカーネルは %s です（--kernel %s は未対応）",
     "이 빌드의 커널은 %s이며 --kernel %s은(는) 지원하지 않습니다",
     "esta compilación usa el núcleo %s, que no admite --kernel %s",
     "esta compilação usa o núcleo %s, que não aceita --kernel %s"},
    {"命令行错误: --estimated 值非法（列 %zu）: %s",
     "command line error: --estimated invalid value (column %zu): %s",
     "コマンドラインエラー: --estimated の値が不正です（%zu 列目）: %s",
     "명령줄 오류: --estimated 값이 잘못되었습니다(열 %zu): %s",
     "error de línea de comandos: valor de --estimated no válido (columna %zu): %s",
     "erro de linha de comando: valor de --estimated inválido (coluna %zu): %s"},

    // ---- 解析 / 求值 / 数值错误 ----
    {"意外字符 '", "unexpected character '", "予期しない文字 '", "예기치 않은 문자 '",
     "carácter inesperado '", "caractere inesperado '"},
    {"指数必须是常数", "exponent must be a constant", "指数は定数でなければなりません",
     "지수는 상수여야 합니다", "el exponente debe ser una constante",
     "o expoente deve ser uma constante"},
    {"不支持的函数: ", "unsupported function: ", "未対応の関数: ", "지원하지 않는 함수: ",
     "función no admitida: ", "função não suportada: "},
    {"非法变量名: ", "illegal variable name: ", "不正な変数名: ", "잘못된 변수 이름: ",
     "nombre de variable no válido: ", "nome de variável inválido: "},
    {"公式意外结束", "unexpected end of formula", "式が予期せず終了しました",
     "수식이 예기치 않게 끝났습니다", "fin de fórmula inesperado", "fim de fórmula inesperado"},
    {"意外标记 '", "unexpected token '", "予期しないトークン '", "예기치 않은 토큰 '",
     "token inesperado '", "token inesperado '"},
    {"缺少 ')'", "expected ')'", "')' が必要です", "')'가 필요합니다", "se esperaba ')'",
     "esperava-se ')'"},
    {"意外的尾部标记 '", "unexpected trailing token '", "末尾に予期しないトークン '",
     "끝에 예기치 않은 토큰 '", "token final inesperado '", "token final inesperado '"},
    {"坏节点", "bad node", "不正なノード", "잘못된 노드", "nodo no válido", "nó inválido"},
    {"非常数指数：不支持符号展开", "non-finite exponent: symbolic form unsupported",
     "非有限指数: 記号展開は未対応です", "유한하지 않은 지수: 기호 전개를 지원하지 않습니다",
     "exponente no finito: forma simbólica no admitida",
     "expoente não finito: forma simbólica não suportada"},
    {"符号展开项数超过上限（可用 --max-terms 调整）",
     "symbolic expansion exceeded the term cap (use --max-terms)",
     "記号展開の項数が上限を超えました（--max-terms で調整）",
     "기호 전개 항 수가 상한을 초과했습니다(--max-terms로 조정)",
     "la expansión simbólica superó el límite de términos (use --max-terms)",
     "a expansão simbólica excedeu o limite de termos (use --max-terms)"},
    {"内存不足", "out of memory", "メモリ不足", "메모리 부족", "sin memoria", "sem memória"},
    {"精度必须 >= 2", "precision must be >= 2", "精度は 2 以上である必要があります",
     "정밀도는 2 이상이어야 합니다", "la precisión debe ser >= 2", "a precisão deve ser >= 2"},

    // ---- 输出块 ----
    {"  精确边界（单变量变，其余固定在估计点）",
     "  Exact boundary (one variable perturbed, others fixed)",
     "  厳密な境界（1 変数のみ変化、他は評価点に固定）",
     "  정확한 경계(한 변수만 변화, 나머지는 추정점 고정)",
     "  Cota exacta (una variable perturbada, las demás fijas)",
     "  Limite exato (uma variável perturbada, as demais fixas)"},
    {"\n  极限分类（单变量变，其余 δ=0）",
     "\n  Limit classification (one variable perturbed, other δ=0)",
     "\n  極限の分類（1 変数のみ変化、他は δ=0）",
     "\n  극한 분류(한 변수만 변화, 나머지 δ=0)",
     "\n  Clasificación de límites (una variable perturbada, otra δ=0)",
     "\n  Classificação de limites (uma variável perturbada, outra δ=0)"},
    {"%-9s= N/A（符号展开不可用：非整数指数或项数超限）",
     "%-9s= N/A (symbolic expansion unavailable)",
     "%-9s= N/A（記号展開は利用できません）", "%-9s= N/A(기호 전개를 사용할 수 없음)",
     "%-9s= N/A (expansión simbólica no disponible)",
     "%-9s= N/A (expansão simbólica indisponível)"},
    {"δ%s/%s   = N/A（估计点为 0）", "δ%s/%s   = N/A (estimate is 0)",
     "δ%s/%s   = N/A（評価点が 0）", "δ%s/%s   = N/A(추정점이 0)",
     "δ%s/%s   = N/A (el punto de evaluación es 0)",
     "δ%s/%s   = N/A (o ponto de estimativa é 0)"},
    {"  [δv = 0（估计点为 0，相对误差失效）]",
     "  [δv = 0: estimate is 0, relative error not applicable]",
     "  [δv = 0（評価点が 0 のため相対誤差は無効）]",
     "  [δv = 0(추정점이 0이라 상대 오차 무효)]",
     "  [δv = 0: el punto de evaluación es 0, error relativo no aplicable]",
     "  [δv = 0: o ponto de estimativa é 0, erro relativo não aplicável]"},
    {"解析错误（第 %zu 列）: %s\n", "parse error (column %zu): %s\n",
     "構文エラー（%zu 列目）: %s\n", "구문 오류(열 %zu): %s\n",
     "error de sintaxis (columna %zu): %s\n", "erro de sintaxe (coluna %zu): %s\n"},
    {"求值错误: %s\n", "evaluation error: %s\n", "評価エラー: %s\n", "평가 오류: %s\n",
     "error de evaluación: %s\n", "erro de avaliação: %s\n"},
    {"数值错误: %s\n", "numeric error: %s\n", "数値エラー: %s\n", "수치 오류: %s\n",
     "error numérico: %s\n", "erro numérico: %s\n"},
};

const char* lookup_en(const char* en) {
    for (const auto& r : kRows)
        if (std::strcmp(r.en, en) == 0) return col(r, g_lang);
    return nullptr;
}

const char* help_zh =
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
    "      --lim VAR:zeros|infs|A..B              分组与区间（0..inf 即全部 4 状态）\n"
    "      --lang zh|en|ja|ko|es|pt               消息语言（默认 zh）\n"
    "  -V, --version   -G, --gpl   -h, --help\n";

const char* help_en =
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
    "      --lim VAR:zeros|infs|A..B              groups and ranges (0..inf = all four)\n"
    "      --lang zh|en|ja|ko|es|pt               message language (default zh)\n"
    "  -V, --version   -G, --gpl   -h, --help\n";

const char* help_ja =
    "使い方: polyno_err_ana [OPTIONS] -f FORMULA\n"
    "  -R, --real VARNAME ERR [VARNAME ERR ...]   実数変数と誤差\n"
    "  -C, --comp VARNAME RE_ERR IM_ERR           複素変数（実部・虚部の誤差）\n"
    "  -a, --absolute | -r, --relative            誤差モード（既定: 絶対）\n"
    "      --vars                                 記号微分を表示\n"
    "      --estimated VARNAME=VALUE              評価点（複数可、例: 1+2i）\n"
    "  -p, --precision INT                        精度 bit（既定 256）\n"
    "  -k, --branch INT                           複素累乗の分岐番号（既定 0）\n"
    "      --real-root                            負の底で実根を取る（-k と排他）\n"
    "      --max-terms INT                        記号展開の項数上限（既定 200000）\n"
    "      --json                                 JSON 出力（機械可読）\n"
    "      --json-schema                          --json の JSON Schema を表示\n"
    "      --lim all | VAR:p0|n0|pinf|ninf        極限分類の指定\n"
    "      --lim VAR:zeros|infs|A..B              グループと区間（0..inf は 4 状態すべて）\n"
    "      --lang zh|en|ja|ko|es|pt               メッセージ言語（既定 zh）\n"
    "  -V, --version   -G, --gpl   -h, --help\n";

const char* help_ko =
    "사용법: polyno_err_ana [OPTIONS] -f FORMULA\n"
    "  -R, --real VARNAME ERR [VARNAME ERR ...]   실수 변수와 오차\n"
    "  -C, --comp VARNAME RE_ERR IM_ERR           복소 변수(실수부/허수부 오차)\n"
    "  -a, --absolute | -r, --relative            오차 모드(기본: 절대)\n"
    "      --vars                                 기호 미분 표시\n"
    "      --estimated VARNAME=VALUE              추정점(반복 가능, 예: 1+2i)\n"
    "  -p, --precision INT                        정밀도 bit(기본 256)\n"
    "  -k, --branch INT                           복소 거듭제곱의 분기 번호(기본 0)\n"
    "      --real-root                            음수 밑에서 실근 사용(-k와 배타)\n"
    "      --max-terms INT                        기호 전개 항 수 상한(기본 200000)\n"
    "      --json                                 JSON 출력(기계 판독 가능)\n"
    "      --json-schema                          --json의 JSON Schema 출력\n"
    "      --lim all | VAR:p0|n0|pinf|ninf        극한 분류 지정\n"
    "      --lim VAR:zeros|infs|A..B              그룹과 범위(0..inf는 4개 상태 전부)\n"
    "      --lang zh|en|ja|ko|es|pt               메시지 언어(기본 zh)\n"
    "  -V, --version   -G, --gpl   -h, --help\n";

const char* help_es =
    "Uso: polyno_err_ana [OPTIONS] -f FORMULA\n"
    "  -R, --real VARNAME ERR [VARNAME ERR ...]   variable real y su error\n"
    "  -C, --comp VARNAME RE_ERR IM_ERR           variable compleja (error real/imag)\n"
    "  -a, --absolute | -r, --relative            modo de error (por defecto: absoluto)\n"
    "      --vars                                 mostrar derivación simbólica\n"
    "      --estimated VARNAME=VALUE              punto de evaluación (repetible, p. ej. 1+2i)\n"
    "  -p, --precision INT                        precisión en bits (por defecto 256)\n"
    "  -k, --branch INT                           índice de rama para potencias complejas (0)\n"
    "      --real-root                            raíz real para base negativa (excluye -k)\n"
    "      --max-terms INT                        límite de términos simbólicos (200000)\n"
    "      --json                                 salida JSON (legible por máquina)\n"
    "      --json-schema                          imprime el JSON Schema de --json\n"
    "      --lim all | VAR:p0|n0|pinf|ninf        especificación de clasificación de límites\n"
    "      --lim VAR:zeros|infs|A..B              grupos y rangos (0..inf = los cuatro estados)\n"
    "      --lang zh|en|ja|ko|es|pt               idioma de los mensajes (por defecto zh)\n"
    "  -V, --version   -G, --gpl   -h, --help\n";

const char* help_pt =
    "Uso: polyno_err_ana [OPTIONS] -f FORMULA\n"
    "  -R, --real VARNAME ERR [VARNAME ERR ...]   variável real e seu erro\n"
    "  -C, --comp VARNAME RE_ERR IM_ERR           variável complexa (erro real/imaginário)\n"
    "  -a, --absolute | -r, --relative            modo de erro (padrão: absoluto)\n"
    "      --vars                                 mostrar derivação simbólica\n"
    "      --estimated VARNAME=VALUE              ponto de avaliação (repetível, ex.: 1+2i)\n"
    "  -p, --precision INT                        precisão em bits (padrão 256)\n"
    "  -k, --branch INT                           índice de ramo para potências complexas (0)\n"
    "      --real-root                            raiz real para base negativa (exclui -k)\n"
    "      --max-terms INT                        limite de termos simbólicos (200000)\n"
    "      --json                                 saída JSON (legível por máquina)\n"
    "      --json-schema                          imprime o JSON Schema de --json\n"
    "      --lim all | VAR:p0|n0|pinf|ninf        especificação de classificação de limites\n"
    "      --lim VAR:zeros|infs|A..B              grupos e intervalos (0..inf = os quatro estados)\n"
    "      --lang zh|en|ja|ko|es|pt               idioma das mensagens (padrão zh)\n"
    "  -V, --version   -G, --gpl   -h, --help\n";

}  // namespace

void set_lang(Lang l) { g_lang = l; }
Lang lang() { return g_lang; }

const char* lang_code(Lang l) {
    switch (l) {
        case Lang::Zh: return "zh";
        case Lang::En: return "en";
        case Lang::Ja: return "ja";
        case Lang::Ko: return "ko";
        case Lang::Es: return "es";
        case Lang::Pt: return "pt";
    }
    return "en";
}

bool parse_lang(const std::string& code, Lang& out) {
    if (code == "zh") { out = Lang::Zh; return true; }
    if (code == "en") { out = Lang::En; return true; }
    if (code == "ja") { out = Lang::Ja; return true; }
    if (code == "ko") { out = Lang::Ko; return true; }
    if (code == "es") { out = Lang::Es; return true; }
    if (code == "pt") { out = Lang::Pt; return true; }
    return false;
}

const char* tr(const char* zh, const char* en) {
    if (g_lang == Lang::Zh) return zh;
    if (g_lang == Lang::En) return en;
    const char* t = lookup_en(en);
    return t ? t : en;
}

std::string tr_message(const std::string& zh) {
    if (g_lang == Lang::Zh) return zh;
    std::string rest = zh;
    std::string head;
    // 反复匹配前缀（如“命令行错误: 未知选项: --x”需要两次命中）
    for (int round = 0; round < 4; ++round) {
        bool hit = false;
        for (const auto& r : kRows) {
            const std::string from = r.zh;
            if (rest.rfind(from, 0) == 0) {
                head += col(r, g_lang);
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
    switch (g_lang) {
        case Lang::Zh: return help_zh;
        case Lang::En: return help_en;
        case Lang::Ja: return help_ja;
        case Lang::Ko: return help_ko;
        case Lang::Es: return help_es;
        case Lang::Pt: return help_pt;
    }
    return help_en;
}

}  // namespace pna
