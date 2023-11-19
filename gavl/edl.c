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

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/metadata.h>
#include <gavl/metatags.h>
#include <gavl/trackinfo.h>

#include <gavl/edl.h>
#include <gavl/utils.h>
#include <gavl/gavf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

gavl_dictionary_t * gavl_edl_create(gavl_dictionary_t * parent)

  {
  return gavl_dictionary_get_dictionary_create(parent, GAVL_META_EDL);
  }

gavl_dictionary_t * gavl_edl_add_segment(gavl_dictionary_t * stream)
  {
  gavl_array_t * segs;
  gavl_value_t val;
  gavl_dictionary_t * ret;

  gavl_value_init(&val);
  
  if(!(segs = gavl_dictionary_get_array_nc(stream, GAVL_EDL_SEGMENTS)))
    {
    segs = gavl_value_set_array(&val);
    gavl_dictionary_set_nocopy(stream, GAVL_EDL_SEGMENTS, &val);
    }

  ret = gavl_value_set_dictionary(&val);
  gavl_array_splice_val_nocopy(segs, segs->num_entries, 0, &val);
  return ret;
  }

void gavl_edl_segment_set_url(gavl_edl_segment_t * seg, const char * url)
  {
  gavl_dictionary_set_string(seg, GAVL_META_URI, url);
  }

void gavl_edl_segment_set_speed(gavl_edl_segment_t * seg, int num, int den)
  {
  if((num != 1) || (den != 1))
    {
    gavl_dictionary_set_int(seg,  GAVL_EDL_SPEED_NUM, num);
    gavl_dictionary_set_int(seg,  GAVL_EDL_SPEED_DEN, den);
    }
  }
  
void gavl_edl_segment_set(gavl_edl_segment_t * seg,
                          int track,
                          int stream,
                          int timescale,
                          int64_t src_time,
                          int64_t dst_time,
                          int64_t dst_duration)
  {
  gavl_dictionary_set_int(seg,  GAVL_EDL_TRACK_IDX,  track);
  gavl_dictionary_set_int(seg,  GAVL_EDL_STREAM_IDX, stream);
  gavl_dictionary_set_int(seg,  GAVL_META_STREAM_SAMPLE_TIMESCALE,  timescale);
  gavl_dictionary_set_long(seg, GAVL_EDL_SRC_TIME,   src_time);
  gavl_dictionary_set_long(seg, GAVL_EDL_DST_TIME,   dst_time);
  gavl_dictionary_set_long(seg, GAVL_EDL_DST_DUR,    dst_duration);
  }

const char * gavl_edl_segment_get_url(const gavl_edl_segment_t * seg) 
  {
  return gavl_dictionary_get_string(seg, GAVL_META_URI);
  
  }

void gavl_edl_segment_get_speed(const gavl_edl_segment_t * seg, int * num, int * den)
  {
  if(!gavl_dictionary_get_int(seg,  GAVL_EDL_SPEED_NUM, num))
    *num = 1;
  
  if(!gavl_dictionary_get_int(seg,  GAVL_EDL_SPEED_DEN, den))
    *den = 1;
  }

int gavl_edl_segment_get(const gavl_edl_segment_t * seg,
                         int * track_p,
                         int * stream_p,
                         int * timescale_p,
                         int64_t * src_time_p,
                         int64_t * dst_time_p,
                         int64_t * dst_duration_p)
  {
  int track = -1;
  int stream = -1;
  int timescale = -1;
  int64_t src_time = GAVL_TIME_UNDEFINED;
  int64_t dst_time = GAVL_TIME_UNDEFINED;
  int64_t dst_duration = -1;
  
  if(!(gavl_dictionary_get_int(seg,  GAVL_EDL_TRACK_IDX,  &track)) ||
     !(gavl_dictionary_get_int(seg,  GAVL_EDL_STREAM_IDX, &stream)) ||
     !(gavl_dictionary_get_int(seg,  GAVL_META_STREAM_SAMPLE_TIMESCALE,  &timescale)) ||
     !(gavl_dictionary_get_long(seg, GAVL_EDL_SRC_TIME,   &src_time)) ||
     !(gavl_dictionary_get_long(seg, GAVL_EDL_DST_TIME,   &dst_time)) ||
     !(gavl_dictionary_get_long(seg, GAVL_EDL_DST_DUR,    &dst_duration)))
    return 0;
  
  /* Santiy checks */
  if((track < 0) ||
     (stream < 0) ||
     (timescale <= 0) ||
     (src_time == GAVL_TIME_UNDEFINED) ||
     (dst_time == GAVL_TIME_UNDEFINED) ||
     (dst_duration <= 0))
    return 0;

  if(track_p)
    *track_p = track;
  if(stream_p)
    *stream_p = stream;
  if(timescale_p)
    *timescale_p = timescale;
  if(src_time_p)
    *src_time_p = src_time;
  if(dst_time_p)
    *dst_time_p = dst_time;
  if(dst_duration_p)
    *dst_duration_p = dst_duration;
  
  return 1;
  }

