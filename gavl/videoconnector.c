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
#include <string.h>


#include <gavl/connectors.h>
#include <video.h>

typedef struct
  {
  gavl_video_frame_t * sink_frame;
  gavl_video_frame_t * in_frame;
  
  gavl_video_sink_t * sink;
  gavl_video_source_t * src;

  const gavl_video_format_t * fmt;
  
  int * penalties;
  gavl_video_connector_t * c;
  } sink_t;

struct gavl_video_connector_s
  {
  gavl_video_source_t * src;
  const gavl_video_format_t * fmt;
  gavl_video_frame_t * in_frame;
  
  sink_t * sinks;
  int num_sinks;
  int sinks_alloc;

  gavl_video_connector_process_func process_func;
  void * process_priv;
  
  gavl_video_options_t opt;

  int have_in_frame;
  gavl_source_status_t src_st;
  };

gavl_video_connector_t *
gavl_video_connector_create(gavl_video_source_t * src)
  {
  gavl_video_connector_t * ret = calloc(1, sizeof(*ret));
  ret->src = src;
  
  ret->fmt = gavl_video_source_get_src_format(ret->src);
  gavl_video_options_set_defaults(&ret->opt);
  return ret;
  }

gavl_video_options_t *
gavl_video_connector_get_options(gavl_video_connector_t * c)
  {
  return &c->opt;
  }
  
void gavl_video_connector_destroy(gavl_video_connector_t * c)
  {
  int i;

  for(i = 0; i < c->num_sinks; i++)
    {
    if(c->sinks[i].src)
      gavl_video_source_destroy(c->sinks[i].src);
    }
  
  if(c->sinks)
    free(c->sinks);
  free(c);
  }

void gavl_video_connector_connect(gavl_video_connector_t * c,
                                  gavl_video_sink_t * sink)
  {
  sink_t * s;
  if(c->num_sinks + 1 > c->sinks_alloc)
    {
    c->sinks_alloc += 8;
    c->sinks = realloc(c->sinks, c->sinks_alloc * sizeof(*c->sinks));
    memset(c->sinks + c->num_sinks, 0,
           (c->sinks_alloc - c->num_sinks) * sizeof(*c->sinks));
    }
  s = c->sinks + c->num_sinks;

  s->sink = sink;
  s->c = c;
  s->fmt = gavl_video_sink_get_format(s->sink);

  c->num_sinks++;
  }

void
gavl_video_connector_set_process_func(gavl_video_connector_t * c,
                                      gavl_video_connector_process_func func,
                                      void * priv)
  {
  c->process_func = func;
  c->process_priv = priv;
  }

static gavl_source_status_t read_func(void * priv, gavl_video_frame_t ** frame)
  {
  sink_t * s = priv;

  if(s->c->src_st == GAVL_SOURCE_EOF)
    return GAVL_SOURCE_EOF;
  
  if(s->in_frame)
    {
    *frame = s->in_frame;
    s->in_frame = NULL;
    return GAVL_SOURCE_OK;
    }
  else
    return GAVL_SOURCE_AGAIN;
  }

static int flush_src(sink_t * s)
  {
  gavl_source_status_t src_st;
  gavl_sink_status_t sink_st;
  
  while(1)
    {
    if(!s->sink_frame)
      s->sink_frame = gavl_video_sink_get_frame(s->sink);
    
    src_st = gavl_video_source_read_frame(s->src, &s->sink_frame);
    
    switch(src_st)
      {
      case GAVL_SOURCE_EOF:
        s->sink_frame = NULL;
        return 1;
      case GAVL_SOURCE_AGAIN:
        return 1;
      case GAVL_SOURCE_OK:
        break;
      }
    
    sink_st = gavl_video_sink_put_frame(s->sink, s->sink_frame);
    s->sink_frame = NULL;
    if(sink_st != GAVL_SINK_OK)
      return 0;
    }
  return 1;
  }

void gavl_video_connector_reset(gavl_video_connector_t * c)
  {
  int i;
  c->in_frame = NULL;
  c->have_in_frame = 0;
  c->src_st = GAVL_SOURCE_OK;
  for(i = 0; i < c->num_sinks; i++)
    {
    if(c->sinks[i].src)
      gavl_video_source_reset(c->sinks[i].src);
    }
  }

