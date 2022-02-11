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

static void psnr_rgb_16(double * dst,
                        const uint8_t * src1, int src1_stride,
                        const uint8_t * src2, int src2_stride,
                        int w, int h)
  {
  int i, j;
  const uint16_t *s1;
  const uint16_t *s2;

  double r = 0.0, g = 0.0, b = 0.0;

  dst[0] = 0.0;
  dst[1] = 0.0;
  dst[2] = 0.0;
  
  for(i = 0; i < h; i++)
    {
    s1 = (const uint16_t *)(src1 + i * src1_stride);
    s2 = (const uint16_t *)(src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      r = RGB16_TO_R_FLOAT(*s1) - RGB16_TO_R_FLOAT(*s2);
      r *= r;
      dst[0] += r;
      
      g = RGB16_TO_G_FLOAT(*s1) - RGB16_TO_G_FLOAT(*s2);
      g *= g;
      dst[1] += g;

      b = RGB16_TO_B_FLOAT(*s1) - RGB16_TO_B_FLOAT(*s2);
      b *= b;
      dst[1] += b;

      s1++;
      s2++;
      }
    }
  }

static void psnr_rgb_15(double * dst,
                        const uint8_t * src1, int src1_stride,
                        const uint8_t * src2, int src2_stride,
                        int w, int h)
  {
  int i, j;
  const uint16_t *s1;
  const uint16_t *s2;

  double r = 0.0, g = 0.0, b = 0.0;

  dst[0] = 0.0;
  dst[1] = 0.0;
  dst[2] = 0.0;
  
  for(i = 0; i < h; i++)
    {
    s1 = (const uint16_t *)(src1 + i * src1_stride);
    s2 = (const uint16_t *)(src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      r = RGB15_TO_R_FLOAT(*s1) - RGB15_TO_R_FLOAT(*s2);
      r *= r;
      dst[0] += r;
      
      g = RGB15_TO_G_FLOAT(*s1) - RGB15_TO_G_FLOAT(*s2);
      g *= g;
      dst[1] += g;

      b = RGB15_TO_B_FLOAT(*s1) - RGB15_TO_B_FLOAT(*s2);
      b *= b;
      dst[2] += b;
      s1++;
      s2++;
      }
    }
  }

static double psnr_8(const uint8_t * src1, int src1_stride,
                     const uint8_t * src2, int src2_stride,
                     int w, int h, int advance)
  {
  int i, j;
  const uint8_t *s1;
  const uint8_t *s2;
  double ret = 0;
  double diff;
  for(i = 0; i < h; i++)
    {
    s1 = (src1 + i * src1_stride);
    s2 = (src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      diff = RGB_8_TO_FLOAT(*s1) - RGB_8_TO_FLOAT(*s2);
      diff *= diff;
      ret += diff;
      s1+= advance;
      s2+= advance;
      }
    }
  ret /= (w * h);
  return ret;
  }

static double psnr_y_8(const uint8_t * src1, int src1_stride,
                       const uint8_t * src2, int src2_stride,
                       int w, int h, int advance)
  {
  int i, j;
  const uint8_t *s1;
  const uint8_t *s2;
  double ret = 0;
  double diff;
  for(i = 0; i < h; i++)
    {
    s1 = (src1 + i * src1_stride);
    s2 = (src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      diff = Y_8_TO_FLOAT(*s1) - Y_8_TO_FLOAT(*s2);
      diff *= diff;
      ret += diff;
      s1+= advance;
      s2+= advance;
      }
    }
  ret /= (w * h);
  return ret;
  }

static double psnr_uv_8(const uint8_t * src1, int src1_stride,
                        const uint8_t * src2, int src2_stride,
                        int w, int h, int advance)
  {
  int i, j;
  const uint8_t *s1;
  const uint8_t *s2;
  double ret = 0;
  double diff;
  for(i = 0; i < h; i++)
    {
    s1 = (src1 + i * src1_stride);
    s2 = (src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      diff = UV_8_TO_FLOAT(*s1) - UV_8_TO_FLOAT(*s2);
      diff *= diff;
      ret += diff;
      s1+= advance;
      s2+= advance;
      }
    }
  ret /= (w * h);
  return ret;
  }

