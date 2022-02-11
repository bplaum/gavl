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
#include <gavl.h>
#include <gavl/gavl_version.h>
//#include "colorspace.h" // Common routines
#include <stdio.h>
#include <png.h>

#include <accel.h>

gavl_pixelformat_t opt_pfmt = GAVL_PIXELFORMAT_NONE;
gavl_rectangle_f_t src_rect;
gavl_rectangle_i_t dst_rect;
gavl_video_format_t format, format_1;
gavl_video_frame_t * frame, * frame_1;

static int scale_factor_x = 2;
static int scale_factor_y = 2;

static const struct
  {
  gavl_scale_mode_t mode;
  const char * name;
  }
scale_modes[] =
  {
    { GAVL_SCALE_NEAREST, "nearest" },
    { GAVL_SCALE_BILINEAR, "bilinear" },
    { GAVL_SCALE_QUADRATIC, "quadratic" },
    { GAVL_SCALE_CUBIC_BSPLINE, "bspline" },
    { GAVL_SCALE_CUBIC_MITCHELL, "mitchell" },
    { GAVL_SCALE_CUBIC_CATMULL, "catmull" },
    { GAVL_SCALE_SINC_LANCZOS, "sinc" },
  };

#define IN_X 0
#define IN_Y 0

#define OUT_X 10
#define OUT_Y 10

static void write_png(char * filename, gavl_video_format_t * format,
                      gavl_video_frame_t * frame)
  {
  int i;
  unsigned char ** rows;
  gavl_video_options_t * opt;
  int color_type;
  FILE * output;

  png_structp png_ptr;
  png_infop   info_ptr;
  
  gavl_video_converter_t * cnv;
    
  gavl_video_format_t format_1;
  gavl_video_frame_t * frame_1 = NULL;

  
  if((format->pixelformat != GAVL_RGB_24) &&
     (format->pixelformat != GAVL_RGBA_32))
    {
    cnv = gavl_video_converter_create();
    
    gavl_video_format_copy(&format_1, format);

    if(gavl_pixelformat_has_alpha(format->pixelformat))
      {
      format_1.pixelformat = GAVL_RGBA_32;
      color_type = PNG_COLOR_TYPE_RGBA;
      }
    else
      {
      format_1.pixelformat = GAVL_RGB_24;
      color_type = PNG_COLOR_TYPE_RGB;
      }
    frame_1 = gavl_video_frame_create(&format_1);

    opt = gavl_video_converter_get_options(cnv);
    gavl_video_options_set_alpha_mode(opt, GAVL_ALPHA_BLEND_COLOR);    
    gavl_video_options_set_quality(opt, 2);    
    
    gavl_video_converter_init(cnv, format, &format_1);
    
    gavl_video_convert(cnv, frame, frame_1);
    gavl_video_converter_destroy(cnv);
    }
  else if(format->pixelformat == GAVL_RGB_24)
    {
    color_type = PNG_COLOR_TYPE_RGB;
    }
  else
    {
    color_type = PNG_COLOR_TYPE_RGBA;
    }
  
  output = fopen(filename, "wb");
  if(!output)
    return;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
                                         NULL, NULL);

  info_ptr = png_create_info_struct(png_ptr);
  setjmp(png_jmpbuf(png_ptr));
  png_init_io(png_ptr, output);
  
  png_set_IHDR(png_ptr, info_ptr,
               format->image_width,
               format->image_height,
               8, color_type, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  rows = malloc(format->image_height * sizeof(*rows));

  if(frame_1)
    {
    for(i = 0; i < format->image_height; i++)
      rows[i] = frame_1->planes[0] + i * frame_1->strides[0];
    }
  else
    {
    for(i = 0; i < format->image_height; i++)
      rows[i] = frame->planes[0] + i * frame->strides[0];
    }
  
  png_set_rows(png_ptr, info_ptr, rows);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(output);
  free(rows);
  if(frame_1)
    gavl_video_frame_destroy(frame_1);
  }

