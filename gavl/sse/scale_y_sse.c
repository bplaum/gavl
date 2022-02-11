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
#include <attributes.h>

#include <stdio.h>
#include <gavl/gavl.h>
#include <video.h>
#include <scale.h>

// #include "../mmx/mmx.h"
#include "../sse/sse.h"

#if 0
static mmx_t mm_tmp;
#define DUMP_MM(name, reg) MOVQ_R2M(reg, mm_tmp);\
  fprintf(stderr, "%s: %016llx\n", name, mm_tmp.q);
#endif

#define INIT_FLOAT_GLOBAL_4 \
  movups_m2r(ctx->min_values_f[0], xmm5);\
  movups_m2r(ctx->max_values_f[0], xmm6);

#define INIT_FLOAT_GLOBAL_2 \
  movups_m2r(ctx->min_values_f[0], xmm5);\
  movups_m2r(ctx->max_values_f[0], xmm6);\
  shufps_r2ri(xmm5, xmm5, 0x44);\
  shufps_r2ri(xmm6, xmm6, 0x44);

#define INIT_FLOAT_GLOBAL_1 \
  movups_m2r(ctx->min_values_f[0], xmm5);\
  movups_m2r(ctx->max_values_f[0], xmm6);\
  shufps_r2ri(xmm5, xmm5, 0x00);\
  shufps_r2ri(xmm6, xmm6, 0x00);

#define INIT_FLOAT \
  xorps_r2r(xmm3, xmm3); \
  xorps_r2r(xmm4, xmm4);

#define ACCUM_FLOAT(num)  \
  /* Load input */ \
  movaps_m2r(*src,xmm1);\
  movaps_m2r(*(src+16),xmm2);\
  /* Load factor */ \
  movss_m2r(ctx->table_v.pixels[scanline].factor_f[num], xmm0);\
  shufps_r2ri(xmm0, xmm0, 0x00);\
  /* Accumulate xmm0 */ \
  mulps_r2r(xmm0, xmm1);\
  addps_r2r(xmm1, xmm3);\
  /* Accumulate xmm1 */ \
  mulps_r2r(xmm0, xmm2);\
  addps_r2r(xmm2, xmm4)


#define OUTPUT_FLOAT_NOCLIP \
  movups_r2m(xmm3, *dst);\
  movups_r2m(xmm4, *(dst+16));\


#define OUTPUT_FLOAT \
  minps_r2r(xmm6, xmm3);\
  minps_r2r(xmm6, xmm4);\
  maxps_r2r(xmm5, xmm3);\
  maxps_r2r(xmm5, xmm4);\
  movups_r2m(xmm3, *dst);\
  movups_r2m(xmm4, *(dst+16));

#define INIT_C_FLOAT \
  xorps_r2r(xmm2, xmm2);

#define ACCUM_C_FLOAT(num) \
   movss_m2r(*src, xmm1);\
   mulss_m2r(ctx->table_v.pixels[scanline].factor_f[num], xmm1);\
   addss_r2r(xmm1, xmm2);\


#define OUTPUT_C_FLOAT \
   minss_r2r(xmm6, xmm2);\
   maxss_r2r(xmm5, xmm2);\
   movss_r2m(xmm2, *dst);

#define OUTPUT_C_FLOAT_NOCLIP \
   movss_r2m(xmm2, *dst);


/* float */

/* scale_float_x_1_y_bicubic_sse  */

#define FUNC_NAME scale_float_x_1_y_bicubic_sse
#define WIDTH_MUL 1
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_FLOAT_GLOBAL_1
#define INIT        INIT_FLOAT
#define ACCUM       ACCUM_FLOAT
#define OUTPUT      OUTPUT_FLOAT
#define INIT_C        INIT_C_FLOAT
#define ACCUM_C       ACCUM_C_FLOAT
#define OUTPUT_C      OUTPUT_C_FLOAT

#include "../sse/scale_y.h"

/* scale_float_x_1_y_bicubic_noclip_sse  */

#define FUNC_NAME scale_float_x_1_y_bicubic_noclip_sse
#define WIDTH_MUL 1
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_FLOAT_GLOBAL_1
#define INIT        INIT_FLOAT
#define ACCUM       ACCUM_FLOAT
#define OUTPUT      OUTPUT_FLOAT_NOCLIP
#define INIT_C      INIT_C_FLOAT
#define ACCUM_C     ACCUM_C_FLOAT
#define OUTPUT_C    OUTPUT_C_FLOAT_NOCLIP

#include "../sse/scale_y.h"

/* scale_float_x_2_y_bicubic_sse  */

