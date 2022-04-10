#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <config.h>

#include <gavfprivate.h>
#include <metatags.h>

#include <gavl/http.h>
#include <gavl/gavlsocket.h>
#include <gavl/log.h>

#define LOG_DOMAIN "gavf"

#define PROTOCOL "GAVF/"VERSION

#define DUMP_EOF

/*
 *   A gavf handle acts like a software HDMI-Plug which allows to send
 *   A/V data between processes.
 *   
 *   We support:
 *   - Sending A/V streams via separate sockets minimizing latency due to multiplexing
 *   - Sending of shared frames with zero copy (dma-buffers or shared memory)
 *
 *  On Unix domain sockets, the gavf connection is controlled via messages.
 *  In the main media directions, messages are sent in the message stream
 *  with ID GAVL_META_STREAM_ID_MSG_CONTROL (-2). In the opposite direction the messages are
 *  send as "elementary stream" using gavl_msg_write().
 *
 *  Messages from dst to src:
 *
 *  GAVL_CMD_SRC_SELECT_TRACK
 *  GAVL_CMD_SRC_START
 *  GAVL_CMD_SRC_SEEK
 *  GAVL_CMD_SRC_PAUSE
 *  GAVL_CMD_SRC_RESUME
 *  
 *
 */

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
  gavf_packet_index_free(&g->pi);
  memset(&g->pi, 0, sizeof(g->pi));

  gavl_dictionary_reset(&g->cur);
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
  
  if(s->vfmt->framerate_mode == GAVL_FRAMERATE_CONSTANT)
    s->packet_duration = s->vfmt->frame_duration;

#if 0  
  if(((s->vfmt->interlace_mode == GAVL_INTERLACE_MIXED) ||
      (s->vfmt->interlace_mode == GAVL_INTERLACE_MIXED_TOP) ||
      (s->vfmt->interlace_mode == GAVL_INTERLACE_MIXED_BOTTOM)) &&
     (s->id == GAVL_CODEC_ID_NONE))
    s->packet_flags |= GAVF_PACKET_WRITE_INTERLACE;

  if(s->flags & GAVL_COMPRESSION_HAS_FIELD_PICTURES)
    s->packet_flags |= GAVF_PACKET_WRITE_FIELD2;
#endif
  
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

  g->num_streams = gavl_track_get_num_streams_all(&g->cur);
  
  g->streams = calloc(g->num_streams, sizeof(*g->streams));
  
  for(i = 0; i < g->num_streams; i++)
    {
    g->streams[i].h = gavl_track_get_stream_all_nc(&g->cur, i);
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

  if(s->hwctx)
    gavl_hw_ctx_destroy(s->hwctx);
    
  if(s->io)
    gavf_io_destroy(s->io);
  
  }



gavf_t * gavf_create()
  {
  gavf_t * ret;
  ret = calloc(1, sizeof(*ret));
  return ret;
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

#if 0
int gavf_select_track(gavf_t * g, int track)
  {
  //  gavf_chunk_t head;
  int ret = 0;
  
  free_track(g);

  if(!(g->cur = gavl_get_track_nc(&g->mi, track)))
    return 0;

  g->num_streams = gavl_track_get_num_streams_all(g->cur);

  init_streams(g);

  if(GAVF_HAS_FLAG(g, GAVF_FLAG_INTERACTIVE))
    {
    gavl_msg_t msg;
    
    /* Forward comand */
    gavl_msg_init(&msg);
    
    }
  else
    {
    /* TODO: File based multi-track support */
    
    }
#if 0
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
    
    }
  
  

  if(!GAVF_HAS_FLAG(g, GAVF_FLAG_STREAMING))
    {
    /* Read indices */

    
    }

#endif
  
  ret = 1;
  
  //  fail:
  return ret;
  }

#endif

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

#if 0
static int read_header(gavf_t * g,
                       gavf_chunk_t * head,
                       gavl_buffer_t * buf,
                       gavl_dictionary_t * ret)
  {
  int result = 0;
  gavl_msg_t msg;

  
  if(!read_dictionary(g->io, head, buf, ret))
    goto fail;
  
  result = 1;
  fail:

  
  gavl_msg_free(&msg);
  
  return result;
  }
#endif

