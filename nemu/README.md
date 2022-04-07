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

## 笔记

### 正则

此处正则匹配实现使用的是[POSIX风格](https://www.regular-expressions.info/posix.html)正则表达式 (可以`man 7 regex`查看详细说明), 因为在编译正则表达式时 (在`nemu/src/monitor/sdb/expr.c`的`init_regex()`中) 加了**REG_EXTENDED**标志, 因此确切说使用的是POSIX ERE (Extended Regular Expressions) 风格. 要注意POSIX风格中的字符类概念与其他风格中的字符类并不是同一概念. 其他风格中的[字符类](https://www.regular-expressions.info/charclass.html)是类似`[xz]`的东西, 这在POSIX中称为[括号表达式](https://www.regular-expressions.info/posixbrackets.html). 而POSIX中的[字符类](https://www.regular-expressions.info/posixbrackets.html#Character%20Classes)是在括号表达式里用的: `[xz[:digit:]]`, `[[:alpha:]]`. POSIX风格完全不支持[字符类简写](https://www.regular-expressions.info/shorthand.html), 也完全不支持[前瞻后瞻](https://www.regular-expressions.info/lookaround.html).

💡 Python的re模块使用的是PCRE风格.

#### 关于反斜杠

C中没有原生字符串 (raw string), 因此要匹配元字符本身或者转义字符时使用的字符串实际上需要多加一个`\`. 最极端的例子是匹配`\`, 需要用的字符串为`"\\\\"`. 过程是这样的: `"\\\\"` (字符串) --转义--> `\\` (正则表达式) --转义--> 匹配`\`. 同理, 想匹配`+`就需要字符串`"\\+"`, 而匹配`-`的话因为-不是元字符,就用`"-"`就行.

### 杂

#### used attribute

```c
static int nr_token __attribute__((used))  = 0;
static int keep_this(int) __attribute__((used));
```

> This variable attribute informs the compiler that a static variable is to be retained in the object file, even if it is unreferenced. Static variables marked as used are emitted to a single section, in the order they are declared.

> This function attribute informs the compiler that a static function is to be retained in the object file, even if it is unreferenced. Functions marked with __attribute__((used)) are tagged in the object file to avoid removal by linker unused section removal.

这样一来即便编译器选项设置成有未引用变量/函数就报错, 用了这个属性就能正常保留.

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
