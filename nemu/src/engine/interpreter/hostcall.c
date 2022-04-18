#include <stdint.h>
#include <utils.h>
#include <cpu/ifetch.h>
#include <isa.h>
#include <cpu/difftest.h>

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
  ERROR("Invalid instruction!\n\tassemble: %s\n\topcode (last 7 bits): %u\nPC: " FMT_WORD "\n", assemble_str, inst & 0b1111111, thispc);

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
