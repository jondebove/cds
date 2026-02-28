#include <stdio.h>

#include "heap.h"

static bool less(void const *a, void const *b, void *ctx)
{
	(void)ctx;
	return *(int *)a < *(int *)b;
}

int main(void)
{
	int *e;
	int i;

	struct heap h;						/* Heap */
	heap_create(&h, sizeof(*e), less, NULL);		/* Initialization */

	for (i = 10; i > 0; i--) {
		heap_insert(&h, &i);				/* Insert */
	}

	for (i = 0; i < h.len; i++) {				/* Traversal */
		e = heap_at(&h, i);
		printf("at: [%d] = %d\n", i, *e);
	}

	while (h.len > 0) {					/* Remove */
		e = heap_remove(&h, 0);
		printf("remove: %d\n", *e);
	}

	heap_destroy(&h);					/* Clean up */

	return 0;
}
