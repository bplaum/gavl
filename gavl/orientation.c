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


#include <string.h>

#include <gavl/gavl.h>
#include <video.h>

#include <gavl/log.h>
#define LOG_DOMAIN "orientation"

static struct
  {
  gavl_image_orientation_t orient;
  int transpose;
  int flip_h_src;
  int flip_v_src;
  const char * label;
  }
orientations[8] =
  {
    
    { GAVL_IMAGE_ORIENT_NORMAL,       0, 0, 0, "Normal (exif: 1)" }, // EXIF: 1
    { GAVL_IMAGE_ORIENT_ROT90_CW,     1, 1, 0, "Rotated 90 deg CW (exif: 8)" }, // EXIF: 8
    { GAVL_IMAGE_ORIENT_ROT180_CW,    0, 1, 1, "Rotated 180 deg CW (exif: 3)" }, // EXIF: 3
    { GAVL_IMAGE_ORIENT_ROT270_CW,    1, 0, 1, "Rotated 270 deg CW (exif: 6)" }, // EXIF: 6
    
    { GAVL_IMAGE_ORIENT_FH,           0, 1, 0, "Mirrored (exif: 2)" }, // EXIF: 2
    { GAVL_IMAGE_ORIENT_FH_ROT90_CW,  1, 1, 1, "Mirrored rotated 90 deg CW (exif: 7)" }, // EXIF: 7
    { GAVL_IMAGE_ORIENT_FH_ROT180_CW, 0, 0, 1, "Mirrored rotated 180 deg CW (exif: 4)" }, // EXIF: 4
    { GAVL_IMAGE_ORIENT_FH_ROT270_CW, 1, 0, 0, "Mirrored rotated 270 deg CW (exif: 5)" }, // EXIF: 5
    
  };

typedef void (*scanline_func_t)(const uint8_t * in, uint8_t * out, int in_advance, int num);

static int get_orient_idx(gavl_image_orientation_t orient)
  {
  int i;
  for(i = 0; i < 8; i++)
    {
    if(orientations[i].orient == orient)
      return i;
    }
  return -1;
  }

const char *
gavl_image_orientation_to_string(gavl_image_orientation_t orient)
  {
  int i;
  for(i = 0; i < 8; i++)
    {
    if(orientations[i].orient == orient)
      return orientations[i].label;
    }
  return NULL;
  }


static void scanline_func_8(const uint8_t * in, uint8_t * out, int in_advance, int num)
  {
  int i;
  for(i = 0; i < num; i++)
    {
    *out = *in;
    
    in += in_advance;
    out++;
    }
  }

static void scanline_func_16(const uint8_t * in, uint8_t * out, int in_advance, int num)
  {
  int i;
  for(i = 0; i < num; i++)
    {
    out[0] = in[0];
    out[1] = in[1];
    
    in += in_advance;
    out += 2;
    }
  
  }

static void scanline_func_24(const uint8_t * in, uint8_t * out, int in_advance, int num)
  {
  int i;
  for(i = 0; i < num; i++)
    {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    in += in_advance;
    out += 3;
    }
  
  }

static void scanline_func_32(const uint8_t * inp, uint8_t * outp, int in_advance, int num)
  {
  int i;

  const uint32_t * in = (uint32_t *)inp;
  uint32_t * out = (uint32_t *)outp;
  in_advance /= 4;
  for(i = 0; i < num; i++)
    {
    out[0] = in[0];
    in += in_advance;
    out++;
    }
  }

static void scanline_func_48(const uint8_t * in, uint8_t * out, int in_advance, int num)
  {
  int i;

  for(i = 0; i < num; i++)
    {
    memcpy(out, in, 6);
    in += in_advance;
    out+=6;
    }
  
  }

static void scanline_func_64(const uint8_t * inp, uint8_t * outp, int in_advance, int num)
  {
  int i;

  const uint64_t * in = (uint64_t *)inp;
  uint64_t * out = (uint64_t *)outp;
  in_advance /= 8;
  for(i = 0; i < num; i++)
    {
    out[0] = in[0];
    in += in_advance;
    out++;
    }
  }

