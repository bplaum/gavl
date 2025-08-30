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
#include <xf86drm.h>
#include <xf86drmMode.h>

// #define DUMP_DEVICE_INFO




#ifdef HAVE_LINUX_DMA_BUF_H
#include <linux/dma-buf.h>
#endif

#ifdef HAVE_LINUX_DMA_HEAP_H
#include <linux/dma-heap.h>
#endif



#ifdef DUMP_DEVICE_INFO
static void print_drm_info(int fd);
#endif

static uint32_t fourcc_from_gavl(gavl_hw_context_t * ctx, gavl_pixelformat_t pfmt, int * flags);

#define BPP_8  (1<<0)
#define BPP_16 (1<<1)
#define BPP_24 (1<<2)
#define BPP_32 (1<<3)
#define BPP_64 (1<<4)

#define BPP_ALL 0xffffffff

typedef struct
  {
  char * heap_device;
  char * drm_device;
  
  uint32_t * supported_formats;
  gavl_dsp_context_t * dsp;

  int drm_fd;

  uint32_t bpp_supported;
  
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
    { DRM_FORMAT_RGB888,   GAVL_RGB_24                           },
    { DRM_FORMAT_BGR888,   GAVL_BGR_24                           },
    { DRM_FORMAT_RGBA8888, GAVL_RGBA_32                          },
    { DRM_FORMAT_RGB565,   GAVL_RGB_16                           },
    { DRM_FORMAT_YUYV,     GAVL_YUY2                             },
    { DRM_FORMAT_UYVY,     GAVL_UYVY                             },
    { DRM_FORMAT_YUV411,   GAVL_YUV_411_P                          },
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

    /*
     *  Creating Image failed 00003009
     *  [glvideo] Transferring dmabuf to EGL failed
     */
    
    { DRM_FORMAT_ARGB16161616,  GAVL_RGBA_64, GAVL_DMABUF_FLAG_SHUFFLE, { 2, 1, 0, 3 } },
    //    { DRM_FORMAT_ABGR16161616,  GAVL_RGBA_64, GAVL_DMABUF_FLAG_SHUFFLE, { 3, 2, 1, 0 } },

    { DRM_FORMAT_ABGR16161616,  GAVL_RGBA_64 },
    
    { DRM_FORMAT_ARGB8888,      GAVL_RGBA_32, GAVL_DMABUF_FLAG_SHUFFLE, { 2, 1, 0, 3 } },
    { DRM_FORMAT_ABGR8888,      GAVL_RGBA_32  },
    { DRM_FORMAT_XRGB8888,      GAVL_BGR_32  },
    { DRM_FORMAT_XBGR8888,      GAVL_RGB_32  },

    { DRM_FORMAT_XRGB8888,      GAVL_RGB_32, GAVL_DMABUF_FLAG_SHUFFLE, { 2, 1, 0, 3 } },
    { DRM_FORMAT_XBGR8888,      GAVL_BGR_32, GAVL_DMABUF_FLAG_SHUFFLE, { 2, 1, 0, 3 } },

    /* Byte order on the wire: VUYA */
    { DRM_FORMAT_AYUV,          GAVL_YUVA_32, GAVL_DMABUF_FLAG_SHUFFLE, { 2, 1, 0, 3    } },
    
    { DRM_FORMAT_AVUY8888,      GAVL_YUVA_32, GAVL_DMABUF_FLAG_SHUFFLE, { 3, 2, 1, 0    } }, // TODO: Check byte order
    
    /* [63:0] A:Cr:Y:Cb 16:16:16:16 little endian */
    { DRM_FORMAT_Y416,          GAVL_YUVA_64, GAVL_DMABUF_FLAG_SHUFFLE, { 1, 0, 2, 3    } }, // TODO: Check word order

    /* Red formats need shader support */
    { DRM_FORMAT_R8,            GAVL_GRAY_8,   },
    { DRM_FORMAT_R16,           GAVL_GRAY_16,  },
    { DRM_FORMAT_GR88,          GAVL_GRAYA_16, },
    { DRM_FORMAT_GR1616,        GAVL_GRAYA_32, },
    { DRM_FORMAT_RG88,          GAVL_GRAYA_16, },
    { DRM_FORMAT_RG1616,        GAVL_GRAYA_32, },

    { /* */ }
  };


