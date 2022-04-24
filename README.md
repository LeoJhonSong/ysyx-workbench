# 一生一芯学习笔记

| 学号     | ysyx_22041024                                                |
| -------- | ------------------------------------------------------------ |
| 学习记录 | [在线表格](https://docs.qq.com/sheet/DYnNBS2hkaFRzbFB6?newPad=1&newPadType=clone&tab=BB08J2) |


## 参考

- [官方文档](https://docs.ysyx.org/)
- [预学习文档](https://docs.ysyx.org/prestudy/prestudy.html)

## Cheat Sheets

```sh
# 进容器
docker exec -it --workdir="/home/student/ysyx-workbench" ysyx-workbench-container zsh -c "source ~/.zshrc && tmux"
# 生成compile_commands.json且彩色输出的make
compiledb make build -j16 | ccze -A
```

```sh
# cpu-tests
cd am-kernels/tests/cpu-tests

# 编译cpu-test/tests下的dummy.c测试并用riscv64-nemu交互模式运行
make ARCH=riscv64-nemu ALL=dummy run
# 依次编译cpu-test/tests下所有测试并用riscv64-nemu批处理模式运行 (也可以加ALL参数运行指定测试)
make ARCH=riscv64-nemu test
# 查看反汇编结果
cat build/dummy-riscv64-nemu.txt
# 用readelf查看.elf/.o文件所有信息
riscv64-linux-gnu-readelf build/dummy-riscv64-nemu.elf -a
# 用objdump查看.o/.elf文件所有反汇编内容
riscv64-linux-gnu-objdump build/riscv64-nemu/tests/dummy.o -D
```

## 使用

### docker

运行`docker/run_ysyx_workbench_container.sh`来创建/启动一个基于[Arch Linux](https://hub.docker.com/r/archlinux/archlinux/)的支持图形界面的ysyx-workbench用容器.

根目录下文件会挂载到容器中`/home/student/ysyx-workbench`下

### nemu

📑 [nemu部分文档](nemu/README.md)

## 要点

### verilator

- [verilator文档](https://verilator.org/guide/latest/index.html)
- [verilator官方示例](https://verilator.org/guide/latest/example_cc.html)
- [verilator教程](https://www.itsembedded.com/dhd/verilator_1/)

支持波形生成的testbench写法见`npc/csrc/tb_top.cpp`

### verilog

- `.v`文件名需要和模块名一致