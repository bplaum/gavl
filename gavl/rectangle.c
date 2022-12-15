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

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <gavl/gavl.h>

void gavl_rectangle_i_dump(const gavl_rectangle_i_t * r)
  {
  fprintf(stderr, "%dx%d+%d+%d", r->w, r->h, r->x, r->y);
  }

void gavl_rectangle_f_dump(const gavl_rectangle_f_t * r)
  {
  fprintf(stderr, "%fx%f+%f+%f", r->w, r->h, r->x, r->y);
  }

void gavl_rectangle_i_init(gavl_rectangle_i_t * r)
  {
  memset(r, 0, sizeof(*r));
  }

void gavl_rectangle_f_init(gavl_rectangle_f_t * r)
  {
  memset(r, 0, sizeof(*r));
  }


void gavl_rectangle_i_crop_to_format(gavl_rectangle_i_t * r, const gavl_video_format_t * format)
  {
  if(r->x < 0)
    {
    r->w += r->x;
    r->x = 0;
    }
  if(r->y < 0)
    {
    r->h += r->y;
    r->y = 0;
    }

  /* Check if rectangle is empty */
    
  if((r->x > format->image_width) || (r->y > format->image_height) ||
     (r->w < 0) || (r->h < 0))
    {
    r->x = 0;
    r->y = 0;
    r->w = 0;
    r->h = 0;
    return;
    }

  if(r->w > format->image_width - r->x)
    r->w = format->image_width - r->x;

  if(r->h > format->image_height - r->y)
    r->h = format->image_height - r->y;
  
  }

#define GAVL_MIN(x, y) (x < y ? x : y);

void gavl_rectangle_crop_to_format_noscale(gavl_rectangle_i_t * src_rect,
                                           gavl_rectangle_i_t * dst_rect,
                                           const gavl_video_format_t * src_format,
                                           const gavl_video_format_t * dst_format)
  {
  src_rect->w = GAVL_MIN(src_format->image_width,  dst_format->image_width);
  src_rect->h = GAVL_MIN(src_format->image_height, dst_format->image_height);

  dst_rect->w = src_rect->w;
  dst_rect->h = src_rect->h;

  src_rect->x = (src_format->image_width - src_rect->w) / 2;
  src_rect->y = (src_format->image_height - src_rect->h) / 2;

  dst_rect->x = (dst_format->image_width - dst_rect->w) / 2;
  dst_rect->y = (dst_format->image_height - dst_rect->h) / 2;
  
  }

#undef GAVL_MIN

static void
crop_dimension_scale(double * src_off, double * src_len, uint32_t src_size,
                     int32_t * dst_off, int32_t * dst_len, uint32_t dst_size)
  {
  double scale_factor;
  double dst_off_f, dst_len_f;
  double crop;
  
  dst_off_f = (double)(*dst_off);
  dst_len_f = (double)(*dst_len);
  
  scale_factor = (double)(*dst_len) / *src_len;

  /* Lower limit (source) */
  if(*src_off < 0.0)
    {
    crop = - (*src_off);
    dst_off_f += (crop*scale_factor);
    dst_len_f -= (crop*scale_factor);
    *src_len  -= (crop);
    *src_off = 0.0;
    }

  /* Upper limit (source) */
  if(*src_off + *src_len > (double)(src_size))
    {
    crop = *src_off + *src_len - (double)(src_size); 
    dst_len_f -= (crop*scale_factor);
    *src_len  -= crop;
    }

  /* Lower limit (destination) */
  if(dst_off_f < 0.0)
    {
    crop = - (*dst_off);
    *src_off += (crop/scale_factor);
    *src_len -= (crop/scale_factor);
    dst_len_f  -= (crop);
    dst_off_f = 0.0;
    }
  
  /* Upper limit (destination) */
  if(dst_off_f + dst_len_f > (double)(dst_size))
    {
    crop = dst_off_f + dst_len_f - (double)(dst_size); 
    dst_len_f -= (crop);
    *src_len  -= crop/scale_factor;
    }
  *dst_len = (int)(dst_len_f+0.5);
  *dst_off = (int)(dst_off_f+0.5);
  
  }


void gavl_rectangle_crop_to_format_scale(gavl_rectangle_f_t * src_rect,
                                         gavl_rectangle_i_t * dst_rect,
                                         const gavl_video_format_t * src_format,
                                         const gavl_video_format_t * dst_format)
  {
  crop_dimension_scale(&src_rect->x, &src_rect->w, src_format->image_width,
                       &dst_rect->x, &dst_rect->w, dst_format->image_width);
  crop_dimension_scale(&src_rect->y, &src_rect->h, src_format->image_height,
                       &dst_rect->y, &dst_rect->h, dst_format->image_height);
  }


/* Let a rectangle span the whole screen for format */

void gavl_rectangle_i_set_all(gavl_rectangle_i_t * r, const gavl_video_format_t * format)
  {
  r->x = 0;
  r->y = 0;
  r->w = format->image_width;
  r->h = format->image_height;
  
  }

void gavl_rectangle_f_set_all(gavl_rectangle_f_t * r, const gavl_video_format_t * format)
  {
  r->x = 0;
  r->y = 0;
  r->w = format->image_width;
  r->h = format->image_height;
  
  }

