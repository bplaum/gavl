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

#define _GNU_SOURCE
#include <sys/mman.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/hw.h>

#include <hw_private.h>
#include <hw_memfd.h>

#include <gavl/log.h>
#define LOG_DOMAIN "memfd"

static int memfd_buffer_create(gavl_hw_buffer_t * buf, int size)
  {
  if((buf->fd = memfd_create("gavlbuffer", MFD_CLOEXEC | MFD_ALLOW_SEALING)) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot create memfd: %s", strerror(errno));
    return 0;
    }

  if(ftruncate(buf->fd, size) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "ftruncate failed: %s", strerror(errno));
    return 0;
    }

  if(fcntl(buf->fd, F_ADD_SEALS, F_SEAL_SHRINK) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Adding seal failed: %s", strerror(errno));
    return 0;
    }

  buf->map_len = size;
  return 1;
  }

static inline void begin_access(gavl_hw_buffer_t * buf)
  {
  atomic_thread_fence(memory_order_acquire);
  }

static inline void end_access(gavl_hw_buffer_t * buf)
  {
  if(buf->flags & GAVL_HW_BUFFER_WR)
    atomic_thread_fence(memory_order_release);
  }

#if 0
static void memfd_buffer_attach(gavl_memfd_buffer_t * buf, int fd, int size)
  {
  buf->fd = fd;
  buf->map_size = size;
  }
#endif

  
/* Functions */


static gavl_video_frame_t *
video_frame_create_hw_memfd(gavl_hw_context_t * ctx, int alloc_resource)
  {
  gavl_hw_buffer_t * buf = calloc(1, sizeof(*buf));
  
  gavl_video_frame_t * ret = gavl_video_frame_create(NULL);
  ret->storage = buf;

  gavl_hw_buffer_init(buf);
  
  if(alloc_resource &&
     !memfd_buffer_create(buf, gavl_video_format_get_image_size(&ctx->vfmt)))
    goto fail;

  return ret;
  
  fail:
  if(ret)
    gavl_video_frame_destroy(ret);
  return NULL;
  }

static gavl_audio_frame_t *
audio_frame_create_hw_memfd(gavl_hw_context_t * ctx, int alloc_resource)
  {
  gavl_hw_buffer_t * buf = calloc(1, sizeof(*buf));
  
  gavl_audio_frame_t * ret = gavl_audio_frame_create(NULL);
  ret->storage = buf;
  
  if(alloc_resource &&
     !memfd_buffer_create(buf, gavl_audio_format_buffer_size(&ctx->afmt)))
    goto fail;
  
  return ret;
  
  fail:
  if(ret)
    gavl_audio_frame_destroy(ret);
  return NULL;
  }

static gavl_packet_t *
packet_create_hw_memfd(gavl_hw_context_t * ctx, int alloc_resource)
  {
  gavl_hw_buffer_t * buf = calloc(1, sizeof(*buf));
  
  gavl_packet_t * ret = gavl_packet_create();
  ret->storage = buf;
  
  if(alloc_resource &&
     !memfd_buffer_create(buf, ctx->packet_size))
    goto fail;
  
  return ret;
  
  fail:
  if(ret)
    gavl_packet_destroy(ret);
  return NULL;
  }

static void video_frame_destroy_memfd(gavl_video_frame_t * f, int destroy_resource)
  {
  
  if(f->storage)
    {
    gavl_hw_buffer_t * buf = f->storage;

    gavl_hw_buffer_cleanup(buf);
    free(buf);
    f->storage = NULL;
    }
  gavl_video_frame_null(f);
  gavl_video_frame_destroy(f);
  }

static void audio_frame_destroy_memfd(gavl_audio_frame_t * f, int destroy_resource)
  {
  if(f->storage)
    {
    gavl_hw_buffer_t * buf = f->storage;
    gavl_hw_buffer_cleanup(buf);
    free(buf);
    f->storage = NULL;
    }
  gavl_audio_frame_null(f);
  gavl_audio_frame_destroy(f);
  }

static void packet_destroy_memfd(gavl_packet_t * p, int destroy_resource)
  {
  if(p->storage)
    {
    gavl_hw_buffer_t * buf = p->storage;
    gavl_hw_buffer_cleanup(buf);
    free(buf);
    p->storage = NULL;
    }
  gavl_buffer_init(&p->buf); // Paranoia
  gavl_packet_destroy(p);
  }

static int video_frame_map_memfd(gavl_video_frame_t * f, int wr)
  {
  gavl_hw_buffer_t * buf = f->storage;
  
  if(!f->planes[0])
    {
    if(!gavl_hw_buffer_map(buf, wr))
      return 0;
    
    gavl_video_frame_set_planes(f, &f->hwctx->vfmt, buf->map_ptr);
    }

  begin_access(buf);
  
  return 1;
  }

static int audio_frame_map_memfd(gavl_audio_frame_t * f, int wr)
  {
  gavl_hw_buffer_t * buf = f->storage;
  
  if(!f->samples.s_8)
    {
    if(!gavl_hw_buffer_map(buf, wr))
      return 0;
    gavl_audio_frame_from_data(f, &f->hwctx->afmt, buf->map_ptr, buf->map_len);
    }
  begin_access(buf);
  
  return 1;
  }

static int packet_map_memfd(gavl_packet_t * p, int wr)
  {
  gavl_hw_buffer_t * buf = p->storage;
  
  if(!p->buf.buf)
    {
    if(!gavl_hw_buffer_map(buf, wr))
      return 0;
    gavl_buffer_init_static(&p->buf, buf->map_ptr, buf->map_len);
    }

  begin_access(buf);
  
  return 1;
  }

static int video_frame_unmap_memfd(gavl_video_frame_t * frame)
  {
  end_access(frame->storage);
  return 1;
  }

static int audio_frame_unmap_memfd(gavl_audio_frame_t * frame)
  {
  end_access(frame->storage);
  return 1;
  }

static int packet_unmap_memfd(gavl_packet_t * p)
  {
  end_access(p->storage);
  return 1;
  }

static const gavl_hw_funcs_t funcs =
  {
   .video_frame_create     = video_frame_create_hw_memfd,
   .video_frame_destroy    = video_frame_destroy_memfd,
   .video_frame_map        = video_frame_map_memfd,
   .video_frame_unmap      = video_frame_unmap_memfd,

   .audio_frame_create     = audio_frame_create_hw_memfd,
   .audio_frame_destroy    = audio_frame_destroy_memfd,
   .audio_frame_map        = audio_frame_map_memfd,
   .audio_frame_unmap      = audio_frame_unmap_memfd,

   .packet_create          = packet_create_hw_memfd,
   .packet_destroy         = packet_destroy_memfd,
   .packet_map             = packet_map_memfd,
   .packet_unmap           = packet_unmap_memfd,
   
  };

gavl_hw_context_t * gavl_hw_ctx_create_memfd()
  {
  return gavl_hw_context_create_internal(NULL, &funcs, GAVL_HW_MEMFD,
                                         GAVL_HW_SUPPORTS_VIDEO | GAVL_HW_SUPPORTS_VIDEO_MAP |
                                         GAVL_HW_SUPPORTS_SHARED_POOL);
  }


