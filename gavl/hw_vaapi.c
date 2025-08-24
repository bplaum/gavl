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



#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>

// #include <vaapi.h>

#include <va/va_drm.h>

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/hw_vaapi.h>
#include <gavl/metatags.h>
#include <gavl/trackinfo.h>
#include <gavl/log.h>
#define LOG_DOMAIN "hw_vaapi"

#include <hw_private.h>


#ifdef HAVE_DRM
#include <va/va_drmcommon.h>
#include <gavl/hw_dmabuf.h>
#endif

typedef struct
  {
  VADisplay dpy; // Must be first

  int num_image_formats;
  VAImageFormat * image_formats;
  
  int no_derive;

  int num_imported_frames;

  int drm_fd;
  
  } gavl_hw_vaapi_t;


static struct
  {
  gavl_pixelformat_t pfmt;
  uint32_t fourcc;
  unsigned int rt_format;
  }
formats[] =
  {
#if 0
    { GAVL_RGB_32,     VA_FOURCC('R','G','B','X'), VA_RT_FORMAT_RGB32 },
    { GAVL_BGR_32,     VA_FOURCC('B','G','R','X'), VA_RT_FORMAT_RGB32 },
#endif
    { GAVL_RGBA_32,    VA_FOURCC('R','G','B','A'), VA_RT_FORMAT_RGB32 },
    { GAVL_RGBA_32,    VA_FOURCC('B','G','R','A'), VA_RT_FORMAT_RGB32 },
    { GAVL_YUV_420_P,  VA_FOURCC('I','4','2','0'), VA_RT_FORMAT_YUV420 },
    { GAVL_YUV_420_P,  VA_FOURCC('Y','V','1','2'), VA_RT_FORMAT_YUV420 },
    { GAVL_PIXELFORMAT_NONE, 0, 0 }, // End
  };

static gavl_pixelformat_t fourcc_to_pixelformat(uint32_t fourcc)
  {
  int i = 0;
  while(formats[i].pfmt != GAVL_PIXELFORMAT_NONE)
    {
    if(formats[i].fourcc == fourcc)
      return formats[i].pfmt;
    i++;
    }
  return GAVL_PIXELFORMAT_NONE;
  }

static unsigned int pixelformat_to_rt_format(gavl_pixelformat_t pfmt)
  {
  int i = 0;
  while(formats[i].pfmt != GAVL_PIXELFORMAT_NONE)
    {
    if(formats[i].pfmt == pfmt)
      return formats[i].rt_format;
    i++;
    }
  return 0;
  }

static VAImageFormat * pixelformat_to_image_format(gavl_hw_vaapi_t * priv,
                                                   gavl_pixelformat_t pfmt, 
                                                   VAImageFormat * fmts, int num_fmts)
  {
  int i;

  /* Try to find identical masks */

  const uint32_t * masks = gavl_pixelformat_get_masks(pfmt);
  
  if(masks)
    {
    if(gavl_pixelformat_has_alpha(pfmt))
      {
      for(i = 0; i < num_fmts; i++)
        {
        if(fourcc_to_pixelformat(fmts[i].fourcc) != pfmt)
          continue;

        if((masks[0] == fmts[i].red_mask) &&
           (masks[1] == fmts[i].green_mask) &&
           (masks[2] == fmts[i].blue_mask) &&
           (masks[3] == fmts[i].alpha_mask))
          return &fmts[i];
        }
      
      }
    else
      {
      for(i = 0; i < num_fmts; i++)
        {
        if(fourcc_to_pixelformat(fmts[i].fourcc) != pfmt)
          continue;

        if((masks[0] == fmts[i].red_mask) &&
           (masks[1] == fmts[i].green_mask) &&
           (masks[2] == fmts[i].blue_mask))
          return &fmts[i];
        }
      }
    }
  
  for(i = 0; i < num_fmts; i++)
    {
    if(fourcc_to_pixelformat(fmts[i].fourcc) == pfmt)
      return &fmts[i];
    }
  return NULL;
  }

static gavl_video_frame_t * create_common(gavl_hw_context_t * ctx)
  {
  gavl_video_frame_t * ret;
  ret = gavl_video_frame_create(NULL);
  ret->hwctx = ctx;
  return ret;
  }

