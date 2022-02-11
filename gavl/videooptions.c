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

#include <stdlib.h> /* calloc, free */
#include <string.h> /* calloc, free */

#ifdef DEBUG
#include <stdio.h>  
#endif

#include <gavl.h>
#include <config.h>
#include <video.h>
#include <accel.h>

/***************************************************
 * Default Options
 ***************************************************/

static void run_func_default(void (*func)(void*, int, int),
                             void * gavl_data,
                             int start, int len,
                             void * client_data, int thread)
  {
  func(gavl_data, start, len);
  }

static void stop_func_default(void * client_data, int thread)
  {
  }

void gavl_video_options_set_defaults(gavl_video_options_t * opt)
  {
  memset(opt, 0, sizeof(*opt));
  opt->accel_flags = gavl_accel_supported();
  opt->scale_order = 4;
  opt->quality = GAVL_QUALITY_DEFAULT;
  opt->downscale_blur = 1.0;
  opt->downscale_filter = GAVL_DOWNSCALE_FILTER_WIDE;

  opt->num_threads = 1;
  opt->run_func = run_func_default;
  opt->stop_func = stop_func_default;
  
  gavl_init_memcpy();
  }

int gavl_video_options_get_num_threads(const gavl_video_options_t * opt)
  {
  return opt->num_threads;
  }

void gavl_video_options_set_num_threads(gavl_video_options_t * opt, int n)
  {
  opt->num_threads = n;
  }

void gavl_video_options_set_run_func(gavl_video_options_t * opt,
                                     void (*run)(void (*func)(void*, int start, int num),
                                                 void * gavl_data,
                                                 int start, int num,
                                                 void * client_data, int thread), 
                                     void * client_data)
  {
  opt->run_func = run;
  opt->run_data = client_data;
  }

gavl_video_run_func
gavl_video_options_get_run_func(const gavl_video_options_t * opt,
                                void ** client_data)
  {
  *client_data = opt->run_data;
  return opt->run_func;
  }


void gavl_video_options_set_stop_func(gavl_video_options_t * opt,
                                      void (*stop)(void * client_data, int thread), 
                                      void * client_data)
  {
  opt->stop_func = stop;
  opt->stop_data = client_data;
  }

gavl_video_stop_func
gavl_video_options_get_stop_func(const gavl_video_options_t * opt,
                                 void ** client_data)
  {
  *client_data = opt->stop_data;
  return opt->stop_func;
  }

void gavl_video_options_set_rectangles(gavl_video_options_t * opt,
                                       const gavl_rectangle_f_t * src_rect,
                                       const gavl_rectangle_i_t * dst_rect)
  {
  if(!src_rect || !dst_rect)
    {
    opt->have_rectangles = 0;
    return;
    }
  gavl_rectangle_f_copy(&opt->src_rect, src_rect);
  gavl_rectangle_i_copy(&opt->dst_rect, dst_rect);
  opt->have_rectangles = 1;
  }

void
gavl_video_options_get_rectangles(const gavl_video_options_t * opt,
                                  gavl_rectangle_f_t * src_rect,
                                  gavl_rectangle_i_t * dst_rect)
  {
  gavl_rectangle_f_copy(src_rect, &opt->src_rect);
  gavl_rectangle_i_copy(dst_rect, &opt->dst_rect);
  }

#define SET_INT(p) opt->p = p


void gavl_video_options_set_quality(gavl_video_options_t * opt, int quality)
  {
  SET_INT(quality);
  }

int
gavl_video_options_get_quality(const gavl_video_options_t * opt)
  {
  return opt->quality;
  }

void gavl_video_options_set_accel_flags(gavl_video_options_t * opt,
                                        int accel_flags)
  {
  SET_INT(accel_flags);
  }

int gavl_video_options_get_accel_flags(gavl_video_options_t * opt)
  {
  return opt->accel_flags;
  }

void gavl_video_options_set_conversion_flags(gavl_video_options_t * opt,
                                             int conversion_flags)
  {
  SET_INT(conversion_flags);
  }

int
gavl_video_options_get_conversion_flags(const gavl_video_options_t * opt)
  {
  return opt->conversion_flags;
  }

