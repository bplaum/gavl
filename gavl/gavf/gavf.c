#include <stdlib.h>
#include <string.h>

#include <config.h>

#include <gavfprivate.h>
#include <metatags.h>

#define DUMP_EOF

static struct
  {
  gavl_stream_type_t type;
  const char * name;
  }
stream_types[] =
  {
    { GAVL_STREAM_AUDIO, "audio" },
    { GAVL_STREAM_VIDEO, "video" },
    { GAVL_STREAM_TEXT,  "text"  },
    { GAVL_STREAM_OVERLAY,  "overlay" },
    { GAVL_STREAM_MSG,      "msg"  },
  };

// static int align_read(gavf_io_t * io);
// static int align_write(gavf_io_t * io);

static void gavf_stream_free(gavf_stream_t * s);

static void free_track(gavf_t * g)
  {
  int i;
  
  if(g->streams)
    {
    for(i = 0; i < g->num_streams; i++)
      gavf_stream_free(&g->streams[i]);
    free(g->streams);
    g->streams = NULL;
    }
  gavf_sync_index_free(&g->si);
  memset(&g->si, 0, sizeof(g->si));

  gavf_packet_index_free(&g->pi);
  memset(&g->pi, 0, sizeof(g->pi));
  g->cur = NULL;
  }

const char * gavf_stream_type_name(gavl_stream_type_t t)
  {
  int i;
  for(i = 0; i < sizeof(stream_types)/sizeof(stream_types[0]); i++)
    {
    if(stream_types[i].type == t)
      return stream_types[i].name;
    }
  return NULL;
  }

gavf_options_t * gavf_get_options(gavf_t * g)
  {
  return &g->opt;
  }

void gavf_set_options(gavf_t * g, const gavf_options_t * opt)
  {
  gavf_options_copy(&g->opt, opt);
  }


/* Extensions */

int gavf_extension_header_read(gavf_io_t * io, gavl_extension_header_t * eh)
  {
  if(!gavf_io_read_uint32v(io, &eh->key) ||
     !gavf_io_read_uint32v(io, &eh->len))
    return 0;
  return 1;
  }

int gavf_extension_write(gavf_io_t * io, uint32_t key, uint32_t len,
                         uint8_t * data)
  {
  if(!gavf_io_write_uint32v(io, key) ||
     !gavf_io_write_uint32v(io, len) ||
     (gavf_io_write_data(io, data, len) < len))
    return 0;
  return 1;
  }

/*
 */

static int write_sync_header(gavf_t * g, int stream, int64_t packet_pts)
  {
  int ret = 0;
  int result;
  int i;
  gavf_stream_t * s;
  
  gavl_value_t sync_pts_val;
  gavl_value_t sync_pts_arr_val;
  gavl_array_t * sync_pts_arr;
  gavl_msg_t msg;

  int num;
  
  gavl_value_init(&sync_pts_arr_val);

  sync_pts_arr = gavl_value_set_array(&sync_pts_arr_val);
  
  num = gavl_track_get_num_streams_all(g->cur);
  
  for(i = 0; i < num; i++)
    {
    s = &g->streams[i];
    
    if(i == stream)
      {
      s->sync_pts = packet_pts;
      s->packets_since_sync = 0;
      }
    else if((stream >= 0) &&
            (s->ci.flags & GAVL_COMPRESSION_HAS_B_FRAMES))
      {
      gavl_packet_t * p;
      p = gavf_packet_buffer_peek_read(s->pb);
      if(!p || !(p->flags & GAVL_PACKET_KEYFRAME))
        s->sync_pts = GAVL_TIME_UNDEFINED;
      else
        s->sync_pts = p->pts;
      }
    else if((g->streams[i].flags & STREAM_FLAG_DISCONTINUOUS) &&
            (g->streams[i].next_sync_pts == GAVL_TIME_UNDEFINED))
      {
      s->sync_pts = 0;
      s->packets_since_sync = 0;
      }
    else
      {
      s->sync_pts = s->next_sync_pts;
      s->packets_since_sync = 0;
      }

    gavl_value_init(&sync_pts_val);
    gavl_value_set_long(&sync_pts_val, s->sync_pts);
    
    gavl_array_splice_val_nocopy(sync_pts_arr, -1, 0, &sync_pts_val);
    }

  /* Update sync index */
  if(g->opt.flags & GAVF_OPT_FLAG_SYNC_INDEX)
    {
    gavf_sync_index_add(g, g->io->position);
    }

  /* If that's the first sync header, update file index */
  if(!g->first_sync_pos)
    {
    /* Write GAVF_TAG_PACKETS tag */

    gavf_chunk_start(g->io, &g->packets_chunk, GAVF_TAG_PACKETS);
    
    g->first_sync_pos = g->io->position;
  
    }

  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_WRITE_SYNC_HEADER_START, GAVL_MSG_NS_GAVF);
  gavl_msg_set_arg(&msg, 0, &sync_pts_arr_val);

  result = gavl_msg_send(&msg, g->msg_callback, g->msg_data);
  
  gavl_msg_free(&msg);
  
  if(!result)
    goto fail;
  /* Write the sync header */
  if(gavf_io_write_data(g->io, (uint8_t*)GAVF_TAG_SYNC_HEADER, 8) < 8)
    goto fail;
#if 0
  fprintf(stderr, "Write sync header\n");
#endif
  
  for(i = 0; i < num; i++)
    {
#if 0
    fprintf(stderr, "PTS[%d]: %"PRId64"\n", i, g->streams[i].sync_pts);
#endif
    if(!gavf_io_write_int64v(g->io, g->streams[i].sync_pts))
      goto fail;
    }

  /* Update last sync pts */
  for(i = 0; i < num; i++)
    {
    if(g->streams[i].sync_pts != GAVL_TIME_UNDEFINED)
      g->streams[i].last_sync_pts = g->streams[i].sync_pts;
    }
  if(!gavf_io_flush(g->io))
    goto fail;


  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_WRITE_SYNC_HEADER_END, GAVL_MSG_NS_GAVF);
  gavl_msg_set_arg_nocopy(&msg, 0, &sync_pts_arr_val);
  result = gavl_msg_send(&msg, g->msg_callback, g->msg_data);
  gavl_msg_free(&msg);
  
  ret = 1;
  
  fail:

  gavl_value_free(&sync_pts_arr_val);
  
  return ret;
  }

static gavl_sink_status_t do_write_packet(gavf_t * g, int32_t stream_id, int packet_flags,
                                          int64_t last_sync_pts,
                                          int64_t default_duration, const gavl_packet_t * p)
  {
  int result;
  gavl_msg_t msg;

  //  fprintf(stderr, "do_write_packet\n");
  
  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_WRITE_PACKET_START, GAVL_MSG_NS_GAVF);
  result = gavl_msg_send(&msg, g->msg_callback, g->msg_data);
  gavl_msg_free(&msg);

  if(!result)
    return GAVL_SINK_ERROR;
  
  if((gavf_io_write_data(g->io,
                         (const uint8_t*)GAVF_TAG_PACKET_HEADER, 1) < 1) ||
     (!gavf_io_write_int32v(g->io, stream_id)))
    return GAVL_SINK_ERROR;

  if(!gavf_write_gavl_packet(g->io, g->pkt_io, default_duration, packet_flags, last_sync_pts, p))
    return GAVL_SINK_ERROR;

  if(g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
    {
    fprintf(stderr, "ID: %d ", stream_id);
    gavl_packet_dump(p);
    }
  

  if(!gavf_io_flush(g->io))
    return GAVL_SINK_ERROR;
  
  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_WRITE_PACKET_END, GAVL_MSG_NS_GAVF);
  result = gavl_msg_send(&msg, g->msg_callback, g->msg_data);
  gavl_msg_free(&msg);
  
  return GAVL_SINK_OK;
  }
                                     
                                          

