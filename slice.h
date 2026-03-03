/* Copyright (c) 2026, Jonathan Debove
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

#ifndef CDS_SLICE_H
#define CDS_SLICE_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Reference counted slice.
 */
struct slice_mem;

struct slice {
	union {
		char *base;
		long size;
	};
	long len;
	struct slice_mem *mem;
};

struct slice *slice_create(struct slice *s, long len, long cap, long size);

void slice_destroy(struct slice *s);

long slice_len(struct slice const *s);

long slice_cap(struct slice const *s);

long slice_size(struct slice const *s);

void *slice_at(struct slice const *s, long i);

void slice_slice(struct slice *dst, struct slice const *src, long idx, long len);

long slice_copy(struct slice dst, struct slice src);

int slice_clone(struct slice *dst, struct slice const *src);

int slice_concat(struct slice *dst, struct slice const *src);

void *slice_append(struct slice *s, long n);

void *slice_swap(struct slice *s, long i, long j);

#ifdef __cplusplus
}
#endif

#endif /* CDS_SLICE_H */