static double psnr_16(const uint8_t * src1, int src1_stride,
                      const uint8_t * src2, int src2_stride,
                      int w, int h, int advance)
  {
  int i, j;
  const uint16_t *s1;
  const uint16_t *s2;
  double ret = 0;
  double diff;
  for(i = 0; i < h; i++)
    {
    s1 = (const uint16_t *)(src1 + i * src1_stride);
    s2 = (const uint16_t *)(src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      diff = RGB_16_TO_FLOAT(*s1) - RGB_16_TO_FLOAT(*s2);
      diff *= diff;
      ret += diff;
      s1+= advance;
      s2+= advance;
      }
    }
  ret /= (w * h);
  return ret;
  }

static double psnr_y_16(const uint8_t * src1, int src1_stride,
                        const uint8_t * src2, int src2_stride,
                        int w, int h, int advance)
  {
  int i, j;
  const uint16_t *s1;
  const uint16_t *s2;
  double ret = 0;
  double d;
  for(i = 0; i < h; i++)
    {
    s1 = (const uint16_t *)(src1 + i * src1_stride);
    s2 = (const uint16_t *)(src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      d = Y_16_TO_Y_FLOAT(*s1) - Y_16_TO_Y_FLOAT(*s2);
      d *= d;
      ret += d;
      s1+= advance;
      s2+= advance;
      }
    }
  ret /= (w * h);
  return ret;
  }

static double psnr_uv_16(const uint8_t * src1, int src1_stride,
                        const uint8_t * src2, int src2_stride,
                        int w, int h, int advance)
  {
  int i, j;
  const uint16_t *s1;
  const uint16_t *s2;
  double ret = 0;
  double d;
  for(i = 0; i < h; i++)
    {
    s1 = (const uint16_t *)(src1 + i * src1_stride);
    s2 = (const uint16_t *)(src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      d = UV_16_TO_UV_FLOAT(*s1) - UV_16_TO_UV_FLOAT(*s2);
      d *= d;
      ret += d;
      s1+= advance;
      s2+= advance;
      }
    }
  ret /= (w * h);
  return ret;
  }

static double psnr_float(const uint8_t * src1, int src1_stride,
                         const uint8_t * src2, int src2_stride,
                         int w, int h, int advance)
  {
  int i, j;
  const float *s1;
  const float *s2;
  double ret = 0;
  double diff;
  for(i = 0; i < h; i++)
    {
    s1 = (const float *)(src1 + i * src1_stride);
    s2 = (const float *)(src2 + i * src2_stride);
    
    for(j = 0; j < w; j++)
      {
      diff = (*s1) - (*s2);
      diff *= diff;
      ret += diff;
      s1+= advance;
      s2+= advance;
      }
    }
  ret /= (w * h);
  return ret;
  }

