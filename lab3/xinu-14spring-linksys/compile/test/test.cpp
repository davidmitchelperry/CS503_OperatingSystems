#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

struct test {
	int val;
	struct test *next;
};

int funcA(struct test i) {
	struct test next;
	next.val = 7;

	i.next = &next;

	
}

int funcB() {
	struct test i;
	i.val = 5;
	funcA(i);
	printf("i val: %d\n", i.val);
	printf("next val: %d\n", i.next->val);
}

int main() {
	funcB();
	return 0;
}
