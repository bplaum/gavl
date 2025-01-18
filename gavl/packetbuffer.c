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
#include <stdio.h>
// #include <pthread.h>
#include <config.h>

#include <gavl/connectors.h>
#include <gavl/metatags.h>
#include <gavl/utils.h>

#include <gavl/log.h>
#define LOG_DOMAIN "packetbuffer"

#define FLAG_FLUSH                (1<<0)
#define FLAG_GOT_CI               (1<<1)
#define FLAG_NON_CONTINUOUS       (1<<2)
#define FLAG_MARK_LAST            (1<<3)
#define FLAG_CALC_FRAME_DURATIONS (1<<4)

// #define DUMP_IN_PACKETS
// #define DUMP_OUT_PACKETS

// #define DUMP_PACKET_MASK (GAVL_STREAM_AUDIO)
// #define DUMP_PACKET_MASK (GAVL_STREAM_VIDEO)
// #define DUMP_PACKET_MASK (GAVL_STREAM_TEXT)
// #define DUMP_PACKET_MASK (GAVL_STREAM_VIDEO|GAVL_STREAM_AUDIO)

// #define COUNT_PACKETS


// #define MAX_PACKETS 200

typedef struct
  {
  gavl_packet_t ** packets;
  int num;
  int alloc;
  } buf_t;

/* Get first packet (fifo) */
static gavl_packet_t * buf_shift(buf_t * buf)
  {
  gavl_packet_t * ret = NULL;
  if(!buf->num)
    return NULL;
  ret = buf->packets[0];
  buf->num--;

  if(buf->num)
    {
    memmove(buf->packets, buf->packets+1, buf->num * sizeof(*buf->packets));
    buf->packets[buf->num] = NULL;
    }
  else
    buf->packets[0] = NULL;

  return ret;
  }

static void buf_push(buf_t * buf, gavl_packet_t ** p)
  {
  if(buf->num == buf->alloc)
    {
    buf->alloc += 32;
    buf->packets = realloc(buf->packets, buf->alloc * sizeof(*buf->packets));
    memset(buf->packets + buf->num, 0, (buf->alloc - buf->num) * sizeof(*buf->packets) );
    }
  buf->packets[buf->num] = *p;
  *p = NULL;
  buf->num++;
  }

/* Get last packet (pool), return NULL if the pool is empty */
static gavl_packet_t * buf_pop(buf_t * buf)
  {
  gavl_packet_t * ret;
  
  if(!buf->num)
    return NULL;
  
  buf->num--;
  ret = buf->packets[buf->num];
  buf->packets[buf->num] = NULL;
  return ret;
  }

static void buf_free(buf_t * buf)
  {
  int i;
  for(i = 0; i < buf->num; i++)
    gavl_packet_destroy(buf->packets[i]);
  if(buf->packets)
    free(buf->packets);
  }

struct gavl_packet_buffer_s
  {
  gavl_packet_sink_t * sink;
  gavl_packet_source_t * src;
  
  /* Valid packets */

  buf_t buf;  // Packet queue
  buf_t pool;  // Unused packets
  
  /* Kept here to prevent overwriting */
  gavl_packet_t * out_packet;
  gavl_packet_t * in_packet;
  
  int64_t last_duration;
  int64_t pts;

  int64_t max_pts;
  
  const gavl_dictionary_t * stream;
  gavl_compression_info_t ci;
  int packet_scale;
  int sample_scale;
  
  gavl_stream_type_t type;
  int continuous;
  
  int flags;

  int ip_frames_seen;
  int keyframes_seen;
  
  int duration_divisor;
  
#ifdef COUNT_PACKETS
  int in_count;
  int out_count;
#endif
  };


#if 1
static void dump_quit(gavl_packet_buffer_t * buf)
  {
  int i;
  for(i = 0; i < buf->buf.num; i++)
    {
    fprintf(stderr, "packet %d ", i);

    if(buf->buf.packets[i])
      gavl_packet_dump(buf->buf.packets[i]);
    else
      fprintf(stderr, "NULL\n");
    
    }
  abort();
  }
