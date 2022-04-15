#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

typedef struct watchpoint {
    int NO;                  ///< number of watchpoint
    struct watchpoint *next; ///< pointer to next element

    /* TODO: Add more members if necessary */

} WP;

word_t expr(char *e, bool *success);

WP *new_wp();
void free_wp(WP *wp);

#endif
