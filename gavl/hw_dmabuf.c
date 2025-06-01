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
#include <unistd.h> // close()
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/compression.h>
#include <gavl/metatags.h>
#include <gavl/hw.h>
#include <gavl/hw_dmabuf.h>
#include <hw_private.h>
#include <gavl/utils.h>
#include <gavl/gavldsp.h>
#include <gavl/log.h>
#define LOG_DOMAIN "dmabuf"

#ifdef HAVE_DRM

#ifdef HAVE_LINUX_DMA_HEAP_H
#include <linux/dma-heap.h>
#endif


typedef struct
  {
  char * heap_device;

  uint32_t * supported_formats;
  gavl_dsp_context_t * dsp;
  
  } dma_native_t;

static const struct
  {
  uint32_t drm_fourcc;
  gavl_pixelformat_t pfmt;
  int flags;
  int drm_indices[4];
  }
drm_formats[] =
  {
    { DRM_FORMAT_RGB888,   GAVL_RGB_24                                   },
    { DRM_FORMAT_BGR888,   GAVL_BGR_24                                   },
    { DRM_FORMAT_RGBA8888, GAVL_RGBA_32                                  },
    { DRM_FORMAT_RGB565,   GAVL_RGB_16                                   },
    { DRM_FORMAT_YUYV,     GAVL_YUY2                                     },
    { DRM_FORMAT_UYVY,     GAVL_UYVY                                     },
    { DRM_FORMAT_YUV411,   GAVL_YUV_411_P                                },
    { DRM_FORMAT_YVU411,   GAVL_YUV_411_P, GAVL_DMABUF_FLAG_SWAP_CHROMA  },
    { DRM_FORMAT_YUV420,   GAVL_YUV_420_P                                },
    { DRM_FORMAT_YVU420,   GAVL_YUV_420_P, GAVL_DMABUF_FLAG_SWAP_CHROMA  },
    { DRM_FORMAT_YUV410,   GAVL_YUV_410_P                                },
    { DRM_FORMAT_YVU410,   GAVL_YUV_410_P, GAVL_DMABUF_FLAG_SWAP_CHROMA  },
    { DRM_FORMAT_YUV422,   GAVL_YUV_422_P                                },
    { DRM_FORMAT_YVU422,   GAVL_YUV_422_P, GAVL_DMABUF_FLAG_SWAP_CHROMA  },
    { DRM_FORMAT_YUV444,   GAVL_YUV_444_P                                },
    { DRM_FORMAT_YVU444,   GAVL_YUV_444_P, GAVL_DMABUF_FLAG_SWAP_CHROMA  },
    { DRM_FORMAT_YUV420,   GAVL_YUVJ_420_P                               },
    { DRM_FORMAT_YVU420,   GAVL_YUVJ_420_P, GAVL_DMABUF_FLAG_SWAP_CHROMA },
    { DRM_FORMAT_YUV422,   GAVL_YUVJ_422_P                               },
    { DRM_FORMAT_YVU422,   GAVL_YUVJ_422_P, GAVL_DMABUF_FLAG_SWAP_CHROMA },
    { DRM_FORMAT_YUV444,   GAVL_YUVJ_444_P                               },
    { DRM_FORMAT_YVU444,   GAVL_YUVJ_444_P, GAVL_DMABUF_FLAG_SWAP_CHROMA },
    //    { DRM_FORMAT_ABGR16161616F, GAVL_RGBA_64, GAVL_DMABUF_FLAG_SHUFFLE, { 3, 2, 1, 0    } },
    //    { DRM_FORMAT_ABGR16161616,  GAVL_RGBA_64, GAVL_DMABUF_FLAG_SHUFFLE, { 3, 2, 1, 0    } },
    //    { DRM_FORMAT_ARGB8888,      GAVL_RGBA_32, GAVL_DMABUF_FLAG_SHUFFLE, { 1, 2, 3, 0    } },
    //    { DRM_FORMAT_ABGR8888,      GAVL_RGBA_32, GAVL_DMABUF_FLAG_SHUFFLE, { 3, 2, 1, 0    } },
    { DRM_FORMAT_XRGB8888,      GAVL_BGR_32,  }, // GAVL_DMABUF_FLAG_SHUFFLE, { 1, 2, 3, 0x80 } },
    { DRM_FORMAT_XBGR8888,      GAVL_RGB_32,  }, // GAVL_DMABUF_FLAG_SHUFFLE, { 1, 2, 3, 0x80 } },

    { DRM_FORMAT_XRGB8888,      GAVL_RGB_32, GAVL_DMABUF_FLAG_SHUFFLE, { 2, 1, 0, 3 } },
    { DRM_FORMAT_XBGR8888,      GAVL_BGR_32, GAVL_DMABUF_FLAG_SHUFFLE, { 2, 1, 0, 3 } },
    
    { DRM_FORMAT_AYUV,          GAVL_YUVA_32, GAVL_DMABUF_FLAG_SHUFFLE, { 2, 1, 0, 3    } },
    { DRM_FORMAT_AVUY8888,      GAVL_YUVA_32, GAVL_DMABUF_FLAG_SHUFFLE, { 3, 2, 1, 0    } }, // TODO: Check byte order

    /* [63:0] A:Cr:Y:Cb 16:16:16:16 little endian */
    { DRM_FORMAT_Y416,          GAVL_YUVA_64, GAVL_DMABUF_FLAG_SHUFFLE, { 1, 0, 2, 3    } }, // TODO: Check word order
    { /* */ }
  };