int gavf_open_read(gavf_t * g, gavf_io_t * io)
  {
  int ret = 0;
  //  gavf_io_t bufio;
  gavl_buffer_t buf;
  //  gavf_chunk_t head;
  //  gavl_dictionary_t mi;
  
  
  g->io = io;

  gavf_io_reset_position(g->io);
  
  //  gavf_io_set_msg_cb(g->io, g->msg_callback, g->msg_data);

  /* Look for file end */

  gavl_buffer_init(&buf);
  
  if(gavf_io_can_seek(g->io))
    {
#if 0
    
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
#endif
    }
  else /* Streaming mode */
    {
#if 0
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
#endif
    }

  ret = 1;
  
  //  fail:

  gavl_buffer_free(&buf);
  
  return ret;
  }

int gavf_reset(gavf_t * g)
  {
#if 0
  if(g->first_sync_pos != g->io->position)
    {
    if(g->io->seek_func)
      gavf_io_seek(g->io, g->first_sync_pos, SEEK_SET);
    else
      return 0;
    }
#endif
  return 1;
  }

#if 0
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
        
        
        }
      else
        {
        /* Demuxer level message */
        if(g->pkthdr.stream_id == GAVL_META_STREAM_ID_MSG_GAVF)
          {
          gavl_packet_t p;
          gavl_msg_t msg;
          
          gavl_packet_init(&p);
          gavl_msg_init(&msg);
          
          if(!gavf_read_gavl_packet(g->io,
                                    0, // int default_duration,
                                    0, // int packet_flags,
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
#if 0
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
#endif
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

      if(!strncmp(c, GAVF_TAG_PACKETS, 8))
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
#endif

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

#if 0
int gavf_packet_read_packet(gavf_t * g, gavl_packet_t * p)
  {
  gavf_stream_t * s = NULL;

  //  GAVF_CLEAR_FLAG(g, GAVF_FLAG_HAVE_PKT_HEADER);

  if(!(s = gavf_find_stream_by_id(g, g->pkthdr.stream_id)))
    return 0;
  
  if(!gavf_read_gavl_packet(g->io, s->packet_duration, s->packet_flags, &s->next_pts, s->pts_offset, p))
    return 0;

  p->id = s->id;

  if(g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
    {
    fprintf(stderr, "ID: %d ", p->id);
    gavl_packet_dump(p);
    }

  
  return 1;
  }
#endif

#if 0
static gavl_sink_status_t write_demuxer_message(gavf_t * g, const gavl_msg_t * msg)
  {
  gavl_sink_status_t st;

  gavl_packet_t p;
  gavl_packet_init(&p);

  gavf_msg_to_packet(msg, &p);
  st = do_write_packet(g, GAVL_META_STREAM_ID_MSG_GAVF, 0, 0, &p);
  gavl_packet_free(&p);
  return st;
  }
#endif

/* Seek to a specific time. Return the sync timestamps of
   all streams at the current position */

const int64_t * gavf_seek(gavf_t * g, int64_t time, int scale)
  {
#if 0 // TODO: Reimplement
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
#endif
  return NULL;
  }


/* Write support */

#if 0
int gavf_open_write(gavf_t * g, gavf_io_t * io,
                    const gavl_dictionary_t * m)
  {
  g->io = io;

  GAVF_SET_FLAG(g, GAVF_FLAG_WRITE);

#if 0  
  if(!gavf_io_can_seek(io))
    {
    GAVF_SET_FLAG(g, GAVF_FLAG_STREAMING);
    }
  gavf_io_set_msg_cb(g->io, g->msg_callback, g->msg_data);
#endif
  
  
  /* Initialize packet buffer */

  g->pkt_io = gavf_io_create_buf_write();

  /* Initialize track */

  g->cur = gavl_append_track(&g->mi, NULL);
  
  if(m)
    gavl_dictionary_copy(gavl_track_get_metadata_nc(g->cur), m);
  
  return 1;
  }
#endif

static int check_media_info(gavf_t * g, const gavl_dictionary_t * mi)
  {
  int i, j;
  int num_streams;
  gavl_stream_type_t type;

  const gavl_dictionary_t * track;
  const gavl_dictionary_t * stream;
  int num_tracks = gavl_get_num_tracks(mi);
  
  if(num_tracks < 1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Need at least one track\n");
    return 0;
    }
  
  for(i = 0; i < num_tracks; i++)
    {
    track = gavl_get_track(mi, i);

    num_streams = gavl_track_get_num_streams_all(track);

    for(j = 0; j < num_streams; j++)
      {
      stream = gavl_track_get_stream_all(track, i);

      type = gavl_stream_get_type(stream);

      switch(type)
        {
        case GAVL_STREAM_AUDIO:
          break;
        case GAVL_STREAM_VIDEO:
          break;
        case GAVL_STREAM_TEXT:
          break;
        case GAVL_STREAM_OVERLAY:
          break;
        case GAVL_STREAM_MSG:
          break;
        case GAVL_STREAM_NONE:
          break;
        }
      
      }

    }
  
  return 1;
  }

int gavf_set_media_info(gavf_t * g, const gavl_dictionary_t * mi)
  {
  if(!check_media_info(g, mi))
    return 0;

  gavl_dictionary_copy(&g->mi, mi);

  if(GAVF_HAS_FLAG(g, GAVF_FLAG_ONDISK))
    {
    /* TODO: Write program header */
    }
  else if(GAVF_HAS_FLAG(g, GAVF_FLAG_INTERACTIVE))
    {
    gavl_packet_t pkt;
    gavl_msg_t msg;
    gavl_msg_init(&msg);
    gavl_packet_init(&pkt);
    gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_MEDIA_INFO, GAVL_MSG_NS_GAVF);
    gavl_msg_set_arg_dictionary(&msg, 0, &g->mi);

    gavf_msg_to_packet(&msg, &pkt);
    pkt.id = GAVL_META_STREAM_ID_MSG_GAVF;
    pkt.pts = 0;

    if(!gavf_write_gavl_packet(g->io, NULL, &pkt))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Writing media info failed");
      }
    
    gavl_packet_free(&pkt);
    gavl_msg_free(&msg);

    //    fprintf(stderr, "Wrote media info:\n");
    //    gavl_dictionary_dump(&g->mi, 2);
    
    
    }
  
  return 1;
  }


int gavf_program_header_write(gavf_t * g)
  {
  gavf_io_t * bufio;
  int ret = 0;
  gavf_chunk_t chunk;
  gavf_io_t * io = g->io;
  
  //  fprintf(stderr, "Writing program header:\n");
  //  gavl_dictionary_dump(dict, 2);
  
  
  bufio = gavf_chunk_start_io(io, &chunk, GAVF_TAG_PROGRAM_HEADER);
  
  /* Write metadata */
  if(!gavl_dictionary_write(bufio, &g->mi))
    goto fail;

  /* size */
  
  gavf_chunk_finish_io(io, &chunk, bufio);
  

  
  ret = 1;
  fail:
  return ret;
  }


int gavf_start(gavf_t * g)
  {
  gavl_dictionary_t * m;
  
  if(!GAVF_HAS_FLAG(g, GAVF_FLAG_WRITE) || g->streams)
    return 1;
  
  init_streams(g);
  
  gavf_footer_init(g);
  
  gavl_track_delete_implicit_fields(&g->cur);

  m = gavl_track_get_metadata_nc(&g->cur);
  gavl_dictionary_set_string(m, GAVL_META_SOFTWARE, PACKAGE"-"VERSION);
  
  if(g->opt.flags & GAVF_OPT_FLAG_DUMP_HEADERS)
    {
    gavl_dprintf("Writing program header\n");
    gavl_dictionary_dump(&g->mi, 2);
    }
  if(!gavf_program_header_write(g))
    return 0;

  return 1;
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
  gavl_packet_to_video_frame_metadata(p, frame);
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
    
    /* Write footer (might fail silently) */
#if 0
    if(GAVF_HAS_FLAG(g, GAVF_FLAG_STREAMING))
      {
      gavf_chunk_t pend;
      memset(&pend, 0, sizeof(pend));
      /* Just write a program end code */
      gavf_chunk_start(g->io, &pend, GAVF_TAG_PROGRAM_END);
      }
    else
      gavf_footer_write(g);
#endif
    }
  
  /* Free stuff */

  free_track(g);
  
  gavl_dictionary_free(&g->mi);
  
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
  return &g->cur;
  }

const gavl_dictionary_t * gavf_get_current_track(const gavf_t * g)
  {
  return &g->cur;
  }

int gavf_get_flags(gavf_t * g)
  {
  return g->flags;
  
  }

void gavf_clear_eof(gavf_t * g)
  {
  GAVF_CLEAR_FLAG(g, GAVF_FLAG_EOF);
  }

/* New open functions */

static int read_pipe(void * priv, uint8_t * data, int len)
  {
  return fread(data, 1, len, priv);
  }

static int write_pipe(void * priv, const uint8_t * data, int len)
  {
  return fwrite(data, 1, len, priv);
  }

static void close_pipe(void * priv)
  {
  pclose(priv);
  }

static int flush_pipe(void * priv)
  {
  if(!fflush(priv))
    return 1;
  return 0;
  }

static int client_handshake(gavf_io_t * io, int wr, const char * path)
  {
  int ret = 0;
  gavl_dictionary_t req;
  gavl_dictionary_t res;

  gavl_dictionary_init(&req);
  gavl_dictionary_init(&res);

  gavl_http_request_init(&req, (wr ? "PUT" : "GET"), path, PROTOCOL);

  if(!gavl_http_request_write(io, &req))
    goto fail;

  if(!gavl_http_response_read(io, &res))
    goto fail;

  if(wr && gavl_http_response_get_status_int(&res) != 100)
    goto fail;

  else if(!wr && gavl_http_response_get_status_int(&res) != 200)
    goto fail;
  
  ret = 1;
  
  fail:
  
  gavl_dictionary_free(&req);
  gavl_dictionary_free(&res);

  return ret;
  }

static int server_handshake(gavf_io_t * io, int wr, const char * path)
  {
  int ret = 0;
  const char * method;
  const char * protocol;
  gavl_dictionary_t req;
  gavl_dictionary_t res;
  
  gavl_dictionary_init(&req);
  gavl_dictionary_init(&res);

  if(!gavl_http_request_read(io, &req))
    return 0;

  method = gavl_http_request_get_method(&req);
  protocol = gavl_http_request_get_protocol(&req);

  if(strcmp(protocol, PROTOCOL))
    {
    gavl_http_response_init(&res, PROTOCOL, 400, "Bad Request");
    goto fail;
    }
  
  if(!strcmp(method, "GET"))
    {
    if(!wr)
      {
      gavl_http_response_init(&res, PROTOCOL, 405, "Method Not Allowed");
      goto fail;
      }

    if(strcmp(path, gavl_http_request_get_path(&req)))
      {
      gavl_http_response_init(&res, PROTOCOL, 404, "Not Found");
      goto fail;
      }

    gavl_http_response_init(&res, PROTOCOL, 200, "OK");
    ret = 1;
    
    }
  else if(!strcmp(method, "PUT"))
    {
    if(wr)
      {
      gavl_http_response_init(&res, PROTOCOL, 405, "Method Not Allowed");
      goto fail;
      }

    if(strcmp(path, gavl_http_request_get_path(&req)))
      {
      gavl_http_response_init(&res, PROTOCOL, 404, "Not Found");
      goto fail;
      }

    gavl_http_response_init(&res, PROTOCOL, 100, "Continue");
    ret = 1;
    }

  fail:
  
  gavl_http_response_write(io, &res);

  gavl_dictionary_free(&req);
  gavl_dictionary_free(&res);
  
  return ret;
  }


static int open_socket(const char * uri, int wr, gavf_io_t ** io)
  {
  int ret = 0;
  char * host = NULL;
  char * path = NULL;
  char * protocol = NULL;
  int port = 0;
  int fd;
  int server_fd = -1;
  gavl_socket_address_t * addr = NULL;
  int timeout = 1000;
  
  if(!gavl_url_split(uri, &protocol, NULL, NULL, &host, &port, &path))
    goto fail;

  if(!host || !port)
    goto fail;

  if(!path)
    path = gavl_strdup("/");
  
  ret = 1;
  
  if(!strcmp(protocol, GAVF_PROTOCOL_TCP))
    {
    addr = gavl_socket_address_create();
    if(!gavl_socket_address_set(addr, host, port, SOCK_STREAM))
      goto fail;
    
    fd = gavl_socket_connect_inet(addr, timeout);
    if(fd < 0)
      goto fail;
    
    *io = gavf_io_create_socket(fd, 2000, GAVF_IO_SOCKET_DO_CLOSE);
    
    if(!client_handshake(*io, wr, path))
      {
      gavf_io_destroy(*io);
      *io = NULL;
      goto fail;
      }
    }
  else if(!strcmp(protocol, GAVF_PROTOCOL_TCPSERV))
    {
    addr = gavl_socket_address_create();
    if(!gavl_socket_address_set(addr, host, port, SOCK_STREAM))
      goto fail;

    server_fd = gavl_listen_socket_create_inet(addr, 0, 1, 0);

    fd = gavl_listen_socket_accept(server_fd, -1, NULL);

    if(fd < 0)
      goto fail;

    *io = gavf_io_create_socket(fd, 2000, GAVF_IO_SOCKET_DO_CLOSE);

    if(!server_handshake(*io, wr, path))
      {
      gavf_io_destroy(*io);
      *io = NULL;
      goto fail;
      }
    }
  else if(!strcmp(protocol, GAVF_PROTOCOL_UNIX))
    {
    if(!strcmp(path, "/"))
      goto fail;

    fd = gavl_socket_connect_unix(path);

    if(fd < 0)
      goto fail;

    *io = gavf_io_create_socket(fd, 2000, GAVF_IO_SOCKET_DO_CLOSE);

    if(!client_handshake(*io, wr, "/"))
      {
      gavf_io_destroy(*io);
      *io = NULL;
      goto fail;
      }
    
    }
  else if(!strcmp(protocol, GAVF_PROTOCOL_UNIXSERV))
    {
    server_fd = gavl_listen_socket_create_unix(path, 1);
    
    fd = gavl_listen_socket_accept(server_fd, -1, NULL);

    if(fd < 0)
      goto fail;

    *io = gavf_io_create_socket(fd, 2000, GAVF_IO_SOCKET_DO_CLOSE);

    if(!server_handshake(*io, wr, "/"))
      {
      gavf_io_destroy(*io);
      *io = NULL;
      goto fail;
      }
    
    
    }
  else
    {
    /* Invalid protocol */
    ret = 0;
    goto fail;
    }
  
  fail:

  if(server_fd >= 0)
    gavl_listen_socket_destroy(server_fd);
  
  if(host)
    free(host);
  if(path)
    free(path);
  if(protocol)
    free(protocol);
  
  return ret;
  }

static void get_io_flags(gavf_t * g)
  {
  int flags;
  flags = gavf_io_get_flags(g->io);
  
  if(flags & GAVF_IO_IS_SOCKET)
    {
    GAVF_SET_FLAG(g, GAVF_FLAG_INTERACTIVE);
    GAVF_SET_FLAG(g, GAVF_FLAG_SEPARATE_STREAMS);
    
    if(flags & GAVF_IO_IS_UNIX_SOCKET)
      {
      GAVF_SET_FLAG(g, GAVF_FLAG_UNIX);
      }
    }
  else if(flags & GAVF_IO_IS_REGULAR)
    {
    GAVF_SET_FLAG(g, GAVF_FLAG_ONDISK);
    }
  
  }

static int skip_to_msg(gavf_t * g, gavl_msg_t * ret, int id, int ns)
  {
  int i;
  gavl_packet_t pkt;
  
  for(i = 0; i < 10; i++)
    {
    gavl_packet_init(&pkt);
  
    if(!gavf_read_gavl_packet_header(g->io, &pkt))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Readig packet header failed");
      return 0;
      }

    //    fprintf(stderr, "Got packet: ");
    //    gavl_packet_dump(&pkt);
    
    if(pkt.id == GAVL_META_STREAM_ID_MSG_GAVF)
      {
      if(!gavf_read_gavl_packet(g->io, &pkt))
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Readig packet failed");
        return 0;
        }
      if(!gavf_packet_to_msg(&pkt, ret))
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Parsing message failed");
        return 0;
        }
      if((ret->NS == ns) && (ret->ID == id))
        return 1;
      else
        {
        gavl_msg_free(ret);
        gavl_msg_init(ret);
        }
      }
    else
      {
      if(!gavf_skip_gavl_packet(g->io, &pkt))
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Skipping packet failed");
        return 0;
        }
      }
    gavl_packet_free(&pkt);
    }
  return 0;
  }

