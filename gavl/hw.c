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



#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

#include <config.h>

#include <gavl/gavl.h>

#include <gavl/log.h>
#define LOG_DOMAIN "hw_context"

#include <gavl/hw.h>
#include <hw_private.h>

#include <gavl/utils.h>


#include <hw_dmabuf.h>
#include <hw_memfd.h>

#define DEFAULT_MAX_FRAMES 16


  

gavl_hw_context_t * gavl_hw_context_create_internal(void * native,
                                                    const gavl_hw_funcs_t * funcs,
                                                    gavl_hw_type_t type,
                                                    int support_flags)
  {
  gavl_hw_context_t * ret = calloc(1, sizeof(*ret));
  ret->type = type;
  ret->native = native;
  ret->funcs = funcs;
  ret->support_flags = support_flags;
  ret->max_frames = DEFAULT_MAX_FRAMES;
  
  return ret;
  }

int gavl_hw_ctx_get_support_flags(gavl_hw_context_t * ctx)
  {
  return ctx->support_flags;
  }

void gavl_hw_ctx_resync(gavl_hw_context_t * ctx)
  {
  int i;
  
  if(ctx->flags & HW_CTX_FLAG_IMPORTER)
    gavl_hw_frame_pool_reset(ctx, 0);

  for(i = 0; i < ctx->num_frames; i++)
    ctx->frames[i].flags &= ~HW_FRAME_WRITTEN;
  }

void gavl_hw_ctx_reset(gavl_hw_context_t * ctx)
  {
  if(ctx->flags & HW_CTX_FLAG_IMPORTER)
    gavl_hw_frame_pool_reset(ctx, 0);
  else
    gavl_hw_frame_pool_reset(ctx, 1);
  
  ctx->max_frames = DEFAULT_MAX_FRAMES;
  
  //  get_formats(ctx);
  }

void gavl_hw_ctx_destroy(gavl_hw_context_t * ctx)
  {
  gavl_hw_ctx_reset(ctx);
  
  if(ctx->funcs->destroy_native)
    ctx->funcs->destroy_native(ctx->native);


  gavl_hw_reftable_destroy(ctx);

  gavl_array_free(&ctx->import_formats);
  gavl_array_free(&ctx->map_formats);
  gavl_array_free(&ctx->transfer_formats);
  
  free(ctx);
  }


/* Create a video frame. The frame will be a reference for a hardware surface */
gavl_video_frame_t * gavl_hw_video_frame_create(gavl_hw_context_t * ctx, int alloc_resource)
  {
  gavl_video_frame_t * ret;
  if(ctx->funcs->video_frame_create)
    ret = ctx->funcs->video_frame_create(ctx, alloc_resource);
  else
    ret = gavl_video_frame_create(alloc_resource ? &ctx->vfmt : NULL);
  ret->hwctx = ctx;


  return ret;
  }

/* Create a video frame. The frame will be a reference for a hardware surface */
gavl_audio_frame_t * gavl_hw_audio_frame_create(gavl_hw_context_t * ctx, int alloc_resource)
  {
  gavl_audio_frame_t * ret;
  if(ctx->funcs->audio_frame_create)
    ret = ctx->funcs->audio_frame_create(ctx, alloc_resource);
  else
    ret = gavl_audio_frame_create(alloc_resource ? &ctx->afmt : NULL);
  ret->hwctx = ctx;
  return ret;
  }

gavl_packet_t * gavl_hw_packet_create(gavl_hw_context_t * ctx, int alloc_resource)
  {
  gavl_packet_t * ret;
  if(ctx->funcs->packet_create)
    ret = ctx->funcs->packet_create(ctx, alloc_resource);
  else
    {
    ret = gavl_packet_create();
    gavl_packet_alloc(ret, ctx->packet_size);
    }
  ret->hwctx = ctx;
  return ret;
  }

/* Load a video frame from RAM into the hardware */
int gavl_video_frame_ram_to_hw(gavl_video_frame_t * dst,
                               gavl_video_frame_t * src)
  {
  gavl_hw_context_t * ctx = dst->hwctx;
  gavl_video_frame_copy_metadata(dst, src);
  
  if(ctx->funcs->video_frame_to_hw)
    return ctx->funcs->video_frame_to_hw(dst, src);
  else if(ctx->funcs->video_frame_map)
    {
    ctx->funcs->video_frame_map(dst, 1);
    gavl_video_frame_copy(&ctx->vfmt, dst, src);
    if(ctx->funcs->video_frame_unmap)
      ctx->funcs->video_frame_unmap(dst);
    return 1;
    }
  else
    return 0;
  }

/* Load a video frame from the hardware into RAM */
int gavl_video_frame_hw_to_ram(gavl_video_frame_t * dst,
                               gavl_video_frame_t * src)
  {
  gavl_hw_context_t * ctx = src->hwctx;
  gavl_video_frame_copy_metadata(dst, src);

  if(ctx->funcs->video_frame_map)
    {
    ctx->funcs->video_frame_map(src, 0);
    gavl_video_frame_copy(&ctx->vfmt, dst, src);
    if(ctx->funcs->video_frame_unmap)
      ctx->funcs->video_frame_unmap(src);
    return 1;
    }
  else if(ctx->funcs->video_frame_to_ram)
    {
    ctx->funcs->video_frame_to_ram(dst, src);
    return 1;
    }
  else if(src->planes[0])
    {
    gavl_video_frame_copy(&ctx->vfmt, dst, src);
    return 1;
    }
  else
    return 0;
  }

void 
gavl_hw_destroy_video_frame(gavl_hw_context_t * ctx,
                            gavl_video_frame_t * frame, int destroy_resource)
  {
  
  if(ctx->funcs->video_frame_destroy)
    ctx->funcs->video_frame_destroy(frame, destroy_resource);
  else
    {
    frame->hwctx = NULL;
    gavl_video_frame_destroy(frame);
    }
  }

