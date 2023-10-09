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

#include "htable.h"

enum hbucket_state {
	HBUCKET_EMPTY = 0,
	HBUCKET_TOMB  = 1,
	HBUCKET_USED  = 2,
};

struct hbucket {
	unsigned long hash;
};

#if ULONG_MAX == 0xffffffff
#	define HTABLE_BITS 32
#	define HTABLE_MULT 0x93c467e3U
#elif ULONG_MAX == 0xffffffffffffffff
#	define HTABLE_BITS 64
#	define HTABLE_MULT 0x93c467e37db0c7a3U
#else
#	error ULONG_WIDTH different from 32 and 64 not implemented.
#endif

#define HTABLE_PROBE_LOOP(idx, hash, htab, body) do {			\
	long ht__i;							\
	for (ht__i = 0, (idx) = ((hash) * HTABLE_MULT) >>		\
			(HTABLE_BITS - (htab)->shift);;			\
		++ht__i, (idx) = ((idx) + ht__i) & (htab)->mask)	\
	{ body }							\
} while (0)

#define HTABLE_SHIFT_MIN	3
#define HTABLE_SIZE(shift)	(1L << (shift))
#define HTABLE_CAP(shift)	((1L << ((shift) - 2)) * 3)

void htable_create(struct htable *ht, long inc, unsigned long seed,
		struct htable_interface const *hasher)
{
	assert(ht);
	assert(inc > 0);
	assert(hasher);
	assert(hasher->hash);
	assert(hasher->comp);

	ht->data = NULL;
	ht->table = NULL;

	ht->inc = inc;
	ht->len = 0;
	ht->cap = 0;

	ht->mask = -1;
	ht->shift = HTABLE_SHIFT_MIN;

	ht->seed = seed;
	ht->hasher = hasher;
}

void htable_destroy(struct htable *ht)
{
	assert(ht);

	free(ht->table);
	ht->table = NULL;
	ht->data = NULL;
	ht->mask = -1;
	ht->shift = HTABLE_SHIFT_MIN;
	ht->len = 0;
	ht->cap = 0;
}

static
int htable_rehash(struct htable *ht, int shift)
{
	assert(ht);
	assert(shift >= 2 && shift <= HTABLE_BITS - 1);
	assert(HTABLE_CAP(shift) >= ht->len);

	long const inc = ht->inc;
	struct htable htnew;
	htable_create(&htnew, inc, ht->seed, ht->hasher);

	long size = HTABLE_SIZE(shift);

	htnew.table = malloc(size * (sizeof(htnew.table[0]) + inc));
	if (!htnew.table) {
		return -ENOMEM;
	}

	htnew.data = (char *)htnew.table + size * sizeof(htnew.table[0]);

	htnew.cap = HTABLE_CAP(shift);
	htnew.mask = size - 1;
	htnew.shift = shift;
	while (size--) {
		htnew.table[size].hash = HBUCKET_EMPTY;
	}

	long j;
	for (j = 0; j <= ht->mask; j++) {
		if (ht->table[j].hash >= HBUCKET_USED) {
			long i;
			HTABLE_PROBE_LOOP(i, ht->table[j].hash, &htnew,
				if (htnew.table[i].hash == HBUCKET_EMPTY) {
					htnew.table[i].hash = ht->table[j].hash;
					memcpy(htnew.data + i * inc,
							ht->data + j * inc,
							inc);
					break;
				}
			);
		}
	}
	htnew.len = ht->len;
	htnew.cap -= ht->len;

	htable_destroy(ht);
	*ht = htnew;
	return 0;
}

int htable_resize(struct htable *ht, long cap)
{
	assert(ht);
	assert(cap > 0);
	assert(HTABLE_SHIFT_MIN >= 2);

	if (ht->len <= cap) {
		int shift = HTABLE_CAP(ht->shift) <= cap ?
			ht->shift : HTABLE_SHIFT_MIN;
		for (; HTABLE_CAP(shift) < cap; shift++);
		return shift != ht->shift ?
			htable_rehash(ht, shift) : 0;
	}

	return -ENOSPC;
}

void *htable_enter(struct htable *ht, void const *key, int *exists)
{
	assert(ht);

	// rehash if necessary
	if (ht->cap == 0) {
		int const shift = ht->shift +
			(ht->len > HTABLE_CAP(ht->shift) / 2);
		if (htable_rehash(ht, shift)) {
			return NULL;
		}
	}

	unsigned long hash = ht->hasher->hash(key, ht->seed);
	if (hash < HBUCKET_USED) {
		hash = HBUCKET_USED;
	}
	long i;
	long j = -1;
	HTABLE_PROBE_LOOP(i, hash, ht,
		if (ht->table[i].hash == HBUCKET_EMPTY) {
			if (j < 0) {
				ht->cap--;
			} else {
				i = j;
			}
			ht->table[i].hash = hash;
			ht->len++;
			*exists = 0;
			return &ht->data[i * ht->inc];
		} else if (ht->table[i].hash == HBUCKET_TOMB) {
			j = j < 0 ? i : j;
		} else if (ht->table[i].hash == hash &&
				!ht->hasher->comp(key, &ht->data[i * ht->inc])) {
			/* not for multimap */
			*exists = 1;
			return &ht->data[i * ht->inc];
		}
	);

	assert(0);
}

void *htable_find(struct htable const *ht, void const *key)
{
	assert(ht);

	unsigned long hash = ht->hasher->hash(key, ht->seed);
	if (hash < HBUCKET_USED) {
		hash = HBUCKET_USED;
	}
	long i;
	HTABLE_PROBE_LOOP(i, hash, ht,
		if (ht->table[i].hash == HBUCKET_EMPTY) {
			return NULL;
		} else if (ht->table[i].hash == HBUCKET_TOMB) {
			/* skip/maybe insert? */
		} else if (ht->table[i].hash == hash &&
				!ht->hasher->comp(key, &ht->data[i * ht->inc])) {
			/* not for multimap */
			return &ht->data[i * ht->inc];
		}
	);

	assert(0);
}

void *htable_delete(struct htable *ht, void const *key)
{
	assert(ht);

	unsigned long hash = ht->hasher->hash(key, ht->seed);
	if (hash < HBUCKET_USED) {
		hash = HBUCKET_USED;
	}
	long i;
	HTABLE_PROBE_LOOP(i, hash, ht,
		if (ht->table[i].hash == HBUCKET_EMPTY) {
			return NULL;
		} else if (ht->table[i].hash == HBUCKET_TOMB) {
			/* skip */
		} else if (ht->table[i].hash == hash &&
				!ht->hasher->comp(key, &ht->data[i * ht->inc])) {
			/* not for multimap */
			ht->table[i].hash = HBUCKET_TOMB;
			ht->len--;
			return &ht->data[i * ht->inc];
		}
	);

	assert(0);
}

long htable_len(struct htable const *ht)
{
	assert(ht);

	return ht->len;
}

void htable_walk(struct htable const *ht,
		void (*action)(void const *item, void *context),
		void *context)
{
	assert(ht);
	assert(action);

	long j;
	for (j = 0; j <= ht->mask; j++) {
		if (ht->table[j].hash >= HBUCKET_USED) {
			(*action)(&ht->data[j * ht->inc], context);
		}
	}
}

void *htable_yield(struct htable const *ht, long *iter)
{
	assert(ht);
	assert(iter && *iter >= 0);

	long j;
	for (j = *iter; j <= ht->mask; j++) {
		if (ht->table[j].hash >= HBUCKET_USED) {
			*iter = j;
			return &ht->data[j * ht->inc];
		}
	}
	return NULL;
}
