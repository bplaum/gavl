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
#include <gavl/gavl.h>
#include <video.h>
#include <colorspace.h>

#include <attributes.h>
//#define SSE_DEBUG
#include "sse.h"
#include "../mmx/mmx.h"

#include <stdio.h>


#include "../c/colorspace_macros.h"

static const sse_t rgb_2_y = { .sf = { r_float_to_y, g_float_to_y, b_float_to_y, 0.0 } };
static const sse_t rgb_2_u = { .sf = { r_float_to_u, g_float_to_u, b_float_to_u, 0.0 } };
static const sse_t rgb_2_v = { .sf = { r_float_to_v, g_float_to_v, b_float_to_v, 0.0 } };

#define RGB_FLOAT_TO_YUV_FLOAT_SSE \
    /* Y */ \
    movaps_r2r(xmm3, xmm4);\
    mulps_r2r(xmm0,  xmm4); /* xmm4: Ya Yb Yc 0.0 */\
    /* U */ \
    movaps_r2r(xmm3, xmm5); \
    mulps_r2r(xmm1,  xmm5); /* xmm5: Ua Ub Uc 0.0 */\
    /* V */ \
    mulps_r2r(xmm2,  xmm3); /* xmm3: Va Vb Vc 0.0 */

#ifdef SSE3

#define INIT_RGB_YUV   \
   movaps_m2r(rgb_2_y, xmm0); \
   movaps_m2r(rgb_2_u, xmm1); \
   movaps_m2r(rgb_2_v, xmm2);\
   pxor_r2r(xmm6, xmm6)

#define OUTPUT_YUVA \
   haddps_r2r(xmm5, xmm4); /* xmm4: Ya+Yb Yb Ua+Ub Uc */\
   haddps_r2r(xmm6, xmm3); /* xmm5: Va+Vb Vc 0.0   0.0  */\
   haddps_r2r(xmm3, xmm4); /* xmm4: y     u  v     0.0  */\
   movaps_r2m(xmm4, *dst);

#else

#define INIT_RGB_YUV   \
   sse_t tmp; \
   movaps_m2r(rgb_2_y, xmm0); \
   movaps_m2r(rgb_2_u, xmm1); \
   movaps_m2r(rgb_2_v, xmm2)

#define OUTPUT_YUVA \
    movaps_r2m(xmm4, tmp);\
    dst[0] = tmp.sf[0] + tmp.sf[1] + tmp.sf[2]; \
    movaps_r2m(xmm5, tmp);\
    dst[1] = tmp.sf[0] + tmp.sf[1] + tmp.sf[2]; \
    movaps_r2m(xmm3, tmp);\
    dst[2] = tmp.sf[0] + tmp.sf[1] + tmp.sf[2];

#endif


/* rgba_float_to_yuva_float_c */

#define FUNC_NAME   rgba_float_to_yuva_float_sse
#define IN_TYPE     float
#define OUT_TYPE    float
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1

#define INIT INIT_RGB_YUV;

#define CONVERT                                                    \
    /* We can assume, that src and dst are always aligned */\
    movaps_m2r(*src, xmm3);\
    RGB_FLOAT_TO_YUV_FLOAT_SSE; \
    OUTPUT_YUVA\
    dst[3] = src[3];

#include "../csp_packed_packed.h"

#ifdef SSE3
void gavl_init_rgb_yuv_funcs_sse3(gavl_pixelformat_function_table_t * tab, const gavl_video_options_t * opt)
  {
  tab->rgba_float_to_yuva_float = rgba_float_to_yuva_float_sse;
  //  tab->rgb_float_to_yuva_float = rgb_float_to_yuva_float_sse;
  }
#endif
