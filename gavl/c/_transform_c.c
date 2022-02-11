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

// #include <stdio.h>
#include <gavl/gavl.h>
#include <video.h>
#include <transform.h>
#include "scale_macros.h"

#define TMP_TYPE_8 int
#define TMP_TYPE_16 uint32_t
/* transform_rgb_15_c */

#define FUNC_NAME transform_rgb_15_c
#define TYPE color_15
#define INIT TMP_TYPE_8 tmp;

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].r;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->r = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].g;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->g = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].b;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->b = tmp;

#elif NUM_TAPS == 3
#define TRANSFORM \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2].r;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->r = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2].g;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->g = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2].b;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->b = tmp;
#elif NUM_TAPS == 4
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2].r +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[3].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[3].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[3].r +  \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[2].r +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[3].r;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->r = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2].g +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[3].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[3].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[3].g +  \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[2].g +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[3].g; \
  tmp=DOWNSHIFT(tmp,16);\
  dst->g = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2].b +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[3].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[3].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[3].b +  \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[2].b +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[3].b;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->b = tmp;
#endif

#include "transform_c.h"

/* transform_rgb_16_c */

#define FUNC_NAME transform_rgb_16_c
#define TYPE color_16
#define INIT TMP_TYPE_8 tmp;

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].r;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->r = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].g;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->g = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].b;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->b = tmp;

#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2].r;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->r = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2].g;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->g = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2].b;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->b = tmp;


#elif NUM_TAPS == 4
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2].r +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[3].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2].r +  \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[3].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2].r +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[3].r +  \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[0].r +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[1].r +  \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[2].r +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[3].r;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->r = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2].g +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[3].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2].g +  \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[3].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2].g +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[3].g +  \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[0].g +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[1].g +  \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[2].g +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[3].g; \
  tmp=DOWNSHIFT(tmp,16);\
  dst->g = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2].b +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[3].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2].b +  \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[3].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2].b +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[3].b +  \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[0].b +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[1].b +  \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[2].b +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[3].b;   \
  tmp=DOWNSHIFT(tmp,16);\
  dst->b = tmp;

#endif

#include "transform_c.h"

/* transform_uint8_x_1_c */

#define FUNC_NAME transform_uint8_x_1_c
#define TYPE uint8_t
#define INIT TMP_TYPE_8 tmp;

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2] +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1] +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2]; \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;

#elif NUM_TAPS == 4
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[3] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2] +  \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[3] +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[1] +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[3] +  \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[0] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[1] +  \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[2] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[3];  \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;


#endif

#include "transform_c.h"

/* transform_uint8_x_1_advance_c */

#define FUNC_NAME transform_uint8_x_1_advance_c
#define TYPE uint8_t
#define INIT TMP_TYPE_8 tmp;

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[ctx->advance];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2*ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2*ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2*ctx->advance]; \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;

