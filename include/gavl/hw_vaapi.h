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



#ifndef GAVL_HW_VAAPI_H_INCLUDED
#define GAVL_HW_VAAPI_H_INCLUDED

#include <va/va.h>

typedef struct
  {
  VASurfaceID surface;
  VAImage image; /* Only valid while the frame is mapped */
  int wr;
  } gavl_vaapi_video_frame_t;

/* VA-API specific backend functions */

GAVL_PUBLIC VAImageID   gavl_vaapi_get_image_id(const gavl_video_frame_t *);

GAVL_PUBLIC VASurfaceID gavl_vaapi_get_surface_id(const gavl_video_frame_t *);

/* Use only in specific create routines */
GAVL_PUBLIC void gavl_vaapi_set_surface_id(gavl_video_frame_t *, VASurfaceID id);

/* Map into userspace for CPU access */
GAVL_PUBLIC int gavl_vaapi_map_frame(gavl_video_frame_t *, int wr);

/* Unmap */
GAVL_PUBLIC int gavl_vaapi_unmap_frame(gavl_video_frame_t *);


GAVL_PUBLIC VAProfile gavl_vaapi_get_profile(const gavl_dictionary_t * dict);

GAVL_PUBLIC int gavl_vaapi_can_decode(VADisplay dpy, const gavl_dictionary_t * dict);

// GAVL_PUBLIC gavl_video_frame_t *
// gavl_vaapi_import_surface(gavl_hw_context_t * ctx, VASurfaceID surf);

GAVL_PUBLIC gavl_hw_context_t * gavl_hw_ctx_create_vaapi();
GAVL_PUBLIC VADisplay gavl_hw_ctx_vaapi_get_va_display(gavl_hw_context_t *);

#endif // GAVL_HW_VAAPI_H_INCLUDED
