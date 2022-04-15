#include "sdb.h"
#include <stdio.h>

#define NR_WP 32 // number range of watchpoint

static struct watchpoint wp_pool[NR_WP] = {};
static wp_link head = NULL;  // watchpoints in use
static wp_link free_ = NULL; // idle watchpoints

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
void new_wp() {
    wp_link p = free_;
    free_ = free_->next;
    p->next = head;
    head = p;
}

///
///@brief Free a watchpoint from head to free_
///
///@param idx The index of the watchpoint in head
///
void free_wp_by_idx(int idx){
    wp_link p = head;
    for (int i = 0; i < idx - 1; i++) {
        p = p->next;
    }
    // delete p->next from watchpoints in use and push to free_
    wp_link rest = p->next->next;
    p->next->next = free_;
    free_ = p->next;
    p->next = rest;
}

void print_wps() {
    for (wp_link p = head; p; p = p->next) {
        printf("%d\n", p->NO);
    }
}