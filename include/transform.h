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

#define MAX_TRANSFORM_FILTER 4

typedef struct 
  {
  int index_x;
  int index_y;
  int outside;
  
  float factors[MAX_TRANSFORM_FILTER][MAX_TRANSFORM_FILTER];
  int   factors_i[MAX_TRANSFORM_FILTER][MAX_TRANSFORM_FILTER];
  } gavl_transform_pixel_t;

typedef struct gavl_transform_context_s gavl_transform_context_t;

typedef void
(*gavl_transform_scanline_func)(gavl_transform_context_t * ctx,
                                gavl_transform_pixel_t * pixels,
                                uint8_t * dest_start);

typedef struct
  {
  gavl_transform_scanline_func transform_rgb_15;
  gavl_transform_scanline_func transform_rgb_16;
  gavl_transform_scanline_func transform_uint8_x_1_noadvance;
  gavl_transform_scanline_func transform_uint8_x_1_advance;
  gavl_transform_scanline_func transform_uint8_x_2;
  gavl_transform_scanline_func transform_uint8_x_3;
  gavl_transform_scanline_func transform_uint8_x_4;
  gavl_transform_scanline_func transform_uint16_x_1;
  gavl_transform_scanline_func transform_uint16_x_2;
  gavl_transform_scanline_func transform_uint16_x_3;
  gavl_transform_scanline_func transform_uint16_x_4;
  gavl_transform_scanline_func transform_float_x_1;
  gavl_transform_scanline_func transform_float_x_2;
  gavl_transform_scanline_func transform_float_x_3;
  gavl_transform_scanline_func transform_float_x_4;

  /* Bits needed for the integer scaling coefficient */
  int bits_rgb_15;
  int bits_rgb_16;
  int bits_uint8_noadvance;
  int bits_uint8_advance;
  int bits_uint16_x_1;
  int bits_uint16_x_2;
  int bits_uint16_x_3;
  int bits_uint16_x_4;
  } gavl_transform_funcs_t;

void gavl_init_transform_funcs_nearest_c(gavl_transform_funcs_t * tab,
                                         int advance);
void gavl_init_transform_funcs_bilinear_c(gavl_transform_funcs_t * tab,
                                          int advance);
void gavl_init_transform_funcs_quadratic_c(gavl_transform_funcs_t * tab,
                                           int advance);
void gavl_init_transform_funcs_bicubic_c(gavl_transform_funcs_t * tab,
                                         int advance);

#ifdef HAVE_MMX
void gavl_init_transform_funcs_bilinear_mmx(gavl_transform_funcs_t * tab,
                                            int advance);
void gavl_init_transform_funcs_bilinear_mmxext(gavl_transform_funcs_t * tab,
                                               int advance);

void gavl_init_transform_funcs_quadratic_mmx(gavl_transform_funcs_t * tab,
                                             int advance);
void gavl_init_transform_funcs_quadratic_mmxext(gavl_transform_funcs_t * tab,
                                                int advance);

void gavl_init_transform_funcs_bicubic_mmx(gavl_transform_funcs_t * tab,
                                           int advance);
void gavl_init_transform_funcs_bicubic_mmxext(gavl_transform_funcs_t * tab,
                                              int advance);

#endif



typedef struct 
  {
  gavl_transform_pixel_t ** pixels;
  int factors_per_pixel; /* Per dimension */
  } gavl_transform_table_t;

void gavl_transform_table_init(gavl_transform_table_t * t,
                               gavl_video_options_t * opt,
                               gavl_image_transform_func func, void * priv,
                               float off_x, float off_y, float scale_x,
                               float scale_y, int width, int height);

void gavl_transform_table_init_int(gavl_transform_table_t * tab,
                                   int bits, int width, int height);


void
gavl_transform_table_free(gavl_transform_table_t * tab);


/* Context is for one plane and field */

struct gavl_transform_context_s
  {
  gavl_transform_scanline_func func;
  gavl_transform_table_t tab;
  int offset;
  int advance;
  int plane;
  int field;
  int num_fields;
  int dst_width;
  int dst_height;
  
  /* Things set while transforming */
  uint8_t * src; /* Beginning of plane */
  int src_stride;
  gavl_video_options_t * opt;

  gavl_video_frame_t * dst_frame;
#ifdef HAVE_MMX
  int need_emms;
#endif

  };

int
gavl_transform_context_init(gavl_image_transform_t * t,
                            gavl_video_options_t * opt,
                            int field_index, int plane_index,
                            gavl_image_transform_func func, void * priv);

void
gavl_transform_context_transform(gavl_transform_context_t * ctx,
                                 const gavl_video_frame_t * src,
                                 gavl_video_frame_t * dst);

void
gavl_transform_context_free(gavl_transform_context_t * ctx);

struct gavl_image_transform_s
  {
  gavl_video_options_t opt;
  gavl_video_format_t format;
  
  /*
   *  a context is obtained with contexts[field][plane].
   *  field == 2 contains a progressive scaler, which might be
   *  required for frame based interlacing
   */
  gavl_transform_context_t contexts[3][GAVL_MAX_PLANES];
  
  int num_planes;
  int num_fields;
  
  };

