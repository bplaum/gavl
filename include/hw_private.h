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

#ifndef HW_PRIVATE_H_INCLUDED
#define HW_PRIVATE_H_INCLUDED

#include <config.h>
#include <gavl/gavl.h> // Includes hw.h
#include <gavl/compression.h>

#ifdef HAVE_DRM_DRM_FOURCC_H
#include <drm/drm_fourcc.h>
#else
#ifdef HAVE_LIBDRM_RM_FOURCC_H
#include <libdrm/drm_fourcc.h>
#endif

#endif


/* Functions */
typedef struct
  {
  void (*destroy_native)(void * native);

  gavl_pixelformat_t * (*get_image_formats)(gavl_hw_context_t * ctx, gavl_hw_frame_mode_t mode);
  
  void (*video_format_adjust)(gavl_hw_context_t * ctx,
                              gavl_video_format_t * fmt, gavl_hw_frame_mode_t mode);

  gavl_video_frame_t *  (*video_frame_create)(gavl_hw_context_t * ctx, int alloc_resource);
  
  /* Map into / unmap from address space */
  int (*video_frame_map)(gavl_video_frame_t *, int wr);
  int (*video_frame_unmap)(gavl_video_frame_t *);
  
  void (*video_frame_destroy)(gavl_video_frame_t * f, int destroy_resource);
  
  int (*video_frame_to_ram)(gavl_video_frame_t * dst, gavl_video_frame_t * src);
  int (*video_frame_to_hw)(gavl_video_frame_t * dst, gavl_video_frame_t * src);
  
  int (*write_video_frame)(gavl_hw_context_t * ctx, gavl_io_t * io, const gavl_video_frame_t * f);
  int (*read_video_frame)(gavl_hw_context_t * ctx, gavl_io_t * io, gavl_video_frame_t * f);
  
  int (*can_import)(gavl_hw_context_t * ctx, const gavl_hw_context_t * from);
  int (*can_export)(gavl_hw_context_t * ctx, const gavl_hw_context_t * to);
  
  int (*import_video_frame)(const gavl_video_format_t * fmt, gavl_video_frame_t * src, gavl_video_frame_t * dst);
  int (*export_video_frame)(const gavl_video_format_t * fmt, gavl_video_frame_t * src, gavl_video_frame_t * dst);
  
  } gavl_hw_funcs_t;


typedef struct
  {
  void ** frames;
  int num_frames;
  int frames_alloc;
  } frame_pool_t;

int gavl_hw_frame_pool_add(frame_pool_t *, void * frame, int idx);
void gavl_hw_frame_pool_reset(frame_pool_t *);

struct gavl_hw_context_s
  {
  void * native;
  gavl_hw_type_t type;
  const gavl_hw_funcs_t * funcs;

  gavl_pixelformat_t * image_formats_map;
  gavl_pixelformat_t * image_formats_transfer;
  
  int support_flags;
  
  frame_pool_t created;
  frame_pool_t imported;
  
  gavl_video_format_t vfmt;
  
  gavl_hw_frame_mode_t mode;
  };

gavl_hw_context_t *
gavl_hw_context_create_internal(void * native,
                                const gavl_hw_funcs_t * funcs,
                                gavl_hw_type_t type, int support_flags);

void 
gavl_hw_destroy_video_frame(gavl_hw_context_t * ctx,
                            gavl_video_frame_t * frame, int destroy_resource);

gavl_video_frame_t *
gavl_hw_ctx_create_import_vframe(gavl_hw_context_t * ctx,
                                 int buf_idx);


#endif