#elif NUM_TAPS == 4
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[2*ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[3*ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[2*ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[3*ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[2*ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[3*ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[0] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[2*ctx->advance] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[3*ctx->advance];  \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;

#endif

#include "transform_c.h"


/* transform_uint8_x_2_c */

#define FUNC_NAME transform_uint8_x_2_c
#define TYPE uint8_t
#define INIT TMP_TYPE_8 tmp;

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[2];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[3] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[3];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp;
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[4] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[2] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[4] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[2] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[4];  \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[3] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[5] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[3] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[5] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[3] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[5];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp;
#elif NUM_TAPS == 4
#define TRANSFORM \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[4] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[6] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[2] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[4] +   \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[6] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[2] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[4] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[6] +   \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[0] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[2] +   \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[4] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[6];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[3] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[5] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[7] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[3] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[5] +   \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[7] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[3] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[5] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[7] +   \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[1] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[3] +   \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[5] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[7];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp;
#endif

#include "transform_c.h"

/* transform_uint8_x_3_c */

#define FUNC_NAME transform_uint8_x_3_3_c
#define TYPE uint8_t
#define INIT TMP_TYPE_8 tmp;

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[3] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[3];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[4];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[5];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp;
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[3] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[6] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[3] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[6] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[3] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[6];  \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[7] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[4] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[7] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[4] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[7];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[8] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[5] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[8] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[2] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[5] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[8];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp;
#elif NUM_TAPS == 4
#define TRANSFORM \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[3] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[6] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[9] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[3] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[6] +   \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[9] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[3] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[6] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[9] +   \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[0] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[3] +   \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[6] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[9];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[7] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[10] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[4] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[7] +   \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[10] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[4] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[7] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[10] +   \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[1] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[4] +   \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[7] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[10];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[8] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[11] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[5] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[8] +   \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[11] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[2] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[5] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[8] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[11] +   \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[2] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[5] +   \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[8] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[11];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp;

#endif

#include "transform_c.h"

/* transform_uint8_x_3_4_c */

#define FUNC_NAME transform_uint8_x_3_4_c
#define TYPE uint8_t
#define INIT TMP_TYPE_8 tmp;

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[4];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[5];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[6] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[6];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp;
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[8] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[4] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[8] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[4] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[8];  \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[9] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[5] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[9] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[5] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[9];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[6] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[10] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[6] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[10] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[2] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[6] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[10];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp;
#elif NUM_TAPS == 4
#define TRANSFORM \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[8] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[12] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[4] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[8] +   \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[12] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[4] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[8] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[12] +   \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[0] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[4] +   \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[8] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[12];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[9] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[13] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[5] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[9] +   \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[13] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[5] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[9] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[13] +   \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[1] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[5] +   \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[9] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[13];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[6] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[10] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[14] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[6] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[10] +   \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[14] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[2] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[6] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[10] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[14] +   \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[2] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[6] +   \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[10] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[14];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp;

#endif

#include "transform_c.h"

/* transform_uint8_x_4_c */

#define FUNC_NAME transform_uint8_x_4_c
#define TYPE uint8_t
#define INIT TMP_TYPE_8 tmp;

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[4];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[5];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[6] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[6];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[3] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[7] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[7];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[3] = tmp;
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[8] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[4] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[8] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[4] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[8];  \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[9] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[5] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[9] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[5] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[9];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[6] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[10] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[6] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[10] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[2] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[6] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[10];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[3] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[7] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[11] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[3] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[7] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[11] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[3] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[7] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[11];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[3] = tmp;
#elif NUM_TAPS == 4
#define TRANSFORM \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[8] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[12] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[4] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[8] +   \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[12] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[4] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[8] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[12] +   \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[0] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[4] +   \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[8] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[12];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[9] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[13] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[5] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[9] +   \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[13] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[5] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[9] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[13] +   \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[1] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[5] +   \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[9] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[13];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[6] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[10] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[14] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[6] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[10] +   \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[14] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[2] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[6] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[10] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[14] +   \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[2] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[6] +   \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[10] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[14];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp;\
  tmp = (TMP_TYPE_8)pixel->factors_i[0][0] * src_0[3] +  \
        (TMP_TYPE_8)pixel->factors_i[0][1] * src_0[7] +  \
        (TMP_TYPE_8)pixel->factors_i[0][2] * src_0[11] +  \
        (TMP_TYPE_8)pixel->factors_i[0][3] * src_0[15] +  \
        (TMP_TYPE_8)pixel->factors_i[1][0] * src_1[3] +  \
        (TMP_TYPE_8)pixel->factors_i[1][1] * src_1[7] +   \
        (TMP_TYPE_8)pixel->factors_i[1][2] * src_1[11] +   \
        (TMP_TYPE_8)pixel->factors_i[1][3] * src_1[15] +   \
        (TMP_TYPE_8)pixel->factors_i[2][0] * src_2[3] +  \
        (TMP_TYPE_8)pixel->factors_i[2][1] * src_2[7] +   \
        (TMP_TYPE_8)pixel->factors_i[2][2] * src_2[11] +  \
        (TMP_TYPE_8)pixel->factors_i[2][3] * src_2[15] +   \
        (TMP_TYPE_8)pixel->factors_i[3][0] * src_3[3] +  \
        (TMP_TYPE_8)pixel->factors_i[3][1] * src_3[7] +   \
        (TMP_TYPE_8)pixel->factors_i[3][2] * src_3[11] +  \
        (TMP_TYPE_8)pixel->factors_i[3][3] * src_3[15];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[3] = tmp;


#endif

#include "transform_c.h"


/* transform_uint16_x_1_c */

#define FUNC_NAME transform_uint16_x_1_c
#define TYPE uint16_t
#define INIT TMP_TYPE_16 tmp;

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[1] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[1];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[1] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[2] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[1] +  \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[2] +  \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[1] +  \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[2]; \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;

#elif NUM_TAPS == 4
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[1] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[2] +  \
        (TMP_TYPE_16)pixel->factors_i[0][3] * src_0[3] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[1] +  \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[2] +  \
        (TMP_TYPE_16)pixel->factors_i[1][3] * src_1[3] +  \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[1] +  \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[2] +  \
        (TMP_TYPE_16)pixel->factors_i[2][3] * src_2[3] +  \
        (TMP_TYPE_16)pixel->factors_i[3][0] * src_3[0] +  \
        (TMP_TYPE_16)pixel->factors_i[3][1] * src_3[1] +  \
        (TMP_TYPE_16)pixel->factors_i[3][2] * src_3[2] +  \
        (TMP_TYPE_16)pixel->factors_i[3][3] * src_3[3];  \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;


#endif

#include "transform_c.h"


/* transform_uint16_x_2_c */

#define FUNC_NAME transform_uint16_x_2_c
#define TYPE uint16_t
#define INIT TMP_TYPE_16 tmp;

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[2] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[2];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[3] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[3];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp;
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[2] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[4] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[2] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[4] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[2] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[4];  \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[3] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[5] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[3] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[5] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[3] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[5];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp;
#elif NUM_TAPS == 4
#define TRANSFORM \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[2] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[4] +  \
        (TMP_TYPE_16)pixel->factors_i[0][3] * src_0[6] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[2] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[4] +   \
        (TMP_TYPE_16)pixel->factors_i[1][3] * src_1[6] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[2] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[4] +  \
        (TMP_TYPE_16)pixel->factors_i[2][3] * src_2[6] +   \
        (TMP_TYPE_16)pixel->factors_i[3][0] * src_3[0] +  \
        (TMP_TYPE_16)pixel->factors_i[3][1] * src_3[2] +   \
        (TMP_TYPE_16)pixel->factors_i[3][2] * src_3[4] +  \
        (TMP_TYPE_16)pixel->factors_i[3][3] * src_3[6];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[3] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[5] +  \
        (TMP_TYPE_16)pixel->factors_i[0][3] * src_0[7] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[3] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[5] +   \
        (TMP_TYPE_16)pixel->factors_i[1][3] * src_1[7] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[3] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[5] +  \
        (TMP_TYPE_16)pixel->factors_i[2][3] * src_2[7] +   \
        (TMP_TYPE_16)pixel->factors_i[3][0] * src_3[1] +  \
        (TMP_TYPE_16)pixel->factors_i[3][1] * src_3[3] +   \
        (TMP_TYPE_16)pixel->factors_i[3][2] * src_3[5] +  \
        (TMP_TYPE_16)pixel->factors_i[3][3] * src_3[7];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp;
#endif

#include "transform_c.h"

/* transform_uint16_x_3_c */

#define FUNC_NAME transform_uint16_x_3_c
#define TYPE uint16_t
#define INIT TMP_TYPE_16 tmp;

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[3] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[3];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[4];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[5];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp;
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[3] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[6] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[3] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[6] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[3] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[6];  \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[7] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[4] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[7] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[4] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[7];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[8] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[5] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[8] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[2] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[5] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[8];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp;
#elif NUM_TAPS == 4
#define TRANSFORM \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[3] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[6] +  \
        (TMP_TYPE_16)pixel->factors_i[0][3] * src_0[9] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[3] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[6] +   \
        (TMP_TYPE_16)pixel->factors_i[1][3] * src_1[9] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[3] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[6] +  \
        (TMP_TYPE_16)pixel->factors_i[2][3] * src_2[9] +   \
        (TMP_TYPE_16)pixel->factors_i[3][0] * src_3[0] +  \
        (TMP_TYPE_16)pixel->factors_i[3][1] * src_3[3] +   \
        (TMP_TYPE_16)pixel->factors_i[3][2] * src_3[6] +  \
        (TMP_TYPE_16)pixel->factors_i[3][3] * src_3[9];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[7] +  \
        (TMP_TYPE_16)pixel->factors_i[0][3] * src_0[10] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[4] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[7] +   \
        (TMP_TYPE_16)pixel->factors_i[1][3] * src_1[10] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[4] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[7] +  \
        (TMP_TYPE_16)pixel->factors_i[2][3] * src_2[10] +   \
        (TMP_TYPE_16)pixel->factors_i[3][0] * src_3[1] +  \
        (TMP_TYPE_16)pixel->factors_i[3][1] * src_3[4] +   \
        (TMP_TYPE_16)pixel->factors_i[3][2] * src_3[7] +  \
        (TMP_TYPE_16)pixel->factors_i[3][3] * src_3[10];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[8] +  \
        (TMP_TYPE_16)pixel->factors_i[0][3] * src_0[11] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[5] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[8] +   \
        (TMP_TYPE_16)pixel->factors_i[1][3] * src_1[11] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[2] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[5] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[8] +  \
        (TMP_TYPE_16)pixel->factors_i[2][3] * src_2[11] +   \
        (TMP_TYPE_16)pixel->factors_i[3][0] * src_3[2] +  \
        (TMP_TYPE_16)pixel->factors_i[3][1] * src_3[5] +   \
        (TMP_TYPE_16)pixel->factors_i[3][2] * src_3[8] +  \
        (TMP_TYPE_16)pixel->factors_i[3][3] * src_3[11];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp;

#endif

#include "transform_c.h"

/* transform_uint16_x_4_c */

#define FUNC_NAME transform_uint16_x_4_c
#define TYPE uint16_t
#define INIT TMP_TYPE_16 tmp;

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[4];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[5];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[6] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[6];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp; \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[3] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[7] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[7];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[3] = tmp;
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[8] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[4] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[8] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[4] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[8];  \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[9] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[5] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[9] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[5] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[9];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[6] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[10] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[6] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[10] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[2] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[6] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[10];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp; \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[3] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[7] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[11] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[3] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[7] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[11] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[3] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[7] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[11];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp;
#elif NUM_TAPS == 4
#define TRANSFORM \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[0] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[4] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[8] +  \
        (TMP_TYPE_16)pixel->factors_i[0][3] * src_0[12] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[0] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[4] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[8] +   \
        (TMP_TYPE_16)pixel->factors_i[1][3] * src_1[12] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[0] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[4] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[8] +  \
        (TMP_TYPE_16)pixel->factors_i[2][3] * src_2[12] +   \
        (TMP_TYPE_16)pixel->factors_i[3][0] * src_3[0] +  \
        (TMP_TYPE_16)pixel->factors_i[3][1] * src_3[4] +   \
        (TMP_TYPE_16)pixel->factors_i[3][2] * src_3[8] +  \
        (TMP_TYPE_16)pixel->factors_i[3][3] * src_3[12];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[0] = tmp;                                                 \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[1] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[5] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[9] +  \
        (TMP_TYPE_16)pixel->factors_i[0][3] * src_0[13] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[1] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[5] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[9] +   \
        (TMP_TYPE_16)pixel->factors_i[1][3] * src_1[13] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[1] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[5] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[9] +  \
        (TMP_TYPE_16)pixel->factors_i[2][3] * src_2[13] +   \
        (TMP_TYPE_16)pixel->factors_i[3][0] * src_3[1] +  \
        (TMP_TYPE_16)pixel->factors_i[3][1] * src_3[5] +   \
        (TMP_TYPE_16)pixel->factors_i[3][2] * src_3[9] +  \
        (TMP_TYPE_16)pixel->factors_i[3][3] * src_3[13];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[1] = tmp; \
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[2] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[6] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[10] +  \
        (TMP_TYPE_16)pixel->factors_i[0][3] * src_0[14] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[2] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[6] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[10] +   \
        (TMP_TYPE_16)pixel->factors_i[1][3] * src_1[14] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[2] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[6] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[10] +  \
        (TMP_TYPE_16)pixel->factors_i[2][3] * src_2[14] +   \
        (TMP_TYPE_16)pixel->factors_i[3][0] * src_3[2] +  \
        (TMP_TYPE_16)pixel->factors_i[3][1] * src_3[6] +   \
        (TMP_TYPE_16)pixel->factors_i[3][2] * src_3[10] +  \
        (TMP_TYPE_16)pixel->factors_i[3][3] * src_3[14];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[2] = tmp;\
  tmp = (TMP_TYPE_16)pixel->factors_i[0][0] * src_0[3] +  \
        (TMP_TYPE_16)pixel->factors_i[0][1] * src_0[7] +  \
        (TMP_TYPE_16)pixel->factors_i[0][2] * src_0[11] +  \
        (TMP_TYPE_16)pixel->factors_i[0][3] * src_0[15] +  \
        (TMP_TYPE_16)pixel->factors_i[1][0] * src_1[3] +  \
        (TMP_TYPE_16)pixel->factors_i[1][1] * src_1[7] +   \
        (TMP_TYPE_16)pixel->factors_i[1][2] * src_1[11] +   \
        (TMP_TYPE_16)pixel->factors_i[1][3] * src_1[15] +   \
        (TMP_TYPE_16)pixel->factors_i[2][0] * src_2[3] +  \
        (TMP_TYPE_16)pixel->factors_i[2][1] * src_2[7] +   \
        (TMP_TYPE_16)pixel->factors_i[2][2] * src_2[11] +  \
        (TMP_TYPE_16)pixel->factors_i[2][3] * src_2[15] +   \
        (TMP_TYPE_16)pixel->factors_i[3][0] * src_3[3] +  \
        (TMP_TYPE_16)pixel->factors_i[3][1] * src_3[7] +   \
        (TMP_TYPE_16)pixel->factors_i[3][2] * src_3[11] +  \
        (TMP_TYPE_16)pixel->factors_i[3][3] * src_3[15];   \
  tmp=DOWNSHIFT(tmp,16);\
  dst[3] = tmp;
#endif

#include "transform_c.h"

#define FUNC_NAME transform_float_x_1_c
#define TYPE float

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  dst[0] = pixel->factors[0][0] * src_0[0] +  \
           pixel->factors[0][1] * src_0[1] +  \
           pixel->factors[1][0] * src_1[0] +  \
           pixel->factors[1][1] * src_1[1];
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  dst[0] = pixel->factors[0][0] * src_0[0] +  \
           pixel->factors[0][1] * src_0[1] +  \
           pixel->factors[0][2] * src_0[2] +  \
           pixel->factors[1][0] * src_1[0] +  \
           pixel->factors[1][1] * src_1[1] +  \
           pixel->factors[1][2] * src_1[2] +  \
           pixel->factors[2][0] * src_2[0] +  \
           pixel->factors[2][1] * src_2[1] +  \
           pixel->factors[2][2] * src_2[2];

#elif NUM_TAPS == 4
#define TRANSFORM                                               \
  dst[0] = pixel->factors[0][0] * src_0[0] +  \
           pixel->factors[0][1] * src_0[1] +  \
           pixel->factors[0][2] * src_0[2] +  \
           pixel->factors[0][3] * src_0[3] +  \
           pixel->factors[1][0] * src_1[0] +  \
           pixel->factors[1][1] * src_1[1] +  \
           pixel->factors[1][2] * src_1[2] +  \
           pixel->factors[1][3] * src_1[3] +  \
           pixel->factors[2][0] * src_2[0] +  \
           pixel->factors[2][1] * src_2[1] +  \
           pixel->factors[2][2] * src_2[2] +  \
           pixel->factors[2][3] * src_2[3] +  \
           pixel->factors[3][0] * src_3[0] +  \
           pixel->factors[3][1] * src_3[1] +  \
           pixel->factors[3][2] * src_3[2] +  \
           pixel->factors[3][3] * src_3[3];
#endif

#include "transform_c.h"

#define FUNC_NAME transform_float_x_2_c
#define TYPE float

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  dst[0] = pixel->factors[0][0] * src_0[0] +  \
           pixel->factors[0][1] * src_0[2] +  \
           pixel->factors[1][0] * src_1[0] +  \
           pixel->factors[1][1] * src_1[2]; \
  dst[1] = pixel->factors[0][0] * src_0[1] +  \
           pixel->factors[0][1] * src_0[3] +  \
           pixel->factors[1][0] * src_1[1] +  \
           pixel->factors[1][1] * src_1[3];
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  dst[0] = pixel->factors[0][0] * src_0[0] +  \
           pixel->factors[0][1] * src_0[2] +  \
           pixel->factors[0][2] * src_0[4] +  \
           pixel->factors[1][0] * src_1[0] +  \
           pixel->factors[1][1] * src_1[2] +  \
           pixel->factors[1][2] * src_1[4] +  \
           pixel->factors[2][0] * src_2[0] +  \
           pixel->factors[2][1] * src_2[2] +  \
           pixel->factors[2][2] * src_2[4];\
  dst[1] = pixel->factors[0][0] * src_0[1] +  \
           pixel->factors[0][1] * src_0[3] +  \
           pixel->factors[0][2] * src_0[5] +  \
           pixel->factors[1][0] * src_1[1] +  \
           pixel->factors[1][1] * src_1[3] +  \
           pixel->factors[1][2] * src_1[5] +  \
           pixel->factors[2][0] * src_2[1] +  \
           pixel->factors[2][1] * src_2[3] +  \
           pixel->factors[2][2] * src_2[5];
#elif NUM_TAPS == 4
#define TRANSFORM                                               \
  dst[0] = pixel->factors[0][0] * src_0[0] +  \
           pixel->factors[0][1] * src_0[2] +  \
           pixel->factors[0][2] * src_0[4] +  \
           pixel->factors[0][3] * src_0[6] +  \
           pixel->factors[1][0] * src_1[0] +  \
           pixel->factors[1][1] * src_1[2] +  \
           pixel->factors[1][2] * src_1[4] +  \
           pixel->factors[1][3] * src_1[6] +  \
           pixel->factors[2][0] * src_2[0] +  \
           pixel->factors[2][1] * src_2[2] +  \
           pixel->factors[2][2] * src_2[4] +  \
           pixel->factors[2][3] * src_2[6] +  \
           pixel->factors[3][0] * src_3[0] +  \
           pixel->factors[3][1] * src_3[2] +  \
           pixel->factors[3][2] * src_3[4] +  \
           pixel->factors[3][3] * src_3[6];\
  dst[1] = pixel->factors[0][0] * src_0[1] +  \
           pixel->factors[0][1] * src_0[3] +  \
           pixel->factors[0][2] * src_0[5] +  \
           pixel->factors[0][3] * src_0[7] +  \
           pixel->factors[1][0] * src_1[1] +  \
           pixel->factors[1][1] * src_1[3] +  \
           pixel->factors[1][2] * src_1[5] +  \
           pixel->factors[1][3] * src_1[7] +  \
           pixel->factors[2][0] * src_2[1] +  \
           pixel->factors[2][1] * src_2[3] +  \
           pixel->factors[2][2] * src_2[5] +  \
           pixel->factors[2][3] * src_2[7] +  \
           pixel->factors[3][0] * src_3[1] +  \
           pixel->factors[3][1] * src_3[3] +  \
           pixel->factors[3][2] * src_3[5] +  \
           pixel->factors[3][3] * src_3[7];
#endif

#include "transform_c.h"

#define FUNC_NAME transform_float_x_3_c
#define TYPE float

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  dst[0] = pixel->factors[0][0] * src_0[0] +  \
           pixel->factors[0][1] * src_0[3] +  \
           pixel->factors[1][0] * src_1[0] +  \
           pixel->factors[1][1] * src_1[3]; \
  dst[1] = pixel->factors[0][0] * src_0[1] +  \
           pixel->factors[0][1] * src_0[4] +  \
           pixel->factors[1][0] * src_1[1] +  \
           pixel->factors[1][1] * src_1[4];\
  dst[2] = pixel->factors[0][0] * src_0[2] +  \
           pixel->factors[0][1] * src_0[5] +  \
           pixel->factors[1][0] * src_1[2] +  \
           pixel->factors[1][1] * src_1[5];
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  dst[0] = pixel->factors[0][0] * src_0[0] +  \
           pixel->factors[0][1] * src_0[3] +  \
           pixel->factors[0][2] * src_0[6] +  \
           pixel->factors[1][0] * src_1[0] +  \
           pixel->factors[1][1] * src_1[3] +  \
           pixel->factors[1][2] * src_1[6] +  \
           pixel->factors[2][0] * src_2[0] +  \
           pixel->factors[2][1] * src_2[3] +  \
           pixel->factors[2][2] * src_2[6];\
  dst[1] = pixel->factors[0][0] * src_0[1] +  \
           pixel->factors[0][1] * src_0[4] +  \
           pixel->factors[0][2] * src_0[7] +  \
           pixel->factors[1][0] * src_1[1] +  \
           pixel->factors[1][1] * src_1[4] +  \
           pixel->factors[1][2] * src_1[7] +  \
           pixel->factors[2][0] * src_2[1] +  \
           pixel->factors[2][1] * src_2[4] +  \
           pixel->factors[2][2] * src_2[7];\
  dst[2] = pixel->factors[0][0] * src_0[2] +  \
           pixel->factors[0][1] * src_0[5] +  \
           pixel->factors[0][2] * src_0[8] +  \
           pixel->factors[1][0] * src_1[2] +  \
           pixel->factors[1][1] * src_1[5] +  \
           pixel->factors[1][2] * src_1[8] +  \
           pixel->factors[2][0] * src_2[2] +  \
           pixel->factors[2][1] * src_2[5] +  \
           pixel->factors[2][2] * src_2[8];
#elif NUM_TAPS == 4
#define TRANSFORM                                               \
  dst[0] = pixel->factors[0][0] * src_0[0] +  \
           pixel->factors[0][1] * src_0[3] +  \
           pixel->factors[0][2] * src_0[6] +  \
           pixel->factors[0][3] * src_0[9] +  \
           pixel->factors[1][0] * src_1[0] +  \
           pixel->factors[1][1] * src_1[3] +  \
           pixel->factors[1][2] * src_1[6] +  \
           pixel->factors[1][3] * src_1[9] +  \
           pixel->factors[2][0] * src_2[0] +  \
           pixel->factors[2][1] * src_2[3] +  \
           pixel->factors[2][2] * src_2[6] +  \
           pixel->factors[2][3] * src_2[9] +  \
           pixel->factors[3][0] * src_3[0] +  \
           pixel->factors[3][1] * src_3[3] +  \
           pixel->factors[3][2] * src_3[6] +  \
           pixel->factors[3][3] * src_3[9];\
  dst[1] = pixel->factors[0][0] * src_0[1] +  \
           pixel->factors[0][1] * src_0[4] +  \
           pixel->factors[0][2] * src_0[7] +  \
           pixel->factors[0][3] * src_0[10] +  \
           pixel->factors[1][0] * src_1[1] +  \
           pixel->factors[1][1] * src_1[4] +  \
           pixel->factors[1][2] * src_1[7] +  \
           pixel->factors[1][3] * src_1[10] +  \
           pixel->factors[2][0] * src_2[1] +  \
           pixel->factors[2][1] * src_2[4] +  \
           pixel->factors[2][2] * src_2[7] +  \
           pixel->factors[2][3] * src_2[10] +  \
           pixel->factors[3][0] * src_3[1] +  \
           pixel->factors[3][1] * src_3[4] +  \
           pixel->factors[3][2] * src_3[7] +  \
           pixel->factors[3][3] * src_3[10];\
  dst[2] = pixel->factors[0][0] * src_0[2] +  \
           pixel->factors[0][1] * src_0[5] +  \
           pixel->factors[0][2] * src_0[8] +  \
           pixel->factors[0][3] * src_0[11] +  \
           pixel->factors[1][0] * src_1[2] +  \
           pixel->factors[1][1] * src_1[5] +  \
           pixel->factors[1][2] * src_1[8] +  \
           pixel->factors[1][3] * src_1[11] +  \
           pixel->factors[2][0] * src_2[2] +  \
           pixel->factors[2][1] * src_2[5] +  \
           pixel->factors[2][2] * src_2[8] +  \
           pixel->factors[2][3] * src_2[11] +  \
           pixel->factors[3][0] * src_3[2] +  \
           pixel->factors[3][1] * src_3[5] +  \
           pixel->factors[3][2] * src_3[8] +  \
           pixel->factors[3][3] * src_3[11];

#endif

#include "transform_c.h"

#define FUNC_NAME transform_float_x_4_c
#define TYPE float

#if NUM_TAPS == 2
#define TRANSFORM                                               \
  dst[0] = pixel->factors[0][0] * src_0[0] +  \
           pixel->factors[0][1] * src_0[4] +  \
           pixel->factors[1][0] * src_1[0] +  \
           pixel->factors[1][1] * src_1[4]; \
  dst[1] = pixel->factors[0][0] * src_0[1] +  \
           pixel->factors[0][1] * src_0[5] +  \
           pixel->factors[1][0] * src_1[1] +  \
           pixel->factors[1][1] * src_1[5];\
  dst[2] = pixel->factors[0][0] * src_0[2] +  \
           pixel->factors[0][1] * src_0[6] +  \
           pixel->factors[1][0] * src_1[2] +  \
           pixel->factors[1][1] * src_1[6];\
  dst[3] = pixel->factors[0][0] * src_0[3] +  \
           pixel->factors[0][1] * src_0[7] +  \
           pixel->factors[1][0] * src_1[3] +  \
           pixel->factors[1][1] * src_1[7];
#elif NUM_TAPS == 3
#define TRANSFORM                                               \
  dst[0] = pixel->factors[0][0] * src_0[0] +  \
           pixel->factors[0][1] * src_0[4] +  \
           pixel->factors[0][2] * src_0[8] +  \
           pixel->factors[1][0] * src_1[0] +  \
           pixel->factors[1][1] * src_1[4] +  \
           pixel->factors[1][2] * src_1[8] +  \
           pixel->factors[2][0] * src_2[0] +  \
           pixel->factors[2][1] * src_2[4] +  \
           pixel->factors[2][2] * src_2[8];\
  dst[1] = pixel->factors[0][0] * src_0[1] +  \
           pixel->factors[0][1] * src_0[5] +  \
           pixel->factors[0][2] * src_0[9] +  \
           pixel->factors[1][0] * src_1[1] +  \
           pixel->factors[1][1] * src_1[5] +  \
           pixel->factors[1][2] * src_1[9] +  \
           pixel->factors[2][0] * src_2[1] +  \
           pixel->factors[2][1] * src_2[5] +  \
           pixel->factors[2][2] * src_2[9];\
  dst[2] = pixel->factors[0][0] * src_0[2] +  \
           pixel->factors[0][1] * src_0[6] +  \
           pixel->factors[0][2] * src_0[10] +  \
           pixel->factors[1][0] * src_1[2] +  \
           pixel->factors[1][1] * src_1[6] +  \
           pixel->factors[1][2] * src_1[10] +  \
           pixel->factors[2][0] * src_2[2] +  \
           pixel->factors[2][1] * src_2[6] +  \
           pixel->factors[2][2] * src_2[10];
#elif NUM_TAPS == 4
#define TRANSFORM                                               \
  dst[0] = pixel->factors[0][0] * src_0[0] +  \
           pixel->factors[0][1] * src_0[4] +  \
           pixel->factors[0][2] * src_0[8] +  \
           pixel->factors[0][3] * src_0[10] +  \
           pixel->factors[1][0] * src_1[0] +  \
           pixel->factors[1][1] * src_1[4] +  \
           pixel->factors[1][2] * src_1[8] +  \
           pixel->factors[1][3] * src_1[10] +  \
           pixel->factors[2][0] * src_2[0] +  \
           pixel->factors[2][1] * src_2[4] +  \
           pixel->factors[2][2] * src_2[8] +  \
           pixel->factors[2][3] * src_2[12] +  \
           pixel->factors[3][0] * src_3[0] +  \
           pixel->factors[3][1] * src_3[4] +  \
           pixel->factors[3][2] * src_3[8] +  \
           pixel->factors[3][3] * src_3[12];\
  dst[1] = pixel->factors[0][0] * src_0[1] +  \
           pixel->factors[0][1] * src_0[5] +  \
           pixel->factors[0][2] * src_0[9] +  \
           pixel->factors[0][3] * src_0[13] +  \
           pixel->factors[1][0] * src_1[1] +  \
           pixel->factors[1][1] * src_1[5] +  \
           pixel->factors[1][2] * src_1[9] +  \
           pixel->factors[1][3] * src_1[13] +  \
           pixel->factors[2][0] * src_2[1] +  \
           pixel->factors[2][1] * src_2[5] +  \
           pixel->factors[2][2] * src_2[9] +  \
           pixel->factors[2][3] * src_2[13] +  \
           pixel->factors[3][0] * src_3[1] +  \
           pixel->factors[3][1] * src_3[5] +  \
           pixel->factors[3][2] * src_3[9] +  \
           pixel->factors[3][3] * src_3[13];\
  dst[2] = pixel->factors[0][0] * src_0[2] +  \
           pixel->factors[0][1] * src_0[6] +  \
           pixel->factors[0][2] * src_0[10] +  \
           pixel->factors[0][3] * src_0[14] +  \
           pixel->factors[1][0] * src_1[2] +  \
           pixel->factors[1][1] * src_1[6] +  \
           pixel->factors[1][2] * src_1[10] +  \
           pixel->factors[1][3] * src_1[14] +  \
           pixel->factors[2][0] * src_2[2] +  \
           pixel->factors[2][1] * src_2[6] +  \
           pixel->factors[2][2] * src_2[10] +  \
           pixel->factors[2][3] * src_2[14] +  \
           pixel->factors[3][0] * src_3[2] +  \
           pixel->factors[3][1] * src_3[6] +  \
           pixel->factors[3][2] * src_3[10] +  \
           pixel->factors[3][3] * src_3[14];\
  dst[3] = pixel->factors[0][0] * src_0[3] +  \
           pixel->factors[0][1] * src_0[7] +  \
           pixel->factors[0][2] * src_0[11] +  \
           pixel->factors[0][3] * src_0[15] +  \
           pixel->factors[1][0] * src_1[3] +  \
           pixel->factors[1][1] * src_1[7] +  \
           pixel->factors[1][2] * src_1[11] +  \
           pixel->factors[1][3] * src_1[15] +  \
           pixel->factors[2][0] * src_2[3] +  \
           pixel->factors[2][1] * src_2[7] +  \
           pixel->factors[2][2] * src_2[11] +  \
           pixel->factors[2][3] * src_2[15] +  \
           pixel->factors[3][0] * src_3[3] +  \
           pixel->factors[3][1] * src_3[7] +  \
           pixel->factors[3][2] * src_3[11] +  \
           pixel->factors[3][3] * src_3[15];

#endif

#include "transform_c.h"




#if NUM_TAPS == 2
void gavl_init_transform_funcs_bilinear_c(gavl_transform_funcs_t * tab,
                                          int advance)
#elif NUM_TAPS == 3
void gavl_init_transform_funcs_quadratic_c(gavl_transform_funcs_t * tab,
                                           int advance)
#elif NUM_TAPS == 4
void gavl_init_transform_funcs_bicubic_c(gavl_transform_funcs_t * tab,
                                         int advance)
#endif
  {
  tab->transform_rgb_15 =     transform_rgb_15_c;
  tab->transform_rgb_16 =     transform_rgb_16_c;

  tab->transform_uint8_x_1_advance =
    transform_uint8_x_1_advance_c;
  tab->transform_uint8_x_1_noadvance =
    transform_uint8_x_1_c;

  tab->transform_uint8_x_2 =  transform_uint8_x_2_c;
  
  if(advance == 4)
    tab->transform_uint8_x_3 =  transform_uint8_x_3_4_c;
  else
    tab->transform_uint8_x_3 =  transform_uint8_x_3_3_c;

  tab->transform_uint8_x_4 =  transform_uint8_x_4_c;

  tab->transform_uint16_x_1 = transform_uint16_x_1_c;
  tab->transform_uint16_x_2 = transform_uint16_x_2_c;
  tab->transform_uint16_x_3 = transform_uint16_x_3_c;
  tab->transform_uint16_x_4 = transform_uint16_x_4_c;
  tab->transform_float_x_1 =  transform_float_x_1_c;
  tab->transform_float_x_2 =  transform_float_x_2_c;
  tab->transform_float_x_3 =  transform_float_x_3_c;
  tab->transform_float_x_4 =  transform_float_x_4_c;

  tab->bits_rgb_15 = 16;
  tab->bits_rgb_16 = 16;
  tab->bits_uint8_advance  = 16;
  tab->bits_uint8_noadvance  = 16;
  tab->bits_uint16_x_1 = 16;
  tab->bits_uint16_x_2 = 16;
  tab->bits_uint16_x_3 = 16;
  tab->bits_uint16_x_4 = 16;
  }