#if 1
static uint32_t fourcc_from_gavl(gavl_hw_context_t * ctx, gavl_pixelformat_t pfmt, int * flags)
  {
  int i = 0;
  dma_native_t * native = ctx->native;
  
  while(drm_formats[i].drm_fourcc)
    {
    if(drm_formats[i].pfmt != pfmt)
      {
      i++;
      continue;
      }
    /* Check if it is actually supported */
    if(native->supported_formats)
      {
      int j = 0;
      while(native->supported_formats[j])
        {
        if(native->supported_formats[j] == drm_formats[i].drm_fourcc)
          break;
        j++;
        }

      if(!native->supported_formats[j])
        {
        i++;
        continue;
        }
      }
   
    if(gavl_hw_ctx_dma_get_frame_mode(drm_formats[i].drm_fourcc) == ctx->mode)
      {
      if(flags)
        *flags = drm_formats[i].flags;
      return drm_formats[i].drm_fourcc;
      }
    
#if 0
    if(ctx->mode == GAVL_HW_FRAME_MODE_MAP)
      return drm_formats[i].drm_fourcc;
    if(ctx->mode == GAVL_HW_FRAME_MODE_TRANSFER)
      {
      if(drm_formats[i].flags & GAVL_DMABUF_FLAG_SHUFFLE)
        return drm_formats[i].drm_fourcc;
      }
    else if(ctx->mode == GAVL_HW_FRAME_MODE_MAP)
      {
      if(!(drm_formats[i].flags & GAVL_DMABUF_FLAG_SHUFFLE))
        return drm_formats[i].drm_fourcc;
      }
#endif
    i++;
    }
  return 0;
  }
#endif

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
#ifdef HAVE_LINUX_DMA_BUF_H
  struct dma_buf_sync sync;
