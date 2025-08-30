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
#include <stdio.h>
#include <string.h>

#include <config.h>

#include <GL/gl.h>
#include <GL/glext.h>

#define EGL_EGLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>


#include <gavl/gavl.h>
#include <gavl/hw_gl.h>
#include <gavl/hw_egl.h>

#include <gavl/log.h>
#define LOG_DOMAIN "egl"

#include <hw_private.h>

#ifndef EGL_PLATFORM_X11_EXT
#define EGL_PLATFORM_X11_EXT 0x31D5
#endif

#ifdef HAVE_DRM
#include <gavl/hw_v4l2.h>
#include <gavl/hw_dmabuf.h>
#endif

#ifndef GL_TEXTURE_EXTERNAL_OES
#define GL_TEXTURE_EXTERNAL_OES 0x8D65
#endif


static const char* egl_strerror(EGLint error);

#ifdef HAVE_DRM
static int egl_import_dmabuf(gavl_hw_context_t * ctx,
                             const gavl_video_format_t * fmt,
                             gavl_video_frame_t * src,
                             gavl_video_frame_t * dst);
#endif

typedef struct
  {
  EGLDisplay display;
  EGLConfig  config;
  EGLContext  context;
  GLint max_texture_size;
  
  /* If off-screen rendering is used */
  EGLSurface surf;

  /* Current rendering target */
  EGLSurface current_surf;
  
  EGLenum platform;
  
  /* Function pointers */

  /* Originally the last argument the next 2 functions  const EGLAttrib*.
   * We change it to void* to enable compilation against older GLE headers
   * (e.g. on Raspbian).
   * We don't use them anyway.
   */
  
  EGLDisplay (*eglGetPlatformDisplay)(EGLenum, void *, const void*);

  EGLSurface (*eglCreatePlatformWindowSurface)(EGLDisplay, EGLConfig,
                                               void *, const void*);

  
  
  void * (*eglCreateImage)(EGLDisplay dpy, EGLContext ctx, EGLenum target,
                           EGLClientBuffer buffer, const EGLint *attrib_list);
  
  EGLBoolean (*eglDestroyImage)(EGLDisplay dpy, void * image);

  void (*glEGLImageTargetTexture2DOES)(GLenum target, void * image);

  EGLBoolean (*eglQueryDmaBufFormatsEXT)(EGLDisplay dpy,
                                         EGLint max_formats,
                                         EGLint *formats,
                                         EGLint *num_formats);
  
  EGLBoolean (*eglExportDMABUFImageQueryMESA)(EGLDisplay dpy,
                                              EGLImageKHR image,
                                              int *fourcc,
                                              int *num_planes,
                                              EGLuint64KHR *modifiers);

  EGLBoolean (*eglExportDMABUFImageMESA)(EGLDisplay dpy,
                                         EGLImageKHR image,
                                         int *fds,
                                         EGLint *strides,
                                         EGLint *offsets);

  
  EGLSync (*eglCreateSyncKHR)(EGLDisplay display,
                              EGLenum type,
                              EGLAttrib const * attrib_list);

  
  EGLint (*eglClientWaitSyncKHR)(EGLDisplay dpy,
                                 EGLSyncKHR sync,
                                 EGLint flags,
                                 EGLTimeKHR timeout);

  EGLBoolean (*eglDestroySyncKHR)(EGLDisplay dpy,
                                  EGLSyncKHR sync);
  
  } egl_t;

static void destroy_native_egl(void * native)
  {
  egl_t * priv = native;

  if(priv->surf != EGL_NO_SURFACE)
    eglDestroySurface(priv->display,
                      priv->surf);

  eglDestroyContext(priv->display, priv->context);
  eglTerminate(priv->display);
  
  free(priv);
  }

static gavl_video_frame_t *
video_frame_create_hw_egl(gavl_hw_context_t * ctx, int alloc_resource)
  {
  gavl_video_frame_t * ret;
  
  //  fprintf(stderr, "video_frame_create_hw_egl\n");
  //  gavl_video_format_dump(fmt);
  
  gavl_hw_egl_set_current(ctx, EGL_NO_SURFACE);

  if(alloc_resource)
    ret = gavl_gl_create_frame(&ctx->vfmt);
  else
    ret = gavl_gl_create_frame(NULL);
  
  gavl_hw_egl_unset_current(ctx);
  return ret;
  }


