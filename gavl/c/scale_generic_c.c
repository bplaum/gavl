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
#include <stdio.h>
#include <gavl/gavl.h>
#include <video.h>
#include <scale.h>

#include "scale_macros.h"

/* x-Direction */

#define FUNC_NAME scale_rgb_15_x_generic_c
#define TYPE color_15

#define INIT int64_t tmp[3];

#define SCALE_INIT tmp[0] = 0; tmp[1] = 0; tmp[2] = 0;

#define SCALE_ACCUM                                                    \
  tmp[0] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src->r;   \
  tmp[1] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src->g;   \
  tmp[2] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src->b;

#define SCALE_FINISH                                                    \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  tmp[2] = DOWNSHIFT(tmp[2], 16); \
  RECLIP_H(tmp[0],0);                                                     \
  RECLIP_H(tmp[1],1);                                                     \
  RECLIP_H(tmp[2],2);                                                     \
  dst->r = tmp[0];                                                 \
  dst->g = tmp[1];                                                 \
  dst->b = tmp[2];

#include "scale_generic_x.h"

#define FUNC_NAME scale_rgb_16_x_generic_c
#define TYPE color_16

#define INIT int64_t tmp[3];

#define SCALE_INIT tmp[0] = 0; tmp[1] = 0; tmp[2] = 0;

#define SCALE_ACCUM                                                    \
  tmp[0] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src->r;   \
  tmp[1] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src->g;   \
  tmp[2] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src->b;

#define SCALE_FINISH                                                    \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  tmp[2] = DOWNSHIFT(tmp[2], 16); \
  RECLIP_H(tmp[0],0);                                                     \
  RECLIP_H(tmp[1],1);                                                     \
  RECLIP_H(tmp[2],2);                                                     \
  dst->r = tmp[0];                                                 \
  dst->g = tmp[1];                                                 \
  dst->b = tmp[2];

#include "scale_generic_x.h"

#define FUNC_NAME scale_uint8_x_1_x_generic_c
#define TYPE uint8_t

#define INIT int64_t tmp;

#define SCALE_INIT tmp = 0;

#define SCALE_ACCUM tmp += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[0];

#define SCALE_FINISH   \
  tmp = DOWNSHIFT(tmp, 16); \
  RECLIP_H(tmp,ctx->plane);               \
  dst[0] = tmp;

#include "scale_generic_x.h"

#define FUNC_NAME scale_uint8_x_2_x_generic_c
#define TYPE uint8_t

#define INIT int64_t tmp[2];

#define SCALE_INIT tmp[0] = 0; tmp[1] = 0;

#define SCALE_ACCUM tmp[0] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[0];\
  tmp[1] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[1];

#define SCALE_FINISH   \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  RECLIP_H(tmp[0],ctx->plane);               \
  dst[0] = tmp[0];\
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  RECLIP_H(tmp[1],ctx->plane);               \
  dst[1] = tmp[1];

#include "scale_generic_x.h"


#define FUNC_NAME scale_uint8_x_3_x_generic_c
#define TYPE uint8_t

#define INIT int64_t tmp[3];

#define SCALE_INIT tmp[0] = 0;tmp[1] = 0;tmp[2] = 0;

#define SCALE_ACCUM \
  tmp[0] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[0];\
  tmp[1] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[1];\
  tmp[2] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[2];

#define SCALE_FINISH   \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  tmp[2] = DOWNSHIFT(tmp[2], 16); \
  RECLIP_H(tmp[0],0);                        \
  RECLIP_H(tmp[1],1);                        \
  RECLIP_H(tmp[2],2);                        \
  dst[0] = tmp[0];                    \
  dst[1] = tmp[1];                    \
  dst[2] = tmp[2];


#include "scale_generic_x.h"

#define FUNC_NAME scale_uint8_x_4_x_generic_c
#define TYPE uint8_t

#define INIT int64_t tmp[4];

#define SCALE_INIT tmp[0] = 0;tmp[1] = 0;tmp[2] = 0;tmp[3] = 0;

#define SCALE_ACCUM \
  tmp[0] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[0];\
  tmp[1] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[1];\
  tmp[2] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[2];\
  tmp[3] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[3];