int gavf_open_uri_write(gavf_t * g, const char * uri)
  {
  gavf_io_t * io = NULL;
  int flags;
  //  int result;
  
  if(gavl_string_starts_with(uri, GAVF_PROTOCOL"://"))
    uri += strlen(GAVF_PROTOCOL"://");
  
  if(uri[0] == '|')
    {
    FILE * f;
    flags = GAVF_IO_CAN_WRITE | GAVF_IO_IS_LOCAL | GAVF_IO_IS_PIPE;
    
    uri++;
    while(isspace(*uri) && (*uri != '\0'))
      uri++;
    
    f = popen(uri, "w");
    
    io = gavf_io_create(NULL, write_pipe, NULL, close_pipe , flush_pipe, flags, f);
    }
  else if(!strcmp(uri, "-"))
    {
    if(isatty(fileno(stdout)))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Won't write to terminal");
      return 0;
      }
    
    io =  gavf_io_create_file(stdout, 1, 0, 0);

      

    }
  else if(open_socket(uri, 1, &io))
    {
    
    }
  else
    {
    FILE * f;
    f = fopen(uri, "w");
    io =  gavf_io_create_file(f, 1, 1, 1);
    }

  if(!io)
    return 0;

  g->io = io;
  
  flags = gavf_io_get_flags(g->io);
  
  if(flags & GAVF_IO_IS_PIPE)
    {
    int fd;
    int server_fd;
    char * name = NULL;
    char * redirect_uri = NULL;
    gavl_dictionary_t header;

    io = NULL;

    g->io_orig = g->io;
    
    server_fd = gavl_unix_socket_create(&name, 1);

    redirect_uri = gavl_sprintf("%s://%s", GAVF_PROTOCOL_UNIX, name);
    
    gavl_dictionary_init(&header);
    gavl_http_request_init(&header, "REDIRECT", redirect_uri, PROTOCOL);
    gavl_http_request_write(g->io, &header);
    gavl_dictionary_free(&header);
    free(redirect_uri);

    gavf_io_flush(g->io);
    
    fd = gavl_listen_socket_accept(server_fd, 1000, NULL);
    gavl_listen_socket_destroy(server_fd);
    free(name);
    
    if(fd < 0)
      {
      return 0;
      }

    io = gavf_io_create_socket(fd, 2000, GAVF_IO_SOCKET_DO_CLOSE);
    
    if(!server_handshake(io, 1, "/"))
      {
      gavf_io_destroy(io);
      return 0;
      }

    g->io = io;

    }

  get_io_flags(g);
  return 1;
  }