static gavl_sink_status_t
write_packet(gavf_t * g, int stream, const gavl_packet_t * p)
  {
  gavl_time_t pts;
  int write_sync = 0;
  gavf_stream_t * s;

  //  fprintf(stderr, "Write packet\n");
  
  if(stream >= 0)
    s = &g->streams[stream];
  else
    s = NULL;
  
  if(s && (p->pts != GAVL_TIME_UNDEFINED))
    {
    pts = gavl_time_unscale(s->timescale, p->pts);
  
    /* Decide whether to write a sync header */
    if(!g->first_sync_pos)
      write_sync = 1;
    else if(g->sync_distance)
      {
      if(pts - g->last_sync_time > g->sync_distance)
        write_sync = 1;
      }
    else
      {
      if((s->type == GAVL_STREAM_VIDEO) &&
         (s->ci.flags & GAVL_COMPRESSION_HAS_P_FRAMES) &&
         (p->flags & GAVL_PACKET_KEYFRAME) &&
         s->packets_since_sync)
        write_sync = 1;
      }

    if(write_sync)
      {
      if(!write_sync_header(g, stream, p->pts))
        return GAVL_SINK_ERROR;
      if(g->sync_distance)
        g->last_sync_time = pts;
      }
  
    // If a stream has B-frames, this won't be correct
    // for the next sync timestamp (it will be taken from the
    // packet pts in write_sync_header)
  
    if(s->next_sync_pts < p->pts + p->duration)
      s->next_sync_pts = p->pts + p->duration;

    /* Update packet index */
  
    if(g->opt.flags & GAVF_OPT_FLAG_PACKET_INDEX)
      {
      gavf_packet_index_add(&g->pi,
                            s->id, p->flags, g->io->position,
                            p->pts);
      }
    }

  if(do_write_packet(g, s->id, s->packet_flags, s->last_sync_pts,
                     s->packet_duration, p) != GAVL_SINK_OK)
    return GAVL_SINK_ERROR;

#if 0
  
  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_WRITE_PACKET_START, GAVL_MSG_NS_GAVF);
  result = gavl_msg_send(&msg, g->msg_callback, g->msg_data);
  gavl_msg_free(&msg);

  if(!result)
    return GAVL_SINK_ERROR;
  
  if((gavf_io_write_data(g->io,
                         (const uint8_t*)GAVF_TAG_PACKET_HEADER, 1) < 1) ||
     (!gavf_io_write_uint32v(g->io, s->id)))
    return GAVL_SINK_ERROR;

  if(!gavf_write_gavl_packet(g->io, g->pkt_io, s->packet_duration, s->packet_flags, s->last_sync_pts, p))
    return GAVL_SINK_ERROR;

  if(g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
    {
    fprintf(stderr, "ID: %d ", s->id);
    gavl_packet_dump(p);
    }
  

  if(!gavf_io_flush(g->io))
    return GAVL_SINK_ERROR;
  
  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_WRITE_PACKET_END, GAVL_MSG_NS_GAVF);
  result = gavl_msg_send(&msg, g->msg_callback, g->msg_data);
  gavl_msg_free(&msg);

#endif
  
  s->packets_since_sync++;
  return GAVL_SINK_OK;
  }

int gavf_write_gavl_packet(gavf_io_t * io, gavf_io_t * hdr_io, int packet_duration,
                           int packet_flags, int64_t last_sync_pts,
                           const gavl_packet_t * p)
  {
  gavl_buffer_t * buf;
  buf = gavf_io_buf_get(hdr_io);
  gavf_io_buf_reset(hdr_io);

  if(!gavf_write_gavl_packet_header(hdr_io, packet_duration, packet_flags, last_sync_pts, p) ||
     !gavf_io_write_uint32v(io, buf->len + p->data_len) ||
     (gavf_io_write_data(io, buf->buf, buf->len) < buf->len) ||
     (gavf_io_write_data(io, p->data, p->data_len) < p->data_len))
    return 0;
  return 1;
  }


/*
 *  Flush packets. s is the stream of the last packet written.
 *  If s is NULL, flush all streams
 */

gavl_sink_status_t gavf_flush_packets(gavf_t * g, gavf_stream_t * s)
  {
  int i;
  const gavl_packet_t * p;
  int min_index;
  gavl_time_t min_time;
  gavl_time_t test_time;
  gavf_stream_t * ws;
  gavl_sink_status_t st;
#if 0
  if(s)
    fprintf(stderr, "gavf_flush_packets %d %"PRId64"\n", s->id, g->first_sync_pos);
  else
    fprintf(stderr, "gavf_flush_packets NULL %"PRId64"\n", g->first_sync_pos);
#endif
  /* Special case for message streams with undefined timestamp: Transmit out of band */

  if((g->first_sync_pos > 0) && s && (s->type == GAVL_STREAM_MSG))
    {
    for(i = 0; i < g->num_streams; i++)
      {
      if(&g->streams[i] == s)
        {
        if((p = gavf_packet_buffer_get_last(s->pb)) &&
           (p->pts == GAVL_TIME_UNDEFINED))
          {
          st = write_packet(g, i, p);
          gavf_packet_buffer_remove_last(s->pb);
          
          if(st != GAVL_SINK_OK)
            return st;
          }
        }
      }
    }
    
  //  fprintf(stderr, "flush_packets\n");
  
  if(!g->first_sync_pos)
    {
    for(i = 0; i < g->num_streams; i++)
      {
      if(!(g->streams[i].flags & STREAM_FLAG_DISCONTINUOUS) &&
         (g->streams[i].stats.pts_start == GAVL_TIME_UNDEFINED))
        return GAVL_SINK_OK;
      }
    }

  if(g->first_sync_pos > 0)
    {
    /* Flush discontinuous streams */
    for(i = 0; i < g->num_streams; i++)
      {
      ws = &g->streams[i];
      if(!(ws->flags & STREAM_FLAG_DISCONTINUOUS))
        continue;
      
      while((p = gavf_packet_buffer_get_read(ws->pb)))
        {
        fprintf(stderr, "flush_discontinuous\n");
        if((st = write_packet(g, i, p)) != GAVL_SINK_OK)
          return st;
        }
      }
    }
    
  /* Flush as many packets as possible */
  while(1)
    {
    /*
     *  1. Find stream we need to output now
     */

    min_index = -1;
    min_time = GAVL_TIME_UNDEFINED;
    for(i = 0; i < g->num_streams; i++)
      {
      ws = &g->streams[i];

      if(ws->flags & STREAM_FLAG_DISCONTINUOUS)
        continue;
      
      test_time =
        gavf_packet_buffer_get_min_pts(ws->pb);
      
      if(test_time != GAVL_TIME_UNDEFINED)
        {
        if((min_time == GAVL_TIME_UNDEFINED) ||
           (test_time < min_time))
          {
          min_time = test_time;
          min_index = i;
          }
        }
      else
        {
        if(!(ws->flags & STREAM_FLAG_DISCONTINUOUS) &&
           s && (g->encoding_mode == ENC_INTERLEAVE))
          {
          /* Some streams without packets: stop here */
          return GAVL_SINK_OK;
          }
        }
      }

    /*
     *  2. Exit if we are done
     */
    if(min_index < 0)
      return GAVL_SINK_OK;

    /*
     *  3. Output packet
     */

    ws = &g->streams[min_index];
    p = gavf_packet_buffer_get_read(ws->pb);

    if((st = write_packet(g, min_index, p)) != GAVL_SINK_OK)
      return st;
    
    /*
     *  4. If we have B-frames, output the complete Mini-GOP
     */
    
    if(ws->type == GAVL_STREAM_VIDEO)
      {
      while(1)
        {
        p = gavf_packet_buffer_peek_read(ws->pb);
        if(p && ((p->flags & GAVL_PACKET_TYPE_MASK) == GAVL_PACKET_TYPE_B))
          {
          if((st = write_packet(g, min_index, p)) != GAVL_SINK_OK)
            return st;
          }
        else
          break;
        }
      }

    /* Continue */
    
    }
  
  }

static int get_audio_sample_size(const gavl_audio_format_t * fmt,
                                 const gavl_compression_info_t * ci)
  {
  if(!ci || (ci->id == GAVL_CODEC_ID_NONE))
    return gavl_bytes_per_sample(fmt->sample_format);
  else
    return gavl_compression_get_sample_size(ci->id);
  }

int gavf_get_max_audio_packet_size(const gavl_audio_format_t * fmt,
                                   const gavl_compression_info_t * ci)
  {
  int sample_size = 0;

  if(ci && ci->max_packet_size)
    return ci->max_packet_size;

  sample_size =
    get_audio_sample_size(fmt, ci);
  
  return fmt->samples_per_frame * fmt->num_channels * sample_size;
  }


static void set_implicit_stream_fields(gavf_stream_t * s)
  {
  const char * var;
  gavl_dictionary_t * m = gavl_stream_get_metadata_nc(s->h);
  
  var = gavl_compression_get_mimetype(&s->ci);
  if(var)
    gavl_dictionary_set_string(m, GAVL_META_MIMETYPE, var);
  
  var = gavl_compression_get_long_name(s->ci.id);
  if(var)
    gavl_dictionary_set_string(m, GAVL_META_FORMAT, var);

  if(s->ci.bitrate)
    gavl_dictionary_set_int(m, GAVL_META_BITRATE, s->ci.bitrate);
  }

/* Streams */

