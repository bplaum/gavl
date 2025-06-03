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

#include <config.h>

#include <hw_private.h>

#include <gavl/log.h>

#define LOG_DOMAIN "hw_context"

static void get_formats(gavl_hw_context_t * ret)
  {
  if(ret->funcs->get_image_formats)
    {
    ret->image_formats_map = ret->funcs->get_image_formats(ret, GAVL_HW_FRAME_MODE_MAP);
    ret->image_formats_transfer = ret->funcs->get_image_formats(ret, GAVL_HW_FRAME_MODE_TRANSFER);
    }
  }

  

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
  
  get_formats(ret);
  return ret;
  }

int gavl_hw_ctx_get_support_flags(gavl_hw_context_t * ctx)
  {
  return ctx->support_flags;
  }

void gavl_hw_ctx_reset(gavl_hw_context_t * ctx)
  {
  int i;
  gavl_video_frame_t * vf;
  
  for(i = 0; i < ctx->imported.num_frames; i++)
    {
    vf = ctx->imported.frames[i];
    if(vf)
      {
      if(vf->client_data)
        free(vf->client_data);
      
      gavl_hw_destroy_video_frame(ctx, vf, 0);
      }
    }
  for(i = 0; i < ctx->created.num_frames; i++)
    {
    vf = ctx->created.frames[i];
    if(vf)
      {
      if(vf->client_data)
        {
        free(vf->client_data);
        vf->client_data = NULL;
        }
      gavl_hw_destroy_video_frame(ctx, vf, 1);
      }
    }
  
  gavl_hw_frame_pool_reset(&ctx->imported);
  gavl_hw_frame_pool_reset(&ctx->created);

  if(ctx->image_formats_map)
    {
    free(ctx->image_formats_map);
    ctx->image_formats_map = NULL;
    }
  if(ctx->image_formats_transfer)
    {
    free(ctx->image_formats_transfer);
    ctx->image_formats_transfer = NULL;
    }
  
  get_formats(ctx);
  }

void gavl_hw_ctx_destroy(gavl_hw_context_t * ctx)
  {
  gavl_hw_ctx_reset(ctx);
  
  if(ctx->funcs->destroy_native)
    ctx->funcs->destroy_native(ctx->native);

  if(ctx->image_formats_map)
    free(ctx->image_formats_map);
  if(ctx->image_formats_transfer)
    free(ctx->image_formats_transfer);
  
  free(ctx);
  }

#if 0
static gavl_pixelformat_t * copy_pfmt_arr(const gavl_pixelformat_t * pfmts)
  {
  int num = 0;
  gavl_pixelformat_t * ret;
  
  while(pfmts[num] != GAVL_PIXELFORMAT_NONE)
    num++;

  ret = malloc((num+1)*sizeof(*ret));

  num = 0;

  while(pfmts[num] != GAVL_PIXELFORMAT_NONE)
    {
    ret[num] = pfmts[num];
    num++;
    }
  ret[num] = GAVL_PIXELFORMAT_NONE;
  return ret;
  }
#endif

const gavl_pixelformat_t *
gavl_hw_ctx_get_image_formats(gavl_hw_context_t * ctx, gavl_hw_frame_mode_t mode)
  {
  switch(mode)
    {
    case GAVL_HW_FRAME_MODE_MAP:
      return ctx->image_formats_map;
      break;
    case GAVL_HW_FRAME_MODE_TRANSFER:
      return ctx->image_formats_transfer;
      break;
    default:
      break;
    }
  return NULL;
  }


