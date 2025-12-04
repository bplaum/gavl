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



#ifndef GAVL_HW_H_INCLUDED
#define GAVL_HW_H_INCLUDED

#include <gavl/gavldefs.h>
#include <gavl/compression.h>

/**
 * @file hw.h
 * external api header.
 */

/* Each context is either in map or in transfer modes */

typedef enum
  {
    /* Make a frame pool with mappable frames. Use this for
       sources, which write into user supplied memory */
    GAVL_HW_FRAME_MODE_MAP      = 1,

    /* Transfer frames from RAM to the hardware or back */
    GAVL_HW_FRAME_MODE_TRANSFER = 2,

    /* Import frames from another context */
    GAVL_HW_FRAME_MODE_IMPORT   = 3,
  } gavl_hw_frame_mode_t;

/* Support flags for hardware contexts */

#define GAVL_HW_SUPPORTS_AUDIO       (1<<0)
#define GAVL_HW_SUPPORTS_VIDEO       (1<<1)
#define GAVL_HW_SUPPORTS_VIDEO_MAP   (1<<3)
#define GAVL_HW_SUPPORTS_VIDEO_IO    (1<<4)
#define GAVL_HW_SUPPORTS_VIDEO_POOL  (1<<5)

typedef enum
  {
    GAVL_HW_NONE = 0,  // Frames in RAM
    GAVL_HW_EGL_GL   =     (1<<0), // OpenGL Texture in an EGL context
    GAVL_HW_EGL_GLES =     (1<<1), // OpenGL ES Texture in an EGL context
    
    GAVL_HW_VAAPI =        (1<<2),
    GAVL_HW_V4L2_BUFFER =  (1<<3), // V4L2 buffers (mmapped)
    GAVL_HW_DMABUFFER =    (1<<4), // DMA handles, can be exported by V4L and im- and exported by OpenGL
    GAVL_HW_SHM =          (1<<5), // Shared memory, which can be sent to other processes
  } gavl_hw_type_t;


/* Global handle for accessing a piece of hardware */
typedef struct gavl_hw_context_s gavl_hw_context_t;

GAVL_PUBLIC int gavl_hw_supported(gavl_hw_type_t type);

GAVL_PUBLIC const char * gavl_hw_type_to_string(gavl_hw_type_t type);

GAVL_PUBLIC const char * gavl_hw_type_to_id(gavl_hw_type_t type);
GAVL_PUBLIC gavl_hw_type_t gavl_hw_type_from_id(const char * id);

GAVL_PUBLIC int gavl_hw_ctx_can_export(gavl_hw_context_t * ctx, const gavl_hw_context_t * to);
GAVL_PUBLIC int gavl_hw_ctx_can_import(gavl_hw_context_t * ctx, const gavl_hw_context_t * from);


GAVL_PUBLIC void gavl_hw_ctx_destroy(gavl_hw_context_t * ctx);

/* Reset to the initial state (after creating) */
GAVL_PUBLIC void gavl_hw_ctx_reset(gavl_hw_context_t * ctx);

/* Do a post-seek resync. This will destroy all imported frames, because
   an underlying hardware decoder might got restarted */
GAVL_PUBLIC void gavl_hw_ctx_resync(gavl_hw_context_t * ctx);

GAVL_PUBLIC int gavl_hw_ctx_get_support_flags(gavl_hw_context_t * ctx);

/*
 * If *frame2 == NULL, a frame is allocated in the
 * imported pool and owned by the hw context.
 *
 *  
 */

GAVL_PUBLIC int gavl_hw_ctx_transfer_video_frame(gavl_video_frame_t * frame1,
                                                 gavl_hw_context_t * ctx2,
                                                 gavl_video_frame_t ** frame2,
                                                 const gavl_video_format_t * fmt);

GAVL_PUBLIC 
gavl_video_frame_t * gavl_hw_ctx_get_imported_vframe(gavl_hw_context_t * ctx,
                                                     int buf_idx);

GAVL_PUBLIC gavl_video_frame_t *
gavl_hw_ctx_create_import_vframe(gavl_hw_context_t * ctx, int buf_idx);


GAVL_PUBLIC const gavl_pixelformat_t *
gavl_hw_ctx_get_image_formats(gavl_hw_context_t * ctx, gavl_hw_frame_mode_t mode);

GAVL_PUBLIC gavl_hw_type_t gavl_hw_ctx_get_type(const gavl_hw_context_t * ctx);

GAVL_PUBLIC void gavl_hw_video_format_adjust(gavl_hw_context_t * ctx,
                                             gavl_video_format_t * fmt,
                                             gavl_hw_frame_mode_t mode);

/* Format will be adjusted if it's not supported by the hardware */

//GAVL_PUBLIC void gavl_hw_ctx_set_audio(gavl_hw_context_t * ctx, gavl_audio_format_t * fmt, gavl_hw_frame_mode_t mode);
GAVL_PUBLIC void gavl_hw_ctx_set_video(gavl_hw_context_t * ctx, gavl_video_format_t * fmt, gavl_hw_frame_mode_t mode);

/*
 *  Create a video frame. The frame will be a reference for a hardware surface.
 *  If alloc_resource is 1, an actual hardware frame is created. Else, just
 *  the storage pointer is allocated for importing hardware surfaces created elsewhere
 */

GAVL_PUBLIC gavl_video_frame_t *
gavl_hw_video_frame_create(gavl_hw_context_t * ctx, int alloc_resource);

GAVL_PUBLIC int
gavl_hw_video_frame_map(gavl_video_frame_t * frame, int wr);

GAVL_PUBLIC int
gavl_hw_video_frame_unmap(gavl_video_frame_t * frame);

/* Get a frame for writing (this enables the frame pool) */
GAVL_PUBLIC gavl_video_frame_t *
gavl_hw_video_frame_get(gavl_hw_context_t * ctx);

GAVL_PUBLIC void gavl_hw_video_frame_ref(gavl_video_frame_t*);
GAVL_PUBLIC void gavl_hw_video_frame_unref(gavl_video_frame_t*);
GAVL_PUBLIC int gavl_hw_video_frame_refcount(gavl_video_frame_t*);


/* Load a video frame from RAM into the hardware */
GAVL_PUBLIC int gavl_video_frame_ram_to_hw(gavl_video_frame_t * dst,
                                           gavl_video_frame_t * src);

/* Load a video frame from the hardware into RAM */
GAVL_PUBLIC int gavl_video_frame_hw_to_ram(gavl_video_frame_t * dst,
                                           gavl_video_frame_t * src);

GAVL_PUBLIC int gavl_video_frame_hw_can_transfer(gavl_hw_context_t * from,
                                                 gavl_hw_context_t * to);

/* Store in a stream structure */
GAVL_PUBLIC void
gavl_hw_ctx_store(gavl_hw_context_t * ctx, gavl_dictionary_t * dict);

GAVL_PUBLIC gavl_hw_context_t * gavl_hw_ctx_load(const gavl_dictionary_t * dict);

/* Create a HW context with just a type. This function succeeds only for types,
   which don't require other stuff (e.g. device nodes) for creation */

GAVL_PUBLIC gavl_hw_context_t * gavl_hw_ctx_create(gavl_hw_type_t type);

#endif // GAVL_HW_H_INCLUDED