int gavl_video_connector_process(gavl_video_connector_t * c)
  {
  int i;
  sink_t * s;
  gavl_sink_status_t sink_st;

  if(!c->have_in_frame)
    {
    c->in_frame = NULL;
    /* Obtain buffer for input frame */
    for(i = 0; i < c->num_sinks; i++)
      {
      s = c->sinks + i;

      if(!s->src)
        {
        /* Passthrough */
        s->sink_frame = gavl_video_sink_get_frame(s->sink);
        if(!c->in_frame)
          c->in_frame = s->sink_frame;
        }
      }
    c->have_in_frame = 1;
    }
  
  /* Get input frame */

  c->src_st = gavl_video_source_read_frame(c->src, &c->in_frame);

  switch(c->src_st)
    {
    case GAVL_SOURCE_OK:
      break;
    case GAVL_SOURCE_AGAIN:
      return 1;
      break;
    case GAVL_SOURCE_EOF:
      for(i = 0; i < c->num_sinks; i++)
        {
        s = c->sinks + i;
        if(s->src && !flush_src(s))
          return 0;
        }
      return 0;
    }

  if(c->process_func)
    c->process_func(c->process_priv, c->in_frame);
  
  /* Flush sinks */

  for(i = 0; i < c->num_sinks; i++)
    {
    s = c->sinks + i;

    /* No src, pass frame directly */
    if(!s->src)
      {
      /* Passthrough */
      if(!s->sink_frame)
        sink_st = gavl_video_sink_put_frame(s->sink, c->in_frame);
      else
        {
        if(s->sink_frame != c->in_frame)
          {
          gavl_video_frame_copy(s->fmt, s->sink_frame, c->in_frame);
          gavl_video_frame_copy_metadata(s->sink_frame, c->in_frame);
          }
        sink_st = gavl_video_sink_put_frame(s->sink, s->sink_frame);
        }
      if(sink_st != GAVL_SINK_OK)
        return 0;
      }
    else
      {
      /* Pass frame through the sink->src */
      s->in_frame = c->in_frame;
      if(!flush_src(s))
        return 0;
      }
    }
  
  c->have_in_frame = 0;
  return 1;
  }

gavl_source_status_t gavl_video_connector_get_source_status(gavl_video_connector_t * c)
  {
  return c->src_st;
  }


#if 0
static int get_penalty(gavl_video_converter_t * cnv,
                       const gavl_video_format_t * src_fmt,
                       const gavl_video_format_t * dst_fmt)
  {
  return 0;
  }
#endif

void gavl_video_connector_start(gavl_video_connector_t * c)
  {
  int i;
  gavl_video_converter_t * cnv;
  sink_t * s;

  /* Handle trivial case early */
  if(!c->num_sinks)
    return;
  else if(c->num_sinks == 1) 
    {
    gavl_video_source_set_dst(c->src, 0, c->sinks->fmt);
    return;
    }
  else
    gavl_video_source_set_dst(c->src, 0, NULL);
  
  cnv = gavl_video_converter_create();
  gavl_video_options_copy(gavl_video_converter_get_options(cnv), &c->opt);
  
  for(i = 0; i < c->num_sinks; i++)
    {
    s = c->sinks + i;
    if((c->fmt->timescale != s->fmt->timescale) ||
       (c->fmt->framerate_mode != s->fmt->framerate_mode) ||
       gavl_video_converter_init(cnv, c->fmt, s->fmt))
      {
      s->src = gavl_video_source_create(read_func, s, GAVL_SOURCE_SRC_ALLOC, c->fmt);
      gavl_video_options_copy(gavl_video_source_get_options(s->src), &c->opt);
      gavl_video_source_set_dst(s->src, 0, s->fmt);
      }
    }
  gavl_video_converter_destroy(cnv);
  }

const gavl_video_format_t * 
gavl_video_connector_get_process_format(gavl_video_connector_t * c)
  {
  return gavl_video_source_get_dst_format(c->src);
  }
