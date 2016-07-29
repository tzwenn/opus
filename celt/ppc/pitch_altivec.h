/* Copyright (c) 2013 Jean-Marc Valin and John Ridges
   Copyright (c) 2014, Cisco Systems, INC MingXiang WeiZhou MinPeng YanWang
   Copyright (c) 2016, Sven Köhler */
/**
   @file pitch_altivec.h
   @brief Pitch analysis
 */

/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PITCH_ALTIVEC_H
#define PITCH_ALTIVEC_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef OPUS_PPC_MAY_HAVE_ALTIVEC
void xcorr_kernel_altivec(
					const opus_val16 *x,
					const opus_val16 *y,
					opus_val32       sum[4],
					int              len);

#define OVERRIDE_XCORR_KERNEL
#define xcorr_kernel(x, y, sum, len, arch) \
	((void)arch, xcorr_kernel_altivec(x, y, sum, len))



void dual_inner_prod_altivec(const opus_val16 *x,
	const opus_val16 *y01,
	const opus_val16 *y02,
	int               N,
	opus_val32       *xy1,
	opus_val32       *xy2);

#define OVERRIDE_DUAL_INNER_PROD
#undef dual_inner_prod
#define dual_inner_prod(x, y01, y02, N, xy1, xy2, arch) \
	((void)(arch),dual_inner_prod_altivec(x, y01, y02, N, xy1, xy2))



opus_val32 celt_inner_prod_altivec(
	const opus_val16 *x,
	const opus_val16 *y,
	int               N);

#define OVERRIDE_CELT_INNER_PROD
#define celt_inner_prod(x, y, N, arch) \
	((void)arch, celt_inner_prod_altivec(x, y, N))



void comb_filter_const_altivec(opus_val32 *y,
	opus_val32 *x,
	int         T,
	int         N,
	opus_val16  g10,
	opus_val16  g11,
	opus_val16  g12);

#define OVERRIDE_COMB_FILTER_CONST
#undef comb_filter_const
#define comb_filter_const(y, x, T, N, g10, g11, g12, arch) \
	((void)(arch),comb_filter_const_altivec(y, x, T, N, g10, g11, g12))

#endif

#endif
