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

#ifndef CDS_DSTRING_H
#define CDS_DSTRING_H

/*!
 * \file dstring.h
 * \author Jonathan Debove
 * \brief Generic dynamic string.
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifndef DSTRING_NEGATIVE_INDEX
/*! Flag to allow negative indexing to access the end of the string. */
#define DSTRING_NEGATIVE_INDEX	1
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! Dynamic string. */
struct dstring {
	/* private */
	char *str;
	long len;
	long cap;
	//int err;
	//char scratch[64];
};

#define DSTRING() {.str = NULL, .len = 0, .cap = 0}

/*! dstring_create initializes a dynamic string `s`.
 * It cannot fail and does not allocate memory.
 */
inline
void dstring_create(struct dstring *s)
{
	assert(s);

	s->str = NULL;
	s->cap = 0;
	s->len = 0;
}

/*! dstring_destroy frees the memory space internal to the dynamic string.
 * On output, the dynamic string is empty and in a valid state.
 */
inline
void dstring_destroy(struct dstring *s)
{
	assert(s);

	free(s->str);
	dstring_create(s);
}

/*! dstring_setcap sets the maximum capacity of the dynamic string.
 * It returns `0` on success or `-ENOMEM` on out of memory.
 */
inline
int dstring_setcap(struct dstring *s, long cap)
{
	assert(s);
	assert(cap >= 0);
	assert(cap < LONG_MAX);

	if (cap == 0) {
		dstring_destroy(s);
		return 0;
	}

	char *str = (char *)realloc(s->str, cap);  // C++ cast
	if (str) {
		s->str = str;
		s->cap = cap;
		if (s->len > cap - 1) {
			s->len = cap - 1;
			s->str[s->len] = '\0';
		}
		return 0;
	}

	return -ENOMEM;
}

/*! dstring_setlen sets the number of characters of the dynamic string.
 * It returns `0` on success or `-ENOMEM` on out of memory.
 */
inline
int dstring_setlen(struct dstring *s, long len)
{
	assert(s);
	assert(len >= 0);

	if (len > s->cap - 1) {
		/* test overflow */
		long grow = s->cap / 2 + 4;
		grow = s->cap <= LONG_MAX - grow ? s->cap + grow : len + 1;
		int err = dstring_setcap(s, grow > len ? grow : len + 1);
		if (err) {
			return err;
		}
	}

	s->len = len;
	s->str[len] = '\0';

	return 0;
}

/*! dstring_concat appends len characters from `str` to `s`.
 * It returns `0` on success or `-ENOMEM` on out of memory.
 */
inline
int dstring_concat(struct dstring *s, char const *str, long len)
{
	assert(s);
	assert(str);
	assert(len >= 0);

	char *a = s->str + s->len;
	int err = dstring_setlen(s, s->len + len);
	if (err) {
		return err;
	}

	memcpy(a, str, len);
	return 0;
}

/*! dstring_chomp removes trailing line feed.
 * It returns `0` on success or `-1` on error.
 */
inline
int dstring_chomp(struct dstring *s)
{
	assert(s);

	if (s->len > 0 && s->str[s->len - 1] == '\n') {
		s->str[--s->len] = '\0';
		return 0;
	}
	return -1;
}

/*! dstring_printf writes formatted string to `s`.
 * It returns `0` on success or `-ENOMEM` on out of memory.
 */
inline
int dstring_printf(struct dstring *s, char const *fmt, ...)
{
	assert(s);
	assert(fmt);

	va_list ap;

	va_start(ap, fmt);
	int len = vsnprintf(s->str, s->cap, fmt, ap);
	va_end(ap);
	if (len < 0) {
		return -EINVAL;
	}
	if (len < s->cap) {
		s->len = len;
		return 0;
	}

	int err = dstring_setlen(s, len);
	if (err) {
		return err;
	}

	va_start(ap, fmt);
	len = vsnprintf(s->str, s->cap, fmt, ap);
	va_end(ap);

	assert(len == s->len);
	return 0;
}

/*! dstring_setstr copy len characters from `str` to `s`.
 * It returns `0` on success or `-ENOMEM` on out of memory.
 */
inline
int dstring_setstr(struct dstring *s, char const *str, long len)
{
	assert(s);
	assert(str);
	assert(len >= 0);

	int err = dstring_setlen(s, len);
	if (err) {
		return err;
	}

	memmove(s->str, str, len);
	return 0;
}

/*! dstring_str returns a pointer to the first character.
 * It returns `NULL` if the string is empty.
 */
inline
char const *dstring_str(struct dstring const *s)
{
	assert(s);

	return s->len > 0 ? s->str : "";
}

#if DSTRING_NEGATIVE_INDEX
#	define DSTRING_INDEX(a, i) ((i) >= 0 ? (i) : (a)->len - (i))
#else
#	define DSTRING_INDEX(a, i) (assert((i) >= 0), (i))
#endif

/*! dstring_at returns a pointer to the `i`th character.
 * Returns `NULL` if index is out of bounds.
 */
inline
char *dstring_at(struct dstring const *s, long i)
{
	assert(s);

	i = DSTRING_INDEX(s, i);

	return i < s->len ? s->str + i : NULL;
}

/*! dstring_len returns the number of characters. */
inline
long dstring_len(struct dstring const *s)
{
	assert(s);

	return s->len;
}

#ifdef __cplusplus
}
#endif

#endif /* CDS_DSTRING_H */