static gavl_video_frame_t * read_png(const char * filename,
                                     gavl_video_format_t * format,
                                     gavl_pixelformat_t pixelformat)
  {
  int i;
  unsigned char ** rows;
  
  gavl_video_converter_t * cnv;
  gavl_video_options_t * opt;
  gavl_video_format_t format_1;
  gavl_video_frame_t * frame, * frame_1;
    
  int bit_depth;
  int color_type;
  int has_alpha = 0;

  png_structp png_ptr;
  png_infop info_ptr;
  png_infop end_info;

  FILE * file;
  
  file = fopen(filename, "rb");

  if(!file)
    {
    fprintf(stderr, "Cannot open file %s\n", filename);
    return NULL;
    }
  
  png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING, NULL,
     NULL, NULL);
  
  setjmp(png_jmpbuf(png_ptr));
  info_ptr = png_create_info_struct(png_ptr);


  end_info = png_create_info_struct(png_ptr);

  png_init_io(png_ptr, file);

  png_read_info(png_ptr, info_ptr);

  format->frame_width  = png_get_image_width(png_ptr, info_ptr);
  format->frame_height = png_get_image_height(png_ptr, info_ptr);

  format->image_width  = format->frame_width;
  format->image_height = format->frame_height;
  format->pixel_width = 1;
  format->pixel_height = 1;

  bit_depth  = png_get_bit_depth(png_ptr,  info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  switch(color_type)
    {
    case PNG_COLOR_TYPE_GRAY:       /*  (bit depths 1, 2, 4, 8, 16) */
      if(bit_depth < 8)
#if GAVL_MAKE_BUILD(PNG_LIBPNG_VER_MAJOR, PNG_LIBPNG_VER_MINOR, PNG_LIBPNG_VER_RELEASE) < GAVL_MAKE_BUILD(1,2,9)
        png_set_gray_1_2_4_to_8(png_ptr);
#else
      png_set_expand_gray_1_2_4_to_8(png_ptr);
#endif
      if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        {
        png_set_tRNS_to_alpha(png_ptr);
        has_alpha = 1;
        }
      png_set_gray_to_rgb(png_ptr);
      break;
    case PNG_COLOR_TYPE_GRAY_ALPHA: /*  (bit depths 8, 16) */
      if(bit_depth == 16)
        png_set_strip_16(png_ptr);
      png_set_gray_to_rgb(png_ptr);
      break;
    case PNG_COLOR_TYPE_PALETTE:    /*  (bit depths 1, 2, 4, 8) */
      png_set_palette_to_rgb(png_ptr);
      if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        {
        png_set_tRNS_to_alpha(png_ptr);
        has_alpha = 1;
        }
      break;
    case PNG_COLOR_TYPE_RGB:        /*  (bit_depths 8, 16) */
      if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        {
        png_set_tRNS_to_alpha(png_ptr);
        has_alpha = 1;
        }
      if(bit_depth == 16)
        png_set_strip_16(png_ptr);
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:  /*  (bit_depths 8, 16) */
      if(bit_depth == 16)
        png_set_strip_16(png_ptr);
      has_alpha = 1;
      break;
    }
  if(has_alpha)
    format->pixelformat = GAVL_RGBA_32;
  else
    format->pixelformat = GAVL_RGB_24;

  frame = gavl_video_frame_create(format);
  rows = malloc(format->frame_height * sizeof(*rows));
  for(i = 0; i < format->frame_height; i++)
    rows[i] = frame->planes[0] + i * frame->strides[0];

  png_read_image(png_ptr, rows);
  png_read_end(png_ptr, end_info);

  png_destroy_read_struct(&png_ptr, &info_ptr,
                          &end_info);
  fclose(file);
  free(rows);
  
  /* Check wether to set up the converter */

  if(format->pixelformat != pixelformat)
    {
    cnv = gavl_video_converter_create();
    opt = gavl_video_converter_get_options(cnv);
    gavl_video_options_set_alpha_mode(opt, GAVL_ALPHA_BLEND_COLOR);    

    gavl_video_format_copy(&format_1, format);
    format_1.pixelformat = pixelformat;
    frame_1 = gavl_video_frame_create(&format_1);
    
    gavl_video_converter_init(cnv, format, &format_1);
    
    gavl_video_convert(cnv, frame, frame_1);
    gavl_video_converter_destroy(cnv);
    format->pixelformat = pixelformat;
    }
  else
    frame_1 = NULL;

  if(frame_1)
    {
    gavl_video_frame_destroy(frame);
    return frame_1;
    }
  else
    return frame;
  }

static void list_pfmts()
  {
  int i, num;
  num = gavl_num_pixelformats();
  
  for(i = 0; i < num; i++)
    {
    printf("%s\n", gavl_pixelformat_to_string(gavl_get_pixelformat(i)));
    }
  }