void gavl_rectangle_i_crop_left(gavl_rectangle_i_t * r, int num_pixels)
  {
  r->x += num_pixels;
  r->w -= num_pixels;
  }

void gavl_rectangle_i_crop_right(gavl_rectangle_i_t * r, int num_pixels)
  {
  r->w -= num_pixels;
  }

void gavl_rectangle_i_crop_top(gavl_rectangle_i_t * r, int num_pixels)
  {
  r->y += num_pixels;
  r->h -= num_pixels;
  }

void gavl_rectangle_i_crop_bottom(gavl_rectangle_i_t * r, int num_pixels)
  {
  r->h -= num_pixels;
  }

void gavl_rectangle_f_crop_left(gavl_rectangle_f_t * r, double num_pixels)
  {
  r->x += num_pixels;
  r->w -= num_pixels;
  }

void gavl_rectangle_f_crop_right(gavl_rectangle_f_t * r, double num_pixels)
  {
  r->w -= num_pixels;
  }

void gavl_rectangle_f_crop_top(gavl_rectangle_f_t * r, double num_pixels)
  {
  r->y += num_pixels;
  r->h -= num_pixels;
  }

void gavl_rectangle_f_crop_bottom(gavl_rectangle_f_t * r, double num_pixels)
  {
  r->h -= num_pixels;
  }

#define PADD(num, multiple) num -= num % multiple

void gavl_rectangle_i_align(gavl_rectangle_i_t * r, int h_align, int v_align)
  {
  PADD(r->x, h_align);
  PADD(r->w, h_align);

  PADD(r->y, v_align);
  PADD(r->h, v_align);
  }

void gavl_rectangle_i_align_to_format(gavl_rectangle_i_t * r,
                                      const gavl_video_format_t * format)
  {
  int sub_h, sub_v;
  gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
  gavl_rectangle_i_align(r, sub_h, sub_v);
  }


void gavl_rectangle_f_copy(gavl_rectangle_f_t * dst, const gavl_rectangle_f_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  }

void gavl_rectangle_i_copy(gavl_rectangle_i_t * dst, const gavl_rectangle_i_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  }

int gavl_rectangle_i_is_empty(const gavl_rectangle_i_t * r)
  {
  return ((r->w <= 0) || (r->h <= 0)) ? 1 : 0;
  }

int gavl_rectangle_f_is_empty(const gavl_rectangle_f_t * r)
  {
  return ((r->w <= 0.0) || (r->h <= 0.0)) ? 1 : 0;
  }


/* Assuming we take src_rect from a frame in format src_format,
   calculate the optimal dst_rect in dst_format. */

void gavl_rectangle_fit_aspect(gavl_rectangle_i_t * r,
                               const gavl_video_format_t * src_format,
                               const gavl_rectangle_f_t * src_rect,
                               const gavl_video_format_t * dst_format,
                               float zoom, float squeeze)
  {
  float dst_display_aspect;
  float dst_pixel_aspect;
  float src_display_aspect;

  float squeeze_factor;

  squeeze_factor = pow(2.0, squeeze);
  
  src_display_aspect = squeeze_factor * 
    src_rect->w * (float)(src_format->pixel_width) /
    (src_rect->h * (float)(src_format->pixel_height));

  dst_pixel_aspect =
    (float)(dst_format->pixel_width) /
    (float)(dst_format->pixel_height);
  
  dst_display_aspect =
    dst_pixel_aspect * 
    (float)(dst_format->image_width) /
    (float)(dst_format->image_height);

  if(dst_display_aspect > src_display_aspect) /* Bars left and right */
    {
    //    fprintf(stderr, "Bars left and right\n");
    r->w = (int)((float)dst_format->image_height * src_display_aspect * zoom / dst_pixel_aspect + 0.5);
    r->h = (int)((float)dst_format->image_height * zoom + 0.5); 
    //    fprintf(stderr, "Bars left and right %dx%d -> %dx%d (%f, %f)\n", src_rect->w, src_rect->h, r->w, r->h,
    //            (float)(src_rect->w * src_format->pixel_width) /
    //            (float)(src_rect->h * src_format->pixel_height), dst_pixel_aspect);
    }
  else  /* Bars top and bottom */
    {
    //    fprintf(stderr, "Bars top and bottom\n");
    r->w = (int)((float)(dst_format->image_width) * zoom + 0.5);
    r->h = (int)((float)dst_format->image_width   * zoom * dst_pixel_aspect / src_display_aspect + 0.5);
    }
  r->x = ((int)dst_format->image_width - r->w)/2;
  r->y = ((int)dst_format->image_height - r->h)/2;
  gavl_rectangle_i_align_to_format(r, dst_format);
  }

void gavl_rectangle_i_to_f(gavl_rectangle_f_t * dst, const gavl_rectangle_i_t * src)
  {
  dst->x = (double)(src->x);
  dst->y = (double)(src->y);
  dst->w = (double)(src->w);
  dst->h = (double)(src->h);
  }
  
void gavl_rectangle_f_to_i(gavl_rectangle_i_t * dst, const gavl_rectangle_f_t * src)
  {
  dst->x = (int)(src->x+0.5);
  dst->y = (int)(src->y+0.5);
  dst->w = (int)(src->w+0.5);
  dst->h = (int)(src->h+0.5);
  }

