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

#include "../mmx/mmx.h"
#include "../sse/sse.h"

static const sse_t factor_mask = { .uw = { 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 } };

static const sse_t min_13 = { .uw = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 } };
static const sse_t max_13 = { .uw = { 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF } };

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
 *  xmm0: Input1
 *  xmm1: Input2
 *  xmm2: Factor1
 *  xmm3: Factor1
 *  xmm4: Output1
 *  xmm5: Output2
 *  xmm6: Scratch
 *  xmm7: factor_mask
 */

#define INIT_8_GLOBAL \
  pxor_r2r(xmm6, xmm6);\
  movaps_m2r(factor_mask, xmm7);

#define INIT_8 \
  pxor_r2r(xmm3, xmm3);\
  pxor_r2r(xmm4, xmm4);

#define INIT_16_GLOBAL \
  pxor_r2r(xmm6, xmm6);\
  movaps_m2r(factor_mask, xmm7);

#define INIT_16 \
  pxor_r2r(xmm3, xmm3);\
  pxor_r2r(xmm4, xmm4);

#define LOAD_FACTOR_8(num) \
  /* Load factor */ \
  movd_m2r(ctx->table_v.pixels[scanline].factor_i[num], xmm2);\
  pand_r2r(xmm7, xmm2);\
  pshuflw_r2ri(xmm2,xmm5,0x00);\
  pshufd_r2ri(xmm5,xmm5,0x00);

#define LOAD_FACTOR_16(num) \
  /* Load factor */ \
  movd_m2r(ctx->table_v.pixels[scanline].factor_i[num], xmm2);\
  pand_r2r(xmm7, xmm2);\
  pshuflw_r2ri(xmm2,xmm5,0x00);\
  pshufd_r2ri(xmm5,xmm5,0x00);
  

#define ACCUM_8(num)  \
  /* Load input */ \
  movaps_m2r(*src,xmm0);\
  movaps_r2r(xmm0,xmm1);\
  punpcklbw_r2r(xmm6, xmm0); \
  punpckhbw_r2r(xmm6, xmm1); \
  psllw_i2r(7, xmm0);\
  psllw_i2r(7, xmm1);\
  LOAD_FACTOR_8(num); \
  /* Accumulate xmm0 */ \
  pmulhw_r2r(xmm5, xmm0);\
  paddsw_r2r(xmm0, xmm3);\
  /* Accumulate xmm1 */ \
  pmulhw_r2r(xmm5, xmm1);\
  paddsw_r2r(xmm1, xmm4)

#define ACCUM_16(num)  \
  /* Load input */ \
  movaps_m2r(*src,xmm0);\
  movaps_m2r(*(src+16),xmm1);\
  psrlw_i2r(1, xmm0);\
  psrlw_i2r(1, xmm1);\
  LOAD_FACTOR_16(num); \
  /* Accumulate xmm0 */ \
  pmulhw_r2r(xmm5, xmm0);\
  paddsw_r2r(xmm0, xmm3);\
  /* Accumulate xmm1 */ \
  pmulhw_r2r(xmm5, xmm1);\
  paddsw_r2r(xmm1, xmm4)

#define OUTPUT_8 \
  psraw_i2r(5, xmm3);\
  psraw_i2r(5, xmm4);\
  packuswb_r2r(xmm4, xmm3);\
  movups_r2m(xmm3, *dst)

#define OUTPUT_16_NOCLIP \
  psllw_i2r(3, xmm3);\
  psllw_i2r(3, xmm4);\
  movups_r2m(xmm3, *dst);\
  movups_r2m(xmm4, *(dst+16));\

#define OUTPUT_16 \
  pminsw_m2r(max_13, xmm3);\
  pminsw_m2r(max_13, xmm4);\
  pmaxsw_m2r(min_13, xmm3);\
  pmaxsw_m2r(min_13, xmm4);\
  psllw_i2r(3, xmm3);\
  psllw_i2r(3, xmm4);\
  movups_r2m(xmm3, *dst);\
  movups_r2m(xmm4, *(dst+16));\

#define INIT_C_8 \
  tmp = 0;

#define INIT_C_16 \
  tmp = 0;

#define ACCUM_C_8(num) \
   tmp += ctx->table_v.pixels[scanline].factor_i[num] * *src

#define ACCUM_C_16(num) \
   tmp += ctx->table_v.pixels[scanline].factor_i[num] * *(uint16_t*)src

