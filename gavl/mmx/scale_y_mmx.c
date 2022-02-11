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

#include "mmx.h"

static const mmx_t factor_mask = { 0x000000000000FFFFLL };

#if 0
static mmx_t mm_tmp;
#define DUMP_MM(name, reg) MOVQ_R2M(reg, mm_tmp);\
  fprintf(stderr, "%s: %016llx\n", name, mm_tmp.q);
#endif

#ifdef MMXEXT
#define MOVQ_R2M(reg,mem) movntq_r2m(reg, mem)
#else
#define MOVQ_R2M(reg,mem) movq_r2m(reg, mem)
#endif

/*
 *  mm0: Input1
 *  mm1: Input2
 *  mm2: Factor1
 *  mm3: Factor1
 *  mm4: Output1
 *  mm5: Output2
 *  mm6: Scratch
 *  mm7: factor_mask
 */

#define INIT_8_GLOBAL \
  pxor_r2r(mm6, mm6);\
  movq_m2r(factor_mask, mm7);

#define INIT_8 \
  pxor_r2r(mm3, mm3);\
  pxor_r2r(mm4, mm4);

#ifdef MMXEXT
#define LOAD_FACTOR_8(num) \
  /* Load factor */ \
  movd_m2r(ctx->table_v.pixels[scanline].factor_i[num], mm2);\
  pand_r2r(mm7, mm2);\
  pshufw_r2r(mm2,mm5,0x00)

#else

#define LOAD_FACTOR_8(num) \
  /* Load factor */ \
  movd_m2r(ctx->table_v.pixels[scanline].factor_i[num], mm2);\
  pand_r2r(mm7, mm2);\
  movq_r2r(mm2, mm5);\
  psllq_i2r(16, mm5);\
  por_r2r(mm5, mm2);\
  movq_r2r(mm2, mm5);\
  psllq_i2r(32, mm5);\
  por_r2r(mm2, mm5)
#endif  

#define ACCUM_8(num)  \
  /* Load input */ \
  movq_m2r(*src,mm0);\
  movq_r2r(mm0,mm1);\
  punpcklbw_r2r(mm6, mm0); \
  punpckhbw_r2r(mm6, mm1); \
  psllw_i2r(7, mm0);\
  psllw_i2r(7, mm1);\
  LOAD_FACTOR_8(num); \
  /* Accumulate mm0 */ \
  pmulhw_r2r(mm5, mm0);\
  paddsw_r2r(mm0, mm3);\
  /* Accumulate mm1 */ \
  pmulhw_r2r(mm5, mm1);\
  paddsw_r2r(mm1, mm4)

#define OUTPUT_8 \
  psraw_i2r(5, mm3);\
  psraw_i2r(5, mm4);\
  packuswb_r2r(mm4, mm3);\
  MOVQ_R2M(mm3, *dst)

#define ACCUM_C_8(num) \
   tmp += ctx->table_v.pixels[scanline].factor_i[num] * *src

#define OUTPUT_C_8 \
   tmp >>= 14; \
   *dst = (uint8_t)((tmp & ~0xFF)?((-tmp) >> 63) : tmp);

/* scale_uint8_x_1_y_bicubic_mmx  */

#define FUNC_NAME scale_uint8_x_1_y_bicubic_mmx
#define WIDTH_MUL 1
#define BITS 8
#define NUM_TAPS 4

#include "scale_y.h"

/* scale_uint8_x_2_y_bicubic_mmx  */

#define FUNC_NAME scale_uint8_x_2_y_bicubic_mmx
#define WIDTH_MUL 2
#define BITS 8
#define NUM_TAPS 4

#include "scale_y.h"

/* scale_uint8_x_3_y_bicubic_mmx  */

#define FUNC_NAME scale_uint8_x_3_y_bicubic_mmx
#define WIDTH_MUL 3
#define BITS 8
#define NUM_TAPS 4

#include "scale_y.h"

/* scale_uint8_x_4_y_bicubic_mmx  */

#define FUNC_NAME scale_uint8_x_4_y_bicubic_mmx
#define WIDTH_MUL 4
#define BITS 8
#define NUM_TAPS 4

#include "scale_y.h"


/* scale_uint8_x_1_y_quadratic_mmx  */

#define FUNC_NAME scale_uint8_x_1_y_quadratic_mmx
#define WIDTH_MUL 1
#define BITS 8
#define NUM_TAPS 3

#include "scale_y.h"

/* scale_uint8_x_2_y_quadratic_mmx  */

#define FUNC_NAME scale_uint8_x_2_y_quadratic_mmx
#define WIDTH_MUL 2
#define BITS 8
#define NUM_TAPS 3

#include "scale_y.h"

/* scale_uint8_x_3_y_quadratic_mmx  */

#define FUNC_NAME scale_uint8_x_3_y_quadratic_mmx
#define WIDTH_MUL 3
#define BITS 8
#define NUM_TAPS 3

#include "scale_y.h"

/* scale_uint8_x_4_y_quadratic_mmx  */

#define FUNC_NAME scale_uint8_x_4_y_quadratic_mmx
#define WIDTH_MUL 4
#define BITS 8
#define NUM_TAPS 3

#include "scale_y.h"

/* scale_uint8_x_1_y_generic_mmx  */

#define FUNC_NAME scale_uint8_x_1_y_generic_mmx
#define WIDTH_MUL 1
#define BITS 8
#define NUM_TAPS -1

#include "scale_y.h"

/* scale_uint8_x_2_y_generic_mmx  */

#define FUNC_NAME scale_uint8_x_2_y_generic_mmx
#define WIDTH_MUL 2
#define BITS 8
#define NUM_TAPS -1

#include "scale_y.h"

