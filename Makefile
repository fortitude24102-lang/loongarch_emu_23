# ============================================================
# LoongArch 单周期 CPU 模拟器 —— Makefile
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -g -O2
TARGET  = loongarch_emu
OBJS    = main.o decoder.o cpu_core.o compiler_core.o storage.o ui.o
HEADERS = global.h decoder.h cpu_core.h compiler_core.h storage.h ui.h
ASM     ?= examples/demo.asm

ifeq ($(OS),Windows_NT)
TARGET := $(TARGET).exe
endif

.PHONY: all clean run valgrind help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "✅ 编译完成: ./$(TARGET)"

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

# 运行示例，可通过 ASM=path/to/program.asm 指定输入
run: $(TARGET)
	./$(TARGET) "$(ASM)"

# 内存泄漏检测（需安装 valgrind）
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) "$(ASM)"

# 清理
clean:
ifeq ($(OS),Windows_NT)
	powershell -NoProfile -Command "Remove-Item -LiteralPath 'main.o','decoder.o','cpu_core.o','compiler_core.o','storage.o','ui.o','$(TARGET)' -ErrorAction SilentlyContinue"
else
	rm -f $(OBJS) $(TARGET)
endif
	@echo "🧹 清理完成"

help:
	@echo "make          - 编译项目"
	@echo "make run      - 编译并运行 examples/demo.asm"
	@echo "make run ASM=path/to/program.asm - 运行指定汇编文件"
	@echo "make valgrind - 内存泄漏检测"
	@echo "make clean    - 清理编译产物"