static gavl_video_frame_t * gavl_vaapi_video_frame_create(gavl_hw_context_t * ctx, int alloc_resource)
  {
  VAStatus result;
  gavl_video_frame_t * ret = NULL;
  unsigned int rt_format;
  //  VASurfaceID * surf = NULL;
  /* Create surface */
  gavl_vaapi_video_frame_t * fp = NULL;

  gavl_hw_vaapi_t * priv = ctx->native;

  if(alloc_resource && !(rt_format = pixelformat_to_rt_format(ctx->vfmt.pixelformat)))
    goto fail;
  
  fp = calloc(1, sizeof(*fp));

  fp->surface      = VA_INVALID_ID;
  fp->image.image_id = VA_INVALID_ID;
  
  if(alloc_resource)
    {
    if((result = vaCreateSurfaces(priv->dpy, rt_format, ctx->vfmt.image_width,
                                  ctx->vfmt.image_height, &fp->surface, 1, NULL, 0)) != VA_STATUS_SUCCESS)
      goto fail;
    }
  
  ret = create_common(ctx);
  ret->storage = fp;
  
  return ret;

  fail:

  if(ret)
    {
    ret->hwctx = NULL;
    gavl_video_frame_destroy(ret);
    }
  
  if(fp)
    {
    free(fp);
    }
  
  return NULL;
  }


int gavl_vaapi_map_frame(gavl_video_frame_t * f, int wr)
  {
  void * buf;
  uint8_t * buf_i;

  gavl_vaapi_video_frame_t * fp = f->storage;
  gavl_hw_vaapi_t * priv = f->hwctx->native;
  VAStatus result;

  fp->wr = wr;
  
  /* Need to generate image */
  if(fp->image.image_id == VA_INVALID_ID)
    {
    if(!priv->no_derive)
      {
      if((result = vaDeriveImage(priv->dpy,
                                 fp->surface,
                                 &fp->image)) != VA_STATUS_SUCCESS)
        {
        // fprintf(stderr, "vaDeriveImage Failed: %s\n", vaErrorStr(result));
        priv->no_derive = 1;
        }
      }

    if(priv->no_derive)
      {
      VAImageFormat * format;
      if(!(format = pixelformat_to_image_format(priv, f->hwctx->vfmt.pixelformat,
                                                priv->image_formats, priv->num_image_formats)))
        return 0;
      
      if((result = vaCreateImage(priv->dpy,
                                 format,
                                 f->hwctx->vfmt.image_width,
                                 f->hwctx->vfmt.image_height,
                                 &fp->image)) != VA_STATUS_SUCCESS)
        return 0;
      }
    }

  /* Copy surface to image */

  if(priv->no_derive)
    {
    if((result = vaGetImage(priv->dpy,
                            fp->surface,
                            0,
                            0,
                            f->hwctx->vfmt.image_width,
                            f->hwctx->vfmt.image_height,
                            fp->image.image_id)) != VA_STATUS_SUCCESS)
      {
      // fprintf(stderr, " failed: %s\n", vaErrorStr(result));
      return 0;
      }
    }
  
  /* Map image */
  if((result = vaMapBuffer(priv->dpy, fp->image.buf, &buf)) != VA_STATUS_SUCCESS)
    {
    return 0;
    }


  buf_i = buf;

  f->planes[0] = buf_i + fp->image.offsets[0];
  f->strides[0] = fp->image.pitches[0];
  
  if(fp->image.format.fourcc == VA_FOURCC('Y','V','1','2'))
    {
    f->planes[2] = buf_i + fp->image.offsets[1];
    f->strides[2] = fp->image.pitches[1];
  
    f->planes[1] = buf_i + fp->image.offsets[2];
    f->strides[1] = fp->image.pitches[2];
    }
  else if(fp->image.format.fourcc == VA_FOURCC('N','V','1','2'))
    {
    f->planes[1] = buf_i + fp->image.offsets[1];
    f->strides[1] = fp->image.pitches[1];
    
    f->planes[2] = buf_i + fp->image.offsets[1] + fp->image.pitches[1] / 2;
    f->strides[2] = fp->image.pitches[1];
    
    }
  else if((fp->image.num_planes > 1) &&
          (fp->image.offsets[1] + fp->image.pitches[1]))
    {
    f->planes[1] = buf_i + fp->image.offsets[1];
    f->strides[1] = fp->image.pitches[1];
  
    f->planes[2] = buf_i + fp->image.offsets[2];
    f->strides[2] = fp->image.pitches[2];
    }
  
  return 1;
  }