typedef struct
  {
  const char * uri;
  gavl_array_t * streams;
  gavl_array_t * tracks;
  gavl_array_t * segments;

  gavl_dictionary_t * track;
  gavl_dictionary_t * stream;
  int error;
  } finalize_t;

static void finalize_segment(void * priv, 
                             int idx,
                             const gavl_value_t * v_c)
  {
  
  gavl_value_t * v;
  gavl_dictionary_t * seg;
  finalize_t * f = priv;
  
  v = gavl_array_get_nc(f->segments, idx);
  seg = gavl_value_get_dictionary_nc(v);

  if(!gavl_edl_segment_get(seg, NULL, NULL, NULL, NULL, NULL, NULL))
    {
    f->error = 1;
    return;
    }
  if(!gavl_dictionary_get_string(seg, GAVL_META_URI) && f->uri)
    {
    gavl_dictionary_set_string(seg, GAVL_META_URI, f->uri);
    }
  
  }

static void finalize_stream(void * priv, 
                            int idx,
                            const gavl_value_t * v_c)
  {
  int track;
  int stream;
  int timescale;
  int64_t src_time;
  int64_t dst_time;
  int64_t dst_duration;
  
  gavl_value_t * v;
  //  gavl_array_t * arr;
  gavl_dictionary_t * seg;

  gavl_stream_stats_t stats;

  finalize_t * finalize = priv;
  
  v = gavl_array_get_nc(finalize->streams, idx);
  if(!(finalize->stream = gavl_value_get_dictionary_nc(v)) ||
     !(finalize->segments = gavl_dictionary_get_array_nc(finalize->stream, GAVL_EDL_SEGMENTS)))
    {
    finalize->error = 1;
    return;
    }
  gavl_array_foreach(finalize->segments, finalize_segment, finalize);

  if(finalize->error)
    return;
  
  gavl_stream_stats_init(&stats);
  
  if(!(v = gavl_array_get_nc(finalize->segments, 0)) ||
     !(seg = gavl_value_get_dictionary_nc(v)) ||
     !gavl_edl_segment_get(seg, &track, &stream, &timescale, &src_time, &dst_time, &dst_duration))
    {
    finalize->error = 1;
    return;
    }
  
  stats.pts_start = dst_time;

  if(!(v = gavl_array_get_nc(finalize->segments, finalize->segments->num_entries-1)) ||
     !(seg = gavl_value_get_dictionary_nc(v)) ||
     !gavl_edl_segment_get(seg, &track, &stream, &timescale, &src_time, &dst_time, &dst_duration))
    {
    finalize->error = 1;
    return;
    }
  stats.pts_end = dst_time + dst_duration;

  gavl_stream_stats_apply_generic(&stats, NULL, gavl_stream_get_metadata_nc(finalize->stream));
  
  }

static void finalize_track_internal(gavl_dictionary_t * t, finalize_t * f)
  {
  gavl_dictionary_t * m;
  
  if((f->streams = gavl_dictionary_get_array_nc(t, GAVL_META_STREAMS)))
    gavl_array_foreach(f->streams, finalize_stream, f);

  if(f->error)
    return;
  
  if((m = gavl_track_get_metadata_nc(t)))
    gavl_dictionary_set_long(m, GAVL_META_APPROX_DURATION, 0);
  
  gavl_track_finalize(t);
  
  //  gavl_track_compute_duration(t);
  }

static void finalize_track(void * priv, int idx, const gavl_value_t * v_c)
  {
  gavl_value_t * v;
  gavl_array_t * arr;
  gavl_dictionary_t * t;
  finalize_t * finalize = priv;
  
  arr = finalize->tracks;
  
  v = gavl_array_get_nc(arr, idx);
  if(!(t = gavl_value_get_dictionary_nc(v)))
    {
    finalize->error = 1;
    return;
    }
  finalize_track_internal(t, finalize);
  }

int gavl_edl_finalize(gavl_dictionary_t * edl)
  {
  finalize_t finalize;
  memset(&finalize, 0, sizeof(finalize));
  finalize.uri = gavl_dictionary_get_string(edl, GAVL_META_URI);
  finalize.tracks = gavl_dictionary_get_array_nc(edl, GAVL_META_CHILDREN);
  
  if(finalize.tracks)
    gavl_array_foreach(finalize.tracks, finalize_track, &finalize);
  else
    finalize.error = 1;
  
  /* We don't need the global URI anymore */
  gavl_dictionary_set(edl, GAVL_META_URI, NULL);

  gavl_track_update_children(edl);
  
  if(finalize.error)
    return 0;
  else
    return 1;
  }

