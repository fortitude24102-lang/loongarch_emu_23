// CPU 单周期执行状态机实现（22 条指令）
// 数据通路：IRAM[PC/4] → decode() → ALU/Branch → RF[rd] / PC

#include "cpu_core.h"

CPU_State cpu;

void cpu_init(void)
{
    cpu.PC = 0;
    cpu.cycle_count = 0;
    cpu.instr_count = 0;
    cpu.running = 1;
    cpu.cur_instr = 0;
    memset(cpu.cur_asm, 0, sizeof(cpu.cur_asm));
    memset(cpu.RF, 0, sizeof(cpu.RF));
    memset(cpu.IRAM, 0, sizeof(cpu.IRAM));
    memset(cpu.DRAM, 0, sizeof(cpu.DRAM));
}

void cpu_load_instr(uint32_t address, uint32_t machine_code)
{
    uint32_t index = address / 4;
    if (index < IRAM_SIZE) {
        cpu.IRAM[index] = machine_code;
    } else {
        fprintf(stderr, "[CPU] 错误：IRAM 地址越界 (addr=0x%08X)\n", address);
    }
}

int cpu_step(void)
{
    if (!cpu.running) return 0;

    uint32_t pc_index = cpu.PC / 4;
    if (pc_index >= IRAM_SIZE) {
        printf("\n[CPU] 停止：PC=0x%08X 超出 IRAM 范围\n", cpu.PC);
        cpu.running = 0;
        return 0;
    }

    // 阶段1：取指（IF）
    uint32_t machine_code = cpu.IRAM[pc_index];
    cpu.cur_instr = machine_code;

    // 阶段2：译码（ID）
    DecodedInstr decoded;
    decode(machine_code, &decoded);
    decoded_to_string(&decoded, cpu.cur_asm, sizeof(cpu.cur_asm));

    // 阶段3：执行（EX）
    uint32_t alu_result = 0;
    uint32_t next_pc    = cpu.PC + 4;
    int writeback_enable = 0;

    switch (decoded.opcode) {

    case OP_ADD_W:
        alu_result = cpu.RF[decoded.rj] + cpu.RF[decoded.rk];
        writeback_enable = 1;
        break;

    case OP_SUB_W:
        alu_result = cpu.RF[decoded.rj] - cpu.RF[decoded.rk];
        writeback_enable = 1;
        break;

    case OP_ADDI_W:
        alu_result = cpu.RF[decoded.rj] + (uint32_t)decoded.immediate;
        writeback_enable = 1;
        break;

    case OP_AND:
        alu_result = cpu.RF[decoded.rj] & cpu.RF[decoded.rk];
        writeback_enable = 1;
        break;

    case OP_OR:
        alu_result = cpu.RF[decoded.rj] | cpu.RF[decoded.rk];
        writeback_enable = 1;
        break;

    case OP_XOR:
        alu_result = cpu.RF[decoded.rj] ^ cpu.RF[decoded.rk];
        writeback_enable = 1;
        break;

    case OP_NOR:
        alu_result = ~(cpu.RF[decoded.rj] | cpu.RF[decoded.rk]);
        writeback_enable = 1;
        break;

    case OP_ANDI:
        alu_result = cpu.RF[decoded.rj] & (uint32_t)decoded.immediate;
        writeback_enable = 1;
        break;

    case OP_ORI:
        alu_result = cpu.RF[decoded.rj] | (uint32_t)decoded.immediate;
        writeback_enable = 1;
        break;

    case OP_XORI:
        alu_result = cpu.RF[decoded.rj] ^ (uint32_t)decoded.immediate;
        writeback_enable = 1;
        break;

    case OP_SLLI_W:
        alu_result = cpu.RF[decoded.rj] << (decoded.immediate & 0x1F);
        writeback_enable = 1;
        break;

    case OP_SRLI_W:
        alu_result = cpu.RF[decoded.rj] >> (decoded.immediate & 0x1F);
        writeback_enable = 1;
        break;

    case OP_SLT:
        alu_result = ((int32_t)cpu.RF[decoded.rj] < (int32_t)cpu.RF[decoded.rk]) ? 1 : 0;
        writeback_enable = 1;
        break;

    case OP_SLTU:
        alu_result = (cpu.RF[decoded.rj] < cpu.RF[decoded.rk]) ? 1 : 0;
        writeback_enable = 1;
        break;

    case OP_SLTI:
        alu_result = ((int32_t)cpu.RF[decoded.rj] < decoded.immediate) ? 1 : 0;
        writeback_enable = 1;
        break;

    case OP_SLTIU:
        alu_result = (cpu.RF[decoded.rj] < (uint32_t)decoded.immediate) ? 1 : 0;
        writeback_enable = 1;
        break;

    case OP_B:
        next_pc = cpu.PC + (uint32_t)((int32_t)decoded.immediate * 4);
        writeback_enable = 0;
        break;

    case OP_BEQ:
        if (cpu.RF[decoded.rj] == cpu.RF[decoded.rd])
            next_pc = cpu.PC + (uint32_t)((int32_t)decoded.immediate * 4);
        writeback_enable = 0;
        break;

    case OP_BNE:
        if (cpu.RF[decoded.rj] != cpu.RF[decoded.rd])
            next_pc = cpu.PC + (uint32_t)((int32_t)decoded.immediate * 4);
        writeback_enable = 0;
        break;

    case OP_BLT:
        if ((int32_t)cpu.RF[decoded.rj] < (int32_t)cpu.RF[decoded.rd])
            next_pc = cpu.PC + (uint32_t)((int32_t)decoded.immediate * 4);
        writeback_enable = 0;
        break;

    case OP_BGE:
        if ((int32_t)cpu.RF[decoded.rj] >= (int32_t)cpu.RF[decoded.rd])
            next_pc = cpu.PC + (uint32_t)((int32_t)decoded.immediate * 4);
        writeback_enable = 0;
        break;

    case OP_BLTU:
        if (cpu.RF[decoded.rj] < cpu.RF[decoded.rd])
            next_pc = cpu.PC + (uint32_t)((int32_t)decoded.immediate * 4);
        writeback_enable = 0;
        break;

    case OP_BGEU:
        if (cpu.RF[decoded.rj] >= cpu.RF[decoded.rd])
            next_pc = cpu.PC + (uint32_t)((int32_t)decoded.immediate * 4);
        writeback_enable = 0;
        break;

    default:
        writeback_enable = 0;
        break;
    }

    // 阶段4：写回（WB）
    if (writeback_enable && decoded.rd != 0) {
        cpu.RF[decoded.rd] = alu_result;
    }
    cpu.RF[0] = 0;  // $r0 硬件恒为 0

    // 阶段5：PC 更新
    cpu.PC = next_pc;
    cpu.cycle_count++;
    cpu.instr_count++;

    return 1;
}

uint32_t cpu_get_pc(void)    { return cpu.PC; }
uint32_t cpu_get_cycle(void) { return cpu.cycle_count; }
