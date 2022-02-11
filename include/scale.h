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

#ifndef _GAVL_SCALE_H_
#define _GAVL_SCALE_H_

#include <config.h>
#include "video.h"
#include "attributes.h"

/* Typedefs */

typedef struct gavl_video_scale_context_s gavl_video_scale_context_t;

typedef struct
  {
  int index; /* Index of the first row/column */
  int32_t * factor_i;
  float * factor_f;
  } gavl_video_scale_pixel_t;

typedef struct
  {
  int pixels_alloc;
  int factors_alloc;
  int num_pixels; /* Number of pixels (rows/columns) in the output area */
  float * factors_f;
  int32_t * factors_i;
  gavl_video_scale_pixel_t  * pixels;
  int factors_per_pixel;
  int do_clip; /* Use routines with clipping */
  int normalized;
  } gavl_video_scale_table_t;

typedef void
(*gavl_video_scale_scanline_func)(gavl_video_scale_context_t*, int scanline, uint8_t * dst);

typedef float
(*gavl_video_scale_get_weight)(gavl_video_options_t * opt, double t);

gavl_video_scale_get_weight
gavl_video_scale_get_weight_func(gavl_video_options_t * opt,
                                 int * num_points)
  __attribute__ ((visibility("default")));

/* Scale functions */

typedef struct
  {
  gavl_video_scale_scanline_func scale_rgb_15;
  gavl_video_scale_scanline_func scale_rgb_16;
  gavl_video_scale_scanline_func scale_uint8_x_1_noadvance;
  gavl_video_scale_scanline_func scale_uint8_x_1_advance;
  gavl_video_scale_scanline_func scale_uint8_x_2;
  gavl_video_scale_scanline_func scale_uint8_x_3;
  gavl_video_scale_scanline_func scale_uint8_x_4;
  gavl_video_scale_scanline_func scale_uint16_x_1;
  gavl_video_scale_scanline_func scale_uint16_x_2;
  gavl_video_scale_scanline_func scale_uint16_x_3;
  gavl_video_scale_scanline_func scale_uint16_x_4;
  gavl_video_scale_scanline_func scale_float_x_1;
  gavl_video_scale_scanline_func scale_float_x_2;
  gavl_video_scale_scanline_func scale_float_x_3;
  gavl_video_scale_scanline_func scale_float_x_4;

  /* Bits needed for the integer scaling coefficient */
  int bits_rgb_15;
  int bits_rgb_16;
  int bits_uint8_noadvance;
  int bits_uint8_advance;
  int bits_uint16;
  } gavl_scale_func_tab_t;

typedef struct
  {
  gavl_scale_func_tab_t funcs_x;
  gavl_scale_func_tab_t funcs_y;
  gavl_scale_func_tab_t funcs_xy;
  } gavl_scale_funcs_t;

void gavl_init_scale_funcs_nearest_c(gavl_scale_funcs_t * tab,
                                     int src_advance,
                                     int dst_advance);

void gavl_init_scale_funcs_bilinear_c(gavl_scale_funcs_t * tab);
void gavl_init_scale_funcs_bilinear_noclip_c(gavl_scale_funcs_t * tab);
void gavl_init_scale_funcs_bilinear_fast_c(gavl_scale_funcs_t * tab);

void gavl_init_scale_funcs_quadratic_c(gavl_scale_funcs_t * tab);
void gavl_init_scale_funcs_quadratic_noclip_c(gavl_scale_funcs_t * tab);

void gavl_init_scale_funcs_bicubic_c(gavl_scale_funcs_t * tab);
void gavl_init_scale_funcs_bicubic_noclip_c(gavl_scale_funcs_t * tab);


void gavl_init_scale_funcs_generic_c(gavl_scale_funcs_t * tab);
void gavl_init_scale_funcs_generic_noclip_c(gavl_scale_funcs_t * tab);


#ifdef HAVE_MMX
void gavl_init_scale_funcs_bicubic_y_mmx(gavl_scale_funcs_t * tab,
                                         int src_advance,
                                         int dst_advance);

void gavl_init_scale_funcs_quadratic_y_mmx(gavl_scale_funcs_t * tab,
                                           int src_advance,
                                           int dst_advance);


void gavl_init_scale_funcs_generic_y_mmx(gavl_scale_funcs_t * tab,
                                         int src_advance,
                                         int dst_advance);

void gavl_init_scale_funcs_bilinear_y_mmx(gavl_scale_funcs_t * tab,
                                          int src_advance, int dst_advance);

void gavl_init_scale_funcs_bicubic_x_mmx(gavl_scale_funcs_t * tab,
                                         int src_advance,
                                         int dst_advance);

void gavl_init_scale_funcs_quadratic_x_mmx(gavl_scale_funcs_t * tab,
                                           int src_advance,
                                           int dst_advance);

