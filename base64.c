/* Copyright (c) 2024,2025, Jonathan Debove
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
static uint8_t decoding[257] = { 0 };

#define PADDING '='
#define INVALID 0xff

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
		*dst++ = PADDING;
		*dst++ = PADDING;
		return dst;
	}

	val |= (uint32_t)(*s++) << 8;
	assert(s == end);

	*dst++ = encoding[val >> 18 & 0x3f];
	*dst++ = encoding[val >> 12 & 0x3f];
	*dst++ = encoding[val >>  6 & 0x3f];
	*dst++ = PADDING;
	return dst;
}

#define DECODE_CHAR(s, v, n) do {		\
	uint8_t c = decoding[(int)(*(s)++)];	\
	if (c == INVALID) return NULL;		\
	if ((n) == 18) (v) = (uint32_t)c << 18;	\
	else (v) |= (uint32_t)c << n;		\
} while (0)

void *base64_decode(void *dst, char const *src, size_t n)
{
	assert(dst);
	assert(src);

	if (decoding[256] != INVALID) {
		memset(decoding, INVALID, sizeof(decoding));
		for (int i = 0; i < (int)sizeof(encoding) - 1; i++) {
			decoding[(int)encoding[i]] = i;
		}
	}

	uint8_t *d = dst;
	if (n == 0) {
		return d;
	}
	if (n % 4 != 0) {
		return NULL;
	}

	char const *end = src + ((n - 4) / 4) * 4;
	uint32_t val = 0;
	while (src != end) {
		DECODE_CHAR(src, val, 18);
		DECODE_CHAR(src, val, 12);
		DECODE_CHAR(src, val,  6);
		DECODE_CHAR(src, val,  0);

		*d++ = val >> 16;
		*d++ = val >> 8;
		*d++ = val >> 0;
	}

	DECODE_CHAR(src, val, 18);
	DECODE_CHAR(src, val, 12);
	*d++ = val >> 16;
	if (*src == PADDING) {
		return d;
	}

	DECODE_CHAR(src, val,  6);
	*d++ = val >> 8;
	if (*src == PADDING) {
		return d;
	}

	DECODE_CHAR(src, val,  0);
	*d++ = val >> 0;
	return d;
}
