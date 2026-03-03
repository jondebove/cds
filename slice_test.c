#include <stdio.h>

#include "slice.h"

int main(void)
{
	int i;
	int *e;

	struct slice s1;
	slice_create(&s1, 10, 10, sizeof(*e));

	for (i = 0; i < slice_len(&s1); i++) {
		e = slice_at(&s1, i);
		*e = i;
	}

	for (i = 0; i < slice_len(&s1); i++) {
		e = slice_at(&s1, i);
		printf("%d\n", *e);
	}

	struct slice s2;
	slice_create(&s2, 0, 0, sizeof(*e));

	slice_slice(&s2, &s1, 3, 3);
	for (i = 0; i < slice_len(&s2); i++) {
		e = slice_at(&s2, i);
		printf("%d\n", *e);
	}

	slice_concat(&s2, &s1);
	for (i = 0; i < slice_len(&s2); i++) {
		e = slice_at(&s2, i);
		printf("%d\n", *e);
	}

	slice_destroy(&s2);
	slice_destroy(&s1);
}