int gavf_open_uri_read(gavf_t * g, const char * uri)
  {
  gavf_io_t * io;
  int flags;
  
  if(gavl_string_starts_with(uri, GAVF_PROTOCOL"://"))
    uri += strlen(GAVF_PROTOCOL"://");
  
  if(uri[0] == '<')
    {
    FILE * f;
    flags = GAVF_IO_CAN_WRITE | GAVF_IO_IS_LOCAL | GAVF_IO_IS_PIPE;
    
    uri++;
    while(isspace(*uri) && (*uri != '\0'))
      uri++;
    
    f = popen(uri, "r");
    
    io = gavf_io_create(read_pipe, NULL, NULL, close_pipe , flush_pipe, flags, f);
    }
  else if(!strcmp(uri, "-"))
    {
    io =  gavf_io_create_file(stdin, 0, 0, 0);
    }
  else if(open_socket(uri, 0, &io))
    {
    
    }
  else
    {
    FILE * f;
    f = fopen(uri, "r");
    io =  gavf_io_create_file(f, 0, 1, 1);
    }

  if(!io)
    return 0;

  g->io = io;
  
  flags = gavf_io_get_flags(io);

  /* Do pipe redirection */

  if(flags & GAVF_IO_IS_PIPE)
    {
    int ret = 0;
    const char * method;
    const char * path;
    gavl_dictionary_t header;
    
    g->io_orig = g->io;
    g->io = NULL;
    
    gavl_dictionary_init(&header);

    if(!gavl_http_request_read(g->io_orig, &header))
      return 0;

    if(!(method = gavl_http_request_get_method(&header)))
      return 0;

    if(!(path = gavl_http_request_get_path(&header)))
      return 0;

    if(strcmp(method, "REDIRECT"))
      return 0;
    
    gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Got redirection to %s", path);
    
    ret = gavf_open_uri_read(g, path);

    gavl_dictionary_free(&header);
    return ret;
    }

  get_io_flags(g);

  if(GAVF_HAS_FLAG(g, GAVF_FLAG_ONDISK))
    {
    /* TODO: Read program header */
    }
  else if(GAVF_HAS_FLAG(g, GAVF_FLAG_INTERACTIVE))
    {
    /* Read media info */
    gavl_msg_t msg;
    gavl_msg_init(&msg);
    if(!skip_to_msg(g, &msg, GAVL_MSG_GAVF_MEDIA_INFO, GAVL_MSG_NS_GAVF))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got no media info");
      return 0;
      }
    gavl_msg_get_arg_dictionary(&msg, 0, &g->mi);
    gavl_msg_free(&msg);
    }
  
  return 1;
  }

