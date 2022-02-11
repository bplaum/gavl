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

#include <gavl.h>
#include <stdlib.h>
#include <math.h>
#include "c/colorspace_tables.h"
#include "c/colorspace_macros.h"

static void absdiff_rgb_16(uint8_t * dst, int dst_stride,
                           const uint8_t * src1, int src1_stride,
                           const uint8_t * src2, int src2_stride,
                           int w, int h)
  {
  int i, j;
  uint16_t * d;
  const uint16_t *s1;
  const uint16_t *s2;

  int r, g, b;
  
  for(i = 0; i < h; i++)
    {
    d = (uint16_t *)(dst + i * dst_stride);
    s1 = (const uint16_t *)(src1 + i * src1_stride);
    s2 = (const uint16_t *)(src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      r = abs(RGB16_TO_R_8(*s1) - RGB16_TO_R_8(*s2));
      g = abs(RGB16_TO_G_8(*s1) - RGB16_TO_G_8(*s2));
      b = abs(RGB16_TO_B_8(*s1) - RGB16_TO_B_8(*s2));
      PACK_8_TO_RGB16(r,g,b,*d);
      d++;
      s1++;
      s2++;
      }
    }
  }

static void absdiff_rgb_15(uint8_t * dst, int dst_stride,
                           const uint8_t * src1, int src1_stride,
                           const uint8_t * src2, int src2_stride,
                           int w, int h)
  {
  int i, j;
  uint16_t * d;
  const uint16_t *s1;
  const uint16_t *s2;

  int r, g, b;
  
  for(i = 0; i < h; i++)
    {
    d = (uint16_t *)(dst + i * dst_stride);
    s1 = (const uint16_t *)(src1 + i * src1_stride);
    s2 = (const uint16_t *)(src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      r = abs(RGB15_TO_R_8(*s1) - RGB15_TO_R_8(*s2));
      g = abs(RGB15_TO_G_8(*s1) - RGB15_TO_G_8(*s2));
      b = abs(RGB15_TO_B_8(*s1) - RGB15_TO_B_8(*s2));
      PACK_8_TO_RGB15(r,g,b,*d);
      d++;
      s1++;
      s2++;
      }
    }
  }


static void absdiff_8(uint8_t * dst, int dst_stride,
                      const uint8_t * src1, int src1_stride,
                      const uint8_t * src2, int src2_stride,
                      int w, int h)
  {
  int i, j;
  uint8_t * d;
  const uint8_t *s1;
  const uint8_t *s2;

  for(i = 0; i < h; i++)
    {
    d = (dst + i * dst_stride);
    s1 = (src1 + i * src1_stride);
    s2 = (src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      *d = abs(*s1 - *s2);
      d++;
      s1++;
      s2++;
      }
    }
  }

static void absdiff_16(uint8_t * dst, int dst_stride,
                       const uint8_t * src1, int src1_stride,
                       const uint8_t * src2, int src2_stride,
                       int w, int h)
  {
  int i, j;
  uint16_t * d;
  const uint16_t *s1;
  const uint16_t *s2;

  for(i = 0; i < h; i++)
    {
    d = (uint16_t *)(dst + i * dst_stride);
    s1 = (const uint16_t *)(src1 + i * src1_stride);
    s2 = (const uint16_t *)(src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      *d = abs(*s1 - *s2);
      d++;
      s1++;
      s2++;
      }
    }
  }

static void absdiff_float(uint8_t * dst, int dst_stride,
                          const uint8_t * src1, int src1_stride,
                          const uint8_t * src2, int src2_stride,
                          int w, int h)
  {
  int i, j;
  float * d;
  const float *s1;
  const float *s2;

  for(i = 0; i < h; i++)
    {
    d = (float *)(dst + i * dst_stride);
    s1 = (const float *)(src1 + i * src1_stride);
    s2 = (const float *)(src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      *d = fabs(*s1 - *s2);
      d++;
      s1++;
      s2++;
      }
    }
  }

