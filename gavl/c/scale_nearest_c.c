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

#include <string.h> /* memcpy */
#include <stdio.h>
#include <gavl/gavl.h>
#include <video.h>
#include <scale.h>
#include <accel.h> /* gavl_memcpy */


#define SCALE_FUNC_HEAD \
  for(i = 0; i < ctx->dst_rect.w; i++)       \
    {

#define SCALE_FUNC_TAIL \
    }

/* Nearest neighbor x-y direction */

static void scale_rgb_16_xy_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint16_t * src, *dst;
  src = (uint16_t*)(ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride);
  dst = (uint16_t*)(dest_start);
  SCALE_FUNC_HEAD
    *dst = src[ctx->table_h.pixels[i].index];
  dst++;
  SCALE_FUNC_TAIL
  }

static void scale_uint8_x_1_xy_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint8_t * src;
  src = ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride;

  SCALE_FUNC_HEAD
    *(dest_start) = *(src + ctx->table_h.pixels[i].index * ctx->offset->src_advance);
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }

static void scale_uint8_x_3_xy_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint8_t * src, *src1;
  src = ctx->src + (ctx->table_v.pixels[scanline].index * ctx->src_stride);
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index*ctx->offset->src_advance;
    dest_start[0] = src1[0];
    dest_start[1] = src1[1];
    dest_start[2] = src1[2];
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }

static void scale_uint8_x_4_xy_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint32_t * src, *dst;
  src = (uint32_t*)(ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride);
  dst = (uint32_t*)(dest_start);
  
  SCALE_FUNC_HEAD
    *dst = src[ctx->table_h.pixels[i].index];
    dst++;
  SCALE_FUNC_TAIL
  }

static void scale_uint16_x_1_xy_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint16_t * src, *src1, *dst;
  src = (uint16_t *)(ctx->src + (ctx->table_v.pixels[scanline].index * ctx->src_stride));
  dst = (uint16_t *)(dest_start);
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index;
    *dst = *src1;
    dst++;
  SCALE_FUNC_TAIL
  }

static void scale_uint16_x_3_xy_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint16_t * src, *src1;
  src = (uint16_t *)(ctx->src + (ctx->table_v.pixels[scanline].index * ctx->src_stride));
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index*3;
    memcpy(dest_start, src1, 6);
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }

static void scale_uint16_x_4_xy_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint16_t * src, *src1;
  src = (uint16_t*)(ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride);
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index*4;
    memcpy(dest_start, src1, 8);
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }

static void scale_float_x_1_xy_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  float * src, *src1;
  src = (float*)(ctx->src + (ctx->table_v.pixels[scanline].index * ctx->src_stride));
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index;
    memcpy(dest_start, src1, sizeof(float));
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }

static void scale_float_x_2_xy_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  float * src, *src1;
  src = (float*)(ctx->src + (ctx->table_v.pixels[scanline].index * ctx->src_stride));
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index*2;
    memcpy(dest_start, src1, 2 * sizeof(float));
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }


static void scale_float_x_3_xy_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  float * src, *src1;
  src = (float*)(ctx->src + (ctx->table_v.pixels[scanline].index * ctx->src_stride));
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index*3;
    memcpy(dest_start, src1, 3 * sizeof(float));
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }

static void scale_float_x_4_xy_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  float * src, *src1;
  src = (float*)(ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride);
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index*4;
    memcpy(dest_start, src1, 4 * sizeof(float));
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }

/* Nearest neighbor x direction */

static void scale_rgb_16_x_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint16_t * src, *dst;
  src = (uint16_t*)(ctx->src + scanline * ctx->src_stride);
  dst = (uint16_t*)(dest_start);
  SCALE_FUNC_HEAD
    *dst = src[ctx->table_h.pixels[i].index];
    dst++;
  SCALE_FUNC_TAIL
  }

static void scale_uint8_x_1_x_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint8_t * src;
  src = (uint8_t*)(ctx->src + scanline * ctx->src_stride);

  SCALE_FUNC_HEAD
    *(dest_start) = *(src + ctx->table_h.pixels[i].index * ctx->offset->src_advance);
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }

static void scale_uint8_x_3_x_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint8_t * src, *src1;
  src = ctx->src + (scanline * ctx->src_stride);
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index*ctx->offset->src_advance;
    dest_start[0] = src1[0];
    dest_start[1] = src1[1];
    dest_start[2] = src1[2];
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }

static void scale_uint8_x_4_x_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint8_t * src, *src1;
  src = (uint8_t*)(ctx->src + scanline * ctx->src_stride);
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index*4;
    dest_start[0] = src1[0];
    dest_start[1] = src1[1];
    dest_start[2] = src1[2];
    dest_start[3] = src1[3];
    dest_start += 4;
  SCALE_FUNC_TAIL
  }

static void scale_uint16_x_1_x_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint16_t * src, *src1, *dst;
  src = (uint16_t *)(ctx->src + (scanline * ctx->src_stride));
  dst = (uint16_t *)(dest_start);
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index;
    *dst = *src1;
    dst++;
  SCALE_FUNC_TAIL
  }

