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
#include <gavl/gavl.h>
#include <gavl/log.h>
#define LOG_DOMAIN "videoframepool"

typedef struct
  {
  gavl_video_frame_t * frame;
  int used;
  } pool_element_t;

struct gavl_video_frame_pool_s
  {
  int num_frames;
  int frames_alloc;
  pool_element_t * frames;

  gavl_video_format_t format;
  
  gavl_hw_context_t * hwctx;
  };

gavl_video_frame_pool_t *
gavl_video_frame_pool_create(gavl_hw_context_t * ctx)
  {
  gavl_video_frame_pool_t * ret;
  ret = calloc(1, sizeof(*ret));
  ret->hwctx = ctx;
  return ret;
  }

static gavl_video_frame_t * create_frame(gavl_video_frame_pool_t *p)
  {
  gavl_video_frame_t * ret;
  
  if(p->hwctx)
    ret = gavl_hw_video_frame_create_ram(p->hwctx, &p->format);
  else
    {
    ret = gavl_video_frame_create(&p->format);
    ret->buf_idx = p->num_frames;
    }
  ret->client_data = p;
  return ret;
  }

gavl_video_frame_t * gavl_video_frame_pool_get(gavl_video_frame_pool_t *p)
  {
  int i;

  if(!p->format.image_width)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Format must be set before getting frames");
    return NULL;
    }

  for(i = 0; i < p->num_frames; i++)
    {
    if(!p->frames[i].used)
      {
      p->frames[i].used = 1;
      return p->frames[i].frame;
      }
    }
  /* Allocate a new frame */
  if(p->num_frames == p->frames_alloc)
    {
    p->frames_alloc += 16;
    p->frames = realloc(p->frames, p->frames_alloc * sizeof(*p->frames));
    memset(p->frames + p->num_frames, 0,
           sizeof(*p->frames)*(p->frames_alloc - p->num_frames));
    }
  
  p->frames[p->num_frames].frame = create_frame(p);
  p->num_frames++;
  
  p->frames[p->num_frames-1].used = 1;
  return p->frames[p->num_frames-1].frame;
  }

void gavl_video_frame_pool_release(gavl_video_frame_t * frame)
  {
  int i = 0;
  gavl_video_frame_pool_t *p = frame->client_data;

  for(i = 0; i < p->num_frames; i++)
    {
    if(p->frames[i].frame == frame)
      {
      p->frames[i].used = 0;
      return;
      }
    }
  }

void gavl_video_frame_pool_destroy(gavl_video_frame_pool_t *p)
  {
  int i;
  for(i = 0; i < p->num_frames; i++)
    gavl_video_frame_destroy(p->frames[i].frame);
  if(p->frames)
    free(p->frames);
  free(p);
  }

void gavl_video_frame_pool_reset(gavl_video_frame_pool_t *p)
  {
  int i;
  for(i = 0; i < p->num_frames; i++)
    p->frames[i].used = 0;
  }

int gavl_video_frame_pool_set_format(gavl_video_frame_pool_t *p, const gavl_video_format_t * fmt)
  {
  if(!p->format.image_width)
    {
    gavl_video_format_copy(&p->format, fmt);
    gavl_video_format_set_frame_size(&p->format, 16, 32);
    }
  else
    {
    /* Sanity checks */
    if((p->format.image_width != fmt->image_width) &&
       (p->format.image_height != fmt->image_height) &&
       (p->format.pixelformat != fmt->pixelformat))
      {
      return 0;
      }
    
    }
  return 1;
  }

const gavl_video_format_t *
gavl_video_frame_pool_get_format(const gavl_video_frame_pool_t * p)
  {
  return &p->format;
  }
