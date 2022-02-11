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

#include <pthread.h>

#include <gavl/connectors.h>

#define FLAG_DO_CONVERT       (1<<0)
#define FLAG_DST_SET          (1<<1)
#define FLAG_SCALE_TIMESTAMPS (1<<2)
#define FLAG_SUPPORT_HW       (1<<3) /* Support HW storage */

#define FLAG_HW_TO_RAM        (1<<4) /* Transfer frames from hardware to RAM */

struct gavl_video_source_s
  {
  gavl_video_format_t src_format;
  gavl_video_format_t src_format_nohw; // RAM only format

  gavl_video_format_t dst_format;
  int src_flags;
  int dst_flags;

  /* Callback set by the client */
  
  gavl_video_source_func_t func;
  gavl_video_converter_t * cnv;
  
  void * priv;

  /* FPS Conversion */
  int64_t next_pts;

  int64_t fps_pts;
  int64_t fps_duration;

  int flags;
  
  gavl_video_frame_t * fps_frame;
  gavl_video_frame_t * next_still_frame;
 
  gavl_video_frame_t * transfer_frame;
 
  gavl_video_frame_pool_t * src_fp;
  gavl_video_frame_pool_t * dst_fp;

  /* Callbacks set according to the configuration */

  gavl_source_status_t (*read_frame)(gavl_video_source_t * s,
                                     gavl_video_frame_t ** frame);

  gavl_source_status_t (*read_video)(gavl_video_source_t * s,
                                     gavl_video_frame_t ** frame);

  gavl_connector_lock_func_t lock_func;
  gavl_connector_lock_func_t unlock_func;
  void * lock_priv;

  gavl_video_source_t * prev;
  gavl_connector_free_func_t free_func;

  pthread_mutex_t eof_mutex;
  int eof;
  int have_lock;
  };

void
gavl_video_source_set_free_func(gavl_video_source_t * src,
                                gavl_connector_free_func_t free_func)
  {
  src->free_func = free_func;
  }

gavl_video_source_t *
gavl_video_source_create(gavl_video_source_func_t func,
                         void * priv,
                         int src_flags,
                         const gavl_video_format_t * src_format)
  {
  gavl_video_source_t * ret;

  if(src_format->hwctx && !(src_flags & GAVL_SOURCE_SRC_ALLOC))
    {
    fprintf(stderr, "Hardware based formats need to have GAVL_SOURCE_SRC_ALLOC set\n");
    return NULL;
    }

  ret = calloc(1, sizeof(*ret));

  ret->func = func;
  ret->priv = priv;
  ret->src_flags = src_flags;
  gavl_video_format_copy(&ret->src_format, src_format);
  gavl_video_format_copy(&ret->src_format_nohw, src_format);
  ret->src_format_nohw.hwctx = NULL;
 
  ret->cnv = gavl_video_converter_create();

  pthread_mutex_init(&ret->eof_mutex, NULL);
  return ret;
  }


gavl_video_source_t *
gavl_video_source_create_source(gavl_video_source_func_t func,
                                void * priv,
                                int src_flags,
                                gavl_video_source_t * src)
  {
  gavl_video_source_t * ret =
    gavl_video_source_create(func, priv, src_flags,
                             gavl_video_source_get_dst_format(src));
  
  ret->prev = src;
  return ret;
  }

void
gavl_video_source_set_lock_funcs(gavl_video_source_t * src,
                                 gavl_connector_lock_func_t lock_func,
                                 gavl_connector_lock_func_t unlock_func,
                                 void * priv)
  {
  src->lock_func = lock_func;
  src->unlock_func = unlock_func;
  src->lock_priv = priv;
  }


/* Called by the destination */

void gavl_video_source_support_hw(gavl_video_source_t * s)
  {
  s->flags |= FLAG_SUPPORT_HW;
  }

const gavl_video_format_t *
gavl_video_source_get_src_format(gavl_video_source_t * s)
  {
  return (s->flags & FLAG_SUPPORT_HW) ? &s->src_format : &s->src_format_nohw;
  }

const gavl_video_format_t *
gavl_video_source_get_dst_format(gavl_video_source_t * s)
  {
  return (s->flags & FLAG_DST_SET) ? &s->dst_format : gavl_video_source_get_src_format(s);
  }

