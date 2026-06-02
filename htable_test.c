#include <stdio.h>

#include "htable.h"

static unsigned long hash(void const *key, void *ctx)
{
	(void)ctx; /* unused */
	return (unsigned long)(*(int const *)key);
}

static int comp(void const *key, void const *entry, void *ctx)
{
	(void)ctx; /* unused */
	int const e = *(int const *)entry;
	int const k = *(int const *)key;
	return e * e != k;
}

static void print(void const *item, void *context)
{
	char *s = context;
	int i = *(int *)item;
	printf("%s: key=%d, val=%d\n", s, i * i, i);
}

int main(void)
{
	int *e;
	int i, k, ret;

	struct htable ht;					/* Hash table */
	htable_create(&ht, sizeof(*e), hash, comp, NULL);	/* Initialization */

	for (i = 0; i < 10; i++) {
		k = i * i;
		e = htable_enter(&ht, &k, &i, &ret);		/* Insertion */
		print(e, "enter");
	}

	k = 9;
	e = htable_find(&ht, &k);				/* Search */
	print(e, "find");

	e = htable_delete(&ht, &k);				/* Deletion */
	print(e, "delete");

	htable_walk(&ht, print, "walk");			/* Traversal */
	/*
	HTABLE_FOREACH(e, &ht)
		print(e, "foreach");
	*/

	htable_destroy(&ht);					/* Reset */

	return 0;
}
