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

#include <gavl/gavl.h>
#include <video.h>
#include <deinterlace.h>

static void deinterlace_scale(gavl_video_deinterlacer_t * d,
                              const gavl_video_frame_t * in,
                              gavl_video_frame_t * out)
  { 
  gavl_video_scaler_scale(d->scaler, in, out);
  }

int gavl_deinterlacer_init_scale(gavl_video_deinterlacer_t * d)
  {
  gavl_video_options_t * scaler_opt;
  gavl_video_format_t in_format;
  gavl_video_format_t out_format;
  
  if(!d->scaler)
    d->scaler = gavl_video_scaler_create();
  scaler_opt = gavl_video_scaler_get_options(d->scaler);
  gavl_video_options_copy(scaler_opt, &d->opt);
  
  gavl_video_format_copy(&in_format, &d->format);
  gavl_video_format_copy(&out_format, &d->format);
  
  if(in_format.interlace_mode == GAVL_INTERLACE_NONE)
    in_format.interlace_mode = GAVL_INTERLACE_TOP_FIRST;
  out_format.interlace_mode = GAVL_INTERLACE_NONE;
  
  gavl_video_scaler_init(d->scaler, 
                         &in_format,
                         &out_format);
  
  d->func = deinterlace_scale;
  return 1; 
  }