static int open_stream_sockets(gavf_t * g)
  {
  int i, num_streams;

  //  server_fd = 
  //  redirect_uri = gavl_sprintf("%s://%s", GAVF_PROTOCOL_UNIX, name);

  num_streams = gavl_track_get_num_streams_all(&g->cur);
  
  for(i = 0; i < num_streams; i++)
    {
    gavl_dictionary_t * gavf;
    char * name = NULL;
    g->streams[i].server_fd = gavl_unix_socket_create(&name, 1);

    gavf = gavl_dictionary_get_dictionary_create(g->streams[i].h, GAVF_DICT);
    gavl_dictionary_set_string_nocopy(gavf, GAVL_META_URI, name);
    }
  return 1;
  }

static int accept_stream_sockets(gavf_t * g)
  {
  int i, num_streams;
  int fd;
  int ret = 1;
  //  server_fd = 
  //  redirect_uri = gavl_sprintf("%s://%s", GAVF_PROTOCOL_UNIX, name);

  num_streams = gavl_track_get_num_streams_all(&g->cur);
  
  for(i = 0; i < num_streams; i++)
    {
    if((fd = gavl_listen_socket_accept(g->streams[i].server_fd, 2000, NULL)) >= 0)
      {
      g->streams[i].io = gavf_io_create_socket(fd, 2000, GAVF_IO_SOCKET_DO_CLOSE);
      }
    else
      ret = 0;
    
    gavl_listen_socket_destroy(g->streams[i].server_fd);
    }
  return ret;
  }