void 
gavl_hw_destroy_audio_frame(gavl_hw_context_t * ctx,
                            gavl_audio_frame_t * frame, int destroy_resource)
  {
  
  if(ctx->funcs->audio_frame_destroy)
    ctx->funcs->audio_frame_destroy(frame, destroy_resource);
  else
    {
    frame->hwctx = NULL;
    gavl_audio_frame_destroy(frame);
    }
  }

void 
gavl_hw_destroy_packet(gavl_hw_context_t * ctx,
                       gavl_packet_t * p, int destroy_resource)
  {
  if(ctx->funcs->packet_destroy)
    ctx->funcs->packet_destroy(p, destroy_resource);
  else
    {
    p->hwctx = NULL;
    gavl_packet_destroy(p);
    }
  }

gavl_hw_type_t gavl_hw_ctx_get_type(const gavl_hw_context_t * ctx)
  {
  return ctx->type;
  }

static const struct
  {
  gavl_hw_type_t type;
  const char * name;
  const char * id;
  }
types[] = 
  {
    { GAVL_HW_VAAPI, "vaapi", "vaapi" },

    // EGL Texture (associated with X11 connection)
    { GAVL_HW_EGL_GL, "Opengl Texture (EGL)", "gl" },

    // EGL Texture (associated with X11 connection)
    { GAVL_HW_EGL_GLES, "Opengl ES Texture (EGL)", "gles" },

    // V4L2 buffers (mmap()ed, optionally also with DMA handles)
    { GAVL_HW_V4L2_BUFFER, "V4L2 Buffer", "v4l2" }, 

    // DMA handles, can be exported by V4L and im- and exported by OpenGL and also mmapped to userspace
    { GAVL_HW_DMABUFFER,   "DMA Buffer", "dmabuf" },

    // Shared memory, which can be sent to other processes
    { GAVL_HW_MEMFD,       "memfd shared memory", "memfd" },

    // libdrm Can be allocated on the graphics card and connverted to an OpenGL texture via dmabuf
    { GAVL_HW_NONE,        "RAM", "ram"  },
    { /* End  */ },
  };
  
const char * gavl_hw_type_to_string(gavl_hw_type_t type)
  {
  int i = 0;

  while(types[i].name)
    {
    if(types[i].type == type)
      return types[i].name;
    i++;
    }
  return NULL;
  }

const char * gavl_hw_type_to_id(gavl_hw_type_t type)
  {
  int i = 0;

  while(types[i].name)
    {
    if(types[i].type == type)
      return types[i].id;
    i++;
    }
  return NULL;
  }

gavl_hw_type_t gavl_hw_type_from_id(const char * id)
  {
  int i = 0;

  while(types[i].name)
    {
    if(!strcmp(types[i].id, id))
      return types[i].type;
    i++;
    }
  return 0;
  }

#if 1
const gavl_dictionary_t * gavl_hw_ctx_can_export(gavl_hw_context_t * ctx, const gavl_array_t * to)
  {
  const gavl_dictionary_t * ret;
  if(ctx->funcs->can_export && (ret = ctx->funcs->can_export(ctx, to)))
    return ret;
  else
    return NULL;
  }

int gavl_hw_ctx_can_import(gavl_hw_context_t * ctx, const gavl_hw_context_t * from)
  {
  if((from->flags & HW_CTX_FLAG_VIDEO) &&
     gavl_hw_buf_desc_supports_video_format(&ctx->import_formats, &from->vfmt))
    return 1;
  else
    return 0;
  }
#endif

int gavl_hw_ctx_can_transfer(gavl_hw_context_t * from, gavl_hw_context_t * to)
  {
  return gavl_hw_ctx_can_import(to, from) || gavl_hw_ctx_can_export(from, &to->import_formats);
  }


gavl_video_frame_t * gavl_hw_ctx_get_imported_vframe(gavl_hw_context_t * ctx,
                                                     int buf_idx)
  {
  if(!(ctx->flags & HW_CTX_FLAG_IMPORTER))
    return NULL;
  
  if(ctx->num_frames > buf_idx)
    return ctx->frames[buf_idx].frame;
  else
    return NULL;
  }

gavl_video_frame_t * gavl_hw_ctx_create_import_vframe(gavl_hw_context_t * ctx,
                                                      int buf_idx)
  {
  gavl_video_frame_t * f;

  if(!(ctx->flags & HW_CTX_FLAG_IMPORTER))
    return NULL;
  
  f = gavl_hw_video_frame_create(ctx, 0);
  if(!gavl_hw_frame_pool_add(ctx, f, buf_idx))
    {
    gavl_hw_destroy_video_frame(ctx, f, 0);
    return NULL;
    }
  f->buf_idx = buf_idx;
  return f;
  }


int gavl_hw_ctx_import_video_frame(gavl_video_frame_t * src,
                                   gavl_video_frame_t ** dst,
                                   gavl_hw_context_t * dst_ctx,
                                   const gavl_video_format_t * fmt)
  {
  /* If dst is not given: Enable pool mode */
  if(!*dst)
    {
    if(src->buf_idx < 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot import video frame (need buf_idx)");
      return 0;
      }

    /* Frame was already imported */
    if((*dst = gavl_hw_ctx_get_imported_vframe(dst_ctx, src->buf_idx)))
      {
      gavl_video_frame_copy_metadata(*dst, src);
      return 1;
      }

    *dst = gavl_hw_ctx_create_import_vframe(dst_ctx, src->buf_idx);
    fmt = &dst_ctx->vfmt;
    }
  else if(!fmt)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Importing video frames without frame pool needs format");
    return 0;
    }
  
  return dst_ctx->funcs->import_video_frame(fmt, src, *dst);
  }

