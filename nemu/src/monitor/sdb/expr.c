#include "common.h"
#include "debug.h"
#include "macro.h"
#include "utils.h"

#include <isa.h>
#include <memory/vaddr.h>
#include <regex.h> // We use the POSIX regex functions to process regular expressions. Type 'man 7 regex' for more information about POSIX regex functions.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  TK_NOTYPE = 256,
  TK_DEC,          // decimal non-negative int
  TK_NEG,          // - for negative sign
  TK_HEX,          // hex non-negative int
  TK_EQ,           // ==
  TK_NEQ,          // !=
  TK_AND,          // &&
  TK_DEREF,        // * for dereference
  TK_REG,          // general purpose register name
  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {"[[:blank:]]+", TK_NOTYPE},    // spaces
    {"0[xX][[:xdigit:]]+", TK_HEX}, // hex non-negative int, this must put in front of TK_DEC, otherwise 0x123 split into 0 (TK_DEC), x123 (no match)
    {"[[:digit:]]+", TK_DEC},       // decimal non-negative int
    {"\\(", '('},                   // (
    {"\\)", ')'},                   // )
    {"==", TK_EQ},                  // equal
    {"!=", TK_NEQ},                 // not equal
    {"&&", TK_AND},                 // logic and
    {"\\+", '+'},                   // plus
    {"-", '-'},                     // minus
    {"\\*", '*'},                   // plus
    {"/", '/'},                     // divide
    {"\\$[[:alnum:]]+", TK_REG},     // general purpose register name
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] = {};
static int nr_token = 0; // amount of tokens detected

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        #ifdef DEBUG_expr
        // print all rules matched
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
        #endif // DEBUG_expr

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        int offset = 0;
        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case TK_HEX:
            offset++;  // strip prefix 0x/0X
          case TK_REG:
            offset++;  // strip prefix $
          case TK_DEC:
            strncpy(tokens[nr_token].str, substr_start + offset, substr_len - offset);
            tokens[nr_token].str[substr_len - offset] = '\0'; // terminate the string manually since strncpy may not null-terminate it for us
          default:
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
        }

        break; // once matched, break for loop
      }
    }

    if (i == NR_REGEX) {
      ERROR("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  #ifdef DEBUG_expr
  // print all tokens
  for (i = 0; i < ARRLEN(tokens); i++) {
    if (tokens[i].type < 256) {
      printf("│ %d: %c, %s\t", i, tokens[i].type, tokens[i].str);
    } else {
      printf("│ %d: %d, %s\t", i, tokens[i].type, tokens[i].str);
    }
  }
  printf("\n");
  #endif // DEBUG_expr

  return true;
}

bool check_parentheses(int p, int q, bool *success) {
  // Detect if expression between p and q is surrounded by a matched pair of brackets
  if (tokens[p].type != '(' || tokens[q].type != ')') {
    return false;
  } else {
    int pair_acc = 0;
    for (int i = p; i <= q; i++) {
      switch (tokens[i].type) {
        case '(': pair_acc += 1; break;
        case ')': pair_acc -= 1; break;
        default: break;
      }
      if (pair_acc < 0) {
        // this must separate from the below one, to recognize expressions like (1)) + ((1)
        ERROR("Missing brackets\n");
        *success = false;
        return false;
      } else if (pair_acc == 0 && i != q) {
        // Brackets at p and q does not match
        *success = true;
        return false;
      }
    }
    if (pair_acc == 0) {
      *success = true;
      return true;
    } else {
      // Missing bracket
      ERROR("Missing brackets\n");
      *success = false;
      return false;
    }
  }
}

word_t eval(int p, int q, bool *success) {
  if (p > q) {
    ERROR("Missing number\n");
    *success = false;
    return 0;
  } else if (p == q) {
    // Case: single token, return the value.
    // For now this token should be a number, return the value of the number.
    // This token should be: TK_DEC/TK_HEX/TK_REG
    switch (tokens[p].type) {
      case TK_DEC: return strtol(tokens[p].str, NULL, 10);
      case TK_HEX: return strtol(tokens[p].str, NULL, 16);
      case TK_REG: return isa_reg_str2val(tokens[p].str, success);
      default:
        ERROR("Missing operand\n");
        *success = false;
        return 0;
    }
  } else if (check_parentheses(p, q, success) == true) {
    // Case: the expression is surrounded by a matched pair of parentheses.
    // If that is the case, just throw away the parentheses.
    return eval(p + 1, q - 1, success);
  } else {
    if (*success != true) {
      return 0;
    }
    int op = -1;          // Position of the main operator in the token expression
    uint precedence = -1; // Actually max uint, precedence of op
    int current_precedence;
    int pair_acc = 0;
    // Find the main operator. main operator is:
    // - Calculation operator
    // - Not inside brackets
    // - With lowest precedence in the expression
    // - When there are multiple operators with same precedence, the right most one is what we want
    for (int i = q; i >= p; i--) {
      current_precedence = 0;
      switch (tokens[i].type) {
        case '(': pair_acc += 1; break;
        case ')': pair_acc -= 1; break;
        case '*':
        case '/':
          current_precedence++;
        case '+':
        case '-':
          if (pair_acc == 0 && precedence > current_precedence) {
            op = i;
            precedence = current_precedence;
          }
      }
      if (pair_acc > 0) {
        // this must separate from the below one, to recognize expressions like (1)) + ((1)
        ERROR("Missing brackets\n");
        *success = false;
        return false;
      } else if (op != -1 && precedence == 0) {
        // just an acceleration
        break;
      }
    }

    if (op == -1) {
      if (pair_acc != 0) {
        ERROR("Missing brackets\n");
      } else {
        // process unary operators: negative sign, dereference
        if (tokens[p].type == TK_NEG) { return -eval(p + 1, q, success); }
        if (tokens[p].type == TK_DEREF) {
          return vaddr_read(strtol(tokens[q].str, NULL, 10), sizeof(word_t));
        }
        ERROR("Missing operator\n");
      }
      *success = false;
      return 0;
    }

    #ifdef DEBUG_expr
    // print the expression stack
    printf("\n┌───────────────┐\n");
    for (int i = p; i <= op - 1; i++) {
      switch (tokens[i].type) {
        case TK_DEC: printf("%s", tokens[i].str); break;
        case TK_NEG: printf("-"); break;
        case TK_HEX: printf("0x%s", tokens[i].str); break;
        case TK_EQ: printf("=="); break;
        case TK_NEQ: printf("!="); break;
        case TK_AND: printf("&&"); break;
        case TK_DEREF: printf("*"); break;
        case TK_REG: printf("$%s", tokens[i].str); break;
        default: printf("%c", tokens[i].type); break;
      }
    }
    #endif // DEBUG_expr

    uint32_t val1 = eval(p, op - 1, success);
    if (*success != true) { return 0; } // End if error in subexpressions

    #ifdef DEBUG_expr
    printf("\t%c\t", tokens[op].type);
    for (int i = op + 1; i <= q; i++) {
      switch (tokens[i].type) {
        case TK_DEC: printf("%s", tokens[i].str); break;
        case TK_NEG: printf("-"); break;
        case TK_HEX: printf("0x%s", tokens[i].str); break;
        case TK_EQ: printf("=="); break;
        case TK_NEQ: printf("!="); break;
        case TK_AND: printf("&&"); break;
        case TK_DEREF: printf("*"); break;
        case TK_REG: printf("$%s", tokens[i].str); break;
        default: printf("%c", tokens[i].type); break;
      }
    }
    printf("\n");
    #endif // DEBUG_expr

    uint32_t val2 = eval(op + 1, q, success);
    if (*success != true) { return 0; } // End if error in subexpressions

    #ifdef DEBUG_expr
    printf("└───────────────┘\n");
    #endif // DEBUG_expr

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/':
        if (val2 == 0) {
          ERROR("Division by zero\n");
          *success = false;
          return 0;
        }
        return val1 / val2;
      default:
        ERROR("Something wrong\n"); // This should be actually not reachable
        *success = false;
        return 0;
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // detect negative sign
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '-' && (i == 0 ||
                                  tokens[i - 1].type == '(' ||
                                  tokens[i - 1].type == '+' ||
                                  tokens[i - 1].type == '-' ||
                                  tokens[i - 1].type == '*' ||
                                  tokens[i - 1].type == '/')) {
      tokens[i].type = TK_NEG;
    }
  }
  // detect dereference operator
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*' && (i == 0 ||
                                  tokens[i - 1].type == '(' ||
                                  tokens[i - 1].type == '+' ||
                                  tokens[i - 1].type == '-' ||
                                  tokens[i - 1].type == '*' ||
                                  tokens[i - 1].type == '/')) {
      tokens[i].type = TK_DEREF;
    }
  }
  *success = true;
  return eval(0, nr_token - 1, success);
}
