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

#include <pthread.h>
#include <semaphore.h>

#include <stdatomic.h>


#include <config.h>
#include <gavl/gavl.h>
#include <gavl/compression.h>

#include <gavl/hw.h>


#ifdef HAVE_DRM
#include <drm_fourcc.h>
#endif

#define HW_CTX_FLAG_SHARED_REFTABLE (1<<0)
#define HW_CTX_FLAG_CREATOR         (1<<1)
#define HW_CTX_FLAG_IMPORTER        (1<<2)

#define HW_CTX_FLAG_AUDIO           (1<<3)
#define HW_CTX_FLAG_VIDEO           (1<<4)
#define HW_CTX_FLAG_PACKET          (1<<5)
#define HW_CTX_MODE_MASK    (HW_CTX_FLAG_AUDIO | HW_CTX_FLAG_VIDEO | HW_CTX_FLAG_PACKET)

/* Frame was already written, transmit just the buffer index from now on */
#define HW_FRAME_WRITTEN            (1<<0)

/* Functions */
typedef struct
  {
  void (*destroy_native)(void * native);
  const gavl_dictionary_t * (*can_export)(gavl_hw_context_t * ctx, const gavl_array_t * arr);
  
  /* Video stuff */
    
  //  gavl_pixelformat_t * (*get_image_formats)(gavl_hw_context_t * ctx, gavl_hw_frame_mode_t mode);
  
  void (*video_format_adjust)(gavl_hw_context_t * ctx,
                              gavl_video_format_t * fmt, gavl_hw_frame_mode_t mode);

  void (*audio_format_adjust)(gavl_hw_context_t * ctx,
                              gavl_audio_format_t * fmt, gavl_hw_frame_mode_t mode);
  
  gavl_video_frame_t *  (*video_frame_create)(gavl_hw_context_t * ctx, int alloc_resource);

  /* Map into / unmap from address space */
  int (*video_frame_map)(gavl_video_frame_t *, int wr);
  int (*video_frame_unmap)(gavl_video_frame_t *);
  
  void (*video_frame_destroy)(gavl_video_frame_t * f, int destroy_resource);
  
  int (*video_frame_to_ram)(gavl_video_frame_t * dst, gavl_video_frame_t * src);
  int (*video_frame_to_hw)(gavl_video_frame_t * dst, gavl_video_frame_t * src);
  
  
  int (*import_video_frame)(const gavl_video_format_t * fmt, gavl_video_frame_t * src, gavl_video_frame_t * dst);
  int (*export_video_frame)(const gavl_video_format_t * fmt, gavl_video_frame_t * src, gavl_video_frame_t * dst);

  /* Audio */


  gavl_audio_frame_t *  (*audio_frame_create)(gavl_hw_context_t * ctx, int alloc_resource);
  
  int (*audio_frame_map)(gavl_audio_frame_t *, int wr);
  int (*audio_frame_unmap)(gavl_audio_frame_t *);
  
  void (*audio_frame_destroy)(gavl_audio_frame_t * f, int destroy_resource);

  /* For un-mappable frames */
  //  int (*audio_frame_to_ram)(gavl_video_frame_t * dst, gavl_video_frame_t * src);
  //  int (*audio_frame_to_hw)(gavl_video_frame_t * dst, gavl_video_frame_t * src);
  
  int (*import_audio_frame)(const gavl_audio_format_t * fmt, gavl_audio_frame_t * src, gavl_audio_frame_t * dst);
  int (*export_audio_frame)(const gavl_audio_format_t * fmt, gavl_audio_frame_t * src, gavl_audio_frame_t * dst);

  /*
    Packet
   */
  
  void (*packet_destroy)(gavl_packet_t * p, int destroy_resource);
  gavl_packet_t *  (*packet_create)(gavl_hw_context_t * ctx, int alloc_resource);
  int (*packet_map)(gavl_packet_t *, int wr);
  int (*packet_unmap)(gavl_packet_t *);
  
  /* I/O */
  gavl_source_status_t (*read_audio_frame)(gavl_hw_context_t * hwctx, gavl_io_t * io, gavl_audio_frame_t * f);
  gavl_sink_status_t (*write_audio_frame)(gavl_hw_context_t * hwctx, gavl_io_t * io, gavl_audio_frame_t * f);

  gavl_source_status_t (*read_video_frame)(gavl_hw_context_t * hwctx, gavl_io_t * io, gavl_video_frame_t * f);
  gavl_sink_status_t (*write_video_frame)(gavl_hw_context_t * hwctx, gavl_io_t * io, gavl_video_frame_t * f);

  gavl_source_status_t (*read_packet)(gavl_hw_context_t * hwctx, gavl_io_t * io, gavl_packet_t * p);
  gavl_sink_status_t (*write_packet)(gavl_hw_context_t * hwctx, gavl_io_t * io, gavl_packet_t * p);
  
  } gavl_hw_funcs_t;