int gavl_hw_ctx_export_video_frame(gavl_video_frame_t * src,
                                   gavl_video_frame_t ** dst,
                                   gavl_hw_context_t * dst_ctx,
                                   const gavl_video_format_t * fmt)
  {
  /* If dst is not given: Enable pool mode */
  if(!*dst)
    {
    if(src->buf_idx < 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot export video frame (need buf_idx)");
      return 0;
      }

    /* Frame was already imported */
    if((*dst = gavl_hw_ctx_get_imported_vframe(dst_ctx, src->buf_idx)))
      {
      gavl_video_frame_copy_metadata(*dst, src);
      return 1;
      }

    *dst = gavl_hw_ctx_create_import_vframe(dst_ctx, src->buf_idx);
    fmt = &dst_ctx->vfmt;
    }
  else if(!fmt)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Exporting video frames without frame pool needs format");
    return 0;
    }
  
  return src->hwctx->funcs->export_video_frame(fmt, src, *dst);
  }


/* ctx1 exports frame to ctx2 */
static int export_frame(gavl_hw_context_t * ctx1,
                        gavl_hw_context_t * ctx2,
                        gavl_video_frame_t * src,
                        gavl_video_frame_t ** dst,
                        const gavl_video_format_t * fmt)
  {
  *dst = gavl_hw_ctx_create_import_vframe(ctx2, src->buf_idx);

  //  fprintf(stderr, "Export frame: %d %d\n", src->buf_idx, (*dst)->buf_idx);
  
  return ctx1->funcs->export_video_frame(fmt, src, *dst);
  }

/* ctx2 imports frame from ctx1 */
static int import_frame(gavl_hw_context_t * ctx1,
                        gavl_hw_context_t * ctx2,
                        gavl_video_frame_t * src,
                        gavl_video_frame_t ** dst,
                        const gavl_video_format_t * fmt)
  {
  if(!*dst)
    *dst = gavl_hw_ctx_create_import_vframe(ctx2, src->buf_idx);
  
  //  fprintf(stderr, "Import frame: %d %d\n", src->buf_idx, (*dst)->buf_idx);
  
  return ctx2->funcs->import_video_frame(fmt, src, *dst);
  }

int gavl_hw_ctx_transfer_video_frame(gavl_video_frame_t * frame1,
                                     gavl_hw_context_t * ctx2,
                                     gavl_video_frame_t ** frame2,
                                     const gavl_video_format_t * fmt)
  {
  int ret;
  gavl_hw_context_t * ctx1 = frame1->hwctx;
  //  gavl_hw_type_t t = gavl_hw_ctx_get_type(ctx1);

  if(!fmt)
    fmt = &ctx2->vfmt;
  
  //  fprintf(stderr, "Transfer video frame: %p %p %p\n", frame1, frame1->hwctx, *frame2);
  
  if((frame1->buf_idx < 0) && !(*frame2))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot transfer video frame (need buf_idx)");
    return 0;
    }

  if(!(*frame2))
    {
    if((*frame2 = gavl_hw_ctx_get_imported_vframe(ctx2, frame1->buf_idx)))
      {
      gavl_video_frame_copy_metadata(*frame2, frame1);
      return 1;
      }
    }
  
  if(gavl_hw_ctx_can_import(ctx2, ctx1))
    {
    ret = import_frame(ctx1, ctx2, frame1, frame2, fmt);

    if(frame1 && *frame2)
      {
      gavl_video_frame_copy_metadata(*frame2, frame1);
      (*frame2)->buf_idx = frame1->buf_idx;
      }
    return ret;
    }

  //  t = gavl_hw_ctx_get_type(ctx2);
  
  if(gavl_hw_ctx_can_export(ctx1, &ctx2->import_formats))
    {
    ret = export_frame(ctx1, ctx2, frame1, frame2, fmt);
    if(frame1 && *frame2)
      {
      gavl_video_frame_copy_metadata(*frame2, frame1);
      (*frame2)->buf_idx = frame1->buf_idx;
      }
    return ret;
    }

  gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot transfer video frame");
  return 0;
  }


void gavl_hw_ctx_set_video_creator(gavl_hw_context_t * ctx, gavl_video_format_t * fmt,
                                   gavl_hw_frame_mode_t mode)
  {
  ctx->mode = mode;
  ctx->flags |= (HW_CTX_FLAG_CREATOR | HW_CTX_FLAG_VIDEO);
  gavl_video_format_copy(&ctx->vfmt, fmt);
  fmt->hwctx = ctx;

  /* TODO: Check if format is valid (gmerlin clients make sure it is) */
  }

void gavl_hw_ctx_set_video_importer(gavl_hw_context_t * ctx,
                                    gavl_hw_context_t * ctx_src,
                                    gavl_video_format_t * vfmt)
  {
  ctx->mode = GAVL_HW_FRAME_MODE_IMPORT;
  ctx->flags |= (HW_CTX_FLAG_IMPORTER | HW_CTX_FLAG_VIDEO);

  if(ctx_src)
    {
    ctx->ctx_src = ctx_src;

    gavl_video_format_copy(&ctx->vfmt, &ctx_src->vfmt);
    ctx->vfmt.hwctx = ctx;
    
    if(vfmt)
      gavl_video_format_copy(vfmt, &ctx->vfmt);
    
    }
  else
    {
    if(vfmt)
      {
      vfmt->hwctx = ctx;
      gavl_video_format_copy(&ctx->vfmt, vfmt);
      }
    }
  
  /* TODO: Check if format is valid (gmerlin clients make sure it is) */
  
  }

void gavl_hw_ctx_set_audio_creator(gavl_hw_context_t * ctx, gavl_audio_format_t * fmt,
                                   gavl_hw_frame_mode_t mode)
  {
  ctx->flags |= (HW_CTX_FLAG_CREATOR | HW_CTX_FLAG_VIDEO);

  gavl_audio_format_copy(&ctx->afmt, fmt);
  ctx->vfmt.hwctx = ctx;

  
  }