static void gavf_stream_init_audio(gavf_t * g, gavf_stream_t * s)
  {
  int sample_size;

  s->afmt = gavl_stream_get_audio_format_nc(s->h);
  
  s->timescale = s->afmt->samplerate;
  
  s->ci.max_packet_size =
    gavf_get_max_audio_packet_size(s->afmt, &s->ci);

  sample_size = get_audio_sample_size(s->afmt, &s->ci);
  
  /* Figure out the packet duration */
  if(gavl_compression_constant_frame_samples(s->ci.id) ||
     sample_size)
    s->packet_duration = s->afmt->samples_per_frame;
  else
    s->packet_flags |= GAVF_PACKET_WRITE_DURATION;
  if(GAVF_HAS_FLAG(g, GAVF_FLAG_WRITE))
    {
    /* Create packet sink */
    gavf_stream_create_packet_sink(g, s);
    if(s->ci.id == GAVL_CODEC_ID_NONE)
      {
      gavl_dictionary_t * m = gavl_stream_get_metadata_nc(s->h);
      gavl_dictionary_set_string_endian(m);
      }
    }
  else
    {
    /* Create packet source */
    gavf_stream_create_packet_src(g, s);

    /* Set redundant metadata fields */
    set_implicit_stream_fields(s);
    }
  }

GAVL_PUBLIC 
int gavf_get_max_video_packet_size(const gavl_video_format_t * fmt,
                                   const gavl_compression_info_t * ci)
  {
  if(ci->max_packet_size)
    return ci->max_packet_size;
  if(ci->id == GAVL_CODEC_ID_NONE)
    return gavl_video_format_get_image_size(fmt);
  return 0;
  }


static void gavf_stream_init_video(gavf_t * g, gavf_stream_t * s,
                                   int num_streams)
  {
  s->vfmt = gavl_stream_get_video_format_nc(s->h);
  
  s->timescale = s->vfmt->timescale;

  if((s->ci.flags & GAVL_COMPRESSION_HAS_B_FRAMES) ||
     (s->type == GAVL_STREAM_OVERLAY))
    s->packet_flags |= GAVF_PACKET_WRITE_PTS;
  
  if(s->ci.flags & GAVL_COMPRESSION_HAS_P_FRAMES)
    g->sync_distance = 0;
  
  if(s->vfmt->framerate_mode == GAVL_FRAMERATE_CONSTANT)
    s->packet_duration = s->vfmt->frame_duration;
  else
    s->packet_flags |= GAVF_PACKET_WRITE_DURATION;
  
  if(((s->vfmt->interlace_mode == GAVL_INTERLACE_MIXED) ||
      (s->vfmt->interlace_mode == GAVL_INTERLACE_MIXED_TOP) ||
      (s->vfmt->interlace_mode == GAVL_INTERLACE_MIXED_BOTTOM)) &&
     (s->id == GAVL_CODEC_ID_NONE))
    s->packet_flags |= GAVF_PACKET_WRITE_INTERLACE;

  if(s->flags & GAVL_COMPRESSION_HAS_FIELD_PICTURES)
    s->packet_flags |= GAVF_PACKET_WRITE_FIELD2;
    
  
  if(num_streams > 1)
    {
    if((s->vfmt->framerate_mode == GAVL_FRAMERATE_STILL) ||
       (s->type == GAVL_STREAM_OVERLAY))
      s->flags |= STREAM_FLAG_DISCONTINUOUS;
    }
  
  s->ci.max_packet_size =
    gavf_get_max_video_packet_size(s->vfmt, &s->ci);
  
  if(GAVF_HAS_FLAG(g, GAVF_FLAG_WRITE))
    {
    /* Create packet sink */
    gavf_stream_create_packet_sink(g, s);
    if(s->ci.id == GAVL_CODEC_ID_NONE)
      {
      gavl_dictionary_t * m = gavl_stream_get_metadata_nc(s->h);
      gavl_dictionary_set_string_endian(m);
      }
    }
  else
    {
    /* Create packet source */
    gavf_stream_create_packet_src(g, s);
    
    /* Set redundant metadata fields */
    set_implicit_stream_fields(s);
    }
  }

static void gavf_stream_init_text(gavf_t * g, gavf_stream_t * s,
                                  int num_streams)
  {
  gavl_dictionary_t * m = gavl_stream_get_metadata_nc(s->h);
  
  gavl_dictionary_get_int(m, GAVL_META_STREAM_SAMPLE_TIMESCALE, &s->timescale);
  
  s->packet_flags |=
    (GAVF_PACKET_WRITE_PTS|GAVF_PACKET_WRITE_DURATION);

  if(num_streams > 1)
    s->flags |= STREAM_FLAG_DISCONTINUOUS;
  
  if(GAVF_HAS_FLAG(g, GAVF_FLAG_WRITE))
    {
    /* Create packet sink */
    gavf_stream_create_packet_sink(g, s);
    }
  else
    {
    /* Create packet source */
    gavf_stream_create_packet_src(g, s);

    /* Set redundant metadata fields */
    set_implicit_stream_fields(s);
    }
  }

static void gavf_stream_init_msg(gavf_t * g, gavf_stream_t * s,
                                 int num_streams)
  {
  gavl_dictionary_t * m = gavl_stream_get_metadata_nc(s->h);

  gavl_dictionary_get_int(m, GAVL_META_STREAM_SAMPLE_TIMESCALE, &s->timescale);
  
  s->packet_flags |= (GAVF_PACKET_WRITE_PTS);

  if(num_streams > 1)
    s->flags |= STREAM_FLAG_DISCONTINUOUS;
  
  if(GAVF_HAS_FLAG(g, GAVF_FLAG_WRITE))
    {
    /* Create packet sink */
    gavf_stream_create_packet_sink(g, s);
    }
  else
    {
    /* Create packet source */
    gavf_stream_create_packet_src(g, s);

    /* Set redundant metadata fields */
    set_implicit_stream_fields(s);
    }
  }

#if 0
int gavf_stream_get_timescale(const gavl_dictionary_t * sh)
  {
  gavl_stream_type_t type = gavl_stream_get_type(sh);

  switch(type)
    {
    case GAVL_STREAM_AUDIO:
      return sh->format.audio.samplerate;
      break;
    case GAVL_STREAM_VIDEO:
    case GAVL_STREAM_OVERLAY:
      return sh->format.video.timescale;
      break;
    case GAVL_STREAM_TEXT:
      {
      int ret = 0;
      gavl_dictionary_get_int(&sh->m, GAVL_META_STREAM_SAMPLE_TIMESCALE, &ret);
      return ret;
      }
      break;
    case GAVL_STREAM_NONE:
      break;
    case GAVL_STREAM_MSG:
      return GAVL_TIME_SCALE;
      break;
    }
  return 0;
  }
#endif

static void init_streams(gavf_t * g)
  {
  int i;

  g->num_streams = gavl_track_get_num_streams_all(g->cur);
  
  gavf_sync_index_init(&g->si, g->num_streams);

  
  g->streams = calloc(g->num_streams, sizeof(*g->streams));
  
  for(i = 0; i < g->num_streams; i++)
    {
    g->streams[i].sync_pts = GAVL_TIME_UNDEFINED;
    g->streams[i].next_sync_pts = GAVL_TIME_UNDEFINED;
    g->streams[i].last_sync_pts = GAVL_TIME_UNDEFINED;
    
    g->streams[i].h = gavl_track_get_stream_all_nc(g->cur, i);
    g->streams[i].m = gavl_stream_get_metadata_nc(g->streams[i].h);
    
    g->streams[i].type = gavl_stream_get_type(g->streams[i].h);
    g->streams[i].g = g;
    
    if(!gavl_stream_get_id(g->streams[i].h, &g->streams[i].id))
      {
      
      }
    
    switch(g->streams[i].type)
      {
      case GAVL_STREAM_AUDIO:

        gavl_stream_get_compression_info(g->streams[i].h, &g->streams[i].ci);
        gavf_stream_init_audio(g, &g->streams[i]);
        break;
      case GAVL_STREAM_VIDEO:
      case GAVL_STREAM_OVERLAY:
        gavl_stream_get_compression_info(g->streams[i].h, &g->streams[i].ci);
        gavf_stream_init_video(g, &g->streams[i], g->num_streams);
        break;
      case GAVL_STREAM_TEXT:
        gavf_stream_init_text(g, &g->streams[i], g->num_streams);
        break;
      case GAVL_STREAM_MSG:
        gavf_stream_init_msg(g, &g->streams[i], g->num_streams);
        break;
      case GAVL_STREAM_NONE:
        break;
      }
    g->streams[i].pb =
      gavf_packet_buffer_create(g->streams[i].timescale);
    }

  }