uint32_t gavl_drm_fourcc_from_gavl(gavl_hw_context_t * ctx, gavl_pixelformat_t pfmt)
  {
  int i = 0;

  while(drm_formats[i].drm_fourcc)
    {
    if(drm_formats[i].pfmt == pfmt)
      return drm_formats[i].drm_fourcc;
    i++;
    }
  return 0;
  }

gavl_pixelformat_t gavl_drm_pixelformat_from_fourcc(uint32_t fourcc, int * flags, int * drm_indices)
  {
  int i = 0;

  while(drm_formats[i].drm_fourcc)
    {
    if(drm_formats[i].drm_fourcc == fourcc)
      {
      if(flags)
        *flags = drm_formats[i].flags;
      if(drm_indices)
        memcpy(drm_indices, drm_formats[i].drm_indices, sizeof(drm_formats[i].drm_indices));
      return drm_formats[i].pfmt;
      }
    i++;
    }
  return GAVL_PIXELFORMAT_NONE;
  }

static int video_frame_map_dmabuf(gavl_video_frame_t * f, int wr)
  {
  int i;
  gavl_dmabuf_video_frame_t *dmabuf = f->storage;

  if(!f->planes[0])
    {
    int prot = PROT_READ;
    if(wr)
      prot |= PROT_WRITE;
  
    for(i = 0; i < dmabuf->num_buffers; i++)
      {
      dmabuf->buffers[i].map_ptr =
        mmap(NULL, dmabuf->buffers[i].map_len, prot, MAP_SHARED,
             dmabuf->buffers[i].fd, 0);

      if(dmabuf->buffers[i].map_ptr == MAP_FAILED)
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Memory mmapping failed: %s",
                 strerror(errno));
        return 0;
        }
      }
  
    for(i = 0; i < dmabuf->num_planes; i++)
      {
      /* Set plane pointer */
      f->strides[i] = dmabuf->planes[i].stride;
      f->planes[i] = dmabuf->buffers[dmabuf->planes[i].buf_idx].map_ptr +
        dmabuf->planes[i].offset;
      }
    }
  
  
  return 1;
  }

static int
video_frame_unmap_dmabuf(gavl_video_frame_t * f)
  {
#if 0 // TODO: Handle R/W sync
  gavl_dmabuf_video_frame_t *dmabuf = f->storage;
  int i;
  for(i = 0; i < dmabuf->num_buffers; i++)
    {
    if(dmabuf->buffers[i].map_ptr)
      {
      munmap(dmabuf->buffers[i].map_ptr, dmabuf->buffers[i].map_len);
      dmabuf->buffers[i].map_ptr = NULL;
      }
    }
  gavl_video_frame_null(f);
#endif
  return 1;
  }


