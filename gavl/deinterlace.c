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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <gavl/gavl.h>
#include <video.h>

#include <deinterlace.h>
#include <accel.h>

gavl_video_deinterlacer_t * gavl_video_deinterlacer_create()
  {
  gavl_video_deinterlacer_t * ret;
  ret = calloc(1, sizeof(*ret));
  gavl_video_options_set_defaults(&ret->opt);

  ret->src_field = gavl_video_frame_create(NULL);
  ret->dst_field = gavl_video_frame_create(NULL);
  return ret;
  }

void gavl_video_deinterlacer_destroy(gavl_video_deinterlacer_t * d)
  {
  gavl_video_frame_destroy(d->src_field);
  gavl_video_frame_destroy(d->dst_field);

  if(d->scaler)
    gavl_video_scaler_destroy(d->scaler);
  
  free(d);
  }

gavl_video_options_t *
gavl_video_deinterlacer_get_options(gavl_video_deinterlacer_t * d)
  {
  return &d->opt;
  }


int gavl_video_deinterlacer_init(gavl_video_deinterlacer_t * d,
                                 const gavl_video_format_t * src_format)
  {
  
  gavl_video_format_copy(&d->format, src_format);
  gavl_video_format_copy(&d->half_height_format, src_format);

  if((d->format.interlace_mode == GAVL_INTERLACE_MIXED) ||
     (d->format.interlace_mode == GAVL_INTERLACE_MIXED_TOP) ||
     (d->format.interlace_mode == GAVL_INTERLACE_MIXED_BOTTOM))
    d->mixed = 1;
  else
    d->mixed = 0;
  
  d->half_height_format.image_height /= 2;
  d->half_height_format.frame_height /= 2;

  d->num_planes = gavl_pixelformat_num_planes(d->format.pixelformat);
  gavl_pixelformat_chroma_sub(d->format.pixelformat, &d->sub_h, &d->sub_v);
  
  switch(d->opt.deinterlace_mode)
    {
    case GAVL_DEINTERLACE_NONE:
      break;
    case GAVL_DEINTERLACE_COPY:
      gavl_deinterlacer_init_copy(d);
      break;
    case GAVL_DEINTERLACE_SCALE:
      gavl_deinterlacer_init_scale(d);
      break;
    case GAVL_DEINTERLACE_BLEND:
      if(!gavl_deinterlacer_init_blend(d))
        return 0;
      break;
    }
  return 1;
  }

void gavl_video_deinterlacer_deinterlace(gavl_video_deinterlacer_t * d,
                                         const gavl_video_frame_t * input_frame,
                                         gavl_video_frame_t * output_frame)
  {
  if(d->mixed)
    {
    if((input_frame->interlace_mode != GAVL_INTERLACE_NONE) ||
       (d->opt.conversion_flags & GAVL_FORCE_DEINTERLACE))
      d->func(d, input_frame, output_frame);
    else
      gavl_video_frame_copy(&d->format, output_frame, input_frame);
    }
  else
    d->func(d, input_frame, output_frame);
  }

