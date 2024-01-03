#include <stdio.h>

#include "dstring.h"

static void print(struct dstring const *s)
{
	printf("cap=%ld, len=%ld, str=\"%s\"\n", s->cap, s->len, dstring_str(s));
}

int main(void)
{
	struct dstring s;					/* Dynamic string */
	dstring_create(&s);					/* Initialization */
	print(&s);

	dstring_setstr(&s, "foo", 4);
	print(&s);
	dstring_setstr(&s, "foobar", 5);
	print(&s);
	dstring_concat(&s, "foobar", 2);
	print(&s);
	dstring_concatf(&s, "+%s %d", "foo", 2);
	print(&s);
	dstring_setcap(&s, s.len + 1);
	print(&s);

	dstring_destroy(&s);					/* Reset */

	return 0;
}
