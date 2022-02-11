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

#define FLAG_PASSTHROUGH      (1<<0)
#define FLAG_PASSTHROUGH_INIT (1<<1)
#define FLAG_DO_CONVERT       (1<<2)
#define FLAG_DST_SET          (1<<3)

struct gavl_audio_source_s
  {
  gavl_audio_format_t src_format;
  gavl_audio_format_t dst_format;
  int src_flags;
  int dst_flags;

  int64_t next_pts;

  /* Samples in the frame from which we buffer */
  int frame_samples;

  /*
   *  Samples in the output frame from the last incomplete call
   */
  int incomplete_samples;
  
  /* For conversion */
  gavl_audio_frame_t * in_frame;
  gavl_audio_frame_t * out_frame;
  gavl_audio_frame_t * dst_frame;

  /* For buffering */
  gavl_audio_frame_t * buffer_frame;
  
  gavl_audio_frame_t * frame;
  
  /* Callback set by the client */
  
  gavl_audio_source_func_t func;
  void * priv;
  gavl_audio_converter_t * cnv;

  int flags;

  int skip_samples;
  int64_t skip_time;
  
  gavl_connector_lock_func_t lock_func;
  gavl_connector_lock_func_t unlock_func;
  void * lock_priv;

  gavl_audio_source_t * prev;

  gavl_connector_free_func_t free_func;

  pthread_mutex_t eof_mutex;
  int eof;
  
  int have_lock;
  };

gavl_audio_source_t *
gavl_audio_source_create(gavl_audio_source_func_t func,
                         void * priv,
                         int src_flags,
                         const gavl_audio_format_t * src_format)
  {
  gavl_audio_source_t * ret = calloc(1, sizeof(*ret));

  ret->func = func;
  ret->priv = priv;
  ret->src_flags = src_flags;
  gavl_audio_format_copy(&ret->src_format, src_format);
  ret->cnv = gavl_audio_converter_create();
  pthread_mutex_init(&ret->eof_mutex, NULL);

  ret->skip_time = GAVL_TIME_UNDEFINED;
  
  return ret;
  }

gavl_audio_source_t *
gavl_audio_source_create_source(gavl_audio_source_func_t func,
                                void * priv,
                                int src_flags,
                                gavl_audio_source_t * src)
  {
  gavl_audio_source_t * ret =
    gavl_audio_source_create(func, priv, src_flags,
                             gavl_audio_source_get_dst_format(src));
  
  ret->prev = src;
  return ret;
  }

void
gavl_audio_source_set_lock_funcs(gavl_audio_source_t * src,
                                 gavl_connector_lock_func_t lock_func,
                                 gavl_connector_lock_func_t unlock_func,
                                 void * priv)
  {
  src->lock_func = lock_func;
  src->unlock_func = unlock_func;
  src->lock_priv = priv;
  }


const gavl_audio_format_t *
gavl_audio_source_get_src_format(gavl_audio_source_t * s)
  {
  return &s->src_format;
  }

const gavl_audio_format_t *
gavl_audio_source_get_dst_format(gavl_audio_source_t * s)
  {
  return (s->flags & FLAG_DST_SET) ? &s->dst_format : &s->src_format;
  }

int
gavl_audio_source_get_dst_flags(gavl_audio_source_t * s)
  {
  return s->dst_flags;
  }

gavl_audio_options_t * gavl_audio_source_get_options(gavl_audio_source_t * s)
  {
  return gavl_audio_converter_get_options(s->cnv);
  }

void gavl_audio_source_reset(gavl_audio_source_t * s)
  {
  s->next_pts = GAVL_TIME_UNDEFINED;
  if(s->frame)
    s->frame = NULL;
  s->skip_samples = 0;

  if(s->flags & FLAG_PASSTHROUGH_INIT)
    s->flags |= FLAG_PASSTHROUGH;

  s->incomplete_samples = 0;
  
#if 0
  if(s->src_fp)
    gavl_audio_frame_pool_reset(s->src_fp);
  if(s->dst_fp)
    gavl_audio_frame_pool_reset(s->dst_fp);
#endif
  }