gavl_video_options_t * gavl_video_source_get_options(gavl_video_source_t * s)
  {
  return gavl_video_converter_get_options(s->cnv);
  }

GAVL_PUBLIC
void gavl_video_source_reset(gavl_video_source_t * s)
  {
  s->next_pts = GAVL_TIME_UNDEFINED;
  if(s->src_fp)
    gavl_video_frame_pool_reset(s->src_fp);
  if(s->dst_fp)
    gavl_video_frame_pool_reset(s->dst_fp);
  s->next_still_frame = NULL;
  s->fps_frame = NULL;
  }

GAVL_PUBLIC
void gavl_video_source_destroy(gavl_video_source_t * s)
  {
  if(s->src_fp)
    gavl_video_frame_pool_destroy(s->src_fp);
  if(s->dst_fp)
    gavl_video_frame_pool_destroy(s->dst_fp);

  if(s->transfer_frame)
    gavl_video_frame_destroy(s->transfer_frame);
  
  gavl_video_converter_destroy(s->cnv);

  if(s->priv && s->free_func)
    s->free_func(s->priv);

  pthread_mutex_destroy(&s->eof_mutex);
  
  free(s);
  }

static void scale_pts(gavl_video_source_t * s,
                      gavl_video_frame_t * f)
  {
  int64_t next_pts;
  if(!(s->flags & FLAG_SCALE_TIMESTAMPS))
    return;

  if(s->src_flags & GAVL_SOURCE_SRC_DISCONTINUOUS)
    {
    f->timestamp = gavl_time_rescale(s->src_format.timescale,
                                     s->dst_format.timescale,
                                     f->timestamp);
    f->duration = gavl_time_rescale(s->src_format.timescale,
                                    s->dst_format.timescale,
                                    f->duration);
    }
  else
    {
    if(s->next_pts == GAVL_TIME_UNDEFINED)
      s->next_pts = gavl_time_rescale(s->src_format.timescale,
                                      s->dst_format.timescale,
                                      f->timestamp);
    next_pts = gavl_time_rescale(s->src_format.timescale,
                                 s->dst_format.timescale,
                                 f->timestamp + f->duration);
    f->timestamp = s->next_pts;
    f->duration = next_pts - f->timestamp;
    s->next_pts = next_pts;            
    }
  }

static gavl_source_status_t read_frame(gavl_video_source_t * s,
                                       gavl_video_frame_t ** frame)
  {
  gavl_source_status_t ret;

  if(s->lock_func && !s->have_lock) 
    s->lock_func(s->lock_priv);

  ret = s->func(s->priv, frame);
  
  if(s->unlock_func && !s->have_lock)
    s->unlock_func(s->lock_priv);
  return ret;
  }

static gavl_source_status_t read_frame_transfer(gavl_video_source_t * s,
                                                gavl_video_frame_t ** frame)
  {
  gavl_source_status_t ret;
  gavl_video_frame_t * tmp_frame = NULL;

  if(s->lock_func && !s->have_lock)
    s->lock_func(s->lock_priv);

  ret = s->func(s->priv, &tmp_frame);

  if(ret == GAVL_SOURCE_OK)
    {
    if(!s->transfer_frame)
      s->transfer_frame = gavl_hw_video_frame_create_ram(s->src_format.hwctx,
                                                         &s->src_format);

    if(!gavl_video_frame_hw_to_ram(&s->src_format,
                                   s->transfer_frame,
                                   tmp_frame))
      {
      fprintf(stderr, "Frame transfer failed\n");
      ret = GAVL_SOURCE_EOF;
      }
    }
  *frame = s->transfer_frame;

  if(s->unlock_func && !s->have_lock)
    s->unlock_func(s->lock_priv);
  return ret;

  }

