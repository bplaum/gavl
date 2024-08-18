
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> // close()

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/compression.h>
#include <gavl/metatags.h>
#include <gavl/hw.h>
#include <gavl/hw_dmabuf.h>
#include <hw_private.h>

#ifdef HAVE_DRM_DRM_FOURCC_H 
#include <drm/drm_fourcc.h>
#define HAVE_DRM
#else // !HAVE_DRM_DRM_FOURCC_H 

#ifdef HAVE_LIBDRM_DRM_FOURCC_H 
#include <libdrm/drm_fourcc.h>
#define HAVE_DRM
#endif
#endif // !HAVE_DRM_DRM_FOURCC_H 

#ifdef HAVE_DRM
static const struct
  {
  uint32_t drm_fourcc;
  gavl_pixelformat_t pfmt;
  }
drm_formats[] =
  {
    { DRM_FORMAT_RGB888, GAVL_RGB_24 },
    { DRM_FORMAT_BGR888, GAVL_BGR_24 },
    { DRM_FORMAT_YUYV,   GAVL_YUY2   },
    { DRM_FORMAT_UYVY,   GAVL_UYVY   },
    { DRM_FORMAT_YUV411, GAVL_YUV_411_P },
    { DRM_FORMAT_YUV420, GAVL_YUV_420_P },
    { DRM_FORMAT_YUV422, GAVL_YUV_422_P },
    { DRM_FORMAT_YUV444, GAVL_YUV_444_P },
    { DRM_FORMAT_YUV420, GAVL_YUVJ_420_P },
    { DRM_FORMAT_YUV422, GAVL_YUVJ_422_P },
    { DRM_FORMAT_YUV444, GAVL_YUVJ_444_P },
    { /* */ }
  };

uint32_t gavl_drm_fourcc_from_gavl(gavl_pixelformat_t pfmt)
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

static gavl_video_frame_t * video_frame_create_hw_dmabuf(gavl_hw_context_t * ctx,
                                                         gavl_video_format_t * fmt)
  {
  gavl_video_frame_t * ret;
  gavl_dmabuf_video_frame_t *f = calloc(1, sizeof(*f));
  
  ret = gavl_video_frame_create(NULL);
  ret->storage = f;
  return ret;
  }

static void video_frame_destroy_hw_dmabuf(gavl_video_frame_t * f)
  {
  
  if(f->storage)
    {
    int i;
    gavl_dmabuf_video_frame_t * info;
    info = f->storage;

    for(i = 0; i < info->num_buffers; i++)
      close(info->buffers[i].fd);

    free(info);
    }
  
  f->hwctx = NULL;
  gavl_video_frame_destroy(f);
  }

#if 0
typedef struct
  {
  int fourcc;
  int num_planes;
  
  struct
    {
    int buf_idx;
    int offset;
    } planes[GAVL_MAX_PLANES];
  
  } dma_payload_t;


static int video_frame_to_packet_dmabuf(gavl_hw_context_t * ctx,
                                        const gavl_video_format_t * fmt,
                                        const gavl_video_frame_t * frame,
                                        gavl_packet_t * p)
  {
  int i;
  dma_payload_t * pl;
  const gavl_dmabuf_video_frame_t *info = frame->storage;
  gavl_packet_alloc(p, sizeof(dma_payload_t));
  pl = (dma_payload_t*)p->buf.buf;

  pl->num_planes = info->num_planes;
  pl->fourcc = info->fourcc;
  
  for(i = 0; i < info->num_planes; i++)
    {
    pl->planes[i].buf_idx = info->planes[i].buf_idx;
    pl->planes[i].offset = info->planes[i].offset;
    }

  for(i = 0; i < info->num_buffers; i++)
    p->fds[i] = info->buffers[i].fd;
  
  p->num_fds = info->num_buffers;
  return 1;
  }

static int video_frame_from_packet_dmabuf(gavl_hw_context_t * ctx,
                                           const gavl_video_format_t * fmt,
                                           gavl_video_frame_t * frame,
                                           const gavl_packet_t * p)
  {
  int i;

  const dma_payload_t * pl = (const dma_payload_t *)p->buf.buf;
  gavl_dmabuf_video_frame_t *info = frame->storage;

  info->num_planes = pl->num_planes;
  info->fourcc     = pl->fourcc;
  
  for(i = 0; i < info->num_planes; i++)
    {
    info->planes[i].buf_idx = pl->planes[i].buf_idx;
    info->planes[i].offset = pl->planes[i].offset;
    }

  
  info->num_buffers = p->num_fds;
  
  for(i = 0; i < info->num_buffers; i++)
    info->buffers[i].fd = p->fds[i];
  gavl_video_frame_set_strides(frame, fmt);
  frame->buf_idx = p->buf_idx;
  
  return 1;
  }
#endif

#if 0
static void destroy_native_dmabuf(void * data)
  {
  free(data);
  }
#endif

static const gavl_hw_funcs_t funcs =
  {
    //    .destroy_native         = destroy_native_dmabuf,
   //    .get_image_formats      = gavl_gl_get_image_formats,
   //    .get_overlay_formats    = gavl_gl_get_overlay_formats,
   .video_frame_create_hw  = video_frame_create_hw_dmabuf,
   .video_frame_destroy    = video_frame_destroy_hw_dmabuf,

   //   .video_frame_to_packet = video_frame_to_packet_dmabuf,
   //   .video_frame_from_packet = video_frame_from_packet_dmabuf,
   
   //    .video_frame_to_ram     = video_frame_to_ram_egl,
   //    .video_frame_to_hw      = video_frame_to_hw_egl,
   //    .video_format_adjust    = gavl_gl_adjust_video_format,
   //    .overlay_format_adjust  = gavl_gl_adjust_video_format,
   //    .can_export             = exports_type_v4l2,
   //    .export_video_frame = export_video_frame_v4l2,
  };

gavl_hw_context_t * gavl_hw_ctx_create_dma()
  {
  int support_flags = GAVL_HW_SUPPORTS_VIDEO;

  return gavl_hw_context_create_internal(NULL, &funcs, GAVL_HW_DMABUFFER, support_flags);
  }
#else // No DRM
gavl_hw_context_t * gavl_hw_ctx_create_dma()
  {
  return NULL;
  }

#endif
