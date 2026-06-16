#include <stdio.h>

#include "heap.h"

static int comp(void const *a, void const *b, void *ctx)
{
	int xa = *(int const *)a;
	int xb = *(int const *)b;
	(void)ctx;
	return xa < xb ? -1 : xa > xb;
}

int main(void)
{
	int *e;
	int i;

	struct heap h;						/* Heap */
	heap_create(&h, sizeof(*e), comp, NULL);		/* Initialization */

	for (i = 10; i > 0; i--) {
		heap_insert(&h, &i);				/* Insert */
	}

	for (i = 0; i < heap_len(&h); i++) {			/* Traversal */
		e = heap_at(&h, i);
		printf("at: [%d] = %d\n", i, *e);
	}

	while (heap_len(&h) > 0) {				/* Remove */
		e = heap_remove(&h, 0);
		printf("remove: %d\n", *e);
	}

	heap_destroy(&h);					/* Clean up */

	return 0;
}