#endif

/* Sink functions */

static gavl_packet_t * sink_get_func(void * priv)
  {
  gavl_packet_buffer_t * buf = priv;

  if(buf->in_packet)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Called gavl_packet_sink_get_packet twice for %s buffer",
             gavl_stream_type_name(buf->type));
    dump_quit(buf);
    return NULL;
    }
  
  if(!(buf->in_packet = buf_pop(&buf->pool)))
    buf->in_packet = gavl_packet_create();
  
  return buf->in_packet;
  }

/* The following functions are, where pretty much of the A/V synchronization in the whole
   gmerlin architecture is done */

static void pts_from_duration(gavl_packet_buffer_t * buf, gavl_packet_t * p)
  {
  /* Initialize first timestamp */
  if(buf->pts == GAVL_TIME_UNDEFINED)
    {
    if(p->pes_pts != GAVL_TIME_UNDEFINED)
      buf->pts = gavl_time_rescale(buf->packet_scale, buf->sample_scale,
                                   p->pes_pts);
    else
      buf->pts = 0;

    // fprintf(stderr, "Buffer resync: %"PRId64"\n", gavl_time_unscale(buf->sample_scale, buf->pts));
    }
  
  p->pts = buf->pts;
  buf->pts += p->duration;
  }

static void duration_from_pts(gavl_packet_buffer_t * buf, gavl_packet_t * p, gavl_packet_t * next)
  {
  if(next)
    {
    p->duration = next->pts - p->pts;
    buf->last_duration = p->duration;
    }
  else
    p->duration = buf->last_duration;
  }

static void update_timestamps_low_delay(gavl_packet_buffer_t * buf)
  {
  int i;

  /* PTS from DTS */
  for(i = buf->buf.num - 1; i >= 0; i--)
    {
    if((buf->buf.packets[i]->pts == GAVL_TIME_UNDEFINED) &&
       (buf->buf.packets[i]->dts != GAVL_TIME_UNDEFINED))
      buf->buf.packets[i]->pts = buf->buf.packets[i]->dts;
    else
      break;
    }
  
  /* Duration from PTS */
  if(buf->flags & FLAG_CALC_FRAME_DURATIONS)
    {
    for(i = buf->buf.num - 2; i >= 0; i--)
      {
      if((buf->buf.packets[i]->duration < 0) &&
         (buf->buf.packets[i]->pts != GAVL_TIME_UNDEFINED) &
         (buf->buf.packets[i+1]->pts != GAVL_TIME_UNDEFINED))
        duration_from_pts(buf, buf->buf.packets[i], buf->buf.packets[i+1]);
      else
        break;
      }

    if(buf->flags & FLAG_FLUSH)
      duration_from_pts(buf, buf->buf.packets[buf->buf.num-1], NULL);
    }
  
  /* Duration from PES PTS */
  if((buf->duration_divisor > 0) &&
     (buf->buf.packets[buf->buf.num-1]->pts == GAVL_TIME_UNDEFINED) &&
     (buf->buf.packets[buf->buf.num-1]->duration < 0) &&
     (buf->buf.packets[buf->buf.num-1]->pes_pts != GAVL_TIME_UNDEFINED))
    {
    for(i = 0; i < buf->buf.num-1; i++)
      {
      int frames_per_packet;
      int approx_samples;
      
      if(buf->buf.packets[i]->duration > 0)
        continue;

      approx_samples = gavl_time_rescale(buf->packet_scale,
                                         buf->sample_scale,
                                         buf->buf.packets[i+1]->pes_pts -
                                         buf->buf.packets[i]->pes_pts);
      frames_per_packet = (approx_samples + buf->duration_divisor/2) / buf->duration_divisor;
      buf->buf.packets[i]->duration = frames_per_packet * buf->duration_divisor;
      buf->last_duration = buf->buf.packets[i]->duration;

      if(buf->buf.packets[i]->pts == GAVL_TIME_UNDEFINED)
        pts_from_duration(buf, buf->buf.packets[i]);
      }
    if(buf->flags & FLAG_FLUSH)
      {
      buf->buf.packets[buf->buf.num-1]->duration = buf->last_duration;
      if(buf->buf.packets[buf->buf.num-1]->pts == GAVL_TIME_UNDEFINED)
        pts_from_duration(buf, buf->buf.packets[buf->buf.num-1]);
      }
    }
  
  /* PTS from duration */

  if((buf->buf.packets[buf->buf.num-1]->pts == GAVL_TIME_UNDEFINED) &&
     (buf->buf.packets[buf->buf.num-1]->duration >= 0))
    {
    for(i = 0; i < buf->buf.num; i++)
      {
      if(buf->buf.packets[i]->pts != GAVL_TIME_UNDEFINED)
        continue;

      pts_from_duration(buf, buf->buf.packets[i]);
      }
    }
  }