void gavl_hw_video_format_adjust(gavl_hw_context_t * ctx,
                                 gavl_video_format_t * fmt, gavl_hw_frame_mode_t mode)
  {
  if(ctx->funcs->get_image_formats)
    fmt->pixelformat = gavl_pixelformat_get_best(fmt->pixelformat,
                                                 gavl_hw_ctx_get_image_formats(ctx, mode),
                                                 NULL);
  
  if(ctx->funcs->video_format_adjust)
    ctx->funcs->video_format_adjust(ctx, fmt, mode);
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
  if(frame->client_data)
    {
    /* Free refcounter */
    
    }
  
  if(ctx->funcs->video_frame_destroy)
    ctx->funcs->video_frame_destroy(frame, destroy_resource);
  else
    {
    frame->hwctx = NULL;
    gavl_video_frame_destroy(frame);
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
    { GAVL_HW_VAAPI_X11, "vaapi through X11", "va_x11" },

    // EGL Texture (associated with X11 connection)
    { GAVL_HW_EGL_GL_X11, "Opengl Texture (EGL+X11)", "egl_gl_x11" },

    // EGL Texture (associated with X11 connection)
    { GAVL_HW_EGL_GLES_X11, "Opengl ES Texture (EGL+X11)", "egl_gles_x11" },

    // GAVL_HW_EGL_WAYLAND,  // EGL Texture (wayland) Not implemented yet

    // V4L2 buffers (mmap()ed, optionaly also with DMA handles)
    { GAVL_HW_V4L2_BUFFER, "V4L2 Buffer", "v4l2" }, 

    // DMA handles, can be exported by V4L and im- and exported by OpenGL and also mmaped to userspace
    { GAVL_HW_DMABUFFER,   "DMA Buffer", "dmabuf" },

    // Shared memory, which can be sent to other processes
    { GAVL_HW_SHM,         "Shared Memory", "shm"  },

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

int gavl_hw_ctx_can_export(gavl_hw_context_t * ctx, const gavl_hw_context_t * to)
  {
  if(ctx->funcs->can_export && ctx->funcs->can_export(ctx, to))
    return 1;
  else
    return 0;
  }

int gavl_hw_ctx_can_import(gavl_hw_context_t * ctx, const gavl_hw_context_t * from)
  {
  if(ctx->funcs->can_import && ctx->funcs->can_import(ctx, from))
    return 1;
  else
    return 0;
  }

gavl_video_frame_t * gavl_hw_ctx_get_imported_vframe(gavl_hw_context_t * ctx,
                                                     int buf_idx)
  {
  if(ctx->imported.num_frames > buf_idx)
    return ctx->imported.frames[buf_idx];
  else
    return NULL;
  }

gavl_video_frame_t * gavl_hw_ctx_create_import_vframe(gavl_hw_context_t * ctx,
                                                      int buf_idx)
  {
  gavl_video_frame_t * f = gavl_hw_video_frame_create(ctx, 0);
  if(!gavl_hw_frame_pool_add(&ctx->imported, f, buf_idx))
    {
    gavl_hw_destroy_video_frame(ctx, f, 0);
    return NULL;
    }
  f->buf_idx = buf_idx;
  return f;
  }

/* ctx1 exports frame to ctx2 */
static int export_frame(gavl_hw_context_t * ctx1,
                        gavl_hw_context_t * ctx2,
                        gavl_video_frame_t * src,
                        gavl_video_frame_t ** dst,
                        const gavl_video_format_t * fmt)
  {
  *dst = gavl_hw_ctx_create_import_vframe(ctx2, src->buf_idx);
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
      gavl_video_frame_copy_metadata(*frame2, frame1);
    return ret;
    }

  //  t = gavl_hw_ctx_get_type(ctx2);
  
  if(gavl_hw_ctx_can_export(ctx1, ctx2))
    {
    ret = export_frame(ctx1, ctx2, frame1, frame2, fmt);
    if(frame1 && *frame2)
      gavl_video_frame_copy_metadata(*frame2, frame1);
    return ret;
    }

  gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot transfer video frame");
  return 0;
  }

void gavl_hw_ctx_set_video(gavl_hw_context_t * ctx, gavl_video_format_t * fmt, gavl_hw_frame_mode_t mode)
  {
  ctx->mode = mode;
  if(mode != GAVL_HW_FRAME_MODE_IMPORT)
    gavl_hw_video_format_adjust(ctx, fmt, mode);
  gavl_video_format_copy(&ctx->vfmt, fmt);
  }

int gavl_hw_supported(gavl_hw_type_t type)
  {
  switch(type)
    {
    case GAVL_HW_NONE:  // Frames in RAM
      return 1;
      break;
    case GAVL_HW_EGL_GL_X11:     // EGL Texture (associated with X11 connection)
    case GAVL_HW_EGL_GLES_X11:   // EGL Texture (associated with X11 connection)
#ifdef HAVE_EGL
      return 1;
#endif
    case GAVL_HW_VAAPI_X11:
#ifdef HAVE_LIBVA_X11
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
    case GAVL_HW_SHM:         // Shared memory, which can be sent to other processes
      return 1;
      break;
      break;
    }
  return 0;
  }

/* Frame pool */

int gavl_hw_video_frame_refcount(gavl_video_frame_t * f)
  {
  int * rc;
  
  if(!f->client_data)
    return -1;

  rc = f->client_data;
  
  return *rc;
  }

void gavl_hw_video_frame_ref(gavl_video_frame_t * f)
  {
  int * rc;
  if(!f->hwctx)
    return;
  
  if(!f->client_data)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "gavl_hw_video_frame_ref called with no client data set");
    return;
    }
  rc = f->client_data;
  (*rc)++;
  }

