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
#include <deinterlace.h>
#include <accel.h>

static void deinterlace_copy(gavl_video_deinterlacer_t * d,
                             const gavl_video_frame_t * input_frame,
                             gavl_video_frame_t * output_frame)
  {
  int i, j, jmax;
  int bytes;
  uint8_t * src;
  uint8_t * dst;
  int src_field =
    (d->opt.deinterlace_drop_mode == GAVL_DEINTERLACE_DROP_TOP) ? 1 : 0;
  
  jmax = d->format.image_height / 2;
  bytes = d->line_width;

    
  for(i = 0; i < d->num_planes; i++)
    {
    src = input_frame->planes[i] + src_field * input_frame->strides[i];
    dst = output_frame->planes[i];
    
    if(i == 1)
      {
      jmax /= d->sub_v;
      bytes /= d->sub_h;
      }
    for(j = 0; j < jmax; j++)
      {
      gavl_memcpy(dst, src, bytes);
      dst += output_frame->strides[i];
      gavl_memcpy(dst, src, bytes);
      dst += output_frame->strides[i];
      src += input_frame->strides[i] * 2;
      }
    }
  }

int
gavl_deinterlacer_init_copy(gavl_video_deinterlacer_t* d)
  {
  d->func = deinterlace_copy;
  d->line_width = gavl_pixelformat_is_planar(d->format.pixelformat) ?
    d->format.image_width *
    gavl_pixelformat_bytes_per_component(d->format.pixelformat) :
    d->format.image_width *
    gavl_pixelformat_bytes_per_pixel(d->format.pixelformat);

  gavl_init_memcpy();
  
  return 1;
  }

