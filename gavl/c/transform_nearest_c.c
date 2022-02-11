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
#include <gavl/gavl.h>
#include <video.h>
#include <transform.h>

#define TRANSFORM_FUNC_HEAD \
  for(i = 0; i < ctx->dst_width; i++)       \
    {

#define TRANSFORM_FUNC_TAIL \
    pixel++; \
    }

static void transform_rgb_16_nearest_c(gavl_transform_context_t * ctx,
                                       gavl_transform_pixel_t * pixels, uint8_t * dest_start)
  {
  int i;
  uint16_t * src, *dst;
  gavl_transform_pixel_t * pixel = pixels;
  dst = (uint16_t*)(dest_start);
  TRANSFORM_FUNC_HEAD
    if(!pixel->outside)
      {
      src = (uint16_t*)(ctx->src +
                        pixel->index_y * ctx->src_stride) +
        pixel->index_x;
      *dst = *src;
      }
  dst++;
  TRANSFORM_FUNC_TAIL
  }

static void transform_uint8_x_1_nearest_c(gavl_transform_context_t * ctx,
                                          gavl_transform_pixel_t * pixels, uint8_t * dest_start)
  {
  int i;
  uint8_t * src, *dst;
  gavl_transform_pixel_t * pixel = pixels;
  dst = (uint8_t*)(dest_start);
  TRANSFORM_FUNC_HEAD
    if(!pixel->outside)
      {
    src = (uint8_t*)(ctx->src +
                      pixel->index_y * ctx->src_stride) +
    pixel->index_x * ctx->advance;
  *dst = *src;
      }
  dst+=ctx->advance;
  TRANSFORM_FUNC_TAIL
  }

static void transform_uint8_x_3_nearest_c(gavl_transform_context_t * ctx,
                                          gavl_transform_pixel_t * pixels, uint8_t * dest_start)
  {
  int i;
  uint8_t * src, *dst;
  gavl_transform_pixel_t * pixel = pixels;
  dst = (uint8_t*)(dest_start);
  TRANSFORM_FUNC_HEAD
    if(!pixel->outside)
      {
    src = (uint8_t*)(ctx->src +
                      pixel->index_y * ctx->src_stride) +
    pixel->index_x * ctx->advance;
  dst[0] = src[0];
  dst[1] = src[1];
  dst[2] = src[2];
      }
  dst+=ctx->advance;
  TRANSFORM_FUNC_TAIL
  }

static void transform_uint8_x_4_nearest_c(gavl_transform_context_t * ctx,
                                          gavl_transform_pixel_t * pixels, uint8_t * dest_start)
  {
  int i;
  uint32_t * src, *dst;
  gavl_transform_pixel_t * pixel = pixels;
  dst = (uint32_t*)(dest_start);
  TRANSFORM_FUNC_HEAD
    if(!pixel->outside)
      {
    src = (uint32_t*)(ctx->src +
                      pixel->index_y * ctx->src_stride) +
    pixel->index_x;
     *dst = *src;
      }
  dst++;
  TRANSFORM_FUNC_TAIL
  }

static void transform_uint16_x_3_nearest_c(gavl_transform_context_t * ctx,
                                           gavl_transform_pixel_t * pixels, uint8_t * dest_start)
  {
  int i;
  uint16_t * src, *dst;
  gavl_transform_pixel_t * pixel = pixels;
  dst = (uint16_t*)(dest_start);
  TRANSFORM_FUNC_HEAD
    if(!pixel->outside)
      {
    src = (uint16_t*)(ctx->src +
                      pixel->index_y * ctx->src_stride) +
    pixel->index_x * 3;
     dst[0] = src[0];
     dst[1] = src[1];
     dst[2] = src[2];
      }
  dst+=3;
  TRANSFORM_FUNC_TAIL
  }

static void transform_uint16_x_4_nearest_c(gavl_transform_context_t * ctx,
                                           gavl_transform_pixel_t * pixels, uint8_t * dest_start)
  {
  int i;
  uint64_t * src, *dst;
  gavl_transform_pixel_t * pixel = pixels;
  dst = (uint64_t*)(dest_start);
  TRANSFORM_FUNC_HEAD
    if(!pixel->outside)
      {
    src = (uint64_t*)(ctx->src +
                      pixel->index_y * ctx->src_stride) +
    pixel->index_x;
     *dst = *src;
      }
  dst++;
  TRANSFORM_FUNC_TAIL
  }