static gavl_source_status_t
read_video_simple(gavl_video_source_t * s,
                  gavl_video_frame_t ** frame)
  {
  gavl_source_status_t st;
  gavl_video_frame_t * in_frame;
  int direct = 0;
  
  /* Pass from src to dst */

  if(!(*frame) && (s->src_flags & GAVL_SOURCE_SRC_ALLOC))
    direct = 1;
  
  /* Pass from dst to src (this is the legacy behavior) */

  if(*frame && !(s->src_flags & GAVL_SOURCE_SRC_ALLOC))
    direct = 1;
  
  if(direct)
    {
    if((st = s->read_frame(s, frame)) != GAVL_SOURCE_OK)
      return st;
    scale_pts(s, *frame);
    return GAVL_SOURCE_OK;
    }
  
  /* memcpy */

  if(!(s->src_flags & GAVL_SOURCE_SRC_ALLOC))
    in_frame = gavl_video_frame_pool_get(s->src_fp);
  else
    in_frame = NULL;
  
  if(!(*frame))
    {
    if(!s->dst_fp)
      s->dst_fp = gavl_video_frame_pool_create(NULL, &s->dst_format);
    *frame = gavl_video_frame_pool_get(s->dst_fp);
    }
  if((st = s->read_frame(s, &in_frame)) != GAVL_SOURCE_OK)
    return st;

  gavl_video_frame_copy(&s->src_format, *frame, in_frame);
  gavl_video_frame_copy_metadata(*frame, in_frame);
  
  scale_pts(s, *frame);
  return GAVL_SOURCE_OK;
  }

static gavl_source_status_t
read_video_cnv(gavl_video_source_t * s,
               gavl_video_frame_t ** frame)
  {
  gavl_source_status_t st;
  gavl_video_frame_t * in_frame = NULL;
  
  if(!(s->src_flags & GAVL_SOURCE_SRC_ALLOC))
    in_frame = gavl_video_frame_pool_get(s->src_fp);

  if((st = s->read_frame(s, &in_frame)) != GAVL_SOURCE_OK)
    return st;

  if(!(*frame))
    {
    if(!s->dst_fp)
      s->dst_fp = gavl_video_frame_pool_create(NULL, &s->dst_format);
    *frame = gavl_video_frame_pool_get(s->dst_fp);
    }
  gavl_video_convert(s->cnv, in_frame, *frame);
  scale_pts(s, *frame);
  return GAVL_SOURCE_OK;
  }

static gavl_source_status_t
read_frame_fps(gavl_video_source_t * s)
  {
  gavl_source_status_t st;
  if(!(s->src_flags & GAVL_SOURCE_SRC_ALLOC))
    {
    if(s->fps_frame)
      s->fps_frame->refcount = 0;
    s->fps_frame = gavl_video_frame_pool_get(s->src_fp);
    }
  else
    s->fps_frame = NULL;
  
  if((st = s->read_frame(s, &s->fps_frame)) != GAVL_SOURCE_OK)
    return st;
    
  s->fps_pts      = s->fps_frame->timestamp;
  s->fps_duration = s->fps_frame->duration;

  // fprintf(stderr, "read_frame_fps %ld %ld\n", s->fps_pts, s->fps_duration);
  
  return GAVL_SOURCE_OK;
  }

static gavl_source_status_t
read_video_fps(gavl_video_source_t * s,
               gavl_video_frame_t ** frame)
  {
  int new_frame = 0;
  int expired = 0;
  gavl_source_status_t st;
  int64_t out_pts;
  
  //  fprintf(stderr, "read_video_fps %ld\n", s->next_pts);
  
  /* Read frame if we don't have one yet */
  if(!s->fps_frame)
    {
    if((st = read_frame_fps(s)) != GAVL_SOURCE_OK)
      return st;
    new_frame = 1;
    
    s->next_pts = gavl_time_rescale(s->src_format.timescale,
                                    s->dst_format.timescale,
                                    s->fps_pts);
    }

  out_pts = s->next_pts;
  s->next_pts += s->dst_format.frame_duration;
  
  /* Read frame until we have one for our time */
  while(gavl_time_rescale(s->src_format.timescale,
                          s->dst_format.timescale,
                          s->fps_pts + s->fps_duration) <= out_pts)
    {
    if((st = read_frame_fps(s)) != GAVL_SOURCE_OK)
      return st;
    new_frame = 1;
    }
  
  /* Check if frame will be expired next time */
  if(gavl_time_rescale(s->src_format.timescale,
                       s->dst_format.timescale,
                       s->fps_pts + s->fps_duration) <= s->next_pts)
    expired = 1;
  
  /* Now check what to do */
  
  /* Convert frame if it was newly read */

  if((s->flags & FLAG_DO_CONVERT) && new_frame)
    {
    if(expired)
      {
      if(!(*frame))
        {
        if(!s->dst_fp)
          s->dst_fp = gavl_video_frame_pool_create(NULL, &s->dst_format);
        *frame = gavl_video_frame_pool_get(s->dst_fp);
        }
      gavl_video_convert(s->cnv, s->fps_frame, *frame);
      (*frame)->timestamp = out_pts;
      (*frame)->duration = s->dst_format.frame_duration;

      //      fprintf(stderr, "FPS frame: %ld %ld\n",
      //              (*frame)->timestamp,
      //              (*frame)->duration);
      
      return GAVL_SOURCE_OK;
      }
    else
      {
      /* Convert into local buffer */
      gavl_video_frame_t * tmp_frame;
      if(!s->dst_fp)
        s->dst_fp = gavl_video_frame_pool_create(NULL, &s->dst_format);
      tmp_frame = gavl_video_frame_pool_get(s->dst_fp);
      gavl_video_convert(s->cnv, s->fps_frame, tmp_frame);
      s->fps_frame = tmp_frame;
      s->fps_frame->refcount = 1;
      }
    }

  s->fps_frame->timestamp = out_pts;
  s->fps_frame->duration  = s->dst_format.frame_duration;

      //  fprintf(stderr, "FPS frame: %ld %ld\n",
      //          s->fps_frame->timestamp,
      //          s->fps_frame->duration);
  
  if(!*frame)
    *frame = s->fps_frame;
  else
    {
    gavl_video_frame_copy(&s->dst_format, *frame, s->fps_frame);
    gavl_video_frame_copy_metadata(*frame, s->fps_frame);
    }
  return GAVL_SOURCE_OK;
  }

