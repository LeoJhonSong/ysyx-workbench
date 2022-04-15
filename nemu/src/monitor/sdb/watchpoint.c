#include "common.h"
#include "debug.h"
#include "sdb.h"
#include "utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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
///@param expr The string of expression the new watchpoint should watch
///
void new_wp(char *expr_str) {
    if (wps_in_use == NR_WP) {
        ERROR("All %d watchpoints are in use, no more idle watchpoints\n", NR_WP);
    } else {
        wp_link p = free_;
        bool success;
        word_t value = expr(expr_str, &success);

        if (success == true) {
            free_ = free_->next;
            p->next = head;
            strcpy(p->expr, expr_str);
            p->value = value;
            head = p;
            wps_in_use++;
        }
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
        printf(ANSI_FMT("%2d", ANSI_FG_NORMAL_CYAN) " │ " ANSI_FMT("%s", ANSI_FG_NORMAL_BLUE) " = " ANSI_FMT("%lu", ANSI_FG_NORMAL_GREEN) "\n", i, p->expr, p->value);
        i++;
    }
}

///
///@brief Check if value of any watchpoint changes
///
///@return true values of some watchpoints change
///@return false values of all watchpoint keep the same
///
bool wps_check() {
    bool is_changed = false;
    bool success;
    int i = 0;

    for (wp_link p = head; p; p = p->next) {
        word_t value = expr(p->expr, &success);
        if (p->value != value) {
            is_changed = true;
            printf("value of watchpoint " ANSI_FMT("%d", ANSI_FG_NORMAL_CYAN) " with expression " ANSI_FMT("%s", ANSI_FG_NORMAL_BLUE) " changes from " ANSI_FMT("%lu", ANSI_FG_NORMAL_GREEN) " to " ANSI_FMT("%lu\n", ANSI_FG_NORMAL_GREEN), i, p->expr, p->value, value);
            p->value = value;
        }
        i++;
    }

    return is_changed;
}