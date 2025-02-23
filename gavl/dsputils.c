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
#include <string.h>

int gavl_dsp_interpolate_video_frame(gavl_dsp_context_t * ctx,
                                     gavl_video_format_t * format,
                                     gavl_video_frame_t * src_1,
                                     gavl_video_frame_t * src_2,
                                     gavl_video_frame_t * dst,
                                     float factor)
  {
  int num_planes;
  int sub_v, sub_h;
  int width, height;
  uint8_t * s1, *s2, *d;
  int i, j;
  
  void (*interpolate)(const uint8_t * src_1,
                      const uint8_t * src_2, 
                      uint8_t * dst, int num, float fac) = NULL;
  
  num_planes = gavl_pixelformat_num_planes(format->pixelformat);
  gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
  
  width  = format->image_width;
  height = format->image_height;
  
  switch(format->pixelformat)
    {
    case GAVL_RGB_15:
    case GAVL_BGR_15:
      interpolate = ctx->funcs.interpolate_rgb15;
      break;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
      interpolate = ctx->funcs.interpolate_rgb16;
      break;
    case GAVL_GRAY_8:
      interpolate = ctx->funcs.interpolate_8;
      break;
    case GAVL_GRAYA_16:
      interpolate = ctx->funcs.interpolate_8;
      width *= 3;
      break;
    case GAVL_GRAY_16:
      interpolate = ctx->funcs.interpolate_16;
      break;
    case GAVL_GRAYA_32:
      interpolate = ctx->funcs.interpolate_16;
      width *= 2;
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
      interpolate = ctx->funcs.interpolate_8;
      width *= 3;
      break;
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_RGBA_32:
    case GAVL_YUVA_32:
      interpolate = ctx->funcs.interpolate_8;
      width *= 4;
      break;
    case GAVL_RGB_48:
      interpolate = ctx->funcs.interpolate_16;
      width *= 3;
      break;
    case GAVL_RGBA_64:
    case GAVL_YUVA_64:
      interpolate = ctx->funcs.interpolate_16;
      width *= 4;
      break;
    case GAVL_GRAY_FLOAT:
      interpolate = ctx->funcs.interpolate_f;
      break;
    case GAVL_GRAYA_FLOAT:
      interpolate = ctx->funcs.interpolate_f;
      width *= 2;
      break;
    case GAVL_RGB_FLOAT:
    case GAVL_YUV_FLOAT:
      width *= 3;
      interpolate = ctx->funcs.interpolate_f;
      break;
    case GAVL_YUVA_FLOAT:
    case GAVL_RGBA_FLOAT:
      width *= 4;
      interpolate = ctx->funcs.interpolate_f;
      break;
    case GAVL_YUY2:
    case GAVL_UYVY:
      interpolate = ctx->funcs.interpolate_8;
      width *= 2;
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUV_410_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_444_P:
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
    case GAVL_YUV_422_P:
      interpolate = ctx->funcs.interpolate_8;
      break;
    case GAVL_YUV_422_P_16:
    case GAVL_YUV_444_P_16:
      interpolate = ctx->funcs.interpolate_16;
      break;
    case GAVL_PIXELFORMAT_NONE:
      break;
    }

  if(!interpolate)
    return 0;
  
  for(i = 0; i < num_planes; i++)
    {
    s1 = src_1->planes[i];
    s2 = src_2->planes[i];
    d  = dst->planes[i];
    
    for(j = 0; j < height; j++)
      {
      interpolate(s1, s2, d, width, factor);
      s1 += src_1->strides[i];
      s2 += src_2->strides[i];
      d  += dst->strides[i];
      }
    
    if(!i)
      {
      width /= sub_h;
      height /= sub_v;
      }
    }
  return 1;
  }

/* Audio frame utilities */

