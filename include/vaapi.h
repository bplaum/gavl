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



#ifndef VAAPI_H_INCLUDED
#define VAAPI_H_INCLUDED


#include <va/va.h>
#include <hw_private.h>

#include <gavl/gavldsp.h>

typedef struct
  {
  VADisplay dpy; // Must be first

  int num_image_formats;
  VAImageFormat * image_formats;
  
  int no_derive;

  int num_imported_frames;
  
  } gavl_hw_vaapi_t;


gavl_video_frame_t * gavl_vaapi_video_frame_create(gavl_hw_context_t * ctx, int alloc_resource);


void gavl_vaapi_video_frame_destroy(gavl_video_frame_t * f, int destroy_resource);

gavl_pixelformat_t *
gavl_vaapi_get_image_formats(gavl_hw_context_t * ctx, gavl_hw_frame_mode_t mode);

void gavl_vaapi_cleanup(void * priv);

void gavl_vaapi_video_format_adjust(gavl_hw_context_t * ctx,
                                    gavl_video_format_t * fmt, gavl_hw_frame_mode_t mode);

int gavl_vaapi_export_video_frame(const gavl_video_format_t * fmt,
                                  gavl_video_frame_t * src, gavl_video_frame_t * dst);

int gavl_vaapi_exports_type(gavl_hw_context_t * ctx, const gavl_hw_context_t * other);



#endif // VAAPI_H_INCLUDED
