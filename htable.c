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

static struct hbucket table_empty[1UL << HTABLE_SHIFT_MIN] = { 0 };

void htable_create(struct htable *ht, long inc, unsigned long seed,
		struct htable_interface const *hasher)
{
	assert(ht);
	assert(inc > 0);
	assert(hasher);
	assert(hasher->hash);
	assert(hasher->comp);

	ht->data = NULL;
	ht->table = table_empty;

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

	if (ht->table != table_empty) {
		free(ht->table);
		ht->table = table_empty;
	}
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
	long const size = HTABLE_SIZE(shift);

	struct htable htnew;
	htable_create(&htnew, inc, ht->seed, ht->hasher);

	htnew.table = malloc(size * (sizeof(htnew.table[0]) + inc));
	if (!htnew.table) {
		return -ENOMEM;
	}

	htnew.data = (char *)htnew.table + size * sizeof(htnew.table[0]);
	htnew.cap = HTABLE_CAP(shift);
	htnew.mask = size - 1;
	htnew.shift = shift;

	long j;
	for (j = 0; j < size; j++) {
		htnew.table[j].hash = HBUCKET_EMPTY;
	}

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

static
unsigned long ht_hashof(struct htable const *ht, void const *key)
{
	unsigned long hash = ht->hasher->hash(key, ht->seed);
	return hash >= HBUCKET_USED ? hash : HBUCKET_USED;
}

static
int ht_isequal(struct htable const *ht, long idx, unsigned long hash,
		void const *key)
{
	return ht->table[idx].hash == hash &&
		ht->hasher->comp(key, &ht->data[idx * ht->inc]) == 0;
}

void *htable_enter_unsafe(struct htable *ht, void const *key, int *err)
{
	assert(ht);

	// rehash if necessary
	if (ht->cap == 0) {
		int const shift = ht->shift +
			(ht->len > HTABLE_CAP(ht->shift) / 2);
		if ((*err = htable_rehash(ht, shift))) {
			return NULL;
		}
	}

	unsigned long hash = ht_hashof(ht, key);
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
			*err = 0;
			return &ht->data[i * ht->inc];
		} else if (ht->table[i].hash == HBUCKET_TOMB) {
			j = j < 0 ? i : j;
		} else if (ht_isequal(ht, i, hash, key)) {
			*err = -EEXIST;
			return &ht->data[i * ht->inc];
		}
	);

	assert(0);
}

void *htable_enter(struct htable *ht,
		void const *key, void const *entry, int *err)
{
	assert(ht);

	if (ht->hasher->comp(key, entry)) {
		*err = -EINVAL;
		return NULL;
	}

	void *e = htable_enter_unsafe(ht, key, err);
	if (*err == 0) {
		memmove(e, entry, ht->inc);
	}
	return e;
}

void *htable_find(struct htable const *ht, void const *key)
{
	assert(ht);

	unsigned long hash = ht_hashof(ht, key);
	long i;
	HTABLE_PROBE_LOOP(i, hash, ht,
		if (ht->table[i].hash == HBUCKET_EMPTY) {
			return NULL;
		} else if (ht->table[i].hash == HBUCKET_TOMB) {
			continue;
		} else if (ht_isequal(ht, i, hash, key)) {
			return &ht->data[i * ht->inc];
		}
	);

	assert(0);
}

void *htable_delete(struct htable *ht, void const *key)
{
	assert(ht);

	unsigned long hash = ht_hashof(ht, key);
	long i;
	HTABLE_PROBE_LOOP(i, hash, ht,
		if (ht->table[i].hash == HBUCKET_EMPTY) {
			return NULL;
		} else if (ht->table[i].hash == HBUCKET_TOMB) {
			continue;
		} else if (ht_isequal(ht, i, hash, key)) {
			ht->table[i].hash = HBUCKET_TOMB;
			ht->len--;
			return &ht->data[i * ht->inc];
		}
	);

	assert(0);
}

int htable_delete_unsafe(struct htable *ht, void const *entry)
{
	assert(ht);
	assert(entry);

	long const i = ((char *)entry - ht->data) / ht->inc;
	if (ht->table[i].hash >= HBUCKET_USED) {
		ht->table[i].hash = HBUCKET_TOMB;
		ht->len--;
		return 0;
	}

	return -ENOENT;
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
