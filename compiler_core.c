// 两遍扫描汇编器实现（22 条指令）
// Pass 1: 收集标签 → 符号表; Pass 2: 翻译指令 → 机器码

#include "compiler_core.h"

static char* trim(char* str)
{
    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

static void remove_comment(char* line)
{
    char* p = strchr(line, ';');
    if (p) *p = '\0';
}

static int is_label_line(const char* line)
{
    const char* p = line;
    while (*p && !isspace((unsigned char)*p) && *p != ':') p++;
    return (*p == ':') ? 1 : 0;
}

static void extract_label(const char* line, char* buf, int buf_size)
{
    int i = 0;
    while (*line && *line != ':' && i < buf_size - 1) {
        buf[i++] = *line++;
    }
    buf[i] = '\0';
}

static const char* skip_label(const char* line)
{
    while (*line && *line != ':') line++;
    if (*line == ':') line++;
    while (*line && isspace((unsigned char)*line)) line++;
    return line;
}

static int parse_register(const char* token, uint32_t* reg_num)
{
    if (token[0] == '$' && (token[1] == 'r' || token[1] == 'R')) {
        char* end;
        long val = strtol(token + 2, &end, 10);
        if (end != token + 2 && val >= 0 && val <= 31) {
            *reg_num = (uint32_t)val;
            return 1;
        }
    }
    return 0;
}

int symbol_cmp(const void* a, const void* b)
{
    const SymbolEntry* sa = (const SymbolEntry*)a;
    const SymbolEntry* sb = (const SymbolEntry*)b;
    return strcmp(sa->name, sb->name);
}

int symbol_bsearch(const SymbolEntry* symbols, int n, const char* name)
{
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int cmp = strcmp(symbols[mid].name, name);
        if (cmp == 0)
            return mid;
        else if (cmp < 0)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}

int translate_line(const char* line,
                   const SymbolEntry* symbols,
                   int num_symbols,
                   uint32_t current_addr,
                   uint32_t* machine_code)
{
    char buf[MAX_LINE_LEN];
    strncpy(buf, line, MAX_LINE_LEN - 1);
    buf[MAX_LINE_LEN - 1] = '\0';

    remove_comment(buf);
    char* trimmed = trim(buf);
    if (strlen(trimmed) == 0) return 0;

    char mnemonic[32];
    char rest[MAX_LINE_LEN];
    if (sscanf(trimmed, "%31s %[^\n]", mnemonic, rest) < 1) return 0;

    for (char* p = rest; *p; p++)
        if (*p == ',') *p = ' ';

    // Reg3 格式指令（三寄存器）
    if (strcmp(mnemonic, "ADD.W") == 0 || strcmp(mnemonic, "add.w") == 0 ||
        strcmp(mnemonic, "SUB.W") == 0 || strcmp(mnemonic, "sub.w") == 0 ||
        strcmp(mnemonic, "AND")   == 0 || strcmp(mnemonic, "and")   == 0 ||
        strcmp(mnemonic, "OR")    == 0 || strcmp(mnemonic, "or")    == 0 ||
        strcmp(mnemonic, "XOR")   == 0 || strcmp(mnemonic, "xor")   == 0 ||
        strcmp(mnemonic, "NOR")   == 0 || strcmp(mnemonic, "nor")   == 0 ||
        strcmp(mnemonic, "SLT")   == 0 || strcmp(mnemonic, "slt")   == 0 ||
        strcmp(mnemonic, "SLTU")  == 0 || strcmp(mnemonic, "sltu")  == 0)
    {
        char tok1[32], tok2[32], tok3[32];
        if (sscanf(rest, "%31s %31s %31s", tok1, tok2, tok3) != 3)
            return 0;

        uint32_t rd, rj, rk;
        if (!parse_register(tok1, &rd)) return 0;
        if (!parse_register(tok2, &rj)) return 0;
        if (!parse_register(tok3, &rk)) return 0;

        uint32_t op;
        if      (strcmp(mnemonic, "ADD.W") == 0 || strcmp(mnemonic, "add.w") == 0) op = OP_ADD_W;
        else if (strcmp(mnemonic, "SUB.W") == 0 || strcmp(mnemonic, "sub.w") == 0) op = OP_SUB_W;
        else if (strcmp(mnemonic, "AND")   == 0 || strcmp(mnemonic, "and")   == 0) op = OP_AND;
        else if (strcmp(mnemonic, "OR")    == 0 || strcmp(mnemonic, "or")    == 0) op = OP_OR;
        else if (strcmp(mnemonic, "XOR")   == 0 || strcmp(mnemonic, "xor")   == 0) op = OP_XOR;
        else if (strcmp(mnemonic, "NOR")   == 0 || strcmp(mnemonic, "nor")   == 0) op = OP_NOR;
        else if (strcmp(mnemonic, "SLT")   == 0 || strcmp(mnemonic, "slt")   == 0) op = OP_SLT;
        else if (strcmp(mnemonic, "SLTU")  == 0 || strcmp(mnemonic, "sltu")  == 0) op = OP_SLTU;
        else return 0;

        // Reg3 编码：op(17b)@31:15 | rk(5b)@14:10 | rj(5b)@9:5 | rd(5b)@4:0
        *machine_code = 0;
        *machine_code |= (op  & 0x1FFFF) << 15;
        *machine_code |= (rk  & 0x1F) << 10;
        *machine_code |= (rj  & 0x1F) << 5;
        *machine_code |= (rd  & 0x1F);
        return 1;
    }

    // Reg2I5 格式（移位指令）
    if (strcmp(mnemonic, "SLLI.W") == 0 || strcmp(mnemonic, "slli.w") == 0 ||
        strcmp(mnemonic, "SRLI.W") == 0 || strcmp(mnemonic, "srli.w") == 0)
    {
        char tok1[32], tok2[32], tok3[32];
        if (sscanf(rest, "%31s %31s %31s", tok1, tok2, tok3) != 3)
            return 0;

        uint32_t rd, rj;
        if (!parse_register(tok1, &rd)) return 0;
        if (!parse_register(tok2, &rj)) return 0;

        int32_t imm = (int32_t)strtol(tok3, NULL, 0);
        if (imm < 0 || imm > 31) {
            fprintf(stderr, "[asm] 移位量 %d 越界 (0-31)\n", imm);
            return 0;
        }

        uint32_t op;
        if      (strcmp(mnemonic, "SLLI.W") == 0 || strcmp(mnemonic, "slli.w") == 0) op = OP_SLLI_W;
        else if (strcmp(mnemonic, "SRLI.W") == 0 || strcmp(mnemonic, "srli.w") == 0) op = OP_SRLI_W;
        else return 0;

        // Reg2I5 编码：op(17b)@31:15 | imm(5b)@14:10 | rj(5b)@9:5 | rd(5b)@4:0
        *machine_code = 0;
        *machine_code |= (op  & 0x1FFFF) << 15;
        *machine_code |= ((uint32_t)imm & 0x1F) << 10;
        *machine_code |= (rj  & 0x1F) << 5;
        *machine_code |= (rd  & 0x1F);
        return 1;
    }

    // Reg2I12 格式指令（两寄存器 + 12位立即数）
    if (strcmp(mnemonic, "ADDI.W") == 0 || strcmp(mnemonic, "addi.w") == 0 ||
        strcmp(mnemonic, "ANDI")   == 0 || strcmp(mnemonic, "andi")   == 0 ||
        strcmp(mnemonic, "ORI")    == 0 || strcmp(mnemonic, "ori")    == 0 ||
        strcmp(mnemonic, "XORI")   == 0 || strcmp(mnemonic, "xori")   == 0 ||
        strcmp(mnemonic, "SLTI")   == 0 || strcmp(mnemonic, "slti")   == 0 ||
        strcmp(mnemonic, "SLTIU")  == 0 || strcmp(mnemonic, "sltiu")  == 0)
    {
        char tok1[32], tok2[32], tok3[32];
        if (sscanf(rest, "%31s %31s %31s", tok1, tok2, tok3) != 3)
            return 0;

        uint32_t rd, rj;
        if (!parse_register(tok1, &rd)) return 0;
        if (!parse_register(tok2, &rj)) return 0;

        int32_t imm = (int32_t)strtol(tok3, NULL, 0);

        uint32_t op;
        if      (strcmp(mnemonic, "ADDI.W") == 0 || strcmp(mnemonic, "addi.w") == 0) op = OP_ADDI_W;
        else if (strcmp(mnemonic, "ANDI")   == 0 || strcmp(mnemonic, "andi")   == 0) op = OP_ANDI;
        else if (strcmp(mnemonic, "ORI")    == 0 || strcmp(mnemonic, "ori")    == 0) op = OP_ORI;
        else if (strcmp(mnemonic, "XORI")   == 0 || strcmp(mnemonic, "xori")   == 0) op = OP_XORI;
        else if (strcmp(mnemonic, "SLTI")   == 0 || strcmp(mnemonic, "slti")   == 0) op = OP_SLTI;
        else if (strcmp(mnemonic, "SLTIU")  == 0 || strcmp(mnemonic, "sltiu")  == 0) op = OP_SLTIU;
        else return 0;

        // Reg2I12 编码：op(10b)@31:22 | imm(12b)@21:10 | rj(5b)@9:5 | rd(5b)@4:0
        *machine_code = 0;
        *machine_code |= (op  & 0x3FF) << 22;
        *machine_code |= ((uint32_t)imm & 0xFFF) << 10;
        *machine_code |= (rj  & 0x1F) << 5;
        *machine_code |= (rd  & 0x1F);
        return 1;
    }

    // I26 格式：B label（无条件跳转）
    if (strcmp(mnemonic, "B") == 0 || strcmp(mnemonic, "b") == 0) {
        char label[MAX_LABEL_LEN];
        if (sscanf(rest, "%63s", label) != 1) return 0;

        int idx = symbol_bsearch(symbols, num_symbols, label);
        if (idx < 0) {
            fprintf(stderr, "[asm] 未定义标签 '%s'\n", label);
            return 0;
        }

        int32_t offset = (int32_t)(symbols[idx].address - current_addr) / 4;

        *machine_code = 0;
        *machine_code |= (OP_B & 0x3F) << 26;
        *machine_code |= ((uint32_t)offset & 0x03FFFFFF);
        return 1;
    }

    // Reg2I16 格式：条件分支（BEQ, BNE, BLT, BGE, BLTU, BGEU）
    if (strcmp(mnemonic, "BEQ")  == 0 || strcmp(mnemonic, "beq")  == 0 ||
        strcmp(mnemonic, "BNE")  == 0 || strcmp(mnemonic, "bne")  == 0 ||
        strcmp(mnemonic, "BLT")  == 0 || strcmp(mnemonic, "blt")  == 0 ||
        strcmp(mnemonic, "BGE")  == 0 || strcmp(mnemonic, "bge")  == 0 ||
        strcmp(mnemonic, "BLTU") == 0 || strcmp(mnemonic, "bltu") == 0 ||
        strcmp(mnemonic, "BGEU") == 0 || strcmp(mnemonic, "bgeu") == 0)
    {
        char tok1[32], tok2[32], label[MAX_LABEL_LEN];
        if (sscanf(rest, "%31s %31s %63s", tok1, tok2, label) != 3)
            return 0;

        uint32_t rj, rd;
        if (!parse_register(tok1, &rj)) return 0;
        if (!parse_register(tok2, &rd)) return 0;

        int idx = symbol_bsearch(symbols, num_symbols, label);
        if (idx < 0) {
            fprintf(stderr, "[asm] 未定义标签 '%s'\n", label);
            return 0;
        }

        int32_t offset = (int32_t)(symbols[idx].address - current_addr) / 4;
        if (offset < -32768 || offset > 32767) {
            fprintf(stderr, "[asm] 分支偏移 %d 超出16位有符号范围\n", offset);
            return 0;
        }

        uint32_t op;
        if      (strcmp(mnemonic, "BEQ")  == 0 || strcmp(mnemonic, "beq")  == 0) op = OP_BEQ;
        else if (strcmp(mnemonic, "BNE")  == 0 || strcmp(mnemonic, "bne")  == 0) op = OP_BNE;
        else if (strcmp(mnemonic, "BLT")  == 0 || strcmp(mnemonic, "blt")  == 0) op = OP_BLT;
        else if (strcmp(mnemonic, "BGE")  == 0 || strcmp(mnemonic, "bge")  == 0) op = OP_BGE;
        else if (strcmp(mnemonic, "BLTU") == 0 || strcmp(mnemonic, "bltu") == 0) op = OP_BLTU;
        else if (strcmp(mnemonic, "BGEU") == 0 || strcmp(mnemonic, "bgeu") == 0) op = OP_BGEU;
        else return 0;

        // Reg2I16 编码：op(6b)@31:26 | offset(16b)@25:10 | rj(5b)@9:5 | rd(5b)@4:0
        *machine_code = 0;
        *machine_code |= (op  & 0x3F) << 26;
        *machine_code |= ((uint32_t)offset & 0xFFFF) << 10;
        *machine_code |= (rj  & 0x1F) << 5;
        *machine_code |= (rd  & 0x1F);
        return 1;
    }

    fprintf(stderr, "[asm] 未知指令 '%s'\n", mnemonic);
    return 0;
}

int assemble(const char* asm_file, InstrNode** head)
{
    FILE* fp = fopen(asm_file, "r");
    if (!fp) {
        perror("[asm] 无法打开文件");
        return -1;
    }

    // Pass 1：收集标签
    SymbolEntry symbols[MAX_LABELS];
    int num_symbols = 0;
    uint32_t current_addr = 0;
    char line_buf[MAX_LINE_LEN];

    printf("[asm] Pass 1：第一遍扫描（收集标签）\n");

    while (fgets(line_buf, MAX_LINE_LEN, fp)) {
        remove_comment(line_buf);
        char* trimmed = trim(line_buf);
        if (strlen(trimmed) == 0) continue;

        if (is_label_line(trimmed)) {
            if (num_symbols >= MAX_LABELS) {
                fprintf(stderr, "[asm] 符号表溢出！\n");
                fclose(fp);
                return -2;
            }
            extract_label(trimmed, symbols[num_symbols].name, MAX_LABEL_LEN);
            symbols[num_symbols].address = current_addr;
            num_symbols++;

            const char* after = skip_label(trimmed);
            if (strlen(after) > 0)
                current_addr += 4;
        } else {
            current_addr += 4;
        }
    }

    qsort(symbols, num_symbols, sizeof(SymbolEntry), symbol_cmp);

    printf("[asm] 收集到 %d 个标签，已按 ASCII 排序：\n", num_symbols);
    for (int i = 0; i < num_symbols; i++) {
        printf("  [%2d] %-20s → 0x%08X\n", i, symbols[i].name, symbols[i].address);
    }

    // Pass 2：翻译机器码
    printf("[asm] Pass 2：第二遍扫描（翻译机器码）\n");

    rewind(fp);
    current_addr = 0;
    *head = NULL;
    InstrNode* tail = NULL;
    int instr_count = 0;

    while (fgets(line_buf, MAX_LINE_LEN, fp)) {
        remove_comment(line_buf);
        char* trimmed = trim(line_buf);
        if (strlen(trimmed) == 0) continue;

        if (is_label_line(trimmed)) {
            const char* after = skip_label(trimmed);
            if (strlen(after) > 0) {
                trimmed = (char*)after;
            } else {
                continue;
            }
        }

        uint32_t machine_code = 0;
        int ret = translate_line(trimmed, symbols, num_symbols,
                                  current_addr, &machine_code);
        if (!ret) {
            fprintf(stderr, "[asm] 翻译失败 @0x%08X: %s\n", current_addr, trimmed);
            fclose(fp);
            return -3;
        }

        InstrNode* node = (InstrNode*)malloc(sizeof(InstrNode));
        if (!node) {
            perror("[asm] malloc 失败");
            fclose(fp);
            return -4;
        }
        node->machine_code = machine_code;
        node->address = current_addr;
        strncpy(node->asm_text, trimmed, MAX_LINE_LEN - 1);
        node->asm_text[MAX_LINE_LEN - 1] = '\0';
        node->next = NULL;

        if (*head == NULL) {
            *head = node;
            tail = node;
        } else {
            tail->next = node;
            tail = node;
        }

        current_addr += 4;
        instr_count++;
    }

    fclose(fp);
    printf("[asm] 翻译完成：%d 条指令\n", instr_count);
    return instr_count;
}
