/* Copyright (c) 2023, Jonathan Debove
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

#ifndef CDS_FNV1A_H
#define CDS_FNV1A_H

/*!
 * \file fnv1a.h
 * \author Jonathan Debove
 * \brief FNV1a hash functions.
 *
 * http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 */

#include <limits.h>
#include <stddef.h>

#if ULONG_MAX == 0xffffffff
#	define FNV1A_SEED 0x811c9dc5
#	define FNV1A_MULT 0x01000193
#elif ULONG_MAX == 0xffffffffffffffff
#	define FNV1A_SEED 0xCBF29CE484222325
#	define FNV1A_MULT 0x00000100000001b3
#else
#	error ULONG_WIDTH different from 32 ou 64 not implemented.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * fnv1a_mem computes the hash code of a memory area of len bytes
 * pointed to by s using the FNV1a algorithm.
 * seed enables to chain FNV1a computations or to introduce a
 * user-defined seed (FNV1A_SEED macro is the official value).
 */
static inline
unsigned long fnv1a_mem(void const *s, size_t len, unsigned long seed)
{
	unsigned char const *b = (unsigned char const *)s;
	while (len--) {
		int c = *b++;
		seed = FNV1A_MULT * (seed ^ c);
	}
	return seed;
}

/*!
 * fnv1a_str computes the FNV1a hash code of a string s.
 * seed enables to chain FNV1a computations or to introduce a
 * user-defined seed (FNV1A_SEED macro is the official value).
 */
static inline
unsigned long fnv1a_str(char const *s, unsigned long seed)
{
	int c;
	while ((c = *s++)) {
		seed = FNV1A_MULT * (seed ^ c);
	}
	return seed;
}

#ifdef __cplusplus
}
#endif

#endif	/* CDS_FNV1A_H */
