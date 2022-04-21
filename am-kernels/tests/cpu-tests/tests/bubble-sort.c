#include "trap.h"

#define N 3

// int a[N] = {2, 12, 14, 6, 13, 15, 16, 10, 0, 18, 11, 19, 9, 1, 7, 5, 4, 3, 8, 17};
int a[N] = {2, 1, 0};

__attribute__((noinline))
void bubble_sort() {
	int i, j, t;
	for(j = 0; j < N; j ++) {
		for(i = 0; i < N - 1 - j; i ++) {
			if(a[i] > a[i + 1]) {
				t = a[i];
				a[i] = a[i + 1];
				a[i + 1] = t;
			}
		}
	}
}

int main() {
	bubble_sort();

	int i;
	for(i = 0; i < N; i ++) {
		check(a[i] == i);
	}

	// check(i == N);

	// bubble_sort();

	// for(i = 0; i < N; i ++) {
	// 	check(a[i] == i);
	// }

	// check(i == N);

	return 0;
}