static void video_frame_destroy_egl(gavl_video_frame_t * f, int destroy_resource)
  {
  gavl_hw_context_t * hw = f->hwctx;
  gavl_hw_egl_set_current(hw, EGL_NO_SURFACE);
  gavl_gl_destroy_frame(f, destroy_resource);
  gavl_hw_egl_unset_current(hw);
  return;
  }

static int video_frame_to_ram_egl(gavl_video_frame_t * dst,
                                  gavl_video_frame_t * src)
  {
  gavl_hw_egl_set_current(src->hwctx, EGL_NO_SURFACE);
  gavl_gl_frame_to_ram(&src->hwctx->vfmt, dst, src);
  gavl_hw_egl_unset_current(src->hwctx);
  return 1;
  }

static int video_frame_to_hw_egl(gavl_video_frame_t * dst,
                                 gavl_video_frame_t * src)
  {
  gavl_hw_egl_set_current(dst->hwctx, EGL_NO_SURFACE);
  gavl_gl_frame_to_hw(&dst->hwctx->vfmt, dst, src);
  gavl_hw_egl_unset_current(dst->hwctx);
  return 1;
  }

static int exports_type_egl(gavl_hw_context_t * ctx, const gavl_hw_context_t * to)
  {
  const char * extensions;
  egl_t * priv = ctx->native;
  gavl_hw_type_t hw = gavl_hw_ctx_get_type(to);

  extensions = eglQueryString(priv->display, EGL_EXTENSIONS);
  
  switch(hw)
    {
    case GAVL_HW_DMABUFFER:
      if(strstr(extensions, "EGL_MESA_image_dma_buf_export"))
        return 1;
      break;
    default:
      break;
    }
  return 0;
  }

static int imports_type_egl(gavl_hw_context_t * ctx, const gavl_hw_context_t * from)
  {
  const char * extensions;
  egl_t * priv = ctx->native;
  gavl_hw_type_t hw = gavl_hw_ctx_get_type(from);
  extensions = eglQueryString(priv->display, EGL_EXTENSIONS);
  
  switch(hw)
    {
#ifdef HAVE_DRM
    case GAVL_HW_DMABUFFER:
      if(strstr(extensions, "EGL_EXT_image_dma_buf_import") &&
         strstr(extensions, "EGL_EXT_image_dma_buf_import_modifiers"))
        return 1;
      break;
#endif
    default:
      break;
    }
  return 0;
  }

static int import_video_frame_egl(const gavl_video_format_t * vfmt,
                                  gavl_video_frame_t * src,
                                  gavl_video_frame_t * dst)
  {
  gavl_hw_type_t src_hw_type = gavl_hw_ctx_get_type(src->hwctx);
  
  switch(src_hw_type)
    {
#ifdef HAVE_DRM
    case GAVL_HW_DMABUFFER:
      return egl_import_dmabuf(dst->hwctx, vfmt, src, dst);
      break;
#endif
    default:
      break;
    }
  return 0;
  }

