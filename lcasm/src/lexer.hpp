#pragma once

#include "general.hpp"

enum class TokenKind {
    ADD,
    AND,
    NOT,
    LD,
    LDR,
    LDI,
    LEA,
    ST,
    STR,
    STI,
    BR,
    JMP,
    JSR,
    JSRR,
    RET,

    NOP,
    TRAP,
    HALT,

    ORIG,
    FILL,
    BLKW,
    STRINGZ,
    END,

    REG,
    LITERAL,
    STR_LITERAL,
    LABEL,

    COMMA,
    E_O_F,
};

struct Token {
    Token(TokenKind kind) : kind(kind) {}
    Token(TokenKind kind, std::string_view&& str) : kind(kind), str(str) {}
    Token(TokenKind kind, uint16_t num) : kind(kind), num(num) {}
    TokenKind kind;
    union {
        std::string_view str;
        uint16_t num;
    };
};

struct Lexer {
    void load_source(std::string&& src);

    void error(const char* msg);
    void error(std::string&& msg);

    bool is_break(char ch);
    bool is_dec(char ch);
    bool check_ch(char ch);
    void shift_till_token();
    Token eat_label_or_keyword();
    Token eat_dec_literal();
    Token eat_token();

    std::string src;
    size_t curI;

    size_t line;
};
