#include "general.hpp"

struct LC3 {
    uint8_t reg1();
    uint8_t reg2();
    int8_t imm5();
    int16_t pcoffset9();
    int8_t offset6();

    void execute();
    void emulate(const char* binPath);

    void cache_status();
    void print_status();
    void print_stack();

    bool halted = false;
    uint16_t PC = 0x3000;  // LC3 programs start at 0x3000
    uint16_t IR;
    uint16_t R[8];
    uint16_t cc;  // condition codes - nzp

    uint16_t prevPC;
    uint16_t prevR[8];

    uint16_t memory[1 << 16];
};

uint8_t LC3::reg1() { return (IR >> 9) & 0x7; }

uint8_t LC3::reg2() { return (IR >> 6) & 0x7; }

int8_t LC3::imm5() { return (IR & 0x1F) | ((IR & 0x10) ? (int8_t)0xE0 : 0); }

int16_t LC3::pcoffset9() { return (IR & 0x1FF) | ((IR & 0x100) ? (int16_t)0xFE00 : 0); }

int8_t LC3::offset6() { return (IR & 0x3F) | ((IR & 0x20) ? (int8_t)0xC0 : 0); }

void LC3::execute() {
    char opcode = IR >> 12;
    switch (opcode) {
        case 0x0: {  // BR
            bool n = IR & 0x800;
            bool z = IR & 0x400;
            bool p = IR & 0x200;
            int16_t cond = (int16_t)cc;
            if ((n && cond < 0) || (z && (cond == 0)) || (p && cond > 0)) {
                PC = PC + pcoffset9();
            }
        } break;
        case 0x1:  // ADD
            if (IR & 0x20) {
                cc = R[reg1()] = R[reg2()] + imm5();
            } else {
                cc = R[reg1()] = R[reg2()] + (int16_t)R[IR & 0x7];
            }
            break;
        case 0x2:  // LD
            cc = R[reg1()] = memory[PC + pcoffset9()];
            break;
        case 0x3:  // ST
            cc = memory[PC + pcoffset9()] = R[reg1()];
            break;
        case 0x4:  // JSR/JSRR
            R[7] = PC;
            if (IR & 0x800) {
                PC += (IR & 0x7FF) | ((IR & 0x400) ? (int16_t)0xF800 : 0);
            } else {
                PC = R[reg2()];
            }
            break;
        case 0x5:  // AND
            if (IR & 0x20) {
                cc = R[reg1()] = R[reg2()] & (IR & 0x1F);
            } else {
                cc = R[reg1()] = R[reg2()] & R[IR & 0x7];
            }
            break;
        case 0x6:  // LDR
            cc = R[reg1()] = memory[R[reg2()] + offset6()];
            break;
        case 0x7:  // STR
            cc = memory[R[reg2()] + offset6()] = R[reg1()];
            break;
        case 0x8: {
            fatal("Opcode 0x8 is undefined");
        } break;
        case 0x9:  // NOT
            cc = R[reg1()] = ~R[reg2()];
            break;
        case 0xA:  // LDI
            cc = R[reg1()] = memory[memory[PC + pcoffset9()]];
            break;
        case 0xB:  // STI
            cc = memory[memory[PC + pcoffset9()]] = R[reg1()];
            break;
        case 0xC:  // JMP
            PC = R[reg2()];
            break;
        case 0xD: {
            fatal("Opcode 0xD is undefined");
        } break;
        case 0xE:  // LEA
            R[reg1()] = PC + pcoffset9();
            break;
        case 0xF:  // TRAP
            R[7] = PC;
            switch (IR & 0xFF) {
                case 0x21:
                    printf("lcemu@0x%04x: 0x%04x(%d)\n", PC, R[0], R[0]);
                    break;
                case 0x25:
                    halted = true;
                    printf("Halting at %04x...\n", PC);
                    break;
                default:
                    fatal("Unimplemented trap address at %04x: addr=%02x\n", PC, IR & 0xFF);
            }
            break;
        default:
            fatal("Unimplemented opcode\n");
    }
}

void LC3::cache_status() {
    prevPC = PC;
    for (size_t i = 0; i < 8; i++) {
        prevR[i] = R[i];
    }
}

namespace {
// clang-format off
const char NUM_OPCODES = 0x10;
const char* opcodeToStr[NUM_OPCODES] = {
    "BR",       "ADD",  "LD",  "ST",
    "JSR/JSRR", "AND",  "LDR", "STR",
    "UNK_8",    "NOT",  "LDI", "STI",
    "JMP",      "UNK_D","LEA", "TRAP",
};
// clang-format on
}  // namespace

void LC3::print_status() {
    printf("--------------\n");
    printf("PC=%04x -> %04x [%s:%04x]\n", prevPC, PC, opcodeToStr[memory[prevPC] >> 12],
           memory[prevPC]);
    for (size_t i = 0; i < 8; i++) {
        printf("R%d= %4.02x(%6.hi) -> %4.02x(%6.hi)", i, prevR[i], prevR[i], R[i], R[i]);
        if (prevR[i] != R[i]) {
            printf(" <-");
        }
        printf("\n");
    }
    for (int i = 0x6000; memory[i]; i++) {
        printf("%c", (char)memory[i]);
    }
    printf("\n");
    for (int i = 0x4000; i < 0x4007; i++) {
        printf("%hi::", memory[i]);
    }
    printf("||\n");
    print_stack();
}

void LC3::print_stack() {
    printf("[0xff00]:");
    for (size_t i = 0xf000 - 1; i >= R[6]; i--) {
        printf("%04x:", memory[i]);
    }
    printf("|\n");
}

void LC3::emulate(const char* binPath) {
    std::string file = read_file(binPath);
    for (size_t i = 0; i < file.length(); i += 2) {
        memory[i >> 1] = (file[i] << 8) | (file[i + 1] & 0xFF);
    }
    while (!halted) {
        cache_status();
        IR = memory[PC++];
        execute();
    }
}

int main(int argc, char** argv) {
    printf("lcemu version 1.1\n");

    if (argc != 2) {
        printf("Usage: lcemu [source_file].bin\n");
        return -1;
    }
    try {
        LC3().emulate(argv[1]);
    } catch (std::exception& e) {
        printf("%s\n", e.what());
    }
    return 0;
}