static int connect_stream_sockets(gavf_t * g)
  {
  int i, num_streams;
  int fd;
  int ret = 1;
  const char * uri;
  const gavl_dictionary_t * gavf;
  
  num_streams = gavl_track_get_num_streams_all(&g->cur);
  
  for(i = 0; i < num_streams; i++)
    {
    if((gavf = gavl_dictionary_get_dictionary(g->streams[i].h, GAVF_DICT)) &&
       (uri = gavl_dictionary_get_string(gavf, GAVL_META_URI)) &&
       (fd = gavl_socket_connect_unix(uri)))
      {
      g->streams[i].io = gavf_io_create_socket(fd, 2000, GAVF_IO_SOCKET_DO_CLOSE);
      }
    }
  return ret;
  
  }

int gavf_read_writer_command(gavf_t * g, int timeout)
  {
  int ret = 1;
  gavl_msg_t msg;
  
  gavl_msg_init(&msg);
  
  if(!gavf_io_can_read(g->io, timeout))
   goto fail;

  if(!gavl_msg_read(&msg, g->io))
    goto fail;

  switch(msg.NS)
    {
    case GAVL_MSG_NS_SRC:
      {
      switch(msg.ID)
        {
        case GAVL_CMD_SRC_SELECT_TRACK:
          {
          /* TODO: Resync writer and send response */
          const gavl_dictionary_t * cur;
          gavl_msg_t resp;
          gavl_packet_t pkt;
          int track;
          
          /* Call external callback. This should stop all threads */
          if(g->msg_callback)
            g->msg_callback(g->msg_data, &msg);

          free_track(g);
          
          if(GAVF_HAS_FLAG(g, GAVF_FLAG_STARTED))
            {
            GAVF_CLEAR_FLAG(g, GAVF_FLAG_STARTED);
            /* TODO: Shut down previous playback */
            }
          
          track = gavl_msg_get_arg_int(&msg, 0);
          
          if(!(cur = gavl_get_track_nc(&g->mi, track)))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Invalid track: %d", track);
            goto fail;
            }
          
          gavl_dictionary_copy(&g->cur, cur);
          init_streams(g);
          
          if(GAVF_HAS_FLAG(g, GAVF_FLAG_SEPARATE_STREAMS))
            {
            /* Open stream sockets */
            open_stream_sockets(g);
            }

          gavl_packet_init(&pkt);
          gavl_msg_init(&resp);
          gavl_msg_set_id_ns(&resp, GAVL_MSG_GAVF_SELECT_TRACK, GAVL_MSG_NS_GAVF);
          gavl_msg_set_arg_dictionary(&resp, 0, &g->cur);

          gavf_msg_to_packet(&resp, &pkt);
          pkt.id = GAVL_META_STREAM_ID_MSG_GAVF;
          gavf_write_gavl_packet(g->io, NULL, &pkt);
          gavl_msg_free(&msg);
          gavl_packet_free(&pkt);
          
          if(GAVF_HAS_FLAG(g, GAVF_FLAG_SEPARATE_STREAMS))
            {
            accept_stream_sockets(g);
            }
          
          }
          break;
        case GAVL_CMD_SRC_SEEK:
          /* TODO: Resync writer and send response */

          /* Call external callback. This should stop all threads */
          if(g->msg_callback)
            g->msg_callback(g->msg_data, &msg);
          
          break;
        case GAVL_CMD_SRC_START:
          /* Reader is about to start */
          GAVF_SET_FLAG(g, GAVF_FLAG_STARTED);

          /* Send response */
          
          break;
        case GAVL_CMD_SRC_PAUSE:
        case GAVL_CMD_SRC_RESUME:
        default:
          /* Nothing */
          if(g->msg_callback)
            g->msg_callback(g->msg_data, &msg);
          
          break;
        }
      break;
      }
    case GAVL_MSG_NS_GAVF:
      /* Nothing */
      if(g->msg_callback)
        g->msg_callback(g->msg_data, &msg);
      break;
    }
  
  fail:
  
  gavl_msg_free(&msg);
  
  return ret;
  }

