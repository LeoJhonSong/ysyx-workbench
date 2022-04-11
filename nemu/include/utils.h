#ifndef __UTILS_H__
#define __UTILS_H__

#include <common.h>

// ----------- state -----------

enum { NEMU_RUNNING, NEMU_STOP, NEMU_END, NEMU_ABORT, NEMU_QUIT };

typedef struct {
  int state;
  vaddr_t halt_pc;
  uint32_t halt_ret;
} NEMUState;

extern NEMUState nemu_state;

// ----------- timer -----------

uint64_t get_time();

// ----------- log -----------

// FIXME: typo ASNI -> ANSI
#define ASNI_FG_BLACK           "\33[1;30m"
#define ASNI_FG_RED             "\33[1;31m"
#define ASNI_FG_GREEN           "\33[1;32m"
#define ASNI_FG_YELLOW          "\33[1;33m"
#define ASNI_FG_BLUE            "\33[1;34m"
#define ASNI_FG_MAGENTA         "\33[1;35m"
#define ASNI_FG_CYAN            "\33[1;36m"
#define ASNI_FG_WHITE           "\33[1;37m"
#define ASNI_FG_PINK            "\33[1;38;5;225m"
#define ASNI_FG_NORMAL_BLACK    "\33[30m"
#define ASNI_FG_NORMAL_RED      "\33[31m"
#define ASNI_FG_NORMAL_GREEN    "\33[32m"
#define ASNI_FG_NORMAL_YELLOW   "\33[33m"
#define ASNI_FG_NORMAL_BLUE     "\33[34m"
#define ASNI_FG_NORMAL_MAGENTA  "\33[35m"
#define ASNI_FG_NORMAL_CYAN     "\33[36m"
#define ASNI_FG_NORMAL_WHITE    "\33[37m"
#define ASNI_FG_NORMAL_PINK     "\33[38;5;183m"
#define ASNI_BG_BLACK           "\33[1;40m"
#define ASNI_BG_RED             "\33[1;41m"
#define ASNI_BG_GREEN           "\33[1;42m"
#define ASNI_BG_YELLOW          "\33[1;43m"
#define ASNI_BG_BLUE            "\33[1;44m"
#define ASNI_BG_MAGENTA         "\33[1;35m"
#define ASNI_BG_CYAN            "\33[1;46m"
#define ASNI_BG_WHITE           "\33[1;47m"
#define ASNI_BG_PINK            "\33[1;48;5;183m"
#define ASNI_DIM                "\33[2m"
#define ASNI_NONE               "\33[0m"

#define ASNI_FMT(str, fmt) fmt str ASNI_NONE

// about __VA_OPT__, __VA_ARGS__, see: https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
#define ERROR(str, ...) printf(ASNI_FMT(str, ASNI_FG_RED) __VA_OPT__(,) __VA_ARGS__)

#define log_write(...) IFDEF(CONFIG_TARGET_NATIVE_ELF, \
  do { \
    extern FILE* log_fp; \
    extern bool log_enable(); \
    if (log_enable()) { \
      fprintf(log_fp, __VA_ARGS__); \
      fflush(log_fp); \
    } \
  } while (0) \
)

#define _Log(...) \
  do { \
    printf(__VA_ARGS__); \
    log_write(__VA_ARGS__); \
  } while (0)


#endif