void gavl_hw_ctx_set_audio_importer(gavl_hw_context_t * ctx,
                                    gavl_hw_context_t * ctx_src,
                                    gavl_audio_format_t * fmt)
  {
  ctx->mode = GAVL_HW_FRAME_MODE_IMPORT;
  ctx->flags |= (HW_CTX_FLAG_IMPORTER | HW_CTX_FLAG_AUDIO);
  
  }

void gavl_hw_ctx_set_packet_creator(gavl_hw_context_t * ctx, int max_size)
  {
  ctx->flags |= (HW_CTX_FLAG_CREATOR | HW_CTX_FLAG_PACKET);
  ctx->packet_size = max_size;
  }

void gavl_hw_ctx_set_packet_importer(gavl_hw_context_t * ctx)
  {
  ctx->mode = GAVL_HW_FRAME_MODE_IMPORT;
  ctx->flags |= (HW_CTX_FLAG_IMPORTER | HW_CTX_FLAG_PACKET);
  
  }

int gavl_hw_supported(gavl_hw_type_t type)
  {
  switch(type)
    {
    case GAVL_HW_NONE:  // Frames in RAM
      return 1;
      break;
    case GAVL_HW_EGL_GL:     // EGL Texture
    case GAVL_HW_EGL_GLES:   // EGL Texture
#ifdef HAVE_EGL
      return 1;
#endif
    case GAVL_HW_VAAPI:
#ifdef HAVE_LIBVA
      return 1;
#endif
      break;
    case GAVL_HW_V4L2_BUFFER: // V4L2 buffers (mmapped)
#ifdef HAVE_V4L2
      return 1;
#endif
      break;
    case GAVL_HW_DMABUFFER:   // DMA handles, can be exported by V4L and im- and exported by OpenGL
#ifdef HAVE_DRM
      return 1;
#endif
      break;
    case GAVL_HW_MEMFD:        // Shared memory, which can be sent to other processes
      return 1;
      break;
      break;
    }
  return 0;
  }

/* Frame pool */

static int refcount(gavl_hw_context_t * ctx, int buf_idx)
  {
  return atomic_load_explicit(&ctx->reftab->frames[buf_idx].refcount,
                              memory_order_relaxed);

  }

static void unref(gavl_hw_context_t * ctx, int buf_idx)
  {
  int val;

  if((val = atomic_fetch_sub_explicit(&ctx->reftab->frames[buf_idx].refcount,
                                      1, memory_order_release)) == 1)
    {
    /* Increase free frames */
    sem_post(&ctx->reftab->free_buffers);
    }
  }

static void ref(gavl_hw_context_t * ctx, int buf_idx)
  {
  atomic_fetch_add_explicit(&ctx->reftab->frames[buf_idx].refcount,
                            1, memory_order_acquire);
  }

#define CHECK_HWCTX_RET if(!f->hwctx) \
    { \
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, \
             "%s called with no hwctx set", __func__); \
    return -1; \
    }

#define CHECK_HWCTX if(!f->hwctx)               \
    { \
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, \
             "%s called with no hwctx set", __func__); \
    return; \
    }

int gavl_hw_video_frame_refcount(gavl_video_frame_t * f)
  {
  CHECK_HWCTX_RET
  return refcount(f->hwctx, f->buf_idx);
  }

void gavl_hw_video_frame_unref(gavl_video_frame_t * f)
  {
  CHECK_HWCTX
  unref(f->hwctx, f->buf_idx);
  }

void gavl_hw_video_frame_ref(gavl_video_frame_t * f)
  {
  CHECK_HWCTX
  ref(f->hwctx, f->buf_idx);
  }

int gavl_hw_audio_frame_refcount(gavl_audio_frame_t * f)
  {
  CHECK_HWCTX_RET
  return refcount(f->hwctx, f->buf_idx);
  }

void gavl_hw_audio_frame_unref(gavl_audio_frame_t * f)
  {
  CHECK_HWCTX
  unref(f->hwctx, f->buf_idx);
  }

void gavl_hw_audio_frame_ref(gavl_audio_frame_t * f)
  {
  CHECK_HWCTX
  ref(f->hwctx, f->buf_idx);
  }

int gavl_hw_packet_refcount(gavl_packet_t * f)
  {
  CHECK_HWCTX_RET
  return refcount(f->hwctx, f->buf_idx);
  }

void gavl_hw_packet_unref(gavl_packet_t * f)
  {
  CHECK_HWCTX
  unref(f->hwctx, f->buf_idx);
  }

void gavl_hw_packet_ref(gavl_packet_t * f)
  {
  CHECK_HWCTX
  ref(f->hwctx, f->buf_idx);
  }

/* Get a frame for *writing* */

static void * get_free_frame(gavl_hw_context_t * ctx)
  {
  int i;
  for(i = 0; i < ctx->num_frames; i++)
    {
    if(!refcount(ctx, i))
      {
      //      fprintf(stderr, "gavl_hw_video_frame_get: reusing frame\n");
      ref(ctx, i);
      return ctx->frames[i].frame;
      }
    }
  return NULL;
  }

