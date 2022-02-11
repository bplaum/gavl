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
#include <stdio.h>
#include <inttypes.h>

#include <vaapi.h>
#include <gavl/hw_vaapi.h>

// #define DUMP_FORMATS

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

gavl_video_frame_t * gavl_vaapi_video_frame_create_hw(gavl_hw_context_t * ctx,
                                                      gavl_video_format_t * fmt)
  {
  VAStatus result;
  gavl_video_frame_t * ret = NULL;
  unsigned int rt_format;
  VASurfaceID * surf = NULL;
  /* Create surface */
  gavl_hw_vaapi_t * priv = ctx->native;

  if(!(rt_format = pixelformat_to_rt_format(fmt->pixelformat)))
    goto fail;
  
  surf = calloc(1, sizeof(*surf));
  
  if((result = vaCreateSurfaces(priv->dpy, rt_format, fmt->image_width, fmt->image_height, surf, 1, NULL, 0)) !=
     VA_STATUS_SUCCESS)
    goto fail;
  
  ret = create_common(ctx);
  ret->user_data = surf;
  
  return ret;

  fail:

  if(ret)
    {
    ret->hwctx = NULL;
    gavl_video_frame_destroy(ret);
    }

  if(surf)
    free(surf);
  return NULL;
  }

static int map_frame(gavl_hw_vaapi_t * priv, gavl_video_frame_t * f)
  {
  void * buf;
  uint8_t * buf_i;
  VAStatus result;
  VAImage * image;

  image = f->user_data;
  
  /* Map */
  if((result = vaMapBuffer(priv->dpy, image->buf, &buf)) != VA_STATUS_SUCCESS)
    {
    return 0;
    }

  buf_i = buf;

  f->planes[0] = buf_i + image->offsets[0];
  f->strides[0] = image->pitches[0];
  
  if(image->format.fourcc == VA_FOURCC('Y','V','1','2'))
    {
    f->planes[2] = buf_i + image->offsets[1];
    f->strides[2] = image->pitches[1];
  
    f->planes[1] = buf_i + image->offsets[2];
    f->strides[1] = image->pitches[2];
    }
  else if((image->num_planes > 1) &&
          (image->offsets[1] + image->pitches[1]))
    {
    f->planes[1] = buf_i + image->offsets[1];
    f->strides[1] = image->pitches[1];
  
    f->planes[2] = buf_i + image->offsets[2];
    f->strides[2] = image->pitches[2];
    }
  
  return 1;
  }

void gavl_vaapi_map_frame(gavl_video_frame_t * f)
  {
  map_frame(f->hwctx->native, f);  
  }

/* Unmap */
void gavl_vaapi_unmap_frame(gavl_video_frame_t * f)
  {
  VAImage * image;
  gavl_hw_vaapi_t * priv = f->hwctx->native;
  image = f->user_data;
  vaUnmapBuffer(priv->dpy, image->buf);
  }

static gavl_video_frame_t *
video_frame_create_ram(gavl_hw_context_t * ctx,
                       gavl_video_format_t * fmt, int ovl)
  {
  vaapi_frame_t * image;
  gavl_video_frame_t * ret;
  VAImageFormat * format;
  VAStatus result;
  
  gavl_hw_vaapi_t * priv = ctx->native;

  if(ovl)
    {
    if(!(format = pixelformat_to_image_format(priv, fmt->pixelformat,
                                              priv->subpicture_formats,
                                              priv->num_subpicture_formats)))
      return NULL;
    }
  else
    {
    if(!(format = pixelformat_to_image_format(priv, fmt->pixelformat,
                                              priv->image_formats,
                                              priv->num_image_formats)))
      return NULL;
    }
  
  ret = create_common(ctx);
  image = calloc(1, sizeof(*image));
  image->ovl = VA_INVALID_ID;
  
  /* Create image */
  if((result = vaCreateImage(priv->dpy,
                             format,
                             fmt->frame_width,
                             fmt->frame_height,
                             &image->image)) != VA_STATUS_SUCCESS)
    goto fail;
  
  ret->user_data = image;

  if(!map_frame(priv, ret))
    goto fail;
  
  return ret;
  
  fail:
  if(image)
    free(image);
  if(ret)
    {
    gavl_video_frame_null(ret);
    ret->hwctx = NULL;
    gavl_video_frame_destroy(ret);
    }
  return NULL;
  }