#define OUTPUT_C_8 \
   tmp >>= 14; \
   *dst = (uint8_t)((tmp & ~0xFF)?((-tmp) >> 31) : tmp);

#define OUTPUT_C_16 \
   tmp >>= 14; \
   *(uint16_t*)dst = (uint16_t)((tmp & ~0xFFFF)?((-tmp) >> 31) : tmp);

/* scale_uint8_x_1_y_bicubic_sse2  */

#define FUNC_NAME scale_uint8_x_1_y_bicubic_sse2
#define WIDTH_MUL 1
#define BYTES 1
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_8_GLOBAL
#define INIT        INIT_8
#define ACCUM       ACCUM_8
#define OUTPUT      OUTPUT_8
#define INIT_C        INIT_C_8
#define ACCUM_C       ACCUM_C_8
#define OUTPUT_C      OUTPUT_C_8

#include "scale_y.h"

/* scale_uint8_x_2_y_bicubic_sse  */

#define FUNC_NAME scale_uint8_x_2_y_bicubic_sse2
#define WIDTH_MUL 2
#define BYTES 1
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_8_GLOBAL
#define INIT        INIT_8
#define ACCUM       ACCUM_8
#define OUTPUT      OUTPUT_8
#define INIT_C        INIT_C_8
#define ACCUM_C       ACCUM_C_8
#define OUTPUT_C      OUTPUT_C_8

#include "scale_y.h"

/* scale_uint8_x_3_y_bicubic_sse2  */

#define FUNC_NAME scale_uint8_x_3_y_bicubic_sse2
#define WIDTH_MUL 3
#define BYTES 1
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_8_GLOBAL
#define INIT        INIT_8
#define ACCUM       ACCUM_8
#define OUTPUT      OUTPUT_8
#define INIT_C        INIT_C_8
#define ACCUM_C       ACCUM_C_8
#define OUTPUT_C      OUTPUT_C_8

#include "scale_y.h"

/* scale_uint8_x_4_y_bicubic_sse2  */

#define FUNC_NAME scale_uint8_x_4_y_bicubic_sse2
#define WIDTH_MUL 4
#define BYTES 1
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_8_GLOBAL
#define INIT        INIT_8
#define ACCUM       ACCUM_8
#define OUTPUT      OUTPUT_8
#define INIT_C        INIT_C_8
#define ACCUM_C       ACCUM_C_8
#define OUTPUT_C      OUTPUT_C_8

#include "scale_y.h"


/* scale_uint8_x_1_y_quadratic_sse2  */

#define FUNC_NAME scale_uint8_x_1_y_quadratic_sse2
#define WIDTH_MUL 1
#define BYTES 1
#define NUM_TAPS 3

#define INIT_GLOBAL INIT_8_GLOBAL
#define INIT        INIT_8
#define ACCUM       ACCUM_8
#define OUTPUT      OUTPUT_8
#define INIT_C        INIT_C_8
#define ACCUM_C       ACCUM_C_8
#define OUTPUT_C      OUTPUT_C_8

#include "scale_y.h"

/* scale_uint8_x_2_y_quadratic_sse2  */

#define FUNC_NAME scale_uint8_x_2_y_quadratic_sse2
#define WIDTH_MUL 2
#define BYTES 1
#define NUM_TAPS 3

#define INIT_GLOBAL INIT_8_GLOBAL
#define INIT        INIT_8
#define ACCUM       ACCUM_8
#define OUTPUT      OUTPUT_8
#define INIT_C        INIT_C_8
#define ACCUM_C       ACCUM_C_8
#define OUTPUT_C      OUTPUT_C_8

#include "scale_y.h"

/* scale_uint8_x_3_y_quadratic_sse2  */

#define FUNC_NAME scale_uint8_x_3_y_quadratic_sse2
#define WIDTH_MUL 3
#define BYTES 1
#define NUM_TAPS 3

#define INIT_GLOBAL INIT_8_GLOBAL
#define INIT        INIT_8
#define ACCUM       ACCUM_8
#define OUTPUT      OUTPUT_8
#define INIT_C      INIT_C_8
#define ACCUM_C     ACCUM_C_8
#define OUTPUT_C    OUTPUT_C_8

#include "scale_y.h"

/* scale_uint8_x_4_y_quadratic_sse2  */