void gavl_init_scale_funcs_bicubic_noclip_x_mmx(gavl_scale_funcs_t * tab,
                                                int src_advance,
                                                int dst_advance);

void gavl_init_scale_funcs_generic_x_mmx(gavl_scale_funcs_t * tab,
                                         int src_advance,
                                         int dst_advance);

void gavl_init_scale_funcs_bilinear_x_mmx(gavl_scale_funcs_t * tab,
                                          int src_advance, int dst_advance);

/* */
void gavl_init_scale_funcs_bicubic_y_mmxext(gavl_scale_funcs_t * tab,
                                            int src_advance,
                                            int dst_advance);

void gavl_init_scale_funcs_quadratic_y_mmxext(gavl_scale_funcs_t * tab,
                                              int src_advance,
                                              int dst_advance);


void gavl_init_scale_funcs_generic_y_mmxext(gavl_scale_funcs_t * tab,
                                            int src_advance,
                                            int dst_advance);

void gavl_init_scale_funcs_bilinear_y_mmxext(gavl_scale_funcs_t * tab,
                                             int src_advance,
                                             int dst_advance);

void gavl_init_scale_funcs_bicubic_x_mmxext(gavl_scale_funcs_t * tab,
                                            int src_advance,
                                            int dst_advance);

void gavl_init_scale_funcs_quadratic_x_mmxext(gavl_scale_funcs_t * tab,
                                              int src_advance,
                                              int dst_advance);

void gavl_init_scale_funcs_bicubic_noclip_x_mmxext(gavl_scale_funcs_t * tab,
                                                   int src_advance,
                                                   int dst_advance);

void gavl_init_scale_funcs_generic_x_mmxext(gavl_scale_funcs_t * tab,
                                            int src_advance,
                                            int dst_advance);

void gavl_init_scale_funcs_bilinear_x_mmxext(gavl_scale_funcs_t * tab,
                                             int src_advance,
                                             int dst_advance);

#endif

#ifdef HAVE_SSE
void gavl_init_scale_funcs_quadratic_y_sse(gavl_scale_funcs_t * tab,
                                           int src_advance, int dst_advance);
  
void gavl_init_scale_funcs_bicubic_y_sse(gavl_scale_funcs_t * tab,
                                         int src_advance, int dst_advance);

void gavl_init_scale_funcs_bicubic_y_noclip_sse(gavl_scale_funcs_t * tab,
                                                int src_advance, int dst_advance);

void gavl_init_scale_funcs_generic_y_sse(gavl_scale_funcs_t * tab,
                                         int src_advance, int dst_advance);

void gavl_init_scale_funcs_bilinear_y_sse(gavl_scale_funcs_t * tab,
                                          int src_advance, int dst_advance);

void gavl_init_scale_funcs_quadratic_x_sse(gavl_scale_funcs_t * tab);
  
void gavl_init_scale_funcs_bicubic_x_sse(gavl_scale_funcs_t * tab);

void gavl_init_scale_funcs_bicubic_x_noclip_sse(gavl_scale_funcs_t * tab);

void gavl_init_scale_funcs_generic_x_sse(gavl_scale_funcs_t * tab);

void gavl_init_scale_funcs_bilinear_x_sse(gavl_scale_funcs_t * tab);


#endif


#ifdef HAVE_SSE2
void gavl_init_scale_funcs_bicubic_y_sse2(gavl_scale_funcs_t * tab,
                                         int src_advance,
                                         int dst_advance);

void gavl_init_scale_funcs_bicubic_y_noclip_sse2(gavl_scale_funcs_t * tab,
                                                 int src_advance,
                                                 int dst_advance);

void gavl_init_scale_funcs_quadratic_y_sse2(gavl_scale_funcs_t * tab,
                                           int src_advance,
                                           int dst_advance);


void gavl_init_scale_funcs_generic_y_sse2(gavl_scale_funcs_t * tab,
                                         int src_advance,
                                         int dst_advance);

void gavl_init_scale_funcs_bilinear_y_sse2(gavl_scale_funcs_t * tab,
                                          int src_advance, int dst_advance);


#endif

#ifdef HAVE_SSE3
void gavl_init_scale_funcs_bicubic_x_sse3(gavl_scale_funcs_t * tab);

void gavl_init_scale_funcs_bicubic_x_noclip_sse3(gavl_scale_funcs_t * tab);

void gavl_init_scale_funcs_generic_x_sse3(gavl_scale_funcs_t * tab);


#endif

void gavl_init_scale_funcs(gavl_scale_funcs_t * tab,
                           gavl_video_options_t * opt,
                           int src_advance,
                           int dst_advance,
                           gavl_video_scale_table_t * tab_h,
                           gavl_video_scale_table_t * tab_v);