static void gavf_stream_free(gavf_stream_t * s)
  {
  if(s->asrc)
    gavl_audio_source_destroy(s->asrc);
  if(s->vsrc)
    gavl_video_source_destroy(s->vsrc);
  if(s->psrc)
    gavl_packet_source_destroy(s->psrc);

  if(s->asink)
    gavl_audio_sink_destroy(s->asink);
  if(s->vsink)
    gavl_video_sink_destroy(s->vsink);
  if(s->psink)
    gavl_packet_sink_destroy(s->psink);
  if(s->pb)
    gavf_packet_buffer_destroy(s->pb);
  
  if(s->aframe)
    {
    gavl_audio_frame_null(s->aframe);
    gavl_audio_frame_destroy(s->aframe);
    }
  if(s->vframe)
    {
    gavl_video_frame_null(s->vframe);
    gavl_video_frame_destroy(s->vframe);
    }
  if(s->dsp)
    gavl_dsp_context_destroy(s->dsp);
  }



gavf_t * gavf_create()
  {
  gavf_t * ret;
  ret = calloc(1, sizeof(*ret));
  return ret;
  }


static int read_sync_header(gavf_t * g)
  {
  int i;
  int result;
  gavl_msg_t msg;

  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_READ_SYNC_HEADER_START, GAVL_MSG_NS_GAVF);
  result = gavl_msg_send(&msg, g->msg_callback, g->msg_data);
  gavl_msg_free(&msg);

  if(!result)
    return 0;
  
  /* Read sync header */
#if 0
  fprintf(stderr, "Read sync header\n");
#endif
  for(i = 0; i < g->num_streams; i++)
    {
    if(!gavf_io_read_int64v(g->io, &g->streams[i].sync_pts))
      return 0;
#if 0
    fprintf(stderr, "PTS[%d]: %ld\n", i, g->streams[i].sync_pts);
#endif
    if(g->streams[i].sync_pts != GAVL_TIME_UNDEFINED)
      {
      g->streams[i].last_sync_pts = g->streams[i].sync_pts;
      g->streams[i].next_pts = g->streams[i].sync_pts;
      }
    }

  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_READ_SYNC_HEADER_END, GAVL_MSG_NS_GAVF);
  gavl_msg_send(&msg, g->msg_callback, g->msg_data);
  gavl_msg_free(&msg);

  if(!result)
    return 0;
  
  return 1;
  }

void gavf_clear_buffers(gavf_t * g)
  {
  int i;

  for(i = 0; i < g->num_streams; i++)
    {
    if(g->streams[i].pb)
      gavf_packet_buffer_clear(g->streams[i].pb);
    }
  }

#if 0
static void calc_pts_offset(gavf_t * g)
  {
  gavl_time_t min_time= GAVL_TIME_UNDEFINED;
  gavl_time_t test_time;
  int i, min_index = -1;
  int64_t off;
  int scale;
  
  for(i = 0; i < g->num_streams; i++)
    {
    if(g->streams[i].sync_pts != GAVL_TIME_UNDEFINED)
      {
      test_time =
        gavl_time_unscale(g->streams[i].timescale,
                          g->streams[i].sync_pts +g->streams[i].ci.pre_skip);
      if((min_time == GAVL_TIME_UNDEFINED) ||
         (test_time < min_time))
        {
        min_index = i;
        min_time = test_time;
        }
      }
    }

  if(min_index < 0)
    return;

  off = -(g->streams[min_index].sync_pts + g->streams[min_index].ci.pre_skip);
  scale = g->streams[min_index].timescale;
  
  g->streams[min_index].pts_offset = off;
  
  for(i = 0; i < g->num_streams; i++)
    {
    if(i != min_index)
      {
      g->streams[i].pts_offset =
        gavl_time_rescale(scale,
                          g->streams[i].timescale,
                          off);
      }
#if 0
    fprintf(stderr, "PTS offset %d: %"PRId64"\n", i,
            g->streams[i].pts_offset);
#endif
    }
  }
#endif

gavl_dictionary_t * gavf_get_media_info_nc(gavf_t * g)
  {
  return &g->mi;
  }

const gavl_dictionary_t * gavf_get_media_info(const gavf_t * g)
  {
  return &g->mi;
  }

int gavf_select_track(gavf_t * g, int track)
  {
  gavf_chunk_t head;
  int ret = 0;
  free_track(g);

  if(!(g->cur = gavl_get_track_nc(&g->mi, track)))
    return 0;

  g->num_streams = gavl_track_get_num_streams_all(g->cur);

  init_streams(g);
  
  if(GAVF_HAS_FLAG(g, GAVF_FLAG_STREAMING) &&
     !GAVF_HAS_FLAG(g, GAVF_FLAG_MULTI_HEADER))
    {
    /* Seek until the first sync header */

    while(1)
      {
      if(!gavf_io_align_read(g->io))
        goto fail;
      
      if(!gavf_chunk_read_header(g->io, &head))
        goto fail;
      
      fprintf(stderr, "Got signature:\n");
      gavl_hexdump((uint8_t*)head.eightcc, 8, 8);

      if(!strcmp(head.eightcc, GAVF_TAG_PACKETS))
        break;
      }

    if(!gavf_io_align_read(g->io))
      goto fail;

    if(gavf_io_read_data(g->io, (uint8_t*)head.eightcc, 8) < 8)
      goto fail;
    
    fprintf(stderr, "Got signature:\n");
    gavl_hexdump((uint8_t*)head.eightcc, 8, 8);
    
    if(strncmp(head.eightcc, GAVF_TAG_SYNC_HEADER, 8))
      goto fail;

    if(!read_sync_header(g))
      goto fail;
    }
  
  

  if(!GAVF_HAS_FLAG(g, GAVF_FLAG_STREAMING))
    {
    /* Read indices */

    
    }

  ret = 1;
  
  fail:
  return ret;
  }

static int read_dictionary(gavf_io_t * io,
                           gavf_chunk_t * head,
                           gavl_buffer_t * buf,
                           gavl_dictionary_t * ret)
  {
  int result = 0;
  gavf_io_t bufio;
  memset(&bufio, 0, sizeof(bufio));

  gavl_buffer_alloc(buf, head->len);

  if((buf->len = gavf_io_read_data(io, buf->buf, head->len)) < head->len)
    {
    fprintf(stderr, "read_dictionary: Reading %"PRId64" bytes failed (got %d)\n",
            head->len, buf->len);
    goto fail;
    }

  gavf_io_init_buf_read(&bufio, buf);
  
  gavl_dictionary_reset(ret);

  if(!gavl_dictionary_read(&bufio, ret))
    {
    fprintf(stderr, "read_dictionary: Reading dictionary failed\n");
    goto fail;
    }
  result = 1;
  
  fail:
  
  
  gavf_io_cleanup(&bufio);

  return result;
  }

int gavf_read_dictionary(gavf_io_t * io,
                         gavf_chunk_t * head,
                         gavl_dictionary_t * ret)
  {
  int result;
  gavl_buffer_t buf;
  gavl_buffer_init(&buf);
  result = read_dictionary(io, head, &buf, ret);
  gavl_buffer_free(&buf);
  return result;
  }

static int read_header(gavf_t * g,
                       gavf_chunk_t * head,
                       gavl_buffer_t * buf,
                       gavl_dictionary_t * ret)
  {
  int result = 0;
  gavl_msg_t msg;

  
  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_READ_PROGRAM_HEADER_START,
                     GAVL_MSG_NS_GAVF);

  if(!gavl_msg_send(&msg, g->msg_callback, g->msg_data))
    goto fail;
  
  gavl_msg_free(&msg);

  if(!read_dictionary(g->io, head, buf, ret))
    goto fail;
  
  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_READ_PROGRAM_HEADER_END,
                     GAVL_MSG_NS_GAVF);
  gavl_msg_set_arg_dictionary(&msg, 0, ret);
  
  if(!gavl_msg_send(&msg, g->msg_callback, g->msg_data))
    goto fail;

  
  result = 1;
  fail:

  
  gavl_msg_free(&msg);
  
  return result;
  }