static void scale_uint16_x_3_x_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint16_t * src, *src1;
  src = (uint16_t *)(ctx->src + (scanline * ctx->src_stride));
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index*3;
    memcpy(dest_start, src1, 6);
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }

static void scale_uint16_x_4_x_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint16_t * src, *src1;
  src = (uint16_t*)(ctx->src + scanline * ctx->src_stride);
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index*4;
    memcpy(dest_start, src1, 8);
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }

static void scale_float_x_1_x_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  float * src, *src1;
  src = (float*)(ctx->src + (scanline * ctx->src_stride));
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index;
    memcpy(dest_start, src1, sizeof(float));
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }


static void scale_float_x_2_x_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  float * src, *src1;
  src = (float*)(ctx->src + (scanline * ctx->src_stride));
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index*2;
    memcpy(dest_start, src1, 2*sizeof(float));
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }


static void scale_float_x_3_x_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  float * src, *src1;
  src = (float*)(ctx->src + (scanline * ctx->src_stride));
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index*3;
    memcpy(dest_start, src1, 3*sizeof(float));
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }

static void scale_float_x_4_x_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  float * src, *src1;
  src = (float*)(ctx->src + scanline * ctx->src_stride);
  SCALE_FUNC_HEAD
    src1 = src + ctx->table_h.pixels[i].index*4;
    memcpy(dest_start, src1, 4*sizeof(float));
    dest_start += ctx->offset->dst_advance;
  SCALE_FUNC_TAIL
  }

/* Nearest neighbor y direction */

static void scale_rgb_16_y_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  memcpy(dest_start, ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride, 2 * ctx->dst_rect.w);
  }

static void scale_uint8_x_1_y_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i;
  uint8_t * src;
  src = (uint8_t*)(ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride);
    
  SCALE_FUNC_HEAD
    *(dest_start) = *src;
    dest_start += ctx->offset->dst_advance;
    src += ctx->offset->src_advance;
  SCALE_FUNC_TAIL
  }

static void scale_uint8_x_3_y_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  gavl_memcpy(dest_start, ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride,
         ctx->offset->src_advance * ctx->dst_rect.w);
  }

static void scale_uint8_x_4_y_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  gavl_memcpy(dest_start, ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride,
         ctx->offset->src_advance * ctx->dst_rect.w);
  }

static void scale_uint16_x_1_y_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  gavl_memcpy(dest_start, ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride, 2 * ctx->dst_rect.w);
  }

static void scale_uint16_x_3_y_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  gavl_memcpy(dest_start, ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride,
         ctx->offset->src_advance * ctx->dst_rect.w);
  }

static void scale_uint16_x_4_y_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  gavl_memcpy(dest_start, ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride,
         ctx->offset->src_advance * ctx->dst_rect.w);
  }

static void scale_float_x_1_y_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  gavl_memcpy(dest_start, ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride,
         ctx->offset->src_advance * ctx->dst_rect.w);
  }

static void scale_float_x_2_y_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  gavl_memcpy(dest_start, ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride,
         ctx->offset->src_advance * ctx->dst_rect.w);
  }

static void scale_float_x_3_y_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  gavl_memcpy(dest_start, ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride,
         ctx->offset->src_advance * ctx->dst_rect.w);
  }

static void scale_float_x_4_y_nearest_c(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  gavl_memcpy(dest_start, ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride,
         ctx->offset->src_advance * ctx->dst_rect.w);
  }

