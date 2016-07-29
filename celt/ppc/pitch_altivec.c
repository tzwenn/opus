/* Copyright (c) 2016, Sven Köhler


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

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "pitch.h"

#if __powerpc64__
#ifndef OPUS_PPC_MAY_HAVE_ALTIVEC
#define OPUS_PPC_MAY_HAVE_ALTIVEC
#endif
#endif

#ifdef OPUS_PPC_MAY_HAVE_ALTIVEC

#define ENABLE_ALTIVEC 1

#include <altivec.h>

////////////////////////////////

vector float vec_loadUnaligned(const float *data)
{
#if _ARCH_PWR8 && defined(__IBMC__)
	return vec_xlw4(0, data);
#else
	return *((vector float *) data);
#endif
}

void vec_storeUnaligned(float *target, vector float value)
{
	*((vector float *) target) = value;
}

#if !defined(__IBMC__)

#define extract_field(a, oct_field, offset) (unsigned char)((((a) >> ((oct_field) * 3)) & 7) * 4 + (offset))

vector unsigned char vec_gpci(int a)
{
	// FIXME: Endianess. And this function as a whole
	vector unsigned char mask = {
		extract_field(a, 3, 0), extract_field(a, 3, 1), extract_field(a, 3, 2), extract_field(a, 3, 3),
		extract_field(a, 2, 0), extract_field(a, 2, 1), extract_field(a, 2, 2), extract_field(a, 2, 3),
		extract_field(a, 1, 0), extract_field(a, 1, 1), extract_field(a, 1, 2), extract_field(a, 1, 3),
		extract_field(a, 0, 0), extract_field(a, 0, 1), extract_field(a, 0, 2), extract_field(a, 0, 3)
	};
	return mask;
}
#endif

vector float vec_permEl(vector float a, vector float b, int pattern)
{
	return vec_perm(a, b, vec_gpci(pattern));
}

vector float vec_splatf(float value)
{
	vector float res = {value, value, value, value};
	return res;
}

//////////////////////////////////


void xcorr_kernel_altivec(const opus_val16 *x, const opus_val16 *y, opus_val32 sum[4], int len)
{
#if ENABLE_ALTIVEC
	int j;
	vector float xsum1 = vec_loadUnaligned(sum);
	vector float xsum2 = {0, 0, 0, 0};

	for (j = 0; j < len - 3; j += 4)
	{
		vector float x0 = vec_loadUnaligned(x + j);
		vector float yj = vec_loadUnaligned(y + j);
		vector float y3 = vec_loadUnaligned(y + j + 3);

		xsum1 = vec_add(xsum1, vec_mul(vec_splat(x0, 0), yj));
		xsum2 = vec_add(xsum2, vec_mul(vec_splat(x0, 1),
									   vec_permEl(yj, y3, 01201)));
		xsum1 = vec_add(xsum1, vec_mul(vec_splat(x0, 2),
									   vec_permEl(yj, y3, 02312)));
		xsum2 = vec_add(xsum2, vec_mul(vec_splat(x0, 3), y3));
	}
	if (j < len) {
		xsum1 = vec_add(xsum1, vec_mul(vec_splatf(x[j]), vec_loadUnaligned(y+j)));
		if (++j < len) {
			xsum2 = vec_add(xsum2, vec_mul(vec_splatf(x[j]), vec_loadUnaligned(y+j)));
			if (++j < len) {
				xsum1 = vec_add(xsum1, vec_mul(vec_splatf(x[j]), vec_loadUnaligned(y+j)));
			}
		}
	}
	vec_storeUnaligned(sum, vec_add(xsum1, xsum2));
#else
	int j;
	opus_val16 y_0, y_1, y_2, y_3;
	celt_assert(len>=3);
	y_3=0; /* gcc doesn't realize that y_3 can't be used uninitialized */
	y_0=*y++;
	y_1=*y++;
	y_2=*y++;
	for (j=0;j<len-3;j+=4)
	{
	   opus_val16 tmp;
	   tmp = *x++;
	   y_3=*y++;
	   sum[0] = MAC16_16(sum[0],tmp,y_0);
	   sum[1] = MAC16_16(sum[1],tmp,y_1);
	   sum[2] = MAC16_16(sum[2],tmp,y_2);
	   sum[3] = MAC16_16(sum[3],tmp,y_3);
	   tmp=*x++;
	   y_0=*y++;
	   sum[0] = MAC16_16(sum[0],tmp,y_1);
	   sum[1] = MAC16_16(sum[1],tmp,y_2);
	   sum[2] = MAC16_16(sum[2],tmp,y_3);
	   sum[3] = MAC16_16(sum[3],tmp,y_0);
	   tmp=*x++;
	   y_1=*y++;
	   sum[0] = MAC16_16(sum[0],tmp,y_2);
	   sum[1] = MAC16_16(sum[1],tmp,y_3);
	   sum[2] = MAC16_16(sum[2],tmp,y_0);
	   sum[3] = MAC16_16(sum[3],tmp,y_1);
	   tmp=*x++;
	   y_2=*y++;
	   sum[0] = MAC16_16(sum[0],tmp,y_3);
	   sum[1] = MAC16_16(sum[1],tmp,y_0);
	   sum[2] = MAC16_16(sum[2],tmp,y_1);
	   sum[3] = MAC16_16(sum[3],tmp,y_2);
	}
	if (j++<len)
	{
	   opus_val16 tmp = *x++;
	   y_3=*y++;
	   sum[0] = MAC16_16(sum[0],tmp,y_0);
	   sum[1] = MAC16_16(sum[1],tmp,y_1);
	   sum[2] = MAC16_16(sum[2],tmp,y_2);
	   sum[3] = MAC16_16(sum[3],tmp,y_3);
	}
	if (j++<len)
	{
	   opus_val16 tmp=*x++;
	   y_0=*y++;
	   sum[0] = MAC16_16(sum[0],tmp,y_1);
	   sum[1] = MAC16_16(sum[1],tmp,y_2);
	   sum[2] = MAC16_16(sum[2],tmp,y_3);
	   sum[3] = MAC16_16(sum[3],tmp,y_0);
	}
	if (j<len)
	{
	   opus_val16 tmp=*x++;
	   y_1=*y++;
	   sum[0] = MAC16_16(sum[0],tmp,y_2);
	   sum[1] = MAC16_16(sum[1],tmp,y_3);
	   sum[2] = MAC16_16(sum[2],tmp,y_0);
	   sum[3] = MAC16_16(sum[3],tmp,y_1);
	}
