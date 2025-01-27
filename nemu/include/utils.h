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

#define ANSI_FG_BLACK           "\33[1;30m"
#define ANSI_FG_RED             "\33[1;31m"
#define ANSI_FG_GREEN           "\33[1;32m"
#define ANSI_FG_YELLOW          "\33[1;33m"
#define ANSI_FG_BLUE            "\33[1;34m"
#define ANSI_FG_MAGENTA         "\33[1;35m"
#define ANSI_FG_CYAN            "\33[1;36m"
#define ANSI_FG_WHITE           "\33[1;37m"
#define ANSI_FG_PINK            "\33[1;38;5;225m"
#define ANSI_FG_NORMAL_BLACK    "\33[30m"
#define ANSI_FG_NORMAL_RED      "\33[31m"
#define ANSI_FG_NORMAL_GREEN    "\33[32m"
#define ANSI_FG_NORMAL_YELLOW   "\33[33m"
#define ANSI_FG_NORMAL_BLUE     "\33[34m"
#define ANSI_FG_NORMAL_MAGENTA  "\33[35m"
#define ANSI_FG_NORMAL_CYAN     "\33[36m"
#define ANSI_FG_NORMAL_WHITE    "\33[37m"
#define ANSI_FG_NORMAL_PINK     "\33[38;5;183m"
#define ANSI_BG_BLACK           "\33[1;40m"
#define ANSI_BG_RED             "\33[1;41m"
#define ANSI_BG_GREEN           "\33[1;42m"
#define ANSI_BG_YELLOW          "\33[1;43m"
#define ANSI_BG_BLUE            "\33[1;44m"
#define ANSI_BG_MAGENTA         "\33[1;35m"
#define ANSI_BG_CYAN            "\33[1;46m"
#define ANSI_BG_WHITE           "\33[1;47m"
#define ANSI_BG_PINK            "\33[1;48;5;183m"
#define ANSI_DIM                "\33[2m"
#define ANSI_NONE               "\33[0m"

#define ANSI_FMT(str, fmt) fmt str ANSI_NONE

// about __VA_OPT__, __VA_ARGS__, see: https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
#define ERROR(str, ...) printf(ANSI_FMT(str, ANSI_FG_RED) __VA_OPT__(,) __VA_ARGS__)

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
