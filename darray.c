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

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "darray.h"

void darray_create(struct darray *da, long inc)
{
	assert(da);
	assert(inc > 0);

	da->data = NULL;
	da->cap = 0;
	da->len = 0;
	da->inc = inc;
}

void darray_destroy(struct darray *da)
{
	assert(da);

	free(da->data);
	da->data = NULL;
	da->len = 0;
	da->cap = 0;
}

int darray_setcap(struct darray *da, long cap)
{
	assert(da);
	assert(cap >= 0);
	assert(cap <= LONG_MAX / da->inc);

	if (cap == 0) {
		darray_destroy(da);
		return 0;
	}

	char *data = realloc(da->data, cap * da->inc);
	if (data) {
		da->data = data;
		da->cap = cap;
		if (da->len > cap) {
			da->len = cap;
		}
		return 0;
	}

	return -ENOMEM;
}

void *darray_splice(struct darray *da, long off, long rem, long ins)
{
	assert(da);
	assert(rem >= 0);
	assert(ins >= 0);
	assert(ins - rem <= LONG_MAX - da->len);	/* overflow */

	off = DARRAY_INDEX(da, off);
	assert(off >= 0 && off + rem <= da->len);

	if (darray_setlen(da, da->len - rem + ins)) {
		return NULL;
	}

	long const inc = da->inc;
	char *const a = da->data + off * inc;
	memmove(a + ins * inc, a + rem * inc, (da->len - off - ins) * inc);
	return a;
}

void *darray_removeswap(struct darray *da, long i)
{
	assert(da);

	i = DARRAY_INDEX(da, i);

	if (i >= 0 && i < da->len) {
		void const *const last = darray_pop(da, 1);
		if (last) {
			long const inc = da->inc;
			return memmove(da->data + i * inc, last, inc);
		}
	}
	return NULL;
}

void *darray_swap(struct darray *da, long i, long j)
{
	assert(da);

	i = DARRAY_INDEX(da, i);
	j = DARRAY_INDEX(da, j);

	if (i == j) {
		return darray_at(da, j);
	} else if (i >= 0 && i < da->len && j >= 0 && j < da->len) {
		char *const tmp = darray_push(da, 1);
		if (tmp) {
			long const inc = da->inc;
			char *const ai = da->data + i * inc;
			char *const aj = da->data + j * inc;
			memcpy(tmp, ai, inc);
			memcpy(ai, aj, inc);
			memcpy(aj, tmp, inc);
			return darray_pop(da, 1);
		}
	}
	return NULL;
}

extern void *darray_data(struct darray const *da);
extern void *darray_at(struct darray const *da, long i);
extern int darray_setlen(struct darray *da, long len);
extern void *darray_push(struct darray *da, long n);
extern void *darray_pop(struct darray *da, long n);
