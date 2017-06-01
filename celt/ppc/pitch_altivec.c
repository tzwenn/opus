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

#if defined(OPUS_PPC_MAY_HAVE_ALTIVEC)

#include <altivec.h>

////////////////////////////////

vector float vec_loadUnaligned(const float *data)
{
#if _ARCH_PWR8 && defined(__xlc__)
	return vec_xlw4(0, data);
#else
	return *((vector float *) data);
#endif
}

void vec_storeUnaligned(float *target, vector float value)
{
	*((vector float *) target) = value;
}

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
	int j;
	vector float xsum1 = vec_loadUnaligned(sum);
	vector float xsum2 = {0, 0, 0, 0};

	for (j = 0; j < len - 3; j += 4)
	{
		vector float x0 = vec_loadUnaligned(x + j);
		vector float yj = vec_loadUnaligned(y + j);
		vector float y3 = vec_loadUnaligned(y + j + 3);

		xsum1 = vec_madd(vec_splat(x0, 0), yj,                        xsum1);
		xsum2 = vec_madd(vec_splat(x0, 1), vec_permEl(yj, y3, 01201), xsum2);
		xsum1 = vec_madd(vec_splat(x0, 2), vec_permEl(yj, y3, 02312), xsum1);
		xsum2 = vec_madd(vec_splat(x0, 3), y3,                        xsum2);
	}
	if (j < len) {
		xsum1 = vec_madd(vec_splatf(x[j]), vec_loadUnaligned(y+j), xsum1);
		if (++j < len) {
			xsum2 = vec_madd(vec_splatf(x[j]), vec_loadUnaligned(y+j), xsum2);
			if (++j < len) {
				xsum1 = vec_madd(vec_splatf(x[j]), vec_loadUnaligned(y+j), xsum1);
			}
		}
	}
	vec_storeUnaligned(sum, vec_add(xsum1, xsum2));
}

void dual_inner_prod_altivec(const opus_val16 *x, const opus_val16 *y01, const opus_val16 *y02,
							 int N, opus_val32 *xy1, opus_val32 *xy2)
{
	int i;
	vector float xsum1 = {0, 0, 0, 0};
	vector float xsum2 = {0, 0, 0, 0};

	for (i = 0; i < N - 3; i += 4) {
		vector float xi = vec_loadUnaligned(x + i);
		vector float y1i = vec_loadUnaligned(y01 + i);
		vector float y2i = vec_loadUnaligned(y02 + i);
		xsum1 = vec_madd(xi, y1i, xsum1);
		xsum2 = vec_madd(xi, y2i, xsum2);
	}
	/* Horizontal sum */

	xsum1 = vec_add(xsum1, vec_permEl(xsum1, xsum1, 02367));
	*xy1 = xsum1[0] + xsum1[1];

	xsum2 = vec_add(xsum2, vec_permEl(xsum2, xsum2, 02367));
	*xy2 = xsum2[0] + xsum2[1];

	for (;i<N;i++)
	{
	   *xy1 = MAC16_16(*xy1, x[i], y01[i]);
	   *xy2 = MAC16_16(*xy2, x[i], y02[i]);
	}
}

opus_val32 celt_inner_prod_altivec(const opus_val16 *x, const opus_val16 *y,
	  int N)
{
	int i;
	float xy;

	vector float sum = {0, 0, 0, 0};

	for (i = 0; i < N - 3; i += 4) {
		vector float xi = vec_loadUnaligned(x + i);
		vector float yi = vec_loadUnaligned(y + i);
		sum = vec_madd(xi, yi, sum);
	}

	/* Horizontal sum */
	sum = vec_add(sum, vec_permEl(sum, sum, 02367));
	xy = sum[0] + sum[1];
	for (;i<N;i++)
	{
	   xy = MAC16_16(xy, x[i], y[i]);
	}
	return xy;
}

void comb_filter_const_altivec(opus_val32 *y, opus_val32 *x, int T, int N,
	  opus_val16 g10, opus_val16 g11, opus_val16 g12)
{
	int i;
	vector float x0v = vec_loadUnaligned(&x[-T-2]);
	vector float g10v = vec_splatf(g10);
	vector float g11v = vec_splatf(g11);
	vector float g12v = vec_splatf(g12);

	for (i = 0; i < N - 3; i += 4) {
		const opus_val32 *xp = &x[i-T-2];
		vector float yi = vec_loadUnaligned(x + i);
		vector float x4v = vec_loadUnaligned(xp + 4);

		vector float x1v = vec_permEl(x0v, x4v, 01234);
		vector float x2v = vec_permEl(x0v, x4v, 02345);
		vector float x3v = vec_permEl(x0v, x4v, 03456);

		yi = vec_madd(g10v, x2v, yi);
		yi = vec_madd(g11v, vec_add(x3v, x1v), yi);
		yi = vec_madd(g12v, vec_add(x4v, x0v), yi);
		x0v = x4v;
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
}

#endif
