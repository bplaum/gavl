#include <gavfprivate.h>
#include <gavl/metatags.h>

/* Packet source */

static gavl_source_status_t
read_packet_func_separate(void * priv, gavl_packet_t ** p)
  {
  gavl_msg_t msg;
  gavf_stream_t * s = priv;

  gavl_packet_t pkt;
  gavl_packet_init(&pkt);

  gavl_msg_init(&msg);

  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_READY, GAVL_MSG_NS_GAVF);
  gavl_msg_write(&msg, s->io);
  gavl_msg_free(&msg);
  
  if(!gavf_read_gavl_packet_header(s->io, &pkt))
    return GAVL_SOURCE_EOF;

  if(pkt.id == GAVL_META_STREAM_ID_MSG_GAVF)
    {
    return GAVL_SOURCE_EOF;
    }

  gavl_packet_copy_metadata(*p, &pkt);

  if(!gavf_read_gavl_packet(s->io, *p))
    {
    return GAVL_SOURCE_EOF;
    }
  
  if(s->g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
    {
    gavl_packet_dump(*p);
    }
  
  return GAVL_SOURCE_OK;
  }

#if 0
static gavl_source_status_t
read_packet_func_separate_discont(void * priv, gavl_packet_t ** p)
  {
  gavl_msg_t msg;
  gavf_stream_t * s = priv;

  gavl_packet_t pkt;
  gavl_packet_init(&pkt);
  
  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_READY, GAVL_MSG_NS_GAVF);
  gavl_msg_write(&msg, s->io);
  gavl_msg_free(&msg);
  
  if(!gavf_read_gavl_packet_header(s->io, &pkt))
    return GAVL_SOURCE_EOF;

  if(pkt.id == GAVL_META_STREAM_ID_MSG_GAVF)
    {
    return GAVL_SOURCE_EOF;
    }

  gavl_packet_copy_metadata(*p, &pkt);

  if(!gavf_read_gavl_packet(s->io, *p))
    {
    return GAVL_SOURCE_EOF;
    }
  
  if(s->g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
    {
    gavl_packet_dump(*p);
    }
  
  return GAVL_SOURCE_OK;
  }
#endif

/* Process one packet */
gavl_source_status_t gavf_demux_iteration(gavf_t * g)
  {
  gavl_packet_t pkt;
  gavf_stream_t * read_stream;

  gavl_packet_init(&pkt);
  
  if(!gavf_read_gavl_packet_header(g->io, &pkt))
    return GAVL_SOURCE_EOF;

  if(pkt.id == GAVL_META_STREAM_ID_MSG_GAVF)
    {
    /* TODO: Handle messages */
    return GAVL_SOURCE_EOF;
    }
  
  read_stream = gavf_find_stream_by_id(g, pkt.id);
  if(!read_stream)
    {
    /* Should never happen */
    return GAVL_SOURCE_EOF;
    }

  if(read_stream->flags & STREAM_FLAG_SKIP)
    {
    if(!gavf_skip_gavl_packet(g->io, &pkt))
      return GAVL_SOURCE_EOF;
    }
  else
    {
    gavl_packet_t * read_packet;

    read_packet = gavf_packet_buffer_get_write(read_stream->pb);
    gavl_packet_copy_metadata(read_packet, &pkt);

    if(!gavf_read_gavl_packet(g->io, read_packet))
      return GAVL_SOURCE_EOF;
    
    gavf_packet_buffer_done_write(read_stream->pb);
    }
  
  
  
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
  if(GAVF_HAS_FLAG(g, GAVF_FLAG_SEPARATE_STREAMS))
    {
    func = read_packet_func_separate;
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
  
  s->psrc = gavl_packet_source_create(func, s, flags, s->h);
  }

/* Packet sink */

static gavl_sink_status_t
put_packet_func_separate(void * priv, gavl_packet_t * p)
  {
  gavf_stream_t * s = priv;
  p->id = s->id;

  while(1)
    {
    gavl_msg_t msg;
    gavl_msg_init(&msg);
    
    if(!gavl_msg_read(&msg, s->io))
      return 0;

    switch(msg.NS)
      {
      case GAVL_MSG_NS_SRC:

        
        break;

      case GAVL_MSG_NS_GAVF:

        switch(msg.ID)
          {
          case GAVL_MSG_GAVF_READY:
            break;
          }
        break;
        
      }
    
    gavl_msg_free(&msg);
    
    }

  if(!gavf_write_gavl_packet(s->io, s, p))
    {
    return GAVL_SINK_ERROR;
    }
  else
    return GAVL_SINK_OK;
  
  }

static gavl_sink_status_t
put_packet_func_multiplex(void * priv, gavl_packet_t * p)
  {
  gavf_stream_t * s = priv;
  p->id = s->id;
  
  /* Update footer */
#if 0
  fprintf(stderr, "put packet %d\n", s->id);
  gavl_packet_dump(p);
#endif
  
  gavl_stream_stats_update(&s->stats, p);

  if(!gavf_write_gavl_packet(s->g->io, s, p))
    return GAVL_SINK_ERROR;
  
  //  return gavf_flush_packets(s->g, s);

  /* TODO */
  return GAVL_SINK_OK;
  // return gavf_write_packet(s->g, (int)(s - s->g->streams), p) ?
  //    GAVL_SINK_OK : GAVL_SINK_ERROR;
  }

#if 0
static gavl_packet_t *
get_packet_func_multiplex(void * priv)
  {
  gavf_stream_t * s = priv;
  return gavf_packet_buffer_get_write(s->pb);
  }
#endif

void gavf_stream_create_packet_sink(gavf_t * g, gavf_stream_t * s)
  {
  if(GAVF_HAS_FLAG(g, GAVF_FLAG_SEPARATE_STREAMS))
    {
    s->psink = gavl_packet_sink_create(NULL,
                                       put_packet_func_separate, s);
    }
  else
    {
    s->psink = gavl_packet_sink_create(NULL,
                                       put_packet_func_multiplex, s);
    }
  
  }