/* Unmap */
int gavl_vaapi_unmap_frame(gavl_video_frame_t * f)
  {
  gavl_hw_vaapi_t * priv = f->hwctx->native;
  gavl_vaapi_video_frame_t * fp = f->storage;
  VAStatus result;

  vaUnmapBuffer(priv->dpy, fp->image.buf);

  /* TODO: Copy back to surface */

  if(fp->wr)
    {
    if((result = vaPutImage(priv->dpy,
                            fp->surface,
                            fp->image.image_id,
                            0, // int  	src_x,
                            0, // int  	src_y,
                            f->hwctx->vfmt.image_width, // unsigned int  	src_width,
                            f->hwctx->vfmt.image_height, // unsigned int  	src_height,
                            0, // int  	dest_x,
                            0, // int  	dest_y,
                            f->hwctx->vfmt.image_width, // unsigned int  	dest_width,
                            f->hwctx->vfmt.image_height)) != VA_STATUS_SUCCESS) //unsigned int  	dest_height 
      {
      // fprintf(stderr, " failed: %s\n", vaErrorStr(result));
      return 0;
      }
    }
  vaDestroyImage(priv->dpy, fp->image.image_id);
  fp->image.image_id = VA_INVALID_ID;
  
  gavl_video_frame_null(f);
  return 1;
  }

static void gavl_vaapi_video_frame_destroy(gavl_video_frame_t * f, int destroy_resource)
  {
  gavl_hw_vaapi_t * priv = f->hwctx->native;
  
  gavl_vaapi_video_frame_t * image;
  image = f->storage;
  
  if(image->image.image_id != VA_INVALID_ID)
    vaDestroyImage(priv->dpy, image->image.image_id);
  
  if(destroy_resource)
    {
    if(image->surface != VA_INVALID_ID)
      vaDestroySurfaces(priv->dpy, &image->surface, 1);
    }
  free(image);
  gavl_video_frame_null(f);
  f->hwctx = NULL;
  gavl_video_frame_destroy(f);
  }

#ifdef DUMP_FORMATS
static void dump_image_format(const VAImageFormat * f)
  {
  fprintf(stderr, "  fourcc: %c%c%c%c\n",
          (f->fourcc) & 0xff,
          (f->fourcc >> 8) & 0xff,
          (f->fourcc >> 16) & 0xff,
          (f->fourcc >> 24) & 0xff);
  fprintf(stderr, "  byte_order:     %d\n", f->byte_order);
  fprintf(stderr, "  bits_per_pixel: %d\n", f->bits_per_pixel);
  fprintf(stderr, "  depth:          %d\n", f->depth);
  fprintf(stderr, "  red_mask:       %08x\n", f->red_mask);
  fprintf(stderr, "  green_mask:     %08x\n", f->green_mask);
  fprintf(stderr, "  blue_mask:      %08x\n", f->blue_mask);
  fprintf(stderr, "  alpha_mask:     %08x\n", f->alpha_mask);
  }
#endif

static gavl_pixelformat_t *
gavl_vaapi_get_image_formats(gavl_hw_context_t * ctx, gavl_hw_frame_mode_t mode)
  {
  int num, i;
  gavl_pixelformat_t * ret;
  gavl_hw_vaapi_t * p = ctx->native;
  int num_ret = 0;
  gavl_pixelformat_t fmt;
  int j;
  int take_it;
  
  num = vaMaxNumImageFormats(p->dpy);
  p->image_formats = malloc(num * sizeof(*p->image_formats));

  vaQueryImageFormats(p->dpy, p->image_formats, &p->num_image_formats);

  ret = calloc(p->num_image_formats + 1, sizeof(*ret));
  
  for(i = 0; i < p->num_image_formats; i++)
    {
#ifdef DUMP_FORMATS
    fprintf(stderr, "Image Format %d\n", i);
    dump_image_format(&p->image_formats[i]);
#endif
    if((fmt = fourcc_to_pixelformat(p->image_formats[i].fourcc)) == GAVL_PIXELFORMAT_NONE)
      continue;

    j = 0;
    take_it = 1;
    
    while(j < num_ret)
      {
      if(ret[j] == fmt)
        {
        take_it = 0;
        break;
        }
      j++;
      }

    if(take_it)
      {
      ret[num_ret] = fmt;
      num_ret++;
      }
    }
  ret[num_ret] = GAVL_PIXELFORMAT_NONE;
  return ret;
  }


static void gavl_vaapi_cleanup(void * priv)
  {
  gavl_hw_vaapi_t * p = priv;
  if(p->image_formats)
    free(p->image_formats);
  if(p->dpy)
    vaTerminate(p->dpy);

  if(p->drm_fd >= 0)
    close(p->drm_fd);
  free(p);
  }

VASurfaceID gavl_vaapi_get_surface_id(const gavl_video_frame_t * f)
  {
  VASurfaceID * surf = f->storage;
  return *surf;
  }