static void do_scale(gavl_video_scaler_t * scaler, const char * accel,
                     const char * mode)
  {
  char filename_buffer[1024];
  gavl_video_options_set_rectangles(gavl_video_scaler_get_options(scaler),
                                    &src_rect, &dst_rect);
  
  if(!gavl_video_scaler_init(scaler,
                            &format, &format_1))  // int output_height
    {
    fprintf(stderr, "No scaling routine for %s defined\n", accel);
    return;
    }
  
  gavl_video_frame_clear(frame_1, &format_1);
  
  gavl_video_scaler_scale(scaler, frame, frame_1);
  
  sprintf(filename_buffer, "%s-%s-%s-scaled.png",
          gavl_pixelformat_to_string(format_1.pixelformat),
          mode,
          accel);
  write_png(filename_buffer, &format_1, frame_1);
  fprintf(stderr, "Wrote %s\n", filename_buffer);
  }

static void print_help()
  {
  printf("Usage: scaletest [-pfmt <pfmt>] [-x <num>] [-y <num>] file.png\n");
  printf("       scaletest -help\n\n");
  printf("       scaletest -listpfmt\n\n");
  printf("-help\n  Print this help and exit\n");
  printf("-listpfmt\n  List pixelformats and exit\n");
  }

int main(int argc, char ** argv)
  {
  
  int i, imax;
  int j, jmax;
  gavl_video_scaler_t *scaler;
    

  gavl_video_options_t * opt;
    
  gavl_pixelformat_t csp;

  memset(&format, 0, sizeof(format));
  memset(&format_1, 0, sizeof(format_1));
  
  imax = gavl_num_pixelformats();
  jmax = sizeof(scale_modes)/sizeof(scale_modes[0]);
  scaler = gavl_video_scaler_create();
  
  opt = gavl_video_scaler_get_options(scaler);

  if(argc < 2)
    {
    print_help();
    return 0;
    }
  i = 1;
  while(i < argc)
    {
    if(!strcmp(argv[i], "-help"))
      {
      print_help();
      return 0;
      }
    else if(!strcmp(argv[i], "-listpfmts"))
      {
      list_pfmts();
      return 0;
      }
    else if(!strcmp(argv[i], "-pfmt"))
      {
      i++;
      opt_pfmt = gavl_string_to_pixelformat(argv[i]);
      i++;
      }
    else if(!strcmp(argv[i], "-x"))
      {
      i++;
      scale_factor_x = atoi(argv[i]);
      i++;
      }
    else if(!strcmp(argv[i], "-y"))
      {
      i++;
      scale_factor_y = atoi(argv[i]);
      i++;
      }
    else
      i++;
    }
  
  
  for(i = 0; i < imax; i++)
    {
    csp = gavl_get_pixelformat(i);

    if((opt_pfmt != GAVL_PIXELFORMAT_NONE) &&
       (opt_pfmt != csp))
      continue;
    
    fprintf(stderr, "Pixelformat: %s\n", gavl_pixelformat_to_string(csp));
    
    frame = read_png(argv[argc-1], &format, csp);

    src_rect.w = format.image_width;
    src_rect.h = format.image_height;
    src_rect.x = 0;
    src_rect.y = 0;
    
    dst_rect.w = src_rect.w*scale_factor_x;
    dst_rect.h = src_rect.h*scale_factor_y;
    dst_rect.x = 0;
    dst_rect.y = 0;

    
#if 0
    /* Write test frame */
    sprintf(filename_buffer, "%s-test.png", gavl_pixelformat_to_string(csp));
    write_png(filename_buffer, &format, frame);
#endif   
    gavl_video_format_copy(&format_1, &format);
    
    format_1.image_width  = dst_rect.w + dst_rect.x;
    format_1.image_height = dst_rect.h + dst_rect.y;

    format_1.frame_width  = dst_rect.w + dst_rect.x;
    format_1.frame_height = dst_rect.h + dst_rect.y;
    frame_1 = gavl_video_frame_create(&format_1);

    gavl_video_options_set_quality(opt, 0);
    
    gavl_video_options_set_scale_order(opt, 5);

    
    for(j = 0; j < jmax; j++)
      {
      gavl_video_options_set_scale_mode(opt, scale_modes[j].mode);
      
      gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_C);
      do_scale(scaler, "C", scale_modes[j].name);
      
      gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_MMX);
      do_scale(scaler, "MMX", scale_modes[j].name);
      
      gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_MMXEXT);
      do_scale(scaler, "MMXEXT", scale_modes[j].name);

      gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_SSE);
      do_scale(scaler, "SSE", scale_modes[j].name);

      gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_SSE2);
      do_scale(scaler, "SSE2", scale_modes[j].name);

      gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_SSE3);
      do_scale(scaler, "SSE3", scale_modes[j].name);
      
      }
    
    /* */
    
    gavl_video_frame_destroy(frame_1);
    gavl_video_frame_destroy(frame);
    }
  gavl_video_scaler_destroy(scaler);
  return 0;
  }
