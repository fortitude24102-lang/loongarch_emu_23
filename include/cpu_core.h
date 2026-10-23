#ifndef CPU_CORE_H
#define CPU_CORE_H

#include "global.h"
#include "decoder.h"

void cpu_init(void);
void cpu_load_instr(uint32_t address, uint32_t machine_code);

// 单周期执行：取指→译码→执行→写回→PC更新
int cpu_step(void);

uint32_t cpu_get_pc(void);
uint32_t cpu_get_cycle(void);

#endif /* CPU_CORE_H */
