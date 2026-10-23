// CPU 状态监视器与可视化 —— 头文件

#ifndef UI_H
#define UI_H

#include "global.h"
#include <sys/time.h>

void ui_print_banner(void);
void ui_print_cpu_state(void);
void ui_print_summary(void);
void ui_print_timing(const struct timeval* start, const struct timeval* end);

#endif