static void scanline_func_floatx3(const uint8_t * inp, uint8_t * outp, int in_advance, int num)
  {
  int i;

  const uint64_t * in = (uint64_t *)inp;
  uint64_t * out = (uint64_t *)outp;
  in_advance /= 8;
  for(i = 0; i < num; i++)
    {
    memcpy(out, in, 3*sizeof(float));
    in += in_advance;
    out+=3*sizeof(float);
    }
  }

static void scanline_func_floatx4(const uint8_t * inp, uint8_t * outp, int in_advance, int num)
  {
  int i;

  const uint64_t * in = (uint64_t *)inp;
  uint64_t * out = (uint64_t *)outp;
  in_advance /= 8;
  for(i = 0; i < num; i++)
    {
    memcpy(out, in, 4*sizeof(float));
    in += in_advance;
    out+=4*sizeof(float);
    }
  }

/*
    { GAVL_IMAGE_ORIENT_NORMAL,       0, 0, 0 }, // EXIF: 1
    { GAVL_IMAGE_ORIENT_ROT90_CW,     1, 1, 0 }, // EXIF: 8
    { GAVL_IMAGE_ORIENT_ROT180_CW,    0, 1, 1 }, // EXIF: 3
    { GAVL_IMAGE_ORIENT_ROT270_CW,    1, 0, 1 }, // EXIF: 6
    
    { GAVL_IMAGE_ORIENT_FH,           0, 1, 0 }, // EXIF: 2
    { GAVL_IMAGE_ORIENT_FH_ROT90_CW,  1, 1, 1 }, // EXIF: 7
    { GAVL_IMAGE_ORIENT_FH_ROT180_CW, 0, 0, 1 }, // EXIF: 4
    { GAVL_IMAGE_ORIENT_FH_ROT270_CW, 1, 0, 0 }, // EXIF: 5
  
 */

static void normalize_plane(int out_width, int out_height,
                            const gavl_video_frame_t * in_frame,
                            gavl_video_frame_t * out_frame, int plane, int bytes, int idx,
                            scanline_func_t scanline_func)
  {
  const uint8_t * in;
  uint8_t * out;
  int i;
  int col_advance; // Byte advance in the SRC image to advance to the next col in the DST image
  int row_advance; // Byte advance in the SRC image to advance to the next row in the DST image
  
  in = in_frame->planes[plane];
  out = out_frame->planes[plane];
  
  if(orientations[idx].transpose)
    {
    col_advance = in_frame->strides[plane];
    row_advance = bytes;

    if(orientations[idx].flip_h_src) // 7, 8
      {
      in += row_advance * (out_height - 1);
      row_advance = -row_advance;
      }
    if(orientations[idx].flip_v_src) // 6, 7
      {
      in += col_advance * (out_width - 1);
      col_advance = -col_advance;
      }
    }
  else
    {
    row_advance = in_frame->strides[plane];
    col_advance = bytes;
    
    if(orientations[idx].flip_h_src) // 3, 2
      {
      in += col_advance * (out_width - 1);
      col_advance = -col_advance;
      }
    if(orientations[idx].flip_v_src)
      {
      in += row_advance * (out_height - 1);
      row_advance = -row_advance;
      }
    }
  
  for(i = 0; i < out_height; i++)
    {
    scanline_func(in, out, col_advance, out_width);
    out += out_frame->strides[plane];
    in += row_advance;
    }
  
  }

static scanline_func_t get_scanline_func(int bytes)
  {
  switch(bytes)
    {
    case 1:
      return scanline_func_8;
      break;
    case 2:
      return scanline_func_16;
      break;
    case 3:
      return scanline_func_24;
      break;
    case 4:
      return scanline_func_32;
      break;
    case 6:
      return scanline_func_48;
      break;
    case 8:
      return scanline_func_64;
      break;
    case 3*sizeof(float):
      return scanline_func_floatx3;
      break;
    case 4*sizeof(float):
      return scanline_func_floatx4;
      break;
    }
  gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "No scanline function for %d bytes found", bytes);
  return NULL;
  }
  
