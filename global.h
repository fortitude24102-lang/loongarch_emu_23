// LoongArch 单周期 CPU 模拟器 —— 全局硬件抽象层（22 条指令）

#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 硬件参数
#define NUM_REGS        32
#define IRAM_SIZE       1024
#define DRAM_SIZE       1024
#define MAX_LABELS      256
#define MAX_LINE_LEN    256
#define MAX_LABEL_LEN   64

// 算术/逻辑指令（Reg3 格式，17-bit opcode）
#define OP_ADD_W        0x20
#define OP_SUB_W        0x22
#define OP_SLT          0x24
#define OP_SLTU         0x25
#define OP_NOR          0x28
#define OP_AND          0x29
#define OP_OR           0x2A
#define OP_XOR          0x2B

// 立即数指令（Reg2I12 格式，10-bit opcode）
#define OP_SLTI         0x08
#define OP_SLTIU        0x09
#define OP_ADDI_W       0x0A
#define OP_ANDI         0x0D
#define OP_ORI          0x0E
#define OP_XORI         0x0F

// 移位指令（Reg2I5 格式，17-bit opcode）
#define OP_SLLI_W       0x81
#define OP_SRLI_W       0x89

// 分支/跳转指令（6-bit opcode）
#define OP_B            0x14
#define OP_BEQ          0x16
#define OP_BNE          0x17
#define OP_BLT          0x18
#define OP_BGE          0x19
#define OP_BLTU         0x1A
#define OP_BGEU         0x1B

// 指令格式枚举
typedef enum {
    FMT_REG3    = 0,
    FMT_REG2I5  = 1,
    FMT_REG2I12 = 2,
    FMT_REG2I16 = 3,
    FMT_I26     = 4
} InstrFormat;

// 符号表条目
typedef struct {
    char     name[MAX_LABEL_LEN];
    uint32_t address;
} SymbolEntry;

// 指令链表节点
typedef struct InstrNode {
    uint32_t          machine_code;
    uint32_t          address;
    char              asm_text[MAX_LINE_LEN];
    struct InstrNode* next;
} InstrNode;

// CPU 硬件状态
typedef struct {
    uint32_t PC;
    uint32_t RF[NUM_REGS];          // $r0 硬件恒为 0
    uint32_t IRAM[IRAM_SIZE];
    uint32_t DRAM[DRAM_SIZE];
    uint32_t cycle_count;
    uint32_t instr_count;
    uint8_t  running;
    uint32_t cur_instr;
    char     cur_asm[128];
} CPU_State;

extern CPU_State cpu;

#endif
