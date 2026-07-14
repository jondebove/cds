
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "base64.h"

static
void test_encode(void const *data, size_t size, char const *want)
{
	char got[64];
	assert(sizeof(got) >= BASE64_ENCODEDSIZE(size));

	size = base64_encode(got, data, size) - got;
	assert(size == strlen(want));
	assert(memcmp(got, want, size) == 0);
}

static
void test_decode(char const *data, size_t size, void const *want, size_t len)
{
	char got[64];
	assert(sizeof(got) >= BASE64_DECODEDSIZE(size));

	char *end = base64_decode(got, data, size);
	assert(end); /* test if error */
	assert((size_t)(end - got) == len);
	assert(memcmp(got, want, len) == 0);
}

int main(void)
{
	/* Test vectors from RFC 4648 */
	char const dec[] = "foobar";
	char const *enc[] = {
		"",
		"Zg==",
		"Zm8=",
		"Zm9v",
		"Zm9vYg==",
		"Zm9vYmE=",
		"Zm9vYmFy",
	};

	for (size_t i = 0; i < sizeof(dec); i++) {
		test_encode(dec, i, enc[i]);
		test_decode(enc[i], strlen(enc[i]), dec, i);
	}

	return 0;
}