void gavl_video_frame_absdiff(gavl_video_frame_t * dst,
                              const gavl_video_frame_t * src1,
                              const gavl_video_frame_t * src2,
                              const gavl_video_format_t * format)
  {
  int sub_h, sub_v;
  switch(format->pixelformat)
    {
    case GAVL_GRAY_8:
      absdiff_8(dst->planes[0], dst->strides[0],
                src1->planes[0], src1->strides[0],
                src2->planes[0], src2->strides[0],
                format->image_width, format->image_height);
      break;
    case GAVL_GRAY_16:
      absdiff_16(dst->planes[0], dst->strides[0],
                 src1->planes[0], src1->strides[0],
                 src2->planes[0], src2->strides[0],
                 format->image_width, format->image_height);
  
      break;
    case GAVL_GRAY_FLOAT:
      absdiff_float(dst->planes[0], dst->strides[0],
                    src1->planes[0], src1->strides[0],
                    src2->planes[0], src2->strides[0],
                    format->image_width, format->image_height);
      break;
    case GAVL_GRAYA_16:
    case GAVL_YUY2:
    case GAVL_UYVY:
      absdiff_8(dst->planes[0], dst->strides[0],
                src1->planes[0], src1->strides[0],
                src2->planes[0], src2->strides[0],
                format->image_width*2, format->image_height);
      break;
    case GAVL_GRAYA_32:
      absdiff_16(dst->planes[0], dst->strides[0],
                 src1->planes[0], src1->strides[0],
                 src2->planes[0], src2->strides[0],
                 2 * format->image_width, format->image_height);
  
      break;

    case GAVL_GRAYA_FLOAT:
      absdiff_float(dst->planes[0], dst->strides[0],
                    src1->planes[0], src1->strides[0],
                    src2->planes[0], src2->strides[0],
                    2 * format->image_width, format->image_height);
      break;
    case GAVL_RGB_15:
    case GAVL_BGR_15:
      absdiff_rgb_15(dst->planes[0], dst->strides[0],
                     src1->planes[0], src1->strides[0],
                     src2->planes[0], src2->strides[0],
                     format->image_width, format->image_height);
      break;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
      absdiff_rgb_16(dst->planes[0], dst->strides[0],
                     src1->planes[0], src1->strides[0],
                     src2->planes[0], src2->strides[0],
                     format->image_width, format->image_height);
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
      absdiff_8(dst->planes[0], dst->strides[0],
                src1->planes[0], src1->strides[0],
                src2->planes[0], src2->strides[0],
                format->image_width*3, format->image_height);
      break;
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_RGBA_32:
    case GAVL_YUVA_32:
      absdiff_8(dst->planes[0], dst->strides[0],
                src1->planes[0], src1->strides[0],
                src2->planes[0], src2->strides[0],
                format->image_width*4, format->image_height);
      break;
    case GAVL_RGB_48:
      absdiff_16(dst->planes[0], dst->strides[0],
                 src1->planes[0], src1->strides[0],
                 src2->planes[0], src2->strides[0],
                 3 * format->image_width, format->image_height);
      break;
    case GAVL_RGBA_64:
    case GAVL_YUVA_64:
      absdiff_16(dst->planes[0], dst->strides[0],
                 src1->planes[0], src1->strides[0],
                 src2->planes[0], src2->strides[0],
                 4 * format->image_width, format->image_height);
      break;
    case GAVL_RGB_FLOAT:
    case GAVL_YUV_FLOAT:
      absdiff_float(dst->planes[0], dst->strides[0],
                    src1->planes[0], src1->strides[0],
                    src2->planes[0], src2->strides[0],
                    3 * format->image_width, format->image_height);
      break;
    case GAVL_RGBA_FLOAT:
    case GAVL_YUVA_FLOAT:
      absdiff_float(dst->planes[0], dst->strides[0],
                    src1->planes[0], src1->strides[0],
                    src2->planes[0], src2->strides[0],
                    4 * format->image_width, format->image_height);
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUV_410_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_444_P:
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
      gavl_pixelformat_chroma_sub(format->pixelformat,
                                  &sub_h, &sub_v);

      absdiff_8(dst->planes[0], dst->strides[0],
                src1->planes[0], src1->strides[0],
                src2->planes[0], src2->strides[0],
                format->image_width, format->image_height);
      absdiff_8(dst->planes[1], dst->strides[1],
                src1->planes[1], src1->strides[1],
                src2->planes[1], src2->strides[1],
                format->image_width/sub_h, format->image_height/sub_v);
      absdiff_8(dst->planes[2], dst->strides[2],
                src1->planes[2], src1->strides[2],
                src2->planes[2], src2->strides[2],
                format->image_width/sub_h, format->image_height/sub_v);
      break;
    case GAVL_YUV_444_P_16:
    case GAVL_YUV_422_P_16:
      gavl_pixelformat_chroma_sub(format->pixelformat,
                                  &sub_h, &sub_v);
      absdiff_16(dst->planes[0], dst->strides[0],
                src1->planes[0], src1->strides[0],
                src2->planes[0], src2->strides[0],
                format->image_width, format->image_height);
      absdiff_16(dst->planes[1], dst->strides[1],
                src1->planes[1], src1->strides[1],
                src2->planes[1], src2->strides[1],
                format->image_width/sub_h, format->image_height/sub_v);
      absdiff_16(dst->planes[2], dst->strides[2],
                src1->planes[2], src1->strides[2],
                src2->planes[2], src2->strides[2],
                format->image_width/sub_h, format->image_height/sub_v);
      break;
    case GAVL_PIXELFORMAT_NONE:
      break;
    }
  
  }
