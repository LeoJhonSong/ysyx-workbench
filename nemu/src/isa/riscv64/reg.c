#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  int i;
  int cols = 4;
  int size = 4; // each gpr is 4 bytes
  for (i = 0; i < cols; i++) {
    printf("\33[1;37mReg     \33[1;32mHex        \33[1;36mDec     \33[0m│ ");
  }
  printf("\n");
  for (i = 0; i < ARRLEN(regs); i++) {
    // register name in white color and value in green color
    printf("\33[1;37m%-3s \33[1;32m", regs[i]);
    for (int j = size - 1; j >= 0; j--) {
      printf("%02lx ", (cpu.gpr[i] >> 8 * j) & 0xff);
    }
    printf("\33[1;36m%10lu\33[0m │ ", cpu.gpr[i]);
    if (i % cols == cols - 1) {
      printf("\n");
    }
  }
  if ((i - 1) % cols != cols - 1) {
    printf("\n");
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  return 0;
}
