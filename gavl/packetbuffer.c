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
#include <string.h>
#include <stdio.h>
// #include <pthread.h>
#include <config.h>

#include <gavl/connectors.h>
#include <gavl/metatags.h>

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
#define DUMP_PACKET_MASK (GAVL_STREAM_VIDEO|GAVL_STREAM_AUDIO)

// #define COUNT_PACKETS

struct gavl_packet_buffer_s
  {
  gavl_packet_sink_t * sink;
  gavl_packet_source_t * src;

  /* Unused packets */
  //  gavl_packet_t ** pool;
  //  int pool_alloc;
  //  int pool_size;

  /* Valid packets */
  gavl_packet_t ** packets;
  int packets_alloc;
  int valid_packets;
  int num_packets; // Total packets (used and unused)
  
  /* Kept here to prevent overwriting */
  gavl_packet_t * out_packet;
  
  
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
  
#ifdef COUNT_PACKETS
  int in_count;
  int out_count;
#endif
  };

static void do_realloc(gavl_packet_buffer_t * buf)
  {
  buf->packets_alloc += 32;
  buf->packets = realloc(buf->packets, buf->packets_alloc * sizeof(*buf->packets));
  memset(buf->packets + buf->num_packets, 0,
         (buf->packets_alloc - buf->num_packets) * sizeof(*buf->packets));
  }

static void recycle_out_packet(gavl_packet_buffer_t * buf)
  {
  if(!buf->out_packet)
    return;

  if(buf->packets_alloc == buf->num_packets)
    do_realloc(buf);
  
  buf->packets[buf->num_packets] = buf->out_packet;
  buf->num_packets++;
  buf->out_packet = NULL;
  }

/* Sink functions */

static gavl_packet_t * sink_get_func(void * priv)
  {
  gavl_packet_buffer_t * buf = priv;

  if(buf->packets_alloc == buf->valid_packets)
    do_realloc(buf);
  
  if(buf->valid_packets == buf->num_packets)
    {
    buf->packets[buf->num_packets] = gavl_packet_create();
    buf->num_packets++;
    }
  
  gavl_packet_reset(buf->packets[buf->valid_packets]);
  return buf->packets[buf->valid_packets];
  }

/*
 *  Timestamp generation modes:
 *
 *  1. PTS from frame duration (needs inititial pts from PES packet)
 *  2. PTS from DTS: Get durations from DTS and synchronize first frame from DTS
 *  3. Duration from PTS (like in Matroska)
 */

static void update_timestamps_write(gavl_packet_buffer_t * buf)
  {
  gavl_packet_t * last_p, * last2_p;

  if(buf->valid_packets < 1)
    return;

  if((buf->packets[buf->valid_packets-1]->pts != GAVL_TIME_UNDEFINED) &&
     (buf->packets[buf->valid_packets-1]->duration > 0))
    return;

  if((buf->valid_packets >= 2) &&
     (buf->packets[buf->valid_packets-2]->duration < 0))
    {
    last_p = buf->packets[buf->valid_packets-1];
    last2_p = buf->packets[buf->valid_packets-2];
    
    /* Set duration from DTS */
    if((last_p->dts != GAVL_TIME_UNDEFINED) &&
       (last2_p->dts != GAVL_TIME_UNDEFINED))
      {
      last2_p->duration = last_p->dts - last2_p->dts;
      buf->last_duration = last2_p->duration;
      }

    /* Set duration from PTS (low delay case) */
    if(!(buf->ci.flags & GAVL_COMPRESSION_HAS_B_FRAMES) &&
       (last_p->pts != GAVL_TIME_UNDEFINED) &&
       (last2_p->pts != GAVL_TIME_UNDEFINED) &&
       (buf->flags & FLAG_CALC_FRAME_DURATIONS))
      {
      last2_p->duration = last_p->pts - last2_p->pts;
      buf->last_duration = last2_p->duration;
      }
    }
  }

/* Get next IP index *after* idx or -1 */

static int get_next_ip_index(gavl_packet_buffer_t * buf, int idx)
  {
  int ret = idx+1;

  while(ret < buf->valid_packets)
    {
    if((buf->packets[ret]->flags & GAVL_PACKET_TYPE_MASK) != GAVL_PACKET_TYPE_B)
      return ret;
    ret++;
    }
  
  return -1;
  }

