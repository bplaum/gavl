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

#include <video.h>
#include <config.h>
#include <accel.h>
#include <hw_private.h>


#define DO_ROUND
#include "c/colorspace_tables.h"
#include "c/colorspace_macros.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memalign.h>

#define ALIGNMENT_BYTES 16
#define ALIGN(a) a=((a+ALIGNMENT_BYTES-1)/ALIGNMENT_BYTES)*ALIGNMENT_BYTES

static void video_frame_alloc(gavl_video_frame_t * ret,
                              const gavl_video_format_t * format, int align)
  {
  int planar;

  if(format->pixelformat == GAVL_PIXELFORMAT_NONE)
    {
    fprintf(stderr, "Pixelformat not specified for video frame\n");
    return;
    }
  planar = gavl_pixelformat_is_planar(format->pixelformat);
  
  if(planar)
    {
    int sub_h;
    int sub_v;
    int bpc;
    
    gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
    
    bpc = gavl_pixelformat_bytes_per_component(format->pixelformat);

    if(!ret->strides[0])
      {
      ret->strides[0] = bpc * format->frame_width;
      ret->strides[1] = bpc * ((format->frame_width + sub_h - 1) / sub_h);
      ret->strides[2] = ret->strides[1];
      
      if(align)
        {
        ALIGN(ret->strides[0]);
        ALIGN(ret->strides[1]);
        ALIGN(ret->strides[2]);
        }
      }
    
    ret->planes[0] = gavl_memalign(ALIGNMENT_BYTES,
                              ret->strides[0]*format->frame_height+
                              ret->strides[1]*((format->frame_height+sub_v-1)/sub_v)+
                              ret->strides[2]*((format->frame_height+sub_v-1)/sub_v));
    ret->planes[1] = ret->planes[0] + ret->strides[0]*format->frame_height;
    ret->planes[2] = ret->planes[1] + ret->strides[1]*((format->frame_height+sub_v-1)/sub_v);
    }
  else // Packed
    {
    if(!ret->strides[0])
      {
      ret->strides[0] =
        format->frame_width * gavl_pixelformat_bytes_per_pixel(format->pixelformat);
      if(align)
        ALIGN(ret->strides[0]);
      }
    ret->planes[0] = gavl_memalign(ALIGNMENT_BYTES,
                              ret->strides[0] * format->frame_height);
    }
  }

static void video_frame_free(gavl_video_frame_t * frame)
  {
  if(frame->planes[0])
    free(frame->planes[0]);
  frame->planes[0] = NULL;
  }

gavl_video_frame_t * gavl_video_frame_create(const gavl_video_format_t * format)
  {
  gavl_video_frame_t * ret = calloc(1, sizeof(gavl_video_frame_t));
  if(format)
    video_frame_alloc(ret, format, 1);
  ret->timecode = GAVL_TIMECODE_INVALID_MASK;
  return ret;
  }

gavl_video_frame_t * gavl_video_frame_create_nopad(const gavl_video_format_t * format)
  {
  gavl_video_frame_t * ret = calloc(1, sizeof(gavl_video_frame_t));
  if(format)
    video_frame_alloc(ret, format, 0);
  ret->timecode = GAVL_TIMECODE_INVALID_MASK;
  return ret;
  }


void gavl_video_frame_destroy(gavl_video_frame_t * frame)
  {
  if(frame->hwctx)
    {
    gavl_hw_destroy_video_frame(frame->hwctx, frame);
    return;
    }
  
  video_frame_free(frame);
  free(frame);
  }

void gavl_video_frame_null(gavl_video_frame_t* frame)
  {
  frame->planes[0] = NULL;
  }

