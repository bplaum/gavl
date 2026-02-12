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

#include <pthread.h>

#include <config.h>
#include <gavl/connectors.h>
#include <gavl/hw.h>


#include <gavl/log.h>
#define LOG_DOMAIN "videosource"

#define FLAG_DO_CONVERT       (1<<0)
#define FLAG_DST_SET          (1<<1)
#define FLAG_SCALE_TIMESTAMPS (1<<2)
#define FLAG_SUPPORT_HW       (1<<3) /* Support HW storage */

#define FLAG_HW_TO_RAM        (1<<4) /* Transfer frames from hardware to RAM */

#define FLAG_EOS              (1<<5) /* End of stream */

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
  int64_t pts;
  
  int flags;

  gavl_video_frame_t * in_frame;
  gavl_video_frame_t * next_in_frame;
  
  gavl_video_frame_t * out_frame;
  
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

  int64_t pts_offset;
  };

static gavl_video_frame_t * create_in_frame(gavl_video_source_t * src)
  {
  gavl_video_frame_t * ret;
  
  if(src->flags & FLAG_HW_TO_RAM)
    ret = gavl_video_frame_create(&src->src_format);
  else if(src->src_format.hwctx)
    ret = gavl_hw_video_frame_create(src->src_format.hwctx, 1);
  else
    ret = gavl_video_frame_create(&src->src_format);

  ret->timestamp = GAVL_TIME_UNDEFINED;
  return ret;
  }

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

  return ret;
  }

