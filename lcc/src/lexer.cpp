#include "lexer.hpp"

const char* token_type_to_str(TokenType type) {
    switch (type) {
        case TokenType::L_CURLY:
            return "{";
        case TokenType::R_CURLY:
            return "}";
        case TokenType::L_PARENS:
            return "(";
        case TokenType::R_PARENS:
            return ")";
        case TokenType::IF:
            return "if";
        case TokenType::ELSE:
            return "else";
        case TokenType::WHILE:
            return "while";
        case TokenType::INT:
            return "int";
        case TokenType::VOID:
            return "void";
        case TokenType::IDENTIFIER:
            return "[IDENTIFIER]";
        case TokenType::INT_LITERAL:
            return "[NUM_LITERAL]";
        case TokenType::STR_LITERAL:
            return "[STR_LITERAL]";
        case TokenType::ASTERISK:
            return "*";
        case TokenType::OP_ASSIGN:
            return "=";
        case TokenType::BIT_AND:
            return "&";
        case TokenType::BIT_NOT:
            return "~";
        case TokenType::OP_ADD:
            return "+";
        case TokenType::OP_ADD_ADD:
            return "++";
        case TokenType::OP_ADD_EQUALS:
            return "+=";
        case TokenType::OP_SUB_NEGATE:
            return "-";
        case TokenType::OP_SUB_SUB:
            return "--";
        case TokenType::OP_SUB_EQUALS:
            return "-=";
        case TokenType::COND_NOT:
            return "!";
        case TokenType::COND_EQUAL:
            return "==";
        case TokenType::COND_GT:
            return ">";
        case TokenType::COND_LT:
            return "<";
        case TokenType::COND_GT_EQUAL:
            return ">=";
        case TokenType::COND_LT_EQUAL:
            return "<=";
        case TokenType::RETURN:
            return "return";
        case TokenType::SEMI_COLON:
            return ";";
        case TokenType::ERROR:
            return "[ERROR]";
        case TokenType::END:
            return "[END]";
        default:
            fatal("Unknown token_to_str conversion: %02x\n", type);
    }
}

void Lexer::load_source(std::string&& source) {
    this->source = std::move(source);
    peekedToken = nullptr;
    line = 1;
    col = 0;
    curI = 0;
    curOff = 0;
}

void Lexer::error(std::string&& msg) { fatal(("(l:%d,c:%d): " + msg).c_str(), line, col); }

void Lexer::literal_overflow_error(int val) {
    error(("Numeric literal " + std::to_string(val) +
           " is larger than the allowed signed 16-bit max magnitude in this compiler"));
}

void Lexer::eat_ch() {
    col++;
    curOff++;
}

bool Lexer::check_ch(char ch) {
    return (curI + curOff) < source.size() && source[curI + curOff] == ch;
}

bool Lexer::check_ident(const char* ident) {
    while (*ident) {
        if (!check_ch(*ident)) {
            curOff = 0;
            return false;
        }
        curOff++;
        ident++;
    }
    col += curOff;
    return true;
}

std::unique_ptr<Token> Lexer::make_token(TokenType type) {
    int oOff = curOff;
    curI += curOff;
    curOff = 0;
    return std::make_unique<Token>(type, line, col - oOff);
}

namespace {
bool is_whitespace(char ch) { return ch == ' ' || ch == '\r' || ch == '\t' || ch == '\n'; }

bool is_letter(char ch) {
    if ((ch & 0xE0) == 0x40 || (ch & 0xE0) == 0x60) {
        if (0x1 <= (ch & 0x1F) && (ch & 0x1F) <= 0x1A) {
            return true;
        }
    }
    if (ch == '_') {
        return true;
    }
    return false;
}

bool is_number(char ch) { return (ch & 0xF0) == 0x30 && 0x0 <= (ch & 0xF) && (ch & 0xF) <= 0x9; }

bool is_hex_letter(char ch) {
    if (1 <= (ch & 0x7) && (ch & 0x7) <= 6) {
        return (ch & 0xF8) == 0x40 || (ch & 0xF8) == 0x60;
    }
    return false;
}

}  // namespace