#endif
  dma_native_t * native = f->hwctx->native;
  gavl_dmabuf_video_frame_t *dmabuf = f->storage;

  dmabuf->wr = wr;
  
  /* Map the buffers. We'll keep this even if unmap is called */
  if(!f->planes[0])
    {
    int prot = PROT_READ;
    int fd;

    if(wr)
      prot |= PROT_WRITE;
  
    for(i = 0; i < dmabuf->num_buffers; i++)
      {
      if(dmabuf->buffers[i].drm_handle >= 0)
        fd = native->drm_fd;
      else
        fd = dmabuf->buffers[i].fd;
      
      dmabuf->buffers[i].map_ptr =
        mmap(NULL, dmabuf->buffers[i].map_len, prot, MAP_SHARED,
             fd, dmabuf->buffers[i].map_offset);


      
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

#ifdef HAVE_LINUX_DMA_BUF_H
  sync.flags = DMA_BUF_SYNC_START|DMA_BUF_SYNC_READ;
  if(wr)
    sync.flags |= DMA_BUF_SYNC_WRITE;
  for(i = 0; i < dmabuf->num_buffers; i++)
    ioctl(dmabuf->buffers[i].fd, DMA_BUF_IOCTL_SYNC, &sync);
  
#endif
  
  return 1;
  }

static int
video_frame_unmap_dmabuf(gavl_video_frame_t * f)
  {
  int i;
  gavl_dmabuf_video_frame_t *dmabuf = f->storage;

#ifdef HAVE_LINUX_DMA_BUF_H
  struct dma_buf_sync sync;
  sync.flags = DMA_BUF_SYNC_END|DMA_BUF_SYNC_READ;
  if(dmabuf->wr)
    sync.flags |= DMA_BUF_SYNC_WRITE;
#endif
  
  for(i = 0; i < dmabuf->num_buffers; i++)
    {
#ifdef __aarch64__
    /* Flush cache synchronously */
    if(dmabuf->buffers[i].map_ptr)
      __builtin___clear_cache(dmabuf->buffers[i].map_ptr,
                              dmabuf->buffers[i].map_ptr + dmabuf->buffers[i].map_len);
#endif

#ifdef HAVE_LINUX_DMA_BUF_H
  for(i = 0; i < dmabuf->num_buffers; i++)
    ioctl(dmabuf->buffers[i].fd, DMA_BUF_IOCTL_SYNC, &sync);
#endif
    }

  dmabuf->wr = 0;
  
  return 1;
  }

static void swap_chroma(gavl_video_frame_t * frame)
  {
  uint8_t * swp = frame->planes[1];
  frame->planes[1] = frame->planes[2];
  frame->planes[2] = swp;
  }


#ifdef HAVE_LINUX_DMA_HEAP_H

static int alloc_cma(gavl_hw_context_t * ctx, gavl_video_frame_t * ret)
  {
  int i;
  int heap_fd;
  dma_native_t * native = ctx->native;
  struct dma_heap_allocation_data heap_data;

  int offsets[GAVL_MAX_PLANES];
  int size;
  int flags = 0;
  gavl_dmabuf_video_frame_t *f = ret->storage;
  
  f->fourcc = fourcc_from_gavl(ctx, ctx->vfmt.pixelformat, &flags);
    
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

  if(!(flags & GAVL_DMABUF_FLAG_SWAP_CHROMA))
    swap_chroma(ret);
  
  return 1;
  fail:
  return 0;
  
  }

#endif



static int get_bpp(gavl_pixelformat_t pfmt)
  {
  if(gavl_pixelformat_num_planes(pfmt) > 1)
    return gavl_pixelformat_bytes_per_component(pfmt) * 8;
  else
    return gavl_pixelformat_bytes_per_pixel(pfmt) * 8;
  }


static int alloc_drm(gavl_hw_context_t * ctx, gavl_video_frame_t * ret)
  {
  int i;
  gavl_dmabuf_video_frame_t *f = ret->storage;
  dma_native_t * native = ctx->native;

  struct drm_mode_create_dumb create_struct;
  struct drm_mode_map_dumb map_struct;
  struct drm_prime_handle prime;

  int flags = 0;

  
#define INIT(s) memset(&s, 0, sizeof(s))

  INIT(create_struct);
  INIT(map_struct);
  INIT(prime);
  
  create_struct.width  = ctx->vfmt.frame_width;
  create_struct.height = ctx->vfmt.frame_height; 

  f->fourcc = fourcc_from_gavl(ctx, ctx->vfmt.pixelformat, &flags);
  
  f->num_planes = gavl_pixelformat_num_planes(ctx->vfmt.pixelformat);
  f->num_buffers = f->num_planes;

  if(native->drm_fd < 0)
    {
    native->drm_fd = open(native->drm_device, O_RDWR);
    }
  
  create_struct.bpp = get_bpp(ctx->vfmt.pixelformat);
  
  for(i = 0; i < f->num_planes; i++)
    {
    if(i == 1)
      {
      int sub_h, sub_v;
      gavl_pixelformat_chroma_sub(ctx->vfmt.pixelformat, &sub_h, &sub_v);
      create_struct.width /= sub_h;
      create_struct.height /= sub_v;
      }

    if(drmIoctl(native->drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_struct) < 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "DRM_IOCTL_MODE_CREATE_DUMB failed: %s", strerror(errno));
      }

    map_struct.handle = create_struct.handle;
    
    if(drmIoctl(native->drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_struct) < 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
               "DRM_IOCTL_MODE_MAP_DUMB failed: %s", strerror(errno));
      }
    
    f->buffers[i].drm_handle = create_struct.handle;
    f->planes[i].stride = create_struct.pitch;
    f->buffers[i].map_len    = create_struct.size;
    f->buffers[i].map_offset = map_struct.offset;

    prime.handle = create_struct.handle;
    prime.flags = DRM_CLOEXEC | DRM_RDWR;
    if(drmIoctl(native->drm_fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime) < 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "DRM_IOCTL_PRIME_HANDLE_TO_FD failed: %s", strerror(errno));
      
      
      }
    
    f->planes[i].buf_idx = i;
    f->buffers[i].fd = prime.fd;
    }

  if(flags & GAVL_DMABUF_FLAG_SWAP_CHROMA)
    swap_chroma(ret);
  
  return 1;
  }

