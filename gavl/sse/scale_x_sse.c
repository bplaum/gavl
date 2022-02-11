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

/* SSE Optimized scaling (x) */

#include <config.h>
#include <attributes.h>

#include <stdio.h>
#include <gavl/gavl.h>
#include <video.h>
#include <scale.h>

#include "../sse/sse.h"

static void
scale_float_x_4_x_bilinear_sse(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i, imax;
  uint8_t * src, * dst, *src_start;
  
  imax = ctx->dst_size;
  
  src_start = ctx->src + scanline * ctx->src_stride;
  dst = dest_start;

  for(i = 0; i < imax; i++)
    {
    src = src_start + 16*ctx->table_h.pixels[i].index;

    /* Load factor */
    movss_m2r(ctx->table_h.pixels[i].factor_f[0], xmm0);
    shufps_r2ri(xmm0, xmm0, 0x00);

    /* Load src1 */
    movaps_m2r(*src, xmm5);

    /* Load src2 */
    movaps_m2r(*(src+16), xmm4);

    /* xmm4 = src1 - src2 */
    subps_r2r(xmm4, xmm5);

    /* xmm4 = (src1 - src2)*f */
    mulps_r2r(xmm0, xmm5);

    /* xmm4 = (src1 - src2)*f + src2 */
    
    addps_r2r(xmm4, xmm5);

    /* Store */
    movaps_r2m(xmm5, *dst);
    dst += 16;
    }
  }

static void
scale_float_x_4_x_quadratic_sse(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i, imax;
  uint8_t * src, * dst, *src_start;
  
  imax = ctx->dst_size;
  
  src_start = ctx->src + scanline * ctx->src_stride;
  dst = dest_start;
  
  for(i = 0; i < imax; i++)
    {
    xorps_r2r(xmm5, xmm5);
    src = src_start + 16*ctx->table_h.pixels[i].index;

    /* Load factor */
    movss_m2r(ctx->table_h.pixels[i].factor_f[0], xmm0);
    shufps_r2ri(xmm0, xmm0, 0x00);

    /* Load src1 */
    movaps_m2r(*src, xmm4);

    /* Multiply */
    mulps_r2r(xmm0, xmm4);

    /* Add */
    addps_r2r(xmm4, xmm5);

    /* Load factor */
    movss_m2r(ctx->table_h.pixels[i].factor_f[1], xmm0);
    shufps_r2ri(xmm0, xmm0, 0x00);

    /* Load src1 */
    movaps_m2r(*(src+16), xmm4);

    /* Multiply */
    mulps_r2r(xmm0, xmm4);

    /* Add */
    addps_r2r(xmm4, xmm5);

    /* Load factor */
    movss_m2r(ctx->table_h.pixels[i].factor_f[2], xmm0);
    shufps_r2ri(xmm0, xmm0, 0x00);

    /* Load src1 */
    movaps_m2r(*(src+32), xmm4);

    /* Multiply */
    mulps_r2r(xmm0, xmm4);

    /* Add */
    addps_r2r(xmm4, xmm5);

    /* Store */
    movaps_r2m(xmm5, *dst);
    dst += 16;
    
    }
  
  }

static void
scale_float_x_4_x_bicubic_sse(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i, imax;
  uint8_t * src, * dst, *src_start;
  
  imax = ctx->dst_size;
  
  src_start = ctx->src + scanline * ctx->src_stride;
  dst = dest_start;

  movups_m2r(ctx->min_values_f[0], xmm6);
  movups_m2r(ctx->max_values_f[0], xmm7);

  for(i = 0; i < imax; i++)
    {
    xorps_r2r(xmm5, xmm5);
    src = src_start + 16*ctx->table_h.pixels[i].index;

    /* Load factor */
    movss_m2r(ctx->table_h.pixels[i].factor_f[0], xmm0);
    shufps_r2ri(xmm0, xmm0, 0x00);

    /* Load src1 */
    movaps_m2r(*src, xmm4);

    /* Multiply */
    mulps_r2r(xmm0, xmm4);

    /* Add */
    addps_r2r(xmm4, xmm5);

    /* Load factor */
    movss_m2r(ctx->table_h.pixels[i].factor_f[1], xmm0);
    shufps_r2ri(xmm0, xmm0, 0x00);

    /* Load src1 */
    movaps_m2r(*(src+16), xmm4);

    /* Multiply */
    mulps_r2r(xmm0, xmm4);

    /* Add */
    addps_r2r(xmm4, xmm5);

    /* Load factor */
    movss_m2r(ctx->table_h.pixels[i].factor_f[2], xmm0);
    shufps_r2ri(xmm0, xmm0, 0x00);

    /* Load src1 */
    movaps_m2r(*(src+32), xmm4);

    /* Multiply */
    mulps_r2r(xmm0, xmm4);

    /* Add */
    addps_r2r(xmm4, xmm5);

    /* Load factor */
    movss_m2r(ctx->table_h.pixels[i].factor_f[3], xmm0);
    shufps_r2ri(xmm0, xmm0, 0x00);

    /* Load src1 */
    movaps_m2r(*(src+48), xmm4);

    /* Multiply */
    mulps_r2r(xmm0, xmm4);

    /* Add */
    addps_r2r(xmm4, xmm5);

    /* Clip */
    minps_r2r(xmm7, xmm5);
    maxps_r2r(xmm6, xmm5);
    
    /* Store */
    movaps_r2m(xmm5, *dst);
    dst += 16;
    
    }

  }

