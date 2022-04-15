#include "debug.h"
#include "sdb.h"
#include "utils.h"

#include <stdio.h>

#define NR_WP 32 // number range of watchpoint

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
wp_link new_wp() {
    wp_link p = free_;
    free_ = free_->next;
    p->next = head;
    head = p;
    wps_in_use++;
    return head;
}

///
///@brief Pop a watchpoint from head and push to free_
///
///@param idx The index of the watchpoint in use to pop
///
void free_wp_by_idx(int idx) {
    if (idx < 0 || idx >= NR_WP) {
        ERROR("index %d out of range of 0<= idx <= %d", idx, wps_in_use - 1);
    } else {
        wp_link p = head;
        if (idx == 0) {
            head = head->next;
            p->next = free_;
            free_ = p;
        } else {
            for (int i = 0; i < idx - 1; i++) {
                p = p->next;
            }
            // delete p->next from watchpoints in use and push to free_
            wp_link rest = p->next->next;
            p->next->next = free_;
            free_ = p->next;
            p->next = rest;
        }
    }
}

///
///@brief Traverse from the head and print info of each watchpoint
///
void print_wps() {
    int i = 0;
    for (wp_link p = head; p; p = p->next) {
        printf("%d: [%d]\n", i, p->NO);
        i++;
    }
}