void gavl_video_source_set_pts_offset(gavl_video_source_t * src, int64_t offset)
  {
  src->pts_offset = offset;

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

int
gavl_video_source_get_src_flags(gavl_video_source_t * s)
  {
  return s->src_flags;
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
  s->pts = GAVL_TIME_UNDEFINED;

  if(s->in_frame)
    s->in_frame->timestamp = GAVL_TIME_UNDEFINED;

  if(s->next_in_frame)
    s->next_in_frame->timestamp = GAVL_TIME_UNDEFINED;

  if(s->out_frame)
    s->out_frame->timestamp = GAVL_TIME_UNDEFINED;
  
  s->flags &= ~FLAG_EOS;
  
  }

GAVL_PUBLIC
void gavl_video_source_destroy(gavl_video_source_t * s)
  {
  
  if(s->in_frame)
    gavl_video_frame_destroy(s->in_frame);

  if(s->next_in_frame)
    gavl_video_frame_destroy(s->next_in_frame);

  if(s->out_frame)
    gavl_video_frame_destroy(s->out_frame);
  
  gavl_video_converter_destroy(s->cnv);

  if(s->priv && s->free_func)
    s->free_func(s->priv);

  free(s);
  }

static void scale_pts(gavl_video_source_t * s,
                      gavl_video_frame_t * f)
  {
  if(!(s->flags & FLAG_SCALE_TIMESTAMPS))
    {
    f->timestamp += s->pts_offset;
    return;
    }
  if(f->duration != GAVL_TIME_UNDEFINED)
    {
    int64_t timestamp_scaled;
    
    timestamp_scaled = gavl_time_rescale(s->src_format.timescale,
                                         s->dst_format.timescale,
                                         f->timestamp);
      
    f->duration = gavl_time_rescale(s->src_format.timescale,
                                    s->dst_format.timescale,
                                    f->timestamp + f->duration) -
      timestamp_scaled;
    f->timestamp = timestamp_scaled;
    }
  else
    f->timestamp = gavl_time_rescale(s->src_format.timescale,
                                     s->dst_format.timescale,
                                     f->timestamp);
  
  f->timestamp += s->pts_offset;
  }

static gavl_source_status_t read_frame(gavl_video_source_t * s,
                                       gavl_video_frame_t ** frame)
  {
  gavl_source_status_t ret;

  if(s->lock_func) 
    s->lock_func(s->lock_priv);

  ret = s->func(s->priv, frame);
  
  if(s->unlock_func)
    s->unlock_func(s->lock_priv);

  if(ret != GAVL_SOURCE_OK)
    return ret;

  if(frame && *frame)
    {
    if(((*frame)->timestamp != GAVL_TIME_UNDEFINED) &&
       (s->pts == GAVL_TIME_UNDEFINED))
      s->pts = gavl_time_rescale(s->src_format.timescale,
                                 s->dst_format.timescale,
                                 (*frame)->timestamp);
    }
  return ret;
  }

static gavl_source_status_t read_frame_transfer(gavl_video_source_t * s,
                                                gavl_video_frame_t ** frame)
  {
  gavl_source_status_t ret;
  gavl_video_frame_t * tmp_frame = NULL;

  if(s->lock_func)
    s->lock_func(s->lock_priv);

  ret = s->func(s->priv, &tmp_frame);

  if(ret == GAVL_SOURCE_OK)
    {
    if(!gavl_video_frame_hw_to_ram(*frame,
                                   tmp_frame))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Frame transfer failed");
      ret = GAVL_SOURCE_EOF;
      }
    else
      {
      if((tmp_frame->timestamp != GAVL_TIME_UNDEFINED) &&
         (s->pts == GAVL_TIME_UNDEFINED))
        s->pts = gavl_time_rescale(s->src_format.timescale,
                                   s->dst_format.timescale,
                                   tmp_frame->timestamp);
      }
    }
  
  if(s->unlock_func)
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
    {
    if(!s->in_frame)
      s->in_frame = create_in_frame(s);
    in_frame = s->in_frame;
    }
  else
    in_frame = NULL;
  
  if(!(*frame))
    {
    if(!s->out_frame)
      s->out_frame = gavl_video_frame_create(&s->dst_format);
    *frame = s->out_frame;
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
    {
    if(!s->in_frame)
      s->in_frame = create_in_frame(s);
    in_frame = s->in_frame;
    }
  
  if((st = s->read_frame(s, &in_frame)) != GAVL_SOURCE_OK)
    return st;

  if(!(*frame))
    {
    if(!s->out_frame)
      s->out_frame = gavl_video_frame_create(&s->dst_format);
    *frame = s->out_frame;
    }
  
  gavl_video_convert(s->cnv, in_frame, *frame);
  scale_pts(s, *frame);
  return GAVL_SOURCE_OK;
  }

static void next_in_frame_fps(gavl_video_source_t * s)
  {
  gavl_video_frame_t * sav = s->in_frame;
  s->in_frame = s->next_in_frame;
  s->next_in_frame = sav;

  if(s->out_frame)
    s->out_frame->timestamp = GAVL_TIME_UNDEFINED;
  }

static void put_frame_fps(gavl_video_source_t * s,
                          gavl_video_frame_t ** frame,
                          gavl_video_frame_t * in_frame)
  {
  if(*frame)
    {
    if(s->flags & FLAG_DO_CONVERT)
      {
      if(!s->out_frame)
        s->out_frame = gavl_video_frame_create(&s->dst_format);

      if(s->out_frame->timestamp == GAVL_TIME_UNDEFINED)
        gavl_video_convert(s->cnv, in_frame, s->out_frame);
      
      gavl_video_frame_copy(&s->dst_format, *frame, s->out_frame);
      }
    else
      {
      gavl_video_frame_copy(&s->dst_format, *frame, in_frame);
      }
    }
  else
    {
    if(s->flags & FLAG_DO_CONVERT)
      {
      if(!s->out_frame)
        {
        s->out_frame = gavl_video_frame_create(&s->dst_format);
        s->out_frame->timestamp = GAVL_TIME_UNDEFINED;
        }
      
      if(s->out_frame->timestamp == GAVL_TIME_UNDEFINED)
        gavl_video_convert(s->cnv, in_frame, s->out_frame);
      }
    else
      {
      *frame = in_frame;
      }
    }

  (*frame)->timestamp = s->pts;
  (*frame)->duration = s->dst_format.frame_duration;

  s->pts += s->dst_format.frame_duration;
  
  }

static gavl_source_status_t
read_video_fps(gavl_video_source_t * s,
               gavl_video_frame_t ** frame)
  {
  gavl_source_status_t st;

  if(s->flags & FLAG_EOS)
    return GAVL_SOURCE_EOF;
  
  if(!s->in_frame)
    {
    s->in_frame = create_in_frame(s);
    s->next_in_frame = create_in_frame(s);

    s->in_frame->timestamp = GAVL_TIME_UNDEFINED;
    s->next_in_frame->timestamp = GAVL_TIME_UNDEFINED;
    }
  
  if(s->in_frame->timestamp == GAVL_TIME_UNDEFINED)
    {
    if((st = s->read_frame(s, &s->in_frame)) != GAVL_SOURCE_OK)
      return st;
    }

  /* Check if we moved out of the window */
  if((s->next_in_frame->timestamp != GAVL_TIME_UNDEFINED) &&
     (s->pts >= s->next_in_frame->timestamp))
    next_in_frame_fps(s);
  
  if(s->next_in_frame->timestamp == GAVL_TIME_UNDEFINED)
    {
    st = s->read_frame(s, &s->next_in_frame);
    if(st == GAVL_SOURCE_EOF)
      s->flags |= FLAG_EOS;
    }

  put_frame_fps(s, frame, s->in_frame);
  return GAVL_SOURCE_OK;
  }

void gavl_video_source_set_dst(gavl_video_source_t * s, int dst_flags,
                               const gavl_video_format_t * dst_format)
  {
  int convert_fps;
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
      convert_fps = 1;
      }
    }
  
  if(!convert_fps)
    {
    if(s->src_format.timescale != s->dst_format.timescale)
      s->flags |= FLAG_SCALE_TIMESTAMPS;
    }

  /* Check if we need to transfer frames to RAM */
  if(convert_fps || (s->flags & FLAG_DO_CONVERT))
    s->dst_format.hwctx = NULL;

  if(s->src_format.hwctx && !s->dst_format.hwctx)
    s->flags |= FLAG_HW_TO_RAM;
  
  if(convert_fps)
    s->read_video = read_video_fps;
  else if(s->flags & FLAG_DO_CONVERT)
    s->read_video = read_video_cnv;
  else
    s->read_video = read_video_simple;

  if(s->flags & FLAG_HW_TO_RAM) 
    {
    s->read_frame = read_frame_transfer;
    s->src_flags &= ~GAVL_SOURCE_SRC_ALLOC;
    }
  else
    s->read_frame = read_frame;
  
  gavl_video_source_reset(s);
  
  s->flags |= FLAG_DST_SET;
  }
  
gavl_source_status_t
gavl_video_source_read_frame(void * sp, gavl_video_frame_t ** frame)
  {
  gavl_video_source_t * s = sp;

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

