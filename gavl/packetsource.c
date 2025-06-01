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
#include <stdio.h>
#include <pthread.h>

#include <config.h>

#include <gavl/connectors.h>
#include <gavl/trackinfo.h>
#include <gavl/metatags.h>
#include <gavl/log.h>

#define LOG_DOMAIN "packetsource"

// #define FLAG_HAS_AFMT         (1<<0)
// #define FLAG_HAS_VFMT         (1<<1)
#define FLAG_HAS_CI           (1<<2)

struct gavl_packet_source_s
  {
  const gavl_dictionary_t * s; // Stream info (might be a pointer to previous source)
  
  gavl_dictionary_t s_priv;    // Private (if different from previous source)
  
  //  gavl_audio_format_t audio_format;
  //  gavl_video_format_t video_format;
  //  gavl_compression_info_t ci;
  //  int timescale;
  
  gavl_packet_t p;
  
  gavl_packet_t * out_packet; // Saved here by gavl_packet_source_peek_packet_read
  
  int src_flags;
  
  gavl_packet_source_func_t func;
  void * priv;
  int flags;

  gavl_connector_lock_func_t lock_func;
  gavl_connector_lock_func_t unlock_func;
  void * lock_priv;
  gavl_connector_free_func_t free_func;

  int64_t pts_offset;
  
  };

gavl_packet_source_t *
gavl_packet_source_create(gavl_packet_source_func_t func,
                          void * priv, int src_flags, const gavl_dictionary_t * s)
  {
  gavl_packet_source_t * ret;
  ret = calloc(1, sizeof(*ret));
  ret->func = func;
  ret->priv = priv;
  ret->src_flags = src_flags;

  if(s)
    ret->s = s;
  else
    ret->s = &ret->s_priv;
  
  return ret;
  }

void gavl_packet_source_set_pts_offset(gavl_packet_source_t * src, int64_t offset)
  {
  src->pts_offset = offset;
  }

#if 0
gavl_packet_source_t *
gavl_packet_source_create_audio(gavl_packet_source_func_t func,
                                void * priv, int src_flags,
                                const gavl_compression_info_t * ci,
                                const gavl_audio_format_t * afmt)
  {
  gavl_packet_source_t * ret = gavl_packet_source_create(func, priv, src_flags, NULL);
  
  gavl_compression_info_copy(&ret->ci, ci);
  ret->flags |= FLAG_HAS_CI;

  gavl_init_audio_stream(&ret->s);
  gavl_audio_format_copy(gavl_stream_get_audio_format_nc(&ret->s), afmt);
  
  return ret;
  }

gavl_packet_source_t *
gavl_packet_source_create_video(gavl_packet_source_func_t func,
                                void * priv, int src_flags,
                                const gavl_compression_info_t * ci,
                                const gavl_video_format_t * vfmt,
                                gavl_stream_type_t type)
  {
  gavl_packet_source_t * ret = gavl_packet_source_create(func, priv, src_flags, NULL);

  if(type == GAVL_STREAM_VIDEO)
    {
    gavl_init_video_stream(&ret->s);
    }
  else if(type == GAVL_STREAM_OVERLAY)
    {
    gavl_init_overlay_stream(&ret->s);
    }
  
  gavl_compression_info_copy(&ret->ci, ci);
  ret->flags |= FLAG_HAS_CI;
  
  gavl_video_format_copy(gavl_stream_get_video_format_nc(&ret->s), vfmt);

  return ret;
  }

gavl_packet_source_t *
gavl_packet_source_create_text(gavl_packet_source_func_t func,
                               void * priv, int src_flags,
                               int timescale)
  {
  gavl_packet_source_t * ret = gavl_packet_source_create(func, priv, src_flags, NULL);
  
  ret->timescale = timescale;
  return ret;
  }
#endif

gavl_packet_source_t *
gavl_packet_source_create_source(gavl_packet_source_func_t func,
                                 void * priv, int src_flags,
                                 gavl_packet_source_t * src)
  {
  gavl_packet_source_t * ret = gavl_packet_source_create(func, priv, src_flags, src->s);
  ret->flags = src->flags;
  return ret;
  }

void
gavl_packet_source_set_lock_funcs(gavl_packet_source_t * src,
                                  gavl_connector_lock_func_t lock_func,
                                  gavl_connector_lock_func_t unlock_func,
                                  void * priv)
  {
  src->lock_func = lock_func;
  src->unlock_func = unlock_func;
  src->lock_priv = priv;
  }

#if 0
const gavl_compression_info_t *
gavl_packet_source_get_ci(gavl_packet_source_t * s)
  {
  if(!(s->flags & FLAG_HAS_CI) && !(gavl_stream_get_compression_info(&s->s, &s->ci)))
    return NULL;
  else
    return &s->ci;
  }
