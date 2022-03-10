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

#include <stdlib.h>
#include <string.h>

#include <config.h>

#include <hw_private.h>

#include <gavl/log.h>

#define LOG_DOMAIN "hw_context"

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
  
  
  if(ret->funcs->get_image_formats)
    ret->image_formats = ret->funcs->get_image_formats(ret);

  if(ret->funcs->get_overlay_formats)
    ret->overlay_formats = ret->funcs->get_overlay_formats(ret);
  
  return ret;
  }

int gavl_hw_ctx_get_support_flags(gavl_hw_context_t * ctx)
  {
  return ctx->support_flags;
  }


void gavl_hw_ctx_destroy(gavl_hw_context_t * ctx)
  {
  if(ctx->imported_vframes)
    {
    int i;
    for(i = 0; i < ctx->imported_vframes_alloc; i++)
      {
      if(ctx->imported_vframes[i])
        gavl_video_frame_destroy(ctx->imported_vframes[i]);
      }
    free(ctx->imported_vframes);
    }
  
  if(ctx->funcs->destroy_native)
    ctx->funcs->destroy_native(ctx->native);

  if(ctx->image_formats)
    free(ctx->image_formats);
  if(ctx->overlay_formats)
    free(ctx->overlay_formats);
  
  free(ctx);
  }

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


gavl_pixelformat_t *
gavl_hw_ctx_get_image_formats(gavl_hw_context_t * ctx)
  {
  return copy_pfmt_arr(ctx->image_formats);
  }

gavl_pixelformat_t *
gavl_hw_ctx_get_overlay_formats(gavl_hw_context_t * ctx)
  {
  return copy_pfmt_arr(ctx->overlay_formats);
  }

void gavl_hw_video_format_adjust(gavl_hw_context_t * ctx,
                                 gavl_video_format_t * fmt)
  {
  if(ctx->funcs->get_image_formats)
    fmt->pixelformat = gavl_pixelformat_get_best(fmt->pixelformat,
                                                 gavl_hw_ctx_get_image_formats(ctx),
                                                 NULL);
  
  if(ctx->funcs->video_format_adjust)
    ctx->funcs->video_format_adjust(ctx, fmt);
  }

void gavl_hw_overlay_format_adjust(gavl_hw_context_t * ctx,
                                   gavl_video_format_t * fmt)
  {
  if(ctx->funcs->get_overlay_formats)
    fmt->pixelformat = gavl_pixelformat_get_best(fmt->pixelformat,
                                                 gavl_hw_ctx_get_overlay_formats(ctx),
                                                 NULL);

  if(ctx->funcs->overlay_format_adjust)
    ctx->funcs->overlay_format_adjust(ctx, fmt);
  }


/* Create a video frame. The frame will be a reference for a hardware surface */
gavl_video_frame_t * gavl_hw_video_frame_create_hw(gavl_hw_context_t * ctx,
                                                   gavl_video_format_t * fmt)
  {
  gavl_video_frame_t * ret;
  if(fmt)
    gavl_hw_video_format_adjust(ctx, fmt);
  if(ctx->funcs->video_frame_create_hw)
    ret = ctx->funcs->video_frame_create_hw(ctx, fmt);
  else
    ret = gavl_video_frame_create(fmt);
  ret->hwctx = ctx;
  return ret;
  }

gavl_video_frame_t * gavl_hw_video_frame_create_ovl(gavl_hw_context_t * ctx,
                                                    gavl_video_format_t * fmt)
  {
  gavl_video_frame_t * ret;
  gavl_hw_overlay_format_adjust(ctx, fmt);
  if(ctx->funcs->video_frame_create_ovl)
    ret = ctx->funcs->video_frame_create_ovl(ctx, fmt);
  else
    ret = gavl_video_frame_create(fmt);
  return ret;
  }

/* Create a video frame. The frame will have data available for CPU access but is
 suitable for transfer to a hardware surface */
