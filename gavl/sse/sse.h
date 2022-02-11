/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2012 Members of the Gmerlin project
 * gmerlin-general@lists.sourceforge.net
 * http://gmerlin.sourceforge.net
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * *****************************************************************/

/*
 * SSE* macros
 * Modeled after libmmx
 *
 * SSE macros copied from xine
 * SSE2+ macros gathered from online docs
 * sse_t struct extended for SSE2+ integer routines
 */

#include <stdio.h>

#include <inttypes.h>

typedef	union {
  float  sf[4];	/* Single-precision (32-bit) value */
  double df[2];	/* Double-precision (64-bit) value */

  int64_t  q[2];    /* Quadword (64-bit) value */
  uint64_t uq[2];  /* Unsigned Quadword */
  int32_t  d[4];    /* 2 Doubleword (32-bit) values */
  uint32_t ud[4];   /* 2 Unsigned Doubleword */
  int16_t  w[8];    /* 4 Word (16-bit) values */
  uint16_t uw[8];   /* 4 Unsigned Word */
  int8_t   b[16];   /* 8 Byte (8-bit) values */
  uint8_t  ub[16];  /* 8 Unsigned Byte */
  } ATTR_ALIGN(16) sse_t;

#define	sse_i2r(op, imm, reg) \
	__asm__ __volatile__ (#op " %0, %%" #reg \
			      : /* nothing */ \
			      : "X" (imm) )

#define	sse_m2r(op, mem, reg) \
	__asm__ __volatile__ (#op " %0, %%" #reg \
			      : /* nothing */ \
			      : "m" (mem))

#define	sse_r2m(op, reg, mem) \
	__asm__ __volatile__ (#op " %%" #reg ", %0" \
			      : "=m" (mem) \
			      : /* nothing */ )

