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

#include "deque.h"

void deque_create(struct deque *q, long inc)
{
	assert(q);
	assert(inc > 0);

	q->data = NULL;
	q->bits = -1;
	q->off = 0;
	q->len = 0;
	q->inc = inc;
}

void deque_destroy(struct deque *q)
{
	assert(q);

	free(q->data);
	q->data = NULL;
	q->off = 0;
	q->bits = -1;
	q->len = 0;
}

#if ULONG_MAX == 0xffffffff
#	define DEQUE_BITS_MAX 30
#elif ULONG_MAX == 0xffffffffffffffff
#	define DEQUE_BITS_MAX 62
#else
#	error ULONG_WIDTH different from 32 and 64 not implemented.
#endif

int deque_setcap(struct deque *q, long bits)
{
	assert(q);
	assert(bits <= DEQUE_BITS_MAX);

	if (bits < 0) {
		deque_destroy(q);
		return 0;
	}

	char *data = malloc(DEQUE_CAP(bits) * q->inc);
	if (data) {
		if (q->len > 0) {
			long n = DEQUE_CAP(q->bits) - q->off;
			memcpy(data, q->data + q->off * q->inc, n * q->inc);
			if (n < q->len) {
				memcpy(data + n * q->inc, q->data,
						(q->len - n)* q->inc);
			}
		}
		q->data = data;
		q->bits = bits;
		q->off = 0;
		return 0;
	}

	return -ENOMEM;
}

void *deque_swap(struct deque *q, long i, long j)
{
	assert(q);

	if (i == j) {
		return deque_at(q, j);
	} else if (i >= 0 && i < q->len && j >= 0 && j < q->len) {
		i = DEQUE_INDEX(q, i);
		j = DEQUE_INDEX(q, j);
		void *const tmp = deque_push(q);
		if (tmp) {
			long const inc = q->inc;
			void *const qi = q->data + i * inc;
			void *const qj = q->data + j * inc;
			memcpy(tmp, qi, inc);
			memcpy(qi, qj, inc);
			memcpy(qj, tmp, inc);
			return deque_pop(q);
		}
	}
	return NULL;
}

extern void *deque_at(struct deque const *q, long i);
extern void *deque_push(struct deque *q);
extern void *deque_pop(struct deque *q);
extern void *deque_shift(struct deque *q);
extern void *deque_unshift(struct deque *q);