static int export_video_frame_egl(const gavl_video_format_t * fmt,
                                  gavl_video_frame_t * src, gavl_video_frame_t * dst)
  {
  egl_t * priv = src->hwctx->native;
  gavl_gl_frame_info_t * gl = src->storage;
  
  //  gavl_hw_vaapi_t * dev = src->hwctx->native;
  gavl_hw_type_t dst_hw_type = gavl_hw_ctx_get_type(dst->hwctx);

  switch(dst_hw_type)
    {
#ifdef HAVE_DRM
    case GAVL_HW_DMABUFFER:
      {
      int i;
      EGLImage image;
      uint64_t modifier;
      uint32_t format;
      int fds[4], strides[4], offsets[4];
      int num_planes;
      
      gavl_dmabuf_video_frame_t * dma_frame = dst->storage;

      /*
        typedef struct
        {
        int num_textures;
        GLuint textures[GAVL_MAX_PLANES];
        GLenum texture_target;
        } ;
        
      */
      
      //   gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Exported GL rexture as DMA buffer");
      
      // EGL-Image von Textur erstellen
      EGLint image_attrs[] = {
        EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
        EGL_NONE
      };

      gavl_hw_egl_set_current(priv->context, EGL_NO_SURFACE);
      for(i = 0; i < gl->num_textures; i++)
        {
        image = priv->eglCreateImage(priv->display, priv->context,
                                     EGL_GL_TEXTURE_2D_KHR,
                                     (EGLClientBuffer)(intptr_t)gl->textures[i],
                                     image_attrs);
      
        if(image == EGL_NO_IMAGE_KHR)
          {
          printf("Fehler beim Erstellen des EGL-Images\n");
          return 0;
          }
      
        if(!priv->eglExportDMABUFImageQueryMESA(priv->display, image,
                                                (int*)&format, &num_planes,
                                                &modifier))
          {
          printf("Fehler beim Abfragen der DMA-Buffer-Informationen\n");
          return 0;
          }

        //        fprintf(stderr, "Modifier: %lx\n", modifier);
        
        if(!priv->eglExportDMABUFImageMESA(priv->display, image,
                                           fds, strides, offsets))
          {
          printf("Fehler beim Exportieren des DMA-Buffers\n");
          return 0;
          }

        dma_frame->buffers[i].fd = fds[0];
        dma_frame->buffers[i].map_len = 0; // Frames are not meant for mapping

        dma_frame->planes[i].buf_idx = i;
        dma_frame->planes[i].offset = offsets[i];
        dma_frame->planes[i].stride = strides[i];
        dma_frame->planes[i].modifiers = modifier;
        
        dst->strides[i] = strides[i];
        
        priv->eglDestroyImage(priv->display, image);
        }
      gavl_hw_egl_unset_current(priv->context);
      
      dma_frame->num_buffers = gl->num_textures;
      dma_frame->num_planes = gl->num_textures;
      dma_frame->fourcc = format;
      
      }
#endif
    default:
      break;
    }
  return 0;
  }



static int get_num_dma_import_formats(gavl_hw_context_t * ctx)
  {
#ifdef HAVE_DRM
  egl_t * priv = ctx->native;
  EGLint num_formats;
  const char * extensions;

  if(ctx->type != GAVL_HW_EGL_GLES)
    return 0;
  
  extensions = eglQueryString(priv->display, EGL_EXTENSIONS);

  if(strstr(extensions, "EGL_EXT_image_dma_buf_import_modifiers"))
    {
    if(priv->eglQueryDmaBufFormatsEXT(priv->display, 0, NULL, &num_formats))
      return num_formats;
    }
#endif
  return 0;
  }

uint32_t * gavl_hw_ctx_egl_get_dma_import_formats(gavl_hw_context_t * ctx)
  {
  EGLint *formats = NULL;
#ifdef HAVE_DRM
  int i = 0;
  EGLint max_formats;
  egl_t * priv = ctx->native;
  int num_formats = get_num_dma_import_formats(ctx);
  
  if(!num_formats)
    return NULL;

  // fprintf(stderr, "Getting supported DMA formats\n");

  formats = calloc(num_formats+1, sizeof(*formats));
  max_formats = num_formats;
  
  priv->eglQueryDmaBufFormatsEXT(priv->display, max_formats, formats, &num_formats);

  while(i < num_formats)
    {
    
    if(gavl_drm_pixelformat_from_fourcc(formats[i], NULL, NULL) == GAVL_PIXELFORMAT_NONE)
      {
#if 0
      fprintf(stderr, "Got unhandled format: %c%c%c%c\n",
              (formats[i] >> 0) & 0xff,
              (formats[i] >> 8) & 0xff,
              (formats[i] >> 16) & 0xff,
              (formats[i] >> 24) & 0xff);
#endif 
      /* We move the trailing zero */
      memmove(formats + i, formats + i + 1, (num_formats - i)*sizeof(*formats));
      num_formats--;

      }
    else
      {
#if 0
      fprintf(stderr, "Suported format: %c%c%c%c -> %s\n",
              (formats[i] >> 0) & 0xff,
              (formats[i] >> 8) & 0xff,
              (formats[i] >> 16) & 0xff,
              (formats[i] >> 24) & 0xff, gavl_pixelformat_to_string(gavl_drm_pixelformat_from_fourcc(formats[i], NULL, NULL)));
#endif 
      i++;
      }
    }
#endif
  return (uint32_t*)formats;
  }

