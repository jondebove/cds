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

#ifndef CDS_HTABLE_H
#define CDS_HTABLE_H

/*!
 * \file htable.h
 * \author Jonathan Debove
 * \brief Generic hash table.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct hbucket;

/*! Interface for a hash table. */
struct htable_interface {
	/*! hash computes hash code of `key`. */
	unsigned long (*hash)(void const *key, unsigned long seed);
	/*! comp returns 0 if `key` and `entry` match. */
	int (*comp)(void const *key, void const *entry);

	/* in case of double hashing strategy (not implemented) */
	//unsigned long (*jump)(void const *key, unsigned long seed);
};

/*! Generic hash table.
 * Do not modify its fields unless you know what you are doing.
 */
struct htable {
	/* private */
	char *data;
	struct hbucket *table;
	long inc;
	long len;
	long cap;	/* size * 0.75	*/
	long mask;	/* size - 1	*/
	int shift;	/* log2(size)	*/
	unsigned long seed;
	struct htable_interface const *hasher;
};

/*! htable_create initializes a hash table `ht` of element of size `inc`.
 * It is recommended to supply a random `seed` in case the hash table is
 * under attack. Cannot fail and does not allocate memory.
 */
void htable_create(struct htable *ht, long inc, unsigned long seed,
		struct htable_interface const *hasher);

/*! htable_destroy frees the memory space internal to the hash table.
 * It does not free the memory allocated by the user for the htable nor
 * the entries. On output, the hash table is empty and in a valid state.
 */
void htable_destroy(struct htable *ht);

/*! htable_resize resizes hash table space for at least `cap` entries.
 * It returns `0` on success, `-ENOMEM` on out of memory and `-ENOSPC`
 * if `cap` is too small to contain all the entries.
 */
int htable_resize(struct htable *ht, long cap);

/*! htable_enter inserts an entry with key in the hash table.
 * `exists` is a boolean (0 or 1) indicating if an entry with `key`
 * is already present in the hash table.
 * It returns a pointer to the inserted entry or `NULL` on out of memory.
 */
void *htable_enter(struct htable *ht, void const *key, int *exists);

/*! htable_find searches an entry with key in the hash table.
 * It returns a pointer to the entry or `NULL` if not found.
 */
void *htable_find(struct htable const *ht, void const *key);

/*! htable_delete removes an entry from the hash table.
 * It returns a pointer to the entry or `NULL` if not found.
 */
void *htable_delete(struct htable *ht, void const *key);

/*! htable_len returns the number of entries. */
long htable_len(struct htable const *ht);

/*! htable_walk iterates over all the entries of the hash table.
 * For each entry, the function `action` is called with `context`.
 */
void htable_walk(struct htable const *ht,
		void (*action)(void const *item, void *context),
		void *context);

/*! htable_yield returns a pointer to the first entry in the hash table
 * with position greater than or equal to `iter`.
 * Returns `NULL` if no more entry exists at or after `iter` position.
 */
void *htable_yield(struct htable const *ht, long *iter);

/*! HTABLE_FOREACH iterates over all the entries of the hash table. */
#define HTABLE_FOREACH(entry, htable)					\
	for (long ht__idx = 0;						\
			((entry) = htable_yield((htable), &ht__idx));	\
			 ht__idx++)

#ifdef __cplusplus
}
#endif

#endif /* CDS_HTABLE_H */
