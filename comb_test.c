#include <stdbool.h>
#include <stdio.h>

#include "comb.h"

bool print_and_count(long k, long a[static k], void *ctx)
{
	/* count */
	if (ctx) {
		(*(long *)ctx)++;
	}

	/* print */
	long i;
	for (i = 0; i < k; i++) {
		printf("%ld ", a[i]);
	}
	putchar('\n');

	/* continue */
	return true;
}

int main(void)
{
	long count;
	long a[64];

	puts("combination(4, 2)");
	count = 0;
	comb(4L, 2L, a, print_and_count, &count, 0);
	printf("count = %ld\n", count);

	puts("permutation(4, 2) with repetition");
	count = 0;
	comb(4L, 2L, a, print_and_count, &count,
			COMB_ORDERED | COMB_REPETITION);
	printf("count = %ld\n", count);

	puts("product(3, 2, 3, 4)");
	count = 0;
	prod(3L, a, print_and_count, &count, 2L, 3L, 4L);
	printf("count = %ld\n", count);

	return 0;
}