static gavl_pixelformat_t * get_image_formats_egl(gavl_hw_context_t * ctx, gavl_hw_frame_mode_t mode)
  {
  int num;
  if(mode == GAVL_HW_FRAME_MODE_TRANSFER)
    return gavl_gl_get_image_formats(ctx, &num);
  else
    return NULL;
  }

static const gavl_hw_funcs_t funcs =
  {
    .destroy_native         = destroy_native_egl,
    //    .video_format_adjust    = adjust_video_format_egl,

    //    .get_image_formats      = gavl_gl_get_image_formats,
    .get_image_formats      = get_image_formats_egl,
    .video_frame_create      = video_frame_create_hw_egl,
    .video_frame_destroy    = video_frame_destroy_egl,
    .video_frame_to_ram     = video_frame_to_ram_egl,
    .video_frame_to_hw      = video_frame_to_hw_egl,
    
    .can_export             = exports_type_egl,
    .can_import             = imports_type_egl,
    .import_video_frame     = import_video_frame_egl,
    .export_video_frame     = export_video_frame_egl
  };

static const EGLint
gles_attributes[] =
  {
#ifdef EGL_CONTEXT_MAJOR_VERSION
   EGL_CONTEXT_MAJOR_VERSION, 3,
#else
   EGL_CONTEXT_CLIENT_VERSION, 3,
#endif
   EGL_NONE
  };

gavl_hw_context_t *
gavl_hw_ctx_create_egl(EGLenum platform,
                       EGLint const * attrs,
                       gavl_hw_type_t type,
                       void * native_display)
  {
  int support_flags;
  egl_t * priv;
  EGLint num_configs = 0;

  const EGLint * attributes = NULL;
  gavl_hw_context_t * ctx = NULL;

  const char * extensions;
  
  support_flags = GAVL_HW_SUPPORTS_VIDEO;
  
  switch(type)
    {
    case GAVL_HW_NONE:  // Autodetect
      break;
    case GAVL_HW_EGL_GLES:
      eglBindAPI(EGL_OPENGL_ES_API);
      attributes = gles_attributes;
      break;
    case GAVL_HW_EGL_GL:
      eglBindAPI(EGL_OPENGL_API);
      break;
    default:
      return NULL;
    }
  
  priv = calloc(1, sizeof(*priv));

  if(!platform) // Off-Screen renderer to DMA buffers
    {
    const char *client_extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if(strstr(client_extensions, "EGL_MESA_platform_surfaceless"))
      platform = EGL_PLATFORM_SURFACELESS_MESA;
    
    if(!platform)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "No off-screen render platform found");
      goto fail;
      }
    
    }
  
  priv->platform = platform;
  
  priv->eglGetPlatformDisplay          = (void*)eglGetProcAddress("eglGetPlatformDisplayEXT");

  /* EGL_DEFAULT_DISPLAY is NULL (hopefully forever) */
  //  if(!native_display)
  //    native_display = EGL_DEFAULT_DISPLAY;
  
  if((priv->display = priv->eglGetPlatformDisplay(platform, native_display, NULL)) == EGL_NO_DISPLAY)
    goto fail;

  if(eglInitialize(priv->display, NULL, NULL) == EGL_FALSE)
    goto fail;
  
  if(eglChooseConfig(priv->display, attrs, &priv->config, 1, &num_configs) == EGL_FALSE)
    goto fail;

  //  fprintf(stderr, "EGL Extensions: %s\n",
  //          );
          
  if((priv->context = eglCreateContext(priv->display,
                                       priv->config,
                                       EGL_NO_CONTEXT,
                                       attributes)) == EGL_NO_CONTEXT)
    goto fail;
  
  priv->surf = EGL_NO_SURFACE;

  priv->eglCreatePlatformWindowSurface = (void*)eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");
  
  priv->glEGLImageTargetTexture2DOES = (void*)eglGetProcAddress("glEGLImageTargetTexture2DOES");
  priv->eglQueryDmaBufFormatsEXT = (void*)eglGetProcAddress("eglQueryDmaBufFormatsEXT");

  /*
    
  EGLBoolean (*eglExportDMABUFImageQueryMESA)(EGLDisplay dpy,
                                              EGLImageKHR image,
                                              int *fourcc,
                                              int *num_planes,
                                              EGLuint64KHR *modifiers);

  EGLBoolean (*eglExportDMABUFImageMESA)(EGLDisplay dpy,
                                         EGLImageKHR image,
                                         int *fds,
                                         EGLint *strides,
                                         EGLint *offsets);
                                         
  */

  priv->eglExportDMABUFImageQueryMESA = (void*)eglGetProcAddress("eglExportDMABUFImageQueryMESA");
  priv->eglExportDMABUFImageMESA = (void*)eglGetProcAddress("eglExportDMABUFImageMESA");
  
  priv->eglCreateImage = (void*)eglGetProcAddress("eglCreateImageKHR");
  priv->eglDestroyImage = (void*)eglGetProcAddress("eglDestroyImageKHR");

  extensions = eglQueryString(priv->display, EGL_EXTENSIONS);

  /* Synchronization */
  if(strstr(extensions, "EGL_KHR_fence_sync"))
    {
    priv->eglCreateSyncKHR     = (void*)eglGetProcAddress("eglCreateSyncKHR");
    priv->eglClientWaitSyncKHR = (void*)eglGetProcAddress("eglClientWaitSyncKHR");
    priv->eglDestroySyncKHR    = (void*)eglGetProcAddress("eglDestroySyncKHR");
    }
  
  ctx = gavl_hw_context_create_internal(priv, &funcs, type, support_flags);

  gavl_hw_egl_set_current(ctx, NULL);
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &priv->max_texture_size);

  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Created GL context");
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Type: %s", gavl_hw_type_to_string(type));
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "GL Version: %s", glGetString(GL_VERSION) );
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION) );
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Renderer: %s", glGetString(GL_RENDERER) );
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Vendor: %s", glGetString(GL_VENDOR) );
  
  gavl_hw_egl_unset_current(ctx);
  
  return ctx;
  
  fail:

  if(priv)
    destroy_native_egl(priv);

  return NULL;
  }


