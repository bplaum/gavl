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

#include <gavl/gavl.h>

struct gavl_video_frame_pool_s
  {
  int num_frames;
  int frames_alloc;
  gavl_video_frame_t ** frames;
  gavl_video_frame_t * (*create_frame)(void * priv);
  void * priv;
  };

gavl_video_frame_pool_t *
gavl_video_frame_pool_create(gavl_video_frame_t * (create_frame)(void * priv),
                             void * priv)
  {
  gavl_video_frame_pool_t * ret;
  ret = calloc(1, sizeof(*ret));
  ret->create_frame = create_frame;
  ret->priv = priv;
  return ret;
  }
  
gavl_video_frame_t * gavl_video_frame_pool_get(gavl_video_frame_pool_t *p)
  {
  int i;
  for(i = 0; i < p->num_frames; i++)
    {
    if(!p->frames[i]->refcount)
      return p->frames[i];
    }
  /* Allocate a new frame */
  if(p->num_frames == p->frames_alloc)
    {
    p->frames_alloc += 16;
    p->frames = realloc(p->frames, p->frames_alloc * sizeof(*p->frames));
    }

  if(p->create_frame)
    p->frames[p->num_frames] = p->create_frame(p->priv);
  else
    {
    p->frames[p->num_frames] = gavl_video_frame_create(p->priv);
    gavl_video_frame_clear(p->frames[p->num_frames], p->priv);
    }
  p->num_frames++;
  return p->frames[p->num_frames-1];
  }

void gavl_video_frame_pool_destroy(gavl_video_frame_pool_t *p)
  {
  int i;
  for(i = 0; i < p->num_frames; i++)
    gavl_video_frame_destroy(p->frames[i]);
  if(p->frames)
    free(p->frames);
  free(p);
  }

void gavl_video_frame_pool_reset(gavl_video_frame_pool_t *p)
  {
  int i;
  for(i = 0; i < p->num_frames; i++)
    p->frames[i]->refcount = 0;
  }