static void *
frame_get_write(gavl_hw_context_t * ctx)
  {
  int result;
  void * f = NULL;

  if(!(ctx->flags & HW_CTX_FLAG_CREATOR))
    return NULL;

  gavl_hw_frame_pool_init(ctx);
  
  // fprintf(stderr, "gavl_hw_video_frame_get: %d\n", ctx->created.num_frames);

  while(1)
    {
    result = sem_trywait(&ctx->reftab->free_buffers);

    if(!result)
      return get_free_frame(ctx);
    
    if(errno == EINTR)
      continue;
    else
      break;
    }
  
  /* No free frame, create one if we are below max_frames */
  if(ctx->num_frames < ctx->max_frames)
    {
    switch(ctx->flags & HW_CTX_MODE_MASK)
      {
      case HW_CTX_FLAG_AUDIO:
        {
        gavl_audio_frame_t * af =
          gavl_hw_audio_frame_create(ctx, 1);
        af->buf_idx = ctx->num_frames;

        ref(ctx, af->buf_idx);
        
        if(ctx->mode == GAVL_HW_FRAME_MODE_MAP) 
          gavl_hw_audio_frame_map(af, 1);
        f = af;
        }
        break;
      case HW_CTX_FLAG_VIDEO:
        {
        gavl_video_frame_t * vf =
          gavl_hw_video_frame_create(ctx, 1);
        
        vf->buf_idx = ctx->num_frames;

        ref(ctx, vf->buf_idx);

        if(ctx->mode == GAVL_HW_FRAME_MODE_MAP) 
          gavl_hw_video_frame_map(vf, 1);
        f = vf;
        }
        break;
      case HW_CTX_FLAG_PACKET:
        {
        gavl_packet_t * pkt =
          gavl_hw_packet_create(ctx, 1);
        pkt->buf_idx = ctx->num_frames;

        ref(ctx, pkt->buf_idx);

        if(ctx->mode == GAVL_HW_FRAME_MODE_MAP) 
          gavl_hw_packet_map(pkt, 1);
        f = pkt;
        }
        break;
      }
    
    gavl_hw_frame_pool_add(ctx, f, -1);
    
    /* Add a reference, which is passed to the caller */
    return f;
    }
  else
    {
    /* No more frames can be created. Wait for a free frame */
    struct timespec ts;
    int timeout_ms = 1000; // 1 sec.
    
    // Get current time (CLOCK_REALTIME required!)
    clock_gettime(CLOCK_REALTIME, &ts);
    
    // Add timeout
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    
    // Normalize (if nsec >= 1 second)
    if(ts.tv_nsec >= 1000000000)
      {
      ts.tv_sec++;
      ts.tv_nsec -= 1000000000;
      }
    
    // Wait with timeout
    while(sem_timedwait(&ctx->reftab->free_buffers, &ts) == -1)
      {
      if(errno != EINTR)
        return NULL;
      }
    return get_free_frame(ctx);
    }
  
  //  fprintf(stderr, "gavl_hw_video_frame_get: creating frame\n");
  }

gavl_video_frame_t *
gavl_hw_video_frame_get_write(gavl_hw_context_t * ctx)
  {
  if(!(ctx->flags & HW_CTX_FLAG_VIDEO))
    return NULL;
  return frame_get_write(ctx);
  }

gavl_audio_frame_t *
gavl_hw_audio_frame_get_write(gavl_hw_context_t * ctx)
  {
  if(!(ctx->flags & HW_CTX_FLAG_AUDIO))
    return NULL;
  return frame_get_write(ctx);
  }

gavl_packet_t *
gavl_hw_packet_get_write(gavl_hw_context_t * ctx)
  {
  if(!(ctx->flags & HW_CTX_FLAG_PACKET))
    return NULL;
  return frame_get_write(ctx);
  }
   

void gavl_hw_frame_pool_init(gavl_hw_context_t * ctx)
  {
  /* Don't initialize twice */
  if(ctx->frames)
    return;

  ctx->frames = calloc(ctx->max_frames, sizeof(*ctx->frames));

  /* Create reftable */
  if(ctx->ctx_src)
    ctx->reftab = ctx->ctx_src->reftab;
  else if(ctx->shm_name)
    {
    if(ctx->flags & HW_CTX_FLAG_CREATOR)
      {
      /* Create new shm segment */
      }
    else
      {
      /* Map existing shm segment */
      
      }
    }
  else if(ctx->flags & HW_CTX_FLAG_CREATOR)
    {
    /* Create new reftable in ordinary RAM */
    ctx->reftab_priv = gavl_hw_reftable_create_local(ctx);
    ctx->reftab = ctx->reftab_priv;
    }
  else if(ctx->flags  & HW_CTX_FLAG_IMPORTER)
    ctx->reftab = ctx->ctx_src->reftab;
  
  }


int gavl_hw_frame_pool_add(gavl_hw_context_t * ctx, void * frame, int idx)
  {
  gavl_hw_frame_pool_init(ctx);
  
  if(idx < 0)
    idx = ctx->num_frames;
  
  if(idx >= ctx->max_frames)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "max_frames exceeded");
    return 0;
    }
  
  ctx->frames[idx].frame = frame;
  ctx->frames[idx].flags = 0;
  ctx->num_frames++;
  return 1;
  }

void gavl_hw_frame_pool_reset(gavl_hw_context_t * ctx, int free_resources)
  {
  int i;
  for(i = 0; i < ctx->num_frames; i++)
    {
    switch(ctx->flags & HW_CTX_MODE_MASK)
      {
      case HW_CTX_FLAG_AUDIO:
        break;
      case HW_CTX_FLAG_VIDEO:
        gavl_hw_destroy_video_frame(ctx, ctx->frames[i].frame, free_resources);
        
        break;
      case HW_CTX_FLAG_PACKET:
        break;
      }
    ctx->frames[i].frame = NULL;
    }
  
  if(ctx->frames)
    {
    free(ctx->frames);
    ctx->frames = NULL;
    ctx->num_frames = 0;
    }
  }

/* Store / Load */

// #define GAVL_META_HW_TYPE       "hw-type"
// #define GAVL_META_HW_MAX_FRAMES "hw-frames"
// #define GAVL_META_HW_SHMADDR    "hw-shmaddr"