static gavl_video_frame_t * video_frame_create_hw_dmabuf(gavl_hw_context_t * ctx,
                                                         int alloc_resources)
  {
  gavl_video_frame_t * ret;
  gavl_dmabuf_video_frame_t *f = calloc(1, sizeof(*f));
  int i;
  
  for(i = 0; i < GAVL_MAX_PLANES; i++)
    f->buffers[i].fd = -1;
  
  ret = gavl_video_frame_create(NULL);
  ret->storage = f;
  
  if(alloc_resources)
    {
    int heap_fd;
    dma_native_t * native = ctx->native;
    struct dma_heap_allocation_data heap_data;

    int offsets[GAVL_MAX_PLANES];
    int size;

    f->fourcc = gavl_drm_fourcc_from_gavl(ctx, ctx->vfmt.pixelformat);
    
    gavl_video_format_get_frame_layout(&ctx->vfmt,
                                       offsets,
                                       ret->strides,
                                       &size,
                                       0);
    memset(&heap_data, 0, sizeof(heap_data));
    heap_data.len = size;
    heap_data.fd_flags = O_RDWR | O_CLOEXEC;
    
    if((heap_fd = open(native->heap_device, O_RDONLY | O_CLOEXEC)) < 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Opening %s failed: %s",
               native->heap_device, strerror(errno));
      goto fail;
      }

    if(ioctl(heap_fd, DMA_HEAP_IOCTL_ALLOC, &heap_data) < 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
               "Allocating %d bytes from %s failed: %s",
               size, native->heap_device, strerror(errno));
      goto fail;
      }
    
    close(heap_fd);
    f->buffers[0].fd = heap_data.fd;
    f->buffers[0].map_len = size;
    f->num_buffers = 1;
    
    f->num_planes = gavl_pixelformat_num_planes(ctx->vfmt.pixelformat);

    for(i = 0; i < f->num_planes; i++)
      {
      f->planes[i].buf_idx = 0;
      f->planes[i].offset = offsets[i];
      f->planes[i].stride = ret->strides[i];
      }

    /* Write to the memory once to make sure it's there */
    video_frame_map_dmabuf(ret, 1);
    gavl_video_frame_clear(ret, &ctx->vfmt);
    video_frame_unmap_dmabuf(ret);
    
    }

  return ret;

  fail:

  gavl_video_frame_destroy(ret);
  return NULL;
  
  }


static void
video_frame_destroy_hw_dmabuf(gavl_video_frame_t * f,
                              int destroy_resources)
  {
  if(f->storage)
    {
    int i;
    gavl_dmabuf_video_frame_t * info;
    
    video_frame_unmap_dmabuf(f);
    
    info = f->storage;

    for(i = 0; i < GAVL_MAX_PLANES; i++)
      {
      if(info->buffers[i].map_ptr)
        munmap(info->buffers[i].map_ptr, info->buffers[i].map_len);
      
      /* Even imported dma-bufs must be closed */
      if(info->buffers[i].fd >= 0)
        close(info->buffers[i].fd);
      }
    free(info);
    }
  
  f->hwctx = NULL;
  gavl_video_frame_null(f);
  gavl_video_frame_destroy(f);
  }

static void destroy_native_dmabuf(void * n)
  {
  dma_native_t * native = n;
  if(native->heap_device)
    free(native->heap_device);
  free(native);
  }

static int frame_to_ram_dmabuf(gavl_video_frame_t * dst,
                               gavl_video_frame_t * src)
  {

  return 1;
  }

