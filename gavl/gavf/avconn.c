#include <string.h>

#include <gavfprivate.h>

/*
 *  Utility
 */

void gavf_shrink_audio_frame(gavl_audio_frame_t * f, 
                             gavl_packet_t * p, 
                             const gavl_audio_format_t * format)
  {
  int bytes_per_sample, i;
  if(f->valid_samples == format->samples_per_frame)
    return;

  bytes_per_sample = gavl_bytes_per_sample(format->sample_format);
  switch(format->interleave_mode)
    {
    case GAVL_INTERLEAVE_ALL:
      break;
    case GAVL_INTERLEAVE_NONE:
      for(i = 1; i < format->num_channels; i++)
        memmove(p->data+bytes_per_sample*i*f->valid_samples, 
                p->data+bytes_per_sample*i*format->samples_per_frame,
                bytes_per_sample * f->valid_samples);
      break;
    case GAVL_INTERLEAVE_2:
      for(i = 2; i < format->num_channels; i += 2)
        memmove(p->data+bytes_per_sample*i*f->valid_samples,
                p->data+bytes_per_sample*i*format->samples_per_frame,
                2 * bytes_per_sample * f->valid_samples);
      if(format->num_channels % 2)
        memmove(p->data+bytes_per_sample*(format->num_channels-1)*f->valid_samples,    
                p->data+bytes_per_sample*(format->num_channels-1)*format->samples_per_frame,
                bytes_per_sample * f->valid_samples);
      break;
    }
  p->data_len = f->valid_samples * format->num_channels * bytes_per_sample;
  }


/*
 * Audio source
 */

static gavl_source_status_t
read_audio_func(void * priv, gavl_audio_frame_t ** frame)
  {
  gavl_packet_t * p = NULL;
  gavl_source_status_t st;
  gavf_stream_t * s = priv;
  
  if((st = gavl_packet_source_read_packet(s->psrc, &p)) != GAVL_SOURCE_OK)
    return st;

  if(!s->aframe)
    s->aframe = gavl_audio_frame_create(NULL);

  gavf_packet_to_audio_frame(p, s->aframe, s->afmt, s->m, &s->dsp);
  *frame = s->aframe;
  return GAVL_SOURCE_OK;
  }

gavl_audio_source_t *
gavf_get_audio_source(gavf_t * g, uint32_t id)
  {
  gavf_stream_t * s;

  if(GAVF_HAS_FLAG(g, GAVF_FLAG_WRITE))
    return NULL;
  
  if(!(s = gavf_find_stream_by_id(g, id)))
    return NULL;
  
  if(s->ci.id != GAVL_CODEC_ID_NONE)
    return NULL;

  if(!s->asrc)
    s->asrc = gavl_audio_source_create(read_audio_func, s,
                                       GAVL_SOURCE_SRC_ALLOC,
                                       s->afmt);
  
  return s->asrc;
  }

/*
 * Video source
 */

static gavl_source_status_t
read_video_func(void * priv, gavl_video_frame_t ** frame)
  {
  gavl_packet_t * p = NULL;
  gavl_source_status_t st;
  gavf_stream_t * s = priv;

  if((st = gavl_packet_source_read_packet(s->psrc, &p)) != GAVL_SOURCE_OK)
    return st;

  if(!s->vframe)
    s->vframe = gavl_video_frame_create(NULL);
  
  gavf_packet_to_video_frame(p, s->vframe, s->vfmt, s->m, &s->dsp);
  *frame = s->vframe;
  return GAVL_SOURCE_OK;
  }

static gavl_source_status_t
read_overlay_func(void * priv, gavl_video_frame_t ** frame)
  {
  gavl_packet_t * p = NULL;
  gavl_source_status_t st;
  gavf_stream_t * s = priv;
  
  if((st = gavl_packet_source_read_packet(s->psrc, &p)) != GAVL_SOURCE_OK)
    return st;
  gavf_packet_to_overlay(p, *frame, s->vfmt);
  return GAVL_SOURCE_OK;
  }

gavl_video_source_t *
gavf_get_video_source(gavf_t * g, uint32_t id)
  {
  gavf_stream_t * s;
  if(GAVF_HAS_FLAG(g, GAVF_FLAG_WRITE))
    return NULL;

  if(!(s = gavf_find_stream_by_id(g, id)))
    return NULL;

  if(s->ci.id != GAVL_CODEC_ID_NONE)
    return NULL;

  if(!s->vsrc)
    {
    if(s->type == GAVL_STREAM_VIDEO)
      s->vsrc = gavl_video_source_create(read_video_func, s,
                                         GAVL_SOURCE_SRC_ALLOC,
                                         s->vfmt);
    else if(s->type == GAVL_STREAM_OVERLAY)
      s->vsrc = gavl_video_source_create(read_overlay_func, s,
                                         0, s->vfmt);
    }
  return s->vsrc;
  }

