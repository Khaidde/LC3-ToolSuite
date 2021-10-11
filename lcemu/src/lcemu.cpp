#include "general.hpp"

struct LC3 {
    uint8_t reg1();
    uint8_t reg2();
    int16_t pcoffset9();
    int8_t imm5();

    void execute();
    void emulate(const char* binPath);

    bool halted = false;
    uint16_t PC = 0x3000;  // LC3 programs start at 0x3000
    uint16_t IR;
    uint16_t R[8];
    uint16_t cc;  // condition codes - nzp

    uint16_t memory[1 << 16];
};

uint8_t LC3::reg1() { return (IR >> 9) & 0x7; }

uint8_t LC3::reg2() { return (IR >> 6) & 0x7; }

int16_t LC3::pcoffset9() {
    int16_t pcoff = (IR & 0x1FF);
    return (pcoff & 0x1FF) | ((pcoff & 0x100) ? (int16_t)0xFE00 : 0);
}

int8_t LC3::imm5() {
    int8_t imm5 = (IR & 0x1F);
    return (imm5 & 0x1F) | ((imm5 & 0x10) ? (int8_t)0xE0 : 0);
}

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
                PC = PC + (int16_t)(IR & 0x7FF);
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
            cc = R[reg1()] = memory[R[reg2()] + (int8_t)(IR & 0x3F)];
            break;
        case 0x7:  // STR
            cc = memory[R[reg2()] + (int8_t)(IR & 0x3F)] = R[reg1()];
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
                case 0x25:
                    halted = true;
                    printf("Halting at %04x...\n", PC);
                    break;
                default:
                    fatal("Unimplemented trap address\n");
            }
            break;
        default:
            fatal("Unimplemented opcode\n");
    }
}

void LC3::emulate(const char* binPath) {
    std::string file = read_file(binPath);
    size_t baseAddr = (size_t)((file[0] << 8) | (file[1] & 0xFF));
    for (size_t i = 2; i < file.length(); i += 2) {
        memory[baseAddr + (i >> 1) - 1] = (file[i] << 8) | (file[i + 1] & 0xFF);
    }
    while (!halted) {
        IR = memory[PC++];
        execute();
    }
    printf("%04x\n", memory[0x3009]);
}

int main(int argc, char** argv) {
    printf("lcemu version 1.0\n");

    if (argc != 2 && argc != 3) {
        printf("Usage: lcemu [source_file].bin\n");
        return -1;
    }
    LC3().emulate(argv[1]);
    return 0;
}
