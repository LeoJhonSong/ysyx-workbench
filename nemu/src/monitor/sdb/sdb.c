#include "sdb.h"

#include "common.h"
#include "utils.h"

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/ifetch.h>
#include <isa.h>
#include <memory/vaddr.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  // Meaning of \001 and \002, see: https://stackoverflow.com/a/55773513/10088906
  line_read = readline("\001" ASNI_FG_NORMAL_PINK "\002\001" ASNI_NONE ASNI_BG_PINK "\002nemu\001" ASNI_NONE ASNI_FG_NORMAL_PINK "\002\001" ASNI_NONE "\002 ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  // set NEMU state to QUIT, otherwise will return bad exit status by is_exit_status_bad()
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_s(char *args) {
  // execute one line by default
  if (args == NULL) {
    cpu_exec(1);
  } else {
    cpu_exec(strtol(args, NULL, 10));
  }
  return 0;
}

static int cmd_i(char *args) {
  if (args == NULL) {
    printf("Missing subcommand\n");
  } else if (strcmp(args, "r") == 0) {
    isa_reg_display();
  } else if (strcmp(args, "w") == 0) {
    ;
  } else {
    printf("Unknown subcommand '%s'\n", args);
  }
  return 0;
}

static int cmd_x(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");

  if (arg == NULL) {
    ERROR("Missing N: how many words to scan\n");
  } else {
    // TODO: only accept hex number for now
    bool success;
    word_t expression = expr(args, &success);
    printf(" starting from 0x" ASNI_FMT("%lx", ASNI_FG_WHITE) "\n", expression);
    printf(ASNI_FMT(MUXDEF(CONFIG_ISA64, "         63        32           0\n                              \n", "         31         0\n                   \n"), ASNI_DIM));
    for (int i = 0; i < strtol(arg, NULL, 10); i++) {
      printf(ASNI_FG_WHITE "%8lx" ASNI_NONE ":", i * sizeof(word_t));
      // print a word of memory by bytes, high order bytes first
      for (int j = sizeof(word_t) - 1; j >= 0; j--) {
        word_t value = vaddr_read(expression + i * 4 + j, 1);
        printf(" %s%02lx%s", value == 0 ? ASNI_DIM : ASNI_FG_NORMAL_GREEN, value, ASNI_NONE);
      }
      printf("\n");
    }
  }
  return 0;
}

static int cmd_p(char *args) {
  bool success;
  word_t value = expr(args, &success);
  if (success) {
    printf("result: " ASNI_FMT("%lu | %016lx\n", ASNI_FG_NORMAL_GREEN), value, value);
  }
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
  {"help", "Display informations about all supported commands", cmd_help},
  {"c", "Continue the execution of the program", cmd_c},
  {"q", "Exit NEMU", cmd_q},
  /* TODO: Add more commands */
  {"s", "Single step N instructions", cmd_s},
  {"i", "Display register/watchpoint infomation", cmd_i},
  {"x", "print N 4-bytes in memory start from EXPR", cmd_x},
  {"p", "evaluation value of given expression", cmd_p},
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    ERROR("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) {
      ERROR("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