#define FUNC_NAME scale_uint8_x_4_y_quadratic_sse2
#define WIDTH_MUL 4
#define BYTES 1
#define NUM_TAPS 3

#define INIT_GLOBAL INIT_8_GLOBAL
#define INIT        INIT_8
#define ACCUM       ACCUM_8
#define OUTPUT      OUTPUT_8
#define INIT_C        INIT_C_8
#define ACCUM_C       ACCUM_C_8
#define OUTPUT_C      OUTPUT_C_8

#include "scale_y.h"

/* scale_uint8_x_1_y_generic_sse2  */

#define FUNC_NAME scale_uint8_x_1_y_generic_sse2
#define WIDTH_MUL 1
#define BYTES 1
#define NUM_TAPS -1

#define INIT_GLOBAL INIT_8_GLOBAL
#define INIT        INIT_8
#define ACCUM       ACCUM_8
#define OUTPUT      OUTPUT_8
#define INIT_C      INIT_C_8
#define ACCUM_C     ACCUM_C_8
#define OUTPUT_C    OUTPUT_C_8

#include "scale_y.h"

/* scale_uint8_x_2_y_generic_sse2  */

#define FUNC_NAME scale_uint8_x_2_y_generic_sse2
#define WIDTH_MUL 2
#define BYTES 1
#define NUM_TAPS -1

#define INIT_GLOBAL INIT_8_GLOBAL
#define INIT        INIT_8
#define ACCUM       ACCUM_8
#define OUTPUT      OUTPUT_8
#define INIT_C        INIT_C_8
#define ACCUM_C       ACCUM_C_8
#define OUTPUT_C      OUTPUT_C_8

#include "scale_y.h"

/* scale_uint8_x_4_y_generic_sse2  */

#define FUNC_NAME scale_uint8_x_4_y_generic_sse2
#define WIDTH_MUL 4
#define BYTES 1
#define NUM_TAPS -1

#define INIT_GLOBAL INIT_8_GLOBAL
#define INIT        INIT_8
#define ACCUM       ACCUM_8
#define OUTPUT      OUTPUT_8
#define INIT_C        INIT_C_8
#define ACCUM_C       ACCUM_C_8
#define OUTPUT_C      OUTPUT_C_8

#include "scale_y.h"

/* scale_uint8_x_3_y_generic_sse2  */

#define FUNC_NAME scale_uint8_x_3_y_generic_sse2
#define WIDTH_MUL 3
#define BYTES 1
#define NUM_TAPS -1

#define INIT_GLOBAL INIT_8_GLOBAL
#define INIT        INIT_8
#define ACCUM       ACCUM_8
#define OUTPUT      OUTPUT_8
#define INIT_C        INIT_C_8
#define ACCUM_C       ACCUM_C_8
#define OUTPUT_C      OUTPUT_C_8

#include "scale_y.h"


/* 16 bits */

/* scale_uint16_x_1_y_bicubic_sse2  */

#define FUNC_NAME scale_uint16_x_1_y_bicubic_sse2
#define WIDTH_MUL 1
#define BYTES 2
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* scale_uint16_x_1_y_bicubic_noclip_sse2  */

#define FUNC_NAME scale_uint16_x_1_y_bicubic_noclip_sse2
#define WIDTH_MUL 1
#define BYTES 2
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16_NOCLIP
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* scale_uint16_x_2_y_bicubic_sse2  */

#define FUNC_NAME scale_uint16_x_2_y_bicubic_sse2
#define WIDTH_MUL 2
#define BYTES 2
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* scale_uint16_x_2_y_bicubic_noclip_sse2  */

#define FUNC_NAME scale_uint16_x_2_y_bicubic_noclip_sse2
#define WIDTH_MUL 2
#define BYTES 2
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16_NOCLIP
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* scale_uint16_x_3_y_bicubic_sse2  */

#define FUNC_NAME scale_uint16_x_3_y_bicubic_sse2
#define WIDTH_MUL 3
#define BYTES 2
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* scale_uint16_x_3_y_bicubic_noclip_sse2  */

#define FUNC_NAME scale_uint16_x_3_y_bicubic_noclip_sse2
#define WIDTH_MUL 3
#define BYTES 2
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16_NOCLIP
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* scale_uint16_x_4_y_bicubic_sse2  */

