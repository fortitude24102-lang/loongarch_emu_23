// 动态链表与虚拟内存管理 —— 头文件

#ifndef STORAGE_H
#define STORAGE_H

#include "global.h"

// 尾插法追加节点
InstrNode* list_append(InstrNode** head, uint32_t machine_code,
                       uint32_t address, const char* asm_text);

// 链表 → IRAM 加载
void list_load_to_iram(InstrNode* head);

// 释放链表
void list_free_all(InstrNode** head);

int list_length(InstrNode* head);
void list_print(InstrNode* head);

#endif
