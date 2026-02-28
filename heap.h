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

/*! Generic dynamic heap.
 * Do not modify its fields unless you know what you are doing.
 */
struct heap {
	void *tmp;
	char *data;
	long len;
	long cap;
	long inc;

	bool (*less)(void const *a, void const *b, void *ctx);
	void *ctx;
};

/*! heap_create initializes a heap `h` containing elements of size `inc`.
 * The heap will be ordered according to the comparison function `less`.
 * It cannot fail and does not allocate memory.
 */
struct heap *heap_create(struct heap *h, long inc,
		bool (*less)(void const *a, void const *b, void *ctx),
		void *ctx);

/*! heap_destroy frees the memory space internal to the heap.
 * It does not free the memory allocated by the user for the heap nor
 * the entries.
 */
void heap_destroy(struct heap *h);

/*! heap_at returns a pointer to the `i`th element.
 * Returns `NULL` if index is out of bounds.
 */
static inline void *heap_at(struct heap const *h, long i)
{
	return i >= 0 && i < h->len ? h->data + i * h->inc : NULL;
}

/*! heap_len returns the number of elements in the heap `h`. */
static inline long heap_len(struct heap const *h)
{
	return h->len;
}

/*! heap_insert inserts an element in the heap.
 * It returns 0 on success and `-ENOMEM` if out of memory.
 */
int heap_insert(struct heap *h, void const *x);

/*! heap_remove removes the `i`-th element from the heap.
 * It returns a pointer to the value or `NULL` if `i` is out of bounds.
 */
void *heap_remove(struct heap *h, long i);

/*! heap_update fixes the heap in case the element `i` has been updated. */
void heap_update(struct heap *h, long i);

/*! HEAP_FOREACH iterates over all the elements of the heap. */
#define HEAP_FOREACH(elem, heap)					\
	for (long h__idx = 0;						\
			((elem) = heap_at((heap), h__idx));		\
			h__idx++)

/*! heap_sort sorts an array with `n` elements of size `size`.
 * The `base` argument points to the start of the array.
 * The contents of the array are sorted in ascending order according to the
 * comparison function pointed to by `cmp`.
 */
void heap_sort(void *base, size_t n, size_t size,
		int (*cmp)(void const *a, void const *b, void *ctx), void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CDS_HEAP_H */
