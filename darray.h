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

#ifndef CDS_DARRAY_H
#define CDS_DARRAY_H

/*!
 * \file darray.h
 * \author Jonathan Debove
 * \brief Generic dynamic array.
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#ifndef DARRAY_NEGATIVE_INDEX
/*! Flag to allow negative indexing to access the end of the array. */
#define DARRAY_NEGATIVE_INDEX	1
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! Generic dynamic array.
 * Do not modify its fields unless you know what you are doing.
 */
struct darray {
	/* private */
	char *data;
	long len;
	long cap;
	long inc;
	//int err;
	//char scratch[64];
};

/*! darray_create initializes a dynamic array `da` of element of size `inc`.
 * It cannot fail and does not allocate memory.
 */
inline
void darray_create(struct darray *da, long inc)
{
	assert(da);
	assert(inc > 0);

	da->data = NULL;
	da->cap = 0;
	da->len = 0;
	da->inc = inc;
}

/*! darray_destroy frees the memory space internal to the dynamic array.
 * It does not free the memory allocated by the user for the darray nor
 * the entries. On output, the dynamic array is empty and in a valid state.
 */
inline
void darray_destroy(struct darray *da)
{
	assert(da);

	free(da->data);
	da->data = NULL;
	da->len = 0;
	da->cap = 0;
}

/*! darray_setcap sets the maximum capacity of the dynamic array.
 * It returns `0` on success or `-ENOMEM` on out of memory.
 */
inline
int darray_setcap(struct darray *da, long cap)
{
	assert(da);
	assert(cap >= 0);
	assert(cap <= LONG_MAX / da->inc);

	if (cap == 0) {
		darray_destroy(da);
		return 0;
	}

	char *data = (char *)realloc(da->data, cap * da->inc);  // C++ cast
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

/*! darray_setlen sets the number of elements of the dynamic array.
 * It returns `0` on success or `-ENOMEM` on out of memory.
 */
inline
int darray_setlen(struct darray *da, long len)
{
	assert(da);
	assert(len >= 0);

	if (len > da->cap) {
		/* test overflow */
		long grow = da->cap / 2 + 8;
		grow = da->cap <= LONG_MAX - grow ? da->cap + grow : len;
		int err = darray_setcap(da, grow > len ? grow : len);
		if (err) {
			return err;
		}
	}

	da->len = len;

	return 0;
}

/*! darray_push grows the array by `n` elements.
 * Returns the pointer to the first appended element.
 * Returns `NULL` if it fails (out of memory).
 */
inline
void *darray_push(struct darray *da, long n)
{
	assert(da);
	assert(n >= 0);
	assert(n <= LONG_MAX - da->len);

	long const len = da->len;
	if (darray_setlen(da, len + n)) {
		return NULL;
	}
	return da->data + len * da->inc;
}

/*! darray_pop shrinks the array by `n` elements.
 * Returns a pointer to the first erased element.
 * Returns `NULL` on error.
 */
inline
void *darray_pop(struct darray *da, long n)
{
	assert(da);
	assert(n >= 0);

	if (da->len >= n) {
		da->len -= n;
		return da->data + da->len * da->inc;
	}

	return NULL;
}

#if DARRAY_NEGATIVE_INDEX
#	define DARRAY_INDEX(a, i) ((i) >= 0 ? (i) : (a)->len + (i))
#else
#	define DARRAY_INDEX(a, i) (assert((i) >= 0), (i))
#endif

/*! darray_splice removes `rem` elements and inserts `ins` elements at
 * index `off`. The array grows and shrinks as necessary.
 * Returns a pointer to the element at index `off`, or `NULL` if out of memory.
 */
inline
void *darray_splice(struct darray *da, long off, long rem, long ins)
{
	assert(da);
	assert(rem >= 0);
	assert(ins >= 0);
	assert(ins - rem <= LONG_MAX - da->len);	/* overflow */

	off = DARRAY_INDEX(da, off);
	assert(off + rem <= da->len);

	if (darray_setlen(da, da->len - rem + ins)) {
		return NULL;
	}

	long const inc = da->inc;
	char *const a = da->data + off * inc;
	memmove(a + ins * inc, a + rem * inc, (da->len - off - ins) * inc);
	return a;
}

/*! darray_removeswap removes the element at index `i`.
 * Order of the array is not preserved but the operation is faster.
 * Returns a pointer to the swapped element or `NULL` if index is out of bounds.
 */
inline
void *darray_removeswap(struct darray *da, long i)
{
	assert(da);

	i = DARRAY_INDEX(da, i);

	if (i < da->len) {
		void const *const last = darray_pop(da, 1);
		if (last) {
			long const inc = da->inc;
			return memmove(da->data + i * inc, last, inc);
		}
	}
	return NULL;
}

/*! darray_swap swaps the elements at index `i` and `j`.
 * Returns `NULL` on error.
 */
inline
void *darray_swap(struct darray *da, long i, long j)
{
	assert(da);

	i = DARRAY_INDEX(da, i);
	j = DARRAY_INDEX(da, j);

	if (i < da->len && j < da->len && i != j) {
		char *const tmp = (char *)darray_push(da, 1);  // C++ cast
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

/*! darray_data returns a pointer to the first element.
 * Returns `NULL` if array is empty.
 */
inline
void *darray_data(struct darray const *da)
{
	assert(da);

	return da->len > 0 ? da->data : NULL;
}

/*! darray_at returns a pointer to the `i`th element.
 * Returns `NULL` if index is out of bounds.
 */
inline
void *darray_at(struct darray const *da, long i)
{
	assert(da);

	i = DARRAY_INDEX(da, i);

	return i < da->len ?
		da->data + i * da->inc :
		NULL;
}

/*! darray_len returns the number of elements. */
inline
long darray_len(struct darray const *da)
{
	assert(da);

	return da->len;
}

/*! DARRAY_FOREACH iterates over all the elements of the dynamic array. */
#define DARRAY_FOREACH(elem, darray)					\
	for (long da__idx = 0;						\
			((elem) = darray_at((darray), da__idx));	\
			da__idx++)

#ifdef __cplusplus
}
#endif

#endif /* CDS_DARRAY_H */