static gavl_video_frame_t * video_frame_create_hw_dmabuf(gavl_hw_context_t * ctx,
                                                         int alloc_resources)
  {
  gavl_video_frame_t * ret;
  gavl_dmabuf_video_frame_t *f = calloc(1, sizeof(*f));
  int i;
  dma_native_t * native = ctx->native;
    
  for(i = 0; i < GAVL_MAX_PLANES; i++)
    {
    f->buffers[i].fd = -1;
    f->buffers[i].drm_handle = -1;
    }
  ret = gavl_video_frame_create(NULL);
  ret->storage = f;
  
  if(alloc_resources)
    {
#ifdef HAVE_LINUX_DMA_HEAP_H
    if(native->heap_device)
      {
      /* Error */
      if(!alloc_cma(ctx, ret))
        goto fail;
      }
    else
#endif
    if(native->drm_device)
      {
      /* Error */
      if(!alloc_drm(ctx, ret))
        goto fail;
      }
    else
      {
      /* Error */
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Neither cma nor drm buffers can be allocated");
      goto fail;
      }
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
  dma_native_t * native = f->hwctx->native;
  
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

      if(destroy_resources)
        {
        struct drm_mode_destroy_dumb destroy;
        
        if(info->buffers[i].drm_handle > 0)
          {
          destroy.handle = info->buffers[i].drm_handle;
          drmIoctl(native->drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
          }
        }
      
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
  if(native->drm_device)
    free(native->drm_device);
  if(native->supported_formats)
    free(native->supported_formats);

  if(native->drm_fd >= 0)
    close(native->drm_fd);

  if(native->dsp)
    gavl_dsp_context_destroy(native->dsp);
  
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

#define CHECK_FORMAT(fourcc)     \
  (gavl_drm_pixelformat_from_fourcc(fourcc, NULL, NULL) != GAVL_PIXELFORMAT_NONE) && \
  (gavl_hw_ctx_dma_get_frame_mode(fourcc) == mode)

static int check_format(gavl_hw_context_t * ctx, uint32_t fourcc, gavl_hw_frame_mode_t mode)
  {
  int bpp;
  gavl_pixelformat_t pfmt;
  dma_native_t * native = ctx->native;
  
  if((pfmt = gavl_drm_pixelformat_from_fourcc(fourcc, NULL, NULL)) == GAVL_PIXELFORMAT_NONE)
    return 0;
  
  if(gavl_hw_ctx_dma_get_frame_mode(fourcc) != mode)
    return 0;
  
  if(native->bpp_supported == BPP_ALL)
    return 1;
  
  bpp = get_bpp(pfmt);

  switch(bpp)
    {
    case 8:
      return !!(native->bpp_supported & BPP_8);
      break;
    case 16:
      return !!(native->bpp_supported & BPP_16);
      break;
    case 24:
      return !!(native->bpp_supported & BPP_24);
      break;
    case 32:
      return !!(native->bpp_supported & BPP_32);
      break;
    case 64:
      return !!(native->bpp_supported & BPP_64);
      break;
    }
  
  return 0;
  }

static gavl_pixelformat_t *
get_image_formats_dmabuf(gavl_hw_context_t * ctx, gavl_hw_frame_mode_t mode)
  {
  gavl_pixelformat_t * ret = NULL;

  dma_native_t * native = ctx->native;
  
  int num = 0;
  int i, j, k;
  
  if(native->supported_formats)
    {
    i = 0;
    /* Count formats */
    while(native->supported_formats[i])
      {
      if(check_format(ctx, native->supported_formats[i], mode))
        {
        num++;
        }
      i++;
      }

    ret = calloc(num + 1, sizeof(*ret));

    j = 0;
    i = 0;
    while(native->supported_formats[i])
      {
      if(check_format(ctx, native->supported_formats[i], mode))
        {
        k = 0;
        while(drm_formats[k].drm_fourcc)
          {
          if(drm_formats[k].drm_fourcc == native->supported_formats[i])
            break;
          k++;
          }
        
        ret[j] = drm_formats[k].pfmt;
        j++;
        }
      i++;
      }
    ret[j] = GAVL_PIXELFORMAT_NONE;
    }
  else
    {
    /* Assume that everything is supported */

    /* Count formats */

    i = 0;
    while(drm_formats[i].drm_fourcc)
      {
      if(check_format(ctx, drm_formats[i].drm_fourcc, mode))
        num++;
      i++;
      }
      
    ret = calloc(num + 1, sizeof(*ret));
    
    i = 0;
    j = 0;
    while(drm_formats[i].drm_fourcc)
      {
      if(check_format(ctx, drm_formats[i].drm_fourcc, mode))
        {
        ret[j] = drm_formats[i].pfmt;
        j++;
        }
      i++;
      }
    }
    
  return ret;
  }

static const gavl_hw_funcs_t funcs =
  {

    .destroy_native         = destroy_native_dmabuf,
    .get_image_formats      = get_image_formats_dmabuf,
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


gavl_hw_frame_mode_t gavl_hw_ctx_dma_get_frame_mode(uint32_t format)
  {
  int i = 0;

  while(drm_formats[i].drm_fourcc)
    {
    if(drm_formats[i].drm_fourcc != format)
      {
      i++;
      continue;
      }
    if(gavl_pixelformat_is_yuv(drm_formats[i].pfmt) &&
       (drm_formats[i].flags & GAVL_DMABUF_FLAG_SHUFFLE))
      return GAVL_HW_FRAME_MODE_TRANSFER;
    else
      return GAVL_HW_FRAME_MODE_MAP;
    }
  return GAVL_HW_FRAME_MODE_MAP;
  }


static int bpp_supported(int drm_fd, int bpp)
  {
  struct drm_mode_create_dumb create_req = {0};
  struct drm_mode_destroy_dumb destroy_req = {0};
  int ret;
  
  // Prepare rquest
  create_req.width  = 64;
  create_req.height = 64;
  create_req.bpp = bpp;
  create_req.flags = 0;
    
  // Try to create buffer
  ret = ioctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_req);
    
  if(ret == 0)
    {
    // Success, release again
    destroy_req.handle = create_req.handle;
    ioctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_req);
    gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "%d bpp supported", bpp);
    return 1;
    }
  
  gavl_log(GAVL_LOG_WARNING, LOG_DOMAIN, "%d bpp not supported", bpp);
  return 0;
  }

static int get_supported_bpp_values(int drm_fd)
  {
  int ret = 0;
  if(bpp_supported(drm_fd, 8))
    ret |= BPP_8;
  if(bpp_supported(drm_fd, 16))
    ret |= BPP_16;
  if(bpp_supported(drm_fd, 24))
    ret |= BPP_24;
  if(bpp_supported(drm_fd, 32))
    ret |= BPP_32;
  if(bpp_supported(drm_fd, 64))
    ret |= BPP_64;
  return ret;
  }

gavl_hw_context_t * gavl_hw_ctx_create_dma()
  {
  dma_native_t * native;
  int support_flags = GAVL_HW_SUPPORTS_VIDEO;

  native = calloc(1, sizeof(*native));
  
  native->drm_fd = -1;
  
  // #undef HAVE_LINUX_DMA_HEAP_H
  
#ifdef HAVE_LINUX_DMA_HEAP_H  
  if(test_heap_dev(native, "/dev/dma_heap/linux,cma"))
    {
    support_flags |= GAVL_HW_SUPPORTS_VIDEO_POOL | GAVL_HW_SUPPORTS_VIDEO_MAP;
    gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Using CMA for allocating DMA buffers");
    native->bpp_supported = BPP_ALL;
    }
  else
#endif
    {
    /* Get Graphics card */
    int i;
    int fd;
    uint64_t cap;
    for(i = 0; i < 8; i++)
      {
      native->drm_device = gavl_sprintf("/dev/dri/card%d", i);
      
      fd = open(native->drm_device, O_RDWR);
      
      if(fd >= 0)
        {
        drmVersionPtr version = NULL;
        
        // Check for working DRM-driver
        if((version = drmGetVersion(fd)) &&
           !drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &cap) && cap &&
           !drmGetCap(fd, DRM_CAP_PRIME, &cap) && (cap & DRM_PRIME_CAP_EXPORT))
          {
#ifdef DUMP_DEVICE_INFO
          print_drm_info(fd);
#endif

          native->bpp_supported = get_supported_bpp_values(fd);

          close(fd);
          gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Using drm device: %s", native->drm_device);
          support_flags |= GAVL_HW_SUPPORTS_VIDEO_POOL | GAVL_HW_SUPPORTS_VIDEO_MAP;
          drmFreeVersion(version);
          break;
          }
        if(version)
          drmFreeVersion(version);
        close(fd);
        }
      free(native->drm_device);
      native->drm_device = NULL;
      }
    }
  
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

  /* Set pixelformats for mapping */
  //  if(ctx->

  
  }