void gavl_video_frame_clear_mask(gavl_video_frame_t * frame,
                                 const gavl_video_format_t * format, int mask)
  {
  int i, j;
  uint16_t * ptr_16;
  uint16_t * ptr_16_u;
  uint16_t * ptr_16_v;
  float * ptr_float;
  uint8_t * line_start;
  uint8_t * line_start_u;
  uint8_t * line_start_v;
  int bytes;
  
  switch(format->pixelformat)
    {
    case GAVL_RGB_15:
    case GAVL_BGR_15:
    case GAVL_RGB_16:
    case GAVL_BGR_16:
    case GAVL_RGB_24:
    case GAVL_BGR_24:
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_RGB_48:
    case GAVL_RGB_FLOAT:
    case GAVL_GRAY_8:
    case GAVL_GRAY_16:
    case GAVL_GRAY_FLOAT:
    case GAVL_YUV_FLOAT:
      if(mask & CLEAR_MASK_PLANE_0)
        {
        bytes = format->frame_width * gavl_pixelformat_bytes_per_pixel(format->pixelformat);
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[0] + i * frame->strides[0], 0x00, bytes);
        }
      break;
    case GAVL_RGBA_32:
      if(!(mask & CLEAR_MASK_PLANE_0))
        break;
      for(i = 0; i < format->frame_height; i++)
        {
        for(j = 0; j < format->frame_width; j++)
          {
          frame->planes[0][4*j + i*frame->strides[0]]   = 0x00; /* R */
          frame->planes[0][4*j + i*frame->strides[0]+1] = 0x00; /* G */
          frame->planes[0][4*j + i*frame->strides[0]+2] = 0x00; /* B */
          frame->planes[0][4*j + i*frame->strides[0]+3] = 0xFF; /* A */
          }
        }
      break;
    case GAVL_GRAYA_16:
      if(!(mask & CLEAR_MASK_PLANE_0))
        break;
      for(i = 0; i < format->frame_height; i++)
        {
        for(j = 0; j < format->frame_width; j++)
          {
          frame->planes[0][2*j + i*frame->strides[0]]   = 0x00; /* Gray */
          frame->planes[0][2*j + i*frame->strides[0]+1] = 0xFF; /* A    */
          }
        }
      break;
    case GAVL_YUVA_32:
      if(!(mask & CLEAR_MASK_PLANE_0))
        break;
      for(i = 0; i < format->frame_height; i++)
        {
        for(j = 0; j < format->frame_width; j++)
          {
          frame->planes[0][4*j + i*frame->strides[0]]   = 0x00; /* Y */
          frame->planes[0][4*j + i*frame->strides[0]+1] = 0x80; /* U */
          frame->planes[0][4*j + i*frame->strides[0]+2] = 0x80; /* V */
          frame->planes[0][4*j + i*frame->strides[0]+3] = 0xEB; /* A */
          }
        }
      break;
    case GAVL_RGBA_64:
      if(!(mask & CLEAR_MASK_PLANE_0))
        break;
      
      line_start = frame->planes[0];
      
      for(i = 0; i < format->frame_height; i++)
        {
        ptr_16 = (uint16_t*)line_start;
        for(j = 0; j < format->frame_width; j++)
          {
          *(ptr_16++) = 0x0000;
          *(ptr_16++) = 0x0000;
          *(ptr_16++) = 0x0000;
          *(ptr_16++) = 0xFFFF;
          }
        line_start += frame->strides[0];
        }
      
      break;
    case GAVL_GRAYA_32:
      if(!(mask & CLEAR_MASK_PLANE_0))
        break;
      
      line_start = frame->planes[0];
      
      for(i = 0; i < format->frame_height; i++)
        {
        ptr_16 = (uint16_t*)line_start;
        for(j = 0; j < format->frame_width; j++)
          {
          *(ptr_16++) = 0x0000;
          *(ptr_16++) = 0xFFFF;
          }
        line_start += frame->strides[0];
        }
      
      break;
    case GAVL_YUVA_64:
      if(!(mask & CLEAR_MASK_PLANE_0))
        break;
      
      line_start = frame->planes[0];
      
      for(i = 0; i < format->frame_height; i++)
        {
        ptr_16 = (uint16_t*)line_start;
        for(j = 0; j < format->frame_width; j++)
          {
          *(ptr_16++) = 0x0000;
          *(ptr_16++) = 0x8000;
          *(ptr_16++) = 0x8000;
          *(ptr_16++) = 0xFFFF;
          }
        line_start += frame->strides[0];
        }
      
      break;
    case GAVL_RGBA_FLOAT:
    case GAVL_YUVA_FLOAT:
      if(!(mask & CLEAR_MASK_PLANE_0))
        break;
      line_start = frame->planes[0];
      
      for(i = 0; i < format->frame_height; i++)
        {
        ptr_float = (float*)line_start;
        for(j = 0; j < format->frame_width; j++)
          {
          *(ptr_float++) = 0.0;
          *(ptr_float++) = 0.0;
          *(ptr_float++) = 0.0;
          *(ptr_float++) = 1.0;
          }
        line_start += frame->strides[0];
        }
      break;
    case GAVL_GRAYA_FLOAT:
      if(!(mask & CLEAR_MASK_PLANE_0))
        break;
      line_start = frame->planes[0];
      
      for(i = 0; i < format->frame_height; i++)
        {
        ptr_float = (float*)line_start;
        for(j = 0; j < format->frame_width; j++)
          {
          *(ptr_float++) = 0.0;
          *(ptr_float++) = 1.0;
          }
        line_start += frame->strides[0];
        }
      break;
    case GAVL_YUY2:
      if(!(mask & CLEAR_MASK_PLANE_0))
        break;
      for(i = 0; i < format->frame_height; i++)
        {
        for(j = 0; j < format->frame_width; j++)
          {
          frame->planes[0][2*j + i*frame->strides[0]]   = 0x00; /* Y   */
          frame->planes[0][2*j + i*frame->strides[0]+1] = 0x80; /* U/V */
          }
        }
      break;
    case GAVL_UYVY:
      if(!(mask & CLEAR_MASK_PLANE_0))
        break;
      for(i = 0; i < format->frame_height; i++)
        {
        for(j = 0; j < format->frame_width; j++)
          {
          frame->planes[0][2*j + i*frame->strides[0]+1] = 0x00; /* Y */
          frame->planes[0][2*j + i*frame->strides[0]]   = 0x80; /* U/V   */
          }
        }
      break;
    case GAVL_YUV_444_P_16:
      if(mask & CLEAR_MASK_PLANE_0)
        {
        bytes = format->frame_width * 2;
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[0] + i * frame->strides[0], 0x00, bytes);
        }
      
      if(!(mask & (CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2)))
        break;
      
      line_start_u = frame->planes[1];
      line_start_v = frame->planes[2];
      for(i = 0; i < format->frame_height; i++)
        {
        ptr_16_u = (uint16_t*)line_start_u;
        ptr_16_v = (uint16_t*)line_start_v;

        if(mask & CLEAR_MASK_PLANE_1)
          {
          for(j = 0; j < format->frame_width; j++)
            {
            *(ptr_16_u++) = 0x8000;
            }
          }
        
        if(mask & CLEAR_MASK_PLANE_2)
          {
          for(j = 0; j < format->frame_width; j++)
            {
            *(ptr_16_v++) = 0x8000;
            }
          }
        
        line_start_u += frame->strides[1];
        line_start_v += frame->strides[2];
        }
      break;
    case GAVL_YUV_422_P_16:
      if(mask & CLEAR_MASK_PLANE_0)
        {
        bytes = format->frame_width * 2;
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[0] + i * frame->strides[0], 0x00, bytes);
        }
      
      if(!(mask & (CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2)))
        break;
      
      line_start_u = frame->planes[1];
      line_start_v = frame->planes[2];
      for(i = 0; i < format->frame_height; i++)
        {
        ptr_16_u = (uint16_t*)line_start_u;
        ptr_16_v = (uint16_t*)line_start_v;

        if(mask & CLEAR_MASK_PLANE_1)
          {
          for(j = 0; j < format->frame_width/2; j++)
            {
            *(ptr_16_u++) = 0x8000;
            }
          }
        if(mask & CLEAR_MASK_PLANE_2)
          {
          for(j = 0; j < format->frame_width/2; j++)
            {
            *(ptr_16_v++) = 0x8000;
            }
          }
        
        line_start_u += frame->strides[1];
        line_start_v += frame->strides[2];
        }
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUVJ_420_P:
      if(mask & CLEAR_MASK_PLANE_0)
        {
        bytes = format->frame_width;
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[0] + i * frame->strides[0], 0x00, bytes);
        }
      if(mask & CLEAR_MASK_PLANE_1)
        {
        bytes = format->frame_width / 2;
        for(i = 0; i < format->frame_height / 2; i++)
          memset(frame->planes[1] + i * frame->strides[1], 0x80, bytes);
        }
      if(mask & CLEAR_MASK_PLANE_2)
        {
        bytes = format->frame_width / 2;
        for(i = 0; i < format->frame_height / 2; i++)
          memset(frame->planes[2] + i * frame->strides[2], 0x80, bytes);
        }
      break;
    case GAVL_YUV_410_P:
      if(mask & CLEAR_MASK_PLANE_0)
        {
        bytes = format->frame_width;
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[0] + i * frame->strides[0], 0x00, bytes);
        }
      if(mask & CLEAR_MASK_PLANE_1)
        {
        bytes = format->frame_width / 4;
        for(i = 0; i < format->frame_height / 4; i++)
          memset(frame->planes[1] + i * frame->strides[1], 0x80, bytes);
        }
      if(mask & CLEAR_MASK_PLANE_2)
        {
        bytes = format->frame_width / 4;
        for(i = 0; i < format->frame_height / 4; i++)
          memset(frame->planes[2] + i * frame->strides[2], 0x80, bytes);
        }
      break;
    case GAVL_YUV_411_P:
      if(mask & CLEAR_MASK_PLANE_0)
        {
        bytes = format->frame_width;
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[0] + i * frame->strides[0], 0x00, bytes);
        }
      if(mask & CLEAR_MASK_PLANE_1)
        {
        bytes = format->frame_width / 4;
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[1] + i * frame->strides[1], 0x80, bytes);
        }
      if(mask & CLEAR_MASK_PLANE_2)
        {
        bytes = format->frame_width / 4;
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[2] + i * frame->strides[2], 0x80, bytes);
        }
      break;
    case GAVL_YUV_422_P:
    case GAVL_YUVJ_422_P:
      if(mask & CLEAR_MASK_PLANE_0)
        {
        bytes = format->frame_width;
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[0] + i * frame->strides[0], 0x00, bytes);
        }
      if(mask & CLEAR_MASK_PLANE_1)
        {
        bytes = format->frame_width / 2;
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[1] + i * frame->strides[1], 0x80, bytes);
        }
      if(mask & CLEAR_MASK_PLANE_2)
        {
        bytes = format->frame_width / 2;
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[2] + i * frame->strides[2], 0x80, bytes);
        }
      break;
    case GAVL_YUV_444_P:
    case GAVL_YUVJ_444_P:
      if(mask & CLEAR_MASK_PLANE_0)
        {
        bytes = format->frame_width;
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[0] + i * frame->strides[0], 0x00, bytes);
        }
      if(mask & CLEAR_MASK_PLANE_1)
        {
        bytes = format->frame_width;
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[1] + i * frame->strides[1], 0x80, bytes);
        }
      if(mask & CLEAR_MASK_PLANE_2)
        {
        bytes = format->frame_width;
        for(i = 0; i < format->frame_height; i++)
          memset(frame->planes[2] + i * frame->strides[2], 0x80, bytes);
        }
      break;
    case GAVL_PIXELFORMAT_NONE:
      break;
    }
  }


