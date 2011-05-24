/*
 * Copyright (c) 2003, 2007-8 Matteo Frigo
 * Copyright (c) 2003, 2007-8 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* This file was automatically generated --- DO NOT EDIT */
/* Generated on Mon Feb  9 19:53:30 EST 2009 */

#include "codelet-dft.h"

#ifdef HAVE_FMA

/* Generated by: ../../../genfft/gen_twidsq_c -fma -reorder-insns -schedule-for-pipeline -simd -compact -variables 4 -pipeline-latency 8 -n 2 -dif -name q1bv_2 -include q1b.h -sign 1 */

/*
 * This function contains 6 FP additions, 4 FP multiplications,
 * (or, 6 additions, 4 multiplications, 0 fused multiply/add),
 * 8 stack variables, 0 constants, and 8 memory accesses
 */
#include "q1b.h"

static void q1bv_2(R *ri, R *ii, const R *W, stride rs, stride vs, INT mb, INT me, INT ms)
{
     INT m;
     R *x;
     x = ii;
     for (m = mb, W = W + (mb * ((TWVL / VL) * 2)); m < me; m = m + VL, x = x + (VL * ms), W = W + (TWVL * 2), MAKE_VOLATILE_STRIDE(rs), MAKE_VOLATILE_STRIDE(vs)) {
	  V T1, T2, T4, T5, T3, T6;
	  T1 = LD(&(x[0]), ms, &(x[0]));
	  T2 = LD(&(x[WS(rs, 1)]), ms, &(x[WS(rs, 1)]));
	  T4 = LD(&(x[WS(vs, 1)]), ms, &(x[WS(vs, 1)]));
	  T5 = LD(&(x[WS(vs, 1) + WS(rs, 1)]), ms, &(x[WS(vs, 1) + WS(rs, 1)]));
	  ST(&(x[0]), VADD(T1, T2), ms, &(x[0]));
	  T3 = BYTW(&(W[0]), VSUB(T1, T2));
	  ST(&(x[WS(rs, 1)]), VADD(T4, T5), ms, &(x[WS(rs, 1)]));
	  T6 = BYTW(&(W[0]), VSUB(T4, T5));
	  ST(&(x[WS(vs, 1)]), T3, ms, &(x[WS(vs, 1)]));
	  ST(&(x[WS(vs, 1) + WS(rs, 1)]), T6, ms, &(x[WS(vs, 1) + WS(rs, 1)]));
     }
}

static const tw_instr twinstr[] = {
     VTW(0, 1),
     {TW_NEXT, VL, 0}
};

static const ct_desc desc = { 2, "q1bv_2", twinstr, &GENUS, {6, 4, 0, 0}, 0, 0, 0 };

void X(codelet_q1bv_2) (planner *p) {
     X(kdft_difsq_register) (p, q1bv_2, &desc);
}
#else				/* HAVE_FMA */

/* Generated by: ../../../genfft/gen_twidsq_c -simd -compact -variables 4 -pipeline-latency 8 -n 2 -dif -name q1bv_2 -include q1b.h -sign 1 */

/*
 * This function contains 6 FP additions, 4 FP multiplications,
 * (or, 6 additions, 4 multiplications, 0 fused multiply/add),
 * 8 stack variables, 0 constants, and 8 memory accesses
 */
#include "q1b.h"

static void q1bv_2(R *ri, R *ii, const R *W, stride rs, stride vs, INT mb, INT me, INT ms)
{
     INT m;
     R *x;
     x = ii;
     for (m = mb, W = W + (mb * ((TWVL / VL) * 2)); m < me; m = m + VL, x = x + (VL * ms), W = W + (TWVL * 2), MAKE_VOLATILE_STRIDE(rs), MAKE_VOLATILE_STRIDE(vs)) {
	  V T1, T2, T3, T4, T5, T6;
	  T1 = LD(&(x[0]), ms, &(x[0]));
	  T2 = LD(&(x[WS(rs, 1)]), ms, &(x[WS(rs, 1)]));
	  T3 = BYTW(&(W[0]), VSUB(T1, T2));
	  T4 = LD(&(x[WS(vs, 1)]), ms, &(x[WS(vs, 1)]));
	  T5 = LD(&(x[WS(vs, 1) + WS(rs, 1)]), ms, &(x[WS(vs, 1) + WS(rs, 1)]));
	  T6 = BYTW(&(W[0]), VSUB(T4, T5));
	  ST(&(x[WS(vs, 1)]), T3, ms, &(x[WS(vs, 1)]));
	  ST(&(x[WS(vs, 1) + WS(rs, 1)]), T6, ms, &(x[WS(vs, 1) + WS(rs, 1)]));
	  ST(&(x[0]), VADD(T1, T2), ms, &(x[0]));
	  ST(&(x[WS(rs, 1)]), VADD(T4, T5), ms, &(x[WS(rs, 1)]));
     }
}

static const tw_instr twinstr[] = {
     VTW(0, 1),
     {TW_NEXT, VL, 0}
};

static const ct_desc desc = { 2, "q1bv_2", twinstr, &GENUS, {6, 4, 0, 0}, 0, 0, 0 };

void X(codelet_q1bv_2) (planner *p) {
     X(kdft_difsq_register) (p, q1bv_2, &desc);
}
#endif				/* HAVE_FMA */