int gavf_open_read(gavf_t * g, gavf_io_t * io)
  {
  int ret = 0;
  gavf_io_t bufio;
  gavl_buffer_t buf;
  gavf_chunk_t head;
  gavl_dictionary_t mi;
  
  
  g->io = io;

  gavf_io_reset_position(g->io);
  
  gavf_io_set_msg_cb(g->io, g->msg_callback, g->msg_data);

  /* Look for file end */

  gavl_buffer_init(&buf);
  
  if(gavf_io_can_seek(g->io))
    {
    int num_tracks;
    int64_t footer_pos;
    int64_t header_pos;

    int64_t total_bytes;

    gavl_dictionary_t * track;
    gavl_dictionary_t foot;

    fprintf(stderr, "gavf [read]: Non streaming mode\n");
    
    gavl_dictionary_init(&foot);
    
    /* Read tail */
    
    gavf_io_seek(g->io, -8*3, SEEK_END);

    while(1)
      {
      if(!gavf_chunk_read_header(g->io, &head) ||
         strcmp(head.eightcc, GAVF_TAG_TAIL) ||
         !gavf_io_read_int64f(io, &total_bytes))
        return 0;
      
      header_pos = gavf_io_position(io) - total_bytes;

      footer_pos = head.len - header_pos;
      
      if(header_pos)
        GAVF_SET_FLAG(g, GAVF_FLAG_MULTITRACK);
      
      /* Read track header */
      
      gavf_io_seek(g->io, header_pos, SEEK_SET);

      if(!gavf_chunk_read_header(g->io, &head))
        goto fail;
      
      if(!strcmp(head.eightcc, GAVF_TAG_PROGRAM_HEADER))
        {
        gavl_dictionary_init(&mi);
        if(!read_header(g, &head, &buf, &mi))
          goto fail;
        }
      else
        goto fail;

      num_tracks = gavl_get_num_tracks(&mi);
      
      /* Extract track */
      if(num_tracks > 1)
        {
        /* Multitrack header: Need to switch track by external means */
        track = gavl_get_track_nc(&mi, 0);
        }
      else if(num_tracks == 1)
        track = gavl_get_track_nc(&mi, 0);
      else
        goto fail;
      
      /* Read track footer */

      gavf_io_seek(g->io, footer_pos, SEEK_SET);
      
      if(!gavf_chunk_read_header(g->io, &head) ||
         strcmp(head.eightcc, GAVF_TAG_FOOTER))
        return 0;
      
      gavl_buffer_alloc(&buf, head.len);

      if((buf.len = gavf_io_read_data(g->io, buf.buf, head.len)) < head.len)
        goto fail;

      buf.pos = 0;
      
      gavf_io_init_buf_read(&bufio, &buf);
      
      if(!gavl_dictionary_read(&bufio, &foot))
        goto fail;

      fprintf(stderr, "Got footer\n");
      gavl_dictionary_dump(&foot, 2);
      
      /* Apply footer */

      gavl_track_apply_footer(track, &foot);

      gavl_track_finalize(track);

      // gavl_dictionary_merge2(track, &foot);
      
      fprintf(stderr, "Merged footer\n");
      gavl_dictionary_dump(track, 2);
      
      gavl_prepend_track(&g->mi, track);
      
      /* Go back */
      if(!header_pos)
        break;
      
      gavf_io_seek(g->io, header_pos-8*3, SEEK_SET);
      
      gavl_dictionary_reset(&foot);
      }
    
    }
  else /* Streaming mode */
    {
    GAVF_SET_FLAG(g, GAVF_FLAG_STREAMING);

    fprintf(stderr, "gavf [read]: Streamig mode\n");
    
    g->first_sync_pos = -1;
    
    while(1)
      {
      
      if(!gavf_io_align_read(g->io))
        goto fail;
      
      if(!gavf_chunk_read_header(g->io, &head))
        goto fail;
      
      fprintf(stderr, "Got signature:\n");
      gavl_hexdump((uint8_t*)head.eightcc, 8, 8);

      if(!strcmp(head.eightcc, GAVF_TAG_PROGRAM_HEADER))
        {
        if(!read_header(g, &head, &buf, &g->mi))
          goto fail;
        break;
        }
      goto fail;
      
      //      if(g->first_sync_pos > 0)
        break;
      }
    }

  ret = 1;
  
  fail:

  gavl_buffer_free(&buf);
  
  return ret;
  }

int gavf_reset(gavf_t * g)
  {
  if(g->first_sync_pos != g->io->position)
    {
    if(g->io->seek_func)
      gavf_io_seek(g->io, g->first_sync_pos, SEEK_SET);
    else
      return 0;
    }

  GAVF_CLEAR_FLAG(g, GAVF_FLAG_HAVE_PKT_HEADER);
  
  if(!read_sync_header(g))
    return 0;
  return 1;
  }


const gavf_packet_header_t * gavf_packet_read_header(gavf_t * g)
  {
  int result;
  gavl_msg_t msg;
  
  char c[8];

#ifdef DUMP_EOF
  //  fprintf(stderr, "gavf_packet_read_header\n");
#endif
  
  if(GAVF_HAS_FLAG(g, GAVF_FLAG_EOF))
    {
#ifdef DUMP_EOF
    fprintf(stderr, "EOF 0\n");
#endif
    return NULL;
    }
  
  while(1)
    {
    if(!gavf_io_read_data(g->io, (uint8_t*)c, 1))
      {
#ifdef DUMP_EOF
      fprintf(stderr, "EOF 1\n");
#endif
      goto got_eof;
      }
    /* Check if we need to realign the data */
    if(c[0] == 0x00)
      {
      gavf_io_align_read(g->io);
      
      if(!gavf_io_read_data(g->io, (uint8_t*)c, 1))
        {
#ifdef DUMP_EOF
        fprintf(stderr, "EOF 3\n");
#endif
        goto got_eof;
        }
      }
    
    if(c[0] == GAVF_TAG_PACKET_HEADER_C)
      {
      gavf_stream_t * s;
      /* Got new packet */
      if(!gavf_io_read_int32v(g->io, &g->pkthdr.stream_id))
        {
#ifdef DUMP_EOF
        fprintf(stderr, "EOF 4\n");
#endif
        goto got_eof;
        }
      
      /* Check whether to skip this stream */
      if((s = gavf_find_stream_by_id(g, g->pkthdr.stream_id)) &&
         (s->flags & STREAM_FLAG_SKIP))
        {
        int result;
        gavl_msg_t msg;
        
        if(s->unref_func)
          {
          if(!gavf_packet_read_packet(g, &g->skip_pkt))
            {
#ifdef DUMP_EOF
            fprintf(stderr, "EOF 4\n");
#endif
            goto got_eof;
            }
          s->unref_func(&g->skip_pkt, s->unref_priv);
          
          // if(s->skip_func)
          //   s->skip_func(g, s->skip_priv);
          }
        else
          {
          gavf_packet_skip(g);
          }

        gavl_msg_init(&msg);
        gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_READ_PACKET_END, GAVL_MSG_NS_GAVF);
        result = gavl_msg_send(&msg, g->msg_callback, g->msg_data);
        
        if(!result)
          {
#ifdef DUMP_EOF
          fprintf(stderr, "EOF 5\n");
#endif
          goto got_eof;
          }
        
        }
      else
        {
        /* Demuxer level message */
        if(g->pkthdr.stream_id == GAVL_META_STREAM_ID_MSG_DEMUXER)
          {
          gavl_packet_t p;
          gavl_msg_t msg;
          
          gavl_packet_init(&p);
          gavl_msg_init(&msg);
          
          if(!gavf_read_gavl_packet(g->io,
                                    0, // int default_duration,
                                    0, // int packet_flags,
                                    0, // int64_t last_sync_pts,
                                    NULL, // int64_t * next_pts,
                                    0, // int64_t pts_offset,
                                    &p))
            {
#ifdef DUMP_EOF
            fprintf(stderr, "EOF 6\n");
#endif

            goto got_eof;
            }
          if(!gavf_packet_to_msg(&p, &msg))
            {
#ifdef DUMP_EOF
            fprintf(stderr, "EOF 7\n");
#endif
            goto got_eof;
            }
            
          fprintf(stderr, "Got demuxer message\n");
          gavl_msg_dump(&msg, 2);

          switch(msg.NS)
            {
            case GAVL_MSG_NS_SRC:

              switch(msg.ID)
                {
                case GAVL_MSG_SRC_RESYNC:
                  {
                  int64_t t = 0;
                  int scale = 0;
                  int discard = 0;
                  int discont = 0;
                  
                  gavl_msg_get_src_resync(&msg, &t, &scale, &discard, &discont);
                  fprintf(stderr, "RESYNC: %"PRId64" %d %d %d\n", t, scale, discard, discont);
                  gavl_msg_send(&msg, g->msg_callback, g->msg_data);
                  }
                  break;
                }
              break;
              
            }
          
          gavl_msg_free(&msg);
          gavl_packet_free(&p);

#ifdef DUMP_EOF
          fprintf(stderr, "EOF 8\n");
#endif
          
          goto got_eof;
          }
#if 0
        else if(g->pkthdr.stream_id == GAVL_META_STREAM_ID_MSG_PROGRAM)
          {
          fprintf(stderr, "gavf: Got program level message\n");
          }
#endif   
        GAVF_SET_FLAG(g, GAVF_FLAG_HAVE_PKT_HEADER);
        return &g->pkthdr;
        }
      }
    else
      {
      if(gavf_io_read_data(g->io, (uint8_t*)&c[1], 7) < 7)
        {
#ifdef DUMP_EOF
        fprintf(stderr, "EOF 9\n");
#endif
        goto got_eof;
        }

      if(!strncmp(c, GAVF_TAG_SYNC_HEADER, 8))
        {
        if(!read_sync_header(g))
          {
#ifdef DUMP_EOF
          fprintf(stderr, "EOF 10\n");
#endif
          goto got_eof;
          }
        }
      else if(!strncmp(c, GAVF_TAG_PACKETS, 8))
        gavf_io_skip(g->io, 8); // Skip size
      else if(!strncmp(c, GAVF_TAG_PROGRAM_END, 8))
        {
        gavf_io_skip(g->io, 8); // Skip size
        fprintf(stderr, "EOF 11 %c%c%c%c%c%c%c%c\n",
                c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]);
        goto got_eof;
        }
      else
        {
#ifdef DUMP_EOF
        fprintf(stderr, "EOF 12 %c%c%c%c%c%c%c%c\n",
                c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]);
#endif
        goto got_eof;
        }
      }
    }

