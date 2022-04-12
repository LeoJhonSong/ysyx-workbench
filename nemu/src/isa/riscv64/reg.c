#include "local-include/reg.h"

#include "common.h"
#include "macro.h"

#include <isa.h>
#include <stdio.h>
#include <string.h>

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  int cols = 2;
  for (int i = 0; i < cols; i++) {
    printf(" │ %sReg           %sHex                   %sDec         %s", ASNI_FG_NORMAL_WHITE, ASNI_FG_NORMAL_GREEN, ASNI_FG_NORMAL_CYAN, ASNI_NONE);
  }
  printf(" │\n");
  int n = ARRLEN(regs) / cols;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < cols; j++) {
      printf(" │ " ASNI_FMT("%-3s", ASNI_FG_NORMAL_WHITE), regs[n * j + i]);
      // print the hex by bytes
      word_t value;
      for (int k = sizeof(cpu.gpr[0]) - 1; k >= 0; k--) {
        value = (cpu.gpr[n * j + i] >> 8 * k) & 0xff;
        printf(" %s%02lx%s", value == 0 ? ASNI_DIM : ASNI_FG_NORMAL_GREEN, value, ASNI_NONE);
      }
      value = cpu.gpr[n * j + i];
      printf(" %s%20lu%s", value == 0 ? ASNI_DIM : ASNI_FG_NORMAL_CYAN, value, ASNI_NONE);
    }
    printf(" │ \n");
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  for (int i = 0; i < ARRLEN(regs); i++) {
    if (strcmp(s, regs[i]) == 0) {
      *success = true;
      return cpu.gpr[i];
    }
  }
  *success = false;
  return 0;
}
