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
 * which are part of gnucash (http://gnucash.org)
 */

typedef struct {
  uint64_t hi;
  uint64_t lo;
  short isneg;    /**< sign-bit -- T if number is negative */
  short isbig;    /**< sizeflag -- T if number won't fit in signed 64-bit */
} gavl_int128_t;


/** Multiply a pair of signed 64-bit numbers,
 *  returning a signed 128-bit number.
 */
void gavl_int128_mult(int64_t a, int64_t b, gavl_int128_t * ret);

/** Divide a signed 128-bit number by a signed 64-bit,
 *  returning a signed 128-bit number.
 */

void gavl_int128_div(gavl_int128_t * n, int64_t d, gavl_int128_t * ret);


void gavl_int128_copy(gavl_int128_t *dst, gavl_int128_t * src);
