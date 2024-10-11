#include <stdio.h>

#include "darray.h"

int main(void)
{
	int *e;
	int i;

	struct darray a;					/* Dynamic array */
	darray_create(&a, sizeof(*e));				/* Initialization */

	for (i = 0; i < 10; i++) {
		e = darray_push(&a, 1);				/* Push */
		*e = i;
	}

	e = darray_pop(&a, 1);					/* Pop */
	printf("pop: %d\n", *e);

	e = darray_splice(&a, 3, 2, 1);				/* Splice */
	*e = 42;

	for (i = 0; i < a.len; i++) {				/* Traversal */
		e = darray_at(&a, i);
		printf("at: [%d] = %d\n", i, *e);
	}

	darray_destroy(&a);					/* Reset */

	return 0;
}