/*
 *  Reference table: This contains the refcounts of the frames. It can be
 *  in shared memory so frame buffers can be shared among processes
 *
 *  The buffers themselves must be transferred in terms of filedescriptors
 *  (for memfds for dma-buffers) via a different channel.
 */

typedef struct
  {
  atomic_int refcount;
  } reftable_frame_t;

typedef struct
  {
  /* Number of free buffers is a semaphore so we can
     wait for it to be nonzero */
  
  sem_t free_buffers;
  
  /* TODO: Add atomics for a ringbuffer */
  //  int read_pos;
  //  int write_pos;
  
  reftable_frame_t frames[];
  } reftable_t;

char * gavl_hw_reftable_create_name(void);

reftable_t * gavl_hw_reftable_create_local(gavl_hw_context_t * ctx);
reftable_t * gavl_hw_reftable_create_shared(gavl_hw_context_t * ctx);
reftable_t * gavl_hw_reftable_create_remote(gavl_hw_context_t * ctx);

void gavl_hw_reftable_destroy(gavl_hw_context_t * ctx);

// int gavl_hw_reftable_get_free(reftable_t *);



/* Context */

typedef struct
  {
  void * frame;
  int flags;
  } frame_t;


struct gavl_hw_context_s
  {
  void * native;
  gavl_hw_type_t type;
  const gavl_hw_funcs_t * funcs;

  /* Context we import the frames from */
  gavl_hw_context_t * ctx_src;
  
  int flags;
  
  //  gavl_pixelformat_t * image_formats_map;
  //  gavl_pixelformat_t * image_formats_transfer;
  
  int support_flags;
  
  gavl_video_format_t vfmt;
  gavl_audio_format_t afmt;
  
  gavl_hw_frame_mode_t mode;

  /* Frame pool stuff */
  
  frame_t * frames;
  int num_frames;
  int max_frames;

  /* Size to allocate for packets */
  int packet_size;
  
  /* Reference table */
  
  int shm_fd; /* Returned by shm_open() */
  char * shm_name;
  
  reftable_t * reftab;
  reftable_t * reftab_priv;

  gavl_array_t import_formats;
  gavl_array_t transfer_formats;
  gavl_array_t map_formats;
  
  };

int gavl_hw_frame_pool_add(gavl_hw_context_t * ctx, void * frame, int idx);
void gavl_hw_frame_pool_reset(gavl_hw_context_t * ctx, int free_resources);

void gavl_hw_frame_pool_init(gavl_hw_context_t * ctx);


gavl_hw_context_t *
gavl_hw_context_create_internal(void * native,
                                const gavl_hw_funcs_t * funcs,
                                gavl_hw_type_t type, int support_flags);

void 
gavl_hw_destroy_video_frame(gavl_hw_context_t * ctx,
                            gavl_video_frame_t * frame, int destroy_resource);

void 
gavl_hw_destroy_audio_frame(gavl_hw_context_t * ctx,
                            gavl_audio_frame_t * frame, int destroy_resource);

void 
gavl_hw_destroy_packet(gavl_hw_context_t * ctx,
                       gavl_packet_t * packet, int destroy_resource);

gavl_video_frame_t *
gavl_hw_ctx_create_import_vframe(gavl_hw_context_t * ctx,
                                 int buf_idx);




#endif
