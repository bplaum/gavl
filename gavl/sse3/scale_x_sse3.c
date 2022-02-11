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

/* SSE3 Optimized scaling (x) */

#include <config.h>
#include <attributes.h>

#include <stdio.h>
#include <gavl/gavl.h>
#include <video.h>
#include <scale.h>

#include "../sse/sse.h"

static void
scale_float_x_1_x_bicubic_sse3(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i, imax;
  uint8_t * src, * dst, *src_start;

  movss_m2r(ctx->min_values_f[ctx->plane], xmm6);
  movss_m2r(ctx->max_values_f[ctx->plane], xmm7);
  imax = ctx->dst_size;
  
  src_start = ctx->src + scanline * ctx->src_stride;
  dst = dest_start;

  for(i = 0; i < imax; i++)
    {
    src = src_start + 4*ctx->table_h.pixels[i].index;

    /* Load factors */
    movups_m2r(ctx->table_h.pixels[i].factor_f[0], xmm0);

    /* Load src1 */
    movups_m2r(*src, xmm5);

    mulps_r2r(xmm0, xmm5);
 
    haddps_r2r(xmm5, xmm5);
    haddps_r2r(xmm5, xmm5);

    /* Clip */
    minss_r2r(xmm7, xmm5);
    maxss_r2r(xmm6, xmm5);
    
    /* Store */
    movss_r2m(xmm5, *dst);
    dst += 4;
    }

  }

#if 0
static void
scale_float_x_1_x_bicubic_noclip_sse3(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i, imax;
  uint8_t * src, * dst, *src_start;
  
  imax = ctx->dst_size;
  
  src_start = ctx->src + scanline * ctx->src_stride;
  dst = dest_start;

  for(i = 0; i < imax; i++)
    {
    src = src_start + 4*ctx->table_h.pixels[i].index;

    /* Load factors */
    movups_m2r(ctx->table_h.pixels[i].factor_f[0], xmm0);

    /* Load src1 */
    movups_m2r(*src, xmm5);

    mulps_r2r(xmm0, xmm5);
 
    haddps_r2r(xmm5, xmm5);
    haddps_r2r(xmm5, xmm5);

    /* Store */
    movss_r2m(xmm5, *dst);
    dst += 4;
    }
  
  }
#endif

static void
scale_float_x_1_x_generic_sse3(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i, imax, j, jmax;
  float * factors;
  uint8_t * src, * dst, *src_start;

  movss_m2r(ctx->min_values_f[ctx->plane], xmm6);
  movss_m2r(ctx->max_values_f[ctx->plane], xmm7);
  imax = ctx->dst_size;
  
  src_start = ctx->src + scanline * ctx->src_stride;
  dst = dest_start;

  for(i = 0; i < imax; i++)
    {
    src = src_start + 4*ctx->table_h.pixels[i].index;

    jmax = ctx->table_h.factors_per_pixel / 4;
    factors = ctx->table_h.pixels[i].factor_f;

    xorps_r2r(xmm5, xmm5);
    
    for(j = 0; j < jmax; j++)
      {
      /* Load factors */
      movups_m2r(*factors, xmm0);
      
      /* Load src1 */
      movups_m2r(*src, xmm4);
      
      mulps_r2r(xmm0, xmm4);
      factors += 4;
      src += 16;
      addps_r2r(xmm4, xmm5);
      }

    haddps_r2r(xmm5, xmm5);
    haddps_r2r(xmm5, xmm5);
    
    jmax = ctx->table_h.factors_per_pixel % 4;

    for(j = 0; j < jmax; j++)
      {
      /* Load factors */
      movss_m2r(*factors, xmm0);
      
      /* Load src1 */
      movss_m2r(*src, xmm4);
     
      mulss_r2r(xmm0, xmm4);
      addss_r2r(xmm4, xmm5);
      
      src += 4;
      factors++;
      }
    
    /* Clip */
    minss_r2r(xmm7, xmm5);
    maxss_r2r(xmm6, xmm5);
    
    /* Store */
    movss_r2m(xmm5, *dst);
    dst += 4;
    }
  
  }


void gavl_init_scale_funcs_bicubic_x_sse3(gavl_scale_funcs_t * tab)
  {
  tab->funcs_x.scale_float_x_1 =  scale_float_x_1_x_bicubic_sse3;

  }

void gavl_init_scale_funcs_bicubic_x_noclip_sse3(gavl_scale_funcs_t * tab)
  {
  //  tab->funcs_x.scale_float_x_1 =  scale_float_x_1_x_bicubic_noclip_sse3;

  }

void gavl_init_scale_funcs_generic_x_sse3(gavl_scale_funcs_t * tab)
  {
  tab->funcs_x.scale_float_x_1 =  scale_float_x_1_x_generic_sse3;
  }
