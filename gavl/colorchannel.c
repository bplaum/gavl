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

#include <stdlib.h> // NULL

#include <gavl/gavl.h>
#include "c/colorspace_tables.h"
#include "c/colorspace_macros.h"

typedef struct channel_data_s channel_data_t;

typedef void (*channel_func)(channel_data_t * data,
                             const gavl_video_frame_t * src,
                             gavl_video_frame_t * dst);


struct channel_data_s
  {
  int plane;
  int offset;
  int advance;
  int sub_h;
  int sub_v;

  int width;
  int height;
  
  channel_func extract_func;
  channel_func insert_func;
  };

/*
 *  Extract functions
 */

static void extract_rgb15_r(channel_data_t * d,
                            const gavl_video_frame_t * src,
                            gavl_video_frame_t * dst)
  {
  int i, j;
  const uint16_t * src_ptr;
  uint8_t * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = (uint16_t *)src_ptr_start + d->offset;
    dst_ptr = dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = RGB15_TO_R_8(*src_ptr);
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }
  }

static void extract_rgb15_g(channel_data_t * d,
                            const gavl_video_frame_t * src,
                            gavl_video_frame_t * dst)
  {
  int i, j;
  const uint16_t * src_ptr;
  uint8_t * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = (uint16_t *)src_ptr_start + d->offset;
    dst_ptr = dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = RGB15_TO_G_8(*src_ptr);
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }
  }

static void extract_rgb15_b(channel_data_t * d,
                            const gavl_video_frame_t * src,
                            gavl_video_frame_t * dst)
  {
  int i, j;
  const uint16_t * src_ptr;
  uint8_t * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = (uint16_t *)src_ptr_start + d->offset;
    dst_ptr = dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = RGB15_TO_B_8(*src_ptr);
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }

  }

static void extract_rgb16_r(channel_data_t * d,
                            const gavl_video_frame_t * src,
                            gavl_video_frame_t * dst)
  {
  int i, j;
  const uint16_t * src_ptr;
  uint8_t * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = (uint16_t *)src_ptr_start + d->offset;
    dst_ptr = dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = RGB16_TO_R_8(*src_ptr);
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }
  }

static void extract_rgb16_g(channel_data_t * d,
                            const gavl_video_frame_t * src,
                            gavl_video_frame_t * dst)
  {
  int i, j;
  const uint16_t * src_ptr;
  uint8_t * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = (uint16_t *)src_ptr_start + d->offset;
    dst_ptr = dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = RGB16_TO_G_8(*src_ptr);
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }
  }

static void extract_rgb16_b(channel_data_t * d,
                            const gavl_video_frame_t * src,
                            gavl_video_frame_t * dst)
  {
  int i, j;
  const uint16_t * src_ptr;
  uint8_t * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = (uint16_t *)src_ptr_start + d->offset;
    dst_ptr = dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = RGB16_TO_B_8(*src_ptr);
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }

  }

static void extract_8(channel_data_t * d,
                      const gavl_video_frame_t * src,
                      gavl_video_frame_t * dst)
  {
  int i, j;
  const uint8_t * src_ptr;
  uint8_t * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = src_ptr_start + d->offset;
    dst_ptr = dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = *src_ptr;
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }
  }


static void extract_8_y(channel_data_t * d,
                        const gavl_video_frame_t * src,
                        gavl_video_frame_t * dst)
  {
  int i, j;
  const uint8_t * src_ptr;
  uint8_t * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = src_ptr_start + d->offset;
    dst_ptr = dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = Y_8_TO_YJ_8(*src_ptr);
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }
  }

static void extract_8_uv(channel_data_t * d,
                         const gavl_video_frame_t * src,
                         gavl_video_frame_t * dst)
  {
  int i, j;
  const uint8_t * src_ptr;
  uint8_t * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = src_ptr_start + d->offset;
    dst_ptr = dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = UV_8_TO_UVJ_8(*src_ptr);
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }
  
  }

static void extract_16(channel_data_t * d,
                       const gavl_video_frame_t * src,
                       gavl_video_frame_t * dst)
  {
  int i, j;
  const uint16_t * src_ptr;
  uint16_t * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = (uint16_t *)src_ptr_start + d->offset;
    dst_ptr = (uint16_t *)dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = *src_ptr;
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }
  }