static gavl_packet_t * find_next(gavl_packet_buffer_t * buf,
                                 int start_idx,
                                 int end_idx,
                                 int64_t pts)
  {
  int i;
  gavl_packet_t * p = NULL;
  
  for(i = start_idx; i < end_idx; i++)
    {
    if((pts != GAVL_TIME_UNDEFINED) && buf->packets[i]->pts <= pts)
      continue;
    
    if(!p || (p->pts > buf->packets[i]->pts))
      p = buf->packets[i];
    }
  return p;
  }
                                 
static void update_timestamps_read(gavl_packet_buffer_t * buf)
  {
  if((buf->packets[0]->pts != GAVL_TIME_UNDEFINED) &&
     (!(buf->flags & FLAG_CALC_FRAME_DURATIONS) || (buf->packets[0]->duration > 0)))
    return;
  
  /* Set pts from duration */
  if(buf->packets[0]->pts == GAVL_TIME_UNDEFINED)
    {
    if(buf->packets[0]->duration >= 0) // Need to allow 0 here (e.g. for the first Vorbis packet)
      {
      /* Get start PTS */
      if(buf->pts == GAVL_TIME_UNDEFINED)
        {
        /* 1st choice: pes packet */
        if(buf->packets[0]->pes_pts != GAVL_TIME_UNDEFINED)
          buf->pts = gavl_time_rescale(buf->packet_scale, buf->sample_scale,
                                       buf->packets[0]->pes_pts);
        /* 2nd choice: DTS */
        else if(buf->packets[0]->dts != GAVL_TIME_UNDEFINED)
          buf->pts = buf->packets[0]->dts;
        else
          /* 3rd choice: Zero */
          {
          buf->pts = 0;
          if(buf->ci.pre_skip)
            buf->pts -= buf->ci.pre_skip;
          }
        }
    
      /* Low delay */
      if(!(buf->ci.flags & GAVL_COMPRESSION_HAS_B_FRAMES))
        {
        buf->packets[0]->pts = buf->pts;
        buf->pts += buf->packets[0]->duration;
        }
      else
        {
        int ip_idx2;

        if(buf->valid_packets < 2)
          return;
      
        if((buf->packets[0]->flags & GAVL_PACKET_TYPE_MASK) == GAVL_PACKET_TYPE_B)
          {
          /* Bug!! */
          fprintf(stderr, "BUUUG!!\n!");
          return;
          }

        ip_idx2 = get_next_ip_index(buf, 0);
        //      fprintf(stderr, "IP Index: %d\n", ip_idx2);
        if(ip_idx2 < 0)
          {
          if(buf->flags & FLAG_FLUSH)
            {
            int i;
            /* Flush PBB <EOS> */
            for(i = 1; i < buf->valid_packets; i++)
              {
              buf->packets[i]->pts = buf->pts;
              buf->pts += buf->packets[i]->duration;
              }
            buf->packets[0]->pts = buf->pts;
            buf->pts += buf->packets[0]->duration;
            }
          return;
          }
      
        if(ip_idx2 == 1)
          {
          /* Two consecutive non-B-frames: Flush first one */
          buf->packets[0]->pts = buf->pts;
          buf->pts += buf->packets[0]->duration;
          }
        else
          {
          int i;
          /* Flush sequence PBB(P) */
          for(i = 1; i < ip_idx2; i++)
            {
            buf->packets[i]->pts = buf->pts;
            buf->pts += buf->packets[i]->duration;
            }
          buf->packets[0]->pts = buf->pts;
          buf->pts += buf->packets[0]->duration;
          }
        }
      }
    else // duration < 0
      {
      if(buf->packets[0]->pes_pts >= 0)
        {
        buf->packets[0]->pts = gavl_time_rescale(buf->packet_scale, buf->sample_scale,
                                                 buf->packets[0]->pes_pts);
        }
      }
    }

  
  /* Set duration from PTS (B-frame case) */
  if((buf->packets[0]->pts != GAVL_TIME_UNDEFINED) &&
     (buf->packets[0]->duration < 0) &&
     (buf->flags & FLAG_CALC_FRAME_DURATIONS))
    {
    int ip_idx2;
    int ip_idx3;

    if((buf->packets[0]->flags & GAVL_PACKET_TYPE_MASK) == GAVL_PACKET_TYPE_B)
      {
      /* Bug!! */
      fprintf(stderr, "BUUUG!!\n!");
      return;
      }

    if(buf->flags & FLAG_FLUSH)
      {
      int i;
      gavl_packet_t * p1;
      gavl_packet_t * p2;
      int64_t last_duration = GAVL_TIME_UNDEFINED;

      p1 = find_next(buf, 0, buf->valid_packets,
                     GAVL_TIME_UNDEFINED);
      if(!p1)
        return;
      
      for(i = 0; i < buf->valid_packets; i++)
        {
        p2 = find_next(buf, 0, buf->valid_packets,
                       p1->pts);
        if(!p2)
          {
          p1->duration = last_duration;
          }
        }
      }
    
    if((ip_idx2 = get_next_ip_index(buf, 0)) < 0)
      return;
    
    else if((ip_idx3 = get_next_ip_index(buf, ip_idx2)) < 0)
      {
      if(buf->flags & FLAG_FLUSH)
        {
        // flush PBBPBB
        
        }
      return;
      }
    else
      {
      //      int i, num;

      /* Flush */
      // PBBPBBP -> PBBP
      //      for
      }
    }
  }

