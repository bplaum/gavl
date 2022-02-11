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

#include <config.h>
#include <gavl.h>
#include <video.h>
#include <colorspace.h>

#include <attributes.h>
#include "mmx.h"

/*
 *  Support for mmxext
 *  this macro procudes another set of
 *  functions in ../mmxext
 *  I really wonder if this is the only difference between mmx and mmxext
 */

#ifdef MMXEXT
// #define MOVQ_R2M(reg,mem) movntq_r2m(reg, mem)
#define MOVQ_R2M(reg,mem) movq_r2m(reg, mem)
#define CLEANUP     emms();
// #define PREFETCH(ptr) mmx_fetch(ptr,t0)
#define PREFETCH(ptr)
#else
#define MOVQ_R2M(reg,mem) movq_r2m(reg, mem)
#define CLEANUP     emms();
#define PREFETCH(ptr)
#endif

#include "_rgb_rgb_mmx.c"
#include "_rgb_yuv_mmx.c"
#include "_yuv_rgb_mmx.c"
#include "_yuv_yuv_mmx.c"
