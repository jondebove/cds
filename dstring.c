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

#include <stdarg.h>

#include "dstring.h"

extern void dstring_create(struct dstring *s);
extern void dstring_destroy(struct dstring *s);

extern int dstring_setcap(struct dstring *s, long cap);
extern int dstring_setlen(struct dstring *s, long len);

extern int dstring_chomp(struct dstring *s);
extern int dstring_concat(struct dstring *s, char const *str, long len);
extern int dstring_concatf(struct dstring *s, char const *fmt, ...);
extern int dstring_printf(struct dstring *s, char const *fmt, ...);
extern int dstring_setstr(struct dstring *s, char const *str, long len);

extern char const *dstring_str(struct dstring const *s);
extern char *dstring_at(struct dstring const *s, long i);
extern long dstring_len(struct dstring const *s);

extern int dstring_compare(struct dstring const *s1, struct dstring const *s2);

int dstring_vconcatf(struct dstring *s, char const *fmt, va_list ap)
{
	assert(s);
	assert(fmt);

	int const len = s->len;
	va_list args;

	va_copy(args, ap);
	int n = vsnprintf(s->str + len, s->cap - len, fmt, args);
	va_end(args);
	if (n < 0) {
		return -EINVAL;
	}
	if (n < s->cap - len) {
		s->len += n;
		return 0;
	}

	int err = dstring_setlen(s, s->len + n);
	if (err) {
		return err;
	}

	va_copy(args, ap);
	n = vsnprintf(s->str + len, s->cap - len, fmt, ap);
	va_end(args);

	assert(n == s->len - len);
	return 0;
}

