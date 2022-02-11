#include <gavfprivate.h>

/* Packet source */

static gavl_source_status_t
read_packet_func_nobuffer(void * priv, gavl_packet_t ** p)
  {
  gavf_stream_t * s = priv;

  if(!GAVF_HAS_FLAG(s->g, GAVF_FLAG_HAVE_PKT_HEADER) && !gavf_packet_read_header(s->g))
    return GAVL_SOURCE_EOF;

  if(s->g->pkthdr.stream_id != s->id)
    return GAVL_SOURCE_AGAIN;

  GAVF_CLEAR_FLAG(s->g, GAVF_FLAG_HAVE_PKT_HEADER);
  
  if(!gavf_read_gavl_packet(s->g->io, s->packet_duration, s->packet_flags, s->last_sync_pts, &s->next_pts, s->pts_offset, *p))
    return GAVL_SOURCE_EOF;

  (*p)->id = s->id;

  if(s->g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
    {
    fprintf(stderr, "ID: %d ", s->g->pkthdr.stream_id);
    gavl_packet_dump(*p);
    }

  
  
  return GAVL_SOURCE_OK;
  }

gavl_source_status_t gavf_demux_iteration(gavf_t * g)
  {
  gavl_packet_t * read_packet;
  gavf_stream_t * read_stream;
  
  /* Read header */
  if(!GAVF_HAS_FLAG(g, GAVF_FLAG_HAVE_PKT_HEADER) &&
     !gavf_packet_read_header(g))
    {
    //      fprintf(stderr, "Have no header\n");
    return GAVL_SOURCE_EOF;
    }

  read_stream = gavf_find_stream_by_id(g, g->pkthdr.stream_id);
  if(!read_stream)
    {
    /* Should never happen */
    return GAVL_SOURCE_EOF;
    }
  
  read_packet = gavf_packet_buffer_get_write(read_stream->pb);

  if(!gavf_read_gavl_packet(g->io, read_stream->packet_duration, read_stream->packet_flags,
                            read_stream->last_sync_pts, &read_stream->next_pts, read_stream->pts_offset, read_packet))
    return GAVL_SOURCE_EOF;

  read_packet->id = read_stream->id;
  
  gavf_packet_buffer_done_write(read_stream->pb);
  //    fprintf(stderr, "Got packet id: %d\n", read_stream->h->id);
  //    gavl_packet_dump(read_packet);
    

  GAVF_CLEAR_FLAG(g, GAVF_FLAG_HAVE_PKT_HEADER);
  
  return GAVL_SOURCE_OK;
  }

static gavl_source_status_t
read_packet_func_buffer_cont(void * priv, gavl_packet_t ** p)
  {
  gavl_source_status_t st;
  gavf_stream_t * s = priv;
  gavf_t * g = s->g;
  
  while(!(*p = gavf_packet_buffer_get_read(s->pb)))
    {
    if((st = gavf_demux_iteration(g)) != GAVL_SOURCE_OK)
      return st;
    }
  
  if(g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
    {
    fprintf(stderr, "ID: %d ", g->pkthdr.stream_id);
    gavl_packet_dump(*p);
    }
  
  return GAVL_SOURCE_OK;
  }

static gavl_source_status_t
read_packet_func_buffer_discont(void * priv, gavl_packet_t ** p)
  {
  gavf_stream_t * s = priv;

  if((*p = gavf_packet_buffer_get_read(s->pb)))
    return GAVL_SOURCE_OK;
  else if(GAVF_HAS_FLAG(s->g, GAVF_FLAG_EOF))
    return GAVL_SOURCE_EOF;
  else
    return GAVL_SOURCE_AGAIN;
  }

void gavf_stream_create_packet_src(gavf_t * g, gavf_stream_t * s)
  {
  gavl_packet_source_func_t func;
  int flags;
  
  if(!(g->opt.flags & GAVF_OPT_FLAG_BUFFER_READ))
    {
    func = read_packet_func_nobuffer;
    flags = 0;
    }
  else
    {
    flags = GAVL_SOURCE_SRC_ALLOC;
    if(s->flags & STREAM_FLAG_DISCONTINUOUS)
      func = read_packet_func_buffer_discont;
    else
      func = read_packet_func_buffer_cont;
    }
  
  switch(s->type)
    {
    case GAVL_STREAM_AUDIO:
      s->psrc = gavl_packet_source_create_audio(func, s, flags,
                                                &s->ci, s->afmt);
      break;
    case GAVL_STREAM_OVERLAY:
    case GAVL_STREAM_VIDEO:
      s->psrc = gavl_packet_source_create_video(func, s, flags,
                                                &s->ci, s->vfmt);
      break;
    case GAVL_STREAM_TEXT:
      s->psrc = gavl_packet_source_create_text(func, s, flags,
                                               gavl_stream_get_sample_timescale(s->h));
      break;
    case GAVL_STREAM_MSG:
      s->psrc = gavl_packet_source_create(func, s, flags);
      break;
    case GAVL_STREAM_NONE:
      break;
    }
  }

/* Packet sink */

static gavl_sink_status_t
put_packet_func(void * priv, gavl_packet_t * p)
  {
  gavf_stream_t * s = priv;
  gavf_packet_buffer_done_write(s->pb);
  p->id = s->id;
  
  /* Update footer */
#if 0
  fprintf(stderr, "put packet %d\n", s->id);
  gavl_packet_dump(p);
#endif
  /* Fist packet */
  if(s->stats.pts_start == GAVL_TIME_UNDEFINED)
    s->next_sync_pts = p->pts;
  
  gavl_stream_stats_update(&s->stats, p);
  
  return gavf_flush_packets(s->g, s);
  
  // return gavf_write_packet(s->g, (int)(s - s->g->streams), p) ?
  //    GAVL_SINK_OK : GAVL_SINK_ERROR;
  }

static gavl_packet_t *
get_packet_func(void * priv)
  {
  gavf_stream_t * s = priv;
  return gavf_packet_buffer_get_write(s->pb);
  }

void gavf_stream_create_packet_sink(gavf_t * g, gavf_stream_t * s)
  {
  /* Create packet sink */
  s->psink = gavl_packet_sink_create(get_packet_func,
                                     put_packet_func, s);
  
  }