void gavl_hw_ctx_store(gavl_hw_context_t * ctx, gavl_dictionary_t * dict)
  {
  if(!ctx->shm_name)
    return;
  
  gavl_dictionary_set_string(dict, GAVL_HW_BUF_TYPE, gavl_hw_type_to_id(ctx->type));
  gavl_dictionary_set_int(dict,    GAVL_HW_MAX_FRAMES, ctx->max_frames);
  gavl_dictionary_set_string(dict, GAVL_HW_SHMADDR, ctx->shm_name);
  
  }

gavl_hw_context_t * gavl_hw_ctx_load(const gavl_dictionary_t * dict)
  {
  gavl_hw_context_t * ret;
  const char * hw_id;
  const char * shm_name;
  int max_frames;
  gavl_hw_type_t type;
  
  if(!(hw_id = gavl_dictionary_get_string(dict, GAVL_HW_BUF_TYPE)) ||
     ((type = gavl_hw_type_from_id(hw_id)) == GAVL_HW_NONE))
    return NULL;
    
  if(!(gavl_dictionary_get_int(dict, GAVL_HW_MAX_FRAMES, &max_frames)))
    return NULL;
  
  if(!(shm_name = gavl_dictionary_get_string(dict, GAVL_HW_SHMADDR)))
    return NULL;

  ret = gavl_hw_ctx_create(type);
  
  ret->max_frames = max_frames;
  ret->reftab = gavl_hw_reftable_create_remote(ret);
  return ret;
  }

int
gavl_hw_video_frame_map(gavl_video_frame_t * frame, int wr)
  {
  if(!frame->hwctx)
    return 1;

  if(!(frame->hwctx->support_flags & GAVL_HW_SUPPORTS_VIDEO_MAP))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot map frame: Not supported by context");
    return 0;
    }
  if(!frame->hwctx->funcs->video_frame_map)
    return 0;
  
  return frame->hwctx->funcs->video_frame_map(frame, wr);
  
  }

int
gavl_hw_video_frame_unmap(gavl_video_frame_t * frame)
  {
  if(!frame->hwctx)
    return 1;

  if(!(frame->hwctx->support_flags & GAVL_HW_SUPPORTS_VIDEO_MAP))
    return 0;

  if(!frame->hwctx->funcs->video_frame_unmap)
    return 0;
  
  return frame->hwctx->funcs->video_frame_unmap(frame);
  
  }

int
gavl_hw_audio_frame_map(gavl_audio_frame_t * frame, int wr)
  {
  if(!frame->hwctx)
    return 1;

  if(!(frame->hwctx->support_flags & GAVL_HW_SUPPORTS_AUDIO_MAP))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot map frame: Not supported by context");
    return 0;
    }
  if(!frame->hwctx->funcs->audio_frame_map)
    return 0;
  
  return frame->hwctx->funcs->audio_frame_map(frame, wr);
  
  }

int
gavl_hw_audio_frame_unmap(gavl_audio_frame_t * frame)
  {
  if(!frame->hwctx)
    return 1;

  if(!(frame->hwctx->support_flags & GAVL_HW_SUPPORTS_AUDIO_MAP))
    return 0;

  if(!frame->hwctx->funcs->audio_frame_unmap)
    return 0;
  
  return frame->hwctx->funcs->audio_frame_unmap(frame);
  
  }

int
gavl_hw_packet_map(gavl_packet_t * frame, int wr)
  {
  if(!frame->hwctx)
    return 1;

  if(!(frame->hwctx->support_flags & GAVL_HW_SUPPORTS_PACKET_MAP))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot map packet: Not supported by context");
    return 0;
    }
  if(!frame->hwctx->funcs->packet_map)
    return 0;
  
  return frame->hwctx->funcs->packet_map(frame, wr);
  
  }

int
gavl_hw_packet_unmap(gavl_packet_t * frame)
  {
  if(!frame->hwctx)
    return 1;

  if(!(frame->hwctx->support_flags & GAVL_HW_SUPPORTS_AUDIO_MAP))
    return 0;

  if(!frame->hwctx->funcs->packet_unmap)
    return 0;
  
  return frame->hwctx->funcs->packet_unmap(frame);
  
  }



gavl_hw_context_t * gavl_hw_ctx_create(gavl_hw_type_t type)
  {
  gavl_hw_context_t * ret = NULL;
  switch(type)
    {
    case GAVL_HW_DMABUFFER:
      ret = gavl_hw_ctx_create_dma();
      break;
    case GAVL_HW_MEMFD:
      ret = gavl_hw_ctx_create_memfd();
      break;
    default:
      break;
    }
  if(!ret)
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "gavl_hw_ctx_create is not supported for %s",
             gavl_hw_type_to_string(type));
  return ret;
  }

void gavl_hw_ctx_set_max_frames(gavl_hw_context_t * ctx, int num)
  {
  if(ctx->frames || ctx->reftab)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot change max_frames after frames were created or imported");
    return;
    }
  
  ctx->max_frames = num;
  }

int gavl_hw_ctx_set_shared(gavl_hw_context_t * ctx)
  {
  if(!(ctx->support_flags & GAVL_HW_SUPPORTS_SHARED_POOL))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "hw_ctx doesn't support process sharing");
    return 0;
    }
  
  ctx->shm_name = gavl_hw_reftable_create_name();
  return 1; 
  }

/* */

void gavl_hw_buffer_init(gavl_hw_buffer_t * buf)
  {
  buf->fd = -1;
  }

int gavl_hw_buffer_map(gavl_hw_buffer_t * buf, int wr)
  {
  int flags = PROT_READ;

  if(wr)
    {
    flags |= PROT_WRITE;
    buf->flags |= GAVL_HW_BUFFER_WR;
    }
  if((buf->map_ptr = mmap(NULL, buf->map_len, 
                          flags, MAP_SHARED, buf->fd, buf->map_offset)) == MAP_FAILED)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "mmap failed: %s", strerror(errno));
    return 0;
    }
  return 1;
  }

