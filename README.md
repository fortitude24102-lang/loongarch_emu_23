# loongarch_emu_22

基于 C99 的 LoongArch 单周期 CPU 行为级教学模拟器，集成两遍扫描汇编器、指令译码、寄存器执行状态追踪和运行统计。适合学习汇编指令编码、分支控制流，以及取指、译码、执行、写回和 PC 更新的过程。

> 仓库名沿用项目的“22 条指令”版本名称。按当前源码中的指令助记符统计，实际实现 **23 条指令**：8 条三寄存器指令、6 条立即数指令、2 条移位指令和 7 条分支/跳转指令。

## 功能与执行流程

```text
汇编源文件 (.asm)
    ↓ 第一遍扫描：收集标签、排序符号表
    ↓ 第二遍扫描：解析操作数、生成 32 位机器码
指令链表
    ↓ 加载到虚拟指令存储器 IRAM
单周期执行：取指 → 译码 → 运算/分支 → 写回 → 更新 PC
    ↓
活跃寄存器追踪、最终寄存器值、周期与耗时统计
```

- 32 个 32 位通用寄存器，`$r0` 始终为零。
- IRAM 容量为 1024 个 32 位字，PC 从地址 `0` 开始。
- 支持标签、向前/向后分支，可表达循环与条件判断。
- 每次成功执行计为一个模拟周期；主程序最多执行 200 个周期。
- 使用 C 标准库和 `<sys/time.h>`，无需第三方库。

## 快速开始

推荐使用 Linux 或 Windows 下的 WSL Ubuntu，需要 GCC 和 GNU Make。终端建议使用 UTF-8 编码，以显示中文和表格字符。

```bash
git clone https://github.com/fortitude24102-lang/loongarch_emu_22.git
cd loongarch_emu_22
make
make run
```

也可以直接编译和运行：

```bash
gcc -Wall -Wextra -std=c99 -g -O2 \
    main.c decoder.c cpu_core.c compiler_core.c storage.c ui.c \
    -o loongarch_emu
./loongarch_emu examples/demo.asm
```

Windows 用户可以在 WSL 终端中执行上述命令。使用原生 MinGW 时，需要完整的 GCC 工具链且支持 `<sys/time.h>` / `gettimeofday`；可将输出文件名改为 `loongarch_emu.exe`。

### 常用命令

| 命令 | 作用 |
| --- | --- |
| `make` | 编译模拟器 |
| `make run` | 运行 `examples/demo.asm` |
| `make run ASM=path/to/program.asm` | 运行指定汇编文件 |
| `./loongarch_emu path/to/program.asm` | 直接运行已有可执行文件 |
| `make clean` | 删除编译产物 |
| `make help` | 查看构建命令 |
| `make valgrind` | 使用另行安装的 Valgrind 检查示例运行的内存使用 |

当前命令行只接受一个汇编文件参数，运行模式为连续执行。

## 示例：计算 1 到 10 的和

完整示例见 [examples/demo.asm](examples/demo.asm)：

```asm
    ADDI.W $r1, $r0, 1
    ADDI.W $r2, $r0, 10
    ADDI.W $r3, $r0, 0

loop:
    ADD.W  $r3, $r3, $r1
    ADDI.W $r1, $r1, 1
    BGE    $r2, $r1, loop

done:
    B done
```

运行后，最终寄存器输出应包含：

```text
$r1  = 0x0000000B  (11)
$r2  = 0x0000000A  (10)
$r3  = 0x00000037  (55)
```

`B done` 在求和结束后保持寄存器结果，直到达到 200 周期限制。此时显示的周期上限警告是该示例的预期行为。

## 支持的指令

| 类型 | 指令 | 操作数格式 |
| --- | --- | --- |
| 算术 | `ADD.W`, `SUB.W` | `rd, rj, rk` |
| 逻辑 | `AND`, `OR`, `XOR`, `NOR` | `rd, rj, rk` |
| 比较 | `SLT`, `SLTU` | `rd, rj, rk` |
| 算术/比较立即数 | `ADDI.W`, `SLTI`, `SLTIU` | `rd, rj, imm12` |
| 逻辑立即数 | `ANDI`, `ORI`, `XORI` | `rd, rj, imm12` |
| 逻辑移位 | `SLLI.W`, `SRLI.W` | `rd, rj, imm5` |
| 条件分支 | `BEQ`, `BNE`, `BLT`, `BGE`, `BLTU`, `BGEU` | `rj, rd, label` |
| 无条件跳转 | `B` | `label` |

`SLT` / `SLTI`、`BLT` / `BGE` 使用有符号比较；带 `U` 的对应指令使用无符号比较。`ADDI.W`、`SLTI` 和 `SLTIU` 的 12 位立即数在译码时符号扩展，逻辑立即数零扩展。

### 汇编语法

- 寄存器写作 `$r0`～`$r31`，也接受 `$R` 前缀。
- 助记符支持全大写或全小写；建议统一使用全大写。
- 操作数用逗号或空白分隔。
- 立即数按 C 风格解析：十进制、`0x` 十六进制、前导 `0` 八进制，可带正负号。
- 移位量范围为 `0`～`31`；有符号 12 位立即数应使用 `-2048`～`2047`，逻辑立即数应使用 `0`～`4095`。当前汇编器对 12 位立即数直接截取低位，编写时需自行保证范围。
- 标签写作 `name:`，可单独占一行，也可放在指令前；分支目标使用标签名。
- 使用分号 `;` 写注释；空行会被忽略。
- 不支持 `.text` / `.data` 等汇编伪指令、寄存器 ABI 别名或操作系统调用。

## 项目结构

```text
loongarch_emu_22/
├── main.c               # 汇编、加载和仿真主流程
├── global.h             # 硬件参数、操作码、全局数据结构
├── compiler_core.c/.h   # 两遍扫描汇编器与符号表查找
├── decoder.c/.h         # 指令格式、位域译码与反汇编文本
├── cpu_core.c/.h        # CPU 状态与单周期执行
├── storage.c/.h         # 指令链表、IRAM 加载与内存释放
├── ui.c/.h              # 寄存器显示与运行统计
├── examples/demo.asm    # 循环求和示例
├── Makefile             # 编译与运行入口
├── .gitattributes       # 文本换行规则
└── .gitignore           # 编译产物和本地配置忽略规则
```

## 当前实现范围

- 这是用于课程学习的行为级模拟器，使用配套汇编器生成并执行机器码；未验证与官方 LoongArch 工具链的二进制兼容性。
- 使用联合体和 C 位域译码，位域布局依赖编译器和目标平台；移植到其他平台时需验证。
- 没有 `HALT` 指令，也不会因到达已加载程序末尾而自动结束；当前停止条件是达到 200 周期上限或 PC 超出 IRAM 范围。零填充区域及未识别指令当前不会触发异常。
- DRAM 预留了 1024 个 32 位字，但当前指令子集没有加载/存储指令；未实现流水线、缓存、中断或异常处理。
- 符号表上限为 256 项。输入应使用简短且不重复的标签，并保持每行少于 256 字节。
- 耗时和 MIPS 来自主机墙钟时间，包含终端输出开销，用于观察本程序运行情况，不代表实际 LoongArch 硬件性能。
