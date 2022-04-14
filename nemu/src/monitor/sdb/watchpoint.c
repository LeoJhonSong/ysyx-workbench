#include "sdb.h"

#define NR_WP 32 // number range of watchpoint

typedef struct watchpoint {
  int NO;                  // number of watchpoint
  struct watchpoint *next; // pointer to next element

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL;  // watchpoints in use
static WP *free_ = NULL; // idle watchpoints

// This will initialize head and free_
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
/// \return
WP *new_wp();
void free_wp(WP *wp);
