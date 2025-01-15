/* Copyright (c) 2024, Jonathan Debove
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "base64.h"

static char const encoding[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char const padding = '=';
static uint8_t decoding[257] = { 0 };
static uint8_t const invalid = 0xff;

// C11
//static_assert(sizeof(encoding) == 64 + 1);
//static_assert(sizeof(decoding) == 256 + 1);

char *base64_encode(char *dst, void const *src, size_t n)
{
	assert(dst);
	assert(src);

	uint8_t const *s = src;
	uint8_t const *end = (uint8_t const *)src + (n / 3) * 3;
	uint32_t val = 0;
	while (s != end) {
		val  = (uint32_t)(*s++) << 16;
		val |= (uint32_t)(*s++) << 8;
		val |= (uint32_t)(*s++) << 0;

		*dst++ = encoding[val >> 18 & 0x3f];
		*dst++ = encoding[val >> 12 & 0x3f];
		*dst++ = encoding[val >>  6 & 0x3f];
		*dst++ = encoding[val >>  0 & 0x3f];
	}

	end = (uint8_t const *)src + n;
	if (s == end) {
		return dst;
	}

	val = (uint32_t)(*s++) << 16;
	if (s == end) {
		*dst++ = encoding[val >> 18 & 0x3f];
		*dst++ = encoding[val >> 12 & 0x3f];
		*dst++ = padding;
		*dst++ = padding;
		return dst;
	}

	val |= (uint32_t)(*s++) << 8;
	assert(s == end);

	*dst++ = encoding[val >> 18 & 0x3f];
	*dst++ = encoding[val >> 12 & 0x3f];
	*dst++ = encoding[val >>  6 & 0x3f];
	*dst++ = padding;
	return dst;
}

#define DECODE_CHAR(c, s) do {			\
	(c) = decoding[(int)(*(s)++)];		\
	if ((c) == invalid) return NULL;	\
} while (0)

void *base64_decode(void *dst, char const *src, size_t n)
{
	assert(dst);
	assert(src);

	if (decoding[256] != invalid) {
		memset(decoding, invalid, sizeof(decoding));
		for (int i = 0; i < (int)sizeof(encoding); i++) {
			decoding[(int)encoding[i]] = i;
		}
	}

	uint8_t *d = dst;
	if (n < 4) {
		return NULL;
	}

	char const *end = src + ((n - 4) / 4) * 4;
	uint32_t val = 0;
	uint8_t out;
	while (src != end) {
		DECODE_CHAR(out, src);
		val  = (uint32_t)out << 18;
		DECODE_CHAR(out, src);
		val |= (uint32_t)out << 12;
		DECODE_CHAR(out, src);
		val |= (uint32_t)out << 6;
		DECODE_CHAR(out, src);
		val |= (uint32_t)out << 0;

		*d++ = val >> 16;
		*d++ = val >> 8;
		*d++ = val >> 0;
	}

	DECODE_CHAR(out, src);
	val  = (uint32_t)out << 18;
	DECODE_CHAR(out, src);
	val |= (uint32_t)out << 12;
	*d++ = val >> 16;
	if (*src == padding) {
		return d;
	}

	DECODE_CHAR(out, src);
	val |= (uint32_t)out << 6;
	*d++ = val >> 8;
	if (*src == padding) {
		return d;
	}

	DECODE_CHAR(out, src);
	val |= (uint32_t)out << 0;
	*d++ = val >> 0;
	return d;
}

#ifdef TEST
#include <stdio.h>
int main(void)
{
	uint64_t src = 0x0123456789ABCDEF;
	char dst[base64_decodedsize(sizeof(src)) + 1];
	char *p;

	p = base64_encode(dst, &src, sizeof(src));
	*p = '\0';
	printf("0x%lx => %s\n", src, dst);

	p[-4] = '\r';
	src = 0;
	p = base64_decode(&src, dst, p - dst);
	printf("[%d] 0x%lx <= %s\n", !p, src, dst);

	return 0;
}
#endif