static gavl_source_status_t
read_video_still(gavl_video_source_t * s,
                 gavl_video_frame_t ** frame)
  {
  gavl_source_status_t st;

  if(!s->next_still_frame)
    {
    if(!(s->src_flags & GAVL_SOURCE_SRC_ALLOC))
      s->next_still_frame = gavl_video_frame_pool_get(s->src_fp);
    
    st = s->read_frame(s, &s->next_still_frame);
    
    switch(st)
      {
      case GAVL_SOURCE_OK:
        if(s->next_pts == GAVL_TIME_UNDEFINED)
          s->next_pts =
            gavl_time_rescale(s->src_format.timescale,
                              s->dst_format.timescale,
                              s->next_still_frame->timestamp);
        break;
      case GAVL_SOURCE_AGAIN:
        if(s->next_still_frame)
          {
          s->next_still_frame->refcount = 0;
          s->next_still_frame = NULL;
          }
        break;
      case GAVL_SOURCE_EOF:
        return st;
        break;
      }
    }

  /* Check if our frame expired */
  if(s->next_still_frame &&
     gavl_time_rescale(s->src_format.timescale,
                       s->dst_format.timescale,
                       s->next_still_frame->timestamp) >= s->next_pts)
    {
    if(!s->dst_fp)
      s->dst_fp = gavl_video_frame_pool_create(NULL, &s->dst_format);
    if(!s->fps_frame)
      s->fps_frame = gavl_video_frame_pool_get(s->dst_fp);

    /* Convert into local buffer */
    if(s->flags & FLAG_DO_CONVERT)
      {
      gavl_video_convert(s->cnv, s->next_still_frame, s->fps_frame);
      }
    else
      {
      gavl_video_frame_copy(&s->dst_format, s->fps_frame, s->next_still_frame);
      gavl_video_frame_copy_metadata(s->fps_frame, s->next_still_frame);
      }
    s->next_still_frame->refcount = 0;
    s->next_still_frame = NULL;
    }

  s->fps_frame->timestamp = s->next_pts;
  s->fps_frame->duration = s->dst_format.frame_duration;
  s->next_pts += s->dst_format.frame_duration;
  
  if(!(*frame))
    *frame = s->fps_frame;
  else
    {
    gavl_video_frame_copy(&s->dst_format, s->fps_frame, s->next_still_frame);
    gavl_video_frame_copy_metadata(s->fps_frame, s->next_still_frame);
    }
  return GAVL_SOURCE_OK;
  }