static void
scale_float_x_4_x_bicubic_noclip_sse(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i, imax;
  uint8_t * src, * dst, *src_start;
  
  imax = ctx->dst_size;
  
  src_start = ctx->src + scanline * ctx->src_stride;
  dst = dest_start;


  for(i = 0; i < imax; i++)
    {
    xorps_r2r(xmm5, xmm5);
    src = src_start + 16*ctx->table_h.pixels[i].index;

    /* Load factor */
    movss_m2r(ctx->table_h.pixels[i].factor_f[0], xmm0);
    shufps_r2ri(xmm0, xmm0, 0x00);

    /* Load src1 */
    movaps_m2r(*src, xmm4);

    /* Multiply */
    mulps_r2r(xmm0, xmm4);

    /* Add */
    addps_r2r(xmm4, xmm5);

    /* Load factor */
    movss_m2r(ctx->table_h.pixels[i].factor_f[1], xmm0);
    shufps_r2ri(xmm0, xmm0, 0x00);

    /* Load src1 */
    movaps_m2r(*(src+16), xmm4);

    /* Multiply */
    mulps_r2r(xmm0, xmm4);

    /* Add */
    addps_r2r(xmm4, xmm5);

    /* Load factor */
    movss_m2r(ctx->table_h.pixels[i].factor_f[2], xmm0);
    shufps_r2ri(xmm0, xmm0, 0x00);

    /* Load src1 */
    movaps_m2r(*(src+32), xmm4);

    /* Multiply */
    mulps_r2r(xmm0, xmm4);

    /* Add */
    addps_r2r(xmm4, xmm5);

    /* Load factor */
    movss_m2r(ctx->table_h.pixels[i].factor_f[3], xmm0);
    shufps_r2ri(xmm0, xmm0, 0x00);

    /* Load src1 */
    movaps_m2r(*(src+48), xmm4);

    /* Multiply */
    mulps_r2r(xmm0, xmm4);

    /* Add */
    addps_r2r(xmm4, xmm5);

    
    /* Store */
    movaps_r2m(xmm5, *dst);
    dst += 16;
    
    }

  }

static void
scale_float_x_4_x_generic_sse(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i, imax, j;
  uint8_t * src, * dst, *src_start;
  
  imax = ctx->dst_size;
  
  src_start = ctx->src + scanline * ctx->src_stride;
  dst = dest_start;

  movups_m2r(ctx->min_values_f[0], xmm6);
  movups_m2r(ctx->max_values_f[0], xmm7);

  for(i = 0; i < imax; i++)
    {
    xorps_r2r(xmm5, xmm5);
    src = src_start + 16*ctx->table_h.pixels[i].index;

    for(j = 0; j < ctx->table_h.factors_per_pixel; j++)
      {
      /* Load factor */
      movss_m2r(ctx->table_h.pixels[i].factor_f[j], xmm0);
      shufps_r2ri(xmm0, xmm0, 0x00);
      
      /* Load src1 */
      movaps_m2r(*src, xmm4);
      
      /* Multiply */
      mulps_r2r(xmm0, xmm4);
      
      /* Add */
      addps_r2r(xmm4, xmm5);

      src += 16;
      }
    
    /* Clip */
    minps_r2r(xmm7, xmm5);
    maxps_r2r(xmm6, xmm5);
    
    /* Store */
    movaps_r2m(xmm5, *dst);
    dst += 16;
    
    }

  }

#if 0
static void
scale_float_x_1_x_bicubic_sse(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {

  }

static void
scale_float_x_1_x_bicubic_noclip_sse(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {

  }
#endif

void gavl_init_scale_funcs_bilinear_x_sse(gavl_scale_funcs_t * tab)
  {
  tab->funcs_x.scale_float_x_4 =  scale_float_x_4_x_bilinear_sse;
  }

void gavl_init_scale_funcs_quadratic_x_sse(gavl_scale_funcs_t * tab)
  {
  tab->funcs_x.scale_float_x_4 =  scale_float_x_4_x_quadratic_sse;
  }
  
void gavl_init_scale_funcs_bicubic_x_sse(gavl_scale_funcs_t * tab)
  {
  tab->funcs_x.scale_float_x_4 =  scale_float_x_4_x_bicubic_sse;

  }

void gavl_init_scale_funcs_bicubic_x_noclip_sse(gavl_scale_funcs_t * tab)
  {
  tab->funcs_x.scale_float_x_4 =  scale_float_x_4_x_bicubic_noclip_sse;

  }

void gavl_init_scale_funcs_generic_x_sse(gavl_scale_funcs_t * tab)
  {
  tab->funcs_x.scale_float_x_4 =  scale_float_x_4_x_generic_sse;
  
  }