gavl_video_frame_t *
gavl_vaapi_video_frame_create_ram(gavl_hw_context_t * ctx,
                                  gavl_video_format_t * fmt)
  {
  return video_frame_create_ram(ctx, fmt, 0);
  }

gavl_video_frame_t *
gavl_vaapi_video_frame_create_ovl(gavl_hw_context_t * ctx,
                                  gavl_video_format_t * fmt)
  {
  vaapi_frame_t * image;
  VAStatus result;
  gavl_hw_vaapi_t * priv = ctx->native;
  gavl_video_frame_t * ret;

  if(!(ret = video_frame_create_ram(ctx, fmt, 1)))
    goto fail;
  
  image = ret->user_data;
  
  if((result = vaCreateSubpicture(priv->dpy,
                                  image->image.image_id,
                                  &image->ovl)) != VA_STATUS_SUCCESS)
    {
    fprintf(stderr, "vaCreateSubpicture failed: %s\n",  vaErrorStr(result));
    goto fail;
    }
  return ret;
  
  fail:
  
  if(ret)
    gavl_video_frame_destroy(ret);
  return NULL;
  }

void gavl_vaapi_video_frame_destroy(gavl_video_frame_t * f)
  {
  gavl_hw_vaapi_t * priv = f->hwctx->native;
  
  if(f->planes[0])
    {
    vaapi_frame_t * image;
    image = f->user_data;

    if(image->ovl != VA_INVALID_ID)
      vaDestroySubpicture(priv->dpy, image->ovl);
    vaDestroyImage(priv->dpy, image->image.image_id);
    free(image);
    }
  else
    {
    VASurfaceID * surf = f->user_data;
    vaDestroySurfaces(priv->dpy, surf, 1);
    }
  gavl_video_frame_null(f);
  f->hwctx = NULL;
  gavl_video_frame_destroy(f);
  }

int gavl_vaapi_video_frame_to_ram(const gavl_video_format_t * fmt,
                                  gavl_video_frame_t * dst,
                                  gavl_video_frame_t * src)
  {
  VASurfaceID * surf;
  VAImage * image;
  VAStatus result;
  VAImageFormat * format;
  gavl_hw_vaapi_t * priv = src->hwctx->native;

  image = dst->user_data;
  surf = src->user_data;
  
  vaUnmapBuffer(priv->dpy, image->buf);
  
  if(!priv->no_derive)
    {
    vaDestroyImage(priv->dpy, image->image_id);

    if((result = vaDeriveImage(priv->dpy,
                               *surf,
                               image)) != VA_STATUS_SUCCESS)
      {
      // fprintf(stderr, "vaDeriveImage Failed: %s\n", vaErrorStr(result));
      priv->no_derive = 1;

      if(!(format = pixelformat_to_image_format(priv, fmt->pixelformat, priv->image_formats, priv->num_image_formats)))
        return 0;
      
      /* Create image */
      if((result = vaCreateImage(priv->dpy,
                                 format,
                                 fmt->image_width,
                                 fmt->image_height,
                                 image)) != VA_STATUS_SUCCESS)
        return 0;
      }
    }
  
  if(priv->no_derive)
    {
    // fprintf(stderr, "vaGetImage (Dpy: %p, ImageID: %08x, Surface ID: %08x)", priv->dpy,
    //        image->image_id, *surf);
    if((result = vaGetImage(priv->dpy,
                            *surf,
                            0,
                            0,
                            fmt->image_width,
                            fmt->image_height,
                            image->image_id)) != VA_STATUS_SUCCESS)
      {
      // fprintf(stderr, " failed: %s\n", vaErrorStr(result));
      return 0;
      }
    }
  
  map_frame(priv, dst);

  gavl_vaapi_video_frame_swap_bytes(fmt, dst, 1);
  return 1;
  }

