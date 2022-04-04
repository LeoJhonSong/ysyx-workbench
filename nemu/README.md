# NEMU

## 配置

首先需要运行`make menuconfig`进行[配置](https://docs.ysyx.org/ics-pa/1.3.html#%E9%85%8D%E7%BD%AE%E7%B3%BB%E7%BB%9Fkconfig)

需要改的配置:
```yaml
- Base ISA: riscv64
- Build Options:
  - Enable debug information: Yes
```

## sdb

sdb: **S**imple **D**e**B**uger

| 命令         | 格式          | 使用举例          | 说明                                                         |
| ------------ | ------------- | ----------------- | ------------------------------------------------------------ |
| 帮助(1)      | `help`        | `help`            | 打印命令的帮助信息                                           |
| 继续运行(1)  | `c`           | `c`               | 继续运行被暂停的程序                                         |
| 退出(1)      | `q`           | `q`               | 退出NEMU                                                     |
| 单步执行     | `si [N]`      | `si 10`           | 让程序单步执行`N`条指令后暂停执行, 当`N`没有给出时, 缺省为`1` |
| 打印程序状态 | `info SUBCMD` | `info r` `info w` | 打印寄存器状态 打印监视点信息                                |
| 扫描内存(2)  | `x N EXPR`    | `x 5 0x80000000`  | 求出表达式`EXPR`的值, 将结果作为起始内存 地址, 以十六进制形式输出连续的`N`个4字节 |
| 表达式求值   | `p EXPR`      | `p $eax + 1`      | 求出表达式`EXPR`的值, `EXPR`支持的 运算请见[调试中的表达式求值](https://docs.ysyx.org/ics-pa/1.6.html)小节 |
| 设置监视点   | `w EXPR`      | `w *0x2000`       | 当表达式`EXPR`的值发生变化时, 暂停程序执行                   |
| 删除监视点   | `d N`         | `d 2`             | 删除序号为`N`的监视点

## Introduction

NEMU(NJU Emulator) is a simple but complete full-system emulator designed for teaching purpose.
Currently it supports x86, mips32, riscv32 and riscv64.
To build programs run above NEMU, refer to the [AM project](https://github.com/NJU-ProjectN/abstract-machine).

The main features of NEMU include
* a small monitor with a simple debugger
  * single step
  * register/memory examination
  * expression evaluation without the support of symbols
  * watch point
  * differential testing with reference design (e.g. QEMU)
  * snapshot
* CPU core with support of most common used instructions
  * x86
    * real mode is not supported
    * x87 floating point instructions are not supported
  * mips32
    * CP1 floating point instructions are not supported
  * riscv32
    * only RV32IM
  * riscv64
    * only RV64IM
* memory
* paging
  * TLB is optional (but necessary for mips32)
  * protection is not supported
* interrupt and exception
  * protection is not supported
* 5 devices
  * serial, timer, keyboard, VGA, audio
  * most of them are simplified and unprogrammable
* 2 types of I/O
  * port-mapped I/O and memory-mapped I/O
