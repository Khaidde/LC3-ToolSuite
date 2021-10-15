#include "parser.hpp"

void Parser::load_source(std::string&& src) {
    operandCnt = 0;
    isOrigBlock = false;
    curAddr = 0x0000;
    endAddr = 0x0000;

    lex.load_source(std::move(src));
}

void Parser::insert_word(Word&& word) { memory[curAddr++] = word; }

void Parser::assert_eat_comma() {
    if (lex.eat_token().kind != TokenKind::COMMA) {
        lex.error("Expected a comma to separate operands");
    }
}

char Parser::try_parse_register() {
    operandCnt++;
    Token tkn = lex.eat_token();
    if (tkn.kind != TokenKind::REG) {
        lex.error("Expected a register for operand " + std::to_string(operandCnt));
    }
    return tkn.num;
}

void Parser::try_parse_pcoffset(Word&& word) {
    Token tkn = lex.eat_token();
    if (tkn.kind == TokenKind::LABEL) {
        word.offLabel = tkn.str;
        insert_word(std::move(word));
    } else if (tkn.kind == TokenKind::LITERAL) {
        word.offVal = (int16_t)tkn.num;
        insert_word(build_instr(word));
    } else {
        lex.error("Expected either a label or literal as the last operand instruction");
    }
}

uint16_t Parser::build_instr(Word& word) {
    uint16_t data = word.kind == TokenKind::LD    ? 0x2
                    : word.kind == TokenKind::LDI ? 0xA
                    : word.kind == TokenKind::LDR ? 0x6
                    : word.kind == TokenKind::LEA ? 0xE
                    : word.kind == TokenKind::ST  ? 0x3
                    : word.kind == TokenKind::STI ? 0xB
                    : word.kind == TokenKind::STR ? 0x7
                    : word.kind == TokenKind::BR  ? 0x0
                                                  : 0x4;
    data <<= 12;

    if (word.kind == TokenKind::BR) {
        data |= word.branchCC << 9;
    } else if (word.kind != TokenKind::JSR) {
        data |= word.memInstr.reg << 9;
    }

    if (word.kind == TokenKind::LDR || word.kind == TokenKind::STR) {
        if (check_overflow((int16_t)word.offVal, 6)) {
            lex.error("Immediate value for LDR and STR must be a 6-bit 2's complement number");
        }
        data |= (word.memInstr.baseR << 6) | (word.offVal & 0x3F);
    } else if (word.kind == TokenKind::JSR) {
        if (check_overflow((int16_t)word.offVal, 11)) {
            lex.error(
                "Immediate value for JSR must be an 11-bit 2's complement "
                "number");
        }
        data |= 0x0800 | (word.offVal & 0x7FF);
    } else {
        if (check_overflow((int16_t)word.offVal, 9)) {
            lex.error(
                "Immediate value for PCOffset9 must be a 9-bit 2's complement "
                "number");
        }
        data |= (word.offVal & 0x1FF);
    }

    return data;
}

