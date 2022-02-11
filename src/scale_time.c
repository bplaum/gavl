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
#include <sys/time.h>
#include <stdlib.h>
#include <gavl.h>
//#include "colorspace.h" // Common routines
#include <stdio.h>
#include <string.h>

#include <accel.h>
#include "timeutils.h"

#define NUM_CONVERSIONS 20

//#define SCALE_MODE GAVL_SCALE_SINC_LANCZOS
#define SCALE_MODE GAVL_SCALE_BILINEAR


int main(int argc, char ** argv)
  {
  gavl_rectangle_f_t src_rect;
  gavl_rectangle_i_t dst_rect;

  uint64_t t;
  
  int i, j, imax;
  gavl_video_scaler_t *scaler;
    
  gavl_video_format_t format, format_1;
  gavl_video_frame_t * frame, * frame_1;

  gavl_video_options_t * opt;
    
  gavl_pixelformat_t csp;

  imax = gavl_num_pixelformats();
  scaler = gavl_video_scaler_create();

  opt = gavl_video_scaler_get_options(scaler);

  memset(&format, 0, sizeof(format));
  memset(&format_1, 0, sizeof(format_1));
  
  for(i = 0; i < imax; i++)
    {
    csp = gavl_get_pixelformat(i);

    fprintf(stderr, "Pixelformat: %s imax: %d\n", gavl_pixelformat_to_string(csp), imax);
    
    src_rect.w = atoi(argv[1]);
    src_rect.h = atoi(argv[2]);
    src_rect.x = 0;
    src_rect.y = 0;
    
    dst_rect.w = atoi(argv[3]);
    dst_rect.h = atoi(argv[4]);
    dst_rect.x = 0;
    dst_rect.y = 0;
    
    format.image_width  = src_rect.w + src_rect.x;
    format.image_height = src_rect.h + src_rect.y;

    format.frame_width  = src_rect.w + src_rect.x;
    format.frame_height = src_rect.h + src_rect.y;

    format.pixel_width  = 1;
    format.pixel_height = 1;
        
    format_1.image_width  = dst_rect.w + dst_rect.x;
    format_1.image_height = dst_rect.h + dst_rect.y;

    format_1.frame_width  = dst_rect.w + dst_rect.x;
    format_1.frame_height = dst_rect.h + dst_rect.y;

    format_1.pixel_width  = 1;
    format_1.pixel_height = 1;

    format.pixelformat = csp;
    format_1.pixelformat = csp;
        
    frame   = gavl_video_frame_create(&format);
    frame_1 = gavl_video_frame_create(&format_1);

    gavl_video_frame_clear(frame_1, &format_1);

    /* Now, do the conversions */

    fprintf(stderr, "C-Version:\n");
    
    gavl_video_options_set_defaults(opt);
    gavl_video_options_set_scale_mode(opt, SCALE_MODE);
    gavl_video_options_set_scale_order(opt, 5);
    gavl_video_options_set_quality(opt, 3);
    //    gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_C);
    gavl_video_options_set_rectangles(opt, &src_rect, &dst_rect);

    if(gavl_video_scaler_init(scaler,
                              &format, &format_1) < 0)  // int output_height
      {
      fprintf(stderr, "No scaling routine defined\n");
      }
    else
      {
      timer_init();
      for(j = 0; j < NUM_CONVERSIONS; j++)
        {
        gavl_video_scaler_scale(scaler, frame, frame_1);
        }
      t = timer_stop();
      fprintf(stderr, "Made %d conversions, Time: %e (%e per conversion)\n",
              NUM_CONVERSIONS, (double)t, (double)t/NUM_CONVERSIONS);
      }
#if 1
    fprintf(stderr, "MMX-Version:\n");
    gavl_video_options_set_defaults(opt);
    gavl_video_options_set_scale_mode(opt, SCALE_MODE);
    gavl_video_options_set_quality(opt, 2);
    gavl_video_options_set_scale_order(opt, 5);
    gavl_video_options_set_rectangles(opt, &src_rect, &dst_rect);
        
    if(gavl_video_scaler_init(scaler,
                              &format, &format_1) < 0)  // int output_height
      {
      fprintf(stderr, "No scaling routine defined\n");
      }
    else
      {
      timer_init();
      for(j = 0; j < NUM_CONVERSIONS; j++)
        {
        gavl_video_scaler_scale(scaler, frame, frame_1);
        }
      t = timer_stop();
      fprintf(stderr, "Made %d conversions, Time: %e (%e per conversion)\n",
              NUM_CONVERSIONS, (double)t, (double)t/NUM_CONVERSIONS);
      }

#endif
    gavl_video_frame_destroy(frame);
    gavl_video_frame_destroy(frame_1);
    }
  gavl_video_scaler_destroy(scaler);
  return 0;
  }
