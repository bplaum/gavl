#include <string.h>
#include <stdlib.h>

#include <gavfprivate.h>
#include <gavl/trackinfo.h>
#include <gavl/metatags.h>

#if 0
int gavf_program_header_read(gavf_io_t * io, gavf_program_header_t * ph)
  {
  int64_t len = 0;
  gavl_buffer_t buf;
  gavf_io_t bufio;
  int ret = 0;
  gavl_dictionary_t dict;

  int result;
  gavl_msg_t msg;

  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_READ_PROGRAM_HEADER_START, GAVL_MSG_NS_GAVF);
  result = gavl_msg_send(&msg, io->msg_callback, io->msg_data);
  gavl_msg_free(&msg);
  if(!result)
    goto fail;
  
  gavl_buffer_init(&buf);
  gavl_dictionary_init(&dict);
  
  if(!gavf_io_read_int64f(io, &len))
    goto fail;

  gavl_buffer_alloc(&buf, len);
  
  if((buf.len = gavf_io_read_data(io, buf.buf, len)) < len)
    goto fail;
  
  gavf_io_init_buf_read(&bufio, &buf);
  
  if(!gavl_dictionary_read(&bufio, &dict))
    goto fail;

  gavf_program_header_from_dictionary(ph, &dict);
  
  gavf_footer_init(ph);
  

  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_READ_PROGRAM_HEADER_END, GAVL_MSG_NS_GAVF);
  result = gavl_msg_send(&msg, io->msg_callback, io->msg_data);
  gavl_msg_free(&msg);
  if(!result)
    goto fail;
  
  ret = 1;
  fail:
  
  gavl_buffer_free(&buf);
  
  return ret;
  }


static gavf_stream_header_t *
add_stream(gavf_program_header_t * ph, const gavl_dictionary_t * m)
  {
  gavf_stream_header_t * ret;
  ph->num_streams++;
  
  ph->streams = realloc(ph->streams,
                        ph->num_streams * sizeof(*ph->streams));
  
  ret = &ph->streams[ph->num_streams-1];
  memset(ret, 0, sizeof(*ret));

  if(m)
    gavl_dictionary_copy(&ret->m, m);
  gavl_metadata_delete_implicit_fields(&ret->m);
  
  ret->id = ph->num_streams;
  
  /* Initialize footer */
  ret->stats.duration_min = GAVL_TIME_UNDEFINED;
  ret->stats.duration_max = GAVL_TIME_UNDEFINED;
  ret->stats.pts_start    = GAVL_TIME_UNDEFINED;
  ret->stats.pts_end      = GAVL_TIME_UNDEFINED;
  return ret;
  }

int gavf_program_header_add_audio_stream(gavf_program_header_t * ph,
                                         const gavl_compression_info_t * ci,
                                         const gavl_audio_format_t * format,
                                         const gavl_dictionary_t * m)
  {
  gavf_stream_header_t * h = add_stream(ph, m);

  h->type = GAVL_STREAM_AUDIO;
  
  gavl_compression_info_copy(&h->ci, ci);
  gavl_audio_format_copy(&h->format.audio, format);
  return ph->num_streams-1;
  }

int gavf_program_header_add_video_stream(gavf_program_header_t * ph,
                                         const gavl_compression_info_t * ci,
                                         const gavl_video_format_t * format,
                                         const gavl_dictionary_t * m)
  {
  gavf_stream_header_t * h = add_stream(ph, m);
  h->type = GAVL_STREAM_VIDEO;

  gavl_compression_info_copy(&h->ci, ci);
  gavl_video_format_copy(&h->format.video, format);
  if(h->ci.id == GAVL_CODEC_ID_NONE)
    gavl_video_format_set_frame_size(&h->format.video, 0, 0);

  return ph->num_streams-1;
  }

int gavf_program_header_add_overlay_stream(gavf_program_header_t * ph,
                                           const gavl_compression_info_t * ci,
                                           const gavl_video_format_t * format,
                                           const gavl_dictionary_t * m)
  {
  gavf_stream_header_t * h = add_stream(ph, m);
  h->type = GAVL_STREAM_OVERLAY;
  
  gavl_compression_info_copy(&h->ci, ci);
  gavl_video_format_copy(&h->format.video, format);

  /* Correct the video format */
  h->format.video.framerate_mode = GAVL_FRAMERATE_VARIABLE;
  
  if(h->ci.id == GAVL_CODEC_ID_NONE)
    gavl_video_format_set_frame_size(&h->format.video, 0, 0);

  return ph->num_streams-1;
  }

int gavf_program_header_add_text_stream(gavf_program_header_t * ph,
                                        uint32_t timescale,
                                        const gavl_dictionary_t * m)
  {
  gavf_stream_header_t * h = add_stream(ph, m);
  h->type = GAVL_STREAM_TEXT;
  gavl_dictionary_set_int(&h->m, GAVL_META_STREAM_SAMPLE_TIMESCALE, timescale);
  return ph->num_streams-1;
  }

int gavf_program_header_add_msg_stream(gavf_program_header_t * ph,
                                       const gavl_dictionary_t * m)
  {
  gavf_stream_header_t * h = add_stream(ph, m);
  h->type = GAVL_STREAM_MSG;
  gavl_dictionary_set_int(&h->m, GAVL_META_STREAM_SAMPLE_TIMESCALE, GAVL_TIME_SCALE);
  gavl_dictionary_set_int(&h->m, GAVL_META_STREAM_PACKET_TIMESCALE, GAVL_TIME_SCALE);
  return ph->num_streams-1;
  }