void gavl_video_options_set_alpha_mode(gavl_video_options_t * opt,
                                       gavl_alpha_mode_t alpha_mode)
  {
  SET_INT(alpha_mode);
  }

gavl_alpha_mode_t
gavl_video_options_get_alpha_mode(const gavl_video_options_t * opt)
  {
  return opt->alpha_mode;
  }

void gavl_video_options_set_scale_mode(gavl_video_options_t * opt,
                                       gavl_scale_mode_t scale_mode)
  {
  SET_INT(scale_mode);
  }

gavl_scale_mode_t
gavl_video_options_get_scale_mode(const gavl_video_options_t * opt)
  {
  return opt->scale_mode;
  }



void gavl_video_options_set_scale_order(gavl_video_options_t * opt,
                                        int scale_order)
  {
  SET_INT(scale_order);
  }

int gavl_video_options_get_scale_order(const gavl_video_options_t * opt)
  {
  return opt->scale_order;
  }

void
gavl_video_options_set_deinterlace_mode(gavl_video_options_t * opt,
                                        gavl_deinterlace_mode_t deinterlace_mode)
  {
  SET_INT(deinterlace_mode);
  }

gavl_deinterlace_mode_t
gavl_video_options_get_deinterlace_mode(const gavl_video_options_t * opt)
  {
  return opt->deinterlace_mode;
  }



void
gavl_video_options_set_deinterlace_drop_mode(gavl_video_options_t * opt,
                                             gavl_deinterlace_drop_mode_t deinterlace_drop_mode)
  {
  SET_INT(deinterlace_drop_mode);
  }

gavl_deinterlace_drop_mode_t
gavl_video_options_get_deinterlace_drop_mode(const gavl_video_options_t * opt)
  {
  return opt->deinterlace_drop_mode;
  }

#undef SET_INT

#define CLIP_FLOAT(a) if(a < 0.0) a = 0.0; if(a>1.0) a = 1.0;

void gavl_video_options_set_background_color(gavl_video_options_t * opt,
                                             const double * color)
  {
  opt->background_float[0] = color[0];
  opt->background_float[1] = color[1];
  opt->background_float[2] = color[2];
  
  CLIP_FLOAT(opt->background_float[0]);
  CLIP_FLOAT(opt->background_float[1]);
  CLIP_FLOAT(opt->background_float[2]);
  opt->background_16[0] = (uint16_t)(opt->background_float[0] * 65535.0 + 0.5);
  opt->background_16[1] = (uint16_t)(opt->background_float[1] * 65535.0 + 0.5);
  opt->background_16[2] = (uint16_t)(opt->background_float[2] * 65535.0 + 0.5);
  
  }

void
gavl_video_options_get_background_color(const gavl_video_options_t * opt,
                                        double * color)
  {
  color[0] = opt->background_float[0];
  color[1] = opt->background_float[1];
  color[2] = opt->background_float[2];
  }

void gavl_video_options_set_downscale_filter(gavl_video_options_t * opt,
                                             gavl_downscale_filter_t f)
  {
  opt->downscale_filter = f;
  
  }
  
gavl_downscale_filter_t
gavl_video_options_get_downscale_filter(const gavl_video_options_t * opt)
  {
  return opt->downscale_filter;
  }

void gavl_video_options_set_downscale_blur(gavl_video_options_t * opt,
                                           float f)
  {
  opt->downscale_blur = f;
  }
  

/*!  \ingroup video_options
 *   \brief Get blur factor for downscaling
 *   \param opt Video options
 *   \returns Factor
 *
 *  Since 1.1.0
 */
  
float
gavl_video_options_get_downscale_blur(const gavl_video_options_t * opt)
  {
  return opt->downscale_blur;
  }


gavl_video_options_t * gavl_video_options_create()
  {
  gavl_video_options_t * ret = calloc(1, sizeof(*ret));
  gavl_video_options_set_defaults(ret);
  return ret;
  }

void gavl_video_options_copy(gavl_video_options_t * dst,
                             const gavl_video_options_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  }

void gavl_video_options_destroy(gavl_video_options_t * opt)
  {
  free(opt);
  }