static void extract_16_y(channel_data_t * d,
                         const gavl_video_frame_t * src,
                         gavl_video_frame_t * dst)
  {
  int i, j;
  const uint16_t * src_ptr;
  uint16_t * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = (uint16_t *)src_ptr_start + d->offset;
    dst_ptr = (uint16_t *)dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      Y_16_TO_YJ_16(*src_ptr, *dst_ptr);
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }
  
  }

static void extract_16_uv(channel_data_t * d,
                          const gavl_video_frame_t * src,
                          gavl_video_frame_t * dst)
  {
  int i, j;
  const uint16_t * src_ptr;
  uint16_t * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = (uint16_t *)src_ptr_start + d->offset;
    dst_ptr = (uint16_t *)dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      UV_16_TO_UVJ_16(*src_ptr, *dst_ptr);
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }
  
  }

static void extract_float(channel_data_t * d,
                          const gavl_video_frame_t * src,
                          gavl_video_frame_t * dst)
  {
  int i, j;
  const float * src_ptr;
  float * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = (float *)src_ptr_start + d->offset;
    dst_ptr = (float *)dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = *src_ptr;
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }
  }

static void extract_float_uv(channel_data_t * d,
                             const gavl_video_frame_t * src,
                             gavl_video_frame_t * dst)
  {
  int i, j;
  const float * src_ptr;
  float * dst_ptr;

  const uint8_t * src_ptr_start = src->planes[d->plane];
  uint8_t * dst_ptr_start = dst->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    src_ptr = (float *)src_ptr_start + d->offset;
    dst_ptr = (float *)dst_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = *src_ptr + 0.5; // -0.5 .. 0.5 -> 0.0 .. 1.0
      dst_ptr++;
      src_ptr += d->advance;
      }
    src_ptr_start += src->strides[d->plane];
    dst_ptr_start += dst->strides[0];
    }
  }

/*
 *  Insert functions
 */

// RRRRRGGGGGGBBBBB

#define INSERT_RGB16_R(src, dst) \
  dst &= ~RGB16_UPPER_MASK; \
  dst |= ((int)src<<8)&RGB16_UPPER_MASK

#define INSERT_RGB16_G(src, dst) \
  dst &= ~RGB16_MIDDLE_MASK; \
  dst |= ((int)src<<3)&RGB16_MIDDLE_MASK

#define INSERT_RGB16_B(src, dst) \
  dst &= ~RGB15_LOWER_MASK; \
  dst |= (src>>3)

// 0RRRRRGGGGGBBBBB

#define INSERT_RGB15_R(src, dst) \
  dst &= ~RGB15_UPPER_MASK; \
  dst |= ((int)src<<7)&RGB15_UPPER_MASK

#define INSERT_RGB15_G(src, dst) \
  dst &= ~RGB15_MIDDLE_MASK; \
  dst |= ((int)src<<2)&RGB15_MIDDLE_MASK

#define INSERT_RGB15_B(src, dst) \
  dst &= ~RGB15_LOWER_MASK; \
  dst |= (src>>3)



static void insert_rgb15_r(channel_data_t * d,
                            const gavl_video_frame_t * src,
                            gavl_video_frame_t * dst)
  {
  int i, j;
  uint16_t * dst_ptr;
  const uint8_t * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = (uint16_t *)dst_ptr_start + d->offset;
    src_ptr = src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      INSERT_RGB15_R(*src_ptr, *dst_ptr);
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }
  }

static void insert_rgb15_g(channel_data_t * d,
                            const gavl_video_frame_t * src,
                            gavl_video_frame_t * dst)
  {
  int i, j;
  uint16_t * dst_ptr;
  const uint8_t * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = (uint16_t *)dst_ptr_start + d->offset;
    src_ptr = src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      INSERT_RGB15_G(*src_ptr, *dst_ptr);
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }
  }

static void insert_rgb15_b(channel_data_t * d,
                            const gavl_video_frame_t * src,
                            gavl_video_frame_t * dst)
  {
  int i, j;
  uint16_t * dst_ptr;
  const uint8_t * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = (uint16_t *)dst_ptr_start + d->offset;
    src_ptr = src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      INSERT_RGB15_B(*src_ptr, *dst_ptr);
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }
  }

