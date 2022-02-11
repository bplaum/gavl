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

#include <gavl/gavl.h>
#include <video.h>

#include <deinterlace.h>
#include "mmx.h"

#ifdef MMXEXT
#define MOVQ_R2M(reg,mem) movntq_r2m(reg, mem)
#else
#define MOVQ_R2M(reg,mem) movq_r2m(reg, mem)
#endif



static void blend_func_8_mmx(const uint8_t * t,
                             const uint8_t * m,
                             const uint8_t * b,
                             uint8_t * dst,
                             int num)
  {
  int i;
  int imax;

  imax = num / 8;

  pxor_r2r(mm7, mm7);

  for(i = 0; i < imax; i++)
    {
    movq_m2r(*t, mm0);
    movq_m2r(*m, mm1);
    movq_m2r(*b, mm2);

    movq_r2r(mm0, mm3);
    punpcklbw_r2r(mm7, mm0);
    punpckhbw_r2r(mm7, mm3);

    movq_r2r(mm1, mm4);
    punpcklbw_r2r(mm7, mm1);
    punpckhbw_r2r(mm7, mm4);

    movq_r2r(mm2, mm5);
    punpcklbw_r2r(mm7, mm2);
    punpckhbw_r2r(mm7, mm5);

    /*
     *  mm0: 00 t3 00 t2 00 t1 00 t0
     *  mm1: 00 m3 00 m2 00 m1 00 m0
     *  mm2: 00 b3 00 b2 00 b1 00 b0
     *  mm3: 00 t7 00 t6 00 t5 00 t4
     *  mm4: 00 m7 00 m6 00 m5 00 m4
     *  mm5: 00 b7 00 b6 00 b5 00 b4
     */

    psllw_i2r(1, mm1);
    psllw_i2r(1, mm4);

    paddw_r2r(mm0, mm1);
    paddw_r2r(mm3, mm4);

    paddw_r2r(mm1, mm2);
    paddw_r2r(mm4, mm5);

    psrlw_i2r(2, mm2);
    psrlw_i2r(2, mm5);
    
    packuswb_r2r(mm5, mm2);

    MOVQ_R2M(mm2, *dst);
    
    m += 8;
    b += 8;
    t += 8;
    dst += 8;
    }

  emms();
  
  imax = num % 8;
  for(i = 0; i < imax; i++)
    {
    *(dst++) = (*(t++) + (*(m++) << 1) + *(b++)) >> 2;
    }
  }


void
gavl_find_deinterlacer_blend_funcs_mmx(gavl_video_deinterlace_blend_func_table_t * tab,
                                     const gavl_video_options_t * opt,
                                     const gavl_video_format_t * format)
  {
  //  tab->func_packed_15 = blend_func_packed_15_c;
  //  tab->func_packed_16 = blend_func_packed_16_c;
  tab->func_8 = blend_func_8_mmx;
  //  tab->func_16 = blend_func_16_c;
  //  tab->func_float = blend_func_float_c;
  }