static void add_stream_to_timeline(gavl_dictionary_t * edl_stream,
                                   const gavl_dictionary_t * stream, int idx,
                                   gavl_time_t edl_duration, gavl_time_t track_duration,
                                   const char * uri)
  {
  int timescale_track = 0;
  int timescale_edl = 0;
  int64_t pts_start = 0;
  int64_t pts_end;
  
  const gavl_dictionary_t * m_track;
  const gavl_dictionary_t * m_edl;
  
  gavl_dictionary_t * seg = gavl_edl_add_segment(edl_stream);

  m_track  = gavl_stream_get_metadata(stream);
  m_edl  = gavl_stream_get_metadata(edl_stream);
  
  gavl_dictionary_get_int(m_track, GAVL_META_STREAM_SAMPLE_TIMESCALE, &timescale_track);
  gavl_dictionary_get_int(m_edl, GAVL_META_STREAM_SAMPLE_TIMESCALE, &timescale_edl);

  if(!gavl_stream_get_pts_range(stream, &pts_start, &pts_end))
    {
    pts_start = 0;
    pts_end = gavl_time_scale(timescale_track, track_duration);
    }
  
  /*  
  void gavl_edl_segment_set(gavl_edl_segment_t * seg,
                          int track,
                          int stream,
                          int timescale,
                          int64_t src_time,
                          int64_t dst_time,
                          int64_t dst_duration)
  */
  
  gavl_edl_segment_set(seg,
                       0 /* track */, 
                       idx  /* stream */,
                       timescale_track,
                       pts_start, /* src_time */
                       gavl_time_scale(timescale_edl, edl_duration) +
                       gavl_time_rescale(timescale_track, timescale_edl, pts_start), // dst_time
                       gavl_time_rescale(timescale_track, timescale_edl, pts_end - pts_start)  // dst_duration
                       );
  gavl_edl_segment_set_url(seg, uri);
  }

static int add_streams_to_timeline(gavl_dictionary_t * edl_track,
                                   const gavl_dictionary_t * track,
                                   gavl_stream_type_t type, int64_t edl_duration)
  {
  int ret = 0;
  int i, num;
  
  gavl_dictionary_t * s_edl;          // Stream (edl, dst)
  const gavl_dictionary_t * s_track;  // Stream (track, src)
  const char * location = NULL;
  const gavl_dictionary_t * m;
  gavl_time_t track_duration;
  
  m = gavl_track_get_metadata(track);
  
  if(!gavl_metadata_get_src(m, GAVL_META_SRC, 0, NULL, &location))
    return 0;
  
  if(!gavl_dictionary_get_long(m, GAVL_META_APPROX_DURATION, &track_duration))
    return 0;

  num = gavl_track_get_num_streams(edl_track, type);
  
  for(i = 0; i < num; i++)
    {
    s_edl = gavl_track_get_stream_nc(edl_track, type, i);

    if(!(s_track = gavl_track_get_stream(track, type, i)))
      break;
    
    add_stream_to_timeline(s_edl, s_track, i, edl_duration, track_duration, location);
    ret++;
    }
  return ret;
  }

static void add_external_streams_to_timeline(gavl_dictionary_t * edl_track,
                                             const gavl_dictionary_t * track,
                                             int64_t edl_duration, int start_idx)
  {
  int i, num;
  
  gavl_dictionary_t * s_edl;          // Stream (edl, dst)
  const gavl_dictionary_t * s_track;  // Stream (track, src)
  const char * location = NULL;
  const gavl_dictionary_t * m;
  const gavl_dictionary_t * sm;
  gavl_time_t track_duration;
  
  m = gavl_track_get_metadata(track);
  
  if(!gavl_dictionary_get_long(m, GAVL_META_APPROX_DURATION, &track_duration))
    return;

  num = gavl_track_get_num_external_streams(track);
  
  for(i = 0; i < num; i++)
    {
    s_edl = gavl_track_get_stream_all_nc(edl_track, i + start_idx);

    if(!(s_track = gavl_track_get_external_stream(track, i)) ||
       !(sm = gavl_stream_get_metadata(s_track)) ||
       !gavl_metadata_get_src(sm, GAVL_META_SRC, 0, NULL, &location))
      return;
    
    add_stream_to_timeline(s_edl, s_track, 0, edl_duration, track_duration, location);
    }
  }

static const char * clear_metadata_fields[] =
  {
    GAVL_META_SRC,
    GAVL_META_LABEL,
    GAVL_META_SOFTWARE,
    NULL
  };

