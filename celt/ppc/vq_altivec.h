/* Copyright (c) 2016 */
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

#ifndef VQ_ALTIVEC_H
#define VQ_ALTIVEC_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef OPUS_PPC_MAY_HAVE_ALTIVEC

#ifdef NEED_vq_exp_rotation1
#include "altivec.h"

static void exp_rotation1_stride4(celt_norm *X, int len, opus_val16 c, opus_val16 s)
{
	int i;
	celt_norm x1, x2;
	vector float vx1, vx2;

	vector float vc = {c, c, c, c};
	vector float vs = {s, s, s, s};

	for (i = 0; i < len - 8; i += 4)
	{
		vx1 = *(vector float *)(&X[i]);
		vx2 = *(vector float *)(&X[i + 4]);

		*((vector float *)&X[i])     = vec_msub(vc, vx1, vec_mul(vs, vx2));
		*((vector float *)&X[i + 4]) = vec_madd(vc, vx2, vec_mul(vs, vx1));
	}

	for (; i < len - 4; i++)
	{
		x1 = X[i];
		x2 = X[i + 4];

		X[i]     = c * x1 - s * x2;
		X[i + 4] = c * x2 + s * x1;
	}

	for (i = len - 2 * 4 - 1; i >= 0 ; i--)
	{
		x1 = X[i];
		x2 = X[i + 4];

		X[i]     = c * x1 - s * x2;
		X[i + 4] = c * x2 + s * x1;
	}
}

#define OVERRIDE_vq_exp_rotation1
static void exp_rotation1(celt_norm *X, int len, int stride, opus_val16 c, opus_val16 s)
{
	if (stride == 4) {
		exp_rotation1_stride4(X, len, c, s);
		return;
	}

	int i;
	opus_val16 ms;
	celt_norm *Xptr;
	Xptr = X;
	ms = NEG16(s);
	for (i=0;i<len-stride;i++)
	{
		celt_norm x1, x2;
		x1 = Xptr[0];
		x2 = Xptr[stride];
		Xptr[stride] = EXTRACT16(PSHR32(MAC16_16(MULT16_16(c, x2),  s, x1), 15));
		*Xptr++      = EXTRACT16(PSHR32(MAC16_16(MULT16_16(c, x1), ms, x2), 15));
	}
	Xptr = &X[len-2*stride-1];
	for (i=len-2*stride-1;i>=0;i--)
	{
		celt_norm x1, x2;
		x1 = Xptr[0];
		x2 = Xptr[stride];
		Xptr[stride] = EXTRACT16(PSHR32(MAC16_16(MULT16_16(c, x2),  s, x1), 15));
		*Xptr--      = EXTRACT16(PSHR32(MAC16_16(MULT16_16(c, x1), ms, x2), 15));
	}
}

#endif // NEED_vq_exp_rotation1

#ifndef FIXED_POINT
#define OVERRIDE_renormalise_vector
#define renormalise_vector(X, N, gain, arch) \
 (renormalise_vector_altivec(X, N, gain, arch))
#endif


#endif

#endif