void
gavl_audio_source_set_free_func(gavl_audio_source_t * src,
                                gavl_connector_free_func_t free_func)
  {
  src->free_func = free_func;
  }


void gavl_audio_source_destroy(gavl_audio_source_t * s)
  {
  if(s->out_frame)
    gavl_audio_frame_destroy(s->out_frame);
  if(s->in_frame)
    gavl_audio_frame_destroy(s->in_frame);
  if(s->dst_frame)
    gavl_audio_frame_destroy(s->dst_frame);
  if(s->buffer_frame)
    gavl_audio_frame_destroy(s->buffer_frame);
  
  
  gavl_audio_converter_destroy(s->cnv);

  if(s->priv && s->free_func)
    s->free_func(s->priv);

  pthread_mutex_destroy(&s->eof_mutex);

  free(s);
  }

void
gavl_audio_source_set_dst(gavl_audio_source_t * s, int dst_flags,
                          const gavl_audio_format_t * dst_format)
  {
  s->next_pts = GAVL_TIME_UNDEFINED;
  s->dst_flags = dst_flags;

  if(dst_format)
    gavl_audio_format_copy(&s->dst_format, dst_format);
  else
    gavl_audio_format_copy(&s->dst_format, &s->src_format);

  if(gavl_audio_converter_init(s->cnv,
                               &s->src_format, &s->dst_format))
    s->flags |= FLAG_DO_CONVERT;
  else
    s->flags &= ~FLAG_DO_CONVERT;
  
  if(!(s->flags & FLAG_DO_CONVERT) &&
     (s->src_format.samples_per_frame == s->dst_format.samples_per_frame) &&
     !(s->src_flags & GAVL_SOURCE_SRC_FRAMESIZE_MAX))
    s->flags |= (FLAG_PASSTHROUGH | FLAG_PASSTHROUGH_INIT);
  else
    s->flags &= ~(FLAG_PASSTHROUGH | FLAG_PASSTHROUGH_INIT);
  
  if(s->out_frame)
    {
    gavl_audio_frame_destroy(s->out_frame);
    s->out_frame = NULL;
    }
  if(s->dst_frame)
    {
    gavl_audio_frame_destroy(s->dst_frame);
    s->dst_frame = NULL;
    }
  if(s->buffer_frame)
    {
    gavl_audio_frame_destroy(s->buffer_frame);
    s->buffer_frame = NULL;
    }

  s->frame = NULL;

  s->flags |= FLAG_DST_SET;

  }

static void check_out_frame(gavl_audio_source_t * s)
  {
  if(!s->out_frame)
    {
    gavl_audio_format_t frame_format;
    gavl_audio_format_copy(&frame_format, &s->dst_format);
    frame_format.samples_per_frame =
      gavl_time_rescale(s->src_format.samplerate,
                        s->dst_format.samplerate,
                        s->src_format.samples_per_frame) + 10;
    s->out_frame = gavl_audio_frame_create(&frame_format);
    }
  }

static int process_input(gavl_audio_source_t * s, gavl_audio_frame_t * f)
  {
  if(s->next_pts == GAVL_TIME_UNDEFINED)
    s->next_pts = gavl_time_rescale(s->src_format.samplerate,
                                    s->dst_format.samplerate,
                                    f->timestamp);

  if(s->skip_time != GAVL_TIME_UNDEFINED)
    {
    s->skip_samples = s->skip_time - f->timestamp;

    if(s->skip_samples < 0)
      {
      fprintf(stderr, "audio_source: Cannot skip backwards\n");
      s->skip_samples = 0;
      }
    s->skip_time = GAVL_TIME_UNDEFINED;
    }
  
  if(s->skip_samples)
    {
    int skipped = gavl_audio_frame_skip(&s->src_format,
                                        f, s->skip_samples);;
    s->skip_samples -= skipped;
    s->next_pts += skipped;

    if(!f->valid_samples)
      return 0;
    else
      s->next_pts = gavl_time_rescale(s->src_format.samplerate,
                                      s->dst_format.samplerate,
                                      f->timestamp);
    }

  //  fprintf(stderr, "Process input %ld\n", f->valid_samples);
  
  return 1;
  }