int gavl_dsp_audio_frame_swap_endian(gavl_dsp_context_t * ctx,
                                      gavl_audio_frame_t * frame,
                                      const gavl_audio_format_t * format)
  {
  int bytes_per_sample, len, i;
  void (*do_swap)(void * data, int len) = NULL;
  
  bytes_per_sample = gavl_bytes_per_sample(format->sample_format);
  
  switch(bytes_per_sample)
    {
    case 1:
      return 1;
      break;
    case 2:
      do_swap = ctx->funcs.bswap_16;
      break;
    case 4:
      do_swap = ctx->funcs.bswap_32;
      break;
    case 8:
      do_swap = ctx->funcs.bswap_64;
      break;
    default:
      return 0;
    }
  
  if(!do_swap)
    return 0;
  
  switch(format->interleave_mode)
    {
    
    case GAVL_INTERLEAVE_NONE:
      len = frame->valid_samples;
      for(i = 0; i < format->num_channels; i++)
        do_swap(frame->channels.u_8[i], len);
      break;
    case GAVL_INTERLEAVE_2:
      len = frame->valid_samples * 2;
      for(i = 0; i < format->num_channels/2; i++)
        do_swap(frame->channels.u_8[2*i], len);
      
      if(format->num_channels % 2)
        {
        len = frame->valid_samples;
        do_swap(frame->channels.u_8[format->num_channels-1], len);
        }
    
      break;
    case GAVL_INTERLEAVE_ALL:
      len = frame->valid_samples * format->num_channels;
      do_swap(frame->samples.u_8, len);
      break;
    }
  return 1;
  }

int gavl_dsp_video_frame_swap_endian(gavl_dsp_context_t * ctx,
                                      gavl_video_frame_t * frame,
                                      const gavl_video_format_t * format)
  {
  void (*do_swap)(void * data, int len) = NULL;
  int i, j, num_planes;
  uint8_t * src;
  int sub_h = 1, sub_v = 1;
  int height = format->image_height;
  int len = format->image_width;
  
  num_planes = 1;
  
  gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
  
  switch(format->pixelformat)
    {
    case GAVL_RGB_15:
    case GAVL_BGR_15:
    case GAVL_RGB_16:
    case GAVL_BGR_16:
    case GAVL_GRAY_16:
      do_swap = ctx->funcs.bswap_16;
      break;
    case GAVL_GRAY_FLOAT:
#if SIZEOF_FLOAT == 4
      do_swap = ctx->funcs.bswap_32;
#elif SIZEOF_FLOAT == 8
      do_swap = ctx->funcs.bswap_64;
#endif
      break;
    case GAVL_GRAYA_FLOAT:
      len *= 2;
#if SIZEOF_FLOAT == 4
      do_swap = ctx->funcs.bswap_32;
#elif SIZEOF_FLOAT == 8
      do_swap = ctx->funcs.bswap_64;
#endif
      break;
    case GAVL_GRAYA_32:
      len *= 2;
      do_swap = ctx->funcs.bswap_16;
      break;
    case GAVL_RGB_48:
      len *= 3;
      do_swap = ctx->funcs.bswap_16;
      break;
    case GAVL_RGBA_64:
    case GAVL_YUVA_64:
      len *= 4;
      do_swap = ctx->funcs.bswap_32;
      break;
    case GAVL_RGB_FLOAT:
    case GAVL_YUV_FLOAT:
      len *= 3;
#if SIZEOF_FLOAT == 4
      do_swap = ctx->funcs.bswap_32;
#elif SIZEOF_FLOAT == 8
      do_swap = ctx->funcs.bswap_64;
#endif
      break;
    case GAVL_RGBA_FLOAT:
    case GAVL_YUVA_FLOAT:
      len *= 4;
#if SIZEOF_FLOAT == 4
      do_swap = ctx->funcs.bswap_32;
#elif SIZEOF_FLOAT == 8
      do_swap = ctx->funcs.bswap_64;
#endif
      break;
    case GAVL_YUV_422_P_16:
      do_swap = ctx->funcs.bswap_16;
      num_planes = 3;
      break;
    case GAVL_YUV_444_P_16:
      do_swap = ctx->funcs.bswap_16;
      num_planes = 3;
      break;
    case GAVL_YUY2:
    case GAVL_UYVY:
    case GAVL_RGB_24:
    case GAVL_BGR_24:
    case GAVL_YUV_420_P:
    case GAVL_YUV_410_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_444_P:
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
    case GAVL_PIXELFORMAT_NONE:
    case GAVL_GRAY_8:
    case GAVL_GRAYA_16:
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_RGBA_32:
    case GAVL_YUVA_32:
      return 1;
      break;
    }

  if(!do_swap)
    return 0;

  for(i = 0; i < num_planes; i++)
    {
    if(i == 1)
      {
      height /= sub_v;
      len /= sub_h;
      }
    
    src = frame->planes[i];
    for(j = 0; j < height; j++)
      {
      do_swap(src, len);
      src += frame->strides[i];
      }
    }
  return 1;
  }

