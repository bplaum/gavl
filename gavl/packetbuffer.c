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
// #include <stdio.h>
// #include <pthread.h>

#include <gavl/connectors.h>

struct gavl_packet_buffer_s
  {
  gavl_packet_sink_t * sink;
  gavl_packet_source_t * src;

  gavl_packet_t ** packets;

  int packets_alloc;
  int valid_packets;
  
  };



gavl_packet_buffer_t * gavl_packet_buffer_create()
  {
  gavl_packet_buffer_t * ret;

  ret = calloc(1, sizeof(*ret));
  return ret;
  }

void gavl_packet_buffer_destroy(gavl_packet_buffer_t * b)
  {

  if(b->src)
    gavl_packet_source_destroy(b->src);
  if(b->sink)
    gavl_packet_sink_destroy(b->sink);

  if(b->packets)
    {
    int i;
    for(i = 0; i < b->packets_alloc; i++)
      {
      if(b->packets[i])
        gavl_packet_destroy(b->packets[i]);
      }
    free(b->packets);
    }
  
  }

gavl_packet_sink_t * gavl_packet_buffer_get_sink(gavl_packet_buffer_t * b)
  {
  return b->sink;
  }

gavl_packet_source_t * gavl_packet_buffer_get_source(gavl_packet_buffer_t * b)
  {
  return b->src;
  }