void gavl_video_frame_clear(gavl_video_frame_t * frame,
                            const gavl_video_format_t * format)
  {
  gavl_video_frame_clear_mask(frame, format, CLEAR_MASK_ALL);  
  }

static void copy_plane(gavl_video_frame_t * dst,
                       const gavl_video_frame_t * src, int plane,
                       int bytes_per_line, int height)
  {
  int j;
  uint8_t * sp, *dp;
  sp = src->planes[plane];
  dp = dst->planes[plane];

  if((src->strides[plane] == dst->strides[plane]) && 
     (src->strides[plane] == bytes_per_line))
    gavl_memcpy(dp, sp, bytes_per_line * height);
  else
    {
    for(j = 0; j < height; j++)
      {
      gavl_memcpy(dp, sp, bytes_per_line);
      sp += src->strides[plane];
      dp += dst->strides[plane];
      }
    }
  }

void gavl_video_frame_copy_plane(const gavl_video_format_t * format,
                                 gavl_video_frame_t * dst,
                                 const gavl_video_frame_t * src, int plane)
  {
  int bytes_per_line;
  int sub_h, sub_v;
  int height;

  if(src->src_rect.w && src->src_rect.h)
    {
    gavl_video_format_t fmt;
    gavl_video_frame_t s;
    gavl_video_frame_t d;

    gavl_video_format_copy(&fmt, format);
    fmt.image_width  = src->src_rect.w;
    fmt.image_height = src->src_rect.h;
    
    memset(&s, 0, sizeof(s));
    memset(&d, 0, sizeof(d));
    gavl_video_frame_get_subframe(format->pixelformat, dst, &d, &src->src_rect);
    gavl_video_frame_get_subframe(format->pixelformat, src, &s, &src->src_rect);
    gavl_video_frame_copy_plane(&fmt, &d, &s, plane);
    }

  gavl_init_memcpy();
  height = format->image_height;
  sub_h = 1;
  sub_v = 1;
  
  bytes_per_line = gavl_pixelformat_is_planar(format->pixelformat) ?
    format->image_width * gavl_pixelformat_bytes_per_component(format->pixelformat) :
    format->image_width * gavl_pixelformat_bytes_per_pixel(format->pixelformat);
  
  if(plane > 0)
    {
    gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
    bytes_per_line /= sub_h;
    height /= sub_v;
    }
  copy_plane(dst, src, plane, bytes_per_line, height);
  }

void gavl_video_frame_copy(const gavl_video_format_t * format,
                           gavl_video_frame_t * dst,
                           const gavl_video_frame_t * src)
  {
  int i;
  int height;
  int bytes_per_line;
  int sub_h, sub_v;
  int planes;

  if(src->src_rect.w && src->src_rect.h)
    {
    gavl_video_format_t fmt;
    gavl_video_frame_t s;
    gavl_video_frame_t d;

    gavl_video_format_copy(&fmt, format);
    fmt.image_width  = src->src_rect.w;
    fmt.image_height = src->src_rect.h;
    
    memset(&s, 0, sizeof(s));
    memset(&d, 0, sizeof(d));
    gavl_video_frame_get_subframe(format->pixelformat, dst, &d, &src->src_rect);
    gavl_video_frame_get_subframe(format->pixelformat, src, &s, &src->src_rect);
    gavl_video_frame_copy(&fmt, &d, &s);
    }
  
  gavl_init_memcpy();
#if 0
  fprintf(stderr, "gavl_video_frame_copy, %d %d format:\n",
          src->strides[0], dst->strides[0]);
  gavl_video_format_dump(format);
#endif
  planes = gavl_pixelformat_num_planes(format->pixelformat);
  height = format->image_height;
    
  bytes_per_line = gavl_pixelformat_is_planar(format->pixelformat) ?
    format->image_width * gavl_pixelformat_bytes_per_component(format->pixelformat) :
    format->image_width * gavl_pixelformat_bytes_per_pixel(format->pixelformat);
  
  for(i = 0; i < planes; i++)
    {
    if(i == 1)
      {
      gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
      bytes_per_line /= sub_h;
      height /= sub_v;
      }
    copy_plane(dst, src, i, bytes_per_line, height);
    }
  
  }



static void flip_scanline_1(uint8_t * dst, uint8_t * src, int len)
  {
  int i;
  dst += (len-1);
  
  for(i = 0; i < len; i++)
    {
    *dst = *src;
    dst--;
    src++;
    }
  }

static void flip_scanline_2(uint8_t * dst, uint8_t * src, int len)
  {
  int i;
  dst += 2*(len-1);
  
  for(i = 0; i < len; i++)
    {
    dst[0] = src[0];
    dst[1] = src[1];
    
    dst-=2;
    src+=2;
    }
  
  }