int gavf_handle_reader_command(gavf_t * g, const gavl_msg_t * m)
  {
  switch(m->NS)
    {
    case GAVL_MSG_NS_SRC:
      switch(m->ID)
        {
        case GAVL_CMD_SRC_SELECT_TRACK:
          if(GAVF_HAS_FLAG(g, GAVF_FLAG_INTERACTIVE))
            {
            gavl_msg_t resp;
            free_track(g);
            
            gavl_msg_write(m, g->io);

            if(GAVF_HAS_FLAG(g, GAVF_FLAG_STARTED))
              {
              GAVF_CLEAR_FLAG(g, GAVF_FLAG_STARTED);
              /* TODO: Shut down previous playback */
              }

            gavl_msg_init(&resp);
            
            if(!skip_to_msg(g, &resp, GAVL_MSG_GAVF_SELECT_TRACK, GAVL_MSG_NS_GAVF) ||
               !gavl_msg_get_arg_dictionary(&resp, 0, &g->cur))
              {
              gavl_msg_free(&resp);
              return 0;
              }
            
            gavl_msg_free(&resp);
            
            init_streams(g);

            if(GAVF_HAS_FLAG(g, GAVF_FLAG_SEPARATE_STREAMS))
              {
              connect_stream_sockets(g);
              }
            
            }
          else
            {
            /* TODO */
            }
          break;
        case GAVL_CMD_SRC_SEEK:
          if(GAVF_HAS_FLAG(g, GAVF_FLAG_INTERACTIVE))
            {
            gavl_msg_write(m, g->io);
            }
          else
            {
            /* TODO */
            }
          break;
        case GAVL_CMD_SRC_START:
          if(GAVF_HAS_FLAG(g, GAVF_FLAG_INTERACTIVE))
            {
            gavl_msg_write(m, g->io);
            GAVF_SET_FLAG(g, GAVF_FLAG_STARTED);

            

            }
          else
            {
            /* TODO */
            }
          break;
        case GAVL_CMD_SRC_PAUSE:
          if(GAVF_HAS_FLAG(g, GAVF_FLAG_INTERACTIVE))
            {
            gavl_msg_write(m, g->io);
            }
          break;
        case GAVL_CMD_SRC_RESUME:
          if(GAVF_HAS_FLAG(g, GAVF_FLAG_INTERACTIVE))
            {
            gavl_msg_write(m, g->io);
            }
          break;
        }
      break;
    }

  return 1;
  }

gavf_io_t * gavf_get_io(gavf_t * g)
  {
  return g->io;
  }

/* Called for writers */
int gavf_start_program(gavf_t * g, const gavl_dictionary_t * track)
  {
  if(!GAVF_HAS_FLAG(g, GAVF_FLAG_WRITE))
    return 0;

  gavl_dictionary_reset(&g->cur);
  gavl_dictionary_copy(&g->cur, track);
  
  if(GAVF_HAS_FLAG(g, GAVF_FLAG_INTERACTIVE))
    {
    gavl_msg_t resp;
    gavl_msg_init(&resp);
    gavl_msg_set_id_ns(&resp, GAVL_MSG_GAVF_START, GAVL_MSG_NS_GAVF);
    gavl_msg_set_arg_dictionary(&resp, 0, &g->cur);

    /* TODO: Initialize streams */
    
    }
  else if(GAVF_HAS_FLAG(g, GAVF_FLAG_ONDISK))
    {
    /* TODO: Finalize last program */
    
    /* TODO: Remove disabled streams and write program header */
    
    }
  return 1;  
  }
