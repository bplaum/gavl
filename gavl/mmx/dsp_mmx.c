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

#include <stdio.h>
#include <gavl/gavl.h>
#include <gavl/gavldsp.h>
#include <dsp.h>
#include <attributes.h>
#include "mmx.h"

#ifdef MMXEXT
#define MOVQ_R2M(reg,mem) movntq_r2m(reg, mem)
#else
#define MOVQ_R2M(reg,mem) movq_r2m(reg, mem)
#endif

#if 0
static mmx_t mm_tmp;
#define DUMP_MM(name, reg) MOVQ_R2M(reg, mm_tmp);\
  fprintf(stderr, "%s: %016llx\n", name, mm_tmp.q);
#endif



static void interpolate_8_mmx(const uint8_t * src_1, const uint8_t * src_2, 
                              uint8_t * dst, int num, float fac)
  {
  int i, imax;
  int32_t tmp;
  int32_t fac_i;
  int32_t anti_fac;
  
  fac_i = (float)(fac * 0x4000 + 0.5);
  anti_fac = 0x4000 - fac_i;
  
  //  fprintf(stderr, "interpolate_8_mmx %d %d\n", fac, anti_fac);

  imax = num / 8;
  //  imax = 0;
  
  /* Load factors */

  /*
   *  mm0: Input1
   *  mm1: Input2
   *  mm2: Factor1
   *  mm3: Factor1
   *  mm4: Output1
   *  mm5: Output2
   *  mm6: Scratch
   *  mm7: 0
   */

  pxor_r2r(mm7, mm7);
  
  /* Load factor1 */
  movd_m2r(fac_i, mm2);
  movq_r2r(mm2, mm6);
  psllq_i2r(16, mm6);
  por_r2r(mm6, mm2);
  movq_r2r(mm2, mm6);
  psllq_i2r(32, mm6);
  por_r2r(mm6, mm2);
  /* Load factor2 */
  movd_m2r(anti_fac, mm3);
  movq_r2r(mm3, mm6);
  psllq_i2r(16, mm6);
  por_r2r(mm6, mm3);
  movq_r2r(mm3, mm6);
  psllq_i2r(32, mm6);
  por_r2r(mm6, mm3);
  
  for(i = 0; i < imax; i++)
    {
    /* Load input 1 */
    movq_m2r(*src_1,mm0);
    movq_r2r(mm0,mm1);
    punpcklbw_r2r(mm7, mm0);
    punpckhbw_r2r(mm7, mm1);
    psllw_i2r(7, mm0);
    psllw_i2r(7, mm1);

    /* Accumulate mm0 */
    pmulhw_r2r(mm2, mm0);
    movq_r2r(mm0, mm4);
    /* Accumulate mm1 */ 
    pmulhw_r2r(mm2, mm1);
    movq_r2r(mm1, mm5);

    /* Load input 2 */
    movq_m2r(*(src_2),mm0);
    movq_r2r(mm0,mm1);
    punpcklbw_r2r(mm7, mm0);
    punpckhbw_r2r(mm7, mm1);
    psllw_i2r(7, mm0);
    psllw_i2r(7, mm1);

    /* Accumulate mm0 */
    pmulhw_r2r(mm3, mm0);
    paddsw_r2r(mm0, mm4);
    /* Accumulate mm1 */ 
    pmulhw_r2r(mm3, mm1);
    paddsw_r2r(mm1, mm5);
    
    psraw_i2r(5, mm4);
    psraw_i2r(5, mm5);
    
    packuswb_r2r(mm5, mm4);
    
    MOVQ_R2M(mm4, *dst);
    
    dst += 8;
    src_1 += 8;
    src_2 += 8;
    }

  emms();
  
  imax = num % 8;
  //  imax = num;
  
  if(!imax)
    return;
  
  for(i = 0; i < imax; i++)
    {
    tmp = (*src_1 * fac_i + *src_2 * anti_fac) >> 15;
    *dst = (uint8_t)((tmp & ~0xFF)?((-tmp) >> 31) : tmp);
    /* Accum */
    dst++;
    src_1++;
    src_2++;
    }

  
  }

void gavl_dsp_init_mmx(gavl_dsp_funcs_t * funcs, 
                       int quality)
  {
  if(quality < 3)
    funcs->interpolate_8 = interpolate_8_mmx;
  }
