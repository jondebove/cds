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

#include <stdarg.h>

#include "comb.h"

static void reverse(long n, long arr[static n])
{
	while (n > 1) {
		long tmp = arr[0];
		arr[0] = arr[n - 1];
		arr[n - 1] = tmp;
		arr++;
		n -= 2;
	}
}

static bool comb_rec(long n, long k,
		long arr[restrict static n], long res[restrict static k],
		comb_action f, void *ctx, long l)
{
	if (k == 0) {
		return f(l, res - l, ctx);
	}
	for (long i = n - k + 1; i; i--) {
		*res = *arr;
		if (!comb_rec(--n, k - 1, ++arr, res + 1, f, ctx, l + 1)) {
			return false;
		}
	}
	return true;
}

static bool combr_rec(long n, long k,
		long arr[restrict static n], long res[restrict static k],
		comb_action f, void *ctx, long l)
{
	if (k == 0) {
		return f(l, res - l, ctx);
	}
	for (long i = n; i; i--) {
		*res = *arr;
		if (!combr_rec(n--, k - 1, arr++, res + 1, f, ctx, l + 1)) {
			return false;
		}
	}
	return true;
}

static bool perm_rec(long n, long k,
		long arr[restrict static n], long res[restrict static k],
		comb_action f, void *ctx, long l)
{
	if (k == 0) {
		return f(l, res - l, ctx);
	}
	for (long i = n; i; i--) {
		*res = *arr;
		if (!perm_rec(n - 1, k - 1, arr + 1, res + 1, f, ctx, l + 1)) {
			return false;
		}
		reverse(n - 1, arr + 1);
		reverse(n, arr);
	}
	return true;
}

static bool permr_rec(long n, long k,
		long arr[restrict static n], long res[restrict static k],
		comb_action f, void *ctx, long l)
{
	if (k == 0) {
		return f(l, res - l, ctx);
	}
	for (long i = n; i; i--) {
		*res = *arr;
		if (!permr_rec(n, k - 1, arr, res + 1, f, ctx, l + 1)) {
			return false;
		}
		reverse(n - 1, arr + 1);
		reverse(n, arr);
	}
	return true;
}

int comb(long n, long k, long arr[static n + k], comb_action f, void *ctx,
		int flags)
{
	if (k < 0 || n < (flags & COMB_REPETITION ? 0 : k)) {
		return -1;
	}
	for (long i = 0; i < n; i++) {
		arr[i] = i;
	}
	switch (flags) {
		case 0:
			comb_rec(n, k, arr, arr + n, f, ctx, 0);
			break;
		case COMB_ORDERED:
			perm_rec(n, k, arr, arr + n, f, ctx, 0);
			break;
		case COMB_REPETITION:
			combr_rec(n, k, arr, arr + n, f, ctx, 0);
			break;
		case COMB_ORDERED | COMB_REPETITION:
			permr_rec(n, k, arr, arr + n, f, ctx, 0);
			break;
		default:
			return -1;
	}
	return 0;
}

static bool prod_rec(long k, long res[static k], comb_action f, void *ctx,
		va_list args, long l)
{
	if (k == 0) {
		return f(l, res - l, ctx);
	}
	va_list ap;
	va_copy(ap, args);
	long n = va_arg(ap, long);
	for (long i = 0; i < n; i++) {
		*res = i;
		if (!prod_rec(k - 1, res + 1, f, ctx, ap, l + 1)) {
			va_end(ap);
			return false;
		}
	}
	va_end(ap);
	return true;
}

int prod(long k, long arr[static k], comb_action f, void *ctx, /* long */ ...)
{
	va_list ap;
	va_start(ap, ctx);
	prod_rec(k, arr, f, ctx, ap, 0);
	va_end(ap);
	return 0;
}