#ifdef DUMP_DEVICE_INFO

/* Print DRM info */

static void print_capabilities(int fd)
  {
  uint64_t cap;
    
  printf("\n=== DRM Capabilities ===\n");
  
  if(drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &cap) == 0)
    {
    printf("Dumb Buffer Support: %s\n", cap ? "Yes" : "No");
    }
  
  if (drmGetCap(fd, DRM_CAP_VBLANK_HIGH_CRTC, &cap) == 0)
    {
    printf("High CRTC VBlank: %s\n", cap ? "Yes" : "No");
    }
  
  if (drmGetCap(fd, DRM_CAP_PRIME, &cap) == 0)
    {
    printf("PRIME Support: ");
    if (cap & DRM_PRIME_CAP_IMPORT) printf("Import ");
    if (cap & DRM_PRIME_CAP_EXPORT) printf("Export ");
    if (!cap) printf("None");
    printf("\n");
    }
    
  if (drmGetCap(fd, DRM_CAP_ASYNC_PAGE_FLIP, &cap) == 0)
    {
    printf("Async Page Flip: %s\n", cap ? "Yes" : "No");
    }
  
  if (drmGetCap(fd, DRM_CAP_CURSOR_WIDTH, &cap) == 0)
    {
    printf("Max Cursor Width: %lu\n", cap);
    }
  
  if (drmGetCap(fd, DRM_CAP_CURSOR_HEIGHT, &cap) == 0)
    {
    printf("Max Cursor Height: %lu\n", cap);
    }
  }

