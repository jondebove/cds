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

#ifndef CDS_HEAP_H
#define CDS_HEAP_H

/*!
 * \file heap.h
 * \author Jonathan Debove
 * \brief Generic heap.
 */

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct heap {
	void *tmp;
	char *data;
	long len;
	long cap;
	long inc;

	bool (*less)(void const *a, void const *b, void *ctx);
	void *ctx;
};

struct heap *heap_create(struct heap *h, long inc,
		bool (*less)(void const *a, void const *b, void *ctx),
		void *ctx);

void heap_destroy(struct heap *h);

static inline
void *heap_at(struct heap const *h, long i)
{
	return i >= 0 && i < h->len ? h->data + i * h->inc : NULL;
}

int heap_insert(struct heap *h, void const *x);

void *heap_remove(struct heap *h, long i);

void heap_update(struct heap *h, long i);

#define HEAP_FOREACH(elem, heap)					\
	for (long h__idx = 0;						\
			((elem) = heap_at((heap), h__idx));		\
			h__idx++)

void heap_sort(void *base, size_t n, size_t size,
		int (*cmp)(void const *a, void const *b, void *ctx), void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CDS_HEAP_H */
