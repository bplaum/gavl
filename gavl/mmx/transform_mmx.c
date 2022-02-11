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


#include <config.h>
#include <attributes.h>

#include <gavl/gavl.h>
#include <video.h>
#include <transform.h>

// #include "mmx.h"
#include "mmx_macros.h"

#if 0

static mmx_t dump_tmp;

#define DUMP_MM(reg, name)      \
  movq_r2m(reg, dump_tmp); \
  fprintf(stderr, "%s: %04x%04x%04x%04x\n", \
          name, \
          dump_tmp.uw[3],           \
          dump_tmp.uw[2], \
          dump_tmp.uw[1], \
          dump_tmp.uw[0])
#endif

#define LOAD_32(p, reg) \
  movd_m2r(*(p), reg);    \
  punpcklbw_r2r(mm0,reg); \
  psllq_i2r(7, reg);

#define LOAD_64(p, reg)   \
  movq_m2r(*(p), reg);    \
  psrlw_i2r(1, reg);

#ifdef MMXEXT
#define LOAD_FACTOR_X_1(f, reg) \
  movd_m2r(f, mm1); \
  pshufw_r2r(mm1, reg, 0)
#else
#define LOAD_FACTOR_X_1(f, reg) \
  movd_m2r(f, mm1); \
  movq_r2r(mm1, reg); \
  psllq_i2r(16, mm1); \
  por_r2r(mm1, reg);  \
  movq_r2r(reg, mm1); \
  psllq_i2r(32, mm1); \
  por_r2r(mm1, reg);
#endif

#define NUM_TAPS 2

/* transform_uint8_x_4_linear_mmx */

#define FUNC_NAME transform_uint8_x_4_linear_mmx
#define TYPE uint8_t
#define INIT pxor_r2r(mm0, mm0);
#define FINISH ctx->need_emms = 1;