static int get_next_ip_idx(gavl_packet_buffer_t * buf, int idx)
  {
  int i;
  for(i = idx; i < buf->buf.num; i++)
    {
    if((buf->buf.packets[i]->flags & GAVL_PACKET_TYPE_MASK) != GAVL_PACKET_TYPE_B)
      return i;
    }
  return -1;
  }

static void pts_from_duration_b_frames(gavl_packet_buffer_t * buf)
  {
  int i;
  int ip1 = -1;
  int ip2 = -1;
  
  if((ip1 = get_next_ip_idx(buf, 0)) < 0)
    return;
  
  while(1)
    {
    if((ip2 = get_next_ip_idx(buf, ip1 + 1)) < 0)
      {
      if(buf->flags & FLAG_FLUSH)
        ip2 = buf->buf.num;
      else
        break;
      }

    if(buf->buf.packets[ip1]->pts == GAVL_TIME_UNDEFINED)
      {
      /*
       * PBBP -> P
       * or
       * IPBB -> PBB
       */
    
      for(i = ip1+1; i < ip2; i++)
        pts_from_duration(buf, buf->buf.packets[i]);

      pts_from_duration(buf, buf->buf.packets[ip1]);
      }

    if(ip2 == buf->buf.num)
      break;
    
    ip1 = ip2;
    }
  
  }

/* TODO: Get duration from PTS for a stream with B-frames */

static int get_next_by_pts(gavl_packet_buffer_t * buf, int64_t pts, int start, int end)
  {
  int ret = -1;
  int i;
  if(end < 0)
    end = buf->buf.num;

  for(i = start; i < end; i++)
    {
    if(buf->buf.packets[i]->pts <= pts)
      continue;

    if((ret == -1) || (buf->buf.packets[i]->pts < buf->buf.packets[ret]->pts))
      ret = i;
    }
  return ret;
  }

static void duration_from_pts_b_frames(gavl_packet_buffer_t * buf)   
  {
  int ip1 = -1;
  int ip2 = -1;
  int ip3 = -1;
  int next_idx;
  int i;
  
  /* Get remaining durations with a brute force method */
  if(buf->flags & FLAG_FLUSH)
    {
    int last_idx = 0;
    int64_t duration = 0;
    
    for(i = 0; i < buf->buf.num; i++)
      {
      if(buf->buf.packets[i]->duration > 0)
        continue;

      next_idx = get_next_by_pts(buf, buf->buf.packets[i]->pts, 0, -1);
      if(next_idx < 0)
        last_idx = i;
      else
        duration = buf->buf.packets[next_idx]->pts - buf->buf.packets[i]->pts;
      }
    buf->buf.packets[last_idx]->duration = duration;
    return;
    }
  
  /* Get the first non B-frame with no duration */
  ip1 = get_next_ip_idx(buf, 0);
  if(ip1 < 0)
    return; // Shouldn't happen actually
  
  while(buf->buf.packets[ip1]->duration > 0)
    {
    ip1 = get_next_ip_idx(buf, ip1+1);

    if(ip1 < 0)
      return; // Nothing to do
    }

  if((ip2 = get_next_ip_idx(buf, ip1 + 1)) < 0)
    return; // Nothing to do

  if((ip3 = get_next_ip_idx(buf, ip2 + 1)) < 0)
    return; // Nothing to do

  
  for(i = ip1; i < ip2; i++)
    {
    if(buf->buf.packets[i]->duration > 0)
      continue;
    
    next_idx = get_next_by_pts(buf, buf->buf.packets[i]->pts, ip1+1, ip3);
    
    if(next_idx < 0)
      fprintf(stderr, "Buuuug\n");
    else
      buf->buf.packets[i]->duration = buf->buf.packets[next_idx]->pts - buf->buf.packets[i]->pts;
    }
  
  }