int gavl_hw_buffer_unmap(gavl_hw_buffer_t * buf)
  {
  munmap(buf->map_ptr, buf->map_len);
  buf->map_ptr = NULL;
  buf->flags &= ~GAVL_HW_BUFFER_WR;
  return 1;
  }

int gavl_hw_buffer_cleanup(gavl_hw_buffer_t * buf)
  {
  if(buf->fd >= 0)
    {
    close(buf->fd);
    buf->fd = -1;
    }
  if(buf->map_ptr)
    {
    munmap(buf->map_ptr, buf->map_len);
    buf->map_ptr = NULL;
    }
  return 1;
  }

gavl_dictionary_t * gavl_hw_buf_desc_append(gavl_array_t * arr, gavl_hw_type_t type)
  {
  gavl_dictionary_t * dict;
  dict = gavl_array_append_dictionary(arr);
  gavl_dictionary_set_int(dict, GAVL_HW_BUF_TYPE, type);
  return dict;
  }

/* Append integer format (e.g. gavl_pixelformat_t) */
void gavl_hw_buf_desc_append_format(gavl_dictionary_t * dict,
                                    const char * key, int fmt)
  {
  gavl_value_t val;
  gavl_array_t * arr =
    gavl_dictionary_get_array_create(dict, key);

  gavl_value_init(&val);
  gavl_value_set_int(&val, fmt);
  gavl_array_splice_val_nocopy(arr, -1, 0, &val);
  }

void gavl_hw_buf_desc_set_max_size(gavl_dictionary_t * dict, int w, int h)
  {
  gavl_dictionary_set_int(dict, GAVL_HW_BUF_MAX_W, w);
  gavl_dictionary_set_int(dict, GAVL_HW_BUF_MAX_H, h);
  }

int gavl_hw_buf_desc_get_max_size(const gavl_dictionary_t * dict, int * w, int * h)
  {
  return gavl_dictionary_get_int(dict, GAVL_HW_BUF_MAX_W, w) &&
    gavl_dictionary_get_int(dict, GAVL_HW_BUF_MAX_H, h);
  }


int gavl_hw_buf_desc_supports_format(const gavl_dictionary_t * dict,
                                     const char * key, int fmt)
  {
  int i;
  int val_i = 0;
  const gavl_array_t * arr =
    gavl_dictionary_get_array(dict, key);

  /* Nothing given: Assume everything is supported */
  if(!arr)
    return 1;

  for(i = 0; i < arr->num_entries; i++)
    {
    if(gavl_value_get_int(&arr->entries[i], &val_i) &&
       (val_i == fmt))
      return 1;
    }
  return 0;
  }

#if 0
gavl_hw_type_t gavl_hw_buf_desc_supported_audio(const gavl_array_t * arr,
                                                const gavl_audio_format_t * fmt)
  {
  int i;
  int ret = GAVL_HW_NONE;
  const gavl_dictionary_t * dict;
  
  for(i = 0; i < arr->num_entries; i++)
    {
    if((dict = gavl_value_get_dictionary(&arr->entries[i])) &&
       gavl_hw_buf_desc_supports_format(dict, GAVL_HW_BUF_SAMPLEFORMAT, fmt->sample_format) &&
       gavl_hw_buf_desc_supports_format(dict, GAVL_HW_BUF_INTERLEAVE_MODE, fmt->interleave_mode) &&
       gavl_hw_buf_desc_supports_format(dict, GAVL_HW_BUF_SAMPLERATE, fmt->samplerate))
      {
      gavl_dictionary_get_int(dict, GAVL_HW_BUF_TYPE, &ret);
      return ret;
      }
    }
  return ret;
  }

gavl_hw_type_t gavl_hw_buf_desc_supported_video(const gavl_array_t * arr,
                                                const gavl_video_format_t * fmt)
  {
  int i;
  int ret = GAVL_HW_NONE;
  const gavl_dictionary_t * dict;
  
  for(i = 0; i < arr->num_entries; i++)
    {
    if((dict = gavl_value_get_dictionary(&arr->entries[i])) &&
       gavl_hw_buf_desc_supports_format(dict, GAVL_HW_BUF_PIXELFORMAT, fmt->pixelformat))
      {
      gavl_dictionary_get_int(dict, GAVL_HW_BUF_TYPE, &ret);
      break;
      }
    }
  return ret;
  }



#endif


const gavl_dictionary_t *
gavl_hw_buf_desc_get_pixelformat_conversion(const gavl_array_t * arr,
                                            const gavl_video_format_t * fmt,
                                            gavl_pixelformat_t * pfmt,
                                            int * price)
  {
  int val_i = 0;
  int i, j;
  int test_penalty;
  const gavl_array_t * pfmts;
  const gavl_dictionary_t * dict = NULL;
  const gavl_dictionary_t * ret = NULL;
  
  for(i = 0; i < arr->num_entries; i++)
    {
    if((dict = gavl_value_get_dictionary(&arr->entries[i])) &&
       (pfmts = gavl_dictionary_get_array(dict, GAVL_HW_BUF_PIXELFORMAT)))
      {
      for(j = 0; j < pfmts->num_entries; j++)
        {
        if(gavl_value_get_int(&pfmts->entries[j], &val_i))
          {
          test_penalty = gavl_pixelformat_conversion_penalty(fmt->pixelformat, val_i);

          if((*price == -1) || (*price > test_penalty))
            {
            *price = test_penalty;
            *pfmt = val_i;
            ret = dict;
            }
          }
        }
      }
    }
  return ret;
  }


