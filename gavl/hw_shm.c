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

#include <gavl/gavl.h>
#include <hw_private.h>
#include <hw_shm.h>

#include <gavlshm.h>

typedef struct
  {
  int ctx;
  int idx;
  
  } shm_t;

/* Functions */
static void destroy_native_shm(void * native)
  {
  shm_t * s = native;
  free(s);
  }

static gavl_pixelformat_t * get_image_formats_shm(gavl_hw_context_t * ctx, gavl_hw_frame_mode_t mode)
  {
  int i;
  gavl_pixelformat_t * pixelformats = NULL;
  int num_image_formats;

  if(mode == GAVL_HW_FRAME_MODE_MAP)
    {
    num_image_formats = gavl_num_pixelformats();

    pixelformats = calloc(num_image_formats+1, sizeof(*pixelformats));

    for(i = 0; i < num_image_formats; i++)
      pixelformats[i] = gavl_get_pixelformat(i);
  
    pixelformats[num_image_formats] = GAVL_PIXELFORMAT_NONE;
    
    }
  
  
  return pixelformats;
  }

static gavl_video_frame_t *
video_frame_create_hw_shm(gavl_hw_context_t * ctx, int alloc_resource)
  {
  shm_t * s = ctx->native;

  gavl_shm_video_frame_t * priv = calloc(1, sizeof(*priv));
  
  gavl_video_frame_t * ret = gavl_video_frame_create(NULL);
  ret->storage = priv;

  priv->shm = gavl_shm_create(gavl_video_format_get_image_size(&ctx->vfmt),
                              &s->ctx, &s->idx);
  
  /* Set pointers */
  gavl_video_frame_set_planes(ret, &ctx->vfmt,
                              gavl_shm_get_buffer(priv->shm, NULL));
  
  return ret;
  }

static void video_frame_destroy_shm(gavl_video_frame_t * f, int destroy_resource)
  {
  //  shm_t * p;
  if(f->storage)
    {
    gavl_shm_video_frame_t * priv = f->storage;
    if(priv->shm)
      gavl_shm_destroy(priv->shm);
    free(priv);
    f->storage = NULL;
    }
  gavl_video_frame_null(f);
  gavl_video_frame_destroy(f);
  }


static int video_frame_map_shm(gavl_video_frame_t * f, int wr)
  {
  //   gavl_shm_video_frame_t * priv = f->storage;

  //  gavl_shm_map(const char * name, int size, int wr);

  if(wr)
    {
    }
  else
    {
    
    }
  
  return 1;
  }


static int video_frame_unmap_shm(gavl_video_frame_t * frame)
  {
  return 0;
  }

static const gavl_hw_funcs_t funcs =
  {
   .destroy_native         = destroy_native_shm,
   .get_image_formats      = get_image_formats_shm,
   .video_frame_create     = video_frame_create_hw_shm,
   .video_frame_destroy    = video_frame_destroy_shm,
   .video_frame_map        = video_frame_map_shm,
   .video_frame_unmap        = video_frame_unmap_shm,
   
   //   .video_format_adjust    = video_format_adjust_shm,
   //   .overlay_format_adjust  = overlay_format_adjust_shm,
  };

gavl_hw_context_t * gavl_hw_ctx_create_shm()
  {
  shm_t * native = calloc(1, sizeof(*native));
  return gavl_hw_context_create_internal(native, &funcs, GAVL_HW_SHM,
                                         GAVL_HW_SUPPORTS_VIDEO | GAVL_HW_SUPPORTS_VIDEO_MAP);
  }

/*
  
struct gavl_hw_context_s
  {
  void * native;
  gavl_hw_type_t type;
  const gavl_hw_funcs_t * funcs;
  gavl_pixelformat_t * image_formats;
  gavl_pixelformat_t * overlay_formats;
  };

  
gavl_hw_context_t *
gavl_hw_context_create_internal(void * native,
                                const gavl_hw_funcs_t * funcs,
                                gavl_hw_type_t type);

void 
gavl_hw_destroy_video_frame(gavl_hw_context_t * ctx,
                            gavl_video_frame_t * frame);
*/
