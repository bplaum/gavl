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

/* SSE Optimized scaling (linear, y) */



static void (FUNC_NAME)(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i, imax;
  uint8_t * src;
  uint8_t * src_start;
  uint8_t * dst;
    
  /* Load factor */
  movss_m2r(ctx->table_v.pixels[scanline].factor_f[0], xmm0);
  shufps_r2ri(xmm0, xmm0, 0x00);
  
  dst = dest_start;

  src_start =
    ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride;
  
  /* While source is not aligned... */
  imax = (((long)(src_start)) % 16)/4;
  
  for(i = 0; i < imax; i++)
    {
    src = src_start;

    movss_m2r(*src, xmm1);
    movss_m2r(*(src+ctx->src_stride), xmm2);
    subss_r2r(xmm2, xmm1);
    mulss_r2r(xmm0, xmm1);
    addss_r2r(xmm2, xmm1);
    movss_r2m(xmm1, *dst);
    dst+=4;
    src_start+=4;
    }

  /* SSE routines scale 8 numbers (= 32 bytes) at once */
  imax = (ctx->dst_size * 4 * WIDTH_MUL - (dst - dest_start)) / /* Bytes left */
          (32); /* Bytes processed at once */
          
  for(i = 0; i < imax; i++)
    {
    src = src_start;

    movaps_m2r(*src, xmm1);
    movaps_m2r(*(src+ctx->src_stride), xmm2);

    movaps_m2r(*(src+16), xmm3);
    movaps_m2r(*(src+ctx->src_stride+16), xmm4);
    
    subps_r2r(xmm2, xmm1);
    mulps_r2r(xmm0, xmm1);
    addps_r2r(xmm2, xmm1);
    movups_r2m(xmm1, *dst);

    subps_r2r(xmm4, xmm3);
    mulps_r2r(xmm0, xmm3);
    addps_r2r(xmm4, xmm3);
    movups_r2m(xmm3, *(dst+16));
    
    dst += 32;
    src_start += 32;
    }

  imax = (ctx->dst_size * 4 * WIDTH_MUL - (dst - dest_start)) / 4;
          
  //  imax = (ctx->dst_size * WIDTH_MUL);
  
  if(!imax)
    return;
  
  for(i = 0; i < imax; i++)
    {
    src = src_start;

    movss_m2r(*src, xmm1);
    movss_m2r(*(src+ctx->src_stride), xmm2);
    subss_r2r(xmm2, xmm1);
    mulss_r2r(xmm0, xmm1);
    addss_r2r(xmm2, xmm1);
    movss_r2m(xmm1, *dst);
    
    dst+=4;
    src_start+=4;
    }
  }

#undef FUNC_NAME
#undef NUM_TAPS
#undef WIDTH_MUL
#undef ACCUM
#undef ACCUM_C
#undef OUTPUT
#undef OUTPUT_C

#ifdef INIT_GLOBAL
#undef INIT_GLOBAL
#endif

#ifdef INIT_C
#undef INIT_C
#endif

#undef INIT

