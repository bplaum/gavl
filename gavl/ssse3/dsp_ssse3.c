/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2024 Members of the Gmerlin project
 * http://github.com/bplaum
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
#include <gavl/gavl.h>
#include <gavl/gavldsp.h>
#include <dsp.h>
#include <attributes.h>

#include "../sse/sse.h"

static void shuffle_8_4_ssse3(void * ptr, int num, uint8_t * mask)
  {
  int i, imax;
  sse_t sse_mask;
  
  union
    {
    uint8_t u8[4];
    uint32_t u32;
    } buf;

  union
    {
    uint8_t * u8;
    uint32_t * u32;
    } buf_ptr;

  mask[0] &= (0x80 | 0x03);
  mask[1] &= (0x80 | 0x03);
  mask[2] &= (0x80 | 0x03);
  mask[3] &= (0x80 | 0x03);
  
  buf_ptr.u8 = ptr;

  imax = (16 - (((long)(buf_ptr.u8)) % 16))/4;

  if(imax > 4)
    imax -= 4;
  
  for(i = 0; i < imax; i++)
    {
    buf.u8[0] = mask[0] & 0x80 ? 0 : buf_ptr.u8[mask[0]];
    buf.u8[1] = mask[1] & 0x80 ? 0 : buf_ptr.u8[mask[1]];
    buf.u8[2] = mask[2] & 0x80 ? 0 : buf_ptr.u8[mask[2]];
    buf.u8[3] = mask[3] & 0x80 ? 0 : buf_ptr.u8[mask[3]];
    
    *(buf_ptr.u32) = buf.u32;
    buf_ptr.u32++;
    }
  
  num -= imax;
  
  /* Fast version */
  imax = num / 4;
  
  sse_mask.b[0]  = mask[0];
  sse_mask.b[1]  = mask[1];
  sse_mask.b[2]  = mask[2];
  sse_mask.b[3]  = mask[3];

  sse_mask.d[1] = sse_mask.d[0] | (0x01010101 << 2);
  sse_mask.d[2] = sse_mask.d[0] | (0x02020202 << 2);
  sse_mask.d[3] = sse_mask.d[0] | (0x03030303 << 2);

  movups_m2r(sse_mask, xmm0);
  
  for(i = 0; i < imax; i++)
    {
    movaps_m2r(*buf_ptr.u8,xmm1);
    pshufb_r2r(xmm0, xmm1);
    movaps_r2m(xmm1, *buf_ptr.u8);
    buf_ptr.u8+=16;
    }
  
  num -= imax * 4;

  /* Remainder */
  imax = num;

  for(i = 0; i < imax; i++)
    {
    buf.u8[0] = mask[0] & 0x80 ? 0 : buf_ptr.u8[mask[0]];
    buf.u8[1] = mask[1] & 0x80 ? 0 : buf_ptr.u8[mask[1]];
    buf.u8[2] = mask[2] & 0x80 ? 0 : buf_ptr.u8[mask[2]];
    buf.u8[3] = mask[3] & 0x80 ? 0 : buf_ptr.u8[mask[3]];
    
    *(buf_ptr.u32) = buf.u32;
    buf_ptr.u32++;
    }
  }


void gavl_dsp_init_ssse3(gavl_dsp_funcs_t * funcs, 
                       int quality)
  {
  funcs->shuffle_8_4 = shuffle_8_4_ssse3;
  }