static gavl_sink_status_t sink_put_func(void * priv, gavl_packet_t * p)
  {
  int ok = 0;
  gavl_packet_buffer_t * buf = priv;

  if(p->flags & GAVL_PACKET_SKIP)
    return GAVL_SINK_OK; // Skip
  
  /* Get compression info */
  if(!(buf->flags & FLAG_GOT_CI))
    {
    const gavl_dictionary_t * m = gavl_stream_get_metadata(buf->stream);
    gavl_dictionary_get_int(m, GAVL_META_STREAM_PACKET_TIMESCALE, &buf->packet_scale);
    gavl_dictionary_get_int(m, GAVL_META_STREAM_SAMPLE_TIMESCALE, &buf->sample_scale);
    
    gavl_stream_get_compression_info(buf->stream, &buf->ci);

    if(!gavl_stream_is_continuous(buf->stream))
      buf->flags |= FLAG_NON_CONTINUOUS;
    
    //    buf->ts_mode = gavl_stream_get_ts_mode(buf->stream);
    buf->flags |= FLAG_GOT_CI;
    }
  
  /* Skip packets before first keyframe */
  if(buf->ci.flags & GAVL_COMPRESSION_HAS_P_FRAMES)
    {
    if(p->flags & GAVL_PACKET_KEYFRAME)
      {
      buf->keyframes_seen++;
      ok = 1;
      }
    else // Non keyframe
      {
      if(buf->keyframes_seen > 0)
        ok = 1;
      else
        {
        /* Else: Skip, but allow synchronizing a low delay stream */
        if(!(buf->ci.flags & GAVL_COMPRESSION_HAS_B_FRAMES))
          {
          if((buf->pts == GAVL_TIME_UNDEFINED) &&
             (p->pes_pts != GAVL_TIME_UNDEFINED))
            {
            buf->pts = gavl_time_rescale(buf->packet_scale, buf->sample_scale,
                                         p->pes_pts);
            }
          
          if((buf->pts != GAVL_TIME_UNDEFINED) &&
                  (p->duration > 0))
            buf->pts += p->duration;
          }
        
        return GAVL_SINK_OK;
        }
      }
    }
  else
    ok = 1;
  
  /* Merge field pictures */
  if(p->flags & GAVL_PACKET_FIELD_PIC)
    {
    if((buf->valid_packets >= 2) &&
       (buf->packets[buf->valid_packets-2]->flags & GAVL_PACKET_FIELD_PIC))
      {
      gavl_packet_merge_field2(buf->packets[buf->valid_packets-1], p);
      ok = 0;
      p = buf->packets[buf->valid_packets-1];
      }
    else
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
        ok = 0;
        return GAVL_SINK_OK;
        }
      }
    else /* I/P frame */
      buf->ip_frames_seen++;
    }

  if(ok)
    {
    buf->valid_packets++;
#ifdef DUMP_IN_PACKETS
    if(buf->type & DUMP_PACKET_MASK)
      {
      gavl_dprintf("Buf in ");
      gavl_packet_dump(p);
      }
#endif

#ifdef COUNT_PACKETS
    buf->in_count++;
#endif
    }
  
  if((p->pts != GAVL_TIME_UNDEFINED) &&
     (p->duration > 0))
    return GAVL_SINK_OK;
  
  //  else // No P-frames
  //    buf->valid_packets++;

  update_timestamps_write(buf);
  
  return GAVL_SINK_OK;
  }