#define SCALE_FINISH   \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  tmp[2] = DOWNSHIFT(tmp[2], 16); \
  tmp[3] = DOWNSHIFT(tmp[3], 16); \
  RECLIP_H(tmp[0],0);                        \
  RECLIP_H(tmp[1],1);                        \
  RECLIP_H(tmp[2],2);                        \
  RECLIP_H(tmp[3],3);                        \
  dst[0] = tmp[0];                    \
  dst[1] = tmp[1];                    \
  dst[2] = tmp[2];                    \
  dst[3] = tmp[3];

#include "scale_generic_x.h"

#define FUNC_NAME scale_uint16_x_1_x_generic_c
#define TYPE uint16_t

#define INIT int64_t tmp;

#define SCALE_INIT tmp = 0;

#define SCALE_ACCUM                             \
  tmp += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[0];

#define SCALE_FINISH                            \
  tmp = DOWNSHIFT(tmp, 16); \
  RECLIP_H(tmp,ctx->plane);                    \
  dst[0] = tmp;

#include "scale_generic_x.h"

#define FUNC_NAME scale_uint16_x_2_x_generic_c
#define TYPE uint16_t

#define INIT int64_t tmp[2];

#define SCALE_INIT tmp[0] = 0;tmp[1] = 0;

#define SCALE_ACCUM                                                     \
  tmp[0] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[0];   \
  tmp[1] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[1];

#define SCALE_FINISH                            \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  RECLIP_H(tmp[0],0);                             \
  RECLIP_H(tmp[1],1);                             \
  dst[0] = tmp[0];                        \
  dst[1] = tmp[1];

#include "scale_generic_x.h"

#define FUNC_NAME scale_uint16_x_3_x_generic_c
#define TYPE uint16_t

#define INIT int64_t tmp[3];

#define SCALE_INIT tmp[0] = 0;tmp[1] = 0;tmp[2] = 0;

#define SCALE_ACCUM                                                     \
  tmp[0] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[0];   \
  tmp[1] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[1];   \
  tmp[2] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[2];

#define SCALE_FINISH                            \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  tmp[2] = DOWNSHIFT(tmp[2], 16); \
  RECLIP_H(tmp[0],0);                             \
  RECLIP_H(tmp[1],1);                             \
  RECLIP_H(tmp[2],2);                             \
  dst[0] = tmp[0];                        \
  dst[1] = tmp[1];                        \
  dst[2] = tmp[2];

#include "scale_generic_x.h"

#define FUNC_NAME scale_uint16_x_4_x_generic_c
#define TYPE uint16_t

#define INIT int64_t tmp[4];

#define SCALE_INIT \
tmp[0] = 0;\
tmp[1] = 0;\
tmp[2] = 0;\
tmp[3] = 0;

#define SCALE_ACCUM                                                     \
  tmp[0] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[0];   \
  tmp[1] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[1];   \
  tmp[2] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[2];   \
  tmp[3] += (int64_t)ctx->table_h.pixels[i].factor_i[j] * src[3];

#define SCALE_FINISH                            \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  tmp[2] = DOWNSHIFT(tmp[2], 16); \
  tmp[3] = DOWNSHIFT(tmp[3], 16); \
  RECLIP_H(tmp[0],0);                             \
  RECLIP_H(tmp[1],1);                             \
  RECLIP_H(tmp[2],2);                             \
  RECLIP_H(tmp[3],3);                             \
  dst[0] = tmp[0];                        \
  dst[1] = tmp[1];                        \
  dst[2] = tmp[2];                        \
  dst[3] = tmp[3];

#include "scale_generic_x.h"

#define FUNC_NAME scale_float_x_1_x_generic_c
#define TYPE float

#define SCALE_INIT dst[0] = 0.0;

#define SCALE_ACCUM \
  dst[0] += ctx->table_h.pixels[i].factor_f[j] * src[0];

#define SCALE_FINISH                                                    \
  RECLIP_FLOAT(dst[0], 0);

#include "scale_generic_x.h"

#define FUNC_NAME scale_float_x_2_x_generic_c
#define TYPE float

#define SCALE_INIT dst[0] = 0.0; dst[1] = 0.0;

#define SCALE_ACCUM \
  dst[0] += ctx->table_h.pixels[i].factor_f[j] * src[0];\
  dst[1] += ctx->table_h.pixels[i].factor_f[j] * src[1];

#define SCALE_FINISH                                                    \
  RECLIP_FLOAT(dst[0], 0);                                                 \
  RECLIP_FLOAT(dst[1], 1);

#include "scale_generic_x.h"

#define FUNC_NAME scale_float_x_3_x_generic_c
#define TYPE float