#define FUNC_NAME scale_float_x_2_y_bicubic_sse
#define WIDTH_MUL 2
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_FLOAT_GLOBAL_2
#define INIT        INIT_FLOAT
#define ACCUM       ACCUM_FLOAT
#define OUTPUT      OUTPUT_FLOAT
#define INIT_C      INIT_C_FLOAT
#define ACCUM_C     ACCUM_C_FLOAT
#define OUTPUT_C    OUTPUT_C_FLOAT

#include "../sse/scale_y.h"

/* scale_float_x_2_y_bicubic_noclip_sse  */

#define FUNC_NAME scale_float_x_2_y_bicubic_noclip_sse
#define WIDTH_MUL 2
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_FLOAT_GLOBAL_2
#define INIT        INIT_FLOAT
#define ACCUM       ACCUM_FLOAT
#define OUTPUT      OUTPUT_FLOAT_NOCLIP
#define INIT_C        INIT_C_FLOAT
#define ACCUM_C       ACCUM_C_FLOAT
#define OUTPUT_C      OUTPUT_C_FLOAT_NOCLIP

#include "../sse/scale_y.h"


/* scale_float_x_4_y_bicubic_sse  */

#define FUNC_NAME scale_float_x_4_y_bicubic_sse
#define WIDTH_MUL 4
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_FLOAT_GLOBAL_4
#define INIT        INIT_FLOAT
#define ACCUM       ACCUM_FLOAT
#define OUTPUT      OUTPUT_FLOAT
#define INIT_C        INIT_C_FLOAT
#define ACCUM_C       ACCUM_C_FLOAT
#define OUTPUT_C      OUTPUT_C_FLOAT

#include "../sse/scale_y.h"

/* scale_float_x_4_y_bicubic_noclip_sse  */

#define FUNC_NAME scale_float_x_4_y_bicubic_noclip_sse
#define WIDTH_MUL 4
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_FLOAT_GLOBAL_4
#define INIT        INIT_FLOAT
#define ACCUM       ACCUM_FLOAT
#define OUTPUT      OUTPUT_FLOAT_NOCLIP
#define INIT_C        INIT_C_FLOAT
#define ACCUM_C       ACCUM_C_FLOAT
#define OUTPUT_C      OUTPUT_C_FLOAT

#include "../sse/scale_y.h"

/* */

/* scale_float_x_1_y_quadratic_sse  */

#define FUNC_NAME scale_float_x_1_y_quadratic_sse
#define WIDTH_MUL 1
#define NUM_TAPS 3

#define INIT_GLOBAL INIT_FLOAT_GLOBAL_1
#define INIT        INIT_FLOAT
#define ACCUM       ACCUM_FLOAT
#define OUTPUT      OUTPUT_FLOAT_NOCLIP
#define INIT_C        INIT_C_FLOAT
#define ACCUM_C       ACCUM_C_FLOAT
#define OUTPUT_C      OUTPUT_C_FLOAT_NOCLIP

#include "../sse/scale_y.h"

/* scale_float_x_2_y_quadratic_sse  */

#define FUNC_NAME scale_float_x_2_y_quadratic_sse
#define WIDTH_MUL 2
#define NUM_TAPS 3

#define INIT_GLOBAL INIT_FLOAT_GLOBAL_2
#define INIT        INIT_FLOAT
#define ACCUM       ACCUM_FLOAT
#define OUTPUT      OUTPUT_FLOAT_NOCLIP
#define INIT_C        INIT_C_FLOAT
#define ACCUM_C       ACCUM_C_FLOAT
#define OUTPUT_C      OUTPUT_C_FLOAT_NOCLIP

#include "../sse/scale_y.h"


/* scale_float_x_4_y_quadratic_sse  */

#define FUNC_NAME scale_float_x_4_y_quadratic_sse
#define WIDTH_MUL 4
#define NUM_TAPS 3

#define INIT_GLOBAL INIT_FLOAT_GLOBAL_4
#define INIT        INIT_FLOAT
#define ACCUM       ACCUM_FLOAT
#define OUTPUT      OUTPUT_FLOAT_NOCLIP
#define INIT_C        INIT_C_FLOAT
#define ACCUM_C       ACCUM_C_FLOAT
#define OUTPUT_C      OUTPUT_C_FLOAT_NOCLIP

#include "../sse/scale_y.h"


/* scale_float_x_1_y_generic_sse  */

#define FUNC_NAME scale_float_x_1_y_generic_sse
#define WIDTH_MUL 1
#define NUM_TAPS -1

#define INIT_GLOBAL INIT_FLOAT_GLOBAL_1
#define INIT        INIT_FLOAT
#define ACCUM       ACCUM_FLOAT
#define OUTPUT      OUTPUT_FLOAT
#define INIT_C        INIT_C_FLOAT
#define ACCUM_C       ACCUM_C_FLOAT
#define OUTPUT_C      OUTPUT_C_FLOAT

