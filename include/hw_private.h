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

#ifndef HW_PRIVATE_H_INCLUDED
#define HW_PRIVATE_H_INCLUDED

#include <gavl/gavl.h> // Includes hw.h
#include <gavl/compression.h>

/* Functions */
typedef struct
  {
  void (*destroy_native)(void * native);

  gavl_pixelformat_t * (*get_image_formats)(gavl_hw_context_t * ctx);
  gavl_pixelformat_t * (*get_overlay_formats)(gavl_hw_context_t * ctx);
  
  void (*video_format_adjust)(gavl_hw_context_t * ctx,
                              gavl_video_format_t * fmt);

  void (*overlay_format_adjust)(gavl_hw_context_t * ctx,
                                gavl_video_format_t * fmt);

  
  gavl_video_frame_t *  (*video_frame_create_hw)(gavl_hw_context_t * ctx,
                                                 gavl_video_format_t * fmt);

  gavl_video_frame_t *  (*video_frame_create_ram)(gavl_hw_context_t * ctx,
                                                  gavl_video_format_t * fmt);

  gavl_video_frame_t *  (*video_frame_create_ovl)(gavl_hw_context_t * ctx,
                                                  gavl_video_format_t * fmt);

  void (*video_frame_destroy)(gavl_video_frame_t * f);
  
  int (*video_frame_to_ram)(const gavl_video_format_t * fmt,
                            gavl_video_frame_t * dst,
                            gavl_video_frame_t * src);

  int (*video_frame_to_hw)(const gavl_video_format_t * fmt,
                           gavl_video_frame_t * dst,
                           gavl_video_frame_t * src);

  int (*video_frame_map)(const gavl_video_format_t * fmt,
                         gavl_video_frame_t * f);

  int (*video_frame_unmap)(const gavl_video_format_t * fmt,
                           gavl_video_frame_t * f);

  void (*video_frame_to_packet)(gavl_hw_context_t * ctx,
                                const gavl_video_format_t * fmt,
                                gavl_video_frame_t * frame,
                                gavl_packet_t * p);

  void (*video_frame_from_packet)(gavl_hw_context_t * ctx,
                                  const gavl_video_format_t * fmt,
                                  gavl_packet_t * p,
                                  gavl_video_frame_t * frame);

  int (*can_import)(gavl_hw_context_t * ctx, gavl_hw_type_t t);
  int (*can_export)(gavl_hw_context_t * ctx, gavl_hw_type_t t);

  int (*import_video_frame)(gavl_hw_context_t * ctx, gavl_video_format_t * fmt,
                            gavl_video_frame_t * src, gavl_video_frame_t * dst);

  int (*export_video_frame)(gavl_hw_context_t * ctx, gavl_video_format_t * fmt,
                            gavl_video_frame_t * src, gavl_video_frame_t * dst);
  
  } gavl_hw_funcs_t;

struct gavl_hw_context_s
  {
  void * native;
  gavl_hw_type_t type;
  const gavl_hw_funcs_t * funcs;
  gavl_pixelformat_t * image_formats;
  gavl_pixelformat_t * overlay_formats;
  
  int support_flags;

  gavl_video_frame_t ** imported_vframes;
  int imported_vframes_alloc;
  
  };



gavl_hw_context_t *
gavl_hw_context_create_internal(void * native,
                                const gavl_hw_funcs_t * funcs,
                                gavl_hw_type_t type, int support_flags);

void 
gavl_hw_destroy_video_frame(gavl_hw_context_t * ctx,
                            gavl_video_frame_t * frame);
#endif
