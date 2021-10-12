#pragma once

#include "lexer.hpp"
#include "symtable.hpp"

struct MemInstr {
    char reg;
    char baseR;
};

struct Word {
    Word() {}
    Word(TokenKind kind) : kind(kind), isMachineCode(false) {}
    Word(uint16_t data) : data(data), isMachineCode(true) {}

    union {
        uint16_t data;  // Data to be directly inserted in memory
        struct {
            TokenKind kind;
            union {
                std::string_view offLabel;
                int16_t offVal;
            };
            union {
                char branchCC;
                MemInstr memInstr;
            };
        };
    };
    bool isMachineCode;
};

struct Parser {
    void load_source(std::string&& src);

    void insert_word(Word&& word);

    void assert_eat_comma();

    char try_parse_register();
    void try_parse_pcoffset(Word&& word);

    uint16_t build_instr(Word& word);

    void first_pass_parse();
    void second_pass_parse(const char* destPath);

    char operandCnt;

    Lexer lex;
    SymTable<uint16_t> symTable;

    bool hasDefinedOrig;
    bool hasDefinedEnd;
    uint16_t baseAddr;  // Start address of code
    uint16_t curAddr;   // Current address
    uint16_t endAddr;
    Word memory[1 << 16];
};
