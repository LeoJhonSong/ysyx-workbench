#include "macro.h"

#include <cpu/difftest.h>
#include <cpu/ifetch.h>
#include <isa.h>
#include <stdint.h>
#include <stdio.h>
#include <utils.h>

void set_nemu_state(int state, vaddr_t pc, int halt_ret) {
  difftest_skip_ref();
  nemu_state.state = state;
  nemu_state.halt_pc = pc;
  nemu_state.halt_ret = halt_ret;
}

__attribute__((noinline))
void invalid_inst(vaddr_t thispc) {
  vaddr_t pc = thispc;
  uint32_t inst = inst_fetch(&pc, 4);

  char assemble_str[128];
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(assemble_str, 128, thispc, (uint8_t *)&inst, 4);
  char opcode[8];
  snprintf(opcode, ARRLEN(opcode), "test");
  ERROR("Invalid instruction!\n");
  printf("assemble:\t" ANSI_FMT("%s\n", ANSI_FG_RED), assemble_str);
  printf("opcode:\t\t" ANSI_FMT("%s\n", ANSI_FG_NORMAL_GREEN), opcode);
  printf("PC:\t\t" ANSI_FMT(FMT_WORD "\n", ANSI_FG_WHITE), thispc);

  // printf("There are two cases which will trigger this unexpected exception:\n"
  //     "1. The instruction at PC = " FMT_WORD " is not implemented.\n"
  //     "2. Something is implemented incorrectly.\n", thispc);
  // printf("Find this PC(" FMT_WORD ") in the disassembling result to distinguish which case it is.\n\n", thispc);
  // printf(ANSI_FMT("If it is the first case, see\n%s\nfor more details.\n\n"
  //       "If it is the second case, remember:\n"
  //       "* The machine is always right!\n"
  //       "* Every line of untested code is always wrong!\n\n", ANSI_FG_RED), isa_logo);

  set_nemu_state(NEMU_ABORT, thispc, -1);
}