#define FUNC_NAME scale_uint16_x_4_y_bicubic_sse2
#define WIDTH_MUL 4
#define BYTES 2
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* scale_uint16_x_4_y_bicubic_noclip_sse2  */

#define FUNC_NAME scale_uint16_x_4_y_bicubic_noclip_sse2
#define WIDTH_MUL 4
#define BYTES 2
#define NUM_TAPS 4

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16_NOCLIP
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* */

/* scale_uint16_x_1_y_quadratic_sse2  */

#define FUNC_NAME scale_uint16_x_1_y_quadratic_sse2
#define WIDTH_MUL 1
#define BYTES 2
#define NUM_TAPS 3

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16_NOCLIP
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* scale_uint16_x_2_y_quadratic_sse2  */

#define FUNC_NAME scale_uint16_x_2_y_quadratic_sse2
#define WIDTH_MUL 2
#define BYTES 2
#define NUM_TAPS 3

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16_NOCLIP
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* scale_uint16_x_3_y_quadratic_sse2  */

#define FUNC_NAME scale_uint16_x_3_y_quadratic_sse2
#define WIDTH_MUL 3
#define BYTES 2
#define NUM_TAPS 3

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16_NOCLIP
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* scale_uint16_x_4_y_quadratic_sse2  */

#define FUNC_NAME scale_uint16_x_4_y_quadratic_sse2
#define WIDTH_MUL 4
#define BYTES 2
#define NUM_TAPS 3

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16_NOCLIP
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"


/* scale_uint16_x_1_y_generic_sse2  */

#define FUNC_NAME scale_uint16_x_1_y_generic_sse2
#define WIDTH_MUL 1
#define BYTES 2
#define NUM_TAPS -1

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* scale_uint16_x_2_y_generic_sse2  */

#define FUNC_NAME scale_uint16_x_2_y_generic_sse2
#define WIDTH_MUL 2
#define BYTES 2
#define NUM_TAPS -1

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* scale_uint16_x_3_y_generic_sse2  */

#define FUNC_NAME scale_uint16_x_3_y_generic_sse2
#define WIDTH_MUL 3
#define BYTES 2
#define NUM_TAPS -1

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"

/* scale_uint16_x_4_y_generic_sse2  */

#define FUNC_NAME scale_uint16_x_4_y_generic_sse2
#define WIDTH_MUL 4
#define BYTES 2
#define NUM_TAPS -1

#define INIT_GLOBAL INIT_16_GLOBAL
#define INIT        INIT_16
#define ACCUM       ACCUM_16
#define OUTPUT      OUTPUT_16
#define INIT_C        INIT_C_16
#define ACCUM_C       ACCUM_C_16
#define OUTPUT_C      OUTPUT_C_16

#include "scale_y.h"






void gavl_init_scale_funcs_quadratic_y_sse2(gavl_scale_funcs_t * tab,
                                            int src_advance, int dst_advance)
  {
  if((src_advance == 1) && (dst_advance == 1))
    {
    tab->funcs_y.scale_uint8_x_1_noadvance =  scale_uint8_x_1_y_quadratic_sse2;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 3) && (dst_advance == 3))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_3_y_quadratic_sse2;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 4) && (dst_advance == 4))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_4_y_quadratic_sse2;
    tab->funcs_y.scale_uint8_x_4 =  scale_uint8_x_4_y_quadratic_sse2;
    tab->funcs_y.bits_uint8_noadvance  = 14;
    }
  else if((src_advance == 2) && (dst_advance == 2))
    {
    tab->funcs_y.scale_uint8_x_2 =  scale_uint8_x_2_y_quadratic_sse2;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  tab->funcs_y.scale_uint16_x_1 =  scale_uint16_x_1_y_quadratic_sse2;
  tab->funcs_y.scale_uint16_x_2 =  scale_uint16_x_2_y_quadratic_sse2;
  tab->funcs_y.scale_uint16_x_3 =  scale_uint16_x_3_y_quadratic_sse2;
  tab->funcs_y.scale_uint16_x_4 =  scale_uint16_x_4_y_quadratic_sse2;
  tab->funcs_y.bits_uint16 = 14;
  }
  