void Parser::first_pass_parse() {
    TokenKind instrKind;
    uint16_t wordData;
    Token tkn = lex.eat_token();
    while (tkn.kind != TokenKind::E_O_F) {
        operandCnt = 0;
        if (!isOrigBlock && tkn.kind != TokenKind::ORIG) {
            lex.error("Must define .ORIG before block of instructions");
        }
        if (tkn.kind == TokenKind::LABEL) {
            SymEntry<uint16_t>* entry = symTable.try_insert(tkn.str, std::move(curAddr));
            if (entry) {
                fatal("Error: duplicate label '%s' at address x%04x and x%04x\n",
                      std::string(entry->symbol).c_str(), curAddr, entry->data);
            }
            tkn = lex.eat_token();
            if (tkn.kind == TokenKind::E_O_F) {
                break;
            }
        }
        instrKind = tkn.kind;
        switch (instrKind) {
            case TokenKind::ORIG:
                if (isOrigBlock) {
                    lex.error("Must define .END before starting another .ORIG block");
                }
                isOrigBlock = true;
                tkn = lex.eat_token();
                if (tkn.kind != TokenKind::LITERAL) {
                    lex.error(".ORIG must be followed by a literal");
                }
                for (size_t i = curAddr; i < tkn.num; i++) {
                    insert_word(0);
                }
                curAddr = tkn.num;
                break;
            case TokenKind::FILL:
                tkn = lex.eat_token();
                if (tkn.kind != TokenKind::LITERAL) {
                    lex.error(".FILL must be followed by a literal");
                }
                insert_word(tkn.num);
                break;
            case TokenKind::BLKW:
                tkn = lex.eat_token();
                if (tkn.kind != TokenKind::LITERAL) {
                    lex.error(".BLKW must be followed by a literal");
                }
                for (int i = 0; i < tkn.num; i++) {
                    insert_word(0);
                }
                break;
            case TokenKind::STRINGZ:
                tkn = lex.eat_token();
                if (tkn.kind != TokenKind::STR_LITERAL) {
                    lex.error("Expected a string literal after .STRINGZ");
                }
                for (const char& ch : tkn.str) {
                    insert_word((uint16_t)ch);
                }
                insert_word('\0');
                break;
            case TokenKind::END:
                if (!isOrigBlock) {
                    lex.error("Must define .ORIG before declaring .END");
                }
                isOrigBlock = false;
                if (endAddr < curAddr) {
                    endAddr = curAddr;
                }
                break;
            case TokenKind::HALT:
                insert_word(0xF025);
                break;
            case TokenKind::ADD:
            case TokenKind::AND:
            case TokenKind::NOT:
                wordData = instrKind == TokenKind::ADD   ? 0x1000
                           : instrKind == TokenKind::NOT ? 0x903F
                                                         : 0x5000;

                wordData |= try_parse_register() << 9;
                assert_eat_comma();

                wordData |= try_parse_register() << 6;
                if (instrKind != TokenKind::NOT) {
                    assert_eat_comma();

                    tkn = lex.eat_token();  // Grab register or imm5
                    if (tkn.kind == TokenKind::REG) {
                        wordData |= tkn.num;
                    } else if (tkn.kind == TokenKind::LITERAL) {
                        if (check_overflow((int16_t)tkn.num, 5)) {
                            lex.error("Immediate value must be a 5-bit 2's complement number");
                        }
                        wordData |= 0x20 | (tkn.num & 0x1F);
                    } else {
                        lex.error("Expected either a register or literal as the third operand");
                    }
                }
                insert_word(wordData);
                break;
            case TokenKind::JMP:
            case TokenKind::JSRR:
                if (instrKind == TokenKind::JMP) {
                    wordData = 0xC000;
                } else {
                    wordData = 0x4000;
                }

                wordData |= try_parse_register() << 6;
                assert_eat_comma();

                insert_word(wordData);
                break;
            case TokenKind::BR: {
                Word word(tkn.kind);

                if (tkn.num == 0) {
                    tkn.num = 0x7;
                }
                word.branchCC = tkn.num;

                try_parse_pcoffset(std::move(word));
            } break;
            case TokenKind::JSR: {
                try_parse_pcoffset(tkn.kind);
            } break;
            case TokenKind::LDR:
            case TokenKind::STR: {
                Word word(tkn.kind);

                word.memInstr.reg = try_parse_register();
                assert_eat_comma();

                word.memInstr.baseR = try_parse_register();
                assert_eat_comma();

                try_parse_pcoffset(std::move(word));
            } break;
            case TokenKind::LD:
            case TokenKind::LDI:
            case TokenKind::LEA:
            case TokenKind::ST:
            case TokenKind::STI: {
                Word word(tkn.kind);

                word.memInstr.reg = try_parse_register();
                assert_eat_comma();

                try_parse_pcoffset(std::move(word));
            } break;
            default:
                lex.error("Expected an instruction or pseudo-op");
        }
        tkn = lex.eat_token();
    }
}

void Parser::second_pass_parse(const char* destPath) {
    if (isOrigBlock) {
        lex.error("Must declare end of file with .END");
    }

    std::ofstream file;
    if (destPath) {
        file = std::ofstream(destPath, std::ios::binary);
        if (!file.is_open()) {
            fatal("Could not open ouput file: %s", destPath);
        }
    }

    bool zeroStr = false;
    uint16_t zeroStrStart = 0x0000;
    for (uint16_t i = 0; i < endAddr; i++) {
        Word& word = memory[i];
        if (!word.isMachineCode) {
            uint16_t* entry = symTable.get(word.offLabel);
            if (entry == nullptr) {
                fatal("Could not find referenced label: '%s'\n",
                      std::string(word.offLabel).c_str());
            }

            word.offVal = *entry - (int16_t)(i + 1);
            word.data = build_instr(word);
        }

        if (destPath) {
            uint8_t hi = word.data >> 8;
            uint8_t lo = word.data & 0xFF;
            file.write((const char*)&hi, sizeof(uint8_t));
            file.write((const char*)&lo, sizeof(uint8_t));
        } else {
            if (zeroStr) {
                if (word.data != 0) {
                    zeroStr = false;
                    printf("[%04x:%04x)\n", zeroStrStart, i);
                }
            }
            if (!zeroStr) {
                if (word.data == 0) {
                    zeroStr = true;
                    zeroStrStart = i;
                } else {
                    printf("%04x\n", word.data);
                }
            }
        }
    }
    if (zeroStr) {
        printf("[%04x:%04x)\n", zeroStrStart, endAddr);
    }
    if (destPath) {
        file.close();
    }
}
