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

#ifndef CDS_DEQUE_H
#define CDS_DEQUE_H

/*!
 * \file deque.h
 * \author Jonathan Debove
 * \brief Generic double-ended queue.
 */

#include <assert.h>
#include <stddef.h>

/*! Double-ended queue.
 * Do not modify its fields unless you know what you are doing.
 */
struct deque {
	char *data;
	long off;
	long len;
	long bits;
	long inc;
};

/*! deque_create initializes a queue `q` of element of size `inc`.
 * It cannot fail and does not allocate memory.
 */
void deque_create(struct deque *q, long inc);

/*! deque_destroy frees the memory space internal to the queue.
 * It does not free the memory allocated by the user for the queue nor
 * the entries. On output, the queue is empty and in a valid state.
 */
void deque_destroy(struct deque *q);

#define DEQUE_BITS_MIN		3
#define DEQUE_CAP(bits)		(1L << (bits))
#define DEQUE_INDEX(q, i)	(((i) + (q)->off) & (DEQUE_CAP((q)->bits) - 1))

/*! deque_len returns the number of elements in the queue q. */
inline long deque_len(struct deque const *q)
{
	return q->len;
}

/*! deque_cap returns the capacity of the queue q. */
inline long deque_cap(struct deque const *q)
{
	return q->bits >= 0 ? DEQUE_CAP(q->bits) : 0;
}

/*! deque_at returns a pointer to the `i`th element.
 * Returns `NULL` if index is out of bounds.
 */
inline
void *deque_at(struct deque const *q, long i)
{
	assert(q);

	return i >= 0 && i < q->len ?
		&q->data[DEQUE_INDEX(q, i) * q->inc] :
		NULL;
}

/*! deque_setcap sets the maximum capacity of the queue as 2^bits elements.
 * It returns `0` on success or `-ENOMEM` on out of memory.
 */
int deque_setcap(struct deque *q, long bits);

/*! deque_push adds one element to the end of the queue q.
 * Returns the pointer to the appended element.
 * Returns `NULL` if it fails (out of memory).
 */
inline
void *deque_push(struct deque *q)
{
	assert(q);

	if (q->bits < 0) {
		if (deque_setcap(q, DEQUE_BITS_MIN)) {
			return NULL;
		}
	} else if (q->len == DEQUE_CAP(q->bits)) {
		if (deque_setcap(q, q->bits + 1)) {
			return NULL;
		}
	}

	long idx = DEQUE_INDEX(q, q->len);
	q->len++;
	return &q->data[idx * q->inc];
}

/*! deque_pop removes the last element of the queue q.
 * Returns a pointer to the erased element.
 * Returns `NULL` on error.
 */
inline
void *deque_pop(struct deque *q)
{
	assert(q);

	if (q->len >= 1) {
		q->len--;
		long idx = DEQUE_INDEX(q, q->len);
		return &q->data[idx * q->inc];
	}

	return NULL;
}

/*! deque_shift removes the first element of the queue q.
 * Returns a pointer to the erased element.
 * Returns `NULL` on error.
 */
inline
void *deque_shift(struct deque *q)
{
	assert(q);

	if (q->len >= 1) {
		q->len--;
		long idx = q->off;
		q->off = DEQUE_INDEX(q, 1);
		return &q->data[idx * q->inc];
	}

	return NULL;
}

/*! deque_unshift adds one element to the beginning of the queue q.
 * Returns the pointer to the prepended element.
 * Returns `NULL` if it fails (out of memory).
 */
inline
void *deque_unshift(struct deque *q)
{
	assert(q);

	if (q->bits < 0) {
		if (deque_setcap(q, DEQUE_BITS_MIN)) {
			return NULL;
		}
	} else if (q->len == DEQUE_CAP(q->bits)) {
		if (deque_setcap(q, q->bits + 1)) {
			return NULL;
		}
	}

	q->len++;
	q->off = DEQUE_INDEX(q, -1);
	return &q->data[q->off * q->inc];
}

/*! deque_swap swaps the elements at index `i` and `j`.
 * Returns `NULL` on error.
 */
void *deque_swap(struct deque *q, long i, long j);

/*! DEQUE_FOREACH iterates over all the elements of the queue. */
#define DEQUE_FOREACH(elem, queue)					\
	for (long q__idx = 0;						\
			((elem) = deque_at((queue), q__idx));		\
			q__idx++)

#ifdef __cplusplus
}
#endif

#endif /* CDS_DEQUE_H */