static void process_output(gavl_audio_source_t * s,
                           gavl_audio_frame_t * f)
  {
  //  fprintf(stderr, "Process output %ld %ld\n", s->next_pts, f->valid_samples);
  f->timestamp = s->next_pts;
  s->next_pts += f->valid_samples;
  }

static gavl_source_status_t do_read(gavl_audio_source_t * s,
                                    gavl_audio_frame_t ** frame)
  {
  gavl_source_status_t ret;
  if(s->lock_func && !s->have_lock)
    s->lock_func(s->lock_priv);

  ret = s->func(s->priv, frame);
  
  if(s->unlock_func && !s->have_lock)
    s->unlock_func(s->lock_priv);

  //  if(frame && *frame)
  //    fprintf(stderr, "Read frame %p [%d samples]\n", *frame, (*frame)->valid_samples);
  
  return ret;
  }


static gavl_source_status_t
read_frame_internal(void * sp, gavl_audio_frame_t ** frame, int num_samples)
  {
  gavl_audio_source_t * s = sp;
  int samples_read = s->incomplete_samples;
  int samples_copied;
  gavl_source_status_t ret = GAVL_SOURCE_OK;
  int eat_all = 0;
  
  s->incomplete_samples = 0;
  
  
  while(samples_read < num_samples)
    {
    
    /* Read new frame if neccesary */
    if(!s->frame || !s->frame->valid_samples)
      {
      eat_all = 0;
      /* Check for passthrough */
      if(s->flags & FLAG_PASSTHROUGH)
        {
        if((*frame && !(s->src_flags & GAVL_SOURCE_SRC_ALLOC)) ||
           (!(*frame) && (s->src_flags & GAVL_SOURCE_SRC_ALLOC)))
          {

          ret = do_read(s, frame);
          if(ret == GAVL_SOURCE_OK)
            {
            process_input(s, *frame);
            process_output(s, *frame);
            }
          return ret;
          }
        }
      if(s->flags & FLAG_DO_CONVERT)
        {
        gavl_audio_frame_t * in_frame;

        if(s->src_flags & GAVL_SOURCE_SRC_ALLOC)
          in_frame = NULL;
        else
          {
          if(!s->in_frame)
            s->in_frame = gavl_audio_frame_create(&s->src_format);
          in_frame = s->in_frame;
          }
        
        ret = do_read(s, &in_frame);
        
        if(ret != GAVL_SOURCE_OK)
          break;
        
        if(!process_input(s, in_frame))
          continue;
        
        /* Get out frame */
        check_out_frame(s);
        gavl_audio_convert(s->cnv, in_frame, s->out_frame);
        s->frame = s->out_frame;
#if 0        
        if(s->src_format.samplerate != s->dst_format.samplerate)
          fprintf(stderr, "Converted frame (resample) %d %d\n",
                  in_frame->valid_samples, s->out_frame->valid_samples);
#endif
        }
      else
        {
        if(s->src_flags & GAVL_SOURCE_SRC_ALLOC)
          {
          s->frame = NULL;
          ret = do_read(s, &s->frame);
          if(ret != GAVL_SOURCE_OK)
            break;

          if(!process_input(s, s->frame))
            continue;

          eat_all = 1;
          }
        else
          {

          check_out_frame(s);
          ret = do_read(s, &s->out_frame);
          if(ret != GAVL_SOURCE_OK)
            break;
          s->frame = s->out_frame;
          if(!process_input(s, s->frame))
            continue;
          }
        }
      s->frame_samples = s->frame->valid_samples;
      }

    /* Make sure we have a frame to write to */
    if(!(*frame))
      {
      if(!s->dst_frame)
        s->dst_frame = gavl_audio_frame_create(&s->dst_format);
      *frame = s->dst_frame;
      }
    
    /* Copy samples */

    
    samples_copied =
      gavl_audio_frame_copy(&s->dst_format,
                            *frame,                                     // dst
                            s->frame,                                   // src
                            samples_read,                               // dst_pos
                            s->frame_samples - s->frame->valid_samples, // src_pos
                            num_samples - samples_read,                 // dst_size
                            s->frame->valid_samples);                   // src_size
    s->frame->valid_samples -= samples_copied;
    samples_read += samples_copied;
    }
  
  if(ret == GAVL_SOURCE_AGAIN)
    {
    s->incomplete_samples = samples_read;
    return GAVL_SOURCE_AGAIN;
    }
  
  if(samples_read)
    {
    ret = GAVL_SOURCE_OK;
    (*frame)->valid_samples = samples_read;
    process_output(s, *frame);

    /* Buffer samples for next time (we need to eat up all samples in this call) */
    if(eat_all && s->frame->valid_samples)
      {
      if(!s->buffer_frame)
        s->buffer_frame = gavl_audio_frame_create(&s->src_format);
      
      s->buffer_frame->valid_samples = 
        gavl_audio_frame_copy(&s->src_format,
                              s->buffer_frame,                            // dst
                              s->frame,                                   // src
                              0,                                          // dst_pos
                              s->frame_samples - s->frame->valid_samples, // src_pos
                              s->src_format.samples_per_frame,            // dst_size
                              s->frame->valid_samples);                   // src_size
      
      s->frame = s->buffer_frame;
      s->frame_samples = s->frame->valid_samples;
      }
    }
  else if(*frame)
    (*frame)->valid_samples = 0;
  
  return ret;
  }

