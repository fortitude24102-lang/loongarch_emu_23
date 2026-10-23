// CPU 状态监视器与可视化实现

#include "ui.h"
#include "cpu_core.h"
#include "decoder.h"

void ui_print_banner(void)
{
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║     LoongArch 单周期 CPU 行为级模拟器 v2.0               ║\n");
    printf("║     指令集：22 条 (算术/逻辑/比较/分支)                  ║\n");
    printf("║     支持 if / for 控制流 | 集成电路与微电子专业          ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
}

void ui_print_cpu_state(void)
{
    printf("├─────────────────────────────────────────────────────────┤\n");
    printf("│ 活跃寄存器追踪 (Active Registers):                       │\n");
    printf("├─────────────────────────────────────────────────────────┤\n");

    if (cpu.cur_instr == 0) {
        printf("│  无 (NOP 或暂无指令)                                     │\n");
    } else {
        DecodedInstr dec;
        decode(cpu.cur_instr, &dec);

        if (dec.format == FMT_REG3) {
            printf("│  [rd 目标] $r%-2d = 0x%08X                             │\n", dec.rd, cpu.RF[dec.rd]);
            printf("│  [rj 源1]  $r%-2d = 0x%08X                             │\n", dec.rj, cpu.RF[dec.rj]);
            printf("│  [rk 源2]  $r%-2d = 0x%08X                             │\n", dec.rk, cpu.RF[dec.rk]);
        } else if (dec.format == FMT_REG2I12 || dec.format == FMT_REG2I16) {
            printf("│  [rd]      $r%-2d = 0x%08X                             │\n", dec.rd, cpu.RF[dec.rd]);
            printf("│  [rj]      $r%-2d = 0x%08X                             │\n", dec.rj, cpu.RF[dec.rj]);
        } else if (dec.format == FMT_I26) {
            printf("│  (无条件跳转 B，不涉及通用寄存器)                        │\n");
        }
    }
    printf("└──────────────────────────────────────────────────────────┘\n\n");
}

void ui_print_summary(void)
{
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║                仿真结束 (HALT)                       ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  总周期: %-6u   总指令: %-6u   最终PC: 0x%08X ║\n",
           cpu.cycle_count, cpu.instr_count, cpu.PC);
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    printf("[最终非零寄存器]\n");
    for (int i = 1; i < 32; i++) {
        if (cpu.RF[i] != 0)
            printf("  $r%-2d = 0x%08X  (%d)\n", i, cpu.RF[i], cpu.RF[i]);
    }
    printf("\n");
}

void ui_print_timing(const struct timeval* start, const struct timeval* end)
{
    long sec  = end->tv_sec  - start->tv_sec;
    long usec = end->tv_usec - start->tv_usec;
    double elapsed_ms = (double)sec * 1000.0 + (double)usec / 1000.0;
    double elapsed_s  = elapsed_ms / 1000.0;

    uint32_t instr_count = cpu.instr_count;

    double mips = (elapsed_s > 0.0) ? (double)instr_count / elapsed_s / 1000000.0 : 0.0;
    double ns_per_cycle = (instr_count > 0) ? (elapsed_s * 1e9) / (double)instr_count : 0.0;

    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║              ⏱  性能统计 (Timing)                    ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  墙钟耗时:    %10.3f  ms  (%6.3f s)                   ║\n",
           elapsed_ms, elapsed_s);
    printf("║  模拟指令:    %10u 条                                 ║\n", instr_count);
    printf("║  模拟速率:    %10.3f  MIPS                            ║\n", mips);
    printf("║  每指令耗时:  %10.1f  ns                              ║\n", ns_per_cycle);
    printf("╚══════════════════════════════════════════════════════╝\n\n");
}