void gavl_init_scale_funcs_bicubic_y_sse2(gavl_scale_funcs_t * tab,
                                         int src_advance, int dst_advance)
  {
  if((src_advance == 1) && (dst_advance == 1))
    {
    tab->funcs_y.scale_uint8_x_1_noadvance =  scale_uint8_x_1_y_bicubic_sse2;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 3) && (dst_advance == 3))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_3_y_bicubic_sse2;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 4) && (dst_advance == 4))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_4_y_bicubic_sse2;
    tab->funcs_y.scale_uint8_x_4 =  scale_uint8_x_4_y_bicubic_sse2;
    tab->funcs_y.bits_uint8_noadvance  = 14;
    }
  else if((src_advance == 2) && (dst_advance == 2))
    {
    tab->funcs_y.scale_uint8_x_2 =  scale_uint8_x_2_y_bicubic_sse2;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }

  tab->funcs_y.scale_uint16_x_1 =  scale_uint16_x_1_y_bicubic_sse2;
  tab->funcs_y.scale_uint16_x_2 =  scale_uint16_x_2_y_bicubic_sse2;
  tab->funcs_y.scale_uint16_x_3 =  scale_uint16_x_3_y_bicubic_sse2;
  tab->funcs_y.scale_uint16_x_4 =  scale_uint16_x_4_y_bicubic_sse2;
  tab->funcs_y.bits_uint16 = 14;
  }

void gavl_init_scale_funcs_bicubic_y_noclip_sse2(gavl_scale_funcs_t * tab,
                                                 int src_advance, int dst_advance)
  {
  if((src_advance == 1) && (dst_advance == 1))
    {
    tab->funcs_y.scale_uint8_x_1_noadvance =  scale_uint8_x_1_y_bicubic_sse2;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 3) && (dst_advance == 3))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_3_y_bicubic_sse2;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 4) && (dst_advance == 4))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_4_y_bicubic_sse2;
    tab->funcs_y.scale_uint8_x_4 =  scale_uint8_x_4_y_bicubic_sse2;
    tab->funcs_y.bits_uint8_noadvance  = 14;
    }
  else if((src_advance == 2) && (dst_advance == 2))
    {
    tab->funcs_y.scale_uint8_x_2 =  scale_uint8_x_2_y_bicubic_sse2;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }

  tab->funcs_y.scale_uint16_x_1 =  scale_uint16_x_1_y_bicubic_noclip_sse2;
  tab->funcs_y.scale_uint16_x_2 =  scale_uint16_x_2_y_bicubic_noclip_sse2;
  tab->funcs_y.scale_uint16_x_3 =  scale_uint16_x_3_y_bicubic_noclip_sse2;
  tab->funcs_y.scale_uint16_x_4 =  scale_uint16_x_4_y_bicubic_noclip_sse2;
  tab->funcs_y.bits_uint16 = 14;
  }

#ifdef MMXEXT
void gavl_init_scale_funcs_generic_y_mmxext(gavl_scale_funcs_t * tab,
                                            int src_advance, int dst_advance)
#else
void gavl_init_scale_funcs_generic_y_sse2(gavl_scale_funcs_t * tab,
                                         int src_advance, int dst_advance)
#endif
  {
  if((src_advance == 1) && (dst_advance == 1))
    {
    tab->funcs_y.scale_uint8_x_1_noadvance = scale_uint8_x_1_y_generic_sse2;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 3) && (dst_advance == 3))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_3_y_generic_sse2;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 2) && (dst_advance == 2))
    {
    tab->funcs_y.scale_uint8_x_2 =  scale_uint8_x_2_y_generic_sse2;
    tab->funcs_y.bits_uint8_noadvance = 14;
    }
  else if((src_advance == 4) && (dst_advance == 4))
    {
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_4_y_generic_sse2;
    tab->funcs_y.scale_uint8_x_4 =  scale_uint8_x_4_y_generic_sse2;
    tab->funcs_y.bits_uint8_noadvance  = 14;
    }
  tab->funcs_y.scale_uint16_x_1 =  scale_uint16_x_1_y_generic_sse2;
  tab->funcs_y.scale_uint16_x_2 =  scale_uint16_x_2_y_generic_sse2;
  tab->funcs_y.scale_uint16_x_3 =  scale_uint16_x_3_y_generic_sse2;
  tab->funcs_y.scale_uint16_x_4 =  scale_uint16_x_4_y_generic_sse2;
  tab->funcs_y.bits_uint16 = 14;
  
  }

void gavl_init_scale_funcs_bilinear_y_sse2(gavl_scale_funcs_t * tab,
                                         int src_advance, int dst_advance)
  {
  }
