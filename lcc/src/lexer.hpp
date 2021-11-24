#pragma once

#include <memory>

#include "general.hpp"

enum class TokenType : uint8_t {

    // Blocks
    L_CURLY,
    R_CURLY,
    L_PARENS,
    R_PARENS,

    IF,
    ELSE,
    WHILE,

    INT,  // note: int is 16-bits
    VOID,

    IDENTIFIER,
    INT_LITERAL,
    STR_LITERAL,

    ASTERISK,     // multiplication or ptr or deref
    OP_ASSIGN,    // =
    L_SUBSCRIPT,  // [
    R_SUBSCRIPT,  // ]

    // Bitwise Operators
    BIT_AND,
    BIT_NOT,

    // Operators
    OP_ADD,
    OP_ADD_ADD,
    OP_ADD_EQUALS,
    OP_SUB_NEGATE,
    OP_SUB_SUB,
    OP_SUB_EQUALS,

    // Conditional Operators
    COND_NOT,
    COND_EQUAL,
    COND_GT,
    COND_LT,
    COND_GT_EQUAL,
    COND_LT_EQUAL,

    RETURN,  // int main() { return 0; }

    SEMI_COLON,
    COMMA,
    ERROR,
    END,
};

struct Token {
    Token(TokenType type, int line, int col) : type(type), line(line), col(col) {}

    TokenType type;
    int line;
    int col;
    union {
        std::string_view str;
        uint16_t num;
    };
};

const char* token_type_to_str(TokenType type);

struct Lexer {
    void load_source(std::string&& source);
    [[noreturn]] void error(std::string&& msg);
    void literal_overflow_error(int val);

    void eat_ch();
    bool check_ch(char ch);
    bool check_ident(const char* ident);

    std::unique_ptr<Token> make_token(TokenType type);
    std::unique_ptr<Token> eat_token();
    std::unique_ptr<Token> eat_identifier();
    int eat_num_lit();
    Token* peek_token();

    std::unique_ptr<Token> peekedToken;

    std::string source;

    int line;
    int col;

    size_t curI;
    size_t curOff;
};
