// LoongArch 指令译码器（22 条指令）—— 位域+联合体零开销译码

#include "decoder.h"

uint32_t get_opcode(uint32_t machine_code)
{
    return (machine_code >> 26) & 0x3F;
}

static int32_t sign_ext12(uint32_t v)
{
    if (v & 0x800)
        return (int32_t)(v | 0xFFFFF000);
    return (int32_t)v;
}

void decode(uint32_t machine_code, DecodedInstr* decoded)
{
    Instruction ins;
    ins.raw = machine_code;

    decoded->rd = decoded->rj = decoded->rk = 0;
    decoded->immediate = 0;

    // 第1步：6-bit opcode → 分支/跳转指令
    uint32_t op6 = ins.reg2i16.opcode;

    if (op6 >= 0x14 && op6 <= 0x1B) {
        decoded->opcode = op6;

        if (op6 == OP_B) {
            decoded->format = FMT_I26;
            decoded->immediate = ins.i26.offset;
        } else {
            decoded->format = FMT_REG2I16;
            decoded->rj = ins.reg2i16.rj;
            decoded->rd = ins.reg2i16.rd;
            decoded->immediate = ins.reg2i16.offset;
        }
        return;
    }

    // 第2步：10-bit opcode → Reg2I12 格式
    uint32_t op10 = ins.reg2i12.opcode;

    if (op10 >= 0x08 && op10 <= 0x0F) {
        decoded->opcode = op10;
        decoded->format = FMT_REG2I12;
        decoded->rd = ins.reg2i12.rd;
        decoded->rj = ins.reg2i12.rj;

        if (op10 == OP_ADDI_W || op10 == OP_SLTI || op10 == OP_SLTIU)
            decoded->immediate = sign_ext12(ins.reg2i12.imm);
        else
            decoded->immediate = (int32_t)ins.reg2i12.imm;
        return;
    }

    // 第3步：17-bit opcode → Reg3 / Reg2I5 格式
    uint32_t op17 = ins.reg3.opcode;
    decoded->opcode = op17;

    if (op17 == OP_SLLI_W || op17 == OP_SRLI_W) {
        decoded->format = FMT_REG2I5;
        decoded->rd = ins.reg2i5.rd;
        decoded->rj = ins.reg2i5.rj;
        decoded->immediate = (int32_t)ins.reg2i5.imm;
    } else {
        decoded->format = FMT_REG3;
        decoded->rd = ins.reg3.rd;
        decoded->rj = ins.reg3.rj;
        decoded->rk = ins.reg3.rk;
    }
}

const char* opcode_mnemonic(uint32_t op)
{
    switch (op) {
    case OP_ADD_W:  return "ADD.W";
    case OP_SUB_W:  return "SUB.W";
    case OP_ADDI_W: return "ADDI.W";
    case OP_AND:    return "AND";
    case OP_OR:     return "OR";
    case OP_XOR:    return "XOR";
    case OP_NOR:    return "NOR";
    case OP_ANDI:   return "ANDI";
    case OP_ORI:    return "ORI";
    case OP_XORI:   return "XORI";
    case OP_SLLI_W: return "SLLI.W";
    case OP_SRLI_W: return "SRLI.W";
    case OP_SLT:    return "SLT";
    case OP_SLTU:   return "SLTU";
    case OP_SLTI:   return "SLTI";
    case OP_SLTIU:  return "SLTIU";
    case OP_B:      return "B";
    case OP_BEQ:    return "BEQ";
    case OP_BNE:    return "BNE";
    case OP_BLT:    return "BLT";
    case OP_BGE:    return "BGE";
    case OP_BLTU:   return "BLTU";
    case OP_BGEU:   return "BGEU";
    default:        return "???";
    }
}

void decoded_to_string(const DecodedInstr* d, char* buf, int buf_size)
{
    const char* m = opcode_mnemonic(d->opcode);

    switch (d->format) {
    case FMT_REG3:
        snprintf(buf, buf_size, "%-7s $r%u, $r%u, $r%u",
                 m, d->rd, d->rj, d->rk);
        break;
    case FMT_REG2I5:
    case FMT_REG2I12:
        snprintf(buf, buf_size, "%-7s $r%u, $r%u, %d",
                 m, d->rd, d->rj, d->immediate);
        break;
    case FMT_REG2I16:
        snprintf(buf, buf_size, "%-7s $r%u, $r%u, %+d",
                 m, d->rj, d->rd, d->immediate);
        break;
    case FMT_I26:
        snprintf(buf, buf_size, "%-7s %+d", m, d->immediate);
        break;
    default:
        snprintf(buf, buf_size, "???");
        break;
    }
}