#define SCALE_INIT dst[0] = 0.0; dst[1] = 0.0; dst[2] = 0.0;

#define SCALE_ACCUM \
  dst[0] += ctx->table_h.pixels[i].factor_f[j] * src[0];\
  dst[1] += ctx->table_h.pixels[i].factor_f[j] * src[1];\
  dst[2] += ctx->table_h.pixels[i].factor_f[j] * src[2];

#define SCALE_FINISH                                                    \
  RECLIP_FLOAT(dst[0], 0);                                                 \
  RECLIP_FLOAT(dst[1], 1);                                                 \
  RECLIP_FLOAT(dst[2], 2);                                                 \

#include "scale_generic_x.h"


#define FUNC_NAME scale_float_x_4_x_generic_c
#define TYPE float

#define SCALE_INIT dst[0] = 0.0; dst[1] = 0.0; dst[2] = 0.0; dst[3] = 0.0;

#define SCALE_ACCUM \
  dst[0] += ctx->table_h.pixels[i].factor_f[j] * src[0];    \
  dst[1] += ctx->table_h.pixels[i].factor_f[j] * src[1];    \
  dst[2] += ctx->table_h.pixels[i].factor_f[j] * src[2];    \
  dst[3] += ctx->table_h.pixels[i].factor_f[j] * src[3]; 

#define SCALE_FINISH                                                    \
  RECLIP_FLOAT(dst[0], 0);                                                 \
  RECLIP_FLOAT(dst[1], 1);                                                 \
  RECLIP_FLOAT(dst[2], 2);                                                 \
  RECLIP_FLOAT(dst[3], 3);

#include "scale_generic_x.h"

/* y-Direction */

#define FUNC_NAME scale_rgb_15_y_generic_c
#define TYPE color_15

#define INIT int64_t tmp[3];

#define SCALE_INIT tmp[0] = 0;tmp[1] = 0;tmp[2] = 0;

#define SCALE_ACCUM                             \
  tmp[0] += ctx->table_v.pixels[scanline].factor_i[j] * src->r;\
  tmp[1] += ctx->table_v.pixels[scanline].factor_i[j] * src->g;\
  tmp[2] += ctx->table_v.pixels[scanline].factor_i[j] * src->b;

#define SCALE_FINISH                            \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  tmp[2] = DOWNSHIFT(tmp[2], 16); \
  RECLIP_V(tmp[0], 0);                                       \
  RECLIP_V(tmp[1], 1);                                       \
  RECLIP_V(tmp[2], 2);                                       \
  dst->r = tmp[0];                                    \
  dst->g = tmp[1];                                    \
  dst->b = tmp[2];

#include "scale_generic_y.h"

#define FUNC_NAME scale_rgb_16_y_generic_c
#define TYPE color_16

#define INIT int64_t tmp[3];

#define SCALE_INIT tmp[0] = 0;tmp[1] = 0;tmp[2] = 0;

#define SCALE_ACCUM                             \
  tmp[0] += ctx->table_v.pixels[scanline].factor_i[j] * src->r;\
  tmp[1] += ctx->table_v.pixels[scanline].factor_i[j] * src->g;\
  tmp[2] += ctx->table_v.pixels[scanline].factor_i[j] * src->b;

#define SCALE_FINISH                            \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  tmp[2] = DOWNSHIFT(tmp[2], 16); \
  RECLIP_V(tmp[0], 0);                                       \
  RECLIP_V(tmp[1], 1);                                       \
  RECLIP_V(tmp[2], 2);                                       \
  dst->r = tmp[0];                                    \
  dst->g = tmp[1];                                    \
  dst->b = tmp[2];

#include "scale_generic_y.h"

#define FUNC_NAME scale_uint8_x_1_y_generic_c
#define TYPE uint8_t

#define INIT int64_t tmp;

#define SCALE_INIT tmp = 0;

#define SCALE_ACCUM \
  tmp += ctx->table_v.pixels[scanline].factor_i[j] * src[0];

#define SCALE_FINISH                            \
  tmp = DOWNSHIFT(tmp, 16); \
  RECLIP_V(tmp, ctx->plane);                      \
  dst[0] = tmp;

#include "scale_generic_y.h"

#define FUNC_NAME scale_uint8_x_2_y_generic_c
#define TYPE uint8_t

#define INIT int64_t tmp[2];

#define SCALE_INIT tmp[0] = 0;tmp[1] = 0;

