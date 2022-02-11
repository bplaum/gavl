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

#include <accel.h>

#ifdef NOCLIP
#define RECLIP_H(a,idx)
#define RECLIP_V(a,idx)
#define RECLIP_FLOAT(a, idx)
#else

#define RECLIP_H(a,idx) \
  if(GAVL_UNLIKELY(a < ctx->min_values_h[idx])) a = ctx->min_values_h[idx];    \
  if(GAVL_UNLIKELY(a > ctx->max_values_h[idx])) a = ctx->max_values_h[idx]

#define RECLIP_V(a,idx) \
  if(GAVL_UNLIKELY(a < ctx->min_values_v[idx])) a = ctx->min_values_v[idx];    \
  if(GAVL_UNLIKELY(a > ctx->max_values_v[idx])) a = ctx->max_values_v[idx]

#define RECLIP_FLOAT(a, idx) \
  if(GAVL_UNLIKELY(a < ctx->min_values_f[idx])) \
  a = ctx->min_values_f[idx]; \
  if(GAVL_UNLIKELY(a > ctx->max_values_f[idx])) \
  a = ctx->max_values_f[idx]

#endif

/* Downshifting routines, with and without rounding */

#ifdef HQ
#define DOWNSHIFT(a, bits) (((a) + (1<<(bits-1)))>>bits)
#else
#define DOWNSHIFT(a, bits) ((a) >> bits)
#endif

/* Packed formats for 15/16 colors (idea from libvisual) */

typedef struct {
	uint16_t b:5, g:6, r:5;
} color_16;

typedef struct {
	uint16_t b:5, g:5, r:5;
} color_15;

