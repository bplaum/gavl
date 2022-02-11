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

#ifndef _GAVL_DEINTERLACE_H_
#define _GAVL_DEINTERLACE_H_

/* Private structures for the deinterlacer */

#include "config.h"

typedef void (*gavl_video_deinterlace_func)(gavl_video_deinterlacer_t*,
                                            const gavl_video_frame_t*in,
                                            gavl_video_frame_t*out);

typedef void (*gavl_video_deinterlace_blend_func)(const uint8_t * t,
                                                  const uint8_t * m,
                                                  const uint8_t * b,
                                                  uint8_t * dst,
                                                  int num);

typedef struct
  {
  gavl_video_deinterlace_blend_func func_packed_15;
  gavl_video_deinterlace_blend_func func_packed_16;
  gavl_video_deinterlace_blend_func func_8;
  gavl_video_deinterlace_blend_func func_16;
  gavl_video_deinterlace_blend_func func_float;
  } gavl_video_deinterlace_blend_func_table_t;

struct gavl_video_deinterlacer_s
  {
  gavl_video_options_t opt;
  gavl_video_format_t format;
  gavl_video_format_t half_height_format;
  gavl_video_deinterlace_func func;
  
  gavl_video_frame_t * src_field;
  gavl_video_frame_t * dst_field;
  
  gavl_video_scaler_t * scaler;
  
  gavl_video_deinterlace_blend_func blend_func;
  
  int num_planes;

  /* For blending, it's the number of components per line,
     for copying it's the number of bytes passed to gavl_memcpy
  */
  int line_width;

  int sub_h;
  int sub_v;
  
  int mixed;
  };

/* Find conversion function */

int gavl_deinterlacer_init_scale(gavl_video_deinterlacer_t * d);


int gavl_deinterlacer_init_blend(gavl_video_deinterlacer_t * d);

int gavl_deinterlacer_init_copy(gavl_video_deinterlacer_t * d);

void
gavl_find_deinterlacer_blend_funcs_c(gavl_video_deinterlace_blend_func_table_t * tab,
                                     const gavl_video_options_t * opt,
                                     const gavl_video_format_t * format);

#ifdef HAVE_MMX
void
gavl_find_deinterlacer_blend_funcs_mmx(gavl_video_deinterlace_blend_func_table_t * tab,
                                       const gavl_video_options_t * opt,
                                      const gavl_video_format_t * format);

void
gavl_find_deinterlacer_blend_funcs_mmxext(gavl_video_deinterlace_blend_func_table_t * tab,
                                          const gavl_video_options_t * opt,
                                          const gavl_video_format_t * format);
#endif

#ifdef HAVE_3DNOW
void
gavl_find_deinterlacer_blend_funcs_3dnow(gavl_video_deinterlace_blend_func_table_t * tab,
                                         const gavl_video_options_t * opt,
                                         const gavl_video_format_t * format);
#endif


#endif // _GAVL_DEINTERLACE_H_