#endif

const gavl_dictionary_t *
gavl_packet_source_get_stream(gavl_packet_source_t * s)
  {
  return s->s;
  }

gavl_dictionary_t *
gavl_packet_source_get_stream_nc(gavl_packet_source_t * s)
  {
  if(s->s != &s->s_priv)
    {
    /* Create local copy */
    gavl_dictionary_copy(&s->s_priv, s->s);
    s->s = &s->s_priv;
    }
  return &s->s_priv;
  }

const gavl_audio_format_t *
gavl_packet_source_get_audio_format(gavl_packet_source_t * s)
  {
  return gavl_stream_get_audio_format(s->s);
  }

const gavl_video_format_t *
gavl_packet_source_get_video_format(gavl_packet_source_t * s)
  {
  return gavl_stream_get_video_format(s->s);
  }

int
gavl_packet_source_get_timescale(gavl_packet_source_t * s)
  {
  int ret = 0;
  const gavl_dictionary_t * m = gavl_stream_get_metadata(s->s);
  gavl_dictionary_get_int(m, GAVL_META_STREAM_PACKET_TIMESCALE, &ret);
  return ret;
  }


gavl_source_status_t
gavl_packet_source_read_packet(void*sp, gavl_packet_t ** p)
  {
  gavl_source_status_t st;
  gavl_packet_t *p_src;
  
  gavl_packet_source_t * s = sp;
  
  if(s->out_packet)
    {
    if(*p && (*p != s->out_packet))
      gavl_packet_copy(*p, s->out_packet);
    else
      *p = s->out_packet;
    
    s->out_packet = NULL;
    return GAVL_SOURCE_OK;
    }
  
  gavl_packet_reset(&s->p);

  /* Decide source */
  if(s->src_flags & GAVL_SOURCE_SRC_ALLOC)
    p_src = NULL;
  else
    p_src = &s->p;
  

  if(s->lock_func)
    s->lock_func(s->lock_priv);

  /* Get packet */
  st = s->func(s->priv, &p_src);

  if(s->unlock_func)
    s->unlock_func(s->lock_priv);

  if(st != GAVL_SOURCE_OK)
    return st;
  
  /* Kick out error packets */
#if 0 // negative durations are o longer considered errors
  if(p_src->duration < 0)
    {
    //    gavl_log(GAVL_LOG_WARNING, LOG_DOMAIN, "Negative packet duration encountered");
    //  gavl_packet_dump(p_src);
    // return GAVL_SOURCE_EOF;
    }
#endif
  
  /* Decide destination */
  if(!*p)
    *p = p_src;
  else
    gavl_packet_copy(*p, p_src);
  
  return GAVL_SOURCE_OK;
  }

gavl_source_status_t
gavl_packet_source_peek_packet(void*sp, gavl_packet_t ** p)
  {
  gavl_source_status_t st;
  gavl_packet_source_t * s = sp;

  if(!s->out_packet)
    {
    if((st = gavl_packet_source_read_packet(sp, &s->out_packet)) != GAVL_SOURCE_OK)
      return st;
    }

  if(p)
    {
    if(*p)
      gavl_packet_copy(*p, s->out_packet);
    else
      *p = s->out_packet;
    }
  
  return GAVL_SOURCE_OK;
  }

void gavl_packet_source_drain(gavl_packet_source_t * s)
  {
  gavl_source_status_t st;

  gavl_packet_t * p = NULL;
  while((st = gavl_packet_source_read_packet(s, &p)) == GAVL_SOURCE_OK)
    p = NULL;
  }


void
gavl_packet_source_set_free_func(gavl_packet_source_t * src,
                                 gavl_connector_free_func_t free_func)
  {
  src->free_func = free_func;
  }

void
gavl_packet_source_destroy(gavl_packet_source_t * s)
  {
  gavl_packet_free(&s->p);

  if(s->priv && s->free_func)
    s->free_func(s->priv);

  gavl_dictionary_free(&s->s_priv);
  
  free(s);
  }

void gavl_packet_source_reset(gavl_packet_source_t * s)
  {
  s->out_packet = NULL;
  }

#if 0
void gavl_packet_source_set_eof(gavl_packet_source_t * src, int eof)
  {
  pthread_mutex_lock(&src->eof_mutex);
  src->eof = eof;
  pthread_mutex_unlock(&src->eof_mutex);
  }

int gavl_packet_source_get_eof(gavl_packet_source_t * src)
  {
  int ret;
  pthread_mutex_lock(&src->eof_mutex);
  ret = src->eof;
  pthread_mutex_unlock(&src->eof_mutex);
  return ret;
  }
#endif