void gavl_video_frame_psnr(double * psnr,
                           const gavl_video_frame_t * src1,
                           const gavl_video_frame_t * src2,
                           const gavl_video_format_t * format)
  {
  int sub_h, sub_v;
  int swap_rgb = 0;
  int num_components;
  int i;
  
  switch(format->pixelformat)
    {
    case GAVL_BGR_15:
      swap_rgb = 1;
      // Fall through
    case GAVL_RGB_15:
      psnr_rgb_15(psnr,
                  src1->planes[0], src1->strides[0],
                  src2->planes[0], src2->strides[0],
                  format->image_width, format->image_height);
      break;
    case GAVL_BGR_16:
      swap_rgb = 1;
      // Fall through
    case GAVL_RGB_16:
      psnr_rgb_16(psnr,
                  src1->planes[0], src1->strides[0],
                  src2->planes[0], src2->strides[0],
                  format->image_width, format->image_height);
      break;
    case GAVL_GRAY_8:
      psnr[0] = psnr_8(src1->planes[0], src1->strides[0],
                      src2->planes[0], src2->strides[0],
                      format->image_width, format->image_height, 1);
      break;
    case GAVL_GRAY_16:
      psnr[0] = psnr_8(src1->planes[0], src1->strides[0],
                      src2->planes[0], src2->strides[0],
                      format->image_width, format->image_height, 2);
      psnr[1] = psnr_8(src1->planes[0]+1, src1->strides[0],
                      src2->planes[0]+1, src2->strides[0],
                      format->image_width, format->image_height, 2);
      break;
    case GAVL_GRAY_FLOAT:
      psnr[0] = psnr_float(src1->planes[0], src1->strides[0],
                          src2->planes[0], src2->strides[0],
                          format->image_width, format->image_height, 1);
      break;
    case GAVL_GRAYA_16:
      psnr[0] = psnr_8(src1->planes[0], src1->strides[0],
                      src2->planes[0], src2->strides[0],
                      format->image_width, format->image_height, 2);
      psnr[1] = psnr_8(src1->planes[0]+1, src1->strides[0],
                      src2->planes[0]+1, src2->strides[0],
                      format->image_width, format->image_height, 2);
      break;
    case GAVL_YUY2:
      /* Y */
      psnr[0] = psnr_y_8(src1->planes[0], src1->strides[0],
                        src2->planes[0], src2->strides[0],
                        format->image_width, format->image_height, 2);
      /* U */
      psnr[1] = psnr_uv_8(src1->planes[0]+1, src1->strides[0],
                         src2->planes[0], src2->strides[0],
                         format->image_width, format->image_height, 4);
      /* V */
      psnr[2] = psnr_uv_8(src1->planes[0]+3, src1->strides[0],
                         src2->planes[0], src2->strides[0],
                         format->image_width, format->image_height, 4);
      break;
    case GAVL_UYVY:
      /* Y */
      psnr[0] = psnr_y_8(src1->planes[0]+1, src1->strides[0],
                        src2->planes[0], src2->strides[0],
                        format->image_width, format->image_height, 2);
      /* U */
      psnr[1] = psnr_uv_8(src1->planes[0], src1->strides[0],
                         src2->planes[0], src2->strides[0],
                         format->image_width, format->image_height, 4);
      /* V */
      psnr[2] = psnr_uv_8(src1->planes[0]+2, src1->strides[0],
                         src2->planes[0], src2->strides[0],
                         format->image_width, format->image_height, 4);
      break;
    case GAVL_GRAYA_32:
      psnr[0] = psnr_16(src1->planes[0], src1->strides[0],
                      src2->planes[0], src2->strides[0],
                      format->image_width, format->image_height, 2);
      psnr[1] = psnr_16(src1->planes[0]+2, src1->strides[0],
                      src2->planes[0]+2, src2->strides[0],
                      format->image_width, format->image_height, 2);
      break;
    case GAVL_GRAYA_FLOAT:
      psnr[0] = psnr_float(src1->planes[0], src1->strides[0],
                          src2->planes[0], src2->strides[0],
                          format->image_width, format->image_height, 2);
      psnr[1] = psnr_float(src1->planes[0]+sizeof(float), src1->strides[0],
                          src2->planes[0]+sizeof(float), src2->strides[0],
                          format->image_width, format->image_height, 2);
      break;
    case GAVL_BGR_24:
      swap_rgb = 1;
      // Fall through
    case GAVL_RGB_24:
      psnr[0] = psnr_8(src1->planes[0], src1->strides[0],
                      src2->planes[0], src2->strides[0],
                      format->image_width, format->image_height, 3);
      psnr[1] = psnr_8(src1->planes[0]+1, src1->strides[0],
                      src2->planes[0]+1, src2->strides[0],
                      format->image_width, format->image_height, 3);
      psnr[2] = psnr_8(src1->planes[0]+2, src1->strides[0],
                      src2->planes[0]+2, src2->strides[0],
                      format->image_width, format->image_height, 3);
      break;
    case GAVL_BGR_32:
      swap_rgb = 1;
      // Fall through
    case GAVL_RGB_32:
      psnr[0] = psnr_8(src1->planes[0], src1->strides[0],
                      src2->planes[0], src2->strides[0],
                      format->image_width, format->image_height, 4);
      psnr[1] = psnr_8(src1->planes[0]+1, src1->strides[0],
                      src2->planes[0]+1, src2->strides[0],
                      format->image_width, format->image_height, 4);
      psnr[2] = psnr_8(src1->planes[0]+2, src1->strides[0],
                      src2->planes[0]+2, src2->strides[0],
                      format->image_width, format->image_height, 4);
      break;
    case GAVL_RGBA_32:
      psnr[0] = psnr_8(src1->planes[0], src1->strides[0],
                      src2->planes[0], src2->strides[0],
                      format->image_width, format->image_height, 4);
      psnr[1] = psnr_8(src1->planes[0]+1, src1->strides[0],
                      src2->planes[0]+1, src2->strides[0],
                      format->image_width, format->image_height, 4);
      psnr[2] = psnr_8(src1->planes[0]+2, src1->strides[0],
                      src2->planes[0]+2, src2->strides[0],
                      format->image_width, format->image_height, 4);
      psnr[3] = psnr_8(src1->planes[0]+3, src1->strides[0],
                      src2->planes[0]+3, src2->strides[0],
                      format->image_width, format->image_height, 4);
      break;
    case GAVL_YUVA_32:
      psnr[0] = psnr_y_8(src1->planes[0], src1->strides[0],
                      src2->planes[0], src2->strides[0],
                      format->image_width, format->image_height, 4);
      psnr[1] = psnr_uv_8(src1->planes[0]+1, src1->strides[0],
                         src2->planes[0]+1, src2->strides[0],
                         format->image_width, format->image_height, 4);
      psnr[2] = psnr_uv_8(src1->planes[0]+2, src1->strides[0],
                         src2->planes[0]+2, src2->strides[0],
                         format->image_width, format->image_height, 4);
      psnr[3] = psnr_8(src1->planes[0]+3, src1->strides[0],
                      src2->planes[0]+3, src2->strides[0],
                      format->image_width, format->image_height, 4);
      break;
    case GAVL_RGB_48:
      psnr[0] = psnr_16(src1->planes[0], src1->strides[0],
                       src2->planes[0], src2->strides[0],
                       format->image_width, format->image_height, 3);
      psnr[1] = psnr_16(src1->planes[0]+2, src1->strides[0],
                       src2->planes[0]+2, src2->strides[0],
                       format->image_width, format->image_height, 3);
      psnr[2] = psnr_16(src1->planes[0]+4, src1->strides[0],
                       src2->planes[0]+4, src2->strides[0],
                       format->image_width, format->image_height, 3);
      break;
    case GAVL_RGBA_64:
      psnr[0] = psnr_16(src1->planes[0], src1->strides[0],
                       src2->planes[0], src2->strides[0],
                       format->image_width, format->image_height, 4);
      psnr[1] = psnr_16(src1->planes[0]+2, src1->strides[0],
                       src2->planes[0]+2, src2->strides[0],
                       format->image_width, format->image_height, 4);
      psnr[2] = psnr_16(src1->planes[0]+4, src1->strides[0],
                       src2->planes[0]+4, src2->strides[0],
                       format->image_width, format->image_height, 4);
      psnr[3] = psnr_16(src1->planes[0]+6, src1->strides[0],
                       src2->planes[0]+6, src2->strides[0],
                       format->image_width, format->image_height, 4);
      break;
    case GAVL_YUVA_64:
      psnr[0] = psnr_y_16(src1->planes[0], src1->strides[0],
                         src2->planes[0], src2->strides[0],
                         format->image_width, format->image_height, 4);
      psnr[1] = psnr_uv_16(src1->planes[0]+2, src1->strides[0],
                          src2->planes[0]+2, src2->strides[0],
                          format->image_width, format->image_height, 4);
      psnr[2] = psnr_uv_16(src1->planes[0]+4, src1->strides[0],
                          src2->planes[0]+4, src2->strides[0],
                          format->image_width, format->image_height, 4);
      psnr[3] = psnr_16(src1->planes[0]+6, src1->strides[0],
                       src2->planes[0]+6, src2->strides[0],
                       format->image_width, format->image_height, 4);
      break;
    case GAVL_RGB_FLOAT:
    case GAVL_YUV_FLOAT:
      psnr[0] = psnr_float(src1->planes[0], src1->strides[0],
                       src2->planes[0], src2->strides[0],
                       format->image_width, format->image_height, 3);
      psnr[1] = psnr_float(src1->planes[0]+sizeof(float), src1->strides[0],
                       src2->planes[0]+sizeof(float), src2->strides[0],
                       format->image_width, format->image_height, 3);
      psnr[2] = psnr_float(src1->planes[0]+2*sizeof(float), src1->strides[0],
                       src2->planes[0]+2*sizeof(float), src2->strides[0],
                       format->image_width, format->image_height, 3);
      break;
    case GAVL_RGBA_FLOAT:
    case GAVL_YUVA_FLOAT:
      psnr[0] = psnr_float(src1->planes[0], src1->strides[0],
                          src2->planes[0], src2->strides[0],
                          format->image_width, format->image_height, 4);
      psnr[1] = psnr_float(src1->planes[0]+sizeof(float), src1->strides[0],
                       src2->planes[0]+2, src2->strides[0],
                       format->image_width, format->image_height, 4);
      psnr[2] = psnr_float(src1->planes[0]+2*sizeof(float), src1->strides[0],
                       src2->planes[0]+4, src2->strides[0],
                       format->image_width, format->image_height, 4);
      psnr[4] = psnr_float(src1->planes[0]+3*sizeof(float), src1->strides[0],
                       src2->planes[0]+4, src2->strides[0],
                       format->image_width, format->image_height, 4);
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUV_410_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_444_P:
      gavl_pixelformat_chroma_sub(format->pixelformat,
                                  &sub_h, &sub_v);
      psnr[0] = psnr_y_8(src1->planes[0], src1->strides[0],
                        src2->planes[0], src2->strides[0],
                        format->image_width, format->image_height, 1);
      psnr[1] = psnr_uv_8(src1->planes[1], src1->strides[1],
                        src2->planes[1], src2->strides[1],
                        format->image_width/sub_h, format->image_height/sub_v, 1);
      psnr[2] = psnr_uv_8(src1->planes[2], src1->strides[2],
                         src2->planes[2], src2->strides[2],
                         format->image_width/sub_h, format->image_height/sub_v, 1);
      break;
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
      gavl_pixelformat_chroma_sub(format->pixelformat,
                                  &sub_h, &sub_v);

      psnr[0] = psnr_8(src1->planes[0], src1->strides[0],
                     src2->planes[0], src2->strides[0],
                     format->image_width, format->image_height, 1);
      psnr[1] = psnr_8(src1->planes[1], src1->strides[1],
                     src2->planes[1], src2->strides[1],
                     format->image_width/sub_h, format->image_height/sub_v, 1);
      psnr[2] = psnr_8(src1->planes[2], src1->strides[2],
                     src2->planes[2], src2->strides[2],
                     format->image_width/sub_h, format->image_height/sub_v, 1);
      break;
    case GAVL_YUV_444_P_16:
    case GAVL_YUV_422_P_16:
      gavl_pixelformat_chroma_sub(format->pixelformat,
                                  &sub_h, &sub_v);
      psnr[0] = psnr_y_16(src1->planes[0], src1->strides[0],
                         src2->planes[0], src2->strides[0],
                         format->image_width, format->image_height, 1);
      
      psnr[1] = psnr_uv_16(src1->planes[1], src1->strides[1],
                          src2->planes[1], src2->strides[1],
                          format->image_width/sub_h, format->image_height/sub_v, 1);
      
      psnr[2] = psnr_uv_16(src1->planes[2], src1->strides[2],
                          src2->planes[2], src2->strides[2],
                          format->image_width/sub_h, format->image_height/sub_v, 1);
      break;
    case GAVL_PIXELFORMAT_NONE:
      break;
    }

  if(swap_rgb)
    {
    double swp = psnr[0];
    psnr[0] = psnr[2];
    psnr[2] = swp;
    }

  if(gavl_pixelformat_is_gray(format->pixelformat))
    num_components = 1;
  else
    num_components = 3;
  
  if(gavl_pixelformat_has_alpha(format->pixelformat))
    num_components++;
  
  for(i = 0; i < num_components; i++)
    psnr[i] = 10 * log10(1/psnr[i]);
  
  }