#define SCALE_ACCUM \
  tmp[0] += ctx->table_v.pixels[scanline].factor_i[j] * src[0];\
  tmp[1] += ctx->table_v.pixels[scanline].factor_i[j] * src[1];

#define SCALE_FINISH                                    \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  RECLIP_V(tmp[0], 0);                               \
  RECLIP_V(tmp[1], 1);                               \
  dst[0] = tmp[0];                            \
  dst[1] = tmp[1];

#include "scale_generic_y.h"

#define FUNC_NAME scale_uint8_x_3_y_generic_c
#define TYPE uint8_t

#define INIT int64_t tmp[3];

#define SCALE_INIT tmp[0] = 0;tmp[1] = 0;tmp[2] = 0;

#define SCALE_ACCUM \
  tmp[0] += ctx->table_v.pixels[scanline].factor_i[j] * src[0];\
  tmp[1] += ctx->table_v.pixels[scanline].factor_i[j] * src[1];\
  tmp[2] += ctx->table_v.pixels[scanline].factor_i[j] * src[2];

#define SCALE_FINISH                                    \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  tmp[2] = DOWNSHIFT(tmp[2], 16); \
  RECLIP_V(tmp[0], 0);                               \
  RECLIP_V(tmp[1], 1);                               \
  RECLIP_V(tmp[2], 2);                               \
  dst[0] = tmp[0];                            \
  dst[1] = tmp[1];                            \
  dst[2] = tmp[2];

#include "scale_generic_y.h"


#define FUNC_NAME scale_uint8_x_4_y_generic_c
#define TYPE uint8_t

#define INIT int64_t tmp[4];

#define SCALE_INIT tmp[0] = 0;tmp[1] = 0;tmp[2] = 0;tmp[3] = 0;

#define SCALE_ACCUM \
  tmp[0] += ctx->table_v.pixels[scanline].factor_i[j] * src[0];\
  tmp[1] += ctx->table_v.pixels[scanline].factor_i[j] * src[1];\
  tmp[2] += ctx->table_v.pixels[scanline].factor_i[j] * src[2];\
  tmp[3] += ctx->table_v.pixels[scanline].factor_i[j] * src[3];

#define SCALE_FINISH                                    \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  tmp[2] = DOWNSHIFT(tmp[2], 16); \
  tmp[3] = DOWNSHIFT(tmp[3], 16); \
  RECLIP_V(tmp[0], 0);                               \
  RECLIP_V(tmp[1], 1);                               \
  RECLIP_V(tmp[2], 2);                               \
  RECLIP_V(tmp[3], 3);                               \
  dst[0] = tmp[0];                            \
  dst[1] = tmp[1];                            \
  dst[2] = tmp[2];                            \
  dst[3] = tmp[3];


#include "scale_generic_y.h"

#define FUNC_NAME scale_uint16_x_1_y_generic_c
#define TYPE uint16_t

#define INIT int64_t tmp;

#define SCALE_INIT tmp = 0;

#define SCALE_ACCUM                             \
  tmp += (int64_t)ctx->table_v.pixels[scanline].factor_i[j] * src[0];

#define SCALE_FINISH \
  tmp = DOWNSHIFT(tmp, 16); \
  RECLIP_V(tmp, ctx->plane);                      \
  dst[0] = tmp;

#include "scale_generic_y.h"

#define FUNC_NAME scale_uint16_x_2_y_generic_c
#define TYPE uint16_t

#define INIT int64_t tmp[2];

#define SCALE_INIT tmp[0] = 0;tmp[1] = 0;

#define SCALE_ACCUM                             \
  tmp[0] += (int64_t)ctx->table_v.pixels[scanline].factor_i[j] * src[0];\
  tmp[1] += (int64_t)ctx->table_v.pixels[scanline].factor_i[j] * src[1];

#define SCALE_FINISH \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  RECLIP_V(tmp[0], 0);                      \
  RECLIP_V(tmp[1], 1);                      \
  dst[0] = tmp[0];                  \
  dst[1] = tmp[1];

#include "scale_generic_y.h"

#define FUNC_NAME scale_uint16_x_3_y_generic_c
#define TYPE uint16_t

#define INIT int64_t tmp[3];

#define SCALE_INIT tmp[0] = 0;tmp[1] = 0;tmp[2] = 0;

