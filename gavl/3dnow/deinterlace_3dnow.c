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
#include "../mmx/mmx.h"

#define MOVQ_R2M(reg,mem) movntq_r2m(reg, mem)

static void blend_func_8_3dnow(uint8_t * t,
                             uint8_t * m,
                             uint8_t * b,
                             uint8_t * dst,
                             int num)
  {
  int i;
  int imax;

  imax = num / 8;

  //  pxor_r2r(mm7, mm7);

  for(i = 0; i < imax; i++)
    {
    movq_m2r(*t, mm0);
    //    movq_m2r(*m, mm1);
    //    movq_m2r(*b, mm2);

    pavgbusb_m2r(*b, mm0);
    pavgbusb_m2r(*m, mm0);
    
    MOVQ_R2M(mm0, *dst);
    
    m += 8;
    b += 8;
    t += 8;
    dst += 8;
    }
  
  femms();
  
  imax = num % 8;
  for(i = 0; i < imax; i++)
    {
    *(dst++) = (*(t++) + (*(m++) << 1) + *(b++)) >> 2;
    }
  }


void
gavl_find_deinterlacer_blend_funcs_3dnow(gavl_video_deinterlace_blend_func_table_t * tab,
                                     const gavl_video_options_t * opt,
                                     const gavl_video_format_t * format)
  {
  //  tab->func_packed_15 = blend_func_packed_15_c;
  //  tab->func_packed_16 = blend_func_packed_16_c;
  tab->func_8 = blend_func_8_3dnow;
  //  tab->func_16 = blend_func_16_c;
  //  tab->func_float = blend_func_float_c;
  }
