#ifndef __TRAP_H__
#define __TRAP_H__

#include <am.h>
#include <klib.h>
#include <klib-macros.h>

__attribute__((noinline))
void check(bool cond) {
  if (!cond) halt(1);
}

__attribute__((noinline))
int add(int a, int b) {
	int c = a + b;
	return c;
}

#endif
