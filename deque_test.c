#include <stdio.h>

#include "deque.h"

int main(void)
{
	int *e;
	int i;

	struct deque q;						/* Queue */
	deque_create(&q, sizeof(*e));				/* Initialization */

	for (i = 0; i < 5; i++) {
		e = deque_push(&q);				/* Push */
		*e = i;
		e = deque_unshift(&q);				/* Unshift */
		*e = -i - 1;
	}

	e = deque_pop(&q);					/* Pop */
	printf("pop: %d\n", *e);

	e = deque_shift(&q);					/* Shift */
	printf("shift: %d\n", *e);

	for (i = 0; i < q.len; i++) {				/* Traversal */
		e = deque_at(&q, i);
		printf("at: [%d] = %d\n", i, *e);
	}

	deque_destroy(&q);					/* Reset */

	return 0;
}
