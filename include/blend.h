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

#ifndef BLEND_H_INCLUDED
#define BLEND_H_INCLUDED


#include <gavl/connectors.h>

typedef void (*gavl_blend_func_t)(gavl_overlay_blend_context_t * ctx,
                                  gavl_video_frame_t * frame,
                                  gavl_video_frame_t * overlay);

struct gavl_overlay_blend_context_s
  {
  gavl_video_format_t dst_format;
  gavl_video_format_t ovl_format;
  gavl_blend_func_t func;

  gavl_overlay_t * ovl;
  
  gavl_video_frame_t * ovl_win;
  gavl_video_frame_t * dst_win;

  gavl_rectangle_i_t dst_rect;
    
  gavl_video_options_t opt;
  
  /* Chroma subsampling of the destination format */  
  int dst_sub_h, dst_sub_v;
  
  gavl_video_sink_t * sink;
  };

gavl_blend_func_t
gavl_find_blend_func_c(gavl_overlay_blend_context_t * ctx,
                       gavl_pixelformat_t frame_format,
                       gavl_pixelformat_t * overlay_format);

                       
#endif // BLEND_H_INCLUDED
