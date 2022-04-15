#include "sdb.h"

#define NR_WP 32 // number range of watchpoint

static WP wp_pool[NR_WP] = {};
static WP *head = NULL;  // watchpoints in use
static WP *free_ = NULL; // idle watchpoints

/// @brief initialize head and free_
void init_wp_pool() {
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    }

    head = NULL;
    free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

///
///@brief Get a new watchpoint from free_. When no more idle watchpoint in free_, Assert(0) triggered
///
///@return WP* The first watchpoint from free_
///
WP *new_wp() {
    WP *p = free_;
    free_ = free_->next;
    return p;
}

///
///@brief Free a watchpoint from head to free_
///
///@param wp The watchpoint to free
///
void free_wp(WP *wp);