static void flip_scanline_yuy2(uint8_t * dst, uint8_t * src, int len)
  {
  int i;
  dst += 2*(len-1)-2;
  
  for(i = 0; i < len/2; i++)
    {
    dst[0] = src[2]; /* Y */
    dst[1] = src[1]; /* U */
    dst[2] = src[0]; /* Y */
    dst[3] = src[3]; /* U */
    
    dst-=4;
    src+=4;
    }
  
  }

static void flip_scanline_uyvy(uint8_t * dst, uint8_t * src, int len)
  {
  int i;
  dst += 2*(len-1)-2;
  
  for(i = 0; i < len/2; i++)
    {
    dst[0] = src[0]; /* U */
    dst[1] = src[3]; /* Y */
    dst[2] = src[2]; /* V */
    dst[3] = src[1]; /* Y */
    
    dst-=4;
    src+=4;
    }
  
  }


static void flip_scanline_3(uint8_t * dst, uint8_t * src, int len)
  {
  int i;
  dst += 3*(len-1);
  
  for(i = 0; i < len; i++)
    {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    
    dst-=3;
    src+=3;
    }
  
  }

static void flip_scanline_4(uint8_t * dst, uint8_t * src, int len)
  {
  int i;
  dst += 4*(len-1);
  
  for(i = 0; i < len; i++)
    {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    
    dst-=4;
    src+=4;
    }
  
  }

static void flip_scanline_6(uint8_t * dst, uint8_t * src, int len)
  {
  int i;
  dst += 6*(len-1);
  
  for(i = 0; i < len; i++)
    {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    dst[4] = src[4];
    dst[5] = src[5];
    
    dst-=6;
    src+=6;
    }
  
  }

static void flip_scanline_8(uint8_t * dst, uint8_t * src, int len)
  {
  int i;
  dst += 8*(len-1);
  
  for(i = 0; i < len; i++)
    {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    dst[4] = src[4];
    dst[5] = src[5];
    dst[6] = src[6];
    dst[7] = src[7];
    
    dst-=8;
    src+=8;
    }
  
  }

static void flip_scanline_12(uint8_t * dst, uint8_t * src, int len)
  {
  int i;
  dst += 12*(len-1);
  
  for(i = 0; i < len; i++)
    {
    gavl_memcpy(dst, src, 12);
    
    dst-=12;
    src+=12;
    }
  
  }

static void flip_scanline_16(uint8_t * dst, uint8_t * src, int len)
  {
  int i;
  dst += 16*(len-1);
  
  for(i = 0; i < len; i++)
    {
    gavl_memcpy(dst, src, 16);
    
    dst-=16;
    src+=16;
    }
  }



typedef void (*flip_scanline_func)(uint8_t * dst, uint8_t * src, int len);

static flip_scanline_func find_flip_scanline_func(gavl_pixelformat_t csp)
  {
  switch(csp)
    {
    case GAVL_RGB_15:
    case GAVL_BGR_15:
    case GAVL_RGB_16:
    case GAVL_BGR_16:
    case GAVL_YUV_444_P_16:
    case GAVL_YUV_422_P_16:
    case GAVL_GRAYA_16:
    case GAVL_GRAY_16:
      return flip_scanline_2;
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
      return flip_scanline_3;
      break;
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_RGBA_32:
    case GAVL_YUVA_32:
    case GAVL_GRAYA_32:
    case GAVL_GRAY_FLOAT:
      return flip_scanline_4;
      break;
    case GAVL_RGB_48:
      return flip_scanline_6;
      break;
    case GAVL_RGBA_64:
    case GAVL_YUVA_64:
    case GAVL_GRAYA_FLOAT:
      return flip_scanline_8;
      break;
    case GAVL_RGB_FLOAT:
    case GAVL_YUV_FLOAT:
      return flip_scanline_12;
      break;
    case GAVL_RGBA_FLOAT:
    case GAVL_YUVA_FLOAT:
      return flip_scanline_16;
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUV_410_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_444_P:
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
    case GAVL_GRAY_8:
      return flip_scanline_1;
      break;
    case GAVL_YUY2:
      return flip_scanline_yuy2;
      break;
    case GAVL_UYVY:
      return flip_scanline_uyvy;
      break;
    case GAVL_PIXELFORMAT_NONE:
      break;
    }
  return NULL;
  }

void gavl_video_frame_copy_flip_x(const gavl_video_format_t * format,
                                  gavl_video_frame_t * dst,
                                  const gavl_video_frame_t * src)
  {
  uint8_t * src_ptr;
  uint8_t * dst_ptr;
  
  int i, j, jmax, width;
  int sub_h, sub_v;
  int planes;
  flip_scanline_func func;
  
  planes = gavl_pixelformat_num_planes(format->pixelformat);
  func = find_flip_scanline_func(format->pixelformat);
  
  sub_h = 1;
  sub_v = 1;

  jmax = format->image_height;
  width = format->image_width;

  gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
  
  for(i = 0; i < planes; i++)
    {
    src_ptr = src->planes[i];
    dst_ptr = dst->planes[i];
            
    for(j = 0; j < jmax; j++)
      {
      func(dst_ptr, src_ptr, width);
      
      src_ptr += src->strides[i];
      dst_ptr += dst->strides[i];
      }
    if(!i)
      {
      jmax /= sub_v;
      width /= sub_h;
      }
    }
  
  }

void gavl_video_frame_copy_flip_y(const gavl_video_format_t * format,
                                  gavl_video_frame_t * dst,
                                  const gavl_video_frame_t * src)
  {
  uint8_t * src_ptr;
  uint8_t * dst_ptr;
  
  int i, j;
  int sub_h, sub_v;
  int planes;
  int bytes_per_line;
  gavl_init_memcpy();
  planes = gavl_pixelformat_num_planes(format->pixelformat);
  
  sub_h = 1;
  sub_v = 1;
  
  for(i = 0; i < planes; i++)
    {
    if(i)
      gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
    
    src_ptr = src->planes[i] +
      ((format->image_height / sub_v) - 1) * src->strides[i];

    dst_ptr = dst->planes[i];

    bytes_per_line =
      dst->strides[i] < src->strides[i] ?
      dst->strides[i] : src->strides[i];
    
    for(j = 0; j < format->image_height / sub_v; j++)
      {
      gavl_memcpy(dst_ptr, src_ptr, bytes_per_line);
      
      src_ptr -= src->strides[i];
      dst_ptr += dst->strides[i];
      }
    }

  }

