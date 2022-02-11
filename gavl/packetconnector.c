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


typedef struct
  {
  gavl_packet_t * p;
  gavl_packet_sink_t * sink;
  } sink_t;

struct gavl_packet_connector_s
  {
  gavl_packet_source_t * src;

  int have_in_packet;
  gavl_packet_t * in_packet;
  
  sink_t * sinks;
  int num_sinks;
  int sinks_alloc;
  
  gavl_packet_connector_process_func process_func;
  void * process_priv;
  gavl_source_status_t src_st;
  };

gavl_packet_connector_t *
gavl_packet_connector_create(gavl_packet_source_t * src)
  {
  gavl_packet_connector_t * ret = calloc(1, sizeof(*ret));
  ret->src = src;
  return ret;
  }

void
gavl_packet_connector_destroy(gavl_packet_connector_t * c)
  {
  if(c->sinks)
    free(c->sinks);
  free(c);
  }

void
gavl_packet_connector_connect(gavl_packet_connector_t * c,
                              gavl_packet_sink_t * sink)
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
  c->num_sinks++;
  }

void
gavl_packet_connector_set_process_func(gavl_packet_connector_t * c,
                                       gavl_packet_connector_process_func func,
                                       void * priv)
  {
  c->process_func = func;
  c->process_priv = priv;
  }

int
gavl_packet_connector_process(gavl_packet_connector_t * c)
  {
  gavl_sink_status_t sink_st;
  int i;
  sink_t * s;
  
  if(!c->have_in_packet)
    {
    c->in_packet = NULL;
    /* Obtain buffer for input packet */
    for(i = 0; i < c->num_sinks; i++)
      {
      s = c->sinks + i;
      /* Passthrough */
      s->p = gavl_packet_sink_get_packet(s->sink);
      if(!c->in_packet)
        c->in_packet = s->p;
      }
    c->have_in_packet = 1;
    }
  
  /* Get input frame */
  c->src_st = gavl_packet_source_read_packet(c->src, &c->in_packet);

  switch(c->src_st)
    {
    case GAVL_SOURCE_OK:
      break;
    case GAVL_SOURCE_AGAIN:
      return 1;
      break;
    case GAVL_SOURCE_EOF:
      return 0;
    }

  if(c->process_func)
    c->process_func(c->process_priv, c->in_packet);
  
  for(i = 0; i < c->num_sinks; i++)
    {
    s = c->sinks + i;
    if(!s->p)
      sink_st = gavl_packet_sink_put_packet(s->sink, c->in_packet);
    else
      {
      if(s->p != c->in_packet)
        gavl_packet_copy(s->p, c->in_packet);
      sink_st = gavl_packet_sink_put_packet(s->sink, s->p);
      }
    if(sink_st != GAVL_SINK_OK)
      return 0;
    }
  
  c->have_in_packet = 0;
  return 1;
  }

gavl_source_status_t gavl_packet_connector_get_source_status(gavl_packet_connector_t * c)
  {
  return c->src_st;
  }


