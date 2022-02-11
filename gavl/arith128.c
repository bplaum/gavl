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

/* 128 bit arithmetic routines needed for overflow save
 * 64 bit rescaling functions.
 *
 * Mostly copied and pasted from the qofmath128.[ch] files by
 * Copyright (C) 2004 Linas Vepstas <linas@linas.org>,
 * which are part of gnucash (http://gnucash.org). 
 *
 * Seems the gnucash people are prepared for hyperinflation with
 * their 128 bit arithmetics :)
 */

#include <inttypes.h>
#include <string.h>
#include <arith128.h>

#define HIBIT (0x8000000000000000ULL)

/** Multiply a pair of signed 64-bit numbers,
 *  returning a signed 128-bit number.
 */
void gavl_int128_mult(int64_t a, int64_t b, gavl_int128_t * ret)
  {
  uint64_t a0, a1;
  uint64_t b0, b1;
  uint64_t d, d0, d1;
  uint64_t e, e0, e1;
  uint64_t f, f0, f1;
  uint64_t g, g0, g1;
  uint64_t sum, carry, roll, pmax;

  ret->isneg = 0;
  if (0>a)
  {
    ret->isneg = !ret->isneg;
    a = -a;
  }

  if (0>b)
  {
    ret->isneg = !ret->isneg;
    b = -b;
  }

  a1 = a >> 32;
  a0 = a - (a1<<32);

  b1 = b >> 32;
  b0 = b - (b1<<32);

  d = a0*b0;
  d1 = d >> 32;
  d0 = d - (d1<<32);

  e = a0*b1;
  e1 = e >> 32;
  e0 = e - (e1<<32);

  f = a1*b0;
  f1 = f >> 32;
  f0 = f - (f1<<32);

  g = a1*b1;
  g1 = g >> 32;
  g0 = g - (g1<<32);

  sum = d1+e0+f0;
  carry = 0;
  /* Can't say 1<<32 cause cpp will goof it up; 1ULL<<32 might work */
  roll = 1<<30;
  roll <<= 2;
  pmax = roll-1;
  while (pmax < sum)
  {
    sum -= roll;
    carry ++;
  }

  ret->lo = d0 + (sum<<32);
  ret->hi = carry + e1 + f1 + g0 + (g1<<32);

  ret->isbig = ret->hi || (ret->lo >> 63);
  }

static void shiftleft128(gavl_int128_t * x)
  {
  uint64_t sbit;
  sbit = x->lo & HIBIT;
  x->hi <<= 1;
  x->lo <<= 1;
  x->isbig = 0;
  if (sbit)
  {
    x->hi |= 1;
    x->isbig = 1;
    return;
  }
  if (x->hi)
  {
    x->isbig = 1;
  }
}


/** Divide a signed 128-bit number by a signed 64-bit,
 *  returning a signed 128-bit number.
 */

void gavl_int128_div(gavl_int128_t * n, int64_t d, gavl_int128_t * ret)
  {
  int i;
  uint64_t remainder = 0;

  gavl_int128_copy(ret, n);
  
  if (0 > d)
  {
    d = -d;
    ret->isneg = !ret->isneg;
  }

  /* Use grade-school long division algorithm */
  for (i=0; i<128; i++)
  {
    uint64_t sbit = HIBIT & ret->hi;
    remainder <<= 1;
    if (sbit) remainder |= 1;
    shiftleft128(ret);
    if (remainder >= d)
    {
       remainder -= d;
       ret->lo |= 1;
    }
  }

  /* compute the carry situation */
  ret->isbig = (ret->hi || (ret->lo >> 63));
  
  }

void gavl_int128_copy(gavl_int128_t *dst, gavl_int128_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  }
