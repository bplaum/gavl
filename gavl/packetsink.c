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
#include <pthread.h>

#include <config.h>

#include <gavl/connectors.h>
#include <gavl/log.h>
#define LOG_DOMAIN "packetsink"

// #define FLAG_GET_CALLED (1<<0)


struct gavl_packet_sink_s
  {
  gavl_packet_t * pkt;
  
  gavl_packet_sink_get_func get_func;
  gavl_packet_sink_put_func put_func;
  void * priv;
  int flags;
  
  gavl_connector_lock_func_t lock_func;
  gavl_connector_lock_func_t unlock_func;
  void * lock_priv;
  gavl_connector_free_func_t free_func;
  };
  
gavl_packet_sink_t *
gavl_packet_sink_create(gavl_packet_sink_get_func get_func,
                        gavl_packet_sink_put_func put_func,
                        void * priv)
  {
  gavl_packet_sink_t * ret;
  ret = calloc(1, sizeof(*ret));

  ret->get_func = get_func;
  ret->put_func = put_func;
  ret->priv = priv;
  return ret;
  }

void
gavl_packet_sink_set_lock_funcs(gavl_packet_sink_t * sink,
                                gavl_connector_lock_func_t lock_func,
                                gavl_connector_lock_func_t unlock_func,
                                void * priv)
  {
  sink->lock_func = lock_func;
  sink->unlock_func = unlock_func;
  sink->lock_priv = priv;
  }

void gavl_packet_sink_reset(gavl_packet_sink_t * s)
  {
  s->pkt = NULL;
  }

gavl_packet_t *
gavl_packet_sink_get_packet(gavl_packet_sink_t * s)
  {
  //  gavl_packet_t * ret;
  //  s->flags |= FLAG_GET_CALLED;

  if(s->pkt)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "gavl_packet_sink_get_packet called twice %ld", pthread_self());
    return NULL;
    }
  
  if(s->get_func)
    {
    if(s->lock_func)
      s->lock_func(s->lock_priv);

    s->pkt = s->get_func(s->priv);
    
    if(s->unlock_func)
      s->unlock_func(s->lock_priv);
    
    gavl_packet_reset(s->pkt);
    return s->pkt;
    }
  else
    return NULL;
  }

gavl_sink_status_t
gavl_packet_sink_put_packet(gavl_packet_sink_t * s, gavl_packet_t * p)
  {
  //  gavl_packet_t * dp;
  gavl_sink_status_t st;
  
  if(s->lock_func)
    s->lock_func(s->lock_priv);
  
  if(!p)
    {
    if(s->pkt)
      s->put_func(s->priv, NULL);
    s->pkt = NULL;
    return GAVL_SINK_OK;
    }
  
  if(s->get_func)
    {
    if(!s->pkt)
      s->pkt = s->get_func(s->priv);

    if(s->pkt != p)
      gavl_packet_copy(s->pkt, p);
    
    st = s->put_func(s->priv, s->pkt);
    s->pkt = NULL;
    }
  else
    st = s->put_func(s->priv, p);
  
  if(s->unlock_func)
    s->unlock_func(s->lock_priv);
  return st;
  }

void
gavl_packet_sink_set_free_func(gavl_packet_sink_t * sink,
                               gavl_connector_free_func_t free_func)
  {
  sink->free_func = free_func;
  }

void
gavl_packet_sink_destroy(gavl_packet_sink_t * s)
  {
  if(s->priv && s->free_func)
    s->free_func(s->priv);
  free(s);
  }