int gavl_hw_egl_get_max_texture_size(gavl_hw_context_t * ctx)
  {
  egl_t * p = ctx->native;
  return p->max_texture_size;
  }

EGLSurface gavl_hw_ctx_egl_create_window_surface(gavl_hw_context_t * ctx,
                                                 void * native_window)
  {
  egl_t * p = ctx->native;
  return p->eglCreatePlatformWindowSurface(p->display, p->config, native_window, NULL);
  }

void gavl_hw_ctx_egl_destroy_surface(gavl_hw_context_t * ctx, EGLSurface surf)
  {
  egl_t * p = ctx->native;
  eglDestroySurface(p->display, surf);
  }

EGLDisplay gavl_hw_ctx_egl_get_egl_display(gavl_hw_context_t * ctx)
  {
  egl_t * p = ctx->native;
  return p->display;
  }


EGLint const surface_attribs[] =
  {
    EGL_WIDTH, 10,
    EGL_HEIGHT, 10,
    EGL_NONE
  };


void gavl_hw_egl_set_current(gavl_hw_context_t * ctx, EGLSurface surf)
  {
  egl_t * p = ctx->native;

  //  fprintf(stderr, "gavl_hw_glx_set_current %p %ld\n", ctx, drawable);
  
  if(surf == EGL_NO_SURFACE)
    {
    if(p->surf == EGL_NO_SURFACE)
      {
      p->surf = eglCreatePbufferSurface(p->display,
                                        p->config,
                                        surface_attribs); 
      }
    p->current_surf = p->surf;
    
    }
  else
    p->current_surf = surf;
  
  if(!eglMakeCurrent(p->display, p->current_surf, p->current_surf, p->context))
    {
    fprintf(stderr, "eglMakeCurrent failed: %s\n", egl_strerror(eglGetError()));
    }
  }