/*
 * Audio sink
 */

static gavl_audio_frame_t *
get_audio_func(void * priv)
  {
  gavf_stream_t * s = priv;

  s->p = gavl_packet_sink_get_packet(s->psink);
  
  gavl_packet_reset(s->p);
  gavl_packet_alloc(s->p, s->ci.max_packet_size);

  if(!s->aframe)
    s->aframe = gavl_audio_frame_create(NULL);
  
  s->aframe->valid_samples = s->afmt->samples_per_frame;
  
  gavl_audio_frame_set_channels(s->aframe,
                                s->afmt, s->p->data);
  s->aframe->valid_samples = 0;
  return s->aframe;
  }

static gavl_sink_status_t
put_audio_func(void * priv, gavl_audio_frame_t * frame)
  {
  gavl_sink_status_t st;
  gavf_stream_t * s = priv;
  
  gavf_audio_frame_to_packet_metadata(s->aframe, s->p);
  s->p->data_len = s->ci.max_packet_size;
  
  gavf_shrink_audio_frame(s->aframe, s->p, s->afmt);

  st = gavl_packet_sink_put_packet(s->psink, s->p);
  s->p = NULL;
  return st;
  }

gavl_audio_sink_t *
gavf_get_audio_sink(gavf_t * g, uint32_t id)
  {
  gavf_stream_t * s;
  if(GAVF_HAS_FLAG(g, GAVF_FLAG_WRITE))
    return NULL;

  if(!(s = gavf_find_stream_by_id(g, id)))
    return NULL;
  
  if(s->ci.id != GAVL_CODEC_ID_NONE)
    return NULL;

  if(!s->asink)
    s->asink = gavl_audio_sink_create(get_audio_func,
                                      put_audio_func, s,
                                      s->afmt);
  
  return s->asink;
  }

/*
 * Video sink
 */

static gavl_video_frame_t *
get_video_func(void * priv)
  {
  gavf_stream_t * s = priv;

  s->p = gavl_packet_sink_get_packet(s->psink);
  gavl_packet_reset(s->p);
  gavl_packet_alloc(s->p, s->ci.max_packet_size);

  if(!s->vframe)
    s->vframe = gavl_video_frame_create(NULL);
  s->vframe->strides[0] = 0;
  gavl_video_frame_set_planes(s->vframe,
                              s->vfmt, s->p->data);
  return s->vframe;
  }

static gavl_sink_status_t
put_video_func(void * priv, gavl_video_frame_t * frame)
  {
  gavl_sink_status_t st;
  gavf_stream_t * s = priv;
  gavf_video_frame_to_packet_metadata(frame, s->p);
  s->p->data_len = s->ci.max_packet_size;
  st = gavl_packet_sink_put_packet(s->psink, s->p);
  s->p = NULL;
  return st;
  }

static gavl_sink_status_t
put_overlay_func(void * priv, gavl_video_frame_t * frame)
  {
  gavl_sink_status_t st;
  gavf_stream_t * s = priv;

  s->p = gavl_packet_sink_get_packet(s->psink);
  gavl_packet_reset(s->p);
  gavl_packet_alloc(s->p, s->ci.max_packet_size);
  gavf_overlay_to_packet(frame, s->p, s->vfmt);
  st = gavl_packet_sink_put_packet(s->psink, s->p);
  s->p = NULL;
  return st;
  }

gavl_video_sink_t *
gavf_get_video_sink(gavf_t * g, uint32_t id)
  {
  gavf_stream_t * s;
  if(!GAVF_HAS_FLAG(g, GAVF_FLAG_WRITE))
    return NULL;
  
  if(!(s = gavf_find_stream_by_id(g, id)))
    return NULL;
  if(s->ci.id != GAVL_CODEC_ID_NONE)
    return NULL;

  if(!s->vsink)
    {
    if(s->type == GAVL_STREAM_VIDEO)
      s->vsink = gavl_video_sink_create(get_video_func,
                                        put_video_func, s,
                                        s->vfmt);
    else if(s->type == GAVL_STREAM_OVERLAY)
      s->vsink = gavl_video_sink_create(NULL,
                                        put_overlay_func, s,
                                        s->vfmt);
    }
  
  return s->vsink;
  }
