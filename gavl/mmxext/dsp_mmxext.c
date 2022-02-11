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

#include <stdlib.h>

#include <config.h>
#include <gavl/gavl.h>
#include <gavl/gavldsp.h>
#include <dsp.h>
#include <attributes.h>

#include "../mmx/mmx.h"

static int sad_8_mmxext(const uint8_t * src_1, const uint8_t * src_2, 
                        int stride_1, int stride_2, 
                        int w, int h)
  {
  uint32_t ret = 0, i, j;
  const uint8_t * s1, *s2;
  
  pxor_r2r(mm0, mm0);

  for(i = 0; i < h; i++)
    {
    j = w;
    s1 = src_1;
    s2 = src_2;

    while(j > 7)
      {
      movq_m2r((*s1), mm1);
      movq_m2r((*s2), mm2);

      psadbw_r2r(mm1, mm2);
      paddd_r2r(mm2, mm0);
      
      s1 += 8;
      s2 += 8;
      j -= 8;
      }

    movd_r2m(mm0, ret);

    while(j--)
      {
      ret += abs((*s1)-(*s2));
      s1++;
      s2++;
      }
    src_1 += stride_1;
    src_2 += stride_2;
    }
  return ret;
  }

void gavl_dsp_init_mmxext(gavl_dsp_funcs_t * funcs, 
                          int quality)
  {
  funcs->sad_8 = sad_8_mmxext;
  }
