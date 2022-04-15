#include "debug.h"
#include "sdb.h"
#include "utils.h"

#include <stdio.h>

#define NR_WP 12 // number range of watchpoint

static struct watchpoint wp_pool[NR_WP] = {};
static wp_link head = NULL;  // watchpoints in use
static wp_link free_ = NULL; // idle watchpoints
static int wps_in_use = 0;   // amount of watchpoint in use

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
///@brief Pop a watchpoint from free_ and push to head. When no more idle watchpoint in free_, Assert(0) triggered
///
///@return WP* The first watchpoint from free_, also the head
///
void new_wp(char *expr) {
    if (wps_in_use == NR_WP) {
        ERROR("All %d watchpoints are in use, no more idle watchpoints\n", NR_WP);
    } else {
        wp_link p = free_;
        free_ = free_->next;
        p->next = head;
        printf("%s\n", expr);
        p->expr = expr;
        head = p;
        wps_in_use++;
    }
}

///
///@brief Pop a watchpoint from head and push to free_
///
///@param idx The index of the watchpoint in use to pop
///
void free_wp_by_idx(int idx) {
    wp_link p = head;
    if (idx < 0 || idx >= wps_in_use) {
        ERROR("%d out of range of 0<= index <= %d\n", idx, wps_in_use - 1);
        return;
    } else if (idx == 0) {
        head = head->next;
        p->next = free_;
        free_ = p;
    } else {
        for (int i = 0; i < idx - 1; i++) { p = p->next; }
        // delete p->next from watchpoints in use and push to free_
        wp_link rest = p->next->next;
        p->next->next = free_;
        free_ = p->next;
        p->next = rest;
    }
    wps_in_use--;
}

///
///@brief Traverse from the head and print info of each watchpoint
///
void print_wps() {
    int i = 0;
    for (wp_link p = head; p; p = p->next) {
        printf("%d: >>>%s<<<\n", i, p->expr);
        i++;
    }
}