static void insert_rgb16_r(channel_data_t * d,
                            const gavl_video_frame_t * src,
                            gavl_video_frame_t * dst)
  {
  int i, j;
  uint16_t * dst_ptr;
  const uint8_t * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = (uint16_t *)dst_ptr_start + d->offset;
    src_ptr = src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      INSERT_RGB16_R(*src_ptr, *dst_ptr);
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }
  }

static void insert_rgb16_g(channel_data_t * d,
                            const gavl_video_frame_t * src,
                            gavl_video_frame_t * dst)
  {
  int i, j;
  uint16_t * dst_ptr;
  const uint8_t * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = (uint16_t *)dst_ptr_start + d->offset;
    src_ptr = src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      INSERT_RGB16_G(*src_ptr, *dst_ptr);
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }

  }

static void insert_rgb16_b(channel_data_t * d,
                            const gavl_video_frame_t * src,
                            gavl_video_frame_t * dst)
  {
  int i, j;
  uint16_t * dst_ptr;
  const uint8_t * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = (uint16_t *)dst_ptr_start + d->offset;
    src_ptr = src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      INSERT_RGB16_B(*src_ptr, *dst_ptr);
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }
  }

static void insert_8(channel_data_t * d,
                      const gavl_video_frame_t * src,
                      gavl_video_frame_t * dst)
  {
  int i, j;
  uint8_t * dst_ptr;
  const uint8_t * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = dst_ptr_start + d->offset;
    src_ptr = src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = *src_ptr;
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }
  }

static void insert_8_y(channel_data_t * d,
                        const gavl_video_frame_t * src,
                        gavl_video_frame_t * dst)
  {
  int i, j;
  uint8_t * dst_ptr;
  const uint8_t * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = dst_ptr_start + d->offset;
    src_ptr = src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = YJ_8_TO_Y_8(*src_ptr);
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }
  }

static void insert_8_uv(channel_data_t * d,
                         const gavl_video_frame_t * src,
                         gavl_video_frame_t * dst)
  {
  int i, j;
  uint8_t * dst_ptr;
  const uint8_t * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = dst_ptr_start + d->offset;
    src_ptr = src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = UVJ_8_TO_UV_8(*src_ptr);
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }
  }

static void insert_16(channel_data_t * d,
                       const gavl_video_frame_t * src,
                       gavl_video_frame_t * dst)
  {
  int i, j;
  uint16_t * dst_ptr;
  const uint16_t * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = (uint16_t *)dst_ptr_start + d->offset;
    src_ptr = (uint16_t *)src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = *src_ptr;
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }
  }

static void insert_16_y(channel_data_t * d,
                         const gavl_video_frame_t * src,
                         gavl_video_frame_t * dst)
  {
  int i, j;
  uint16_t * dst_ptr;
  const uint16_t * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = (uint16_t *)dst_ptr_start + d->offset;
    src_ptr = (uint16_t *)src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      YJ_16_TO_Y_16(*src_ptr, *dst_ptr);
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }
  }

static void insert_16_uv(channel_data_t * d,
                          const gavl_video_frame_t * src,
                          gavl_video_frame_t * dst)
  {
  int i, j;
  uint16_t * dst_ptr;
  const uint16_t * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = (uint16_t *)dst_ptr_start + d->offset;
    src_ptr = (uint16_t *)src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      UVJ_16_TO_UV_16(*src_ptr, *dst_ptr);
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }
  
  }

static void insert_float(channel_data_t * d,
                          const gavl_video_frame_t * src,
                          gavl_video_frame_t * dst)
  {
  int i, j;
  float * dst_ptr;
  const float * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = (float*)dst_ptr_start + d->offset;
    src_ptr = (float*)src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = *src_ptr;
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }
  }

static void insert_float_uv(channel_data_t * d,
                             const gavl_video_frame_t * src,
                             gavl_video_frame_t * dst)
  {
  int i, j;
  float * dst_ptr;
  const float * src_ptr;

  uint8_t * dst_ptr_start = dst->planes[d->plane];
  const uint8_t * src_ptr_start = src->planes[0];
  
  for(i = 0; i < d->height; i++)
    {
    dst_ptr = (float*)dst_ptr_start + d->offset;
    src_ptr = (float*)src_ptr_start;
    
    for(j = 0; j < d->width; j++)
      {
      *dst_ptr = *src_ptr - 0.5;
      src_ptr++;
      dst_ptr += d->advance;
      }
    dst_ptr_start += dst->strides[d->plane];
    src_ptr_start += src->strides[0];
    }
  }

