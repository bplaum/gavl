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
#include <stdio.h>
#include <pthread.h>

#include <gavl/connectors.h>

#define FLAG_HAS_AFMT         (1<<0)
#define FLAG_HAS_VFMT         (1<<1)
#define FLAG_HAS_CI           (1<<2)
#define FLAG_HAS_TIMESCALE    (1<<3)

struct gavl_packet_source_s
  {
  gavl_audio_format_t audio_format;
  gavl_video_format_t video_format;
  gavl_compression_info_t ci;
  int timescale;
  
  gavl_packet_t p;
  
  int src_flags;
  
  gavl_packet_source_func_t func;
  void * priv;
  int flags;

  gavl_connector_lock_func_t lock_func;
  gavl_connector_lock_func_t unlock_func;
  void * lock_priv;
  gavl_connector_free_func_t free_func;

  pthread_mutex_t eof_mutex;
  int eof;
  int have_lock;
  };

gavl_packet_source_t *
gavl_packet_source_create(gavl_packet_source_func_t func,
                          void * priv, int src_flags)
  {
  gavl_packet_source_t * ret;
  ret = calloc(1, sizeof(*ret));
  ret->func = func;
  ret->priv = priv;
  ret->src_flags = src_flags;
  pthread_mutex_init(&ret->eof_mutex, NULL);
  return ret;
  }

gavl_packet_source_t *
gavl_packet_source_create_audio(gavl_packet_source_func_t func,
                                void * priv, int src_flags,
                                const gavl_compression_info_t * ci,
                                const gavl_audio_format_t * afmt)
  {
  gavl_packet_source_t * ret = gavl_packet_source_create(func, priv, src_flags);
  
  gavl_compression_info_copy(&ret->ci, ci);
  ret->flags |= FLAG_HAS_CI;
  
  gavl_audio_format_copy(&ret->audio_format, afmt);
  ret->flags |= FLAG_HAS_AFMT;
  return ret;
  }

gavl_packet_source_t *
gavl_packet_source_create_video(gavl_packet_source_func_t func,
                                void * priv, int src_flags,
                                const gavl_compression_info_t * ci,
                                const gavl_video_format_t * vfmt)
  {
  gavl_packet_source_t * ret = gavl_packet_source_create(func, priv, src_flags);
  
  gavl_compression_info_copy(&ret->ci, ci);
  ret->flags |= FLAG_HAS_CI;
  
  gavl_video_format_copy(&ret->video_format, vfmt);
  ret->flags |= FLAG_HAS_VFMT;
  return ret;
  }

gavl_packet_source_t *
gavl_packet_source_create_text(gavl_packet_source_func_t func,
                               void * priv, int src_flags,
                               int timescale)
  {
  gavl_packet_source_t * ret = gavl_packet_source_create(func, priv, src_flags);
  ret->timescale = timescale;
  ret->flags |= FLAG_HAS_TIMESCALE;
  return ret;
  }

gavl_packet_source_t *
gavl_packet_source_create_source(gavl_packet_source_func_t func,
                                 void * priv, int src_flags,
                                 gavl_packet_source_t * src)
  {
  gavl_packet_source_t * ret = gavl_packet_source_create(func, priv, src_flags);
  ret->timescale = src->timescale;
  ret->flags = src->flags;
  gavl_compression_info_copy(&ret->ci, &src->ci);
  gavl_video_format_copy(&ret->video_format, &src->video_format);
  gavl_audio_format_copy(&ret->audio_format, &src->audio_format);
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


const gavl_compression_info_t *
gavl_packet_source_get_ci(gavl_packet_source_t * s)
  {
  if(s->flags & FLAG_HAS_CI)
    return &s->ci;
  else
    return NULL;
  }
 
const gavl_audio_format_t *
gavl_packet_source_get_audio_format(gavl_packet_source_t * s)
  {
  if(s->flags & FLAG_HAS_AFMT)
    return &s->audio_format;
  else
    return NULL;
  }

const gavl_video_format_t *
gavl_packet_source_get_video_format(gavl_packet_source_t * s)
  {
  if(s->flags & FLAG_HAS_VFMT)
    return &s->video_format;
  else
    return NULL;
  }

GAVL_PUBLIC int
gavl_packet_source_get_timescale(gavl_packet_source_t * s)
  {
  if(s->flags & FLAG_HAS_VFMT)
    return s->video_format.timescale;
  else if(s->flags & FLAG_HAS_AFMT)
    return s->audio_format.samplerate;
  return s->timescale;
  }


gavl_source_status_t
gavl_packet_source_read_packet(void*sp, gavl_packet_t ** p)
  {
  gavl_source_status_t st;
  gavl_packet_t *p_src;
  gavl_packet_t *p_dst;
  
  gavl_packet_source_t * s = sp;

  if(gavl_packet_source_get_eof(s))
    return GAVL_SOURCE_EOF;
  
  gavl_packet_reset(&s->p);

  /* Decide source */
  if(s->src_flags & GAVL_SOURCE_SRC_ALLOC)
    p_src = NULL;
  else
    p_src = &s->p;
  
  /* Decide destination */
  if(*p)
    p_dst = *p;
  else
    p_dst = &s->p;

  if(s->lock_func)
    s->lock_func(s->lock_priv);
  
  /* Get packet */
  st = s->func(s->priv, &p_src);

  if(s->unlock_func)
    s->unlock_func(s->lock_priv);

  if(st != GAVL_SOURCE_OK)
    return st;
  
  /* Kick out error packets */
  if(p_src->duration < 0)
    return GAVL_SOURCE_EOF;
  
  /* Memcpy (if necessary) */
  if(p_src != p_dst)
    gavl_packet_copy(p_dst, p_src);

  if(!*p)
    *p = p_dst;

  
  return GAVL_SOURCE_OK;
  }

void gavl_packet_source_drain(gavl_packet_source_t * s)
  {
  gavl_source_status_t st;

  gavl_packet_t * p = NULL;
  while((st = gavl_packet_source_read_packet(s, &p)) == GAVL_SOURCE_OK)
    p = NULL;
  }

void gavl_packet_source_drain_nolock(gavl_packet_source_t * s)
  {
  s->have_lock = 1;
  gavl_packet_source_drain(s);
  s->have_lock = 0;
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
  gavl_compression_info_free(&s->ci);
  gavl_packet_free(&s->p);

  if(s->priv && s->free_func)
    s->free_func(s->priv);

  pthread_mutex_destroy(&s->eof_mutex);

  
  free(s);
  }

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
