#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

typedef struct watchpoint *wp_link; // type: pointer to node of single linked list of watchpoint

struct watchpoint {
    int NO;         ///< number of watchpoint, almost only for debug
    wp_link next;   ///< pointer to next element
    char expr[128]; ///< string of expression to watch
    word_t value;   /// < value of the expression
};

void new_wp(char *str);
void free_wp_by_idx(int idx);
void print_wps();
bool wps_check();

word_t expr(char *e, bool *success);

#endif
