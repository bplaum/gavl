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

#ifndef _GAVL_VIDEO_H_
#define _GAVL_VIDEO_H_

/* Private structures for the video converter */

#include "config.h"

  
struct gavl_video_options_s
  {
  /*
   *  Quality setting from 1 to 5 (0 means undefined).
   *  3 means Standard C routines or accellerated version with
   *  equal quality. Lower numbers mean accellerated versions with lower
   *  quality.
   */
  int quality;         

  /* Explicit accel_flags are mainly for colorspace_test.c */
  int accel_flags;     /* CPU Acceleration flags */

  int conversion_flags;
  
  gavl_alpha_mode_t alpha_mode;

  gavl_scale_mode_t scale_mode;
  int scale_order;

  gavl_deinterlace_mode_t deinterlace_mode;
  gavl_deinterlace_drop_mode_t deinterlace_drop_mode;

  /* Background color (Floating point and 16 bit int)background_float[3]; */
  float background_float[3];

  uint16_t background_16[3];
  
  /* Source and destination rectangles */

  gavl_rectangle_f_t src_rect;
  gavl_rectangle_i_t dst_rect;
  int have_rectangles;
  
  int keep_aspect;
  
  gavl_downscale_filter_t downscale_filter;
  float downscale_blur;

  int num_threads;
  void (*run_func)(void (*func)(void*, int start, int len),
                   void * gavl_data, int start, int len, void * client_data, int thread);
  void * run_data;
  
  void (*stop_func)(void * gavl_data, int thread);
  void * stop_data;
  };

typedef struct gavl_video_convert_context_s gavl_video_convert_context_t;

typedef void (*gavl_video_func_t)(gavl_video_convert_context_t * ctx);

struct gavl_video_convert_context_s
  {
  const gavl_video_frame_t * input_frame;
  gavl_video_frame_t * output_frame;
  gavl_video_options_t * options;

  gavl_video_format_t input_format;
  gavl_video_format_t output_format;

  gavl_video_scaler_t * scaler;
  gavl_video_deinterlacer_t * deinterlacer;
  
  struct gavl_video_convert_context_s * next;
  gavl_video_func_t func;
  };

struct gavl_video_converter_s
  {
  gavl_video_format_t input_format;
  gavl_video_format_t output_format;
  
  gavl_video_options_t options;

  gavl_video_convert_context_t * first_context;
  gavl_video_convert_context_t * last_context;
  int num_contexts;

  int have_frames;
  };


/* Find conversion functions */

gavl_video_func_t
gavl_find_pixelformat_converter(const gavl_video_options_t * opt,
                               gavl_pixelformat_t input_pixelformat,
                               gavl_pixelformat_t output_pixelformat,
                               int width, int height);

/* Check if a pixelformat can be converted by simple scaling */

int gavl_pixelformat_can_scale(gavl_pixelformat_t in_csp, gavl_pixelformat_t out_csp);

/*
 *  Return a pixelformat (or GAVL_PIXELFORMAT_NONE) as an intermediate pixelformat
 *  for which the conversion quality can be improved. E.g. instead of
 *  RGB -> YUV420P, we can do RGB -> YUV444P -> YUV420P with proper chroma scaling
 */

gavl_pixelformat_t gavl_pixelformat_get_intermediate(gavl_pixelformat_t in_csp,
                                                   gavl_pixelformat_t out_csp);

#define CLEAR_MASK_PLANE_0 (1<<0)
#define CLEAR_MASK_PLANE_1 (1<<1)
#define CLEAR_MASK_PLANE_2 (1<<2)
#define CLEAR_MASK_PLANE_3 (1<<3)
#define CLEAR_MASK_ALL (CLEAR_MASK_PLANE_0|\
                        CLEAR_MASK_PLANE_1|\
                        CLEAR_MASK_PLANE_2|\
                        CLEAR_MASK_PLANE_3)

void gavl_video_frame_clear_mask(gavl_video_frame_t * frame,
                                 const gavl_video_format_t * format, int mask);

void gavl_pixelformat_get_offset(gavl_pixelformat_t pixelformat,
                                 int plane,
                                 int * advance, int * offset);



#endif // _GAVL_VIDEO_H_
