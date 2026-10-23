// 动态链表与虚拟内存管理

#include "storage.h"
#include "cpu_core.h"

void list_load_to_iram(InstrNode* head)
{
    InstrNode* p = head;
    while (p) {
        cpu_load_instr(p->address, p->machine_code);
        p = p->next;
    }
    printf("[storage] 已加载 %d 条指令到 IRAM\n", list_length(head));
}

void list_free_all(InstrNode** head)
{
    int count = 0;
    InstrNode* p = *head;
    while (p) {
        InstrNode* next = p->next;
        free(p);
        p = next;
        count++;
    }
    *head = NULL;
    printf("[storage] 已释放 %d 个链表节点，零内存泄漏 ✅\n", count);
}

int list_length(InstrNode* head)
{
    int n = 0;
    for (InstrNode* p = head; p; p = p->next) n++;
    return n;
}

void list_print(InstrNode* head)
{
    printf("\n[storage] 指令链表内容：\n");
    printf("┌────────────┬────────────┬──────────────────────────────────┐\n");
    printf("│  Address   │ Machine Code│  Assembly                        │\n");
    printf("├────────────┼────────────┼──────────────────────────────────┤\n");
    for (InstrNode* p = head; p; p = p->next) {
        printf("│ 0x%08X │ 0x%08X │ %-32s │\n",
               p->address, p->machine_code, p->asm_text);
    }
    printf("└────────────┴────────────┴──────────────────────────────────┘\n");
}