static void update_timestamps_b_frames(gavl_packet_buffer_t * buf)
  {
  int i;
  
  /* Duration from dts */
  if((buf->buf.packets[buf->buf.num-1]->pts == GAVL_TIME_UNDEFINED) &&
     (buf->buf.packets[buf->buf.num-1]->dts != GAVL_TIME_UNDEFINED) &&
     (buf->buf.packets[buf->buf.num-1]->duration <= 0))
    {
    for(i = buf->buf.num - 2; i >= 0; i--)
      {
      if((buf->buf.packets[i]->duration <= 0) &&
         (buf->buf.packets[i]->dts != GAVL_TIME_UNDEFINED) &
         (buf->buf.packets[i+1]->dts != GAVL_TIME_UNDEFINED))
        buf->buf.packets[i]->duration = buf->buf.packets[i+1]->dts - buf->buf.packets[i]->dts;
      else
        break;
      }
    }

  /* PTS from frame type and duration */
  if((buf->buf.num >= 2) &&
     (buf->buf.packets[buf->buf.num-1]->pts == GAVL_TIME_UNDEFINED) &&
     (buf->buf.packets[buf->buf.num-2]->duration > 0))
    {
    pts_from_duration_b_frames(buf);
    }
  
  /* Duration from PTS */
  
  if((buf->flags & FLAG_CALC_FRAME_DURATIONS) &&
     (buf->buf.packets[buf->buf.num-1]->duration < 0))
    {
    duration_from_pts_b_frames(buf);
    }
  
  }

static void update_timestamps(gavl_packet_buffer_t * buf)
  {
  if(buf->buf.num < 1)
    return;

  /* Check if this is necessary at all */
  if((buf->buf.packets[buf->buf.num-1]->pts != GAVL_TIME_UNDEFINED) &&
     (!(buf->flags & FLAG_CALC_FRAME_DURATIONS) ||
      (buf->buf.packets[buf->buf.num-1]->duration > 0)))
    return;
  
  /* Duration from DTS */
  if((buf->buf.num > 1) &&
     (buf->buf.packets[buf->buf.num-2]->duration <= 0) &&
     (buf->buf.packets[buf->buf.num-2]->dts != GAVL_TIME_UNDEFINED) &&
     (buf->buf.packets[buf->buf.num-1]->dts != GAVL_TIME_UNDEFINED))
    {
    buf->buf.packets[buf->buf.num-2]->duration = 
      buf->buf.packets[buf->buf.num-1]->dts -
      buf->buf.packets[buf->buf.num-2]->dts;

    if(buf->flags & FLAG_FLUSH)
      buf->buf.packets[buf->buf.num-1]->duration =
        buf->buf.packets[buf->buf.num-2]->duration;
    
    }

  
  if(buf->ci.flags & GAVL_COMPRESSION_HAS_B_FRAMES)
    update_timestamps_b_frames(buf);
  else
    update_timestamps_low_delay(buf);
  }