void gavl_video_scale_table_init(gavl_video_scale_table_t * tab,
                                 gavl_video_options_t * opt,
                                 double src_off, double src_size,
                                 int dst_size,
                                 int src_width);

void 
gavl_video_scale_table_init_convolve(gavl_video_scale_table_t * tab,
                                     gavl_video_options_t * opt,
                                     int num_coeffs, const float * coeffs,
                                     int size);

void gavl_video_scale_table_init_int(gavl_video_scale_table_t * tab,
                                     int bits);

void gavl_video_scale_table_get_src_indices(gavl_video_scale_table_t * tab,
                                            int * start, int * size);

void gavl_video_scale_table_shift_indices(gavl_video_scale_table_t * tab,
                                          int shift);



void gavl_video_scale_table_cleanup(gavl_video_scale_table_t * tab);

/* For debugging */

void gavl_video_scale_table_dump(gavl_video_scale_table_t * tab);

/* Data needed by the scaline function. */

typedef struct
  {
  /* Offsets and advances are ALWAYS in bytes */
  int src_advance, dst_advance;
  int src_offset,  dst_offset;
  } gavl_video_scale_offsets_t;

/*
 *  Scale context is for one plane of one field.
 *  This means, that depending on the video format, we have 1 - 6 scale contexts.
 */

struct gavl_video_scale_context_s
  {
  /* Data initialized at start */
  gavl_video_scale_table_t table_h;
  gavl_video_scale_table_t table_v;
  gavl_video_scale_scanline_func func1;
  gavl_video_scale_scanline_func func2;

  gavl_video_scale_offsets_t offset1;
  gavl_video_scale_offsets_t offset2;
  
  /* Rectangles */
  gavl_rectangle_f_t src_rect;
  gavl_rectangle_i_t dst_rect;
  
  /* Indices of source and destination planes inside the frame. Can be 0 for chroma channels of
     packed YUV formats */
  
  int src_frame_plane, dst_frame_plane;

  int plane; /* Plane */
  
  /* Advances */

  gavl_video_scale_offsets_t * offset;
  
  /* Temporary buffer */
  uint8_t * buffer;
  int buffer_alloc;
  int buffer_stride;
  
  /* Size of temporary buffer in pixels */
  int buffer_width;
  int buffer_height;

  int num_directions;

  /* Minimum and maximum values for clipping.
     Values can be different for different components */
  
  int min_values_h[4];
  int max_values_h[4];

  int min_values_v[4];
  int max_values_v[4];

  float min_values_f[4];
  float max_values_f[4];
  
  /* For copying */
  int bytes_per_line;
  
  uint8_t * src;
  int src_stride;

  gavl_video_frame_t * dst_frame;
  
  const gavl_video_options_t * opt;
  
  //  uint8_t * dst;
  //  int scanline;
  int dst_size;
  
  int first_scanline;
#ifdef HAVE_MMX
  int need_emms;
#endif

  };

int gavl_video_scale_context_init(gavl_video_scale_context_t*,
                                  gavl_video_options_t * opt,
                                  int plane,
                                  const gavl_video_format_t * input_format,
                                  const gavl_video_format_t * output_format,
                                  int src_field, int dst_field,
                                  int src_fields, int dst_fields);

int gavl_video_scale_context_init_convolve(gavl_video_scale_context_t*,
                                           gavl_video_options_t * opt,
                                           int plane,
                                           const gavl_video_format_t * format,
                                           int num_fields,
                                           int h_radius, const float * h_coeffs,
                                           int v_radius, const float * v_coeffs);

void gavl_video_scale_context_cleanup(gavl_video_scale_context_t * ctx);

void gavl_video_scale_context_scale(gavl_video_scale_context_t * ctx,
                                    const gavl_video_frame_t * src,
                                    gavl_video_frame_t * dst);

struct gavl_video_scaler_s
  {
  gavl_video_options_t opt;

  /*
   *  a context is obtained with contexts[field][plane].
   *  field == 2 contains a progressive scaler, which might be
   *  required for frame based interlacing
   */
  gavl_video_scale_context_t contexts[3][GAVL_MAX_PLANES];
  
  int num_planes;

  /* If src_fields > dst_fields, we deinterlace */
  int src_fields;
  int dst_fields;
    
  gavl_video_frame_t * src;
  gavl_video_frame_t * dst;

  gavl_video_frame_t * src_field;
  gavl_video_frame_t * dst_field;
  
  gavl_video_format_t src_format;
  gavl_video_format_t dst_format;

  gavl_rectangle_i_t dst_rect;
  //  gavl_rectangle_f_t src_rect;

  };


#endif // _GAVL_SCALE_H_
