#include <stdio.h>
#include <string.h>

#include "fnv1a.h"

struct data {
	char const *s;
	unsigned long hs;
	unsigned long hm;
};

int main(void)
{
#if ULONG_MAX == 0xffffffffUL
	struct data d[3] = {
		{       "", 0x811c9dc5UL, 0x050c5d1fUL },
		{      "a", 0xe40c292cUL, 0x2b24d044UL },
		{ "foobar", 0xbf9cf968UL, 0x0c1c9eb8UL },
	};
	char const *fmt = "%6s %08lx %08lx %s\n";
#elif ULONG_MAX == 0xffffffffffffffffUL
	struct data d[3] = {
		{       "", 0xcbf29ce484222325UL, 0xaf63bd4c8601b7dfUL },
		{      "a", 0xaf63dc4c8601ec8cUL, 0x089be207b544f1e4UL },
		{ "foobar", 0x85944171f73967e8UL, 0x34531ca7168b8f38UL },
	};
	char const *fmt = "%6s %16lx %16lx %s\n";
#else
#	error ULONG_WIDTH different from 32 ou 64 not implemented.
#endif

	for (int i = 0; i < 3; i++) {
		size_t len = strlen(d[i].s);
		unsigned long hs = fnv1a_str(d[i].s, FNV1A_SEED);
		unsigned long hm = fnv1a_mem(d[i].s, len + 1, FNV1A_SEED);
		printf(fmt, d[i].s, hs, hm,
				hs == d[i].hs && hm == d[i].hm ? "OK" : "KO");
	}

	return 0;
}