static gavl_sink_status_t sink_put_func(void * priv, gavl_packet_t * p)
  {
  gavl_packet_buffer_t * buf = priv;

  if(!p)
    {
    buf_push(&buf->pool, &buf->in_packet);
    return GAVL_SINK_OK;
    }
  
  if(!p->buf.len)
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Packet has zero length");

  if(buf->in_packet != p)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Something went wrong: in_packet != p");
    return GAVL_SINK_OK; // Skip
    }

  /* Discard skip packet */
  if(p->flags & GAVL_PACKET_SKIP)
    {
    buf_push(&buf->pool, &buf->in_packet);
    return GAVL_SINK_OK; // Skip
    }
  
#ifdef MAX_PACKETS
  if(buf->buf.num >= MAX_PACKETS)
    {
    fprintf(stderr, "Exceeded max packets: %d\n", buf->buf.num);
    dump_quit(buf);
    }
#endif
  
  /* Get compression info */
  if(!(buf->flags & FLAG_GOT_CI))
    {
    const gavl_dictionary_t * m = gavl_stream_get_metadata(buf->stream);

    //    fprintf(stderr, "Getting compression info:");
    //    gavl_dictionary_dump(buf->stream, 2);
    
    gavl_dictionary_get_int(m, GAVL_META_STREAM_PACKET_TIMESCALE, &buf->packet_scale);
    gavl_dictionary_get_int(m, GAVL_META_STREAM_SAMPLE_TIMESCALE, &buf->sample_scale);

    if(gavl_dictionary_get_int(m, GAVL_META_STREAM_PACKET_DURATION_DIVISOR, &buf->duration_divisor))
      gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Got duration divisor %d", buf->duration_divisor);
    else
      buf->duration_divisor = 0;
    
    gavl_stream_get_compression_info(buf->stream, &buf->ci);

    if(!gavl_stream_is_continuous(buf->stream))
      buf->flags |= FLAG_NON_CONTINUOUS;
    
    buf->flags |= FLAG_GOT_CI;
    }
  
  /* Skip packets before first keyframe */
  if(buf->ci.flags & GAVL_COMPRESSION_HAS_P_FRAMES)
    {
    if(p->flags & GAVL_PACKET_KEYFRAME)
      buf->keyframes_seen++;
    else if(!buf->keyframes_seen) // Non keyframe
      {
#if 1
      /* Else: Skip, but allow synchronizing a low delay stream */
      /* This can be the case for e.g. for Ogg theora files, when a keyframe is
         not at the start of a page */
      
      if(!(buf->ci.flags & GAVL_COMPRESSION_HAS_B_FRAMES))
        {
        if((buf->pts == GAVL_TIME_UNDEFINED) &&
           (p->pes_pts != GAVL_TIME_UNDEFINED))
          {
          buf->pts = gavl_time_rescale(buf->packet_scale, buf->sample_scale,
                                       p->pes_pts);
          }
          
        if((buf->pts != GAVL_TIME_UNDEFINED) && (p->duration > 0))
          buf->pts += p->duration;
        }
#endif
      buf_push(&buf->pool, &buf->in_packet);
      return GAVL_SINK_OK;
      }
    }
  
  /* Merge field pictures */
  if((p->flags & GAVL_PACKET_FIELD_PIC) &&
     (buf->buf.num >= 1) &&
     (buf->buf.packets[buf->buf.num-1]->flags & GAVL_PACKET_FIELD_PIC))
    {
    gavl_packet_merge_field2(buf->buf.packets[buf->buf.num-1], p);
    buf_push(&buf->pool, &buf->in_packet);
    //    fprintf(stderr, "Merged field pic\n");
    return GAVL_SINK_OK;
    }
  
  /* Detect frame type from PTS */
  if((buf->ci.flags & GAVL_COMPRESSION_HAS_P_FRAMES) &&
     !(p->flags & GAVL_PACKET_TYPE_MASK) &&
     (p->pts != GAVL_TIME_UNDEFINED))
    {
    if(p->flags & GAVL_PACKET_KEYFRAME)
      {
      p->flags |= GAVL_PACKET_TYPE_I;
      buf->max_pts = p->pts;
      }
    else
      {
      if((buf->max_pts == GAVL_TIME_UNDEFINED) ||
         (buf->max_pts < p->pts))
        {
        buf->max_pts = p->pts;
        p->flags |= GAVL_PACKET_TYPE_P;
        }
      else
        p->flags |= GAVL_PACKET_TYPE_B;
      }
    }
  
  /* Skip undecodeable B-Frames from open GOPs */
  if(buf->ci.flags & GAVL_COMPRESSION_HAS_B_FRAMES)
    {
    if((p->flags & GAVL_PACKET_TYPE_MASK) == GAVL_PACKET_TYPE_B)
      {
      /* Else: Skip */
      if(buf->ip_frames_seen < 2)
        {
        buf_push(&buf->pool, &buf->in_packet);
        return GAVL_SINK_OK;
        }
      }
    else /* I/P frame */
      buf->ip_frames_seen++;
    }
  
  /* Append packet to buffer */
  
