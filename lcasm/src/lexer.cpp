#include "lexer.hpp"

#include "general.hpp"

void Lexer::load_source(std::string&& src) {
    this->src = std::move(src);
    curI = 0;
    line = 1;
}

void Lexer::error(const char* msg) { fatal("Error on line %d: %s", line, msg); }

void Lexer::error(std::string&& msg) { error(msg.c_str()); }

bool Lexer::is_break(char ch) {
    return ch == '\n' || ch == ' ' || ch == '\r' || ch == '\t' || ch == ';' || ch == ',';
}

bool Lexer::is_dec(char ch) { return (ch & 0xF0) == 0x30 && (ch & 0xF) <= 9; }

bool Lexer::check_ch(char ch) { return curI < src.size() && src[curI] == ch; }

void Lexer::shift_till_token() {
    bool isComment = false;
    while (curI < src.size()) {
        switch (src[curI]) {
            case '\n':
                if (isComment) {
                    isComment = false;
                }
                line++;
                break;
            case ';':
                isComment = true;
            case ' ':
            case '\r':
            case '\t':
                break;
            default:
                if (!isComment) {
                    return;
                }
                break;
        }
        curI++;
    }
}

namespace {
constexpr char NUM_KEYWORDS = 22;
struct {
    const char* keyword;
    TokenKind kind;
} keywordMap[NUM_KEYWORDS] = {
    {"ADD", TokenKind::ADD},    {"AND", TokenKind::AND},
    {"NOT", TokenKind::NOT},    {"LD", TokenKind::LD},
    {"LDR", TokenKind::LDR},    {"LDI", TokenKind::LDI},
    {"LEA", TokenKind::LEA},    {"ST", TokenKind::ST},
    {"STR", TokenKind::STR},    {"STI", TokenKind::STI},
    {"JMP", TokenKind::JMP},    {"JSR", TokenKind::JSR},
    {"JSRR", TokenKind::JSRR},  {"RET", TokenKind::RET},

    {"NOP", TokenKind::NOP},    {"TRAP", TokenKind::TRAP},
    {"HALT", TokenKind::HALT},

    {".ORIG", TokenKind::ORIG}, {".FILL", TokenKind::FILL},
    {".BLKW", TokenKind::BLKW}, {".STRINGZ", TokenKind::STRINGZ},
    {".END", TokenKind::END},
};
}  // namespace

Token Lexer::eat_label_or_keyword() {
    size_t curLen = 1;
    while (curI + curLen < src.size() && !is_break(src[curI + curLen])) {
        curLen++;
    }
    std::string_view tokenStr(&src[curI], curLen);
    curI += curLen;
    for (size_t i = 0; i < NUM_KEYWORDS; i++) {
        const char* keyword = keywordMap[i].keyword;
        bool match = true;
        size_t k = 0;
        while (*keyword) {
            if ((tokenStr[k] & 0xE0) == 0x60) {
                if (tokenStr[k] - 0x20 != *keyword) {
                    match = false;
                    break;
                }
            } else if (tokenStr[k] != *keyword) {
                match = false;
                break;
            }
            k++;
            keyword++;
        }
        if (match && k == tokenStr.length()) {
            return Token(keywordMap[i].kind);
        }
    }
    return Token(TokenKind::LABEL, std::move(tokenStr));
}

Token Lexer::eat_dec_literal() {
    bool negate = false;
    if (check_ch('-')) {
        negate = true;
        curI++;
    }
    int decVal = 0;
    while (curI < src.size() && !is_break(src[curI])) {
        char decCh = src[curI++];
        if (!is_dec(decCh)) {
            error("Expected decimal number");
        }
        decVal = (decVal * 10) + (decCh & 0xF);
    }
    if (negate) {
        decVal = -decVal;
    }
    if (check_overflow(decVal, 16)) {
        error("Decimal literal must be storable in 16-bits");
    }
    return Token(TokenKind::LITERAL, decVal);
}

Token Lexer::eat_token() {
    shift_till_token();
    if (curI < src.size()) {
        switch (src[curI]) {
            case ',':
                curI++;
                return Token(TokenKind::COMMA);
            case 'R':
                if (curI < src.size()) {
                    char numCh = src[curI + 1];
                    if (is_dec(numCh)) {
                        if ((numCh & 0xF) >= 8) {
                            error("Register number must be between [0-7] inclusive");
                        }
                        curI += 2;
                        return Token(TokenKind::REG, numCh & 0xF);
                    }
                }
                return eat_label_or_keyword();
            case 'B':
                if (curI < src.size() && src[curI + 1] == 'R') {
                    curI += 2;
                    uint8_t cc = 0;
                    if (check_ch('n')) {
                        cc |= 0x4;
                        curI++;
                    }
                    if (check_ch('z')) {
                        cc |= 0x2;
                        curI++;
                    }
                    if (check_ch('p')) {
                        cc |= 0x1;
                        curI++;
                    }
                    if (curI < src.size() && is_break(src[curI])) {
                        return Token(TokenKind::BR, cc);
                    }
                }
                return eat_label_or_keyword();
            case '"': {
                size_t curLen = 1;
                while (curI + curLen < src.size() && src[curI + curLen] != '"') {
                    if (src[curI + curLen] == '\n') {
                        error("Must end string with \" on one line");
                    }
                    curLen++;
                }
                if (curLen == 1) {
                    error("String literal length must be greater than 1");
                }
                std::string_view str(&src[curI + 1], curLen - 1);
                curI += curLen;
                curI++;  // Increment past second "
                return Token(TokenKind::STR_LITERAL, std::move(str));
            } break;
            case '#':
                curI++;
            case '-':
                return eat_dec_literal();
            case 'x': {
                curI++;
                int hexVal = 0;
                char numDigits = 0;
                while (curI < src.size() && !is_break(src[curI])) {
                    char hexCh = src[curI++];
                    if (is_dec(hexCh)) {
                        hexVal = (hexVal << 4) + (hexCh & 0xF);
                    } else if ((hexCh & 0xF0) == 0x40 && (hexCh & 0x7) <= 6) {
                        hexVal = (hexVal << 4) + (hexCh & 0xF) + 9;
                    } else {
                        error("Expected hexidecimal number after x");
                    }
                    if (++numDigits > 4) {
                        error("Hexidecimal literal must be storable in 16-bits");
                    }
                }
                return Token(TokenKind::LITERAL, hexVal);
            } break;
            default:
                if (is_dec(src[curI])) {
                    return eat_dec_literal();
                }
                return eat_label_or_keyword();
        }
    }
    return Token(TokenKind::E_O_F);
}
