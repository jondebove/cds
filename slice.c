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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "slice.h"

/*
 * Slice memory.
 */
struct slice_mem {
	int count;
	long cap;
	long size;
	char data[];
};

static
struct slice_mem *slice_mem_new(long cap, long size)
{
	if (cap <= 0 || size <= 0) {
		return NULL;
	}

	struct slice_mem *m = malloc(sizeof(*m) + cap * size);
	if (m) {
		m->count = 1;
		m->cap = cap;
		m->size = size;
	}
	return m;
}

static
void slice_mem_incr(struct slice_mem *m)
{
	if (m) {
		m->count++;
	}
}

static
void slice_mem_decr(struct slice_mem *m)
{
	if (m) {
		if (m->count == 1) {
			free(m);
		} else {
			m->count--;
		}
	}
}

/*
 * Slice.
 */
struct slice *slice_create(struct slice *s, long len, long cap, long size)
{
	if (len < 0 || cap < len || size <= 0) {
		return NULL;
	}

	s->len = len;
	if ((s->mem = slice_mem_new(cap, size))) {
		s->base = s->mem->data;
	} else if (cap == 0) {
		s->size = size;
	} else {
		return NULL;
	}

	return s;
}

void slice_destroy(struct slice *s)
{
	slice_mem_decr(s->mem);
	memset(s, 0, sizeof(*s));
}

long slice_len(struct slice const *s)
{
	return s->len;
}

long slice_cap(struct slice const *s)
{
	return s->mem ? s->mem->cap - (s->base - s->mem->data) : 0;
}

long slice_size(struct slice const *s)
{
	return s->mem ? s->mem->size : s->size;
}

void *slice_at(struct slice const *s, long i)
{
	return i >= 0 && i < s->len ? &s->base[i * slice_size(s)] : NULL;
}

void slice_slice(struct slice *dst, struct slice const *src, long idx, long len)
{
	if (len < 0 || idx < 0 || idx + len > slice_cap(src)) {
		abort();
	}

	dst->base = slice_at(src, idx);
	dst->len = len;

	slice_mem_incr(src->mem);
	slice_mem_decr(dst->mem);
	dst->mem = src->mem;
}

long slice_copy(struct slice dst, struct slice src)
{
	long n = dst.len < src.len ? dst.len : src.len;
	if (n > 0) {
		long const size = slice_size(&src);
		if (size != slice_size(&dst)) {
			return -1;
		}
		memmove(dst.base, src.base, n * size);
	}
	return n;
}

int slice_clone(struct slice *dst, struct slice const *src)
{
	if (dst != src) {
		slice_mem_decr(dst->mem);
		long const len = slice_len(src);
		if (!slice_create(dst, len, len, slice_size(src))) {
			return -1;
		}
		slice_copy(*dst, *src);
	}
	return 0;
}

void *slice_append(struct slice *s, long n)
{
	long len = s->len + n;
	long cap = slice_cap(s);
	if (len <= 0) {
		n = s->len;
		s->len = 0;
	} else if (len <= cap) {
		s->len = len;
	} else {
		/* Grow. */
		cap += cap / 2 + 8;
		cap = cap > len ? cap : len;

		struct slice d;
		if (!slice_create(&d, len, cap, slice_size(s))) {
			return NULL;
		}
		slice_copy(d, *s);

		slice_mem_decr(s->mem);
		*s = d;
	}

	return slice_at(s, len - n);
}

#if 0
static int overlap(struct slice s1, struct slice s2)
{
	return s1.mem && s1.mem == s2.mem &&
		s1.base + s1.len > s2.base &&
		s1.base < s2.base + s2.len;
}
#endif

int slice_concat(struct slice *dst, struct slice const *src)
{
	long len = slice_len(src);
	if (len == 0) {
		return 0;
	}

	long const size = slice_size(src);
	if (size != slice_size(dst)) {
		return -1;
	}

	void *x = slice_append(dst, len);
	if (!x) {
		return -1;
	}
	memmove(x, src->base, len * size);

	return 0;
}

void *slice_swap(struct slice *s, long i, long j)
{
	if (i < 0 || i >= s->len || j < 0 || j >= s->len) {
		return NULL;
	}
	if (i == j) {
		return slice_at(s, j);
	}
	void *const tmp = slice_append(s, 1);
	if (tmp) {
		long const size = s->size;
		void *const si = &s->base[i * size];
		void *const sj = &s->base[j * size];
		memcpy(tmp, si, size);
		memcpy(si, sj, size);
		memcpy(sj, tmp, size);
		s->len--;
	}
	return tmp;
}
