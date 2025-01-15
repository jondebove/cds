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

#ifndef BASE64_H
#define BASE64_H

#include <stddef.h>

/**
 * base64_encode encodes n bytes from the object beginning at src.
 * It writes base64_encodedsize(n) bytes to dst and returns a pointer
 * to the byte following the last written byte.
 */
char *base64_encode(char *dst, void const *src, size_t n);

/**
 * base64_decode decodes n bytes from the base64-encoded data beginning
 * at src. It writes at most base64_decodedsize(n) bytes to dst and
 * returns a pointer to the byte following the last written byte.
 * If src contains invalid base64 data, it will return NULL.
 */
void *base64_decode(void *dst, char const *src, size_t n);

/**
 * base64_encodedsize returns the length in bytes of the base64
 * encoding of an input buffer of length n.
 */
static inline size_t base64_encodedsize(size_t n)
{
	return ((n + 2) / 3) * 4;
}

/**
 * base64_decodedsize returns the maximum length in bytes of the
 * decoded data corresponding to n bytes of base64-encoded data.
 */
static inline size_t base64_decodedsize(size_t n)
{
	return (n / 4) * 3;
}

#endif	/* BASE64_H */