#define SCALE_ACCUM                             \
  tmp[0] += (int64_t)ctx->table_v.pixels[scanline].factor_i[j] * src[0];\
  tmp[1] += (int64_t)ctx->table_v.pixels[scanline].factor_i[j] * src[1];\
  tmp[2] += (int64_t)ctx->table_v.pixels[scanline].factor_i[j] * src[2];

#define SCALE_FINISH \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  tmp[2] = DOWNSHIFT(tmp[2], 16); \
  RECLIP_V(tmp[0], 0);                      \
  RECLIP_V(tmp[1], 1);                      \
  RECLIP_V(tmp[2], 2);                      \
  dst[0] = tmp[0];                  \
  dst[1] = tmp[1];                  \
  dst[2] = tmp[2];

#include "scale_generic_y.h"


#define FUNC_NAME scale_uint16_x_4_y_generic_c
#define TYPE uint16_t

#define INIT int64_t tmp[4];

#define SCALE_INIT tmp[0] = 0;tmp[1] = 0;tmp[2] = 0;tmp[3] = 0;

#define SCALE_ACCUM                             \
  tmp[0] += (int64_t)ctx->table_v.pixels[scanline].factor_i[j] * src[0];\
  tmp[1] += (int64_t)ctx->table_v.pixels[scanline].factor_i[j] * src[1];\
  tmp[2] += (int64_t)ctx->table_v.pixels[scanline].factor_i[j] * src[2];\
  tmp[3] += (int64_t)ctx->table_v.pixels[scanline].factor_i[j] * src[3];

#define SCALE_FINISH \
  tmp[0] = DOWNSHIFT(tmp[0], 16); \
  tmp[1] = DOWNSHIFT(tmp[1], 16); \
  tmp[2] = DOWNSHIFT(tmp[2], 16); \
  tmp[3] = DOWNSHIFT(tmp[3], 16); \
  RECLIP_V(tmp[0], 0);                      \
  RECLIP_V(tmp[1], 1);                      \
  RECLIP_V(tmp[2], 2);                      \
  RECLIP_V(tmp[3], 3);                      \
  dst[0] = tmp[0];                  \
  dst[1] = tmp[1];                  \
  dst[2] = tmp[2];                  \
  dst[3] = tmp[3];

#include "scale_generic_y.h"

#define FUNC_NAME scale_float_x_1_y_generic_c
#define TYPE float

#define SCALE_INIT dst[0] = 0.0;

#define SCALE_ACCUM \
  dst[0] += ctx->table_v.pixels[scanline].factor_f[j] * src[0];
  
#define SCALE_FINISH                  \
  RECLIP_FLOAT(dst[0], 0);

#include "scale_generic_y.h"

#define FUNC_NAME scale_float_x_2_y_generic_c
#define TYPE float

#define SCALE_INIT dst[0] = 0.0; dst[1] = 0.0;

#define SCALE_ACCUM \
  dst[0] += ctx->table_v.pixels[scanline].factor_f[j] * src[0];\
  dst[1] += ctx->table_v.pixels[scanline].factor_f[j] * src[1];
  
#define SCALE_FINISH                  \
  RECLIP_FLOAT(dst[0], 0);        \
  RECLIP_FLOAT(dst[1], 1);

#include "scale_generic_y.h"

#define FUNC_NAME scale_float_x_3_y_generic_c
#define TYPE float

#define SCALE_INIT dst[0] = 0.0; dst[1] = 0.0; dst[2] = 0.0;

#define SCALE_ACCUM \
  dst[0] += ctx->table_v.pixels[scanline].factor_f[j] * src[0];\
  dst[1] += ctx->table_v.pixels[scanline].factor_f[j] * src[1];\
  dst[2] += ctx->table_v.pixels[scanline].factor_f[j] * src[2];\
  
#define SCALE_FINISH                  \
  RECLIP_FLOAT(dst[0], 0);        \
  RECLIP_FLOAT(dst[1], 1);        \
  RECLIP_FLOAT(dst[2], 2);

#include "scale_generic_y.h"

#define FUNC_NAME scale_float_x_4_y_generic_c
#define TYPE float

#define SCALE_INIT dst[0] = 0.0; dst[1] = 0.0; dst[2] = 0.0;dst[3] = 0.0;

#define SCALE_ACCUM \
  dst[0] += ctx->table_v.pixels[scanline].factor_f[j] * src[0];\
  dst[1] += ctx->table_v.pixels[scanline].factor_f[j] * src[1];\
  dst[2] += ctx->table_v.pixels[scanline].factor_f[j] * src[2];\
  dst[3] += ctx->table_v.pixels[scanline].factor_f[j] * src[3];
  