int gavl_vaapi_video_frame_to_hw(const gavl_video_format_t * fmt,
                                 gavl_video_frame_t * dst,
                                 gavl_video_frame_t * src)
  {
  VASurfaceID * surf;
  VAImage * image;
  VAStatus result;
  gavl_hw_vaapi_t * priv = src->hwctx->native;

  image = src->user_data;
  surf = dst->user_data;

  gavl_vaapi_video_frame_swap_bytes(fmt, src, 0);
  
  vaUnmapBuffer(priv->dpy, image->buf);

  if((result = vaPutImage(priv->dpy,
                          *surf,
                          image->image_id,
                          0, // int  	src_x,
                          0, // int  	src_y,
                          fmt->image_width, // unsigned int  	src_width,
                          fmt->image_height, // unsigned int  	src_height,
                          0, // int  	dest_x,
                          0, // int  	dest_y,
                          fmt->image_width, // unsigned int  	dest_width,
                          fmt->image_height)) != VA_STATUS_SUCCESS) //unsigned int  	dest_height 
    {
    return 0;
    }
  map_frame(priv, src);
  return 1;
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

gavl_pixelformat_t *
gavl_vaapi_get_image_formats(gavl_hw_context_t * ctx)
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

gavl_pixelformat_t *
gavl_vaapi_get_overlay_formats(gavl_hw_context_t * ctx)
  {
  int num, i;
  gavl_pixelformat_t * ret;
  gavl_hw_vaapi_t * p = ctx->native;
  int num_ret = 0;
  gavl_pixelformat_t fmt;
  int j;
  int take_it;
  
  num = vaMaxNumSubpictureFormats(p->dpy);
  p->subpicture_formats = malloc(num * sizeof(*p->subpicture_formats));

  vaQuerySubpictureFormats(p->dpy, p->subpicture_formats,
                           p->subpicture_flags,
                           (unsigned int*)&p->num_subpicture_formats);

  ret = calloc(p->num_subpicture_formats + 1, sizeof(*ret));
  
  for(i = 0; i < p->num_subpicture_formats; i++)
    {
#ifdef DUMP_FORMATS
    fprintf(stderr, "Overlay format %d\n", i);
    dump_image_format(&p->subpicture_formats[i]);
#endif
    if((fmt = fourcc_to_pixelformat(p->subpicture_formats[i].fourcc)) == GAVL_PIXELFORMAT_NONE)
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

void gavl_vaapi_cleanup(void * priv)
  {
  gavl_hw_vaapi_t * p = priv;
  if(p->image_formats)
    free(p->image_formats);
  if(p->subpicture_formats)
    free(p->subpicture_formats);
  if(p->subpicture_flags)
    free(p->subpicture_flags);
  if(p->dpy)
    vaTerminate(p->dpy);
  if(p->dsp)
    gavl_dsp_context_destroy(p->dsp);
  }
  

VASurfaceID gavl_vaapi_get_surface_id(const gavl_video_frame_t * f)
  {
  VASurfaceID * surf = f->user_data;
  return *surf;
  }

/* Use only in specific create routines */
void gavl_vaapi_set_surface_id(gavl_video_frame_t * f, VASurfaceID id)
  {
  VASurfaceID * surf;
  if(!f->user_data)
    {
    surf = malloc(sizeof(*surf));
    f->user_data = surf;
    }
  else
    surf = f->user_data;
  
  *surf = id;
  }

VASubpictureID gavl_vaapi_get_subpicture_id(const gavl_video_frame_t * f)
  {
  vaapi_frame_t * frame = f->user_data;
  return frame->ovl;
  }

VAImageID gavl_vaapi_get_image_id(const gavl_video_frame_t * f)
  {
  vaapi_frame_t * frame = f->user_data;
  return frame->image.image_id;
  }

void gavl_vaapi_video_frame_swap_bytes(const gavl_video_format_t * fmt,
                                       gavl_video_frame_t * f,
                                       int to_gavl)
  {
  const uint32_t * gavl_masks;
  uint32_t vaapi_masks[4];
  gavl_hw_vaapi_t * priv = f->hwctx->native;
  
  vaapi_frame_t * frame = f->user_data;
  
  if(!(gavl_masks = gavl_pixelformat_get_masks(fmt->pixelformat)))
    return;
  
  vaapi_masks[0] = frame->image.format.red_mask;
  vaapi_masks[1] = frame->image.format.green_mask;
  vaapi_masks[2] = frame->image.format.blue_mask;
  vaapi_masks[3] = frame->image.format.alpha_mask;

  if(!priv->dsp)
    priv->dsp = gavl_dsp_context_create(GAVL_QUALITY_DEFAULT);

  if(to_gavl)
    gavl_dsp_video_frame_shuffle_bytes(priv->dsp, f, fmt, vaapi_masks, gavl_masks);
  else
    gavl_dsp_video_frame_shuffle_bytes(priv->dsp, f, fmt, gavl_masks, vaapi_masks);
  }
  
void gavl_vaapi_video_format_adjust(gavl_hw_context_t * ctx,
                                    gavl_video_format_t * fmt)
  {
  gavl_video_format_set_frame_size(fmt, 16, 16);
  }
