// LoongArch 指令译码器（22条指令）—— 位域+联合体零开销译码

#ifndef DECODER_H
#define DECODER_H

#include "global.h"

// Reg3 格式（三寄存器，17-bit opcode）：用于 ADD.W, SUB.W, SLT, SLTU, NOR, AND, OR, XOR
//  31-15 opcode | 14-10 rk | 9-5 rj | 4-0 rd
struct Reg3Format {
    uint32_t rd      : 5;
    uint32_t rj      : 5;
    uint32_t rk      : 5;
    uint32_t opcode  : 17;
};

// Reg2I5 格式（17-bit opcode）：用于 SLLI.W, SRLI.W
//  31-15 opcode | 14-10 imm5 | 9-5 rj | 4-0 rd
struct Reg2I5Format {
    uint32_t rd      : 5;
    uint32_t rj      : 5;
    uint32_t imm     : 5;
    uint32_t opcode  : 17;
};

// Reg2I12 格式（10-bit opcode）：用于 ADDI.W, ANDI, ORI, XORI, SLTI, SLTIU
//  31-22 opcode | 21-10 imm12 | 9-5 rj | 4-0 rd
struct Reg2I12Format {
    uint32_t rd      : 5;
    uint32_t rj      : 5;
    uint32_t imm     : 12;
    uint32_t opcode  : 10;
};

// Reg2I16 格式（6-bit opcode）：用于 BEQ, BNE, BLT, BGE, BLTU, BGEU
//  31-26 opcode | 25-10 offset16 | 9-5 rj | 4-0 rd
struct Reg2I16Format {
    uint32_t rd      : 5;
    uint32_t rj      : 5;
    int32_t  offset  : 16;
    uint32_t opcode  : 6;
};

// I26 格式（6-bit opcode）：用于 B
//  31-26 opcode | 25-0 offset26
struct I26Format {
    int32_t  offset  : 26;
    uint32_t opcode  : 6;
};

// 指令联合体 —— 一种机器码，五种视图
typedef union {
    uint32_t             raw;
    struct Reg3Format    reg3;
    struct Reg2I5Format  reg2i5;
    struct Reg2I12Format reg2i12;
    struct Reg2I16Format reg2i16;
    struct I26Format     i26;
} Instruction;

// 译码结果
typedef struct {
    InstrFormat format;
    uint32_t    opcode;
    uint32_t    rd, rj, rk;
    int32_t     immediate;
} DecodedInstr;

void        decode(uint32_t machine_code, DecodedInstr* decoded);
uint32_t    get_opcode(uint32_t machine_code);
const char* opcode_mnemonic(uint32_t op);
void        decoded_to_string(const DecodedInstr* d, char* buf, int buf_size);

#endif /* DECODER_H */