void gavl_video_frame_copy_flip_xy(const gavl_video_format_t * format,
                                   gavl_video_frame_t * dst,
                                   const gavl_video_frame_t * src)
  {
  uint8_t * src_ptr;
  uint8_t * dst_ptr;
  
  int i, j;
  int sub_h, sub_v;
  int planes;
  flip_scanline_func func;
  
  planes = gavl_pixelformat_num_planes(format->pixelformat);
  func = find_flip_scanline_func(format->pixelformat);
  
  sub_h = 1;
  sub_v = 1;
  
  for(i = 0; i < planes; i++)
    {
    if(i)
      gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);

    src_ptr = src->planes[i] +
      ((format->image_height / sub_v) - 1) * src->strides[i];

    dst_ptr = dst->planes[i];
            
    for(j = 0; j < format->image_height / sub_v; j++)
      {
      func(dst_ptr, src_ptr, format->image_width / sub_h);

      src_ptr -= src->strides[i];
      dst_ptr += dst->strides[i];
      }
    }
  
  }



void gavl_video_frame_dump(gavl_video_frame_t * frame,
                           const gavl_video_format_t * format,
                           const char * namebase)
  {
  char * filename;
  int baselen;
  int sub_h, sub_v;
  int planes;
  int i, j;
  
  FILE * output;

  planes = gavl_pixelformat_num_planes(format->pixelformat);
  
  baselen = strlen(namebase);
  
  filename = malloc(baselen + 4);
  strcpy(filename, namebase);

  sub_h = 1;
  sub_v = 1;
  
  for(i = 0; i < planes; i++)
    {
    filename[baselen]   = '.';
    filename[baselen+1] = 'p';
    filename[baselen+2] = i + 1 + '0';
    filename[baselen+3] = '\0';

    output = fopen(filename, "wb");

    if(i == 1)
      gavl_pixelformat_chroma_sub(format->pixelformat,
                          &sub_h, &sub_v);
    
    for(j = 0; j < format->image_height / sub_v; j++)
      {
      fwrite(frame->planes[i] + j*frame->strides[i], 1,  format->image_width / sub_h,
             output);
      }
    fclose(output);
    }
  free(filename);
  }

void gavl_video_frame_get_subframe(gavl_pixelformat_t pixelformat,
                                   const gavl_video_frame_t * src,
                                   gavl_video_frame_t * dst,
                                   const gavl_rectangle_i_t * src_rect)
  {
  int x;
  int uv_sub_h, uv_sub_v;
  int i;
  int bytes;
  int num_planes = gavl_pixelformat_num_planes(pixelformat);

  dst->strides[0] = src->strides[0];
  
  if(num_planes > 1)
    {
    gavl_pixelformat_chroma_sub(pixelformat, &uv_sub_h,
                               &uv_sub_v);
    bytes = gavl_pixelformat_bytes_per_component(pixelformat);
    dst->planes[0] = src->planes[0] + src_rect->y * src->strides[0] + src_rect->x * bytes;

    for(i = 1; i < num_planes; i++)
      {
      dst->planes[i] = src->planes[i] +
        (src_rect->y/uv_sub_v) * src->strides[i] + (src_rect->x/uv_sub_h) * bytes;
      dst->strides[i] = src->strides[i];
      }
    }
  else
    {
    x = src_rect->x;
    if(((pixelformat == GAVL_YUY2) || (pixelformat == GAVL_UYVY)) &&
       (x & 1))
      x--;
    bytes = gavl_pixelformat_bytes_per_pixel(pixelformat);
    dst->planes[0] = src->planes[0] + src_rect->y * src->strides[0] + x * bytes;
    }
  }

void gavl_video_frame_get_field(gavl_pixelformat_t pixelformat,
                                const gavl_video_frame_t * src,
                                gavl_video_frame_t * dst,
                                int field)
  {
  int i, num_planes;
  num_planes = gavl_pixelformat_num_planes(pixelformat);
  for(i = 0; i < num_planes; i++)
    {
    dst->planes[i] = src->planes[i] + field * src->strides[i];
    dst->strides[i] = src->strides[i] * 2;
    }
  }