/* Get channel properties */

static int get_channel_properties(gavl_pixelformat_t src_format,
                                  gavl_pixelformat_t * dst_format_ret,
                                  gavl_color_channel_t ch,
                                  channel_data_t * d)
  {
  gavl_pixelformat_t dst_format = GAVL_PIXELFORMAT_NONE;
  
  d->plane = 0;
  d->offset = 0;
  d->advance = 1;
  d->sub_h = 1;
  d->sub_v = 1;
  
  switch(src_format)
    {
    case GAVL_PIXELFORMAT_NONE:
      return 0;
    case GAVL_GRAY_8:
      dst_format = GAVL_GRAY_8;
      d->extract_func = extract_8;
      d->insert_func = insert_8;
      switch(ch)
        {
        case GAVL_CCH_Y:
          break;
        default:
          return 0;
        }
      break;
    case GAVL_GRAY_16:
      dst_format = GAVL_GRAY_16;
      d->extract_func = extract_16;
      d->insert_func = insert_16;
      switch(ch)
        {
        case GAVL_CCH_Y:
          break;
        default:
          return 0;
        }
      break;
    case GAVL_GRAY_FLOAT:
      dst_format = GAVL_GRAY_FLOAT;
      d->extract_func = extract_float;
      d->insert_func = insert_float;
      switch(ch)
        {
        case GAVL_CCH_Y:
          break;
        default:
          return 0;
        }
      break;
    case GAVL_GRAYA_16:
      dst_format = GAVL_GRAY_8;
      d->extract_func = extract_8;
      d->insert_func = insert_8;
      d->advance = 2;
      switch(ch)
        {
        case GAVL_CCH_Y:
          break;
        case GAVL_CCH_ALPHA:
          d->offset  = 1;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_GRAYA_32:
      dst_format = GAVL_GRAY_16;
      d->extract_func = extract_16;
      d->insert_func = insert_16;
      d->advance = 2;
      switch(ch)
        {
        case GAVL_CCH_Y:
          break;
        case GAVL_CCH_ALPHA:
          d->offset  = 1;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_GRAYA_FLOAT:
      dst_format = GAVL_GRAY_FLOAT;
      d->extract_func = extract_float;
      d->insert_func = insert_float;
      d->advance = 2;
      switch(ch)
        {
        case GAVL_CCH_Y:
          break;
        case GAVL_CCH_ALPHA:
          d->offset  = 1;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_RGB_15:
      dst_format = GAVL_GRAY_8;
      switch(ch)
        {
        case GAVL_CCH_RED:
          d->extract_func = extract_rgb15_r;
          d->insert_func  = insert_rgb15_r;
          break;
        case GAVL_CCH_GREEN:
          d->extract_func = extract_rgb15_g;
          d->insert_func  = insert_rgb15_g;
          break;
        case GAVL_CCH_BLUE:
          d->extract_func = extract_rgb15_b;
          d->insert_func  = insert_rgb15_b;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_BGR_15:
      dst_format = GAVL_GRAY_8;
      switch(ch)
        {
        case GAVL_CCH_RED:
          d->extract_func = extract_rgb15_b;
          d->insert_func  = insert_rgb15_b;
          break;
        case GAVL_CCH_GREEN:
          d->extract_func = extract_rgb15_g;
          d->insert_func  = insert_rgb15_g;
          break;
        case GAVL_CCH_BLUE:
          d->extract_func = extract_rgb15_r;
          d->insert_func  = insert_rgb15_r;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_RGB_16:
      dst_format = GAVL_GRAY_8;
      switch(ch)
        {
        case GAVL_CCH_RED:
          d->extract_func = extract_rgb16_r;
          d->insert_func  = insert_rgb16_r;
          break;
        case GAVL_CCH_GREEN:
          d->extract_func = extract_rgb16_g;
          d->insert_func  = insert_rgb16_g;
          break;
        case GAVL_CCH_BLUE:
          d->extract_func = extract_rgb16_b;
          d->insert_func  = insert_rgb16_b;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_BGR_16:
      dst_format = GAVL_GRAY_8;
      switch(ch)
        {
        case GAVL_CCH_RED:
          d->extract_func = extract_rgb16_b;
          d->insert_func  = insert_rgb16_b;
          break;
        case GAVL_CCH_GREEN:
          d->extract_func = extract_rgb16_g;
          d->insert_func  = insert_rgb16_g;
          break;
        case GAVL_CCH_BLUE:
          d->extract_func = extract_rgb16_r;
          d->insert_func  = insert_rgb16_r;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_RGB_24:
      dst_format = GAVL_GRAY_8;
      d->extract_func = extract_8;
      d->insert_func = insert_8;
      d->advance = 3;
      switch(ch)
        {
        case GAVL_CCH_RED:
          d->offset  = 0;
          break;
        case GAVL_CCH_GREEN:
          d->offset  = 1;
          break;
        case GAVL_CCH_BLUE:
          d->offset  = 2;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_BGR_24:
      dst_format = GAVL_GRAY_8;
      d->extract_func = extract_8;
      d->insert_func = insert_8;
      d->advance = 3;
      switch(ch)
        {
        case GAVL_CCH_RED:
          d->offset  = 2;
          break;
        case GAVL_CCH_GREEN:
          d->offset  = 1;
          break;
        case GAVL_CCH_BLUE:
          d->offset  = 0;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_RGB_32:
      dst_format = GAVL_GRAY_8;
      d->extract_func = extract_8;
      d->insert_func = insert_8;
      d->advance = 4;
      switch(ch)
        {
        case GAVL_CCH_RED:
          d->offset  = 0;
          break;
        case GAVL_CCH_GREEN:
          d->offset  = 1;
          break;
        case GAVL_CCH_BLUE:
          d->offset  = 2;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_BGR_32:
      dst_format = GAVL_GRAY_8;
      d->extract_func = extract_8;
      d->insert_func = insert_8;
      d->advance = 4;
      switch(ch)
        {
        case GAVL_CCH_RED:
          d->offset  = 2;
          break;
        case GAVL_CCH_GREEN:
          d->offset  = 1;
          break;
        case GAVL_CCH_BLUE:
          d->offset  = 0;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_RGBA_32:
      dst_format = GAVL_GRAY_8;
      d->extract_func = extract_8;
      d->insert_func = insert_8;
      d->advance = 4;
      switch(ch)
        {
        case GAVL_CCH_RED:
          d->offset  = 0;
          break;
        case GAVL_CCH_GREEN:
          d->offset  = 1;
          break;
        case GAVL_CCH_BLUE:
          d->offset  = 2;
          break;
        case GAVL_CCH_ALPHA:
          d->offset  = 3;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_RGB_48:
      dst_format = GAVL_GRAY_16;
      d->extract_func = extract_16;
      d->insert_func = insert_16;
      d->advance = 3;
      switch(ch)
        {
        case GAVL_CCH_RED:
          d->offset  = 0;
          break;
        case GAVL_CCH_GREEN:
          d->offset  = 1;
          break;
        case GAVL_CCH_BLUE:
          d->offset  = 2;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_RGBA_64:
      dst_format = GAVL_GRAY_16;
      d->extract_func = extract_16;
      d->insert_func = insert_16;
      d->advance = 4;
      switch(ch)
        {
        case GAVL_CCH_RED:
          d->offset  = 0;
          break;
        case GAVL_CCH_GREEN:
          d->offset  = 1;
          break;
        case GAVL_CCH_BLUE:
          d->offset  = 2;
          break;
        case GAVL_CCH_ALPHA:
          d->offset  = 3;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_RGB_FLOAT:
      dst_format = GAVL_GRAY_FLOAT;
      d->extract_func = extract_float;
      d->insert_func = insert_float;
      d->advance = 3;
      switch(ch)
        {
        case GAVL_CCH_RED:
          d->offset  = 0;
          break;
        case GAVL_CCH_GREEN:
          d->offset  = 1;
          break;
        case GAVL_CCH_BLUE:
          d->offset  = 2;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_RGBA_FLOAT:
      dst_format = GAVL_GRAY_FLOAT;
      d->extract_func = extract_float;
      d->insert_func = insert_float;
      d->advance = 4;
      switch(ch)
        {
        case GAVL_CCH_RED:
          d->offset  = 0;
          break;
        case GAVL_CCH_GREEN:
          d->offset  = 1;
          break;
        case GAVL_CCH_BLUE:
          d->offset  = 2;
          break;
        case GAVL_CCH_ALPHA:
          d->offset  = 3;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_YUY2:
      dst_format = GAVL_GRAY_8;
      switch(ch)
        {
        case GAVL_CCH_Y:
          d->extract_func = extract_8_y;
          d->insert_func = insert_8_y;
          d->offset  = 0;
          d->advance = 2;
          break;
        case GAVL_CCH_CB:
          d->extract_func = extract_8_uv;
          d->insert_func = insert_8_uv;
          d->offset  = 1;
          d->advance = 4;
          break;
        case GAVL_CCH_CR:
          d->extract_func = extract_8_uv;
          d->insert_func = insert_8_uv;
          d->offset  = 3;
          d->advance = 4;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_UYVY:
      dst_format = GAVL_GRAY_8;
      switch(ch)
        {
        case GAVL_CCH_Y:
          d->extract_func = extract_8_y;
          d->insert_func = insert_8_y;
          d->offset  = 1;
          d->advance = 2;
          break;
        case GAVL_CCH_CB:
          d->extract_func = extract_8_uv;
          d->insert_func = insert_8_uv;
          d->offset  = 0;
          d->advance = 4;
          break;
        case GAVL_CCH_CR:
          d->extract_func = extract_8_uv;
          d->insert_func = insert_8_uv;
          d->offset  = 2;
          d->advance = 4;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_YUVA_32:
      dst_format = GAVL_GRAY_8;
      d->advance = 4;
      switch(ch)
        {
        case GAVL_CCH_Y:
          d->extract_func = extract_8_y;
          d->insert_func = insert_8_y;
          d->offset  = 0;
          break;
        case GAVL_CCH_CB:
          d->extract_func = extract_8_uv;
          d->insert_func = insert_8_uv;
          d->offset  = 1;
          break;
        case GAVL_CCH_CR:
          d->extract_func = extract_8_uv;
          d->insert_func = insert_8_uv;
          d->offset  = 2;
          break;
        case GAVL_CCH_ALPHA:
          d->offset  = 3;
          d->extract_func = extract_8;
          d->insert_func = insert_8;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_YUVA_64:
      dst_format = GAVL_GRAY_16;
      d->advance = 4;
      switch(ch)
        {
        case GAVL_CCH_Y:
          d->extract_func = extract_16_y;
          d->insert_func = insert_16_y;
          d->offset  = 0;
          break;
        case GAVL_CCH_CB:
          d->extract_func = extract_16_uv;
          d->insert_func = insert_16_uv;
          d->offset  = 1;
          break;
        case GAVL_CCH_CR:
          d->extract_func = extract_16_uv;
          d->insert_func = insert_16_uv;
          d->offset  = 2;
          break;
        case GAVL_CCH_ALPHA:
          d->extract_func = extract_16;
          d->insert_func = insert_16;
          d->offset  = 3;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_YUV_FLOAT:
      dst_format = GAVL_GRAY_FLOAT;
      d->advance = 3;
      switch(ch)
        {
        case GAVL_CCH_Y:
          d->extract_func = extract_float;
          d->insert_func = insert_float;
          d->offset  = 0;
          break;
        case GAVL_CCH_CB:
          d->extract_func = extract_float_uv;
          d->insert_func = insert_float_uv;
          d->offset  = 1;
          break;
        case GAVL_CCH_CR:
          d->extract_func = extract_float_uv;
          d->insert_func = insert_float_uv;
          d->offset  = 2;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_YUVA_FLOAT:
      dst_format = GAVL_GRAY_FLOAT;
      d->advance = 4;
      switch(ch)
        {
        case GAVL_CCH_Y:
          d->extract_func = extract_float;
          d->insert_func = insert_float;
          d->offset  = 0;
          break;
        case GAVL_CCH_CB:
          d->extract_func = extract_float_uv;
          d->insert_func = insert_float_uv;
          d->offset  = 1;
          break;
        case GAVL_CCH_CR:
          d->extract_func = extract_float_uv;
          d->insert_func = insert_float_uv;
          d->offset  = 2;
          break;
        case GAVL_CCH_ALPHA:
          d->extract_func = extract_float;
          d->insert_func = insert_float;
          d->offset  = 3;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_444_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_410_P:
      dst_format = GAVL_GRAY_8;
      switch(ch)
        {
        case GAVL_CCH_Y:
          d->extract_func = extract_8_y;
          d->insert_func = insert_8_y;
          d->plane  = 0;
          break;
        case GAVL_CCH_CB:
          d->extract_func = extract_8_uv;
          d->insert_func = insert_8_uv;
          d->plane  = 1;
          break;
        case GAVL_CCH_CR:
          d->extract_func = extract_8_uv;
          d->insert_func = insert_8_uv;
          d->plane  = 2;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
      dst_format = GAVL_GRAY_8;
      d->extract_func = extract_8;
      d->insert_func = insert_8;
      switch(ch)
        {
        case GAVL_CCH_Y:
          d->plane  = 0;
          break;
        case GAVL_CCH_CB:
          d->plane  = 1;
          break;
        case GAVL_CCH_CR:
          d->plane  = 2;
          break;
        default:
          return 0;
        }
      break;
    case GAVL_YUV_444_P_16:
    case GAVL_YUV_422_P_16:
      dst_format = GAVL_GRAY_16;
      switch(ch)
        {
        case GAVL_CCH_Y:
          d->extract_func = extract_16_y;
          d->insert_func = insert_16_y;
          d->plane  = 0;
          break;
        case GAVL_CCH_CB:
          d->extract_func = extract_16_uv;
          d->insert_func = insert_16_uv;
          d->plane  = 1;
          break;
        case GAVL_CCH_CR:
          d->extract_func = extract_16_uv;
          d->insert_func = insert_16_uv;
          d->plane  = 2;
          break;
        default:
          return 0;
        }
      break;
    }

  /* Get chroma subsampling */
  
  if(d->plane ||
     (((src_format == GAVL_YUY2) ||
       (src_format == GAVL_UYVY)) && (ch != GAVL_CCH_Y)))
    gavl_pixelformat_chroma_sub(src_format, &d->sub_h, &d->sub_v);
  
  /* Return stuff */
  if(dst_format_ret)
    *dst_format_ret = dst_format;
  
  return 1;
  }

int gavl_get_color_channel_format(const gavl_video_format_t * frame_format,
                                  gavl_video_format_t * channel_format,
                                  gavl_color_channel_t ch)
  {
  channel_data_t d;
  
  gavl_video_format_copy(channel_format, frame_format);

  if(!get_channel_properties(frame_format->pixelformat,
                             &channel_format->pixelformat,
                             ch, &d))
    return 0;

  channel_format->image_width /= d.sub_h;
  channel_format->frame_width /= d.sub_h;

  channel_format->image_height /= d.sub_v;
  channel_format->frame_height /= d.sub_v;
  return 1;
  }

int
gavl_video_frame_extract_channel(const gavl_video_format_t * format,
                                 gavl_color_channel_t ch,
                                 const gavl_video_frame_t * src,
                                 gavl_video_frame_t * dst)
  {
  channel_data_t d;
  if(!get_channel_properties(format->pixelformat,
                             NULL,
                             ch, &d))
    return 0;

  d.width  = format->image_width  / d.sub_h;
  d.height = format->image_height / d.sub_v;

  d.extract_func(&d, src, dst);
  
  return 1;
  }

int
gavl_video_frame_insert_channel(const gavl_video_format_t * format,
                               gavl_color_channel_t ch,
                               const gavl_video_frame_t * src,
                               gavl_video_frame_t * dst)
  {
  channel_data_t d;
  if(!get_channel_properties(format->pixelformat,
                             NULL,
                             ch, &d))
    return 0;

  d.width  = format->image_width  / d.sub_h;
  d.height = format->image_height / d.sub_v;

  d.insert_func(&d, src, dst);
  return 1;
  }