gavl_video_frame_t * gavl_hw_video_frame_create_ram(gavl_hw_context_t * ctx,
                                                    gavl_video_format_t * fmt)
  {
  gavl_video_frame_t * ret;
  gavl_hw_video_format_adjust(ctx, fmt);

  if(ctx->funcs->video_frame_create_ram)
    ret = ctx->funcs->video_frame_create_ram(ctx, fmt);
  else
    ret = gavl_video_frame_create(fmt);

  return ret;
  }

/* Load a video frame from RAM into the hardware */
int gavl_video_frame_ram_to_hw(const gavl_video_format_t * fmt,
                                gavl_video_frame_t * dst,
                                gavl_video_frame_t * src)
  {
  gavl_hw_context_t * ctx = dst->hwctx;
  gavl_video_frame_copy_metadata(dst, src);
  return ctx->funcs->video_frame_to_hw ?
    ctx->funcs->video_frame_to_hw(fmt, dst, src) : 0;
  }

/* Load a video frame from the hardware into RAM */
int gavl_video_frame_hw_to_ram(const gavl_video_format_t * fmt,
                               gavl_video_frame_t * dst,
                               gavl_video_frame_t * src)
  {
  gavl_hw_context_t * ctx = src->hwctx;
  gavl_video_frame_copy_metadata(dst, src);
  
  if(ctx->funcs->video_frame_to_ram)
    {
    ctx->funcs->video_frame_to_ram(fmt, dst, src);
    return 1;
    }
  else if(src->planes[0])
    {
    gavl_video_frame_copy(fmt, dst, src);
    return 1;
    }
  else
    return 0;
  }

void 
gavl_hw_destroy_video_frame(gavl_hw_context_t * ctx,
                            gavl_video_frame_t * frame)
  {
  if(ctx->funcs->video_frame_destroy)
    ctx->funcs->video_frame_destroy(frame);
  else
    {
    frame->hwctx = NULL;
    gavl_video_frame_destroy(frame);
    }
  }

gavl_hw_type_t gavl_hw_ctx_get_type(gavl_hw_context_t * ctx)
  {
  return ctx->type;
  }

static const struct
  {
  gavl_hw_type_t type;
  const char * name;
  }
types[] = 
  {
    { GAVL_HW_VAAPI_X11, "vaapi through X11" },

    // EGL Texture (associated with X11 connection)
    { GAVL_HW_EGL_GL_X11, "Opengl Texture (EGL+X11)" },

    // EGL Texture (associated with X11 connection)
    { GAVL_HW_EGL_GLES_X11, "Opengl ES Texture (EGL+X11)" },

    // GAVL_HW_EGL_WAYLAND,  // EGL Texture (wayland) Not implemented yet

    // V4L2 buffers (mmap()ed, optionaly also with DMA handles)
    { GAVL_HW_V4L2_BUFFER, "V4L2 Buffer" }, 

    // DMA handles, can be exported by V4L and im- and exported by OpenGL and also mmaped to userspace
    { GAVL_HW_DMABUFFER,   "DMA Buffer" },

    // Shared memory, which can be sent to other processes
    { GAVL_HW_SHM,         "Shared Memory" },
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

int gavl_hw_ctx_exports_type(gavl_hw_context_t * ctx, gavl_hw_type_t type)
  {
  if(ctx->funcs->can_export && ctx->funcs->can_export(ctx, type))
    return 1;
  else
    return 0;
  }

int gavl_hw_ctx_imports_type(gavl_hw_context_t * ctx, gavl_hw_type_t type)
  {
  if(ctx->funcs->can_import && ctx->funcs->can_import(ctx, type))
    return 1;
  else
    return 0;
  }

static gavl_video_frame_t * create_import_frame(gavl_hw_context_t * ctx,
                                                int buf_idx, gavl_video_format_t * fmt)
  {
  if(buf_idx >= ctx->imported_vframes_alloc)
    {
    int num;
    
    if(buf_idx - ctx->imported_vframes_alloc > 16)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
               "Not importing video frame (buffer index %d too large)",
               buf_idx);
      return NULL;
      }
    
    num = buf_idx + 16;
    ctx->imported_vframes = realloc(ctx->imported_vframes, num * sizeof(*ctx->imported_vframes));
    memset(ctx->imported_vframes + ctx->imported_vframes_alloc, 0,
           (num - ctx->imported_vframes_alloc) * sizeof(*ctx->imported_vframes));
    ctx->imported_vframes_alloc = num;
    }
  ctx->imported_vframes[buf_idx] = gavl_hw_video_frame_create_hw(ctx, NULL);
  return ctx->imported_vframes[buf_idx];
  }

