#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

typedef struct watchpoint *wp_link;

struct watchpoint {
    int NO;       ///< number of watchpoint, almost only for debug
    wp_link next; ///< pointer to next element
    char expr[128];

    /* TODO: Add more members if necessary */
};

void new_wp(char *str);
void free_wp_by_idx(int idx);
void print_wps();

word_t expr(char *e, bool *success);

#endif