void gavl_hw_video_frame_unref(gavl_video_frame_t * f)
  {
  int * rc;
  if(!f->hwctx)
    return;
  
  if(!f->client_data)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "gavl_hw_video_frame_unref called with no client data set");
    return;
    }
  rc = f->client_data;
  (*rc)--;
  }

gavl_video_frame_t *
gavl_hw_video_frame_get(gavl_hw_context_t * ctx)
  {
  int i;
  gavl_video_frame_t * f;

  
  for(i = 0; i < ctx->created.num_frames; i++)
    {
    f = ctx->created.frames[i];
    if(!gavl_hw_video_frame_refcount(f))
      {
      //      fprintf(stderr, "gavl_hw_video_frame_get: reusing frame\n");
      gavl_hw_video_frame_ref(f);
      return f;
      }
    }
  f = gavl_hw_video_frame_create(ctx, 1);
  
  /* Refcounter */
  f->client_data = calloc(1, sizeof(int));
  gavl_hw_frame_pool_add(&ctx->created, f, -1);
  f->buf_idx = ctx->created.num_frames-1;

  /* Add a reference which, is passed to the caller */
  gavl_hw_video_frame_ref(f);

  gavl_hw_video_frame_map(f, 1);
  
  //  fprintf(stderr, "gavl_hw_video_frame_get: creating frame\n");
  return f;
  }

int gavl_hw_frame_pool_add(frame_pool_t * pool, void * frame, int idx)
  {
  if(idx < 0)
    {
    if(pool->num_frames == pool->frames_alloc)
      {
      pool->frames_alloc += 16;
      pool->frames = realloc(pool->frames,
                             sizeof(*pool->frames) * pool->frames_alloc);
    
      memset(pool->frames + pool->num_frames, 0,
             (pool->frames_alloc - pool->num_frames)*sizeof(*pool->frames));
      }
    pool->frames[pool->num_frames] = frame;
    pool->num_frames++;
    
    }
  else
    {
    if(idx - pool->frames_alloc > 16)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
               "Not importing video frame (buffer index %d too large)",
               idx);
      return 0;
      }
    pool->frames_alloc = idx + 16;
    pool->frames = realloc(pool->frames, pool->frames_alloc * sizeof(*pool->frames));
    memset(pool->frames + pool->num_frames, 0,
           (pool->frames_alloc - pool->num_frames)*sizeof(*pool->frames));
    pool->num_frames = pool->frames_alloc;
    pool->frames[idx] = frame;
    }
  return 1;
  }

void gavl_hw_frame_pool_reset(frame_pool_t * pool)
  {
  if(pool->frames)
    free(pool->frames);
  memset(pool, 0, sizeof(*pool));
  }

/* Store / Load */

void gavl_hw_ctx_store(gavl_hw_context_t * ctx, gavl_dictionary_t * dict)
  {
  
  }

gavl_hw_context_t * gavl_hw_ctx_load(const gavl_dictionary_t * dict)
  {
  return NULL;
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

