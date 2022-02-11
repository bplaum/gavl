#include <string.h>


#include <gavfprivate.h>
#include <gavl/metatags.h>



static void stats_to_dict(const gavl_stream_stats_t * s, gavl_dictionary_t * dict)
  {
  gavl_dictionary_set_int(dict, GAVL_META_STREAM_STATS_PACKET_SIZE_MIN, s->size_min);
  gavl_dictionary_set_int(dict, GAVL_META_STREAM_STATS_PACKET_SIZE_MAX, s->size_max);

  gavl_dictionary_set_long(dict, GAVL_META_STREAM_STATS_PACKET_DURATION_MIN, s->duration_min);
  gavl_dictionary_set_long(dict, GAVL_META_STREAM_STATS_PACKET_DURATION_MAX, s->duration_max);

  gavl_dictionary_set_long(dict, GAVL_META_STREAM_STATS_PTS_START, s->pts_start);
  gavl_dictionary_set_long(dict, GAVL_META_STREAM_STATS_PTS_END,   s->pts_end);

  gavl_dictionary_set_long(dict, GAVL_META_STREAM_STATS_NUM_PACKETS, s->total_packets);
  gavl_dictionary_set_long(dict, GAVL_META_STREAM_STATS_NUM_BYTES,   s->total_bytes);
  }

static int stats_from_dict(gavl_stream_stats_t * s, const gavl_dictionary_t * dict)
  {
  memset(s, 0, sizeof(*s));
  
  return gavl_dictionary_get_int(dict, GAVL_META_STREAM_STATS_PACKET_SIZE_MIN, &s->size_min) &&
    gavl_dictionary_get_int(dict, GAVL_META_STREAM_STATS_PACKET_SIZE_MAX, &s->size_max)  &&
    gavl_dictionary_get_long(dict, GAVL_META_STREAM_STATS_PACKET_DURATION_MIN, &s->duration_min) &&
    gavl_dictionary_get_long(dict, GAVL_META_STREAM_STATS_PACKET_DURATION_MAX, &s->duration_max) &&
    gavl_dictionary_get_long(dict, GAVL_META_STREAM_STATS_PTS_START, &s->pts_start) &&
    gavl_dictionary_get_long(dict, GAVL_META_STREAM_STATS_PTS_END,   &s->pts_end) &&
    gavl_dictionary_get_long(dict, GAVL_META_STREAM_STATS_NUM_PACKETS, &s->total_packets) &&
    gavl_dictionary_get_long(dict, GAVL_META_STREAM_STATS_NUM_BYTES,   &s->total_bytes);
  }

void gavl_stream_stats_dump(const gavl_stream_stats_t * stats, int indent)
  {
  gavl_dictionary_t dict;
  gavl_dictionary_init(&dict);
  stats_to_dict(stats, &dict);
  gavl_diprintf(indent, "Stats\n");
  gavl_dictionary_dump(&dict, indent + 2);
  gavl_dictionary_free(&dict);
  }

int gavl_stream_get_stats(const gavl_dictionary_t * s, gavl_stream_stats_t * stats)
  {
  const gavl_dictionary_t * stats_dict;
  if((stats_dict = gavl_dictionary_get_dictionary(s, GAVL_META_STREAM_STATS)) &&
     stats_from_dict(stats, stats_dict))
    return 1;
  else
    return 0;
  }

void gavl_stream_set_stats(gavl_dictionary_t * s, const gavl_stream_stats_t * stats)
  {
  gavl_value_t stats_val;
  gavl_dictionary_t * stats_dict;

  gavl_value_init(&stats_val);
  stats_dict = gavl_value_set_dictionary(&stats_val);
  stats_to_dict(stats, stats_dict);
  gavl_dictionary_set_nocopy(s, GAVL_META_STREAM_STATS, &stats_val);
  }


void gavl_stream_stats_init(gavl_stream_stats_t * f)
  {
  memset(f, 0, sizeof(*f));
  f->duration_min = GAVL_TIME_UNDEFINED;
  f->duration_max = GAVL_TIME_UNDEFINED;
  f->pts_start    = GAVL_TIME_UNDEFINED;
  f->pts_end      = GAVL_TIME_UNDEFINED;
  f->size_min     = -1;
  f->size_max     = -1;
  }
  
void gavl_stream_stats_update(gavl_stream_stats_t * f, const gavl_packet_t * p)
  {
  gavl_stream_stats_update_params(f, p->pts, p->duration, p->data_len,
                                  p->flags);
  }

