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
#include <video.h>
#include <transform.h>

gavl_image_transform_t * gavl_image_transform_create()
  {
  int i, j;
  gavl_image_transform_t * ret;
  ret = calloc(1, sizeof(*ret));
  gavl_video_options_set_defaults(&ret->opt);

  for(i = 0; i < 3; i++)
    {
    for(j = 0; j < GAVL_MAX_PLANES; j++)
      ret->contexts[i][j].opt = &ret->opt;
    }


  return ret;
  }

void gavl_image_transform_destroy(gavl_image_transform_t * t)
  {
  int i, j;
  for(i = 0; i < 3; i++)
    {
    for(j = 0; j < GAVL_MAX_PLANES; j++)
      {
      gavl_transform_context_free(&t->contexts[i][j]);
      }
    }
  free(t);
  }

/** \brief Destroy a transformation engine
 *  \param A transformation engine
 *  \param Format (can be changed)
 *  \param func Coordinate transform function
 *  \param priv The priv argument for func
 */

int gavl_image_transform_init(gavl_image_transform_t * t,
                               gavl_video_format_t * format,
                               gavl_image_transform_func func, void * priv)
  {
  int i, j;
  gavl_video_options_t opt;
  gavl_video_options_copy(&opt, &t->opt);

  if(opt.scale_mode == GAVL_SCALE_AUTO)
    {
    if(opt.quality < 2)
      opt.scale_mode = GAVL_SCALE_NEAREST;
    else if(opt.quality < 3)
      opt.scale_mode = GAVL_SCALE_BILINEAR;
    else
      opt.scale_mode = GAVL_SCALE_CUBIC_BSPLINE;
    }

  /* Warning: Don't change this until you know what you
     are doing. The supported scale modes have a maximum of
     4 filter taps and *no* clipping */
  
  if(opt.scale_mode > GAVL_SCALE_CUBIC_BSPLINE)
    opt.scale_mode = GAVL_SCALE_CUBIC_BSPLINE;
  
  gavl_video_format_copy(&t->format, format);
  
  /* Count fields */
  switch(format->interlace_mode)
    {
    case GAVL_INTERLACE_NONE:
    case GAVL_INTERLACE_UNKNOWN:
      t->num_fields = 1;
      break; 
    case GAVL_INTERLACE_TOP_FIRST:
    case GAVL_INTERLACE_BOTTOM_FIRST:
      t->num_fields = 2;
      break; 
    case GAVL_INTERLACE_MIXED:
    case GAVL_INTERLACE_MIXED_TOP:
    case GAVL_INTERLACE_MIXED_BOTTOM:
      t->num_fields = 3;
      break;
    }

  if((t->format.pixelformat == GAVL_YUY2) ||
     (t->format.pixelformat == GAVL_UYVY))
    t->num_planes = 3;
  else
    t->num_planes =
      gavl_pixelformat_num_planes(t->format.pixelformat);
  
  /* Setup contexts */
  
  for(i = 0; i < t->num_fields; i++)
    for(j = 0; j < t->num_planes; j++)
      {
      if(!gavl_transform_context_init(t, &opt, i, j, func, priv))
        return 0;
      }
  return 1;
  }

/** \brief Transform an image
 *  \param A transformation engine
 *  \param Input frame
 *  \param Output frame
 */
  
void gavl_image_transform_transform(gavl_image_transform_t * t,
                                    gavl_video_frame_t * in_frame,
                                    gavl_video_frame_t * out_frame)
  {
  int i, j;
  
  int num_fields, field;

  if(t->format.interlace_mode == GAVL_INTERLACE_NONE)
    {
    num_fields = 1;
    field = 0;
    }
  else if((t->format.interlace_mode == GAVL_INTERLACE_MIXED) &&
          (in_frame->interlace_mode == GAVL_INTERLACE_NONE))
    {
    num_fields = 1;
    field = 2;
    }
  else
    num_fields = 2;

  if(num_fields == 1)
    {
    for(j = 0; j < t->num_planes; j++)
      {
      //      fprintf(stderr, "Transform: %d\n", field);
      gavl_transform_context_transform(&t->contexts[field][j],
                                       in_frame, out_frame);
      }
    }
  else
    {
    for(i = 0; i < 2; i++)
      {
      for(j = 0; j < t->num_planes; j++)
        {
        gavl_transform_context_transform(&t->contexts[i][j],
                                         in_frame, out_frame);
        }
      }
    }
  
  }

gavl_video_options_t *
gavl_image_transform_get_options(gavl_image_transform_t * t)
  {
  return &t->opt;
  }
