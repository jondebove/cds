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

#ifndef COMB_H_INCLUDED
#define COMB_H_INCLUDED

#include <stdbool.h>

/*!
 * COMB_ORDERED is an option that makes the order of the elements significant
 * when enumerating the combinations.
 */ 
#define COMB_ORDERED	0x1

/*!
 * COMB_REPETITION is an option to allow repetition of elements in
 * combinations.
 */ 
#define COMB_REPETITION	0x2

/*!
 * comb_action is a callback function called for each combination arr of size k.
 * ctx allows to not use global variables in the callback function.
 * comb_action shall return true to continue to the next combination or false
 * to exit early.
 */
typedef bool (*comb_action)(long k, long arr[static k], void *ctx);

/*!
 * comb calls f for each combinations of k elements among n.
 * The algorithm necessitates an array of n + k elements, arr, and may be
 * tweaked with the flags COMB_ORDERED and COMB_REPETITION.
 * comb returns 0 on success and -1 in case of invalid argument.
 */
int comb(long n, long k, long arr[static n + k], comb_action f, void *ctx,
		int flags);

/*!
 * prod calls f for each combinations made by the cartesian product of k arrays.
 * The lengths of the k arrays shall be given after the ctx argument.
 * The algorithm necessitates an array of k elements arr.
 * prod returns 0 on success and -1 in case of invalid argument.
 */
int prod(long k, long arr[static k], comb_action f, void *ctx, /* long */ ...);

#endif /* COMB_H_INCLUDED */