/* Use only in specific create routines */
void gavl_vaapi_set_surface_id(gavl_video_frame_t * f, VASurfaceID id)
  {
  VASurfaceID * surf;
  if(!f->storage)
    {
    surf = malloc(sizeof(*surf));
    f->storage = surf;
    }
  else
    surf = f->storage;
  
  *surf = id;
  }

#if 0
VASubpictureID gavl_vaapi_get_subpicture_id(const gavl_video_frame_t * f)
  {
  vaapi_frame_t * frame = f->storage;
  return frame->ovl;
  }
#endif

VAImageID gavl_vaapi_get_image_id(const gavl_video_frame_t * f)
  {
  gavl_vaapi_video_frame_t * frame = f->storage;
  return frame->image.image_id;
  }
  
static void gavl_vaapi_video_format_adjust(gavl_hw_context_t * ctx,
                                    gavl_video_format_t * fmt, gavl_hw_frame_mode_t mode)
  {
  gavl_video_format_set_frame_size(fmt, 16, 16);
  }

typedef struct
  {
  VAProfile va;
  const char * gavl;
  } profile_map_t;  

static const profile_map_t h264_profiles[] =
  {
    //    { VAProfileH264Baseline, GAVL_META_H264_PROFILE_BASELINE             },
    //    { VAProfileH264Baseline, GAVL_META_H264_PROFILE_CONSTRAINED_BASELINE },
    { VAProfileH264Main,     GAVL_META_H264_PROFILE_MAIN                 },
    
    { VAProfileH264High,     GAVL_META_H264_PROFILE_HIGH },
    { VAProfileH264High,     GAVL_META_H264_PROFILE_CONSTRAINED_HIGH },
    { },
  };

static const profile_map_t mpeg2_profiles[] =
  {
    { VAProfileMPEG2Simple, GAVL_META_MPEG2_LEVEL_LOW },
    { VAProfileMPEG2Main, GAVL_META_MPEG2_LEVEL_MAIN },
    { },
  };

static const profile_map_t mpeg4_profiles[] =
  {
    { VAProfileMPEG4Simple,         GAVL_META_MPEG4_PROFILE_SIMPLE          },
    { VAProfileMPEG4AdvancedSimple, GAVL_META_MPEG4_PROFILE_ADVANCED_SIMPLE },
    { VAProfileMPEG4Main,           GAVL_META_MPEG4_PROFILE_MAIN            },
    { }
  };
    
VAProfile gavl_vaapi_get_profile(const gavl_dictionary_t * dict)
  {
  int i;
  const char * profile;
  const profile_map_t * map = NULL;
  gavl_codec_id_t id;
  gavl_compression_info_t ci;
  gavl_compression_info_init(&ci);
  
  if(!gavl_stream_get_compression_info(dict, &ci))
    return VAProfileNone;
  id = ci.id;
  gavl_compression_info_free(&ci);

  switch(id)
    {
    case GAVL_CODEC_ID_MPEG2:
      map = mpeg2_profiles;
      break;
    case GAVL_CODEC_ID_MPEG4_ASP:
      map = mpeg4_profiles;
      break;
    case GAVL_CODEC_ID_H264:
      map = h264_profiles;
      break;
    default:
      break;
    }

  if(!map)
    return VAProfileNone;
  
  if(!(dict = gavl_stream_get_metadata(dict)))
    return VAProfileNone;

  if(!(profile = gavl_dictionary_get_string(dict, GAVL_META_PROFILE)))
    return VAProfileNone;
  
  i = 0;
  while(map[i].gavl)
    {
    if(!strcmp(profile, map[i].gavl))
      return map[i].va;
    i++;
    }
  
  return VAProfileNone;
  }
  
int gavl_vaapi_can_decode(VADisplay dpy, const gavl_dictionary_t * dict)
  {
  int i;
  int ret = 0;
  VAProfile profile;
  int num_entrypoints = 0;
  VAEntrypoint *entrypoints = NULL;
  
  if((profile = gavl_vaapi_get_profile(dict)) == VAProfileNone)
    return 0;

  entrypoints = calloc(vaMaxNumEntrypoints(dpy), sizeof(*entrypoints));

  vaQueryConfigEntrypoints(dpy,
                           profile,
                           entrypoints,  /* out */
                           &num_entrypoints);        /* out */

  for(i = 0; i < num_entrypoints; i++)
    {
    if(entrypoints[i] == VAEntrypointVLD)
      {
      ret = 1;
      break;
      }
    }
  
  if(entrypoints)
    free(entrypoints);

  return ret;
  
  }

