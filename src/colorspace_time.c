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
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include <gavl.h>
#include <accel.h>

//#include "colorspace.h"

static struct timeval time_before;
static struct timeval time_after;

#define NUM_CONVERSIONS 20

#define FIXED_INPUT_PIXELFORMAT
#define INPUT_PIXELFORMAT GAVL_RGBA_32


static void timer_init()
  {
  gettimeofday(&time_before, NULL);
  }

static void timer_stop()
  {
  double before, after, diff;
  
  gettimeofday(&time_after, NULL);

  before = time_before.tv_sec + time_before.tv_usec / 1.0e6;
  after  = time_after.tv_sec  + time_after.tv_usec  / 1.0e6;

/*   fprintf(stderr, "Before: %f After: %f\n", before, after); */
    
  diff = after - before;
    
  fprintf(stderr, "Made %d conversions, Time: %f (%f per conversion)\n",
          NUM_CONVERSIONS, diff, diff/NUM_CONVERSIONS);
  }

int main()
  {
  int width = 720;
  int height = 576;

  const char * tmp;
  
  int j, k;

  int num_pixelformats = gavl_num_pixelformats();
  
  gavl_video_format_t input_format;
  gavl_video_format_t output_format;

  gavl_video_frame_t * input_frame;
  gavl_video_frame_t * output_frame;

  gavl_video_options_t * opt;
  
  gavl_video_converter_t * cnv = gavl_video_converter_create();
  opt = gavl_video_converter_get_options(cnv);
  input_format.pixelformat = GAVL_RGB_15;
  output_format.pixelformat = GAVL_RGB_15;

  input_format.image_width = width;
  input_format.image_height = height;

  input_format.frame_width = width;
  input_format.frame_height = height;

  input_format.pixel_width = 1;
  input_format.pixel_height = 1;

  output_format.image_width = width;
  output_format.image_height = height;

  output_format.frame_width = width;
  output_format.frame_height = height;

  output_format.pixel_width = 1;
  output_format.pixel_height = 1;

  //  char pixelformat_buffer[20];
  
#ifndef FIXED_INPUT_PIXELFORMAT
  for(i = 0; i < num_pixelformats; i++) /* Input format loop */
    {
    input_format.pixelformat = gavl_get_pixelformat(i);
#else
    input_format.pixelformat = INPUT_PIXELFORMAT;
#endif    
    input_frame = gavl_video_frame_create(&input_format);
    gavl_video_frame_clear(input_frame, &input_format);
    for(j = 0; j < num_pixelformats; j++) /* Output format loop */
      {
      output_format.pixelformat = gavl_get_pixelformat(j);
      if(input_format.pixelformat != output_format.pixelformat)
        {
        output_frame = gavl_video_frame_create(&output_format);
        fprintf(stderr, "************* Pixelformat conversion ");

        tmp = gavl_pixelformat_to_string(input_format.pixelformat);
        fprintf(stderr, "%s -> ", tmp);
        tmp = gavl_pixelformat_to_string(output_format.pixelformat);
        fprintf(stderr, "%s *************\n", tmp);

        gavl_video_options_set_defaults(opt);
        gavl_video_options_set_alpha_mode(opt, GAVL_ALPHA_BLEND_COLOR);
        
        gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_C);
        
        if(gavl_video_converter_init(cnv, &input_format, &output_format) < 1)
          fprintf(stderr, "No Conversion defined yet\n");
        else
          {
          /* Now, do some conversions */
          fprintf(stderr, "ANSI C Version: ");
          timer_init();
          for(k = 0; k < NUM_CONVERSIONS; k++)
            gavl_video_convert(cnv, input_frame, output_frame);
          timer_stop();
          }
        
        /* Now, initialize with MMX */

        fprintf(stderr, "MMX Version:    ");

        gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_MMX);

        if(gavl_video_converter_init(cnv, &input_format, &output_format) < 1)
          fprintf(stderr, "No Conversion defined yet\n");
        else
          {
          timer_init();
          for(k = 0; k < NUM_CONVERSIONS; k++)
            gavl_video_convert(cnv, input_frame, output_frame);
          timer_stop();
          }
        
        /* Now, initialize with MMXEXT */

        gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_MMXEXT);
        fprintf(stderr, "MMXEXT Version: ");
        
        if(gavl_video_converter_init(cnv, &input_format, &output_format) < 1)
          fprintf(stderr, "No Conversion defined yet\n");
        else
          {
          timer_init();
          for(k = 0; k < NUM_CONVERSIONS; k++)
            gavl_video_convert(cnv, input_frame, output_frame);
          timer_stop();
          }
        gavl_video_frame_destroy(output_frame);
        }
      }
    gavl_video_frame_destroy(input_frame);
#ifndef FIXED_INPUT_PIXELFORMAT
    }
#endif
  return 0;
  }