/* ctx1 exports frame to ctx2 */
static int export_frame(gavl_hw_context_t * ctx1,
                        gavl_hw_context_t * ctx2,
                        gavl_video_frame_t * src,
                        gavl_video_frame_t ** dst,
                        gavl_video_format_t * fmt)
  {
  if(src->buf_idx >= 0)
    {
    if((src->buf_idx < ctx2->imported_vframes_alloc) &&
       (ctx2->imported_vframes[src->buf_idx]))
      {
      *dst = ctx2->imported_vframes[src->buf_idx];
      return 1;
      }
    else
      {
      *dst = create_import_frame(ctx2, src->buf_idx, fmt);
      return ctx1->funcs->export_video_frame(ctx1, fmt, src, *dst);
      }
    }
  else
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot transfer video frame (need buf_idx)");
    return 0;
    }
  }

/* ctx2 imports frame from ctx1 */
static int import_frame(gavl_hw_context_t * ctx1,
                        gavl_hw_context_t * ctx2,
                        gavl_video_frame_t * src,
                        gavl_video_frame_t ** dst,
                        gavl_video_format_t * fmt)
  {
  if(src->buf_idx >= 0)
    {
    if((src->buf_idx < ctx2->imported_vframes_alloc) &&
       (ctx2->imported_vframes[src->buf_idx]))
      {
      *dst = ctx2->imported_vframes[src->buf_idx];
      return 1;
      }
    else
      {
      *dst = create_import_frame(ctx2, src->buf_idx, fmt);
      return ctx2->funcs->import_video_frame(ctx2, fmt, src, *dst);
      }
    }
  else
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot transfer video frame (need buf_idx)");
    return 0;
    }
  }

int gavl_hw_ctx_transfer_video_frame(gavl_video_frame_t * frame1,
                                     gavl_hw_context_t * ctx2,
                                     gavl_video_frame_t ** frame2,
                                     gavl_video_format_t * fmt)
  {
  gavl_hw_context_t * ctx1 = frame1->hwctx;
  gavl_hw_type_t t = gavl_hw_ctx_get_type(ctx1);
    
  if(gavl_hw_ctx_imports_type(ctx2, t))
    {
    return import_frame(ctx1, ctx2, frame1, frame2, fmt);
    }

  t = gavl_hw_ctx_get_type(ctx2);
  
  if(gavl_hw_ctx_exports_type(ctx1, t))
    {
    return export_frame(ctx1, ctx2, frame1, frame2, fmt);
    }

  gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot transfer video frame");
  return 0;
  }

int gavl_video_frame_hw_to_packet(gavl_hw_context_t * ctx,
                                  const gavl_video_format_t * fmt,
                                  const gavl_video_frame_t * src,
                                  gavl_packet_t * p)
  {
  if(ctx->funcs->video_frame_to_packet && ctx->funcs->video_frame_to_packet(ctx, fmt, src, p))
    {
    gavl_video_frame_to_packet_metadata(src, p);
    return 1;
    }
  else
    return 0;
  }

gavl_video_frame_t * gavl_video_frame_hw_from_packet(gavl_hw_context_t * ctx,
                                                     const gavl_video_format_t * fmt,
                                                     const gavl_packet_t * src)
  {

  if(ctx->funcs->video_frame_from_packet)
    return NULL;
  
  
#if 0
  
  if(ctx->funcs->video_frame_from_packet(ctx, fmt, frame, src))
    {
    gavl_packet_to_video_frame_metadata(src, frame);
    return 1;
    }
  else
    return 0;

#endif
  }

