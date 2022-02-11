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

#include <math.h>

#include <gavl/gavl.h>
#include <scale.h>
#include <accel.h>

static float get_weight_nearest(gavl_video_options_t * opt, double t)
  {
  if (t < -0.5) return 0.0;
  if (t <= 0.5) return 1.0;
  return 0.0;
  }

static float get_weight_bilinear(gavl_video_options_t * opt, double t)
  {
  if (t <= -1.0) return 0.0;
  if (t < 0.0) return (t + 1.0);
  if (t < 1.0) return (1.0 - t);
  return 0.0;
  }

static float get_weight_quadratic(gavl_video_options_t * opt, double t)
  {
  if (t < 0.0) t = -t;
  if (t < 0.5)
    return 0.75 - (t * t);
  if (t < 1.5)
    {
    t -= 1.5;
    return 0.5 * t * t;
    }
  return 0.0;
  }

static float get_weight_cubic_bspline(gavl_video_options_t * opt, double t)
  {
  if (t < 0.0) t = -t;
  if (t < 1.0)
    {
    return (4.0 + (t * t * (-6.0 + (t * 3.0)))) / 6.0;
    }
  if (t < 2.0)
    {
    t = 2.0 - t;
    return t * t * t / 6.0;
    }
  return 0.0;
  }

static float get_weight_cubic_mitchell(gavl_video_options_t * opt, double t)
  {
  if (t < 0.0) t = -t;
  if (t < 1.0)
    {
    return (16.0/3.0 + (t * (0.0 + (t * (-12.0 + (t * (7.0))))))) / 6.0;
    /*   (3)t^3 - (6)t^2 + 4    */
    
    /*   (1)t^3 - (2)t^2 + 1    */
    /*    3        6       3    */
    /* 2(a+2) = a+3    2a+4=a+3  a=-1 */
    }
  if (t < 2.0)
    {
    return (32.0/3.0 + (t * (-20.0 + (t * (12.0 + (t * -7.0/3.0)))))) / 6.0;
    }
  return 0.0;
  }

static float get_weight_cubic_catmull(gavl_video_options_t * opt, double t)
  {
  if (t < 0.0)
    t = -t;
  if (t < 1.0)
    return (2.0 + (t * t * (-5.0 + (t * 3.0)))) / 2.0;
  if (t < 2.0)
    return (4.0 + (t * (-8.0 + (t * (5.0 - t))))) / 2.0;
  return 0.0;
  }

#define PI 3.1415926535897932384626433832795029L

static float get_weight_sinc(gavl_video_options_t * opt, double t)
  {
  double w, order;
  order = (float)(opt->scale_order);
  if (t < 0.0)
    t = -t;
  if (t == 0.0)
    {
    return 1.0;
    }
  else if (t < order)
    {
    w = sin(PI*t / order) / (PI*t / order);
    return w * sin(t*PI) / (t*PI);
    }
  else
    {
    return 0.0;
    }
  }

gavl_video_scale_get_weight gavl_video_scale_get_weight_func(gavl_video_options_t * opt,
                                                             int * num_points)
  {
  switch(opt->scale_mode)
    {
    case GAVL_SCALE_NEAREST:
      *num_points = 1;
      return get_weight_nearest;
    case GAVL_SCALE_BILINEAR:
      *num_points = 2;
      return get_weight_bilinear;
    case GAVL_SCALE_QUADRATIC:
      *num_points = 3;
      return get_weight_quadratic;
    case GAVL_SCALE_CUBIC_BSPLINE:
      *num_points = 4;
      return get_weight_cubic_bspline;
    case GAVL_SCALE_CUBIC_MITCHELL:
      *num_points = 4;
      return get_weight_cubic_mitchell;
    case GAVL_SCALE_CUBIC_CATMULL:
      *num_points = 4;
      return get_weight_cubic_catmull;
    case GAVL_SCALE_SINC_LANCZOS:
      *num_points = opt->scale_order * 2;
      return get_weight_sinc;
    default:
      *num_points = 0;
      return NULL;
    }
  }