const gavl_dictionary_t *
gavl_hw_buf_desc_supports_video_format(const gavl_array_t * arr,
                                       const gavl_video_format_t * fmt)
  {
  int i;
  int type = GAVL_HW_NONE;
  const gavl_dictionary_t * dict;
  int w = 0, h = 0;
  
  for(i = 0; i < arr->num_entries; i++)
    {
    if((dict = gavl_value_get_dictionary(&arr->entries[i])) &&
       gavl_hw_buf_desc_supports_format(dict, GAVL_HW_BUF_PIXELFORMAT, fmt->pixelformat) &&
      /* Check image size */
       (!gavl_hw_buf_desc_get_max_size(dict, &w, &h) || // No max size -> assume infinite
        (!fmt->frame_width && !fmt->frame_height) ||    // No image size -> be optimistic
        ((fmt->frame_width < w) && (fmt->frame_height < h))))
      {
      if(gavl_dictionary_get_int(dict, GAVL_HW_BUF_TYPE, &type) &&
         (!fmt->hwctx || (type == gavl_hw_ctx_get_type(fmt->hwctx))))
        return dict;
      }
    }
  return NULL;
  }

const gavl_dictionary_t *
gavl_hw_buf_desc_supports_dma_fourcc(const gavl_array_t * arr, int fourcc)
  {
  int i;
  const gavl_dictionary_t * dict;

  for(i = 0; i < arr->num_entries; i++)
    {
    if((dict = gavl_value_get_dictionary(&arr->entries[i])) &&
       gavl_hw_buf_desc_supports_format(dict, GAVL_HW_BUF_DMA_FOURCC, fourcc))
      {
      return dict;
      }
    }
  return NULL;
  }


const gavl_dictionary_t *
gavl_hw_buf_desc_supports_audio_format(const gavl_array_t * arr,
                                       const gavl_audio_format_t * fmt)
  {
  int i;
  int type = GAVL_HW_NONE;
  const gavl_dictionary_t * dict;
  
  for(i = 0; i < arr->num_entries; i++)
    {
    if((dict = gavl_value_get_dictionary(&arr->entries[i])) &&
       gavl_hw_buf_desc_supports_format(dict, GAVL_HW_BUF_SAMPLEFORMAT, fmt->sample_format) &&
       gavl_hw_buf_desc_supports_format(dict, GAVL_HW_BUF_INTERLEAVE_MODE, fmt->interleave_mode) &&
       gavl_hw_buf_desc_supports_format(dict, GAVL_HW_BUF_SAMPLERATE, fmt->samplerate))
      {
      if(gavl_dictionary_get_int(dict, GAVL_HW_BUF_TYPE, &type) &&
         (!fmt->hwctx || (type == gavl_hw_ctx_get_type(fmt->hwctx))))
        return dict;
      }
    }
  return NULL;
  }

const gavl_array_t * gavl_hw_ctx_get_import_formats(const gavl_hw_context_t * ctx)
  {
  return &ctx->import_formats;
  }

const gavl_array_t * gavl_hw_ctx_get_transfer_formats(const gavl_hw_context_t * ctx)
  {
  return &ctx->transfer_formats;
  }

const gavl_array_t * gavl_hw_ctx_get_map_formats(const gavl_hw_context_t * ctx)
  {
  return &ctx->map_formats;
  }

void gavl_hw_ctx_set_transfer_formats(gavl_hw_context_t * ctx, const gavl_array_t * arr)
  {
  gavl_array_reset(&ctx->transfer_formats);
  gavl_array_copy(&ctx->transfer_formats, arr);
  
  }

void gavl_hw_ctx_set_map_formats(gavl_hw_context_t * ctx, const gavl_array_t * arr)
  {
  gavl_array_reset(&ctx->map_formats);
  gavl_array_copy(&ctx->map_formats, arr);
  
  }


gavl_hw_context_t * gavl_hw_ctx_create_from_buffer_format(const gavl_dictionary_t * dict)
  {
  gavl_hw_context_t * ret;
  gavl_dictionary_t * dict_dst;
  int type = GAVL_HW_NONE;

  if(!gavl_dictionary_get_int(dict, GAVL_HW_BUF_TYPE, &type) ||
     !(ret = gavl_hw_ctx_create(type)))
    return NULL;

  /* We override the supported formats completely */
  gavl_array_reset(&ret->map_formats);
  dict_dst = gavl_array_append_dictionary(&ret->map_formats);
  gavl_dictionary_copy(dict_dst, dict);
  return ret;
  }

void gavl_hw_buffer_format_dump(const gavl_dictionary_t * dict)
  {
  int i;
  int type = GAVL_HW_NONE;
  const gavl_array_t * pfmts;
  const gavl_array_t * fourccs;
  int pfmt;
  int fourcc;
  
  gavl_dictionary_get_int(dict, GAVL_HW_BUF_TYPE, &type);
  
  gavl_dprintf("buffer format: %s\n", gavl_hw_type_to_string(type));

  pfmts =   gavl_dictionary_get_array(dict, GAVL_HW_BUF_PIXELFORMAT);
  fourccs = gavl_dictionary_get_array(dict, GAVL_HW_BUF_DMA_FOURCC);

  if(!pfmts)
    return;
  
  for(i = 0; i < pfmts->num_entries; i++)
    {
    gavl_value_get_int(&pfmts->entries[i], &pfmt);

    if(fourccs)
      {
      gavl_value_get_int(&fourccs->entries[i], &fourcc);

      gavl_dprintf("  %c%c%c%c (%s)\n",
                   (fourcc) & 0xff, 
                   (fourcc >> 8) & 0xff, 
                   (fourcc >> 16) & 0xff, 
                   (fourcc >> 24) & 0xff, 
                   gavl_pixelformat_to_string(pfmt));
      }
    else
      gavl_dprintf("  %s\n",
                   gavl_pixelformat_to_string(pfmt));
    }
  }

void gavl_hw_buffer_formats_dump(const gavl_array_t * arr)
  {
  int i;
  for(i = 0; i < arr->num_entries; i++)
    gavl_hw_buffer_format_dump(gavl_value_get_dictionary(&arr->entries[i]));
  }
