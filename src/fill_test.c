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
#include <string.h>
#include <gavl.h>
//#include "colorspace.h" // Common routines
#include <stdio.h>
#include <png.h>

static void
write_png(char * filename, gavl_video_format_t * format, gavl_video_frame_t * frame)
  {
  int i;
  unsigned char ** rows;

  int color_type;
  FILE * output;

  png_structp png_ptr;
  png_infop   info_ptr;
  
  gavl_video_converter_t * cnv;
    
  gavl_video_format_t format_1;
  gavl_video_frame_t * frame_1 = NULL;

  
  if((format->pixelformat != GAVL_RGB_24) && (format->pixelformat != GAVL_RGBA_32))
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

int main(int argc, char ** argv)
  {
  
  char filename_buffer[1024];
  int i, imax;
    
  gavl_video_format_t frame_format;

  gavl_video_frame_t * frame;
  float color[4] = { 0.0, 1.0, 0.0, 0.5 };
  
  memset(&frame_format,   0, sizeof(frame_format));

  frame_format.image_width  = 128;
  frame_format.image_height = 128;
  frame_format.frame_width  = 128;
  frame_format.frame_height = 128;

  frame_format.pixel_width  = 1;
  frame_format.pixel_height = 1;
  
  imax = gavl_num_pixelformats();
  
  for(i = 0; i < imax; i++)
    {
    frame_format.pixelformat = gavl_get_pixelformat(i);
    
    //    if(frame_csp != GAVL_YUVA_32)
    //      continue;
    
    //    csp = GAVL_RGB_24;
    
    frame   = gavl_video_frame_create(&frame_format);

    sprintf(filename_buffer, "fill_%s.png",
            gavl_pixelformat_to_string(frame_format.pixelformat));
    
    gavl_video_frame_fill(frame, &frame_format, color);
    write_png(filename_buffer, &frame_format, frame);
    gavl_video_frame_destroy(frame);
    fprintf(stderr, "Wrote %s\n", filename_buffer);
    }
  return 0;
  }
