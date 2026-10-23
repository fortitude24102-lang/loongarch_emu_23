#include "global.h"
#include "cpu_core.h"
#include "compiler_core.h"
#include "storage.h"
#include "ui.h"

#include <sys/time.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("用法: %s <汇编文件.asm>\n", argv[0]);
        return 1;
    }
    const char* asm_file = argv[1];

    // Phase 1：汇编编译
    ui_print_banner();

    printf("[Phase 1] 汇编编译中...\n");
    printf("  输入文件: %s\n\n", asm_file);

    InstrNode* instr_list = NULL;
    int num_instr = assemble(asm_file, &instr_list);

    if (num_instr < 0) {
        fprintf(stderr, "[Phase 1] 汇编失败，退出。\n");
        list_free_all(&instr_list);
        return 1;
    }
    if (num_instr == 0) {
        fprintf(stderr, "[Phase 1] 警告：未生成任何机器码。\n");
        list_free_all(&instr_list);
        return 1;
    }

    list_print(instr_list);

    // Phase 2：加载到 IRAM
    printf("\n[Phase 2] 加载指令到虚拟 IRAM...\n");

    cpu_init();
    list_load_to_iram(instr_list);

    // Phase 3：主仿真循环
    printf("\n[Phase 3] 启动 CPU 仿真循环...\n");
    printf("───────────────────────────────────────────────────────\n");
    printf("[模式] 连续运行\n");

    struct timeval tv_start, tv_end;
    gettimeofday(&tv_start, NULL);

    const int MAX_CYCLES = 200;

    while (cpu.running && cpu.cycle_count < MAX_CYCLES) {
        int ret = cpu_step();
        if (!ret) break;

        ui_print_cpu_state();
    }

    gettimeofday(&tv_end, NULL);

    if (cpu.cycle_count >= MAX_CYCLES)
        printf("\n[警告] 达到最大周期数限制 (%d)，强制停止。\n", MAX_CYCLES);

    ui_print_summary();
    ui_print_timing(&tv_start, &tv_end);
    list_free_all(&instr_list);

    printf("[完成] 仿真正常退出。\n");
    return 0;
}