#include "../sse/scale_y.h"

/* scale_float_x_2_y_generic_sse  */

#define FUNC_NAME scale_float_x_2_y_generic_sse
#define WIDTH_MUL 2
#define NUM_TAPS -1

#define INIT_GLOBAL INIT_FLOAT_GLOBAL_2
#define INIT        INIT_FLOAT
#define ACCUM       ACCUM_FLOAT
#define OUTPUT      OUTPUT_FLOAT
#define INIT_C        INIT_C_FLOAT
#define ACCUM_C       ACCUM_C_FLOAT
#define OUTPUT_C      OUTPUT_C_FLOAT

#include "../sse/scale_y.h"


/* scale_float_x_4_y_generic_sse  */

#define FUNC_NAME scale_float_x_4_y_generic_sse
#define WIDTH_MUL 4
#define NUM_TAPS -1

#define INIT_GLOBAL INIT_FLOAT_GLOBAL_4
#define INIT        INIT_FLOAT
#define ACCUM       ACCUM_FLOAT
#define OUTPUT      OUTPUT_FLOAT
#define INIT_C        INIT_C_FLOAT
#define ACCUM_C       ACCUM_C_FLOAT
#define OUTPUT_C      OUTPUT_C_FLOAT

#include "../sse/scale_y.h"

void gavl_init_scale_funcs_quadratic_y_sse(gavl_scale_funcs_t * tab,
                                           int src_advance, int dst_advance)
  {
  tab->funcs_y.scale_float_x_1 =  scale_float_x_1_y_quadratic_sse;
  tab->funcs_y.scale_float_x_2 =  scale_float_x_2_y_quadratic_sse;
  tab->funcs_y.scale_float_x_4 =  scale_float_x_4_y_quadratic_sse;
  }
  
void gavl_init_scale_funcs_bicubic_y_sse(gavl_scale_funcs_t * tab,
                                         int src_advance, int dst_advance)
  {
  tab->funcs_y.scale_float_x_1 =  scale_float_x_1_y_bicubic_sse;
  tab->funcs_y.scale_float_x_2 =  scale_float_x_2_y_bicubic_sse;
  tab->funcs_y.scale_float_x_4 =  scale_float_x_4_y_bicubic_sse;
  }

void gavl_init_scale_funcs_bicubic_y_noclip_sse(gavl_scale_funcs_t * tab,
                                                 int src_advance, int dst_advance)
  {
  tab->funcs_y.scale_float_x_1 =  scale_float_x_1_y_bicubic_noclip_sse;
  tab->funcs_y.scale_float_x_2 =  scale_float_x_2_y_bicubic_noclip_sse;
  tab->funcs_y.scale_float_x_4 =  scale_float_x_4_y_bicubic_noclip_sse;
  }

void gavl_init_scale_funcs_generic_y_sse(gavl_scale_funcs_t * tab,
                                         int src_advance, int dst_advance)
  {
  tab->funcs_y.scale_float_x_1 =  scale_float_x_1_y_generic_sse;
  tab->funcs_y.scale_float_x_2 =  scale_float_x_2_y_generic_sse;
  tab->funcs_y.scale_float_x_4 =  scale_float_x_4_y_generic_sse;
  }

/* scale_uint8_x_1_y_bilinear_sse  */

#define FUNC_NAME scale_float_x_1_y_bilinear_sse
#define WIDTH_MUL 1

#include "scale_y_linear.h"

/* scale_float_x_2_y_bilinear_sse  */

#define FUNC_NAME scale_float_x_2_y_bilinear_sse
#define WIDTH_MUL 2

#include "scale_y_linear.h"

/* scale_float_x_4_y_bilinear_sse  */

#define FUNC_NAME scale_float_x_4_y_bilinear_sse
#define WIDTH_MUL 4

#include "scale_y_linear.h"

/* scale_float_x_3_y_bilinear_sse  */

#define FUNC_NAME scale_float_x_3_y_bilinear_sse
#define WIDTH_MUL 3

#include "scale_y_linear.h"

void gavl_init_scale_funcs_bilinear_y_sse(gavl_scale_funcs_t * tab,
                                         int src_advance, int dst_advance)
  {
#if 1 // Too slow
  tab->funcs_y.scale_float_x_1 =  scale_float_x_1_y_bilinear_sse;
  tab->funcs_y.scale_float_x_2 =  scale_float_x_2_y_bilinear_sse;
  tab->funcs_y.scale_float_x_3 =  scale_float_x_3_y_bilinear_sse;
  tab->funcs_y.scale_float_x_4 =  scale_float_x_4_y_bilinear_sse;
  
#endif
  }