#endif
}

void dual_inner_prod_altivec(const opus_val16 *x, const opus_val16 *y01, const opus_val16 *y02,
							 int N, opus_val32 *xy1, opus_val32 *xy2)
{
#if ENABLE_ALTIVEC
	int i;
	vector float xsum1 = {0, 0, 0, 0};
	vector float xsum2 = {0, 0, 0, 0};

	vector float xnull = {0, 0, 0, 0};

	for (i = 0; i < N - 3; i += 4) {
		vector float xi = vec_loadUnaligned(x + i);
		vector float y1i = vec_loadUnaligned(y01 + i);
		vector float y2i = vec_loadUnaligned(y02 + i);
		xsum1 = vec_add(xsum1, vec_mul(xi, y1i));
		xsum2 = vec_add(xsum2, vec_mul(xi, y2i));
	}
	/* Horizontal sum */

	xsum1 = vec_add(xsum1, vec_permEl(xsum1, xsum1, 02367));

	/*
	 * FIXME: Do i actually need vector ops here?
	 * SSE does here: 1. xsum1 = {xsum1[0] + xsum1[1], xsum[1], xsum[2], xsum[3]}
	 *                   xsum1 = xsum1 + {xsum1[1], 0, 0, 0}
	 *                2. *xy1 = xsum[0];
	 * So potentially this should do:
	 *                   *xy1 = xsum1[0] + xsum1[1];
	 */
	xsum1 = vec_add(xsum1, vec_permEl(xsum1, xnull, 01444));
	*xy1 = xsum1[0];

	xsum2 = vec_add(xsum2, vec_permEl(xsum2, xsum2, 02367));
	xsum2 = vec_add(xsum2, vec_permEl(xsum2, xnull, 01444));
	*xy2 = xsum2[0];

	for (;i<N;i++)
	{
	   *xy1 = MAC16_16(*xy1, x[i], y01[i]);
	   *xy2 = MAC16_16(*xy2, x[i], y02[i]);
	}
#else
	int i;
	opus_val32 xy01=0;
	opus_val32 xy02=0;
	for (i=0;i<N;i++)
	{
	   xy01 = MAC16_16(xy01, x[i], y01[i]);
	   xy02 = MAC16_16(xy02, x[i], y02[i]);
	}
	*xy1 = xy01;
	*xy2 = xy02;
#endif
}