void gavl_hw_egl_unset_current(gavl_hw_context_t * ctx)
  {
  egl_t * p = ctx->native;

  //  fprintf(stderr, "gavl_hw_glx_unset %p\n", ctx);

  eglMakeCurrent(p->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  p->current_surf = EGL_NO_SURFACE;
  }

void gavl_hw_egl_swap_buffers(gavl_hw_context_t * ctx)
  {
  egl_t * p = ctx->native;
  
  eglSwapBuffers(p->display, p->current_surf);
 
  }

void gavl_hw_egl_wait(gavl_hw_context_t * ctx)
  {
  egl_t * p = ctx->native;

  glFlush();
  
  /* Use fence synchronization */
  if(p->eglCreateSyncKHR)
    {
    EGLSyncKHR sync = p->eglCreateSyncKHR(p->display, EGL_SYNC_FENCE_KHR, NULL);
    p->eglClientWaitSyncKHR(p->display, sync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, EGL_FOREVER);
    p->eglDestroySyncKHR(p->display, sync);
    }
  else
    {
    glFinish();
    }
  }

#ifdef HAVE_DRM
static int egl_import_dmabuf(gavl_hw_context_t * ctx,
                             const gavl_video_format_t * fmt,
                             gavl_video_frame_t * src,
                             gavl_video_frame_t * dst)
  {
  int sub_h = 1, sub_v = 1;
  egl_t * egl;
  
  EGLImageKHR image = EGL_NO_IMAGE_KHR;
  EGLint attrs[128];
  int aidx = 0;
  gavl_dmabuf_video_frame_t * dmabuf = src->storage;

  gavl_gl_frame_info_t * info = dst->storage;
  
  egl = ctx->native;
  
  attrs[aidx++] = EGL_WIDTH;
  attrs[aidx++] = fmt->image_width;
  attrs[aidx++] = EGL_HEIGHT;
  attrs[aidx++] = fmt->image_height;

  attrs[aidx++] = EGL_LINUX_DRM_FOURCC_EXT;
  attrs[aidx++] = dmabuf->fourcc;
#if 1
  fprintf(stderr, "Importing frame: %c%c%c%c %dx%d strides: %d, %d %d\n",
          (dmabuf->fourcc) & 0xff,
          (dmabuf->fourcc >> 8) & 0xff,
          (dmabuf->fourcc >> 16) & 0xff,
          (dmabuf->fourcc >> 24) & 0xff, fmt->image_width, fmt->image_height, src->strides[0],
          (int)(src->planes[2] - src->planes[1]),
          (int)(src->planes[1] - src->planes[0]));
#endif
  if(gavl_pixelformat_is_yuv(fmt->pixelformat))
    {
    attrs[aidx++] = EGL_SAMPLE_RANGE_HINT_EXT;
    if(gavl_pixelformat_is_jpeg_scaled(fmt->pixelformat))
      attrs[aidx++] = EGL_YUV_FULL_RANGE_EXT;
    else
      attrs[aidx++] = EGL_YUV_NARROW_RANGE_EXT;
    }
  
  /* Chroma placement */

  gavl_pixelformat_chroma_sub(fmt->pixelformat, &sub_h, &sub_v);
  
  if((sub_h == 2) &&
     (sub_v == 2))
    {

    if(fmt->chroma_placement == GAVL_CHROMA_PLACEMENT_DEFAULT)
      {
      attrs[aidx++] = EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT;
      attrs[aidx++] = EGL_YUV_CHROMA_SITING_0_5_EXT;
      attrs[aidx++] = EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT;
      attrs[aidx++] = EGL_YUV_CHROMA_SITING_0_5_EXT;
      }
    else
      {
      attrs[aidx++] = EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT;
      attrs[aidx++] = EGL_YUV_CHROMA_SITING_0_EXT;
      attrs[aidx++] = EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT;
      attrs[aidx++] = EGL_YUV_CHROMA_SITING_0_EXT;
      }
    }
  
  // 
  // EGL_YUV_CHROMA_SITING_0_EXT
  // 
  
  /* Plane 0 */
  attrs[aidx++] = EGL_DMA_BUF_PLANE0_FD_EXT;
  attrs[aidx++] = dmabuf->buffers[dmabuf->planes[0].buf_idx].fd;
  
  attrs[aidx++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
  attrs[aidx++] = dmabuf->planes[0].offset;
  
  attrs[aidx++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
  attrs[aidx++] = src->strides[0];

  if(dmabuf->planes[0].modifiers)
    {
    attrs[aidx++] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
    attrs[aidx++] = (dmabuf->planes[0].modifiers >> 32);

    attrs[aidx++] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
    attrs[aidx++] = (dmabuf->planes[0].modifiers & (uint64_t)0xffffffff);
    }
  
  if(dmabuf->num_planes > 1)
    {
    /* Plane 1 */
    attrs[aidx++] = EGL_DMA_BUF_PLANE1_FD_EXT;
    attrs[aidx++] = dmabuf->buffers[dmabuf->planes[1].buf_idx].fd;

    attrs[aidx++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
    attrs[aidx++] = dmabuf->planes[1].offset;
      
    attrs[aidx++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
    attrs[aidx++] = src->strides[1];

    if(dmabuf->planes[1].modifiers)
      {
      attrs[aidx++] = EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT;
      attrs[aidx++] = (dmabuf->planes[1].modifiers >> 32);

      attrs[aidx++] = EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT;
      attrs[aidx++] = (dmabuf->planes[1].modifiers & (uint64_t)0xffffffff);
      }
    
    }

  if(dmabuf->num_planes > 2)
    {
    /* Plane 2 */
    attrs[aidx++] = EGL_DMA_BUF_PLANE2_FD_EXT;
    attrs[aidx++] = dmabuf->buffers[dmabuf->planes[2].buf_idx].fd;

    attrs[aidx++] = EGL_DMA_BUF_PLANE2_OFFSET_EXT;
    attrs[aidx++] = dmabuf->planes[2].offset;
      
    attrs[aidx++] = EGL_DMA_BUF_PLANE2_PITCH_EXT;
    attrs[aidx++] = src->strides[2];

    if(dmabuf->planes[2].modifiers)
      {
      attrs[aidx++] = EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT;
      attrs[aidx++] = (dmabuf->planes[2].modifiers >> 32);

      attrs[aidx++] = EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT;
      attrs[aidx++] = (dmabuf->planes[2].modifiers & (uint64_t)0xffffffff);
      }
    }
  
  /* Terminate attributes */

  attrs[aidx++] = EGL_NONE;

  eglGetError();
  
  image = egl->eglCreateImage(egl->display, EGL_NO_CONTEXT,
                              EGL_LINUX_DMA_BUF_EXT,
                              NULL, attrs);
  
  if(!image)
    {
    fprintf(stderr, "Creating Image failed %s\n", egl_strerror(eglGetError()));
    return 0;
    }
  gavl_hw_egl_set_current(ctx, EGL_NO_SURFACE);
  
  /* Associate the texture with the dma buffer */

  info->num_textures = 1;
  info->texture_target = GL_TEXTURE_EXTERNAL_OES;

  glGenTextures(1, &info->textures[0]);
  
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, info->textures[0]);
  egl->glEGLImageTargetTexture2DOES (GL_TEXTURE_EXTERNAL_OES, image);
  
  /* Destroy image */

  egl->eglDestroyImage(egl->display, image);
  
  gavl_hw_egl_unset_current(ctx);
  
  return 1;
  
  }
#endif

static const char* egl_strerror(EGLint error) {
    switch (error) {
        case EGL_SUCCESS:
            return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED:
            return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS:
            return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC:
            return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE:
            return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONTEXT:
            return "EGL_BAD_CONTEXT";
        case EGL_BAD_CONFIG:
            return "EGL_BAD_CONFIG";
        case EGL_BAD_CURRENT_SURFACE:
            return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY:
            return "EGL_BAD_DISPLAY";
        case EGL_BAD_SURFACE:
            return "EGL_BAD_SURFACE";
        case EGL_BAD_MATCH:
            return "EGL_BAD_MATCH";
        case EGL_BAD_PARAMETER:
            return "EGL_BAD_PARAMETER";
        case EGL_BAD_NATIVE_PIXMAP:
            return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW:
            return "EGL_BAD_NATIVE_WINDOW";
        case EGL_CONTEXT_LOST:
            return "EGL_CONTEXT_LOST";
        default:
            return "Unknown EGL error";
    }
}