void gavl_video_source_set_dst(gavl_video_source_t * s, int dst_flags,
                               const gavl_video_format_t * dst_format)
  {
  int convert_fps;
  int convert_still;
  gavl_video_format_t dst_fmt;
  
  s->dst_flags = dst_flags;
  if(dst_format)
    gavl_video_format_copy(&s->dst_format, dst_format);
  else
    gavl_video_format_copy(&s->dst_format, &s->src_format);
  
  gavl_video_format_copy(&dst_fmt, &s->dst_format);
  
  dst_fmt.framerate_mode = s->src_format.framerate_mode;
  dst_fmt.timescale      = s->src_format.timescale;
  dst_fmt.frame_duration = s->src_format.frame_duration;
    
  if(gavl_video_converter_init(s->cnv, &s->src_format, &dst_fmt))
    s->flags |= FLAG_DO_CONVERT;
  else
    s->flags &= ~FLAG_DO_CONVERT;

  s->flags &= ~FLAG_SCALE_TIMESTAMPS;
  
  convert_fps = 0;
  convert_still = 0;
  
  if(s->dst_format.framerate_mode == GAVL_FRAMERATE_CONSTANT)
    {
    if((s->src_format.framerate_mode == GAVL_FRAMERATE_VARIABLE) ||
       ((s->src_format.framerate_mode == GAVL_FRAMERATE_CONSTANT) &&
       (s->src_format.timescale * s->dst_format.frame_duration !=
        s->dst_format.timescale * s->src_format.frame_duration)))
      {
      convert_fps = 1;
      }
    else if(s->src_format.framerate_mode == GAVL_FRAMERATE_STILL)
      {
      convert_still = 1;
      }
    }
  
  if(!convert_fps && !convert_still)
    {
    if(s->src_format.timescale != s->dst_format.timescale)
      s->flags |= FLAG_SCALE_TIMESTAMPS;
    }

  /* Check if we need to transfer frames to RAM */
  if(convert_fps || convert_still || (s->flags & FLAG_DO_CONVERT))
    s->dst_format.hwctx = NULL;

  if(s->src_format.hwctx && !s->dst_format.hwctx)
    s->flags |= FLAG_HW_TO_RAM;
  
  if(convert_fps)
    s->read_video = read_video_fps;
  else if(convert_still)
    s->read_video = read_video_still;
  else if(s->flags & FLAG_DO_CONVERT)
    s->read_video = read_video_cnv;
  else
    s->read_video = read_video_simple;

  if(s->flags & FLAG_HW_TO_RAM) 
    s->read_frame = read_frame_transfer;
  else
    s->read_frame = read_frame;

  if(s->src_fp)
    {
    gavl_video_frame_pool_destroy(s->src_fp);
    s->src_fp = NULL;
    }

  if(s->transfer_frame)
    {
    gavl_video_frame_destroy(s->transfer_frame);
    s->transfer_frame = NULL;
    }
  
  gavl_video_source_reset(s);
  
  if(!(s->src_flags & GAVL_SOURCE_SRC_ALLOC))
    s->src_fp = gavl_video_frame_pool_create(NULL, &s->src_format);
  
  s->flags |= FLAG_DST_SET;
  }
  
gavl_source_status_t
gavl_video_source_read_frame(void * sp, gavl_video_frame_t ** frame)
  {
  gavl_video_source_t * s = sp;

  if(gavl_video_source_get_eof(s))
    return GAVL_SOURCE_EOF;
    
  if(!(s->flags & FLAG_DST_SET))
    gavl_video_source_set_dst(s, 0, NULL);
  
  if(!frame)
    {
    /* Forget our status */
    gavl_video_source_reset(s);
    
    /* Skip one frame as cheaply as possible */
    return s->read_frame(s, NULL);
    }
  else
    return s->read_video(s, frame);
  }

void gavl_video_source_drain(gavl_video_source_t * s)
  {
  gavl_source_status_t st;

  gavl_video_frame_t * fr = NULL;
  while((st = gavl_video_source_read_frame(s, &fr)) == GAVL_SOURCE_OK)
    fr = NULL;
  }

void gavl_video_source_drain_nolock(gavl_video_source_t * s)
  {
  s->have_lock = 1;
  gavl_video_source_drain(s);
  s->have_lock = 0;
  }

void gavl_video_source_set_eof(gavl_video_source_t * src, int eof)
  {
  pthread_mutex_lock(&src->eof_mutex);
  src->eof = eof;
  pthread_mutex_unlock(&src->eof_mutex);
  }

int gavl_video_source_get_eof(gavl_video_source_t * src)
  {
  int ret;
  pthread_mutex_lock(&src->eof_mutex);
  ret = src->eof;
  pthread_mutex_unlock(&src->eof_mutex);
  return ret;
  }

