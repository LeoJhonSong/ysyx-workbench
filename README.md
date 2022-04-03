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
compiledb make build | ccze -A
```

## 使用

### docker

运行`docker-compose-up.sh`来创建/启动一个基于[Arch Linux](https://hub.docker.com/r/archlinux/archlinux/)的支持图形界面的ysyx-workbench用容器.

根目录下文件会挂载到容器中`/home/student/ysyx-workbench`下

### nemu

首先需要运行`make menuconfig`进行[配置](https://docs.ysyx.org/ics-pa/1.3.html#%E9%85%8D%E7%BD%AE%E7%B3%BB%E7%BB%9Fkconfig)

需要改的配置:
```yaml
- Base ISA: riscv64
- Build Options:
  - Enable debug information: Yes
```

## 要点

### verilator

- [verilator文档](https://verilator.org/guide/latest/index.html)
- [verilator官方示例](https://verilator.org/guide/latest/example_cc.html)
- [verilator教程](https://www.itsembedded.com/dhd/verilator_1/)

支持波形生成的testbench写法见`npc/csrc/tb_top.cpp`

### verilog

- `.v`文件名需要和模块名一致