gavl_source_status_t
gavl_audio_source_read_frame(void * sp, gavl_audio_frame_t ** frame)
  {
  gavl_audio_source_t * s = sp;

  if(gavl_audio_source_get_eof(s))
    return GAVL_SOURCE_EOF;

  if(!(s->flags & FLAG_DST_SET))
    gavl_audio_source_set_dst(s, 0, NULL);
  return read_frame_internal(s, frame, s->dst_format.samples_per_frame);
  }

void gavl_audio_source_drain(gavl_audio_source_t * s)
  {
  gavl_source_status_t st;

  gavl_audio_frame_t * fr = NULL;
  while((st = gavl_audio_source_read_frame(s, &fr)) == GAVL_SOURCE_OK)
    fr = NULL;
  }

void gavl_audio_source_drain_nolock(gavl_audio_source_t * s)
  {
  s->have_lock = 1;
  gavl_audio_source_drain(s);
  s->have_lock = 0;
  }

/* For cases where it's not immediately known, how many samples will be
   processed */

int gavl_audio_source_read_samples(void * sp, gavl_audio_frame_t * frame,
                                   int num_samples)
  {
  gavl_audio_source_t * s = sp;
  if(!(s->flags & FLAG_DST_SET))
    gavl_audio_source_set_dst(s, 0, NULL);
  
  s->flags &= ~FLAG_PASSTHROUGH;
  if(read_frame_internal(s, &frame, num_samples) != GAVL_SOURCE_OK)
    return 0;
  return frame->valid_samples;
  }

void 
gavl_audio_source_skip(gavl_audio_source_t * s, int num_samples)
  {
  s->skip_samples += num_samples;
  s->flags &= ~FLAG_PASSTHROUGH;
  }

void 
gavl_audio_source_skip_to(gavl_audio_source_t * s,
                          int64_t t, int scale)
  {
  s->skip_time = gavl_time_rescale(scale, s->dst_format.samplerate, t);
  s->flags &= ~FLAG_PASSTHROUGH;
  }

void gavl_audio_source_set_eof(gavl_audio_source_t * src, int eof)
  {
  pthread_mutex_lock(&src->eof_mutex);
  src->eof = eof;
  pthread_mutex_unlock(&src->eof_mutex);
  }

int gavl_audio_source_get_eof(gavl_audio_source_t * src)
  {
  int ret;
  pthread_mutex_lock(&src->eof_mutex);
  ret = src->eof;
  pthread_mutex_unlock(&src->eof_mutex);
  return ret;
  }
