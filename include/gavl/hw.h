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

#include <stdlib.h>


#include <gavl/gavldefs.h>
#include <gavl/compression.h>
#include <gavl/connectors.h>

/**
 * @file hw.h
 * external api header.
 */

/* Each context is either in map or in transfer modes */

typedef enum
  {
    /* Undefined / unsupported */
    GAVL_HW_FRAME_MODE_NONE      = 0,
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
#define GAVL_HW_SUPPORTS_AUDIO_MAP   (1<<1)
#define GAVL_HW_SUPPORTS_AUDIO_POOL  (1<<2)

#define GAVL_HW_SUPPORTS_VIDEO       (1<<8)
#define GAVL_HW_SUPPORTS_VIDEO_MAP   (1<<9)
#define GAVL_HW_SUPPORTS_VIDEO_POOL  (1<<10)

#define GAVL_HW_SUPPORTS_PACKET      (1<<16)
#define GAVL_HW_SUPPORTS_PACKET_MAP  (1<<17)
#define GAVL_HW_SUPPORTS_PACKET_POOL (1<<18)

/* Supports a pool shared among different processes */
#define GAVL_HW_SUPPORTS_SHARED_POOL (1<<24)

/* Buffer handles can be passed to other processes in terms of fds */
#define GAVL_HW_SUPPORTS_FDS         (1<<25)

typedef enum
  {
    GAVL_HW_NONE = 0,  // Frames in RAM
    GAVL_HW_EGL_GL   =     (1<<0), // OpenGL Texture in an EGL context
    GAVL_HW_EGL_GLES =     (1<<1), // OpenGL ES Texture in an EGL context
    
    GAVL_HW_VAAPI =        (1<<2),
    GAVL_HW_V4L2_BUFFER =  (1<<3), // V4L2 buffers (mmapped)
    GAVL_HW_DMABUFFER =    (1<<4), // DMA handles, can be exported by V4L and im- and exported by OpenGL
    GAVL_HW_MEMFD =        (1<<5), // Linux shared memory via memfd
  } gavl_hw_type_t;

#define GAVL_HW_BUFFER_WR (1<<0)

/*
   Plugins exchange information about supported buffer formats
   via dictionaries.
*/

#define GAVL_HW_BUF_TYPE         "type" // gavl_hw_type_t 
#define GAVL_HW_BUF_PIXELFORMAT  "pfmt"

/* fourcc for DMA buffer import (int array). */
#define GAVL_HW_BUF_DMA_FOURCC   "dmafourcc"


#define GAVL_HW_BUF_SAMPLEFORMAT     "sfmt"
#define GAVL_HW_BUF_SAMPLERATE       "srate"
#define GAVL_HW_BUF_INTERLEAVE_MODE  "interleave"
#define GAVL_HW_BUF_SHUFFLE          "shuffle" // int

#define GAVL_HW_BUF_SHARED       "shared" // int
#define GAVL_HW_BUF_MAX_W        "max_w" // int
#define GAVL_HW_BUF_MAX_H        "max_h" // int

/* Set by sinks, which support shuffled RGBA, which is *not* supported
   by gavl. E.g. the egl renderer can flip color channels via the colormatrix */
   
#define GAVL_HW_CAN_SHUFFLE_RGB  "can_shuffle_rgb" // int

/* Source -> sink */

#define GAVL_HW_MAX_FRAMES       "maxframes" 
#define GAVL_HW_SHMADDR          "shmaddr"


/* Write */
GAVL_PUBLIC 
gavl_dictionary_t * gavl_hw_buf_desc_append(gavl_array_t * arr, gavl_hw_type_t type);

/* Append integer format (e.g.  */

GAVL_PUBLIC
void gavl_hw_buf_desc_append_format(gavl_dictionary_t * dict,
                                    const char * key, int fmt);

GAVL_PUBLIC
void gavl_hw_buf_desc_set_max_size(gavl_dictionary_t * dict, int w, int h);

GAVL_PUBLIC
int gavl_hw_buf_desc_get_max_size(const gavl_dictionary_t * dict, int * w, int * h);

GAVL_PUBLIC
int gavl_hw_buf_desc_supports_format(const gavl_dictionary_t * dict,
                                     const char * key, int fmt);

GAVL_PUBLIC
int gavl_hw_buf_desc_can_create_audio(const gavl_array_t * arr,
                                      const gavl_audio_format_t * fmt);

GAVL_PUBLIC
gavl_hw_type_t gavl_hw_buf_desc_can_import_audio(const gavl_array_t * arr,
                                                 const gavl_audio_format_t * fmt);

/* Return the first array entry, which supports the format */

GAVL_PUBLIC const gavl_dictionary_t *
gavl_hw_buf_desc_supports_video_format(const gavl_array_t * arr,
                                       const gavl_video_format_t * fmt);

GAVL_PUBLIC const gavl_dictionary_t *
gavl_hw_buf_desc_supports_dma_fourcc(const gavl_array_t * arr, int fourcc);

GAVL_PUBLIC const gavl_dictionary_t *
gavl_hw_buf_desc_supports_audio_format(const gavl_array_t * arr,
                                       const gavl_audio_format_t * fmt);

/* Return the array entry, for which the conversion price is minimal */

GAVL_PUBLIC const gavl_dictionary_t *
gavl_hw_buf_desc_get_pixelformat_conversion(const gavl_array_t * arr,
                                            const gavl_video_format_t * fmt,
                                            gavl_pixelformat_t * pfmt,
                                            int * price);


/* */


typedef struct
  {
  int fd;                // File handle
  uint8_t * map_ptr;
  size_t map_len;
  size_t map_offset;
  int flags;            // If mapped writable
  } gavl_hw_buffer_t;

GAVL_PUBLIC void gavl_hw_buffer_init(gavl_hw_buffer_t * buf);
GAVL_PUBLIC int gavl_hw_buffer_map(gavl_hw_buffer_t * buf, int wr);
GAVL_PUBLIC int gavl_hw_buffer_unmap(gavl_hw_buffer_t * buf);
GAVL_PUBLIC int gavl_hw_buffer_cleanup(gavl_hw_buffer_t * buf);

GAVL_PUBLIC int gavl_hw_supported(gavl_hw_type_t type);

GAVL_PUBLIC const char * gavl_hw_type_to_string(gavl_hw_type_t type);

GAVL_PUBLIC const char * gavl_hw_type_to_id(gavl_hw_type_t type);
GAVL_PUBLIC gavl_hw_type_t gavl_hw_type_from_id(const char * id);

GAVL_PUBLIC int gavl_hw_ctx_can_transfer(gavl_hw_context_t * from, gavl_hw_context_t * to);

GAVL_PUBLIC const gavl_dictionary_t *
gavl_hw_ctx_can_export(gavl_hw_context_t * ctx, const gavl_array_t * to);

GAVL_PUBLIC 
int gavl_hw_ctx_can_import(gavl_hw_context_t * ctx, const gavl_hw_context_t * from);

/*
 *  Set maximum number of frames to create.
 *  - Makes sense only if we are the one, who creates the frames
 *  - Must be called before frames are created
 */

GAVL_PUBLIC void gavl_hw_ctx_set_max_frames(gavl_hw_context_t * ctx, int num);

/*
 *  Put the reference table into shared memory so other processes can
 *  use them
 */

GAVL_PUBLIC int gavl_hw_ctx_set_shared(gavl_hw_context_t * ctx);

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

/*
 *  Import a video frame from another hardware context.
 *
 *  If *dst == NULL, we enable frame pool mode.
 *
 *  if *dst != NULL, fmt must specify the video format
 */ 

int gavl_hw_ctx_import_video_frame(gavl_video_frame_t * src,
                                   gavl_video_frame_t ** dst,
                                   gavl_hw_context_t * dst_ctx,
                                   const gavl_video_format_t * fmt);

int gavl_hw_ctx_export_video_frame(gavl_video_frame_t * src,
                                   gavl_video_frame_t ** dst,
                                   gavl_hw_context_t * dst_ctx,
                                   const gavl_video_format_t * fmt);


GAVL_PUBLIC 
gavl_video_frame_t * gavl_hw_ctx_get_imported_vframe(gavl_hw_context_t * ctx,
                                                     int buf_idx);

GAVL_PUBLIC gavl_video_frame_t *
gavl_hw_ctx_create_import_vframe(gavl_hw_context_t * ctx, int buf_idx);


GAVL_PUBLIC gavl_hw_type_t gavl_hw_ctx_get_type(const gavl_hw_context_t * ctx);


/* Format will be adjusted if it's not supported by the hardware */

// GAVL_PUBLIC void gavl_hw_ctx_set_video(gavl_hw_context_t * ctx, gavl_video_format_t * fmt, gavl_hw_frame_mode_t mode);

GAVL_PUBLIC void gavl_hw_ctx_set_video_creator(gavl_hw_context_t * ctx, gavl_video_format_t * fmt,
                                               gavl_hw_frame_mode_t mode);

/* Configure ctx as an importer for frames from ctx_src */

GAVL_PUBLIC void gavl_hw_ctx_set_video_importer(gavl_hw_context_t * ctx,
                                                gavl_hw_context_t * ctx_src,
                                                gavl_video_format_t * vfmt);


GAVL_PUBLIC void gavl_hw_ctx_set_audio_creator(gavl_hw_context_t * ctx, gavl_audio_format_t * fmt,
                                               gavl_hw_frame_mode_t mode);

GAVL_PUBLIC void gavl_hw_ctx_set_audio_importer(gavl_hw_context_t * ctx,
                                                gavl_hw_context_t * ctx_src,
                                                gavl_audio_format_t * fmt);

GAVL_PUBLIC void gavl_hw_ctx_set_packet_creator(gavl_hw_context_t * ctx, int max_size);
GAVL_PUBLIC void gavl_hw_ctx_set_packet_importer(gavl_hw_context_t * ctx);


/*
 *  Create a video frame. The frame will be a reference for a hardware surface.
 *  If alloc_resource is 1, an actual hardware frame is created. Else, just
 *  the storage pointer is allocated for importing hardware surfaces created elsewhere
 */

GAVL_PUBLIC gavl_packet_t *
gavl_hw_packet_create(gavl_hw_context_t * ctx, int alloc_resource);

GAVL_PUBLIC int
gavl_hw_packet_map(gavl_packet_t * p, int wr);

GAVL_PUBLIC int
gavl_hw_packet_unmap(gavl_packet_t * p);


GAVL_PUBLIC gavl_audio_frame_t *
gavl_hw_audio_frame_create(gavl_hw_context_t * ctx, int alloc_resource);

GAVL_PUBLIC int
gavl_hw_audio_frame_map(gavl_audio_frame_t * frame, int wr);

GAVL_PUBLIC int
gavl_hw_audio_frame_unmap(gavl_audio_frame_t * frame);


GAVL_PUBLIC gavl_video_frame_t *
gavl_hw_video_frame_create(gavl_hw_context_t * ctx, int alloc_resource);

GAVL_PUBLIC int
gavl_hw_video_frame_map(gavl_video_frame_t * frame, int wr);

GAVL_PUBLIC int
gavl_hw_video_frame_unmap(gavl_video_frame_t * frame);

/* Get a frame for writing */
// GAVL_PUBLIC gavl_video_frame_t *
// gavl_hw_video_frame_get(gavl_hw_context_t * ctx);


GAVL_PUBLIC gavl_video_frame_t *
gavl_hw_video_frame_get_write(gavl_hw_context_t * ctx);

GAVL_PUBLIC gavl_audio_frame_t *
gavl_hw_audio_frame_get_write(gavl_hw_context_t * ctx);

GAVL_PUBLIC gavl_packet_t *
gavl_hw_packet_get_write(gavl_hw_context_t * ctx);


GAVL_PUBLIC void gavl_hw_video_frame_ref(gavl_video_frame_t*);
GAVL_PUBLIC void gavl_hw_video_frame_unref(gavl_video_frame_t*);
GAVL_PUBLIC int gavl_hw_video_frame_refcount(gavl_video_frame_t*);

GAVL_PUBLIC void gavl_hw_audio_frame_ref(gavl_audio_frame_t*);
GAVL_PUBLIC void gavl_hw_audio_frame_unref(gavl_audio_frame_t*);
GAVL_PUBLIC int gavl_hw_audio_frame_refcount(gavl_audio_frame_t*);

GAVL_PUBLIC void gavl_hw_packet_ref(gavl_packet_t*);
GAVL_PUBLIC void gavl_hw_packet_unref(gavl_packet_t*);
GAVL_PUBLIC int gavl_hw_packet_refcount(gavl_packet_t*);


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

GAVL_PUBLIC 
gavl_hw_context_t * gavl_hw_ctx_create_from_buffer_format(const gavl_dictionary_t * dict);


GAVL_PUBLIC const gavl_array_t * gavl_hw_ctx_get_import_formats(const gavl_hw_context_t *);
GAVL_PUBLIC const gavl_array_t * gavl_hw_ctx_get_transfer_formats(const gavl_hw_context_t *);
GAVL_PUBLIC const gavl_array_t * gavl_hw_ctx_get_map_formats(const gavl_hw_context_t *);

GAVL_PUBLIC void gavl_hw_ctx_set_transfer_formats(gavl_hw_context_t *, const gavl_array_t *);
GAVL_PUBLIC void gavl_hw_ctx_set_map_formats(gavl_hw_context_t *, const gavl_array_t *);

GAVL_PUBLIC void gavl_hw_buffer_format_dump(const gavl_dictionary_t * dict);
GAVL_PUBLIC void gavl_hw_buffer_formats_dump(const gavl_array_t * arr);



#endif // GAVL_HW_H_INCLUDED
