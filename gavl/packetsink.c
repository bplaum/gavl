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
#include <gavl/connectors.h>

#define FLAG_GET_CALLED (1<<0)


struct gavl_packet_sink_s
  {
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


gavl_packet_t *
gavl_packet_sink_get_packet(gavl_packet_sink_t * s)
  {
  gavl_packet_t * ret;
  s->flags |= FLAG_GET_CALLED;

  if(s->get_func)
    {
    if(s->lock_func)
      s->lock_func(s->lock_priv);
    
    ret = s->get_func(s->priv);
    
    if(s->unlock_func)
      s->unlock_func(s->lock_priv);
    return ret;
    }
  else
    return NULL;
  }

gavl_sink_status_t
gavl_packet_sink_put_packet(gavl_packet_sink_t * s, gavl_packet_t * p)
  {
  gavl_packet_t * dp;
  gavl_sink_status_t st;
  
  if(s->lock_func)
    s->lock_func(s->lock_priv);
    
  if(!(s->flags & FLAG_GET_CALLED) &&
     s->get_func &&
     (dp = s->get_func(s->priv)))
    {
    gavl_packet_copy(dp, p);
    st = s->put_func(s->priv, dp);
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