std::unique_ptr<Token> Lexer::eat_token() {
    if (peekedToken) {
        return std::move(peekedToken);
    }
    while (curI < source.size()) {
        if (source[curI] == '/' && curI + 1 < source.size()) {
            if (source[curI + 1] == '/') {
                curI += 2;
                while (curI < source.size() && source[curI] != '\n') {
                    curI++;
                }
                if (curI >= source.size()) {
                    break;
                }
            } else if (source[curI + 1] == '*') {
                col += 2;
                curI += 2;
                while (curI < source.size()) {
                    if (source[curI] == '*') {
                        if (curI + 1 < source.size() && source[curI + 1] == '/') {
                            col += 2;
                            curI += 2;
                            break;
                        }
                    }
                    col++;
                    if (source[curI] == '\n') {
                        line++;
                        col = 0;
                    }
                    curI++;
                }
                if (curI >= source.size()) {
                    break;
                }
            }
        }
        if (!is_whitespace(source[curI])) {
            break;
        }
        col++;
        if (source[curI] == '\n') {
            line++;
            col = 0;
        }
        curI++;
    }
    if (curI >= source.size()) {
        return make_token(TokenType::END);
    }
    switch (source[curI]) {
        case '{':
            eat_ch();
            return make_token(TokenType::L_CURLY);
        case '}':
            eat_ch();
            return make_token(TokenType::R_CURLY);
        case '(':
            eat_ch();
            return make_token(TokenType::L_PARENS);
        case ')':
            eat_ch();
            return make_token(TokenType::R_PARENS);
        case '=':
            eat_ch();
            if (check_ch('=')) {
                eat_ch();
                return make_token(TokenType::COND_EQUAL);
            } else {
                return make_token(TokenType::OP_ASSIGN);
            }
        case '[':
            eat_ch();
            return make_token(TokenType::L_SUBSCRIPT);
        case ']':
            eat_ch();
            return make_token(TokenType::R_SUBSCRIPT);
        case '*':
            eat_ch();
            return make_token(TokenType::ASTERISK);
        case '&':
            eat_ch();
            return make_token(TokenType::BIT_AND);
        case '+':
            eat_ch();
            if (check_ch('+')) {
                eat_ch();
                return make_token(TokenType::OP_ADD_ADD);
            } else if (check_ch('=')) {
                eat_ch();
                return make_token(TokenType::OP_ADD_EQUALS);
            } else {
                return make_token(TokenType::OP_ADD);
            }
        case '~':
            eat_ch();
            return make_token(TokenType::BIT_NOT);
        case '!':
            eat_ch();
            return make_token(TokenType::COND_NOT);
        case '>':
            eat_ch();
            if (check_ch('=')) {
                eat_ch();
                return make_token(TokenType::COND_GT_EQUAL);
            } else {
                return make_token(TokenType::COND_GT);
            }
        case '<':
            eat_ch();
            if (check_ch('=')) {
                eat_ch();
                return make_token(TokenType::COND_LT_EQUAL);
            } else {
                return make_token(TokenType::COND_LT);
            }
        case ';':
            eat_ch();
            return make_token(TokenType::SEMI_COLON);
        case ',':
            eat_ch();
            return make_token(TokenType::COMMA);
        case '"': {
            eat_ch();
            while (curI + curOff < source.size()) {
                if (source[curI + curOff] == '"') {
                    break;
                }
                eat_ch();
            }
            if (curI + curOff >= source.size()) {
                error("Unterminated string literal");
            }
            std::string_view str = std::string_view(source.c_str() + curI + 1, curOff - 1);
            eat_ch();
            auto tkn = make_token(TokenType::STR_LITERAL);
            tkn->str = std::move(str);
            return tkn;
        } break;
        case 'e':
            curOff++;
            if (check_ident("lse")) {
                return make_token(TokenType::ELSE);
            }
            return eat_identifier();
        case 'i':
            curOff++;
            if (check_ident("nt")) {
                return make_token(TokenType::INT);
            }
            curOff++;
            if (check_ident("f")) {
                return make_token(TokenType::IF);
            }
            return eat_identifier();
        case 'r':
            curOff++;
            if (check_ident("eturn")) {
                return make_token(TokenType::RETURN);
            }
            return eat_identifier();
        case 'v':
            curOff++;
            if (check_ident("oid")) {
                return make_token(TokenType::VOID);
            }
            return eat_identifier();
        case 'w':
            curOff++;
            if (check_ident("hile")) {
                return make_token(TokenType::WHILE);
            }
            return eat_identifier();
        case '-':
            eat_ch();
            if (check_ch('-')) {
                eat_ch();
                return make_token(TokenType::OP_SUB_SUB);
            } else if (check_ch('=')) {
                eat_ch();
                return make_token(TokenType::OP_SUB_EQUALS);
            } else if (is_number(source[curI + curOff])) {
                uint16_t val = -eat_num_lit();
                auto tkn = make_token(TokenType::INT_LITERAL);
                tkn->num = val;
                return tkn;
            } else {
                return make_token(TokenType::OP_SUB_NEGATE);
            }
        default:
            if (is_letter(source[curI])) {
                return eat_identifier();
            } else if (is_number(source[curI])) {
                int val = eat_num_lit();
                if (val >= 0x8000) {
                    literal_overflow_error(val);
                }
                auto tkn = make_token(TokenType::INT_LITERAL);
                tkn->num = val;
                return tkn;
            }
            break;
    }
    eat_ch();
    return make_token(TokenType::ERROR);
}

std::unique_ptr<Token> Lexer::eat_identifier() {
    eat_ch();  // Assume that first character is already part of the identifier
    while (curI + curOff < source.size()) {
        if (!is_letter(source[curI + curOff]) && !is_number(source[curI + curOff])) {
            break;
        }
        eat_ch();
    }
    std::string_view str = std::string_view(source.c_str() + curI, curOff);
    auto tkn = make_token(TokenType::IDENTIFIER);
    tkn->str = std::move(str);
    return tkn;
}

int Lexer::eat_num_lit() {
    int base = 10;
    if (source[curI + curOff] == '0' && curI + curOff + 1 < source.size() &&
        source[curI + curOff + 1] == 'x') {
        base = 0x10;
        eat_ch();
        eat_ch();
    }
    int val = 0;
    int prev;
    bool overflow = false;
    while (curI + curOff < source.size()) {
        int ch = source[curI + curOff];
        int digit = 0;
        if (base == 10) {
            if (!is_number(ch)) {
                break;
            }
            digit = (source[curI + curOff] & 0xF);
        } else if (base == 0x10) {
            if (is_number(ch)) {
                digit = (source[curI + curOff] & 0xF);
            } else if (is_hex_letter(ch)) {
                digit = (source[curI + curOff] & 0x7) + 9;
            } else {
                break;
            }
        }
        prev = val;
        val = (val * base) + digit;
        if (prev > val) {
            overflow = true;
        }
        eat_ch();
    }
    if (val > 0x8000 || overflow) {
        literal_overflow_error(val);
    }
    return val;
}

Token* Lexer::peek_token() {
    if (!peekedToken) {
        peekedToken = eat_token();
    }
    return peekedToken.get();
}