#ifdef DUMP_IN_PACKETS
  if(buf->type & DUMP_PACKET_MASK)
    {
    gavl_dprintf("Buf in %p ", p);
    gavl_packet_dump(p);

    }
#endif

#ifdef COUNT_PACKETS
  buf->in_count++;
#endif

  if(!buf->in_packet->buf.len)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Added zero size packet");
    gavl_packet_dump(p);
    }
    
  buf_push(&buf->buf, &buf->in_packet);
  //  fprintf(stderr, "put_func: %p %d\n", buf, buf->buf.num);
  
  if((p->pts != GAVL_TIME_UNDEFINED) &&
     (!(buf->flags & FLAG_CALC_FRAME_DURATIONS) ||
      (p->duration >= 0)))
    return GAVL_SINK_OK;
  
  //  else // No P-frames
  //    buf->valid_packets++;

  update_timestamps(buf);
  return GAVL_SINK_OK;
  }


static gavl_source_status_t
source_func(void * priv, gavl_packet_t ** p)
  {
  gavl_packet_buffer_t * buf = priv;

  //  fprintf(stderr, "source_func: %p %d\n", buf, buf->buf.num);
  
  if(buf->out_packet)
    {
    buf_push(&buf->pool, &buf->out_packet);
    }
  
  if(!buf->buf.num)
    {
    if(buf->flags & FLAG_FLUSH)
      return GAVL_SOURCE_EOF;
    else
      return GAVL_SOURCE_AGAIN;
    }

  /*
   *  If we have to mark the last packet, we need at least 2 packets in the buffer
   *  if we are not flushing
   */
  if((buf->buf.num == 1) &&
     ((buf->flags & (FLAG_MARK_LAST|FLAG_FLUSH)) == FLAG_MARK_LAST))
    return GAVL_SOURCE_AGAIN;

  /*
   *  Field pictures need to be merged
   */
  
  if(buf->buf.packets[0]->flags & GAVL_PACKET_FIELD_PIC)
    return GAVL_SOURCE_AGAIN; // Wait for next field
  
  if(!(buf->flags & FLAG_NON_CONTINUOUS)) // If stream is continuous...
    {
    if((buf->buf.packets[0]->pts == GAVL_TIME_UNDEFINED) ||
       ((buf->buf.packets[0]->duration < 0) && (buf->flags & FLAG_CALC_FRAME_DURATIONS)))
      return GAVL_SOURCE_AGAIN;
    }
  
  buf->out_packet = buf_shift(&buf->buf);
  
#ifdef DUMP_OUT_PACKETS
  if(buf->type & DUMP_PACKET_MASK)
    {
    if(!buf->out_packet)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Outputting zero packet");
      dump_quit(buf); 
      }
    else
      {
      gavl_dprintf("Buf out %p ", buf->out_packet);
      gavl_packet_dump(buf->out_packet);
      }
    }
#endif

  *p = buf->out_packet;
  
#ifdef COUNT_PACKETS
  buf->out_count++;
#endif
    
  return GAVL_SOURCE_OK;
  }