void gavl_init_scale_funcs_nearest_c(gavl_scale_funcs_t * tab, int src_advance, int dst_advance)
  {
  //   fprintf(stderr, "gavl_init_scale_funcs_nearest_c\n");
  tab->funcs_xy.scale_rgb_15 =     scale_rgb_16_xy_nearest_c;
  tab->funcs_xy.scale_rgb_16 =     scale_rgb_16_xy_nearest_c;
  tab->funcs_xy.scale_uint8_x_1_advance =  scale_uint8_x_1_xy_nearest_c;
  tab->funcs_xy.scale_uint8_x_1_noadvance =  scale_uint8_x_1_xy_nearest_c;
  tab->funcs_xy.scale_uint8_x_2 =  scale_rgb_16_xy_nearest_c;

  if((src_advance == 4) && (dst_advance == 4))
    tab->funcs_xy.scale_uint8_x_3 =  scale_uint8_x_4_xy_nearest_c;
  else
    tab->funcs_xy.scale_uint8_x_3 =  scale_uint8_x_3_xy_nearest_c;
  tab->funcs_xy.scale_uint8_x_4 =  scale_uint8_x_4_xy_nearest_c;
  tab->funcs_xy.scale_uint16_x_1 = scale_uint16_x_1_xy_nearest_c;
  tab->funcs_xy.scale_uint16_x_2 = scale_uint8_x_4_xy_nearest_c;
  tab->funcs_xy.scale_uint16_x_3 = scale_uint16_x_3_xy_nearest_c;
  tab->funcs_xy.scale_uint16_x_4 = scale_uint16_x_4_xy_nearest_c;
  tab->funcs_xy.scale_float_x_1 =  scale_float_x_1_xy_nearest_c;
  tab->funcs_xy.scale_float_x_2 =  scale_float_x_2_xy_nearest_c;
  tab->funcs_xy.scale_float_x_3 =  scale_float_x_3_xy_nearest_c;
  tab->funcs_xy.scale_float_x_4 =  scale_float_x_4_xy_nearest_c;

  tab->funcs_xy.bits_rgb_15 = 0;
  tab->funcs_xy.bits_rgb_16 = 0;
  tab->funcs_xy.bits_uint8_advance  = 0;
  tab->funcs_xy.bits_uint8_noadvance  = 0;
  tab->funcs_xy.bits_uint16 = 0;

  tab->funcs_x.scale_rgb_15 =     scale_rgb_16_x_nearest_c;
  tab->funcs_x.scale_rgb_16 =     scale_rgb_16_x_nearest_c;
  tab->funcs_x.scale_uint8_x_1_noadvance =  scale_uint8_x_1_x_nearest_c;
  tab->funcs_x.scale_uint8_x_1_advance =  scale_uint8_x_1_x_nearest_c;
  tab->funcs_x.scale_uint8_x_2 =  scale_rgb_16_x_nearest_c;

  if((src_advance == 4) && (dst_advance == 4))
    tab->funcs_x.scale_uint8_x_3 =  scale_uint8_x_4_x_nearest_c;
  else
    tab->funcs_x.scale_uint8_x_3 =  scale_uint8_x_3_x_nearest_c;
  
  tab->funcs_x.scale_uint8_x_4 =  scale_uint8_x_4_x_nearest_c;
  tab->funcs_x.scale_uint16_x_1 = scale_uint16_x_1_x_nearest_c;
  tab->funcs_x.scale_uint16_x_2 = scale_uint8_x_4_x_nearest_c;
  tab->funcs_x.scale_uint16_x_3 = scale_uint16_x_3_x_nearest_c;
  tab->funcs_x.scale_uint16_x_4 = scale_uint16_x_4_x_nearest_c;
  tab->funcs_x.scale_float_x_1 =  scale_float_x_1_x_nearest_c;
  tab->funcs_x.scale_float_x_2 =  scale_float_x_2_x_nearest_c;
  tab->funcs_x.scale_float_x_3 =  scale_float_x_3_x_nearest_c;
  tab->funcs_x.scale_float_x_4 =  scale_float_x_4_x_nearest_c;

  tab->funcs_x.bits_rgb_15 = 0;
  tab->funcs_x.bits_rgb_16 = 0;
  tab->funcs_x.bits_uint8_advance  = 0;
  tab->funcs_x.bits_uint8_noadvance  = 0;
  tab->funcs_x.bits_uint16 = 0;

  tab->funcs_y.scale_rgb_15 =     scale_rgb_16_y_nearest_c;
  tab->funcs_y.scale_rgb_16 =     scale_rgb_16_y_nearest_c;
  tab->funcs_y.scale_uint8_x_1_advance =  scale_uint8_x_1_y_nearest_c;
  tab->funcs_y.scale_uint8_x_1_noadvance =  scale_uint8_x_1_y_nearest_c;
  tab->funcs_y.scale_uint8_x_2 =  scale_rgb_16_y_nearest_c;

  if((src_advance == 4) && (dst_advance == 4))
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_4_y_nearest_c;
  else
    tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_3_y_nearest_c;

  tab->funcs_y.scale_uint8_x_4 =  scale_uint8_x_4_y_nearest_c;
  tab->funcs_y.scale_uint16_x_1 = scale_uint16_x_1_y_nearest_c;
  tab->funcs_y.scale_uint16_x_2 = scale_uint8_x_4_y_nearest_c;
  tab->funcs_y.scale_uint16_x_3 = scale_uint16_x_3_y_nearest_c;
  tab->funcs_y.scale_uint16_x_4 = scale_uint16_x_4_y_nearest_c;
  tab->funcs_y.scale_float_x_1 =  scale_float_x_1_y_nearest_c;
  tab->funcs_y.scale_float_x_2 =  scale_float_x_2_y_nearest_c;
  tab->funcs_y.scale_float_x_3 =  scale_float_x_3_y_nearest_c;
  tab->funcs_y.scale_float_x_4 =  scale_float_x_4_y_nearest_c;

  tab->funcs_y.bits_rgb_15 = 0;
  tab->funcs_y.bits_rgb_16 = 0;
  tab->funcs_y.bits_uint8_advance  = 0;
  tab->funcs_y.bits_uint8_noadvance  = 0;
  tab->funcs_y.bits_uint16 = 0;
  
  }
