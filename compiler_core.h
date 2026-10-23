// 两遍扫描汇编器 —— 头文件（22 条指令）

#ifndef COMPILER_CORE_H
#define COMPILER_CORE_H

#include "global.h"

// 汇编编译主函数（两遍扫描：Pass1收集标签, Pass2翻译机器码）
int assemble(const char* asm_file, InstrNode** head);

// 单行翻译：汇编文本 → 32位机器码
int translate_line(const char* line,
                   const SymbolEntry* symbols,
                   int num_symbols,
                   uint32_t current_addr,
                   uint32_t* machine_code);

// 二分查找符号表（已按 name 排序），O(log N)
int symbol_bsearch(const SymbolEntry* symbols, int n, const char* name);

// qsort 比较函数（按标签名 ASCII 排序）
int symbol_cmp(const void* a, const void* b);

#endif // COMPILER_CORE_H