void gavl_stream_stats_update_params(gavl_stream_stats_t * f,
                                     int64_t pts, int64_t duration, int data_len,
                                     int flags)
  {
  if(f->pts_start == GAVL_TIME_UNDEFINED)
    f->pts_start    = pts;

  if((duration > 0) && !(flags & GAVL_PACKET_NOOUTPUT))
    {
    if((f->pts_end == GAVL_TIME_UNDEFINED) || (f->pts_end < pts + duration))
      f->pts_end = pts + duration;

    if((f->duration_min == GAVL_TIME_UNDEFINED) ||
       (f->duration_min > duration))
      f->duration_min = duration;

    if((f->duration_max == GAVL_TIME_UNDEFINED) ||
       (f->duration_max < duration))
      f->duration_max = duration;
    }
  

  if(data_len > 0)
    {
    if((f->size_min < 0) || (f->size_min > data_len))
      f->size_min = data_len;
    
    if((f->size_max < 0) || (f->size_max < data_len))
      f->size_max = data_len;
    }
  
  if(!(flags & GAVL_PACKET_NOOUTPUT))
    f->total_packets++;
  
  if(data_len > 0)
    f->total_bytes += data_len;
  }

void gavl_stream_stats_apply_generic(gavl_stream_stats_t * f,
                                     gavl_compression_info_t * ci,
                                     gavl_dictionary_t * m)
  {
  if(ci && (ci->max_packet_size <= 0))
    ci->max_packet_size = f->size_max;

  if(f->pts_end > 0)
    {
    int64_t duration = f->pts_end;
    if((f->pts_start != GAVL_TIME_UNDEFINED) && (f->pts_start != 0))
      duration -= f->pts_start;
    gavl_dictionary_set_long(m, GAVL_META_STREAM_DURATION, duration);
    }
  
  }

static void calc_bitrate(gavl_stream_stats_t * f, int timescale,
                         gavl_compression_info_t * ci,
                         gavl_dictionary_t * m)
  {
  if((!ci || (ci->bitrate <= 0)) && (f->total_bytes > 0) && (f->pts_end > f->pts_start))
    {
    double avg_rate =
      (double)(f->total_bytes) / 
      (gavl_time_to_seconds(gavl_time_unscale(timescale,
                                              f->pts_end-f->pts_start)) * 125.0);
    gavl_dictionary_set_float(m, GAVL_META_AVG_BITRATE, avg_rate);
    }
  }
                         

void gavl_stream_stats_apply_audio(gavl_stream_stats_t * f, 
                                   const gavl_audio_format_t * fmt,
                                   gavl_compression_info_t * ci,
                                   gavl_dictionary_t * m)
  {
  if(fmt)
    calc_bitrate(f, fmt->samplerate, ci, m);
  gavl_stream_stats_apply_generic(f, ci, m);
  }

void gavl_stream_stats_apply_video(gavl_stream_stats_t * f, 
                                   gavl_video_format_t * fmt,
                                   gavl_compression_info_t * ci,
                                   gavl_dictionary_t * m)
  {
  if(fmt)
    calc_bitrate(f, fmt->timescale, ci, m);
  
  gavl_stream_stats_apply_generic(f, ci, m);
  
  if(fmt && (fmt->framerate_mode == GAVL_FRAMERATE_VARIABLE))
    {
    if((f->duration_min > 0) && (f->duration_min == f->duration_max))
      {
      /* Detect constant framerate */
      fmt->framerate_mode = GAVL_FRAMERATE_CONSTANT;
      fmt->frame_duration = f->duration_min;
      }
    else if((f->total_packets > 0) && (f->pts_end > f->pts_start))
      {
      double avg_rate =
        (double)f->total_packets / 
        gavl_time_to_seconds(gavl_time_unscale(fmt->timescale,
                                               f->pts_end-f->pts_start));
      
      gavl_dictionary_set_float(m, GAVL_META_AVG_FRAMERATE, avg_rate);
      }
    }
  }

void gavl_stream_stats_apply_subtitle(gavl_stream_stats_t * f, 
                                      gavl_dictionary_t * m)
  {
  gavl_stream_stats_apply_generic(f, NULL, m);
  }

void gavl_track_apply_footer(gavl_dictionary_t * track,
                             const gavl_dictionary_t * footer)
  {
  int i;
  int num_streams;

  gavl_dictionary_t * header_stream;
  const gavl_dictionary_t * footer_stream;
  
  num_streams = gavl_track_get_num_streams_all(track);

  if(gavl_track_get_num_streams_all(footer) != num_streams)
    {
    fprintf(stderr, "gavl_track_apply_footer: Stream counts don't match\n");
    return;
    }

  for(i = 0; i < num_streams; i++)
    {
    header_stream = gavl_track_get_stream_all_nc(track, i);
    footer_stream = gavl_track_get_stream_all(footer, i);

    /* This just adds the stats member to the header */
    gavl_dictionary_merge2(header_stream, footer_stream);
    }
  
  }
                             