void
gavl_dsp_video_frame_shift_bits(gavl_dsp_context_t * ctx,
                                gavl_video_frame_t * frame,
                                const gavl_video_format_t * format, int bits)
  {
  int i, j;
  int sub_h = 1, sub_v = 1;
  int len;
  int height;
  int channels;
  uint8_t * src;

  void (*shift_func)(void *, int, int);
  
  int planes;

  if(!bits)
    return;
  
  planes = gavl_pixelformat_num_planes(format->pixelformat);
    
  if(planes > 1)
    {
    if(gavl_pixelformat_bytes_per_component(format->pixelformat) != 2)
      return;
    channels = 1;
    }
  else
    {
    channels = gavl_pixelformat_num_channels(format->pixelformat);
    if(gavl_pixelformat_bytes_per_pixel(format->pixelformat) != 2*channels)
      return;
    gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
    }

  if(bits < 0)
    {
    shift_func = ctx->funcs.shift_down_16;
    bits = -bits;
    }
  else
    shift_func = ctx->funcs.shift_up_16;
  
  len = format->image_width * channels;
  height = format->image_height;
  
  for(i = 0; i < planes; i++)
    {
    if(i == 1)
      {
      height /= sub_v;
      len /= sub_h;
      }

    src = frame->planes[i];
    
    for(j = 0; j < height; j++)
      {
      shift_func(src, len, bits);
      src += frame->strides[i];
      }
    }
  
  }


void
gavl_dsp_video_frame_shift_bits_copy(gavl_dsp_context_t * ctx,
                                     gavl_video_frame_t * dst,
                                     const gavl_video_frame_t * src,
                                     const gavl_video_format_t * format, int bits)
  {
  int i, j;
  int sub_h = 1, sub_v = 1;
  int len;
  int height;
  int channels;
  const uint8_t * src_ptr;
  uint8_t * dst_ptr;

  void (*shift_func)(void *, const void *, int, int);
  
  int planes;

  if(!bits)
    return;
  
  planes = gavl_pixelformat_num_planes(format->pixelformat);
    
  if(planes > 1)
    {
    if(gavl_pixelformat_bytes_per_component(format->pixelformat) != 2)
      return;
    channels = 1;
    }
  else
    {
    channels = gavl_pixelformat_num_channels(format->pixelformat);
    if(gavl_pixelformat_bytes_per_pixel(format->pixelformat) != 2*channels)
      return;
    gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
    }

  if(bits < 0)
    {
    shift_func = ctx->funcs.shift_down_copy_16;
    bits = -bits;
    }
  else
    shift_func = ctx->funcs.shift_up_copy_16;
  
  len = format->image_width * channels;
  height = format->image_height;
  
  for(i = 0; i < planes; i++)
    {
    if(i == 1)
      {
      height /= sub_v;
      len /= sub_h;
      }

    src_ptr = src->planes[i];
    dst_ptr = dst->planes[i];
    
    for(j = 0; j < height; j++)
      {
      shift_func(dst_ptr, src_ptr, len, bits);
      src_ptr += src->strides[i];
      dst_ptr += dst->strides[i];
      }
    }
  
  }

static int get_byte_index(uint32_t b)
  {
  int ret = 0;

  while(b >>= 1)
    ret++;

  ret /= 8;
  
#ifdef WORDS_BIGENDIAN
  return 3 - ret;
#else
  return ret;
#endif
  }

void
gavl_dsp_video_frame_shuffle_bytes(gavl_dsp_context_t * ctx,
                                   gavl_video_frame_t * frame,
                                   const gavl_video_format_t * format,
                                   const uint32_t * src_masks, const uint32_t * dst_masks)
  {
  uint8_t * ptr;
  int i;
  int src_byte, dst_byte;
  int do_convert = 0;
  uint8_t mask[4] = { 0, 0, 0, 0 };

#if 0
  fprintf(stderr, "gavl_dsp_video_frame_shuffle_bytes\n");
  fprintf(stderr, "  src: %08x %08x %08x %08x\n", src_masks[0], src_masks[1], src_masks[2], src_masks[3]);
  fprintf(stderr, "  dst: %08x %08x %08x %08x\n", dst_masks[0], dst_masks[1], dst_masks[2], dst_masks[3]);
#endif

  for(i = 0; i < 4; i++)
    {
    src_byte = get_byte_index(src_masks[i]);
    dst_byte = get_byte_index(dst_masks[i]);
    if(src_byte != dst_byte)
      do_convert = 1;

    mask[dst_byte] = src_byte;
    }

  if(!do_convert) // Nothing to do
    return;
  
  ptr = frame->planes[0];
  
  for(i = 0; i < format->image_height; i++)
    {
    ctx->funcs.shuffle_8_4(ptr, format->image_width, mask);
    ptr += frame->strides[0];
    }
  
  }