#ifdef DUMP_EOF
  fprintf(stderr, "EOF 13\n");
#endif
    
  got_eof:

  GAVF_SET_FLAG(g, GAVF_FLAG_EOF);

  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_GOT_EOF, GAVL_MSG_NS_GAVF);
  result = gavl_msg_send(&msg, g->msg_callback, g->msg_data);
  gavl_msg_free(&msg);

  /* Check of EOF got clered */
  
  if(result && !GAVF_HAS_FLAG(g, GAVF_FLAG_EOF))
    {
    /* Try again */
    return gavf_packet_read_header(g);
    }
  
  return NULL;
  }

int gavf_put_message(void * data, gavl_msg_t * m)
  {
  gavl_packet_t * p;
  gavl_msg_t msg;
  gavf_t * g = data;
  gavf_stream_t * s = 0;
  int i;
  
  gavl_dprintf("Got inline message\n");
  gavl_msg_dump(m, 2);gavl_dprintf("\n");

  for(i = 0; i < g->num_streams; i++)
    {
    if(g->streams[i].type == GAVL_STREAM_MSG)
      {
      s = &g->streams[i];
      break;
      }
    }
  
  if(!s)
    {
    fprintf(stderr, "BUG: Metadata update without metadata stream\n");
    return 0;
    }

  p = gavl_packet_sink_get_packet(s->psink);

  gavl_msg_init(&msg);
  gavf_msg_to_packet(m, p);
  gavl_packet_sink_put_packet(s->psink, p);
  
  return 1;
  }

void gavf_packet_skip(gavf_t * g)
  {
  uint32_t len;
  if(!gavf_io_read_uint32v(g->io, &len))
    return;
  gavf_io_skip(g->io, len);
  }

int gavf_packet_read_packet(gavf_t * g, gavl_packet_t * p)
  {
  gavf_stream_t * s = NULL;

  GAVF_CLEAR_FLAG(g, GAVF_FLAG_HAVE_PKT_HEADER);

  if(!(s = gavf_find_stream_by_id(g, g->pkthdr.stream_id)))
    return 0;
  
  
  if(!gavf_read_gavl_packet(g->io, s->packet_duration, s->packet_flags, s->last_sync_pts, &s->next_pts, s->pts_offset, p))
    return 0;

  p->id = s->id;

  if(g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
    {
    fprintf(stderr, "ID: %d ", p->id);
    gavl_packet_dump(p);
    }

  
  return 1;
  }

const int64_t * gavf_first_pts(gavf_t * gavf)
  {
  if(gavf->opt.flags & GAVF_OPT_FLAG_SYNC_INDEX)
    return gavf->si.entries[0].pts;
  else
    return NULL;
  }

/* Get the last PTS of the streams */

const int64_t * gavf_end_pts(gavf_t * gavf)
  {
  if(gavf->opt.flags & GAVF_OPT_FLAG_SYNC_INDEX)
    return gavf->si.entries[gavf->si.num_entries - 1].pts;
  else
    return NULL;
  }

static gavl_sink_status_t write_demuxer_message(gavf_t * g, const gavl_msg_t * msg)
  {
  gavl_sink_status_t st;

  gavl_packet_t p;
  gavl_packet_init(&p);

  gavf_msg_to_packet(msg, &p);
  st = do_write_packet(g, GAVL_META_STREAM_ID_MSG_DEMUXER, 0, 0, 0, &p);
  gavl_packet_free(&p);
  return st;
  }

void gavf_write_resync(gavf_t * g, int64_t time, int scale, int discard, int discont)
  {
  gavl_msg_t msg;

  if(discard)
    {
    gavf_clear_buffers(g);
    }
  else
    {
    /* Flush packets if any */
    gavf_flush_packets(g, NULL);
    }

  gavf_footer_init(g);
  
  gavl_msg_init(&msg);
  
  gavl_msg_set_src_resync(&msg, time, scale, discard, discont);
  write_demuxer_message(g, &msg);
  gavl_msg_free(&msg);
  g->first_sync_pos = 0;
  }

/* Seek to a specific time. Return the sync timestamps of
   all streams at the current position */

const int64_t * gavf_seek(gavf_t * g, int64_t time, int scale)
  {
  int stream = 0;
  int64_t index_position;
  int64_t time_scaled;
  int done = 0;
  
  gavf_clear_buffers(g);
  
  if(!(g->opt.flags & GAVF_OPT_FLAG_SYNC_INDEX))
    return NULL;

  GAVF_CLEAR_FLAG(g, GAVF_FLAG_EOF);
  
  index_position = g->si.num_entries - 1;
  while(!done)
    {
    /* Find next continuous stream */
    while(g->streams[stream].flags & STREAM_FLAG_DISCONTINUOUS)
      {
      stream++;
      if(stream >= g->num_streams)
        {
        done = 1;
        break;
        }
      }

    if(done)
      break;
    
    time_scaled = gavl_time_rescale(scale, g->streams[stream].timescale, time);
    
    /* Descrease index pointer until we are before this time */
    
    while(g->si.entries[index_position].pts[stream] > time_scaled)
      {
      if(!index_position)
        {
        done = 1;
        break;
        }
      index_position--;
      }
    stream++;

    if(stream >= g->num_streams)
      {
      done = 1;
      break;
      }
    }
  
  /* Seek to the positon */
  gavf_io_seek(g->io, g->si.entries[index_position].pos, SEEK_SET);

  //  fprintf(stderr, "Index position: %ld, file position: %ld\n", index_position,
  //          g->si.entries[index_position].pos);

  return g->si.entries[index_position].pts;
  }


/* Write support */

int gavf_open_write(gavf_t * g, gavf_io_t * io,
                    const gavl_dictionary_t * m)
  {
  g->io = io;

  GAVF_SET_FLAG(g, GAVF_FLAG_WRITE);

  if(!gavf_io_can_seek(io))
    {
    GAVF_SET_FLAG(g, GAVF_FLAG_STREAMING);
    }
  
  gavf_io_set_msg_cb(g->io, g->msg_callback, g->msg_data);
  
  /* Initialize packet buffer */

  g->pkt_io = gavf_io_create_buf_write();

  /* Initialize track */

  g->cur = gavl_append_track(&g->mi, NULL);
  
  if(m)
    gavl_dictionary_copy(gavl_track_get_metadata_nc(g->cur), m);
  
  return 1;
  }

int gavf_append_audio_stream(gavf_t * g,
                             const gavl_compression_info_t * ci,
                             const gavl_audio_format_t * format,
                             const gavl_dictionary_t * m)
  {
  int ret = 0;
  gavl_audio_format_t * fmt;
  gavl_dictionary_t * s = gavl_track_append_stream(g->cur, GAVL_STREAM_AUDIO);

  //  fprintf(stderr, "gavf_append_audio_stream\n");
  //  gavl_dictionary_dump(m, 2);
  
  fmt = gavl_dictionary_get_audio_format_nc(s, GAVL_META_STREAM_FORMAT);
  gavl_audio_format_copy(fmt, format);

  if(ci)
    gavl_stream_set_compression_info(s, ci);
  if(m)
    gavl_dictionary_copy(gavl_stream_get_metadata_nc(s), m);

  gavl_stream_get_id(s, &ret);
  return ret;
  }


int gavf_append_video_stream(gavf_t * g,
                             const gavl_compression_info_t * ci,
                             const gavl_video_format_t * format,
                             const gavl_dictionary_t * m)
  {
  int ret = 0;
  gavl_video_format_t * fmt;
  gavl_dictionary_t * s = gavl_track_append_stream(g->cur, GAVL_STREAM_VIDEO);

  fmt = gavl_dictionary_get_video_format_nc(s, GAVL_META_STREAM_FORMAT);
  gavl_video_format_copy(fmt, format);

  if(ci)
    gavl_stream_set_compression_info(s, ci);
  if(m)
    gavl_dictionary_copy(gavl_stream_get_metadata_nc(s), m);
  
  gavl_stream_get_id(s, &ret);
  return ret;
  }

int gavf_append_text_stream(gavf_t * g,
                            uint32_t timescale,
                            const gavl_dictionary_t * m)
  {
  int ret = 0;
  gavl_dictionary_t * m_dst;
  gavl_dictionary_t * s = gavl_track_append_stream(g->cur, GAVL_STREAM_TEXT);
  
  m_dst = gavl_stream_get_metadata_nc(s);
  gavl_dictionary_set_int(m_dst, GAVL_META_STREAM_SAMPLE_TIMESCALE, timescale);
  gavl_dictionary_set_int(m_dst, GAVL_META_STREAM_PACKET_TIMESCALE, timescale);
  
  gavl_stream_get_id(s, &ret);
  return ret;
  }


int gavf_append_overlay_stream(gavf_t * g,
                            const gavl_compression_info_t * ci,
                            const gavl_video_format_t * format,
                            const gavl_dictionary_t * m)
  {
  int ret = 0;
  gavl_video_format_t * fmt;
  gavl_dictionary_t * s = gavl_track_append_stream(g->cur, GAVL_STREAM_OVERLAY);
  
  fmt = gavl_dictionary_get_video_format_nc(s, GAVL_META_STREAM_FORMAT);
  gavl_video_format_copy(fmt, format);

  if(ci)
    gavl_stream_set_compression_info(s, ci);
  if(m)
    gavl_dictionary_copy(gavl_stream_get_metadata_nc(s), m);
  
  gavl_stream_get_id(s, &ret);
  return ret;
  }

void gavf_add_msg_stream(gavf_t * g, int id)
  {
  gavl_track_append_msg_stream(g->cur, id);
  }

void gavf_add_streams(gavf_t * g, const gavl_dictionary_t * ph)
  {
  gavl_dictionary_copy(g->cur, ph);
  }

int gavf_program_header_write(gavf_t * g)
  {
  gavf_io_t * bufio;
  int ret = 0;
  gavf_chunk_t chunk;
  int result;
  gavl_msg_t msg;

  gavf_io_t * io = g->io;
  
  //  fprintf(stderr, "Writing program header:\n");
  //  gavl_dictionary_dump(dict, 2);
  
  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_WRITE_HEADER_START, GAVL_MSG_NS_GAVF);
  result = gavl_msg_send(&msg, io->msg_callback, io->msg_data);
  gavl_msg_free(&msg);
  if(!result)
    goto fail;
  
  bufio = gavf_chunk_start_io(io, &chunk, GAVF_TAG_PROGRAM_HEADER);
  
  /* Write metadata */
  if(!gavl_dictionary_write(bufio, &g->mi))
    goto fail;

  /* size */
  
  gavf_chunk_finish_io(io, &chunk, bufio);
  
  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_WRITE_HEADER_END, GAVL_MSG_NS_GAVF);
  result = gavl_msg_send(&msg, io->msg_callback, io->msg_data);
  gavl_msg_free(&msg);
  if(!result)
    goto fail;

  
  ret = 1;
  fail:
  return ret;
  }


