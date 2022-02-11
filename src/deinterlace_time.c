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
#define DEINTERLACE_MODE GAVL_DEINTERLACE_BLEND

#define WIDTH  720
#define HEIGHT 568

int main(int argc, char ** argv)
  {
  uint64_t t;
  
  int i, j, imax;
  gavl_video_deinterlacer_t *deinterlacer;
    
  gavl_video_format_t format;
  gavl_video_frame_t * frame, * frame_1;

  gavl_video_options_t * opt;
    
  gavl_pixelformat_t csp;

  imax = gavl_num_pixelformats();
  deinterlacer = gavl_video_deinterlacer_create();

  opt = gavl_video_deinterlacer_get_options(deinterlacer);

  memset(&format, 0, sizeof(format));
  
  for(i = 0; i < imax; i++)
    {
    csp = gavl_get_pixelformat(i);

    fprintf(stderr, "Pixelformat: %s imax: %d\n", gavl_pixelformat_to_string(csp), imax);
    
    format.image_width  = WIDTH;
    format.image_height = HEIGHT;

    format.frame_width  = WIDTH;
    format.frame_height = HEIGHT;

    format.pixel_width  = 1;
    format.pixel_height = 1;
    
    format.pixelformat = csp;

    frame   = gavl_video_frame_create(&format);
    frame_1 = gavl_video_frame_create(&format);
    
    gavl_video_frame_clear(frame_1, &format);

    /* Now, do the conversions */

    fprintf(stderr, "C-Version:\n");
    
    gavl_video_options_set_defaults(opt);
    gavl_video_options_set_deinterlace_mode(opt, DEINTERLACE_MODE);
    gavl_video_options_set_quality(opt, 3);
    gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_C);

    if(gavl_video_deinterlacer_init(deinterlacer,
                              &format) < 0)  // int output_height
      {
      fprintf(stderr, "No scaling routine defined\n");
      }
    else
      {
      timer_init();
      for(j = 0; j < NUM_CONVERSIONS; j++)
        {
        gavl_video_deinterlacer_deinterlace(deinterlacer, frame, frame_1);
        }
      t = timer_stop();
      fprintf(stderr, "Made %d conversions, Time: %e (%e per conversion)\n",
              NUM_CONVERSIONS, (double)t, (double)t/NUM_CONVERSIONS);
      }
#if 1
    fprintf(stderr, "MMX-Version:\n");
    gavl_video_options_set_defaults(opt);
    gavl_video_options_set_deinterlace_mode(opt, DEINTERLACE_MODE);
    gavl_video_options_set_quality(opt, 2);
    gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_MMX);
    
    gavl_video_deinterlacer_init(deinterlacer, &format);

    timer_init();
    for(j = 0; j < NUM_CONVERSIONS; j++)
      {
      gavl_video_deinterlacer_deinterlace(deinterlacer, frame, frame_1);
      }
    t = timer_stop();
    fprintf(stderr, "Made %d conversions, Time: %e (%e per conversion)\n",
            NUM_CONVERSIONS, (double)t, (double)t/NUM_CONVERSIONS);
    
#endif

#if 1
    fprintf(stderr, "MMXEXT-Version:\n");
    gavl_video_options_set_defaults(opt);
    gavl_video_options_set_deinterlace_mode(opt, DEINTERLACE_MODE);
    gavl_video_options_set_quality(opt, 2);
    gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_MMXEXT);
    
    gavl_video_deinterlacer_init(deinterlacer, &format);

    timer_init();
    for(j = 0; j < NUM_CONVERSIONS; j++)
      {
      gavl_video_deinterlacer_deinterlace(deinterlacer, frame, frame_1);
      }
    t = timer_stop();
    fprintf(stderr, "Made %d conversions, Time: %e (%e per conversion)\n",
            NUM_CONVERSIONS, (double)t, (double)t/NUM_CONVERSIONS);
    
#endif

    gavl_video_frame_destroy(frame);
    gavl_video_frame_destroy(frame_1);
    }
  gavl_video_deinterlacer_destroy(deinterlacer);
  return 0;
  }