static const char * clear_stream_metadata_fields[] =
  {
    GAVL_META_LABEL,
    GAVL_META_STREAM_PACKET_TIMESCALE,
    GAVL_META_FORMAT,
    GAVL_META_BITRATE,
    GAVL_META_AVG_BITRATE,
    GAVL_META_SOFTWARE,
    GAVL_META_SRC,
    NULL
  };

static void stream_clear_foreach_func(void * priv, int idx, const gavl_value_t * val)
  {
  gavl_dictionary_t * s;
  gavl_dictionary_t * sm;
  
  gavl_array_t * arr = priv;
  
  if((s = gavl_value_get_dictionary_nc(&arr->entries[idx])) &&
     (sm = gavl_stream_get_metadata_nc(s)))
    {
    gavl_dictionary_delete_fields(sm, clear_stream_metadata_fields);
    }
  
  }

static void add_stream_func(void * priv, int idx, const gavl_value_t * val)
  {
  const gavl_dictionary_t * src;
  gavl_stream_type_t type;
  gavl_dictionary_t * track = priv;
  
  if((src = gavl_value_get_dictionary(val)))
    {
    gavl_dictionary_t * dst;

    type = gavl_stream_get_type(src);
    switch(type)
      {
      case GAVL_STREAM_AUDIO:
      case GAVL_STREAM_VIDEO:
      case GAVL_STREAM_TEXT:
      case GAVL_STREAM_OVERLAY:
        dst = gavl_track_append_stream(track, gavl_stream_get_type(src));
        gavl_dictionary_reset(dst);
        gavl_dictionary_copy(dst, src);
        break;
      case GAVL_STREAM_MSG:
      case GAVL_STREAM_NONE:
        break;
      }
    }
  }


void gavl_edl_append_track_to_timeline(gavl_dictionary_t * edl_track,
                                       const gavl_dictionary_t * track, int init)
  {
  gavl_time_t edl_duration;
  const gavl_dictionary_t * m;
  gavl_dictionary_t * edl_m;
  const char * klass;
  finalize_t f;
  int idx;
  memset(&f, 0, sizeof(f));

  if(!(m = gavl_track_get_metadata(track)))
    return;
  
  if(init)
    {
    const gavl_array_t * track_streams;
    gavl_array_t * edl_streams;
    edl_duration = 0;
    gavl_dictionary_reset(edl_track);
    //  gavl_dictionary_copy(edl_track, track);
    
    /* Clear metadata (remove src, clear durations etc) */
    edl_m = gavl_dictionary_get_dictionary_create(edl_track, GAVL_META_METADATA);
    gavl_dictionary_copy(edl_m, m);
    
    if((klass = gavl_dictionary_get_string(edl_m, GAVL_META_MEDIA_CLASS)))
      {
      if(!strcmp(klass, GAVL_META_MEDIA_CLASS_MOVIE_PART))
        gavl_dictionary_set_string(edl_m, GAVL_META_MEDIA_CLASS,
                                   GAVL_META_MEDIA_CLASS_MOVIE);
      }
    
    gavl_dictionary_delete_fields(edl_m, clear_metadata_fields);
    gavl_dictionary_set_long(edl_m, GAVL_META_APPROX_DURATION, 0);

    /* Copy internal streams */
    
    if((track_streams = gavl_dictionary_get_array(track, GAVL_META_STREAMS)))
      {
      gavl_array_foreach(track_streams, add_stream_func, edl_track);
      }
    
    /* Copy external streams */

    if((track_streams = gavl_dictionary_get_array(track, GAVL_META_STREAMS_EXT)))
      {
      //      gavl_dictionary_set_array(edl_track, GAVL_META_STREAMS, arr);
      gavl_array_foreach(track_streams, add_stream_func, edl_track);
      }
    
    edl_streams = gavl_dictionary_get_array_nc(edl_track, GAVL_META_STREAMS);
    gavl_array_foreach(edl_streams, stream_clear_foreach_func, edl_streams);
    
    }
  else
    {
    edl_m = gavl_dictionary_get_dictionary_nc(edl_track, GAVL_META_METADATA);
    gavl_dictionary_get_long(edl_m, GAVL_META_APPROX_DURATION, &edl_duration);
    }
  
  idx = add_streams_to_timeline(edl_track, track, GAVL_STREAM_AUDIO,   edl_duration);
  idx += add_streams_to_timeline(edl_track, track, GAVL_STREAM_VIDEO,   edl_duration);
  idx += add_streams_to_timeline(edl_track, track, GAVL_STREAM_TEXT,    edl_duration);
  idx += add_streams_to_timeline(edl_track, track, GAVL_STREAM_OVERLAY, edl_duration);

  add_external_streams_to_timeline(edl_track, track, edl_duration, idx);
  
  finalize_track_internal(edl_track, &f);
  }