int gavf_start(gavf_t * g)
  {
  gavl_dictionary_t * m;
  
  if(!GAVF_HAS_FLAG(g, GAVF_FLAG_WRITE) || g->streams)
    return 1;
  
  g->sync_distance = g->opt.sync_distance;
  
  init_streams(g);
  
  gavf_footer_init(g);
  
  if(g->num_streams == 1)
    {
    g->encoding_mode = ENC_SYNCHRONOUS;
    g->final_encoding_mode = ENC_SYNCHRONOUS;
    }
  else
    {
    g->encoding_mode = ENC_STARTING;
    
    if(g->opt.flags & GAVF_OPT_FLAG_INTERLEAVE)
      g->final_encoding_mode = ENC_INTERLEAVE;
    else
      g->final_encoding_mode = ENC_SYNCHRONOUS;
    }
  
  gavl_track_delete_implicit_fields(g->cur);

  m = gavl_track_get_metadata_nc(g->cur);
  gavl_dictionary_set_string(m, GAVL_META_SOFTWARE, PACKAGE"-"VERSION);
  
  if(g->opt.flags & GAVF_OPT_FLAG_DUMP_HEADERS)
    {
    gavl_dprintf("Writing program header\n");
    gavl_dictionary_dump(&g->mi, 2);
    }
  if(!gavf_program_header_write(g))
    return 0;

  if((g->num_streams == 1) && (g->streams[0].type == GAVL_STREAM_MSG))
    {
    /* Also write the sync header */
    write_sync_header(g, 0, 0);
    }
  
  return 1;
  }


void gavf_video_frame_to_packet_metadata(const gavl_video_frame_t * frame,
                                         gavl_packet_t * pkt)
  {
  pkt->pts = frame->timestamp;
  pkt->duration = frame->duration;
  pkt->timecode = frame->timecode;
  pkt->interlace_mode = frame->interlace_mode;

  gavl_rectangle_i_copy(&pkt->src_rect, &frame->src_rect);
  pkt->dst_x = frame->dst_x;
  pkt->dst_y = frame->dst_y;
  }

/* LEGACY */
int gavf_write_video_frame(gavf_t * g,
                           int stream, gavl_video_frame_t * frame)
  {
  gavf_stream_t * s = &g->streams[stream];
  if(!s->vsink)
    return 0;
  return (gavl_video_sink_put_frame(s->vsink, frame) == GAVL_SINK_OK);
  }

void gavf_packet_to_video_frame(gavl_packet_t * p, gavl_video_frame_t * frame,
                                const gavl_video_format_t * format,
                                const gavl_dictionary_t * m,
                                gavl_dsp_context_t ** ctx)
  {
  frame->timestamp = p->pts;
  frame->duration = p->duration;

  frame->interlace_mode = p->interlace_mode;
  frame->timecode  = p->timecode;

  gavl_rectangle_i_copy(&frame->src_rect, &p->src_rect);
  frame->dst_x = p->dst_x;
  frame->dst_y = p->dst_y;
  
  frame->strides[0] = 0;
  gavl_video_frame_set_planes(frame, format, p->data);

  if(gavl_metadata_do_swap_endian(m))
    {
    if(!(*ctx))
      *ctx = gavl_dsp_context_create();
    gavl_dsp_video_frame_swap_endian(*ctx, frame, format);
    }
  }

static void get_overlay_format(const gavl_video_format_t * src,
                               gavl_video_format_t * dst,
                               const gavl_rectangle_i_t * src_rect)
  {
  gavl_video_format_copy(dst, src);
  dst->image_width  = src_rect->w + src_rect->x;
  dst->image_height = src_rect->h + src_rect->y;
  gavl_video_format_set_frame_size(dst, 0, 0);
  }

void gavf_packet_to_overlay(gavl_packet_t * p, gavl_video_frame_t * frame,
                            const gavl_video_format_t * format)
  {
  gavl_video_format_t copy_format;
  gavl_video_frame_t tmp_frame_src;
  
  frame->timestamp = p->pts;
  frame->duration  = p->duration;
  
  memset(&tmp_frame_src, 0, sizeof(tmp_frame_src));

  get_overlay_format(format, &copy_format, &p->src_rect);
  gavl_video_frame_set_planes(&tmp_frame_src, &copy_format, p->data);
  
  gavl_video_frame_copy(&copy_format, frame, &tmp_frame_src);

  gavl_rectangle_i_copy(&frame->src_rect, &p->src_rect);
  frame->dst_x = p->dst_x;
  frame->dst_y = p->dst_y;
  }