/* scale_uint8_x_4_y_generic_mmx  */

#define FUNC_NAME scale_uint8_x_4_y_generic_mmx
#define WIDTH_MUL 4
#define BITS 8
#define NUM_TAPS -1

#include "scale_y.h"

/* scale_uint8_x_3_y_generic_mmx  */

#define FUNC_NAME scale_uint8_x_3_y_generic_mmx
#define WIDTH_MUL 3
#define BITS 8
#define NUM_TAPS -1

#include "scale_y.h"

#ifdef MMXEXT
void gavl_init_scale_funcs_quadratic_y_mmxext(gavl_scale_funcs_t * tab,
                                           int src_advance, int dst_advance)
#else
void gavl_init_scale_funcs_quadratic_y_mmx(gavl_scale_funcs_t * tab,
                                           int src_advance, int dst_advance)
#endif
  {
  if((src_advance == 1) && (dst_advance == 1))
    {
    tab->funcs_y.scale_uint8_x_1_noadvance =  scale_uint8_x_1_y_quadratic_mmx;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 3) && (dst_advance == 3))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_3_y_quadratic_mmx;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 4) && (dst_advance == 4))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_4_y_quadratic_mmx;
    tab->funcs_y.scale_uint8_x_4 =  scale_uint8_x_4_y_quadratic_mmx;
    tab->funcs_y.bits_uint8_noadvance  = 14;
    }
  else if((src_advance == 2) && (dst_advance == 2))
    {
    tab->funcs_y.scale_uint8_x_2 =  scale_uint8_x_2_y_quadratic_mmx;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  }
  
#ifdef MMXEXT
void gavl_init_scale_funcs_bicubic_y_mmxext(gavl_scale_funcs_t * tab,
                                            int src_advance, int dst_advance)
#else
void gavl_init_scale_funcs_bicubic_y_mmx(gavl_scale_funcs_t * tab,
                                         int src_advance, int dst_advance)
#endif
  {
  if((src_advance == 1) && (dst_advance == 1))
    {
    tab->funcs_y.scale_uint8_x_1_noadvance =  scale_uint8_x_1_y_bicubic_mmx;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 3) && (dst_advance == 3))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_3_y_bicubic_mmx;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 4) && (dst_advance == 4))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_4_y_bicubic_mmx;
    tab->funcs_y.scale_uint8_x_4 =  scale_uint8_x_4_y_bicubic_mmx;
    tab->funcs_y.bits_uint8_noadvance  = 14;
    }
  else if((src_advance == 2) && (dst_advance == 2))
    {
    tab->funcs_y.scale_uint8_x_2 =  scale_uint8_x_2_y_bicubic_mmx;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  }

#ifdef MMXEXT
void gavl_init_scale_funcs_generic_y_mmxext(gavl_scale_funcs_t * tab,
                                            int src_advance, int dst_advance)
#else
void gavl_init_scale_funcs_generic_y_mmx(gavl_scale_funcs_t * tab,
                                         int src_advance, int dst_advance)
#endif
  {
  if((src_advance == 1) && (dst_advance == 1))
    {
    tab->funcs_y.scale_uint8_x_1_noadvance =  scale_uint8_x_1_y_generic_mmx;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 3) && (dst_advance == 3))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_3_y_generic_mmx;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 2) && (dst_advance == 2))
    {
    tab->funcs_y.scale_uint8_x_2 =  scale_uint8_x_2_y_generic_mmx;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 4) && (dst_advance == 4))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_4_y_generic_mmx;
    tab->funcs_y.scale_uint8_x_4 =  scale_uint8_x_4_y_generic_mmx;
    tab->funcs_y.bits_uint8_noadvance  = 14;
    }
  
  }

/* scale_uint8_x_1_y_bilinear_mmx  */

#define FUNC_NAME scale_uint8_x_1_y_bilinear_mmx
#define WIDTH_MUL 1
#define BITS 8

#include "scale_y_linear.h"

/* scale_uint8_x_2_y_bilinear_mmx  */

#define FUNC_NAME scale_uint8_x_2_y_bilinear_mmx
#define WIDTH_MUL 2
#define BITS 8

#include "scale_y_linear.h"

/* scale_uint8_x_4_y_bilinear_mmx  */

#define FUNC_NAME scale_uint8_x_4_y_bilinear_mmx
#define WIDTH_MUL 4
#define BITS 8

#include "scale_y_linear.h"

/* scale_uint8_x_3_y_bilinear_mmx  */

#define FUNC_NAME scale_uint8_x_3_y_bilinear_mmx
#define WIDTH_MUL 3
#define BITS 8

#include "scale_y_linear.h"

#ifdef MMXEXT
void gavl_init_scale_funcs_bilinear_y_mmxext(gavl_scale_funcs_t * tab,
                                             int src_advance, int dst_advance)
#else
void gavl_init_scale_funcs_bilinear_y_mmx(gavl_scale_funcs_t * tab,
                                         int src_advance, int dst_advance)
#endif
  {
  if((src_advance == 1) && (dst_advance == 1))
    {
    tab->funcs_y.scale_uint8_x_1_noadvance =  scale_uint8_x_1_y_bilinear_mmx;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 3) && (dst_advance == 3))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_3_y_bilinear_mmx;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 2) && (dst_advance == 2))
    {
    tab->funcs_y.scale_uint8_x_2 =  scale_uint8_x_2_y_bilinear_mmx;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 4) && (dst_advance == 4))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_4_y_bilinear_mmx;
    tab->funcs_y.scale_uint8_x_4 =  scale_uint8_x_4_y_bilinear_mmx;
    tab->funcs_y.bits_uint8_noadvance  = 14;
    }
  
  }