static int frame_to_hw_dmabuf(gavl_video_frame_t * dst,
                              gavl_video_frame_t * src)
  {
  int i;
  int flags = 0;
  int drm_indices[GAVL_MAX_PLANES];

  dma_native_t * native = dst->hwctx->native;
  gavl_dmabuf_video_frame_t * fp = dst->storage;
  gavl_drm_pixelformat_from_fourcc(fp->fourcc, &flags, drm_indices);
  gavl_hw_video_frame_map(dst, 1);
  
  if(flags & GAVL_DMABUF_FLAG_SHUFFLE)
    {
    int reverse[GAVL_MAX_PLANES];
    if(!native->dsp)
      native->dsp = gavl_dsp_context_create();

    for(i = 0; i < GAVL_MAX_PLANES; i++)
      reverse[ drm_indices[i] % 0x80 ] = i;
    
    gavl_dsp_video_frame_shuffle_4_copy(native->dsp, dst,
                                        src,
                                        &dst->hwctx->vfmt,
                                        // drm_indices
                                        reverse
                                        );
    }
  else
    {
    gavl_video_frame_copy(&dst->hwctx->vfmt, dst, src);
    }
  
  gavl_hw_video_frame_unmap(dst);
  
  return 1;
  }

static const gavl_hw_funcs_t funcs =
  {
    .destroy_native         = destroy_native_dmabuf,
   //    .get_image_formats      = gavl_gl_get_image_formats,
   //    .get_overlay_formats    = gavl_gl_get_overlay_formats,
   .video_frame_create     = video_frame_create_hw_dmabuf,
   .video_frame_destroy    = video_frame_destroy_hw_dmabuf,

   .video_frame_map        = video_frame_map_dmabuf,
   .video_frame_unmap        = video_frame_unmap_dmabuf,
   
   .video_frame_to_ram     = frame_to_ram_dmabuf,
   .video_frame_to_hw      = frame_to_hw_dmabuf,

   //    .video_format_adjust    = gavl_gl_adjust_video_format,
   //    .overlay_format_adjust  = gavl_gl_adjust_video_format,
   //    .can_export             = exports_type_v4l2,
   //    .export_video_frame = export_video_frame_v4l2,
  };

static int test_heap_dev(dma_native_t * native, const char * dev)
  {
  if(!access(dev, R_OK | W_OK))
    {
    native->heap_device = gavl_strdup(dev);
    return 1;
    }
  else
    return 0;
  }

gavl_hw_context_t * gavl_hw_ctx_create_dma()
  {
  dma_native_t * native;
  int support_flags = GAVL_HW_SUPPORTS_VIDEO;

  native = calloc(1, sizeof(*native));

#ifdef HAVE_LINUX_DMA_HEAP_H  
  if(test_heap_dev(native, "/dev/dma_heap/linux,cma") ||
     test_heap_dev(native, "/dev/dma_heap/system"))
    support_flags |= GAVL_HW_SUPPORTS_VIDEO_POOL | GAVL_HW_SUPPORTS_VIDEO_MAP;
#endif
  
  return gavl_hw_context_create_internal(native, &funcs, GAVL_HW_DMABUFFER, support_flags);
  }

void gavl_hw_ctx_dma_set_supported_formats(gavl_hw_context_t * ctx, uint32_t * formats)
  {
  int num;
  int i, j;
  dma_native_t * native = ctx->native;

  if(native->supported_formats)
    free(native->supported_formats);

  num = 0;
  i = 0;
  while(drm_formats[i].drm_fourcc)
    {
    j = 0;
    while(formats[j])
      {
      if(formats[j] == drm_formats[i].drm_fourcc)
        {
        num++;
        break;
        }
      j++;
      }
    i++;
    }

  native->supported_formats = calloc(num+1, sizeof(*native->supported_formats));

  num = 0;
  i = 0;
  while(drm_formats[i].drm_fourcc)
    {
    j = 0;
    while(formats[j])
      {
      if(formats[j] == drm_formats[i].drm_fourcc)
        {
        native->supported_formats[num] = drm_formats[i].drm_fourcc;
        num++;
        break;
        }
      j++;
      }
    i++;
    }
  
  }

#else // No DRM
gavl_hw_context_t * gavl_hw_ctx_create_dma()
  {
  return NULL;
  }

#endif