#define TRANSFORM \
  LOAD_32(src_0, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[0][0], mm2); \
  pmulhw_r2r(mm3,mm2);                          \
  LOAD_32(src_0+4, mm3);                        \
  LOAD_FACTOR_X_1(pixel->factors_i[0][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_32(src_1, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][0], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_32(src_1+4, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  psrlw_i2r(6, mm2);        \
  packuswb_r2r(mm0, mm2); \
  movd_r2m(mm2, *dst);

#include "../c/transform_c.h"

/* transform_uint16_x_4_linear_mmx */

#define FUNC_NAME transform_uint16_x_4_linear_mmx
#define TYPE uint8_t
#define INIT pxor_r2r(mm0, mm0);
#define FINISH ctx->need_emms = 1;

#define TRANSFORM \
  LOAD_64(src_0, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[0][0], mm2); \
  pmulhw_r2r(mm3,mm2);                          \
  LOAD_64(src_0+8, mm3);                        \
  LOAD_FACTOR_X_1(pixel->factors_i[0][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_64(src_1, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][0], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_64(src_1+8, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  psllw_i2r(2, mm2);        \
  MOVQ_R2M(mm2, *dst);

#include "../c/transform_c.h"

#undef NUM_TAPS

#define NUM_TAPS 3

/* transform_uint8_x_4_quadratic_mmx */

#define FUNC_NAME transform_uint8_x_4_quadratic_mmx
#define TYPE uint8_t
#define INIT pxor_r2r(mm0, mm0);
#define FINISH ctx->need_emms = 1;

#define TRANSFORM \
  LOAD_32(src_0, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[0][0], mm2); \
  pmulhw_r2r(mm3,mm2);                          \
  LOAD_32(src_0+4, mm3);                        \
  LOAD_FACTOR_X_1(pixel->factors_i[0][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_32(src_0+8, mm3);                        \
  LOAD_FACTOR_X_1(pixel->factors_i[0][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_32(src_1, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][0], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_32(src_1+4, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_32(src_1+8, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_32(src_2, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][0], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_32(src_2+4, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_32(src_2+8, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  psrlw_i2r(6, mm2);        \
  packuswb_r2r(mm0, mm2); \
  movd_r2m(mm2, *dst);

#include "../c/transform_c.h"

/* transform_uint16_x_4_quadratic_mmx */

#define FUNC_NAME transform_uint16_x_4_quadratic_mmx
#define TYPE uint8_t
#define INIT pxor_r2r(mm0, mm0);
#define FINISH ctx->need_emms = 1;

#define TRANSFORM \
  LOAD_64(src_0, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[0][0], mm2); \
  pmulhw_r2r(mm3,mm2);                          \
  LOAD_64(src_0+8, mm3);                        \
  LOAD_FACTOR_X_1(pixel->factors_i[0][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_64(src_0+16, mm3);                        \
  LOAD_FACTOR_X_1(pixel->factors_i[0][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_64(src_1, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][0], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_64(src_1+8, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_64(src_1+16, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_64(src_2, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][0], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_64(src_2+8, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_64(src_2+16, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  psllw_i2r(2, mm2);        \
  MOVQ_R2M(mm2, *dst);

#include "../c/transform_c.h"


#undef NUM_TAPS
#define NUM_TAPS 4

/* transform_uint8_x_4_bicubic_mmx */

#define FUNC_NAME transform_uint8_x_4_bicubic_mmx
#define TYPE uint8_t
#define INIT pxor_r2r(mm0, mm0);
#define FINISH ctx->need_emms = 1;

#define TRANSFORM \
  LOAD_32(src_0, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[0][0], mm2); \
  pmulhw_r2r(mm3,mm2);                          \
  LOAD_32(src_0+4, mm3);                        \
  LOAD_FACTOR_X_1(pixel->factors_i[0][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_32(src_0+8, mm3);                        \
  LOAD_FACTOR_X_1(pixel->factors_i[0][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_32(src_0+12, mm3);                        \
  LOAD_FACTOR_X_1(pixel->factors_i[0][3], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_32(src_1, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][0], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_32(src_1+4, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_32(src_1+8, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_32(src_1+16, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][3], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_32(src_2, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][0], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_32(src_2+4, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_32(src_2+8, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_32(src_2+12, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][3], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_32(src_3, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[3][0], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_32(src_3+4, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[3][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_32(src_3+8, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[3][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_32(src_3+12, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[3][3], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  psrlw_i2r(6, mm2);        \
  packuswb_r2r(mm0, mm2); \
  movd_r2m(mm2, *dst);

#include "../c/transform_c.h"


/* transform_uint16_x_4_bicubic_mmx */

#define FUNC_NAME transform_uint16_x_4_bicubic_mmx
#define TYPE uint8_t
#define INIT pxor_r2r(mm0, mm0);
#define FINISH ctx->need_emms = 1;

#define TRANSFORM \
  LOAD_64(src_0, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[0][0], mm2); \
  pmulhw_r2r(mm3,mm2);                          \
  LOAD_64(src_0+8, mm3);                        \
  LOAD_FACTOR_X_1(pixel->factors_i[0][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_64(src_0+16, mm3);                        \
  LOAD_FACTOR_X_1(pixel->factors_i[0][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_64(src_0+24, mm3);                        \
  LOAD_FACTOR_X_1(pixel->factors_i[0][3], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_64(src_1, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][0], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_64(src_1+8, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_64(src_1+16, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_64(src_1+24, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[1][3], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_64(src_2, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][0], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_64(src_2+8, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_64(src_2+16, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_64(src_2+24, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[2][3], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_64(src_3, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[3][0], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);  \
  LOAD_64(src_3+8, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[3][1], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_64(src_3+16, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[3][2], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  LOAD_64(src_3+24, mm3);                         \
  LOAD_FACTOR_X_1(pixel->factors_i[3][3], mm4); \
  pmulhw_r2r(mm3,mm4);                          \
  paddusw_r2r(mm4,mm2);                    \
  psllw_i2r(2, mm2);        \
  MOVQ_R2M(mm2, *dst);

#include "../c/transform_c.h"


#ifdef MMXEXT
void gavl_init_transform_funcs_bilinear_mmxext(gavl_transform_funcs_t * tab,
                                            int advance)
#else
void gavl_init_transform_funcs_bilinear_mmx(gavl_transform_funcs_t * tab,
                                            int advance)
#endif
  {
  if(advance == 4)
    {
    tab->transform_uint8_x_3 =  transform_uint8_x_4_linear_mmx;
    tab->transform_uint8_x_4 =  transform_uint8_x_4_linear_mmx;
    tab->bits_uint8_noadvance  = 15;
    // fprintf(stderr, "Using MMX transform\n");
    }
  if(advance == 8)
    {
    tab->transform_uint16_x_4 =  transform_uint16_x_4_linear_mmx;
    tab->bits_uint16_x_4  = 15;
    }
  }

#ifdef MMXEXT
void gavl_init_transform_funcs_quadratic_mmxext(gavl_transform_funcs_t * tab,
                                                int advance)
#else
void gavl_init_transform_funcs_quadratic_mmx(gavl_transform_funcs_t * tab,
                                             int advance)
#endif
  {
  if(advance == 4)
    {
    tab->transform_uint8_x_3 =  transform_uint8_x_4_quadratic_mmx;
    tab->transform_uint8_x_4 =  transform_uint8_x_4_quadratic_mmx;
    tab->bits_uint8_noadvance  = 15;
    }
  if(advance == 8)
    {
    tab->transform_uint16_x_4 =  transform_uint16_x_4_quadratic_mmx;
    tab->bits_uint16_x_4  = 15;
    // fprintf(stderr, "Using MMX transform\n");
    }

  }

#ifdef MMXEXT
void gavl_init_transform_funcs_bicubic_mmxext(gavl_transform_funcs_t * tab,
                                                int advance)
#else
void gavl_init_transform_funcs_bicubic_mmx(gavl_transform_funcs_t * tab,
                                             int advance)
#endif
  {
  if(advance == 4)
    {
    tab->transform_uint8_x_3 =  transform_uint8_x_4_bicubic_mmx;
    tab->transform_uint8_x_4 =  transform_uint8_x_4_bicubic_mmx;
    tab->bits_uint8_noadvance  = 15;
    // fprintf(stderr, "Using MMX transform\n");
    }
  if(advance == 8)
    {
    tab->transform_uint16_x_4 =  transform_uint16_x_4_bicubic_mmx;
    tab->bits_uint16_x_4  = 15;
    // fprintf(stderr, "Using MMX transform\n");
    }

  }