void gavf_program_header_free(gavf_program_header_t * ph)
  {
  int i;
  for(i = 0; i < ph->num_streams; i++)
    gavf_stream_header_free(&ph->streams[i]);

  if(ph->streams)
    free(ph->streams);
  gavl_dictionary_free(&ph->m);
  }

void gavf_program_header_dump(const gavf_program_header_t * ph)
  {
  int i;
  const gavf_stream_header_t * h;
  
  fprintf(stderr, "Program header\n");
  fprintf(stderr, "  Metadata\n");
  gavl_dictionary_dump(&ph->m, 4); fprintf(stderr, "\n");

  for(i = 0; i < ph->num_streams; i++)
    {
    h = &ph->streams[i];
    fprintf(stderr, "  Stream %d\n", i+1);
    gavf_stream_header_dump(h);
    }
  }

int
gavf_program_header_get_num_streams(const gavf_program_header_t * ph,
                                    int type)
  {
  int i;
  int ret = 0;

  for(i = 0; i < ph->num_streams; i++)
    {
    if(ph->streams[i].type == type)
      ret++;
    }
  return ret;
  }

const gavf_stream_header_t *
gavf_program_header_get_stream(const gavf_program_header_t * ph,
                               int index, int type)
  {
  int i;
  int idx = 0;
  
  for(i = 0; i < ph->num_streams; i++)
    {
    if(ph->streams[i].type == type)
      {
      if(idx == index)
        return &ph->streams[i];
      idx++;
      }
    }
  return NULL;
  }

void gavf_program_header_copy(gavf_program_header_t * dst,
                              const gavf_program_header_t * src)
  {
  int i;

  /* Copy metadata */
  gavl_dictionary_copy(&dst->m, &src->m);

  /* Copy streams */
  dst->num_streams = src->num_streams;
  dst->streams = malloc(dst->num_streams * sizeof(*dst->streams));
  
  for(i = 0; i < dst->num_streams; i++)
    {
    /* Copy status fields */
    memcpy(&dst->streams[i], &src->streams[i], sizeof(dst->streams[i]));

    /* Copy pointers */
    gavl_dictionary_init(&dst->streams[i].m);
    gavl_dictionary_copy(&dst->streams[i].m, &src->streams[i].m);
    
    gavl_compression_info_init(&dst->streams[i].ci);
    gavl_compression_info_copy(&dst->streams[i].ci, &src->streams[i].ci);
    }
  }
 
int gavf_program_header_get_duration(const gavf_program_header_t * ph,
                                     gavl_time_t * start_p,
                                     gavl_time_t * duration_p)
  {
  int i, ts;
  gavl_time_t start = GAVL_TIME_UNDEFINED;
  gavl_time_t end   = GAVL_TIME_UNDEFINED;
  gavl_time_t test;
  
  for(i = 0; i < ph->num_streams; i++)
    {
    if(ph->streams[i].stats.pts_start == GAVL_TIME_UNDEFINED)
      return 0;

    ts = gavf_stream_get_timescale(&ph->streams[i]);
    
    test = gavl_time_unscale(ts, ph->streams[i].stats.pts_start);
    if((start == GAVL_TIME_UNDEFINED) || (start > test))
      start = test;

    test = gavl_time_unscale(ts, ph->streams[i].stats.pts_end);
    if((start == GAVL_TIME_UNDEFINED) || (end < test))
      end = test;
    }
  
  if(start_p)
    *start_p = start;
  if(duration_p)
    *duration_p = end - start;
  return 1;
  }

void gavf_program_header_to_dictionary(const gavf_program_header_t * ph,
                                       gavl_dictionary_t * track)
  {
  gavl_dictionary_t * m_dst;
  gavl_dictionary_t * stream;
  int i;

  m_dst = gavl_dictionary_get_dictionary_create(track, GAVL_META_METADATA);
  gavl_dictionary_copy(m_dst, &ph->m);
  
  for(i = 0; i < ph->num_streams; i++)
    {
    stream = NULL;
    switch(ph->streams[i].type)
      {
      case GAVL_STREAM_AUDIO:
        stream = gavl_track_append_audio_stream(track);
        break;
      case GAVL_STREAM_VIDEO:
        stream = gavl_track_append_video_stream(track);
        break;
      case GAVL_STREAM_TEXT:
        stream = gavl_track_append_text_stream(track);
        break;
      case GAVL_STREAM_OVERLAY:
        stream = gavl_track_append_overlay_stream(track);
        break;
      case GAVL_STREAM_MSG:
        stream = gavl_track_append_msg_stream(track);
        break;
      case GAVL_STREAM_NONE:
        break;
      }
    if(!stream)
      continue;
    gavf_stream_header_to_dict(&ph->streams[i], stream);
    }
  }

void gavf_program_header_from_dictionary(gavf_program_header_t * ph,
                                         const gavl_dictionary_t * track)
  {
  int i;
  gavf_stream_header_t * sh;
  const gavl_dictionary_t * dict_src;
  const gavl_array_t * arr;

  if((dict_src = gavl_dictionary_get_dictionary(track, GAVL_META_METADATA)))
    gavl_dictionary_copy(&ph->m, dict_src);

  if((arr = gavl_dictionary_get_array(track, GAVL_META_STREAMS)))
    {
    for(i = 0; i < arr->num_entries; i++)
      {
      if((dict_src = gavl_value_get_dictionary(&arr->entries[i])))
        {
        sh = add_stream(ph, NULL);
        gavf_stream_header_from_dict(sh, dict_src);
        }
      }
    }
  }
