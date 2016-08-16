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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mathops.h"
#include "vq.h"
#include "pitch.h"

#ifdef OPUS_PPC_MAY_HAVE_ALTIVEC

#include <altivec.h>

#ifndef FIXED_POINT

void renormalise_vector_altivec(celt_norm *X, int N, opus_val16 gain, int arch)
{
	int i;
	opus_val32 E;
	opus_val16 g;
	opus_val32 t;
	celt_norm *xptr;
	E = EPSILON + celt_inner_prod(X, X, N, arch);
	t = VSHR32(E, 2*(k-7));
	g = MULT16_16_P15(celt_rsqrt_norm(t),gain);

	xptr = X;
	for (i=0;i<N;i++)
	{
	   *xptr = EXTRACT16(PSHR32(MULT16_16(g, *xptr), k+1));
	   xptr++;
	}
}

#endif

#endif
