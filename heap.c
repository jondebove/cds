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

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "heap.h"

static bool less(struct heap *h, long i, long j)
{
	assert(h);
	assert(j >= 0 && j < h->len);

	if (i == j) {
		return false;
	}
	return h->less(&h->data[i * h->inc], &h->data[j * h->inc], h->ctx);
}

static void swap(struct heap *h, long i, long j)
{
	assert(h);
	assert(i >= 0 && i < h->len);
	assert(j >= 0 && j < h->len);

	if (i != j) {
		void *const a = &h->data[i * h->inc];
		void *const b = &h->data[j * h->inc];
		void *const t = &h->data[h->cap * h->inc];

		memcpy(t, a, h->inc);
		memcpy(a, b, h->inc);
		memcpy(b, t, h->inc);
	}
}

static void up(struct heap *h, long j)
{
	assert(h);
	assert(j >= 0 && j < h->len);

	for (;;) {
		long i = (j - 1) / 2; // parent
		if (i == j || !less(h, j, i)) {
			break;
		}
		swap(h, i, j);
		j = i;
	}
}

static bool down(struct heap *h, long i0, long n)
{
	assert(h);
	assert(i0 >= 0 && i0 <= n);
	assert(n >= 0 && n <= h->len);

	long i = i0;
	for (;;) {
		long j = 2 * i + 1; // left child
		if (j >= n || j < 0) { // j < 0 after overflow
			break;
		}
		long k = j + 1; // right child
		if (k < n && less(h, k, j)) {
			j = k;
		}
		if (!less(h, j, i)) {
			break;
		}
		swap(h, i, j);
		i = j;
	}
	return i > i0;
}

#if 0
void heapify(struct heap *h)
{
	long i;
	for (i = h->len / 2 - 1; i >= 0; i--) {
		down(h, i, h->len);
	}
}
#endif

struct heap *heap_create(struct heap *h, long inc,
		bool (*less)(void const *a, void const *b, void *ctx),
		void *ctx)
{
	if (h && inc > 0) {
		h->data = NULL;
		h->len = 0;
		h->cap = 0;
		h->inc = inc;
		h->less = less;
		h->ctx = ctx;
	}
	return NULL;
}

void heap_destroy(struct heap *h)
{
	free(h->data);
	memset(h, 0, sizeof(*h));
}

int heap_insert(struct heap *h, void const *x)
{
	if (h->len == h->cap) {
		long grow = h->cap > 64 ? h->cap : 64;
		if (grow > LONG_MAX / h->inc - h->cap) {
			return -ERANGE;
		}
		char *data = realloc(h->data, (h->cap + grow + 1) * h->inc);
		if (!data) {
			return -ENOMEM;
		}
		h->data = data;
		h->cap += grow;
	}

	memcpy(h->data + h->len * h->inc, x, h->inc);
	h->len++;
	up(h, h->len - 1);
	return 0;
}

void *heap_remove(struct heap *h, long i)
{
	if (i < 0 || i >= h->len) {
		return NULL;
	}

	long n = h->len - 1;
	if (n != i) {
		swap(h, i, n);
		if (!down(h, i, n)) {
			up(h, i);
		}
	}
	h->len = n;
	return h->data + n * h->inc;
}

void heap_update(struct heap *h, long i)
{
	if (!down(h, i, h->len)) {
		up(h, i);
	}
}
