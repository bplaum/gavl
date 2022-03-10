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

#ifndef GAVL_HW_H_INCLUDED
#define GAVL_HW_H_INCLUDED

#include <gavl/gavldefs.h>
#include <gavl/compression.h>

/**
 * @file hw.h
 * external api header.
 */

/* Support flags for hardware contexts */

#define GAVL_HW_SUPPORTS_AUDIO       (1<<0)
#define GAVL_HW_SUPPORTS_VIDEO       (1<<1)
#define GAVL_HW_SUPPORTS_PACKETS     (1<<2)
#define GAVL_HW_SUPPORTS_SHARED      (1<<3) // Data can be shared among processes
// #define GAVL_HW_SUPPORTS_DMA_EXPORT  (1<<4) // Buffer can be exported as Linux DMA buffer
// #define GAVL_HW_SUPPORTS_DMA_IMPORT  (1<<5) // Buffer can be imported as Linux DMA buffer

typedef enum
  {
    GAVL_HW_NONE = 0,  // Frames in RAM
    GAVL_HW_EGL_GL_X11,     // EGL Texture (associated with X11 connection)
    GAVL_HW_EGL_GLES_X11,   // EGL Texture (associated with X11 connection)
    // GAVL_HW_EGL_WAYLAND, // EGL Texture (wayland) Not implemented yet
    GAVL_HW_VAAPI_X11,
    GAVL_HW_V4L2_BUFFER, // V4L2 buffers (mmapped)
    GAVL_HW_DMABUFFER,   // DMA handles, can be exported by V4L and im- and exported by OpenGL
    GAVL_HW_SHM,         // Shared memory, which can be sent to other processes
  } gavl_hw_type_t;

/* Global handle for accessing a piece of hardware */
typedef struct gavl_hw_context_s gavl_hw_context_t;

GAVL_PUBLIC int gavl_hw_supported(gavl_hw_type_t type);

GAVL_PUBLIC const char * gavl_hw_type_to_string(gavl_hw_type_t type);

GAVL_PUBLIC int gavl_hw_ctx_exports_type(gavl_hw_context_t * ctx, gavl_hw_type_t type);
GAVL_PUBLIC int gavl_hw_ctx_imports_type(gavl_hw_context_t * ctx, gavl_hw_type_t type);

GAVL_PUBLIC void gavl_hw_ctx_destroy(gavl_hw_context_t * ctx);

GAVL_PUBLIC int gavl_hw_ctx_get_support_flags(gavl_hw_context_t * ctx);

GAVL_PUBLIC int gavl_hw_ctx_transfer_video_frame(gavl_video_frame_t * frame1,
                                                 gavl_hw_context_t * ctx2,
                                                 gavl_video_frame_t ** frame2,
                                                 gavl_video_format_t * fmt);


/* Returned array must be free()d */
GAVL_PUBLIC gavl_pixelformat_t *
gavl_hw_ctx_get_image_formats(gavl_hw_context_t * ctx);

/* Returned array must be free()d */
GAVL_PUBLIC gavl_pixelformat_t *
gavl_hw_ctx_get_overlay_formats(gavl_hw_context_t * ctx);

GAVL_PUBLIC gavl_hw_type_t gavl_hw_ctx_get_type(gavl_hw_context_t * ctx);

/* Format will be adjusted if it's not supported by the hardware */
GAVL_PUBLIC void gavl_hw_video_format_adjust(gavl_hw_context_t * ctx,
                                             gavl_video_format_t * fmt);

GAVL_PUBLIC void gavl_hw_overlay_format_adjust(gavl_hw_context_t * ctx,
                                               gavl_video_format_t * fmt);


/* Create a video frame. The frame will be a reference for a hardware surface */
GAVL_PUBLIC gavl_video_frame_t * gavl_hw_video_frame_create_hw(gavl_hw_context_t * ctx,
                                                               gavl_video_format_t * fmt);

/* Create a video frame. The frame will have data available for CPU access but is
 suitable for transfer to a hardware surface */
GAVL_PUBLIC gavl_video_frame_t * gavl_hw_video_frame_create_ram(gavl_hw_context_t * ctx,
                                                                gavl_video_format_t * fmt);

/* Create a video frame for use as an overlay */
GAVL_PUBLIC gavl_video_frame_t * gavl_hw_video_frame_create_ovl(gavl_hw_context_t * ctx,
                                                                gavl_video_format_t * fmt);

/* Load a video frame from RAM into the hardware */
GAVL_PUBLIC int gavl_video_frame_ram_to_hw(const gavl_video_format_t * fmt,
                                           gavl_video_frame_t * dst,
                                           gavl_video_frame_t * src);

/* Load a video frame from the hardware into RAM */
GAVL_PUBLIC int gavl_video_frame_hw_to_ram(const gavl_video_format_t * fmt,
                                           gavl_video_frame_t * dst,
                                           gavl_video_frame_t * src);

GAVL_PUBLIC int gavl_video_frame_hw_to_packet(gavl_hw_context_t * ctx,
                                              const gavl_video_format_t * fmt,
                                              const gavl_video_frame_t * src,
                                              gavl_packet_t * p);

GAVL_PUBLIC gavl_video_frame_t * gavl_video_frame_hw_from_packet(gavl_hw_context_t * ctx,
                                                                 const gavl_video_format_t * fmt,
                                                                 const gavl_packet_t * src);


GAVL_PUBLIC int gavl_video_frame_hw_can_transfer(gavl_hw_context_t * from,
                                                 gavl_hw_context_t * to);

#endif // GAVL_HW_H_INCLUDED