static void print_version_info(int fd)
  {
  drmVersionPtr version = drmGetVersion(fd);
  if (!version)
    {
    printf("Failed to get version info\n");
    return;
    }
  
  printf("\n=== Driver Information ===\n");
  printf("Driver Name: %s\n", version->name);
  printf("Driver Description: %s\n", version->desc);
  printf("Driver Date: %s\n", version->date);
  printf("Version: %d.%d.%d\n", version->version_major, 
         version->version_minor, version->version_patchlevel);
  
  drmFreeVersion(version);
  }

static void print_resources_info(int fd)
  {
  int i;
  drmModeResPtr res = drmModeGetResources(fd);
  if(!res)
    {
    printf("Failed to get mode resources\n");
    return;
    }
  
  printf("\n=== Display Resources ===\n");
  printf("CRTCs: %d\n", res->count_crtcs);
  printf("Encoders: %d\n", res->count_encoders);
  printf("Connectors: %d\n", res->count_connectors);
  printf("Frame Buffers: %d\n", res->count_fbs);
  printf("Min Resolution: %dx%d\n", res->min_width, res->min_height);
  printf("Max Resolution: %dx%d\n", res->max_width, res->max_height);
    
  // Zeige verfügbare Connectors
  printf("\n=== Connected Displays ===\n");
  for(i = 0; i < res->count_connectors; i++)
    {
    drmModeConnectorPtr connector = drmModeGetConnector(fd, res->connectors[i]);
    if (connector)
      {
      printf("Connector %d: %s, Status: %s\n", 
             i + 1,
             drmModeGetConnectorTypeName(connector->connector_type),
             connector->connection == DRM_MODE_CONNECTED ? "Connected" :
             connector->connection == DRM_MODE_DISCONNECTED ? "Disconnected" : "Unknown");
      
      if (connector->connection == DRM_MODE_CONNECTED && connector->count_modes > 0) {
      printf("  Primary Mode: %dx%d@%dHz\n", 
             connector->modes[0].hdisplay,
             connector->modes[0].vdisplay,
             connector->modes[0].vrefresh);
      }
      drmModeFreeConnector(connector);
      }
    }
  
  drmModeFreeResources(res);
  }

static void print_drm_info(int fd)
  {
  // Ausgabe der Informationen
  print_version_info(fd);
  print_capabilities(fd);
  print_resources_info(fd);
  
  printf("\n=== Additional Info ===\n");
  printf("DRM File Descriptor: %d\n", fd);
  printf("Device supports KMS: %s\n", drmIsKMS(fd) ? "Yes" : "No");
  }
#endif // DUMP_DEVICE_INFO

#else // No DRM
gavl_hw_context_t * gavl_hw_ctx_create_dma()
  {
  return NULL;
  }

#endif