void gavl_packet_buffer_flush(gavl_packet_buffer_t * buf)
  {
  //  fprintf(stderr, "gavl_packet_buffer_flush\n");
  
  buf->flags |= FLAG_FLUSH;
  
  if(buf->buf.num > 0)
    {
    if(buf->flags & FLAG_MARK_LAST)
      buf->buf.packets[buf->buf.num-1]->flags |= GAVL_PACKET_LAST;
    }
  update_timestamps(buf);
  }

void gavl_packet_buffer_clear(gavl_packet_buffer_t * buf)
  {
  gavl_packet_t * p;
  
  while((p = buf_pop(&buf->buf)))
    buf_push(&buf->pool, &p);
  
  if(buf->in_packet)
    buf_push(&buf->pool, &buf->in_packet);
  
  if(buf->out_packet)
    buf_push(&buf->pool, &buf->out_packet);
  
  buf->flags &= ~FLAG_FLUSH;
  buf->last_duration = -1;
  buf->pts = GAVL_TIME_UNDEFINED;
  buf->max_pts = GAVL_TIME_UNDEFINED;
  buf->ip_frames_seen = 0;
  buf->keyframes_seen = 0;
  gavl_packet_source_reset(buf->src);
  gavl_packet_sink_reset(buf->sink);
  //  fprintf(stderr, "gavl_packet_buffer_clear %d %d\n", buf->packet_scale, buf->sample_scale);
  }

void gavl_packet_buffer_set_out_pts(gavl_packet_buffer_t * buf, int64_t pts)
  {
  buf->pts = pts;
  }

gavl_packet_buffer_t *
gavl_packet_buffer_create(const gavl_dictionary_t * stream_info)
  {
  int val_i = 0;
  gavl_packet_buffer_t * ret;
  
  ret = calloc(1, sizeof(*ret));
  
  ret->stream = stream_info;
  ret->src = gavl_packet_source_create(source_func, ret, GAVL_SOURCE_SRC_ALLOC, ret->stream);
  ret->sink = gavl_packet_sink_create(sink_get_func, sink_put_func, ret);
  gavl_dictionary_get_int(ret->stream, GAVL_META_STREAM_TYPE, &val_i);
  ret->type = val_i;
  ret->last_duration = -1;
  ret->pts = GAVL_TIME_UNDEFINED;
  
  
  return ret;
  }

void gavl_packet_buffer_destroy(gavl_packet_buffer_t * b)
  {
#ifdef COUNT_PACKETS
  gavl_dprintf("packets in:  %d\npackets out: %d\n",
               b->in_count, b->out_count);

  if(b->out_count < b->in_count)
    {
    int i;
    gavl_dprintf("Packets in buffer: %d\n",
                 b->valid_packets);
    
    for(i = 0; i < b->valid_packets; i++)
      gavl_packet_dump(b->packets[i]);
    }
  
#endif
  
  if(b->src)
    gavl_packet_source_destroy(b->src);
  if(b->sink)
    gavl_packet_sink_destroy(b->sink);

  if(b->in_packet)
    gavl_packet_destroy(b->in_packet);
  if(b->out_packet)
    gavl_packet_destroy(b->out_packet);

  buf_free(&b->buf);
  buf_free(&b->pool);
  
  gavl_compression_info_free(&b->ci);
  
  }

gavl_packet_sink_t * gavl_packet_buffer_get_sink(gavl_packet_buffer_t * b)
  {
  return b->sink;
  }

gavl_packet_source_t * gavl_packet_buffer_get_source(gavl_packet_buffer_t * b)
  {
  return b->src;
  }

void gavl_packet_buffer_set_mark_last(gavl_packet_buffer_t * buf, int mark)
  {
  if(mark)
    buf->flags |= FLAG_MARK_LAST;
  else
    buf->flags &= ~FLAG_MARK_LAST;
  }

void gavl_packet_buffer_set_calc_frame_durations(gavl_packet_buffer_t * buf, int calc)
  {
  if(calc)
    buf->flags |= FLAG_CALC_FRAME_DURATIONS;
  else
    buf->flags &= ~FLAG_CALC_FRAME_DURATIONS;
  }
