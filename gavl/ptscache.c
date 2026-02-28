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

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/compression.h>
#include <gavl/log.h>

#define LOG_DOMAIN "ptscache"

#define ALLOC_SIZE 16

struct gavl_packet_pts_cache_s
  {
  gavl_packet_t *packets;
  int packets_alloc;
  int num_packets;
  
  int dynamic;
  };

gavl_packet_pts_cache_t * gavl_packet_pts_cache_create(int size)
  {
  gavl_packet_pts_cache_t * ret = calloc(1, sizeof(*ret));

  if(size > 0)
    {
    ret->packets = calloc(size, sizeof(*ret->packets));
    ret->packets_alloc = size;
    }
  else
    ret->dynamic = 1;
  
  return ret;
  }

void gavl_packet_pts_cache_destroy(gavl_packet_pts_cache_t * c)
  {
  if(c->packets)
    free(c->packets);
  free(c);
  }

void gavl_packet_pts_cache_push_packet(gavl_packet_pts_cache_t *c, const gavl_packet_t * pkt)
  {
  if(c->num_packets == c->packets_alloc)
    {
    if(c->dynamic)
      {
      c->packets_alloc += ALLOC_SIZE;
      c->packets = realloc(c->packets, c->packets_alloc * sizeof(*c->packets));
      memset(c->packets + c->num_packets, 0, (c->packets_alloc - c->num_packets) * sizeof(*c->packets));
      }
    else
      {
      gavl_log(GAVL_LOG_WARNING, LOG_DOMAIN, "PTS cache overflow");
      gavl_packet_pts_cache_get_first(c, NULL);
      }
    
    }
  memcpy(&c->packets[c->num_packets], pkt, sizeof(*pkt));
  
  memset(&c->packets[c->num_packets].buf, 0, sizeof(c->packets[c->num_packets].buf));
  c->packets[c->num_packets].buf_idx = -1;
  c->num_packets++;
  }

static void remove_packet(gavl_packet_pts_cache_t *c, int idx)
  {
  if(idx < c->num_packets - 1)
    {
    memmove(c->packets + idx, c->packets + (idx+1),
            sizeof(*c->packets) * (c->num_packets - 1 - idx));
    }
  c->num_packets--;
  }

static void return_packet(gavl_packet_pts_cache_t *c, int idx, gavl_packet_t * pkt)
  {
  if(pkt)
    memcpy(pkt, &c->packets[idx], sizeof(*pkt));

  remove_packet(c, idx);
  }

#if 0
static void return_video_frame(gavl_packet_pts_cache_t *c, int idx, gavl_video_frame_t * frame)
  {
  if(frame)
    gavl_packet_to_videoframe(&c->packets[idx], frame);

  remove_packet(c, idx);
  }
#endif

int gavl_packet_pts_cache_get_first(gavl_packet_pts_cache_t *c, gavl_video_frame_t * f)
  {
  int i;
  int min_idx = 0;
  int64_t min_pts;

  gavl_packet_t pkt;
  
  if(c->num_packets == 0)
    return 0;
  
  min_pts = c->packets[0].pts;
  min_idx = 0;

  for(i = 1; i < c->num_packets; i++)
    {
    if(c->packets[i].pts < min_pts)
      {
      min_idx = i;
      min_pts = c->packets[i].pts;
      }
    }
  
  return_packet(c, min_idx, &pkt);
  gavl_packet_to_video_frame_metadata(&pkt, f);
  
  return 1;
  }

int gavl_packet_pts_cache_get_by_pts(gavl_packet_pts_cache_t *c, gavl_packet_t * pkt,
                                     int64_t pts)
  {
  int i = 0;
  int ret = 0;
  
  while(i < c->num_packets)
    {
    if(c->packets[i].pts < pts)
      remove_packet(c, i);
    else if(c->packets[i].pts == pts)
      {
      ret = 1;
      return_packet(c, i, pkt);
      break;
      }
    else
      i++;
    }
  
  return ret;
  }


void gavl_packet_pts_cache_clear(gavl_packet_pts_cache_t *c)
  {
  c->num_packets = 0;
  }