static int gavl_vaapi_export_video_frame(const gavl_video_format_t * fmt,
                                  gavl_video_frame_t * src, gavl_video_frame_t * dst)
  {
  gavl_hw_vaapi_t * dev = src->hwctx->native;
  gavl_hw_type_t dst_hw_type = gavl_hw_ctx_get_type(dst->hwctx);

  switch(dst_hw_type)
    {
#ifdef HAVE_DRM
    case GAVL_HW_DMABUFFER:
      {
      int i;
      VADRMPRIMESurfaceDescriptor prime_desc;

      gavl_dmabuf_video_frame_t * dma_frame = dst->storage;

      gavl_vaapi_video_frame_t * vaapi_storage = src->storage;

      if(vaExportSurfaceHandle(dev->dpy, vaapi_storage->surface,
                               VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2,
                               VA_EXPORT_SURFACE_READ_WRITE|VA_EXPORT_SURFACE_COMPOSED_LAYERS,
                               &prime_desc) != VA_STATUS_SUCCESS)
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "vaExportSurfaceHandle failed");
        return 0;
        }

      gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Exported VASurface as DMA buffer");
      
      dma_frame->num_buffers = prime_desc.num_objects;
      for(i = 0; i < dma_frame->num_buffers; i++)
        {
        dma_frame->buffers[i].fd = prime_desc.objects[i].fd;
        dma_frame->buffers[i].map_len = prime_desc.objects[i].size;
        }
      
      dma_frame->num_planes = prime_desc.layers[0].num_planes;
      dma_frame->fourcc = prime_desc.layers[0].drm_format;
      
      for(i = 0; i < dma_frame->num_planes; i++)
        {
        dma_frame->planes[i].buf_idx = prime_desc.layers[0].object_index[i];
        dma_frame->planes[i].offset = prime_desc.layers[0].offset[i];
        dma_frame->planes[i].stride = prime_desc.layers[0].pitch[i];
        dst->strides[i] = prime_desc.layers[0].pitch[i];
        }
      dst->buf_idx = src->buf_idx;
      }
#endif
    default:
      break;
    }
  return 0;
  }

static int gavl_vaapi_exports_type(gavl_hw_context_t * ctx, const gavl_hw_context_t * other)
  {
  switch(other->type)
    {
#ifdef HAVE_DRM
    case GAVL_HW_DMABUFFER:
      {
      return 1;
      }
      break;
#endif
    default:
      break;
    }
  return 0;
  }

static const gavl_hw_funcs_t funcs =
  {
    .destroy_native = gavl_vaapi_cleanup,
    .get_image_formats = gavl_vaapi_get_image_formats,
    .video_frame_create = gavl_vaapi_video_frame_create,
    .video_frame_destroy = gavl_vaapi_video_frame_destroy,
    .video_frame_map = gavl_vaapi_map_frame,
    .video_frame_unmap = gavl_vaapi_unmap_frame,
    .video_format_adjust  = gavl_vaapi_video_format_adjust,
    .can_export = gavl_vaapi_exports_type,
    .export_video_frame = gavl_vaapi_export_video_frame,

  };



static const char * drm_devices[] =
  {
    "/dev/dri/renderD128",
    "/dev/dri/renderD129",
    "/dev/dri/card0",
    "/dev/dri/card1",
    NULL,
  };

gavl_hw_context_t * gavl_hw_ctx_create_vaapi()
  {
  int major, minor;
  gavl_hw_vaapi_t * priv;
  VAStatus result;
  int support_flags = GAVL_HW_SUPPORTS_VIDEO;
  int i = 0;
  
  priv = calloc(1, sizeof(*priv));
  priv->drm_fd = -1;
  
  /* TODO */
  while(drm_devices[i])
    {
    if((priv->drm_fd = open(drm_devices[i], O_RDWR)) >= 0)
      break;
    i++;
    }
  if(!drm_devices[i])
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "No DRM device available");
    return NULL;
    }
  
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Opened %s for VAAPI", drm_devices[i]);

  priv->dpy = vaGetDisplayDRM(priv->drm_fd);
  
  /* Deriving images doesn't work for weird formats like NV12 */
  priv->no_derive = 1;
  
  if(!priv->dpy)
    return NULL;
  
  if((result = vaInitialize(priv->dpy,
                            &major,
                            &minor)) != VA_STATUS_SUCCESS)
    {
    return NULL;
    }

  return gavl_hw_context_create_internal(priv, &funcs,
                                         GAVL_HW_VAAPI, support_flags);
  }


VADisplay gavl_hw_ctx_vaapi_get_va_display(gavl_hw_context_t * ctx)
  {
  gavl_hw_vaapi_t * p = ctx->native;
  return p->dpy;
  }