static void
transform_float_x_1_nearest_c(gavl_transform_context_t *
                              ctx, gavl_transform_pixel_t * pixels, uint8_t * dest_start)
  {
  int i;
  float * src, *dst;
  gavl_transform_pixel_t * pixel = pixels;
  dst = (float*)(dest_start);
  TRANSFORM_FUNC_HEAD
    if(!pixel->outside)
      {
    src = (float*)(ctx->src +
                      pixel->index_y * ctx->src_stride) +
    pixel->index_x;
     *dst = *src;
      }
  dst++;
  TRANSFORM_FUNC_TAIL
  }

static void
transform_float_x_2_nearest_c(gavl_transform_context_t * ctx,
                              gavl_transform_pixel_t * pixels, uint8_t * dest_start)
  {
  int i;
  float * src, *dst;
  gavl_transform_pixel_t * pixel = pixels;
  dst = (float*)(dest_start);
  TRANSFORM_FUNC_HEAD
    if(!pixel->outside)
      {
    src = (float*)(ctx->src +
                      pixel->index_y * ctx->src_stride) +
    pixel->index_x * 2;
     dst[0] = src[0];
     dst[1] = src[1];
      }
  dst+=2;
  TRANSFORM_FUNC_TAIL
  }

static void
transform_float_x_3_nearest_c(gavl_transform_context_t * ctx,
                              gavl_transform_pixel_t * pixels, uint8_t * dest_start)
  {
  int i;
  float * src, *dst;
  gavl_transform_pixel_t * pixel = pixels;
  dst = (float*)(dest_start);
  TRANSFORM_FUNC_HEAD
    if(!pixel->outside)
      {
    src = (float*)(ctx->src +
                      pixel->index_y * ctx->src_stride) +
    pixel->index_x * 3;
     dst[0] = src[0];
     dst[1] = src[1];
     dst[2] = src[2];
      }
  dst+=3;
  TRANSFORM_FUNC_TAIL
  }

static void
transform_float_x_4_nearest_c(gavl_transform_context_t * ctx,
                              gavl_transform_pixel_t * pixels, uint8_t * dest_start)
  {
  int i;
  float * src, *dst;
  gavl_transform_pixel_t * pixel = pixels;
  dst = (float*)(dest_start);
  TRANSFORM_FUNC_HEAD
    if(!pixel->outside)
      {
      
    src = (float*)(ctx->src +
                      pixel->index_y * ctx->src_stride) +
    pixel->index_x * 4;
     dst[0] = src[0];
     dst[1] = src[1];
     dst[2] = src[2];
     dst[3] = src[3];
      }
  dst+=4;
  TRANSFORM_FUNC_TAIL
  }




void gavl_init_transform_funcs_nearest_c(gavl_transform_funcs_t * tab,
                                         int advance)
  {
  tab->transform_rgb_15 =     transform_rgb_16_nearest_c;
  tab->transform_rgb_16 =     transform_rgb_16_nearest_c;

  tab->transform_uint8_x_1_advance =
    transform_uint8_x_1_nearest_c;
  tab->transform_uint8_x_1_noadvance =
    transform_uint8_x_1_nearest_c;

  tab->transform_uint8_x_2 =  transform_rgb_16_nearest_c;

  if(advance == 4)
    tab->transform_uint8_x_3 =  transform_uint8_x_4_nearest_c;
  else
    tab->transform_uint8_x_3 =  transform_uint8_x_3_nearest_c;
  tab->transform_uint8_x_4 =  transform_uint8_x_4_nearest_c;
  tab->transform_uint16_x_1 = transform_rgb_16_nearest_c;
  tab->transform_uint16_x_2 = transform_uint8_x_4_nearest_c;
  tab->transform_uint16_x_3 = transform_uint16_x_3_nearest_c;
  tab->transform_uint16_x_4 = transform_uint16_x_4_nearest_c;
  tab->transform_float_x_1 =  transform_float_x_1_nearest_c;
  tab->transform_float_x_2 =  transform_float_x_2_nearest_c;
  tab->transform_float_x_3 =  transform_float_x_3_nearest_c;
  tab->transform_float_x_4 =  transform_float_x_4_nearest_c;

  tab->bits_rgb_15 = 0;
  tab->bits_rgb_16 = 0;
  tab->bits_uint8_advance  = 0;
  tab->bits_uint8_noadvance  = 0;
  tab->bits_uint16_x_1 = 0;
  tab->bits_uint16_x_2 = 0;
  tab->bits_uint16_x_3 = 0;
  tab->bits_uint16_x_4 = 0;

  
  }