#define SCALE_FINISH                  \
  RECLIP_FLOAT(dst[0], 0);        \
  RECLIP_FLOAT(dst[1], 1);        \
  RECLIP_FLOAT(dst[2], 2);        \
  RECLIP_FLOAT(dst[3], 3);

#include "scale_generic_y.h"

#ifdef NOCLIP
void gavl_init_scale_funcs_generic_noclip_c(gavl_scale_funcs_t * tab)
#else
void gavl_init_scale_funcs_generic_c(gavl_scale_funcs_t * tab)
#endif
  {
  //  fprintf(stderr, "gavl_init_scale_funcs_generic_c\n");
  tab->funcs_x.scale_rgb_15 =     scale_rgb_15_x_generic_c;
  tab->funcs_x.scale_rgb_16 =     scale_rgb_16_x_generic_c;
  tab->funcs_x.scale_uint8_x_1_advance =  scale_uint8_x_1_x_generic_c;
  tab->funcs_x.scale_uint8_x_1_noadvance =  scale_uint8_x_1_x_generic_c;
  tab->funcs_x.scale_uint8_x_2 =  scale_uint8_x_2_x_generic_c;
  tab->funcs_x.scale_uint8_x_3 =  scale_uint8_x_3_x_generic_c;
  tab->funcs_x.scale_uint8_x_4 =  scale_uint8_x_4_x_generic_c;
  tab->funcs_x.scale_uint16_x_1 = scale_uint16_x_1_x_generic_c;
  tab->funcs_x.scale_uint16_x_2 = scale_uint16_x_2_x_generic_c;
  tab->funcs_x.scale_uint16_x_3 = scale_uint16_x_3_x_generic_c;
  tab->funcs_x.scale_uint16_x_4 = scale_uint16_x_4_x_generic_c;
  tab->funcs_x.scale_float_x_1 =  scale_float_x_1_x_generic_c;
  tab->funcs_x.scale_float_x_2 =  scale_float_x_2_x_generic_c;
  tab->funcs_x.scale_float_x_3 =  scale_float_x_3_x_generic_c;
  tab->funcs_x.scale_float_x_4 =  scale_float_x_4_x_generic_c;
  tab->funcs_x.bits_rgb_15 = 16;
  tab->funcs_x.bits_rgb_16 = 16;
  tab->funcs_x.bits_uint8_advance  = 16;
  tab->funcs_x.bits_uint8_noadvance  = 16;
  tab->funcs_x.bits_uint16 = 16;

  tab->funcs_y.scale_rgb_15 =     scale_rgb_15_y_generic_c;
  tab->funcs_y.scale_rgb_16 =     scale_rgb_16_y_generic_c;
  tab->funcs_y.scale_uint8_x_1_advance =  scale_uint8_x_1_y_generic_c;
  tab->funcs_y.scale_uint8_x_1_noadvance =  scale_uint8_x_1_y_generic_c;
  tab->funcs_y.scale_uint8_x_2 =  scale_uint8_x_2_y_generic_c;
  tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_3_y_generic_c;
  tab->funcs_y.scale_uint8_x_4 =  scale_uint8_x_4_y_generic_c;
  tab->funcs_y.scale_uint16_x_1 = scale_uint16_x_1_y_generic_c;
  tab->funcs_y.scale_uint16_x_2 = scale_uint16_x_2_y_generic_c;
  tab->funcs_y.scale_uint16_x_3 = scale_uint16_x_3_y_generic_c;
  tab->funcs_y.scale_uint16_x_4 = scale_uint16_x_4_y_generic_c;
  tab->funcs_y.scale_float_x_1 =  scale_float_x_1_y_generic_c;
  tab->funcs_y.scale_float_x_2 =  scale_float_x_2_y_generic_c;
  tab->funcs_y.scale_float_x_3 =  scale_float_x_3_y_generic_c;
  tab->funcs_y.scale_float_x_4 =  scale_float_x_4_y_generic_c;
  tab->funcs_y.bits_rgb_15 = 16;
  tab->funcs_y.bits_rgb_16 = 16;
  tab->funcs_y.bits_uint8_advance  = 16;
  tab->funcs_y.bits_uint8_noadvance  = 16;
  tab->funcs_y.bits_uint16 = 16;
  
  }