#define FILL_FUNC_HEAD_PACKED(TYPE) \
  uint8_t * dst_start; \
  TYPE * dst; \
  int i, j; \
  dst_start = frame->planes[0]; \
  for(i = 0; i < format->image_height; i++) \
    { \
    dst = (TYPE*)dst_start; \
    for(j = 0; j < format->image_width; j++) \
      {

#define FILL_FUNC_HEAD_PACKED_422(TYPE) \
  uint8_t * dst_start;          \
  TYPE * dst;                   \
  int i, j, jmax;               \
  jmax = format->image_width / 2; \
  dst_start = frame->planes[0]; \
  for(i = 0; i < format->image_height; i++) \
    { \
    dst = (TYPE*)dst_start; \
    for(j = 0; j < jmax; j++) \
      {


#define FILL_FUNC_TAIL_PACKED(ADVANCE)             \
      dst+=ADVANCE;                                \
      }                                            \
    dst_start += frame->strides[0];                 \
    }

static void fill_8_packed(gavl_video_frame_t * frame,
                          const gavl_video_format_t * format,
                          uint8_t color)
  {
  FILL_FUNC_HEAD_PACKED(uint8_t);
  *dst = color;
  FILL_FUNC_TAIL_PACKED(1);
  }
      
static void fill_16_packed(gavl_video_frame_t * frame,
                           const gavl_video_format_t * format,
                           uint16_t color)
  {
  FILL_FUNC_HEAD_PACKED(uint16_t);
  *dst = color;
  FILL_FUNC_TAIL_PACKED(1);
  }

static void fill_24_packed(gavl_video_frame_t * frame,
                           const gavl_video_format_t * format,
                           const uint8_t * color)
  {
  FILL_FUNC_HEAD_PACKED(uint8_t);
  dst[0] = color[0];
  dst[1] = color[1];
  dst[2] = color[2];
  FILL_FUNC_TAIL_PACKED(3);
  }

static void fill_32_packed(gavl_video_frame_t * frame,
                           const gavl_video_format_t * format,
                           const uint8_t * _color)
  {
  uint32_t * color = (uint32_t*)_color;
  FILL_FUNC_HEAD_PACKED(uint32_t);
  *dst = *color;
  FILL_FUNC_TAIL_PACKED(1);
  }

static void fill_32_packed_422(gavl_video_frame_t * frame,
                               const gavl_video_format_t * format,
                               const uint8_t * _color)
  {
  uint32_t * color = (uint32_t*)_color;
  FILL_FUNC_HEAD_PACKED_422(uint32_t);
  *dst = *color;
  FILL_FUNC_TAIL_PACKED(1);
  }


static void fill_48_packed(gavl_video_frame_t * frame,
                           const gavl_video_format_t * format,
                           const uint16_t * color)
  {
  FILL_FUNC_HEAD_PACKED(uint16_t);
  gavl_memcpy(dst, color, 3*sizeof(*dst));
  FILL_FUNC_TAIL_PACKED(3);
  }

static void fill_64_packed(gavl_video_frame_t * frame,
                           const gavl_video_format_t * format,
                           const uint16_t * color)
  {
  FILL_FUNC_HEAD_PACKED(uint16_t);
  gavl_memcpy(dst, color, 4*sizeof(*dst));
  FILL_FUNC_TAIL_PACKED(4);
  }

static void fill_float_gray(gavl_video_frame_t * frame,
                           const gavl_video_format_t * format,
                           const float * color)
  {
  FILL_FUNC_HEAD_PACKED(float);
  gavl_memcpy(dst, color, sizeof(*dst));
  FILL_FUNC_TAIL_PACKED(1);
  }

static void fill_float_graya(gavl_video_frame_t * frame,
                           const gavl_video_format_t * format,
                           const float * color)
  {
  FILL_FUNC_HEAD_PACKED(float);
  gavl_memcpy(dst, color, 2*sizeof(*dst));
  FILL_FUNC_TAIL_PACKED(2);
  }

static void fill_float_rgb(gavl_video_frame_t * frame,
                           const gavl_video_format_t * format,
                           const float * color)
  {
  FILL_FUNC_HEAD_PACKED(float);
  gavl_memcpy(dst, color, 3*sizeof(*dst));
  FILL_FUNC_TAIL_PACKED(3);
  }

static void fill_float_rgba(gavl_video_frame_t * frame,
                            const gavl_video_format_t * format,
                            const float * color)
  {
  FILL_FUNC_HEAD_PACKED(float);
  gavl_memcpy(dst, color, 4*sizeof(*dst));
  FILL_FUNC_TAIL_PACKED(4);
  }

#define FILL_FUNC_HEAD_PLANAR(TYPE) \
  uint8_t * dst_start; \
  TYPE * dst; \
  int i, j; \
  dst_start = frame->planes[0]; \
  for(i = 0; i < format->image_height; i++) \
    { \
    dst = (TYPE*)dst_start; \
    for(j = 0; j < format->image_width; j++) \
      {

#define FILL_FUNC_TAIL_PLANAR(ADVANCE)             \
      dst+=ADVANCE;                                \
      }                                            \
    dst_start += frame->strides[0];                 \
    }


static void fill_planar_8(gavl_video_frame_t * frame,
                          const gavl_video_format_t * format,
                          uint8_t * color)
  {
  int i, imax;
  int sub_h, sub_v;

  uint8_t * dst, *dst_1;
  
  gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
  
  /* Luminance */
  dst = frame->planes[0];
  for(i = 0; i < format->image_height; i++)
    {
    memset(dst, color[0], format->image_width);
    dst += frame->strides[0];
    }
  /* Chrominance */

  dst   = frame->planes[1];
  dst_1 = frame->planes[2];

  imax = format->image_height / sub_v;

  for(i = 0; i < imax; i++)
    {
    memset(dst,   color[1], format->image_width/sub_h);
    memset(dst_1, color[2], format->image_width/sub_h);
    dst   += frame->strides[1];
    dst_1 += frame->strides[2];
    }
  }

static void fill_planar_16(gavl_video_frame_t * frame,
                           const gavl_video_format_t * format,
                           uint16_t * color)
  {
  int i, j, imax, jmax;
  int sub_h, sub_v;
  
  uint16_t * dst, *dst_1;
  uint8_t * dst_start, *dst_start_1;
  
  gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
  
  /* Luminance */
  dst_start = frame->planes[0];
  for(i = 0; i < format->image_height; i++)
    {
    dst = (uint16_t*)dst_start;

    for(j = 0; j < format->image_width; j++)
      {
      *dst = color[0];
      dst++;
      }
    dst_start += frame->strides[0];
    }
  /* Chrominance */

  imax = format->image_height / sub_v;
  jmax = format->image_width  / sub_h;

  dst_start   = frame->planes[1];
  dst_start_1 = frame->planes[2];

  for(i = 0; i < imax; i++)
    {
    dst = (uint16_t*)dst_start;
    dst_1 = (uint16_t*)dst_start_1;

    for(j = 0; j < jmax; j++)
      {
      *dst = color[1];
      *dst_1 = color[2];
      dst++;
      dst_1++;
      }
    dst_start   += frame->strides[1];
    dst_start_1 += frame->strides[2];
    }

  
  
  }


void gavl_video_frame_fill(gavl_video_frame_t * frame,
                           const gavl_video_format_t * format,
                           const float * color)
  {
  INIT_RGB_FLOAT_TO_YUV
  uint16_t packed_16;
  uint8_t  packed_32[4];
  uint16_t packed_64[4];
  float color_float[4];

  gavl_init_memcpy();
  
  switch(format->pixelformat)
    {
    case GAVL_GRAY_8:
      RGB_FLOAT_TO_YUVJ_8(color[0], color[1], color[2], packed_32[0],
                          packed_32[1], packed_32[2]);
      fill_8_packed(frame, format, packed_32[0]);
      break;
    case GAVL_GRAYA_16:
      RGB_FLOAT_TO_YUVJ_8(color[0], color[1], color[2], packed_32[0],
                          packed_32[1], packed_32[2]);
      /* This overwrites packed_32[1] from the lone before */
      RGB_FLOAT_TO_8(color[3], packed_32[1]);
#ifndef WORDS_BIGENDIAN
      packed_16 = (packed_32[1] << 8) | packed_32[0];
#else
      packed_16 = (packed_32[0] << 8) | packed_32[1];
#endif 
      fill_16_packed(frame, format, packed_16);
      break;
    case GAVL_GRAY_16:
      RGB_FLOAT_TO_YJ_16(color[0], color[1], color[2], packed_16);
      fill_16_packed(frame, format, packed_16);
      break;
    case GAVL_GRAYA_32:
      RGB_FLOAT_TO_YJ_16(color[0], color[1], color[2], packed_64[0]);
      RGB_FLOAT_TO_16(color[3], packed_64[1]);
#ifndef WORDS_BIGENDIAN
      packed_32[0] = packed_64[0] & 0xff;
      packed_32[1] = packed_64[0] >> 8;
      packed_32[2] = packed_64[1] & 0xff;
      packed_32[3] = packed_64[1] >> 8;
#else
      packed_32[1] = packed_64[0] & 0xff;
      packed_32[0] = packed_64[0] >> 8;
      packed_32[3] = packed_64[1] & 0xff;
      packed_32[2] = packed_64[1] >> 8;
#endif 
      fill_32_packed(frame, format, packed_32);
      break;
    case GAVL_RGB_15:
      RGB_FLOAT_TO_8(color[0], packed_32[0]);
      RGB_FLOAT_TO_8(color[1], packed_32[1]);
      RGB_FLOAT_TO_8(color[2], packed_32[2]);
      PACK_8_TO_RGB15(packed_32[0],packed_32[1],packed_32[2],packed_16);
      fill_16_packed(frame, format, packed_16);
      break;
    case GAVL_BGR_15:
      RGB_FLOAT_TO_8(color[0], packed_32[0]);
      RGB_FLOAT_TO_8(color[1], packed_32[1]);
      RGB_FLOAT_TO_8(color[2], packed_32[2]);
      PACK_8_TO_BGR15(packed_32[0],packed_32[1],packed_32[2],packed_16);
      fill_16_packed(frame, format, packed_16);
      break;
    case GAVL_RGB_16:
      RGB_FLOAT_TO_8(color[0], packed_32[0]);
      RGB_FLOAT_TO_8(color[1], packed_32[1]);
      RGB_FLOAT_TO_8(color[2], packed_32[2]);
      PACK_8_TO_RGB16(packed_32[0],packed_32[1],packed_32[2],packed_16);
      fill_16_packed(frame, format, packed_16);
      break;
    case GAVL_BGR_16:
      RGB_FLOAT_TO_8(color[0], packed_32[0]);
      RGB_FLOAT_TO_8(color[1], packed_32[1]);
      RGB_FLOAT_TO_8(color[2], packed_32[2]);
      PACK_8_TO_BGR16(packed_32[0],packed_32[1],packed_32[2],packed_16);
      fill_16_packed(frame, format, packed_16);
      break;
    case GAVL_RGB_24:
      RGB_FLOAT_TO_8(color[0], packed_32[0]);
      RGB_FLOAT_TO_8(color[1], packed_32[1]);
      RGB_FLOAT_TO_8(color[2], packed_32[2]);
      fill_24_packed(frame, format, packed_32);
      break;
    case GAVL_BGR_24:
      RGB_FLOAT_TO_8(color[0], packed_32[2]);
      RGB_FLOAT_TO_8(color[1], packed_32[1]);
      RGB_FLOAT_TO_8(color[2], packed_32[0]);
      fill_24_packed(frame, format, packed_32);
      break;
    case GAVL_RGB_32:
      RGB_FLOAT_TO_8(color[0], packed_32[0]);
      RGB_FLOAT_TO_8(color[1], packed_32[1]);
      RGB_FLOAT_TO_8(color[2], packed_32[2]);
      fill_32_packed(frame, format, packed_32);
      break;
    case GAVL_BGR_32:
      RGB_FLOAT_TO_8(color[0], packed_32[2]);
      RGB_FLOAT_TO_8(color[1], packed_32[1]);
      RGB_FLOAT_TO_8(color[2], packed_32[0]);
      fill_32_packed(frame, format, packed_32);
      break;
    case GAVL_YUVA_32:
      RGB_FLOAT_TO_YUV_8(color[0], color[1], color[2],
                         packed_32[0], packed_32[1], packed_32[2]);
      RGB_FLOAT_TO_8(color[3], packed_32[3]);
      fill_32_packed(frame, format, packed_32);
      break;
    case GAVL_RGBA_32:
      RGB_FLOAT_TO_8(color[0], packed_32[0]);
      RGB_FLOAT_TO_8(color[1], packed_32[1]);
      RGB_FLOAT_TO_8(color[2], packed_32[2]);
      RGB_FLOAT_TO_8(color[3], packed_32[3]);
      fill_32_packed(frame, format, packed_32);
      break;
    case GAVL_RGB_48:
      RGB_FLOAT_TO_16(color[0], packed_64[0]);
      RGB_FLOAT_TO_16(color[1], packed_64[1]);
      RGB_FLOAT_TO_16(color[2], packed_64[2]);
      fill_48_packed(frame, format, packed_64);
      break;
    case GAVL_RGBA_64:
      RGB_FLOAT_TO_16(color[0], packed_64[0]);
      RGB_FLOAT_TO_16(color[1], packed_64[1]);
      RGB_FLOAT_TO_16(color[2], packed_64[2]);
      RGB_FLOAT_TO_16(color[3], packed_64[3]);
      fill_64_packed(frame, format, packed_64);
      break;
    case GAVL_YUVA_64:
      RGB_FLOAT_TO_YUV_16(color[0], color[1], color[2], packed_64[0],
                          packed_64[1], packed_64[2]);
      RGB_FLOAT_TO_16(color[3], packed_64[3]);
      fill_64_packed(frame, format, packed_64);
      break;
    case GAVL_GRAY_FLOAT:
      RGB_FLOAT_TO_Y_FLOAT(color[0], color[1], color[2], color_float[0]);
      fill_float_gray(frame, format, color_float);
      break;
    case GAVL_GRAYA_FLOAT:
      RGB_FLOAT_TO_Y_FLOAT(color[0], color[1], color[2], color_float[0]);
      color_float[1] = color[4];
      fill_float_graya(frame, format, color_float);
      break;
    case GAVL_YUV_FLOAT:
      RGB_FLOAT_TO_YUV_FLOAT(color[0], color[1], color[2],
                             color_float[0], color_float[1], color_float[2]);
      fill_float_rgb(frame, format, color_float);
      break;
    case GAVL_YUVA_FLOAT:
      RGB_FLOAT_TO_YUV_FLOAT(color[0], color[1], color[2],
                             color_float[0], color_float[1], color_float[2]);
      color_float[3] = color[3];
      fill_float_rgba(frame, format, color);
      break;
    case GAVL_RGB_FLOAT:
      fill_float_rgb(frame, format, color);
      break;
    case GAVL_RGBA_FLOAT:
      fill_float_rgba(frame, format, color);
      break;
    case GAVL_YUY2:
      RGB_FLOAT_TO_YUV_8(color[0], color[1], color[2],
                         packed_32[0], /* Y */
                         packed_32[1], /* U */
                         packed_32[3]);/* V */
      packed_32[2] = packed_32[0];     /* Y */
      fill_32_packed_422(frame, format, packed_32);
      break;
    case GAVL_UYVY:
      RGB_FLOAT_TO_YUV_8(color[0], color[1], color[2],
                         packed_32[1], /* Y */
                         packed_32[0], /* U */
                         packed_32[2]);/* V */
      packed_32[3] = packed_32[1];     /* Y */
      fill_32_packed_422(frame, format, packed_32);
      break;
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_444_P:
    case GAVL_YUVJ_422_P:
      RGB_FLOAT_TO_YUVJ_8(color[0], color[1], color[2], packed_32[0],
                          packed_32[1], packed_32[2]);
      fill_planar_8(frame, format, packed_32);
      break;
    case GAVL_YUV_444_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_420_P:
    case GAVL_YUV_410_P:
    case GAVL_YUV_411_P:
      RGB_FLOAT_TO_YUV_8(color[0], color[1], color[2], packed_32[0],
                         packed_32[1], packed_32[2]);
      fill_planar_8(frame, format, packed_32);
      break;
    case GAVL_YUV_422_P_16:
    case GAVL_YUV_444_P_16:
      RGB_FLOAT_TO_YUV_16(color[0], color[1], color[2], packed_64[0],
                          packed_64[1], packed_64[2]);
      fill_planar_16(frame, format, packed_64);
      break;
    case GAVL_PIXELFORMAT_NONE:
      fprintf(stderr, "Pixelformat not specified for video frame\n");
      return;
    }
  }

void gavl_video_frame_copy_metadata(gavl_video_frame_t * dst,
                                    const gavl_video_frame_t * src)
  {
  dst->timestamp       = src->timestamp;
  dst->duration        = src->duration;
  dst->timecode        = src->timecode;
  dst->interlace_mode  = src->interlace_mode;
  dst->dst_x           = src->dst_x;
  dst->dst_y           = src->dst_y;
  gavl_rectangle_i_copy(&dst->src_rect, &src->src_rect);
  }

void gavl_video_frame_set_strides(gavl_video_frame_t * frame,
                                  const gavl_video_format_t * format)
  {
  int i;
  int bytes_per_line;
  int sub_h, sub_v;
  int num_planes = gavl_pixelformat_num_planes(format->pixelformat);
  bytes_per_line = (num_planes > 1) ?
    format->frame_width * gavl_pixelformat_bytes_per_component(format->pixelformat) :
    format->frame_width * gavl_pixelformat_bytes_per_pixel(format->pixelformat);

  gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);

  for(i = 0; i < num_planes; i++)
    {
    frame->strides[i] = bytes_per_line;
    if(i)
      frame->strides[i] /= sub_h;
    }
  }
  
void gavl_video_frame_set_planes(gavl_video_frame_t * frame,
                                 const gavl_video_format_t * format,
                                 uint8_t * buffer)
  {
  int i;
  int sub_h, sub_v;
  int advance;
  int num_planes;
  if(!frame->strides[0])
    gavl_video_frame_set_strides(frame, format);

  if(buffer)
    {
    num_planes = gavl_pixelformat_num_planes(format->pixelformat);
    gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
  
    for(i = 0; i < num_planes; i++)
      {
      frame->planes[i] = buffer;
      advance = frame->strides[i] * format->frame_height;
      if(i)
        advance /= sub_v;
      buffer += advance;
      }
    }
  else
    {
    video_frame_alloc(frame, format, 0);
    }
  }

int gavl_video_frames_equal(const gavl_video_format_t * format,
                            const gavl_video_frame_t * f1,
                            const gavl_video_frame_t * f2)
  {
  int i, j;
  int height;
  int bytes_per_line;
  int sub_h, sub_v;
  int planes;
  
  planes = gavl_pixelformat_num_planes(format->pixelformat);
  height = format->image_height;
    
  bytes_per_line = gavl_pixelformat_is_planar(format->pixelformat) ?
    format->image_width * gavl_pixelformat_bytes_per_component(format->pixelformat) :
    format->image_width * gavl_pixelformat_bytes_per_pixel(format->pixelformat);
  
  for(i = 0; i < planes; i++)
    {
    if(i == 1)
      {
      gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
      bytes_per_line /= sub_h;
      height /= sub_v;
      }

    for(j = 0; j < height; j++)
      {
      if(memcmp(f1->planes[i] + j * f1->strides[i],
                f2->planes[i] + j * f2->strides[i], bytes_per_line))
        return 0;
      }
    }
  return 1;
  }

int gavl_video_frame_continuous(const gavl_video_format_t * format,
                                const gavl_video_frame_t * frame)
  {
  int i;
  int bytes_per_line;
  int bytes_per_plane;
  int sub_h, sub_v;
  
  int planes = gavl_pixelformat_num_planes(format->pixelformat);

  if(planes > 1)
    bytes_per_line = format->frame_width * gavl_pixelformat_bytes_per_component(format->pixelformat);
  else
    bytes_per_line = format->frame_width * gavl_pixelformat_bytes_per_pixel(format->pixelformat);
  
  if(frame->strides[0] != bytes_per_line)
    return 0;
  
  if(planes == 1)
    return 1;

  gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
  bytes_per_plane = bytes_per_line * format->frame_height;
  
  for(i = 1; i < planes; i++)
    {
    if(frame->planes[i] - frame->planes[i-1] != bytes_per_plane)
      return 0;
    
    if(i == 1)
      {
      bytes_per_line /= sub_h;
      bytes_per_plane = bytes_per_line * (format->frame_height / sub_v);
      }

    if(frame->strides[i] != bytes_per_line)
      return 0;
        
    }
  return 1;
  }

void gavl_video_frame_dump_metadata(const gavl_video_format_t * format,
                                    const gavl_video_frame_t * frame)
  {
  int i;
  fprintf(stderr, "PTS: %"PRId64" (%f), D: %"PRId64" (%f) ", 
          frame->timestamp, gavl_time_to_seconds(gavl_time_unscale(format->timescale, frame->timestamp)),
          frame->duration,  gavl_time_to_seconds(gavl_time_unscale(format->timescale, frame->duration)));
  if(format->timecode_format.int_framerate && (frame->timecode != GAVL_TIMECODE_UNDEFINED))
    {
    fprintf(stderr, " TC: ");
    gavl_timecode_dump(&format->timecode_format, frame->timecode);
    }
  fprintf(stderr, "\n");
  fprintf(stderr, "Strides:");
  for(i = 0; i < GAVL_MAX_PLANES; i++)
    {
    if(frame->strides[i])
      fprintf(stderr, " %d", frame->strides[i]);
    else
      break;
    }
  fprintf(stderr, "\nsrc_rect: ");
  gavl_rectangle_i_dump(&frame->src_rect);
  fprintf(stderr, " dst: %d %d\n", frame->dst_x, frame->dst_y);
  }

/* Rotate video frames in multiples of 90 degrees */

#if 0
static void rotate_8(uint8_t * src, int src_stride, 
                     uint8_t * dst, int dst_stride,
                     int ninety_deg_steps,
                     int src_width, int dst_width)
  {
  int i, j;

  uint8_t * src_ptr;
  uint8_t * dst_ptr;
  
  switch(ninety_deg_steps)
    {
    case 0:
      for(i = 0; i < src_height; i++)
        {
        src_ptr = src + 
        
        for(j = 0; j < src_width; j++)
          {
          
          }
        }
      
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    }
  
  }

                     
void gavl_video_frame_copy_rotate_clockwise(const gavl_video_format_t * src_format,
                                            gavl_video_frame_t * dst,
                                            const gavl_video_frame_t * src,
                                            int ninety_deg_steps)
  {
  ninety_deg_steps = ninety_deg_steps % 4;
  
  }
#endif