void gavf_overlay_to_packet(gavl_video_frame_t * frame, 
                            gavl_packet_t * p,
                            const gavl_video_format_t * format)
  {
  gavl_video_format_t copy_format;
  gavl_video_frame_t tmp_frame_src;
  gavl_video_frame_t tmp_frame_dst;
  int sub_h, sub_v; // Not necessary yet but well....
  gavl_rectangle_i_t rect;
  p->pts      = frame->timestamp;
  p->duration = frame->duration;
  
  memset(&tmp_frame_src, 0, sizeof(tmp_frame_src));
  memset(&tmp_frame_dst, 0, sizeof(tmp_frame_dst));
  gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
  
  gavl_rectangle_i_copy(&p->src_rect, &frame->src_rect);

  /* Shift rectangles */

  p->src_rect.x = frame->src_rect.x % sub_h;
  p->src_rect.y = frame->src_rect.y % sub_v;
  
  get_overlay_format(format, &copy_format, &p->src_rect);

  rect.x = frame->src_rect.x - p->src_rect.x;
  rect.y = frame->src_rect.y - p->src_rect.y;
  rect.w = copy_format.image_width;
  rect.h = copy_format.image_height;
  
  gavl_video_frame_get_subframe(format->pixelformat,
                                frame,
                                &tmp_frame_src,
                                &rect);

  /* p->data is assumed to have the proper allocation already!! */
  gavl_video_frame_set_planes(&tmp_frame_dst, &copy_format, p->data);

  gavl_video_frame_copy(&copy_format, &tmp_frame_dst, &tmp_frame_src);
  
  p->dst_x = frame->dst_x;
  p->dst_y = frame->dst_y;
  p->data_len = gavl_video_format_get_image_size(&copy_format);
  }

void gavf_audio_frame_to_packet_metadata(const gavl_audio_frame_t * frame,
                                         gavl_packet_t * pkt)
  {
  pkt->pts = frame->timestamp;
  pkt->duration = frame->valid_samples;
  }

int gavf_write_audio_frame(gavf_t * g, int stream, gavl_audio_frame_t * frame)
  {
  gavf_stream_t * s;
  gavl_audio_frame_t * frame_internal;  
  
  s = &g->streams[stream];
  if(!s->asink)
    return 0;

  frame_internal = gavl_audio_sink_get_frame(s->asink);
  //  gavl_video_frame_copy(&s->h->format.video, frame_internal, frame);

  frame_internal->valid_samples =
    gavl_audio_frame_copy(s->afmt,
                          frame_internal,
                          frame,
                          0,
                          0,
                          frame->valid_samples,
                          s->afmt->samples_per_frame);
  
  frame_internal->timestamp = frame->timestamp;
  return (gavl_audio_sink_put_frame(s->asink, frame_internal) == GAVL_SINK_OK);
  }

void gavf_packet_to_audio_frame(gavl_packet_t * p,
                                gavl_audio_frame_t * frame,
                                const gavl_audio_format_t * format,
                                const gavl_dictionary_t * m,
                                gavl_dsp_context_t ** ctx)
  {
  frame->valid_samples = p->duration;
  frame->timestamp = p->pts;
  gavl_audio_frame_set_channels(frame, format, p->data);

  if(gavl_metadata_do_swap_endian(m))
    {
    if(!(*ctx))
      *ctx = gavl_dsp_context_create();
    gavl_dsp_audio_frame_swap_endian(*ctx, frame, format);
    }

  }


/* Close */

void gavf_close(gavf_t * g, int discard)
  {
  if(GAVF_HAS_FLAG(g, GAVF_FLAG_WRITE))
    {
    if(g->streams && !discard)
      {
      /* Flush packets if any */
      if(!discard)
        {
        gavf_flush_packets(g, NULL);
        
        /* Append final sync header */
        write_sync_header(g, -1, GAVL_TIME_UNDEFINED);
        }
      }
    
    /* Write footer (might fail silently) */

    if(GAVF_HAS_FLAG(g, GAVF_FLAG_STREAMING))
      {
      gavf_chunk_t pend;
      memset(&pend, 0, sizeof(pend));
      /* Just write a program end code */
      gavf_chunk_start(g->io, &pend, GAVF_TAG_PROGRAM_END);
      }
    else
      gavf_footer_write(g);
    }
  
  /* Free stuff */

  free_track(g);
  
  gavl_dictionary_free(&g->mi);
  
  gavl_packet_free(&g->skip_pkt);

  if(g->pkt_io)
    gavf_io_destroy(g->pkt_io);
  
  free(g);
  }

gavl_packet_sink_t *
gavf_get_packet_sink(gavf_t * g, uint32_t id)
  {
  gavf_stream_t * s;
  if(!(s = gavf_find_stream_by_id(g, id)))
    return NULL;
  return s->psink;
  }

gavl_packet_source_t *
gavf_get_packet_source(gavf_t * g, uint32_t id)
  {
  gavf_stream_t * s;
  if(!(s = gavf_find_stream_by_id(g, id)))
    return NULL;
  return s->psrc;
  }

void gavf_stream_set_skip(gavf_t * g, uint32_t id)
  {
  gavf_stream_t * s;
  if((s = gavf_find_stream_by_id(g, id)))
    s->flags |= STREAM_FLAG_SKIP;
  }
  
void gavf_stream_set_unref(gavf_t * g, uint32_t id,
                           gavf_packet_unref_func func, void * priv)
  {
  gavf_stream_t * s;
  if((s = gavf_find_stream_by_id(g, id)))
    {
    s->unref_func = func;
    s->unref_priv = priv;
    
    if(s->pb)
      gavf_packet_buffer_set_unref_func(s->pb, func, priv);
    }
  }

gavf_stream_t * gavf_find_stream_by_id(gavf_t * g, int32_t id)
  {
  int i;
  for(i = 0; i < g->num_streams; i++)
    {
    if(g->streams[i].id == id)
      return &g->streams[i];
    }
  return NULL;
  }


int gavf_chunk_is(const gavf_chunk_t * head, const char * eightcc)
  {
  if(!strncmp(head->eightcc, eightcc, 8))
    return 1;
  else
    return 0;
  }


int gavf_chunk_read_header(gavf_io_t * io, gavf_chunk_t * head)
  {
  /* Byte align (if not already aligned) */
  if(!gavf_io_align_read(io))
    return 0;
  
  if((gavf_io_read_data(io, (uint8_t*)head->eightcc, 8) < 8) ||
     !gavf_io_read_int64f(io, &head->len))
    return 0;
  /* Be strcmp friendly */
  head->eightcc[8] = 0x00;
  return 1;
  }

int gavf_chunk_start(gavf_io_t * io, gavf_chunk_t * head, const char * eightcc)
  {
  gavf_io_align_write(io);
  
  head->start = gavf_io_position(io);

  memcpy(head->eightcc, eightcc, 8);
  
  if((gavf_io_write_data(io, (const uint8_t*)eightcc, 8) < 8) ||
     !gavf_io_write_int64f(io, 0))
    return 0;
  return 1;
  }

int gavf_chunk_finish(gavf_io_t * io, gavf_chunk_t * head, int write_size)
  {
  int64_t position;
  int64_t size;
  
  position = gavf_io_position(io);
  
  if(write_size && gavf_io_can_seek(io))
    {
    size = (position - head->start) - 16;

    position = gavf_io_position(io);
    
    gavf_io_seek(io, head->start+8, SEEK_SET);
    gavf_io_write_int64f(io, size);
    gavf_io_seek(io, position, SEEK_SET);
    }

  /* Pad to multiple of 8 */
    
  gavf_io_align_write(io);

  return 1;
  }

gavf_io_t * gavf_chunk_start_io(gavf_io_t * io, gavf_chunk_t * head, const char * eightcc)
  {
  gavf_io_t * sub_io = gavf_io_create_mem_write();
  
  gavf_chunk_start(sub_io, head, eightcc);
  return sub_io;
  }
  

int gavf_chunk_finish_io(gavf_io_t * io, gavf_chunk_t * head, gavf_io_t * sub_io)
  {
  int len = 0;
  uint8_t * ret;
  
  gavf_chunk_finish(sub_io, head, 1);
  ret = gavf_io_mem_get_buf(sub_io, &len);
  gavf_io_destroy(sub_io);
  
  gavf_io_write_data(io, ret, len);
  gavf_io_flush(io);
  free(ret);

  return 1;
  }

void gavf_set_msg_cb(gavf_t * g, gavl_handle_msg_func msg_callback, void * msg_data)
  {
  g->msg_callback = msg_callback;
  g->msg_data = msg_data;
  }

gavl_dictionary_t * gavf_get_current_track_nc(gavf_t * g)
  {
  return g->cur;
  }

const gavl_dictionary_t * gavf_get_current_track(const gavf_t * g)
  {
  return g->cur;
  }

int gavf_get_flags(gavf_t * g)
  {
  return g->flags;
  
  }

void gavf_clear_eof(gavf_t * g)
  {
  GAVF_CLEAR_FLAG(g, GAVF_FLAG_EOF);
  }