#if 0
static void get_ip_indices(gavl_packet_buffer_t * buf, int * ret)
  {
  int idx;
  
  ret[0] = -1;
  ret[1] = -1;
  ret[2] = -1;

  idx = 0;
  
  while((idx < buf->valid_packets) &&
        (buf->packets[idx]->flags & GAVL_PACKET_TYPE_MASK) == GAVL_PACKET_TYPE_B)
    {
    idx++;
    }

  if(idx >= buf->valid_packets)
    return;
  
  ret[0] = idx;
  idx++;

  while((idx < buf->valid_packets) &&
        (buf->packets[idx]->flags & GAVL_PACKET_TYPE_MASK) == GAVL_PACKET_TYPE_B)
    idx++;

  if(idx >= buf->valid_packets)
    return;
    
  ret[1] = idx;
  idx++;

  while((idx < buf->valid_packets) &&
        (buf->packets[idx]->flags & GAVL_PACKET_TYPE_MASK) == GAVL_PACKET_TYPE_B)
    {
    idx++;
    }
  
  if(idx >= buf->valid_packets)
    return;

  ret[2] = idx;
  
  }
#endif

static gavl_source_status_t
source_func(void * priv, gavl_packet_t ** p)
  {
  gavl_packet_buffer_t * buf = priv;
  
  if(!buf->valid_packets)
    {
    if(buf->flags & FLAG_FLUSH)
      return GAVL_SOURCE_EOF;
    else
      return GAVL_SOURCE_AGAIN;
    }
  
  if((buf->valid_packets == 1) &&
     ((buf->flags & (FLAG_MARK_LAST|FLAG_FLUSH)) == FLAG_MARK_LAST))
    return GAVL_SOURCE_AGAIN;
  
  if(buf->packets[0]->flags & GAVL_PACKET_FIELD_PIC)
    return GAVL_SOURCE_AGAIN; // Wait for next field
  
  if(!(buf->flags & FLAG_NON_CONTINUOUS)) // If stream is continuous...
    {
    update_timestamps_read(buf);
    if((buf->packets[0]->pts == GAVL_TIME_UNDEFINED) ||
       ((buf->packets[0]->duration < 0) && (buf->flags & FLAG_CALC_FRAME_DURATIONS)))
      return GAVL_SOURCE_AGAIN;
    }

  recycle_out_packet(buf);
  
  buf->out_packet = buf->packets[0];
  *p = buf->out_packet;
  
  buf->valid_packets--;
  buf->num_packets--;
  if(buf->num_packets > 0)
    {
    memmove(buf->packets, buf->packets + 1,
            buf->num_packets*sizeof(*buf->packets));
    }
  buf->packets[buf->num_packets] = NULL;
  
#ifdef DUMP_OUT_PACKETS
  if(buf->type & DUMP_PACKET_MASK)
    {
    gavl_dprintf("Buf out ");
    gavl_packet_dump(*p);
    }
#endif

#ifdef COUNT_PACKETS
  buf->out_count++;
#endif
    
  return GAVL_SOURCE_OK;
  }


void gavl_packet_buffer_flush(gavl_packet_buffer_t * buf)
  {
  //  fprintf(stderr, "gavl_packet_buffer_flush\n");
  
  buf->flags |= FLAG_FLUSH;
  
  if(buf->valid_packets > 0)
    {
    if(buf->packets[buf->valid_packets-1]->duration < 0)
      buf->packets[buf->valid_packets-1]->duration = buf->last_duration;

    if(buf->flags & FLAG_MARK_LAST)
      buf->packets[buf->valid_packets-1]->flags |= GAVL_PACKET_LAST;
    }
  }

void gavl_packet_buffer_clear(gavl_packet_buffer_t * buf)
  {
  buf->valid_packets = 0;
  buf->flags &= ~FLAG_FLUSH;
  buf->last_duration = -1;
  buf->pts = GAVL_TIME_UNDEFINED;
  buf->max_pts = GAVL_TIME_UNDEFINED;
  buf->ip_frames_seen = 0;
  buf->keyframes_seen = 0;
  recycle_out_packet(buf);
  gavl_packet_source_reset(buf->src);
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

  if(b->out_packet)
    gavl_packet_destroy(b->out_packet);
  
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