#define	sse_r2r(op, regs, regd) \
	__asm__ __volatile__ (#op " %" #regs ", %" #regd)

#define	sse_r2ri(op, regs, regd, imm) \
	__asm__ __volatile__ (#op " %0, %%" #regs ", %%" #regd \
			      : /* nothing */ \
			      : "X" (imm) )

#define	sse_m2ri(op, mem, reg, subop) \
	__asm__ __volatile__ (#op " %0, %%" #reg ", " #subop \
			      : /* nothing */ \
			      : "X" (mem))

/*
  move four aligned packed single-precision floating-point values
  between XMM registers or memory
*/
#define	movaps_m2r(var, reg)	sse_m2r(movaps, var, reg)
#define	movaps_r2m(reg, var)	sse_r2m(movaps, reg, var)
#define	movaps_r2r(regs, regd)	sse_r2r(movaps, regs, regd)

/*
  non-temporal store of four packed single-precision floating-point
  values from an XMM register into memory 
*/

#define	movntps_r2m(xmmreg, var)	sse_r2m(movntps, xmmreg, var)

/*
  move four unaligned packed single-precision floating-point
  values between XMM registers or memory 
 */

#define	movups_m2r(var, reg)	sse_m2r(movups, var, reg)
#define	movups_r2m(reg, var)	sse_r2m(movups, reg, var)
#define	movups_r2r(regs, regd)	sse_r2r(movups, regs, regd)

/*
  move two packed single-precision floating-point values from
  the high quadword of an XMM register to the low quadword of another
  XMM register 
 */

#define	movhlps_r2r(regs, regd)	sse_r2r(movhlps, regs, regd)

/*
  move two packed single-precision floating-point values from the
  low quadword of an XMM register to the high quadword of another
  XMM register 
 */

#define	movlhps_r2r(regs, regd)	sse_r2r(movlhps, regs, regd)

/*
  move two packed single-precision floating-point values to or from
  the high quadword of an XMM register or memory  
 */

#define	movhps_m2r(var, reg)	sse_m2r(movhps, var, reg)
#define	movhps_r2m(reg, var)	sse_r2m(movhps, reg, var)

/*
  move two packed single-precision floating-point values to or from
  the low quadword of an XMM register or memory 
 */

#define	movlps_m2r(var, reg)	sse_m2r(movlps, var, reg)
#define	movlps_r2m(reg, var)	sse_r2m(movlps, reg, var)

/*
  move scalar single-precision floating-point value between XMM
  registers or memory 
*/

#define	movss_m2r(var, reg)	sse_m2r(movss, var, reg)
#define	movss_r2m(reg, var)	sse_r2m(movss, reg, var)
#define	movss_r2r(regs, regd)	sse_r2r(movss, regs, regd)

/*
  shuffles values in packed single-precision floating-point operands 
 */

#define	shufps_m2ri(var, reg, index)	sse_m2ri(shufps, var, reg, index)
#define	shufps_r2ri(regs, regd, index)	sse_r2ri(shufps, regs, regd, index)

/*
  convert packed doubleword integers to packed single-precision
  floating-point values
 */

#define	cvtpi2ps_m2r(var, xmmreg)	sse_m2r(cvtpi2ps, var, xmmreg)
#define	cvtpi2ps_r2r(mmreg, xmmreg)	sse_r2r(cvtpi2ps, mmreg, xmmreg)

/*
  convert packed single-precision floating-point values to packed
  doubleword integers 
 */

#define	cvtps2pi_m2r(var, mmreg)	sse_m2r(cvtps2pi, var, mmreg)
#define	cvtps2pi_r2r(xmmreg, mmreg)	sse_r2r(cvtps2pi, mmreg, xmmreg)

/*
  convert with truncation packed single-precision floating-point
  values to packed doubleword integers 
 */

#define	cvttps2pi_m2r(var, mmreg)	sse_m2r(cvttps2pi, var, mmreg)
#define	cvttps2pi_r2r(xmmreg, mmreg)	sse_r2r(cvttps2pi, mmreg, xmmreg)

/*
  convert doubleword integer to scalar single-precision floating-point value
 */

#define	cvtsi2ss_m2r(var, xmmreg)	sse_m2r(cvtsi2ss, var, xmmreg)
#define	cvtsi2ss_r2r(reg, xmmreg)	sse_r2r(cvtsi2ss, reg, xmmreg)

/*
  convert scalar single-precision floating-point value to a doubleword integer 
 */

#define	cvtss2si_m2r(var, reg)		sse_m2r(cvtss2si, var, reg)
#define	cvtss2si_r2r(xmmreg, reg)	sse_r2r(cvtss2si, xmmreg, reg)

/*
  convert with truncation scalar single-precision floating-point value to
  scalar doubleword integer 
 */

#define	cvttss2si_m2r(var, reg)		sse_m2r(cvtss2si, var, reg)
#define	cvttss2si_r2r(xmmreg, reg)	sse_r2r(cvtss2si, xmmreg, reg)

#define	movmskps(xmmreg, reg) \
	__asm__ __volatile__ ("movmskps %" #xmmreg ", %" #reg)

/*
  add packed single-precision floating-point values 
 */

#define	addps_m2r(var, reg)		sse_m2r(addps, var, reg)
#define	addps_r2r(regs, regd)		sse_r2r(addps, regs, regd)

/*
  add scalar single-precision floating-point values 
 */

#define	addss_m2r(var, reg)		sse_m2r(addss, var, reg)
#define	addss_r2r(regs, regd)		sse_r2r(addss, regs, regd)

/*
  subtract packed single-precision floating-point values 
 */

#define	subps_m2r(var, reg)		sse_m2r(subps, var, reg)
#define	subps_r2r(regs, regd)		sse_r2r(subps, regs, regd)

/*
  subtract scalar single-precision floating-point values 
 */

#define	subss_m2r(var, reg)		sse_m2r(subss, var, reg)
#define	subss_r2r(regs, regd)		sse_r2r(subss, regs, regd)

/*
  multiply packed single-precision floating-point values 
 */

#define	mulps_m2r(var, reg)		sse_m2r(mulps, var, reg)
#define	mulps_r2r(regs, regd)		sse_r2r(mulps, regs, regd)

/*
  multiply scalar single-precision floating-point values 
 */

#define	mulss_m2r(var, reg)		sse_m2r(mulss, var, reg)
#define	mulss_r2r(regs, regd)		sse_r2r(mulss, regs, regd)

/*
  divide packed single-precision floating-point values 
 */

#define	divps_m2r(var, reg)		sse_m2r(divps, var, reg)
#define	divps_r2r(regs, regd)		sse_r2r(divps, regs, regd)

/*
  divide scalar single-precision floating-point values
 */

#define	divss_m2r(var, reg)		sse_m2r(divss, var, reg)
#define	divss_r2r(regs, regd)		sse_r2r(divss, regs, regd)

/*
  compute reciprocals of packed single-precision floating-point values
 */

#define	rcpps_m2r(var, reg)		sse_m2r(rcpps, var, reg)
#define	rcpps_r2r(regs, regd)		sse_r2r(rcpps, regs, regd)

/*
  compute reciprocal of scalar single-precision floating-point values
 */

#define	rcpss_m2r(var, reg)		sse_m2r(rcpss, var, reg)
#define	rcpss_r2r(regs, regd)		sse_r2r(rcpss, regs, regd)

/*
  compute reciprocals of square roots of packed single-precision
  floating-point values 
 */

#define	rsqrtps_m2r(var, reg)		sse_m2r(rsqrtps, var, reg)
#define	rsqrtps_r2r(regs, regd)		sse_r2r(rsqrtps, regs, regd)

/*
  compute reciprocal of square root of scalar single-precision
  floating-point values 
 */

#define	rsqrtss_m2r(var, reg)		sse_m2r(rsqrtss, var, reg)
#define	rsqrtss_r2r(regs, regd)		sse_r2r(rsqrtss, regs, regd)

/*
  compute square roots of packed single-precision floating-point values 
 */

#define	sqrtps_m2r(var, reg)		sse_m2r(sqrtps, var, reg)
#define	sqrtps_r2r(regs, regd)		sse_r2r(sqrtps, regs, regd)

/*
  compute square root of scalar single-precision floating-point values 
 */

#define	sqrtss_m2r(var, reg)		sse_m2r(sqrtss, var, reg)
#define	sqrtss_r2r(regs, regd)		sse_r2r(sqrtss, regs, regd)

/*
  perform bitwise logical AND of packed single-precision floating-point
  values 
 */

#define	andps_m2r(var, reg)		sse_m2r(andps, var, reg)
#define	andps_r2r(regs, regd)		sse_r2r(andps, regs, regd)

/*
  perform bitwise logical AND NOT of packed single-precision
  floating-point values 
 */

#define	andnps_m2r(var, reg)		sse_m2r(andnps, var, reg)
#define	andnps_r2r(regs, regd)		sse_r2r(andnps, regs, regd)

/*
  perform bitwise logical OR of packed single-precision floating-point values 
 */

#define	orps_m2r(var, reg)		sse_m2r(orps, var, reg)
#define	orps_r2r(regs, regd)		sse_r2r(orps, regs, regd)

/*
  perform bitwise logical XOR of packed single-precision floating-point values 
 */

#define	xorps_m2r(var, reg)		sse_m2r(xorps, var, reg)
#define	xorps_r2r(regs, regd)		sse_r2r(xorps, regs, regd)

/*
  return maximum packed single-precision floating-point values 
 */

#define	maxps_m2r(var, reg)		sse_m2r(maxps, var, reg)
#define	maxps_r2r(regs, regd)		sse_r2r(maxps, regs, regd)

/*
  return maximum scalar single-precision floating-point values 
 */

#define	maxss_m2r(var, reg)		sse_m2r(maxss, var, reg)
#define	maxss_r2r(regs, regd)		sse_r2r(maxss, regs, regd)

/*
  return minimum packed single-precision floating-point values 
 */

#define	minps_m2r(var, reg)		sse_m2r(minps, var, reg)
#define	minps_r2r(regs, regd)		sse_r2r(minps, regs, regd)

/*
  return minimum scalar single-precision floating-point values. 
 */

#define	minss_m2r(var, reg)		sse_m2r(minss, var, reg)
#define	minss_r2r(regs, regd)		sse_r2r(minss, regs, regd)

/*
  compare packed single-precision floating-point values 
 */

#define	cmpps_m2r(var, reg, op)		sse_m2ri(cmpps, var, reg, op)
#define	cmpps_r2r(regs, regd, op)	sse_r2ri(cmpps, regs, regd, op)

#define	cmpeqps_m2r(var, reg)		sse_m2ri(cmpps, var, reg, 0)
#define	cmpeqps_r2r(regs, regd)		sse_r2ri(cmpps, regs, regd, 0)

#define	cmpltps_m2r(var, reg)		sse_m2ri(cmpps, var, reg, 1)
#define	cmpltps_r2r(regs, regd)		sse_r2ri(cmpps, regs, regd, 1)

#define	cmpleps_m2r(var, reg)		sse_m2ri(cmpps, var, reg, 2)
#define	cmpleps_r2r(regs, regd)		sse_r2ri(cmpps, regs, regd, 2)

#define	cmpunordps_m2r(var, reg)	sse_m2ri(cmpps, var, reg, 3)
#define	cmpunordps_r2r(regs, regd)	sse_r2ri(cmpps, regs, regd, 3)

#define	cmpneqps_m2r(var, reg)		sse_m2ri(cmpps, var, reg, 4)
#define	cmpneqps_r2r(regs, regd)	sse_r2ri(cmpps, regs, regd, 4)

#define	cmpnltps_m2r(var, reg)		sse_m2ri(cmpps, var, reg, 5)
#define	cmpnltps_r2r(regs, regd)	sse_r2ri(cmpps, regs, regd, 5)

#define	cmpnleps_m2r(var, reg)		sse_m2ri(cmpps, var, reg, 6)
#define	cmpnleps_r2r(regs, regd)	sse_r2ri(cmpps, regs, regd, 6)

#define	cmpordps_m2r(var, reg)		sse_m2ri(cmpps, var, reg, 7)
#define	cmpordps_r2r(regs, regd)	sse_r2ri(cmpps, regs, regd, 7)

/*
  Compare low single-precision floating-point value from xmm2/m32 with
  low single-precision floating-point value in xmm1 register using imm8
  as comparison predicate.
 */

#define	cmpss_m2r(var, reg, op)		sse_m2ri(cmpss, var, reg, op)
#define	cmpss_r2r(regs, regd, op)	sse_r2ri(cmpss, regs, regd, op)

#define	cmpeqss_m2r(var, reg)		sse_m2ri(cmpss, var, reg, 0)
#define	cmpeqss_r2r(regs, regd)		sse_r2ri(cmpss, regs, regd, 0)

#define	cmpltss_m2r(var, reg)		sse_m2ri(cmpss, var, reg, 1)
#define	cmpltss_r2r(regs, regd)		sse_r2ri(cmpss, regs, regd, 1)

#define	cmpless_m2r(var, reg)		sse_m2ri(cmpss, var, reg, 2)
#define	cmpless_r2r(regs, regd)		sse_r2ri(cmpss, regs, regd, 2)

#define	cmpunordss_m2r(var, reg)	sse_m2ri(cmpss, var, reg, 3)
#define	cmpunordss_r2r(regs, regd)	sse_r2ri(cmpss, regs, regd, 3)

#define	cmpneqss_m2r(var, reg)		sse_m2ri(cmpss, var, reg, 4)
#define	cmpneqss_r2r(regs, regd)	sse_r2ri(cmpss, regs, regd, 4)

#define	cmpnltss_m2r(var, reg)		sse_m2ri(cmpss, var, reg, 5)
#define	cmpnltss_r2r(regs, regd)	sse_r2ri(cmpss, regs, regd, 5)

#define	cmpnless_m2r(var, reg)		sse_m2ri(cmpss, var, reg, 6)
#define	cmpnless_r2r(regs, regd)	sse_r2ri(cmpss, regs, regd, 6)

#define	cmpordss_m2r(var, reg)		sse_m2ri(cmpss, var, reg, 7)
#define	cmpordss_r2r(regs, regd)	sse_r2ri(cmpss, regs, regd, 7)

/*
  perform ordered comparison of scalar single-precision floating-point
  values and set flags in EFLAGS register 
 */

#define	comiss_m2r(var, reg)		sse_m2r(comiss, var, reg)
#define	comiss_r2r(regs, regd)		sse_r2r(comiss, regs, regd)

/*
  perform unordered comparison of scalar single-precision floating-point
  values and set flags in EFLAGS register 
 */

#define	ucomiss_m2r(var, reg)		sse_m2r(ucomiss, var, reg)
#define	ucomiss_r2r(regs, regd)		sse_r2r(ucomiss, regs, regd)

/*
  unpacks and interleaves the two low-order values from two
  single-precision floating-point operands 
 */

#define	unpcklps_m2r(var, reg)		sse_m2r(unpcklps, var, reg)
#define	unpcklps_r2r(regs, regd)	sse_r2r(unpcklps, regs, regd)

/*
  unpacks and interleaves the two high-order values from two
  single-precision floating-point operands 
 */

#define	unpckhps_m2r(var, reg)		sse_m2r(unpckhps, var, reg)
#define	unpckhps_r2r(regs, regd)	sse_r2r(unpckhps, regs, regd)

/*
  
*/

#define	fxrstor(mem) \
	__asm__ __volatile__ ("fxrstor %0" \
			      : /* nothing */ \
			      : "X" (mem))

/*

*/

#define	fxsave(mem) \
	__asm__ __volatile__ ("fxsave %0" \
			      : /* nothing */ \
			      : "X" (mem))

/*
  save %mxcsr register state
 */

#define	stmxcsr(mem) \
	__asm__ __volatile__ ("stmxcsr %0" \
			      : /* nothing */ \
			      : "X" (mem))

/*
  load %mxcsr register
 */

#define	ldmxcsr(mem) \
	__asm__ __volatile__ ("ldmxcsr %0" \
			      : /* nothing */ \
			      : "X" (mem))

/*
 *  SSE2
 */

/*
 * SSE2 Data Movement Instructions
 */

/*
  Move two aligned packed double-precision floating-point values
  between XMM registers and memory
 */

#define movapd_m2r(var, reg) sse_m2r(movapd, var, reg)
#define movapd_r2m(reg, var)    sse_r2m(movapd, reg, var)
#define movapd_r2r(regs, regd)  sse_r2r(movapd, regs, regd)


/*
  move high packed double-precision floating-point value to or from
  the high quadword of an XMM register and memory 
 */

#define movhpd_m2r(var, reg)    sse_m2r(movhpd, var, reg)
#define movhpd_r2m(reg, var)    sse_r2m(movhpd, reg, var)
#define movhpd_r2r(regs, regd)  sse_r2r(movhpd, regs, regd)


/*
  move low packed single-precision floating-point value to or from
  the low quadword of an XMM register and memory 
 */

#define movlpd_m2r(var, reg)    sse_m2r(movlpd, var, reg)
#define movlpd_r2m(reg, var)    sse_r2m(movlpd, reg, var)
#define movlpd_r2r(regs, regd)  sse_r2r(movlpd, regs, regd)
 	
/*
  extract sign mask from two packed double-precision floating-point
  values 
 */

#define movmskpd_r2r(regs, regd)    sse_m2r(movmskpd, regs, regd)

/*
  move scalar double-precision floating-point value between XMM registers
  and memory. 
 */

#define movsd_m2r(var, reg)    sse_m2r(movsd, var, reg)
#define movsd_r2m(reg, var)    sse_r2m(movsd, reg, var)
#define movsd_r2r(regs, regd)  sse_r2r(movsd, regs, regd)

/*
  move two unaligned packed double-precision floating-point values
  between XMM registers and memory 
 */
 	
#define movupd_m2r(var, reg)    sse_m2r(movupd, var, reg)
#define movupd_r2m(reg, var)    sse_r2m(movupd, reg, var)
#define movupd_r2r(regs, regd)  sse_r2r(movupd, regs, regd)

/*
 * SSE2 Packed Arithmetic Instructions
 */

/*
  add packed double-precision floating-point values 
 */

#define addpd_m2r(var, reg) sse_m2r(addpd, var, reg)
#define addpd_r2r(regs, regd) sse_r2r(addpd, regs, regd)

/*
  add scalar double-precision floating-point values
 */
 	
#define addsd_m2r(var, reg) sse_m2r(addsd, var, reg)
#define addsd_r2r(regs, regd) sse_r2r(addsd, regs, regd)

/*
  divide packed double-precision floating-point values 
 */

#define divpd_m2r(var, reg) sse_m2r(divpd, var, reg)
#define divpd_r2r(regs, regd) sse_r2r(divpd, regs, regd)

/*
  divide scalar double-precision floating-point values
 */

#define divsd_m2r(var, reg) sse_m2r(divsd, var, reg)
#define divsd_r2r(regs, regd) sse_r2r(divsd, regs, regd)

/*
  multiply packed double-precision floating-point values 
 */

#define mulpd_m2r(var, reg) sse_m2r(mulpd, var, reg)
#define mulpd_r2r(regs, regd) sse_r2r(mulpd, regs, regd)

/*
  multiply scalar double-precision floating-point values
 */

#define mulsd_m2r(var, reg) sse_m2r(mulsd, var, reg)
#define mulsd_r2r(regs, regd) sse_r2r(mulsd, regs, regd)

/*
  subtract packed double-precision floating-point values 
 */

#define subpd_m2r(var, reg) sse_m2r(subpd, var, reg)
#define subpd_r2r(regs, regd) sse_r2r(subpd, regs, regd)

/*
  subtract scalar double-precision floating-point values
 */

#define subsd_m2r(var, reg) sse_m2r(subsd, var, reg)
#define subsd_r2r(regs, regd) sse_r2r(subsd, regs, regd)

/*
  return maximum packed double-precision floating-point
  values
 */

#define maxpd_m2r(var, reg) sse_m2r(maxpd, var, reg)
#define maxpd_r2r(regs, regd) sse_r2r(maxpd, regs, regd)

/*
  return maximum scalar double-precision floating-point
  value 
 */

#define maxsd_m2r(var, reg) sse_m2r(maxsd, var, reg)
#define maxsd_r2r(regs, regd) sse_r2r(maxsd, regs, regd)

/*
  return minimum packed double-precision floating-point
  values 
 */

#define minpd_m2r(var, reg) sse_m2r(minpd, var, reg)
#define minpd_r2r(regs, regd) sse_r2r(minpd, regs, regd)

/*
  return minimum scalar double-precision floating-point
  value 
 */

#define minsd_m2r(var, reg) sse_m2r(minsd, var, reg)
#define minsd_r2r(regs, regd) sse_r2r(minsd, regs, regd)

/*
  compute packed square roots of packed double-precision
  floating-point values 
 */

#define sqrtpd_m2r(var, reg) sse_m2r(sqrtpd, var, reg)
#define sqrtpd_r2r(regs, regd) sse_r2r(sqrtpd, regs, regd)

/*
  compute scalar square root of scalar double-precision
  floating-point value 
 */

#define sqrtsd_m2r(var, reg) sse_m2r(sqrtsd, var, reg)
#define sqrtsd_r2r(regs, regd) sse_r2r(sqrtsd, regs, regd)

/*
 * SSE2 Logical Instructions
 */

/*
  perform bitwise logical AND NOT of packed double-precision
  floating-point values
 */

#define andnpd_m2r(var, reg) sse_m2r(andnpd, var, reg)
#define andnpd_r2r(regs, regd) sse_r2r(andnpd, regs, regd)

/*
  perform bitwise logical AND of packed double-precision
  floating-point values
 */

#define andpd_m2r(var, reg) sse_m2r(andpd, var, reg)
#define andpd_r2r(regs, regd) sse_r2r(andpd, regs, regd)

/*
  perform bitwise logical OR of packed double-precision
  floating-point values 
 */

#define orpd_m2r(var, reg) sse_m2r(orpd, var, reg)
#define orpd_r2r(regs, regd) sse_r2r(orpd, regs, regd)

/*
  perform bitwise logical XOR of packed double-precision
  floating-point values 
 */

#define xorpd_m2r(var, reg) sse_m2r(xorpd, var, reg)
#define xorpd_r2r(regs, regd) sse_r2r(xorpd, regs, regd)

/*
 * SSE2 Compare Instructions
 */

/*
  compare packed double-precision floating-point values 
 */

#define cmppd_m2r(var, reg) sse_m2r(cmppd, var, reg)
#define cmppd_r2r(regs, regd) sse_r2r(cmppd, regs, regd)

/*
  perform ordered comparison of scalar double-precision
  floating-point values and set flags in EFLAGS register 
 */

#define comisd_m2r(var, reg) sse_m2r(comisd, var, reg)
#define comisd_r2r(regs, regd) sse_r2r(comisd, regs, regd)

/*
  perform unordered comparison of scalar double-precision
  floating-point values and set flags in EFLAGS register
 */

#define ucomisd_m2r(var, reg) sse_m2r(ucomisd, var, reg)
#define ucomisd_r2r(regs, regd) sse_r2r(ucomisd, regs, regd)

/*
 * SSE2 Shuffle and Unpack Instructions
 */

/*
  shuffle values in packed double-precision floating-point
  operands
 */

#define shufpd_m2r(var, reg) sse_m2r(shufpd, var, reg)
#define shufpd_r2r(regs, regd) sse_r2r(shufpd, regs, regd)

/*
  unpack and interleave the high values from two packed
  double-precision floating-point operands
 */

#define unpckhpd_m2r(var, reg) sse_m2r(unpckhpd, var, reg)
#define unpckhpd_r2r(regs, regd) sse_r2r(unpckhpd, regs, regd)

/*
  unpack and interleave the low values from two packed
  double-precision floating-point operands
 */

#define unpcklpd_m2r(var, reg) sse_m2r(unpcklpd, var, reg)
#define unpcklpd_r2r(regs, regd) sse_r2r(unpcklpd, regs, regd)

/*
 * SSE2 Conversion Instructions
 */

/*
  convert packed doubleword integers to packed double-precision
  floating-point values
 */

#define cvtdq2pd_m2r(var, reg) sse_m2r(cvtdq2pd, var, reg)
#define cvtdq2pd_r2r(regs, regd) sse_r2r(cvtdq2pd, regs, regd)

/*
  convert packed double-precision floating-point values to packed
  doubleword integers 
 */

#define cvtpd2dq_m2r(var, reg) sse_m2r(cvtpd2dq, var, reg)
#define cvtpd2dq_r2r(regs, regd) sse_r2r(cvtpd2dq, regs, regd)

/*
  convert packed double-precision floating-point values to packed
  doubleword integers 
 */

#define cvtpd2pi_m2r(var, reg) sse_m2r(cvtpd2pi, var, reg)
#define cvtpd2pi_r2r(regs, regd) sse_r2r(cvtpd2pi, regs, regd)

/*
  convert packed double-precision floating-point values to packed
  single-precision floating-point values
 */

#define cvtpd2ps_m2r(var, reg) sse_m2r(cvtpd2ps, var, reg)
#define cvtpd2ps_r2r(regs, regd) sse_r2r(cvtpd2ps, regs, regd)

/*
  convert packed doubleword integers to packed double-precision
  floating-point values 
 */

#define cvtpi2pd_m2r(var, reg) sse_m2r(cvtpi2pd, var, reg)
#define cvtpi2pd_r2r(regs, regd) sse_r2r(cvtpi2pd, regs, regd)

/*
  convert packed single-precision floating-point values to
  packed double-precision floating-point values 
 */

#define cvtps2pd_m2r(var, reg) sse_m2r(cvtps2pd, var, reg)
#define cvtps2pd_r2r(regs, regd) sse_r2r(cvtps2pd, regs, regd)

/*
  convert scalar double-precision floating-point values to a
  doubleword integer 
 */

#define cvtsd2si_m2r(var, reg) sse_m2r(cvtsd2si, var, reg)
#define cvtsd2si_r2r(regs, regd) sse_r2r(cvtsd2si, regs, regd)

/*
  convert scalar double-precision floating-point values to scalar
  single-precision floating-point values 
 */

#define cvtsd2ss_m2r(var, reg) sse_m2r(cvtsd2ss, var, reg)
#define cvtsd2ss_r2r(regs, regd) sse_r2r(cvtsd2ss, regs, regd)

/*
  convert doubleword integer to scalar double-precision
  floating-point value 
 */

#define cvtsi2sd_m2r(var, reg) sse_m2r(cvtsi2sd, var, reg)
#define cvtsi2sd_r2r(regs, regd) sse_r2r(cvtsi2sd, regs, regd)

/*
  convert scalar single-precision floating-point values to
  scalar double-precision floating-point values 
 */

#define cvtss2sd_m2r(var, reg) sse_m2r(cvtss2sd, var, reg)
#define cvtss2sd_r2r(regs, regd) sse_r2r(cvtss2sd, regs, regd)

/*
  convert with truncation packed double-precision floating-point
  values to packed doubleword integers
 */

#define cvttpd2dq_m2r(var, reg) sse_m2r(cvttpd2dq, var, reg)
#define cvttpd2dq_r2r(regs, regd) sse_r2r(cvttpd2dq, regs, regd)

/*
  convert with truncation packed double-precision floating-point
  values to packed doubleword integers 
 */

#define cvttpd2pi_m2r(var, reg) sse_m2r(cvttpd2pi, var, reg)
#define cvttpd2pi_r2r(regs, regd) sse_r2r(cvttpd2pi, regs, regd)

/*
  convert with truncation scalar double-precision floating-point
  values to scalar doubleword integers 
 */

#define cvttsd2si_m2r(var, reg) sse_m2r(cvttsd2si, var, reg)
#define cvttsd2si_r2r(regs, regd) sse_r2r(cvttsd2si, regs, regd)

/*
 * SSE2 Packed Single-Precision Floating-Point Instructions
 */

/*
  convert packed doubleword integers to packed single-precision
  floating-point values 
 */

#define cvtdq2ps_m2r(var, reg) sse_m2r(cvtdq2ps, var, reg)
#define cvtdq2ps_r2r(regs, regd) sse_r2r(cvtdq2ps, regs, regd)

/*
  convert packed single-precision floating-point values to packed
  doubleword integers 
 */

#define cvtps2dq_m2r(var, reg) sse_m2r(cvtps2dq, var, reg)
#define cvtps2dq_r2r(regs, regd) sse_r2r(cvtps2dq, regs, regd)

/*
  convert with truncation packed single-precision floating-point
  values to packed doubleword integers 
 */

#define cvttps2dq_m2r(var, reg) sse_m2r(cvttps2dq, var, reg)
#define cvttps2dq_r2r(regs, regd) sse_r2r(cvttps2dq, regs, regd)

/*
 * SSE2 128-Bit SIMD Integer Instructions
 */

/*
  move quadword integer from XMM to MMX registers 
 */

#define movdq2q_r2r(regs, regd)  sse_r2r(movdq2q, regs, regd)

/*
  move quadword integer from MMX to XMM registers 
 */

#define movq2dq_r2r(regs, regd)  sse_r2r(movq2dq, regs, regd)

/*
  move aligned double quadword 
 */

#define movdqa_m2r(var, reg) sse_m2r(movdqa, var, reg)
#define movdqa_r2m(reg, var) sse_r2m(movdqa, reg, var)
#define movdqa_r2r(regs, regd)  sse_r2r(movdqa, regs, regd)

/*
  move unaligned double quadword 
 */

#define movdqu_m2r(var, reg) sse_m2r(movdqu, var, reg)
#define movdqu_r2m(reg, var) sse_r2m(movdqu, reg, var)

/*
  add packed quadword integers 
*/

#define paddq_m2r(var, reg) sse_m2r(paddq, var, reg)
#define paddq_r2r(regs, regd) sse_r2r(paddq, regs, regd)

/*
  multiply packed unsigned doubleword integers 
 */

#define pmuludq_m2r(var, reg) sse_m2r(pmuldq, var, reg)
#define pmuludq_r2r(regs, regd) sse_r2r(pmuldq, regs, regd)

/*
  shuffle packed doublewords 
 */

#define pshufd_m2ri(var, reg, imm) sse_m2ri(pshufd, var, reg, imm)
#define pshufd_r2ri(reg, var, imm) sse_r2ri(pshufd, reg, var, imm)

/*
  shuffle packed high words 
 */

#define pshufhw_m2ri(var, reg, imm) sse_m2ri(pshufhw, var, reg, imm)
#define pshufhw_r2ri(regs, regd, imm) sse_r2ri(pshufhw, regs, regd, imm)
 	
/*
  shuffle packed low words 
 */

#define pshuflw_m2ri(var, reg, imm) sse_m2ri(pshuflw, var, reg, imm)
#define pshuflw_r2ri(regs, regd, imm) sse_r2ri(pshuflw, regs, regd, imm)

/*
  shift double quadword left logical
 */

#define pslldq_i2r(imm, reg) sse_i2r(pslldq, imm, reg)

/*
  shift double quadword right logical
 */

#define psrldq_i2r(imm, reg) sse_i2r(psrldq, imm, reg)

/*
  subtract packed quadword integers
 */

#define psubq_m2r(var, reg) sse_m2r(psubq, var, reg)
#define psubq_r2r(regs, regd) sse_r2r(psubq, regs, regd)


/*
  unpack high quadwords
 */

#define punpckhqdq_m2r(var, reg) sse_m2r(punpckhqdq, var, reg)
#define punpckhqdq_r2r(regs, regd) sse_r2r(punpckhqdq, regs, regd)


/*
  unpack low quadwords
 */

#define punpcklqdq_m2r(var, reg) sse_m2r(punpcklqdq, var, reg)
#define punpcklqdq_r2r(regs, regd) sse_r2r(punpcklqdq, regs, regd)

/*
 *  SSE3 instructions
 */

/*
 *  SSE3 SIMD Floating-Point Instructions
 */

/*
  Add/Subtract packed DP FP numbers
 */

#define addsubpd_m2r(var, reg) sse_m2r(addsubpd, var, reg)
#define addsubpd_r2r(regs, regd) sse_r2r(addsubpd, regs, regd)

/*
  Add/Subtract packed SP FP numbers
 */

#define addsubps_m2r(var, reg) sse_m2r(addsubps, var, reg)
#define addsubps_r2r(regs, regd) sse_r2r(addsubps, regs, regd)

/*
  Add horizontally packed DP FP numbers
 */

#define haddpd_m2r(var, reg) sse_m2r(haddpd, var, reg)
#define haddpd_r2r(regs, regd) sse_r2r(haddpd, regs, regd)

/*
  Add horizontally packed SP FP numbers
 */

#define haddps_m2r(var, reg) sse_m2r(haddps, var, reg)
#define haddps_r2r(regs, regd) sse_r2r(haddps, regs, regd)

/*
  Subtract horizontally packed DP FP numbers
 */

#define hsubpd_m2r(var, reg) sse_m2r(hsubpd, var, reg)
#define hsubpd_r2r(regs, regd) sse_r2r(hsubpd, regs, regd)

/*
  Subtract horizontally packed SP FP numbers
 */

#define hsubps_m2r(var, reg) sse_m2r(hsubps, var, reg)
#define hsubps_r2r(regs, regd) sse_r2r(hsubps, regs, regd)

/*
 *  SSE3 SIMD Integer Instructions
 */

/*
  Move 64 bits representing the lower DP data element from
  XMM2/Mem to XMM1 register and duplicate.
 */


#define movddup_m2r(var, reg) sse_m2r(movddup, var, reg)
#define movddup_r2r(regs, regd) sse_r2r(movddup, regs, regd)

/*
  Move 128 bits representing packed SP data elements from
  XMM2/Mem to XMM1 register and duplicate high.  
 */

#define movshdup_m2r(var, reg) sse_m2r(movshdup, var, reg)
#define movshdup_r2r(regs, regd) sse_r2r(movshdup, regs, regd)

/*
  Move 128 bits representing packed SP data elements from
  XMM2/Mem to XMM1 register and duplicate low.
 */

#define movsldup_m2r(var, reg) sse_m2r(movsldup, var, reg)
#define movsldup_r2r(regs, regd) sse_r2r(movsldup, regs, regd)

/*
  Load 128 bits from Mem to XMM register.
 */

#define lddqu_m2r(var, reg) sse_m2r(lddqu, var, reg)

/*
 *  SSSE3
 */

/*
  Negate the elements of a register of bytes, words or dwords
  if the sign of the corresponding elements of another register is
  negative.
 */
  
#define psignw_m2r(var, reg)   sse_m2r(psignw, var, reg)
#define psignw_r2r(regs, regd) sse_r2r(psignw, regs, regd)

#define psignd_m2r(var, reg)   sse_m2r(psignd, var, reg)
#define psignd_r2r(regs, regd) sse_r2r(psignd, regs, regd)

#define psignb_m2r(var, reg)   sse_m2r(psignb, var, reg)
#define psignb_r2r(regs, regd) sse_r2r(psignb, regs, regd)

/*
  Fill the elements of a register of bytes, words or dwords with
  the absolute values of the elements of another register
 */
  
#define pabsw_m2r(var, reg)    sse_m2r(pabsw, var, reg)
#define pabsw_r2r(regs, regd)  sse_r2r(pabsw, regs, regd)

#define pabsd_m2r(var, reg)    sse_m2r(pabsd, var, reg)
#define pabsd_r2r(regs, regd)  sse_r2r(pabsd, regs, regd)

#define pabsb_m2r(var, reg)    sse_m2r(pabsb, var, reg)
#define pabsb_r2r(regs, regd)  sse_r2r(pabsb, regs, regd)

/*
  takes registers of bytes A = [a0 a1 a2 ...] and B = [b0 b1 b2 ...]
  and replaces A with [ab0 ab1 ab2 ...]; except that it replaces the
  ith entry with 0 if the top bit of bi is set.
 */
  
#define pshufb_m2r(var, reg)   sse_m2r(pshufb, var, reg)
#define pshufb_r2r(regs, regd) sse_r2r(pshufb, regs, regd)

/*
  takes registers A = [a0 a1 a2 ...] and B = [b0 b1 b2 ...]
  and outputs [a0-a1 a2-a3 ... b0-b1 b2-b3 ...]
 */
  
#define phsubw_m2r(var, reg)   sse_m2r(phsubw, var, reg)
#define phsubw_r2r(regs, regd) sse_r2r(phsubw, regs, regd)
  
#define phsubd_m2r(var, reg)   sse_m2r(phsubd, var, reg)
#define phsubd_r2r(regs, regd) sse_r2r(phsubd, regs, regd)

/*
  like PHSUBW, but outputs [satsw(a0-a1) satsw(a2-a3) ... satsw(b0-b1)
  satsw(b2-b3) ...]
 */
  
#define phsubsw_m2r(var, reg)   sse_m2r(phsubsw, var, reg)
#define phsubsw_r2r(regs, regd) sse_r2r(phsubsw, regs, regd)

/*
  takes registers A = [a0 a1 a2 ...] and B = [b0 b1 b2 ...]
  and outputs [a0+a1 a2+a3 ... b0+b1 b2+b3 ...]
 */
  
#define phaddw_m2r(var, reg)   sse_m2r(phaddw, var, reg)
#define phaddw_r2r(regs, regd) sse_r2r(phaddw, regs, regd)

#define phaddd_m2r(var, reg)   sse_m2r(phaddd, var, reg)
#define phaddd_r2r(regs, regd) sse_r2r(phaddd, regs, regd)

/*
  like PHADDW, but outputs [satsw(a0+a1) satsw(a2+a3) ... satsw(b0+b1)
  satsw(b2+b3) ...]
 */

#define phaddsw_m2r(var, reg)   sse_m2r(phaddsw, var, reg)
#define phaddsw_r2r(regs, regd) sse_r2r(phaddsw, regs, regd)
  
/*
  treat the sixteen-bit words in registers A and B as signed
  15-bit fixed-point numbers between -1 and 1 (eg 0x4000 is
  treated as 0.5 and 0xa000 as -0.75), and multiply them together
  with correct rounding.
 */
  
#define pmulhrsw_m2r(var, reg)   sse_m2r(pmulhrsw, var, reg)
#define pmulhrsw_r2r(regs, regd) sse_r2r(pmulhrsw, regs, regd)

/*
  Take the bytes in registers A and B, multiply them together, add
  pairs, signed-saturate and store. IE [a0 a1 a2 ...] pmaddubsw
  [b0 b1 b2 ...] = [satsw(a0b0+a1b1) satsw(a2b2+a3b3) ...]
 */
  
#define pmaddubsw_m2r(var, reg)   sse_m2r(pmaddubsw, var, reg)
#define pmaddubsw_r2r(regs, regd) sse_r2r(pmaddubsw, regs, regd)

/*
  take two registers, concatenate their values, and pull out a
  register-length section from an offset given by an immediate
  value encoded in the instruction.
 */
  
#define palignr_r2ri(regs, regd, imm) sse_r2ri(palignr, regs, regd, imm)




#ifdef SSE_DEBUG
static sse_t debug_sse;
#define sse_debug(r) movaps_r2m(r, debug_sse.ub); \
   fprintf(stderr, "%s: %f %f %f %f\n", #r, \
           debug_sse.sf[0], \
           debug_sse.sf[1], \
           debug_sse.sf[2], \
           debug_sse.sf[3]);
#endif