void gavl_video_frame_normalize_orientation(const gavl_video_format_t * in_format,
                                            const gavl_video_format_t * out_format,
                                            const gavl_video_frame_t * in_frame,
                                            gavl_video_frame_t * out_frame)
  {
  int bytes;
  scanline_func_t func;
  
  int idx = get_orient_idx(in_format->orientation);
    
  if(idx < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Invalid image orientation %d", in_format->orientation);
    return;
    }
  
  if(!gavl_pixelformat_is_planar(in_format->pixelformat))
    {
    bytes = gavl_pixelformat_bytes_per_pixel(in_format->pixelformat);

    func = get_scanline_func(bytes);
    
    normalize_plane(out_format->image_width, out_format->image_height,
                    in_frame, out_frame, 0, bytes, idx, func);
    }
  else
    {
    int i;
    int sub_h, sub_v;
    int out_width, out_height;
    int num_planes = gavl_pixelformat_num_planes(in_format->pixelformat);

    out_width  = out_format->image_width;
    out_height = out_format->image_height;
    
    bytes = gavl_pixelformat_bytes_per_component(in_format->pixelformat);
    gavl_pixelformat_chroma_sub(out_format->pixelformat, &sub_h, &sub_v);

    func = get_scanline_func(bytes);
    
    for(i = 0; i < num_planes; i++)
      {
      if(i == 1)
        {
        out_width /= sub_h;
        out_height /= sub_v;
        }
      normalize_plane(out_width, out_height, in_frame, out_frame, i, bytes, idx, func);
      }
    }
  gavl_video_frame_copy_metadata(out_frame, in_frame);
  }

void gavl_video_format_normalize_orientation(gavl_video_format_t * in_format,
                                             gavl_video_format_t * out_format)
  {
  int idx = get_orient_idx(in_format->orientation);
  
  if(idx < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Invalid image orientation %d", in_format->orientation);
    return;
    }
  
  switch(in_format->pixelformat)
    {
    case GAVL_GRAY_8:
    case GAVL_GRAY_16:
    case GAVL_GRAY_FLOAT:
    case GAVL_GRAYA_16:
    case GAVL_GRAYA_32:
    case GAVL_GRAYA_FLOAT:
    case GAVL_RGB_15:
    case GAVL_BGR_15:
    case GAVL_RGB_16:
    case GAVL_BGR_16:
    case GAVL_RGB_24:
    case GAVL_BGR_24:
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_RGBA_32:
    case GAVL_YUVA_32:
    case GAVL_YUVA_64:
    case GAVL_RGB_48:
    case GAVL_RGBA_64:
    case GAVL_RGB_FLOAT:
    case GAVL_RGBA_FLOAT:
    case GAVL_YUV_FLOAT:
    case GAVL_YUVA_FLOAT:
    case GAVL_YUV_444_P:
    case GAVL_YUV_444_P_16:
    case GAVL_YUVJ_444_P:
    case GAVL_YUV_410_P:
    case GAVL_PIXELFORMAT_NONE:
      break;
    case GAVL_YUY2:
    case GAVL_UYVY:
    case GAVL_YUV_422_P:
    case GAVL_YUV_411_P:
      if(in_format->orientation != GAVL_IMAGE_ORIENT_FH_ROT180_CW)
        in_format->pixelformat = GAVL_YUV_444_P;
      break;
    case GAVL_YUV_422_P_16:
      if(in_format->orientation != GAVL_IMAGE_ORIENT_FH_ROT180_CW)
        in_format->pixelformat = GAVL_YUV_444_P_16;
      break;
    case GAVL_YUVJ_420_P:
      if(in_format->chroma_placement != GAVL_CHROMA_PLACEMENT_DEFAULT)
        in_format->pixelformat = GAVL_YUVJ_444_P;
      break;
    case GAVL_YUV_420_P:
      if(in_format->chroma_placement != GAVL_CHROMA_PLACEMENT_DEFAULT)
        in_format->pixelformat = GAVL_YUV_444_P;
      break;
    case GAVL_YUVJ_422_P:
      if(in_format->orientation != GAVL_IMAGE_ORIENT_FH_ROT180_CW)
        in_format->pixelformat = GAVL_YUV_444_P_16;
      break;
    }

  gavl_video_format_copy(out_format, in_format);
  
  if(orientations[idx].transpose)
    {
    out_format->image_width = in_format->image_height;
    out_format->image_height = in_format->image_width;
    out_format->pixel_width = in_format->pixel_height;
    out_format->pixel_height = in_format->pixel_width;
    gavl_video_format_set_frame_size(out_format, 0, 0);
    }
  
  out_format->orientation = GAVL_IMAGE_ORIENT_NORMAL;
  }