opus_val32 celt_inner_prod_altivec(const opus_val16 *x, const opus_val16 *y,
	  int N)
{
#if ENABLE_ALTIVEC
	int i;
	float xy;

	vector float sum = {0, 0, 0, 0};
	vector float xnull = {0, 0, 0, 0};

	for (i = 0; i < N - 3; i += 4) {
		vector float xi = vec_loadUnaligned(x + i);
		vector float yi = vec_loadUnaligned(y + i);
		sum = vec_add(sum, vec_mul(xi, yi));
	}

	/* Horizontal sum */
	// FIXME: Same optimization as above needed
	sum = vec_add(sum, vec_permEl(sum, sum, 02367));
	sum = vec_add(sum, vec_permEl(sum, xnull, 01444));
	xy = sum[0];
	for (;i<N;i++)
	{
	   xy = MAC16_16(xy, x[i], y[i]);
	}
	return xy;
#else
	int i;
	opus_val32 xy=0;
	for (i=0;i<N;i++)
	   xy = MAC16_16(xy, x[i], y[i]);
	return xy;
#endif
}

void comb_filter_const_altivec(opus_val32 *y, opus_val32 *x, int T, int N,
	  opus_val16 g10, opus_val16 g11, opus_val16 g12)
{
#if ENABLE_ALTIVEC
	int i;
	vector float x0v = vec_loadUnaligned(&x[-T-2]);
	vector float g10v = vec_splatf(g10);
	vector float g11v = vec_splatf(g11);
	vector float g12v = vec_splatf(g12);

	for (i = 0; i < N - 3; i += 4) {
		const opus_val32 *xp = &x[i-T-2];
		vector float yi = vec_loadUnaligned(x + i);
		vector float x4v = vec_loadUnaligned(xp + 4);
#if 1
		vector float x1v = vec_loadUnaligned(xp + 1);
		vector float x2v = vec_loadUnaligned(xp + 2);
		vector float x3v = vec_loadUnaligned(xp + 3);
#else
		//FIXME: Not working (yet)
		vector float x2v = vec_permEl(x0v, x4v, 02301);
		vector float x1v = vec_permEl(x0v, x2v, 01212);
		vector float x3v = vec_permEl(x2v, x4v, 01212);
#endif

		yi = vec_add(yi, vec_mul(g10v, x2v));
#if 0 /* Set to 1 to make it bit-exact with the non-SSE/AltiVec version */
		yi = vec_add(yi, vec_mul(g11v, vec_add(x3v, x1v)));
		yi = vec_add(yi, vec_mul(g12v, vec_add(x4v, x0v)));
#else /* Use partial sums */
		vector float yi2 = vec_add(vec_mul(g11v, vec_add(x3v, x1v)),
							   vec_mul(g12v, vec_add(x4v, x0v)));
		yi = vec_add(yi, yi2);
#endif
		x0v =x4v;
		vec_storeUnaligned(y + i, yi);
	}
#ifdef CUSTOM_MODES
   for (;i<N;i++)
   {
	  y[i] = x[i]
			   + MULT16_32_Q15(g10,x[i-T])
			   + MULT16_32_Q15(g11,ADD32(x[i-T+1],x[i-T-1]))
			   + MULT16_32_Q15(g12,ADD32(x[i-T+2],x[i-T-2]));
   }
#endif

#else
	opus_val32 x0, x1, x2, x3, x4;
	int i;
	x4 = x[-T-2];
	x3 = x[-T-1];
	x2 = x[-T];
	x1 = x[-T+1];
	for (i=0;i<N;i++)
	{
	   x0=x[i-T+2];
	   y[i] = x[i]
				+ MULT16_32_Q15(g10,x2)
				+ MULT16_32_Q15(g11,ADD32(x1,x3))
				+ MULT16_32_Q15(g12,ADD32(x0,x4));
	   x4=x3;
	   x3=x2;
	   x2=x1;
	   x1=x0;
	}
#endif
}

#endif
