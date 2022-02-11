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
#include <stdio.h>
#include <pngutil.h>
#include <accel.h>

#define IN_X 0
#define IN_Y 0

#define OUT_X 10
#define OUT_Y 10

int main(int argc, char ** argv)
  {
  char filename_buffer[1024];
  int i, imax;
  gavl_video_deinterlacer_t *deinterlacer;
  
  gavl_video_format_t format;
  gavl_video_frame_t * frame, * frame_1;

  gavl_video_options_t * opt;
    
  gavl_pixelformat_t csp;

  memset(&format, 0, sizeof(format));
  
  imax = gavl_num_pixelformats();
  deinterlacer = gavl_video_deinterlacer_create();

  opt = gavl_video_deinterlacer_get_options(deinterlacer);

  //  imax = 1;
  
  for(i = 0; i < imax; i++)
    {
    csp = gavl_get_pixelformat(i);
    
    //    csp = GAVL_RGB_24;
    
    fprintf(stderr, "Pixelformat: %s\n", gavl_pixelformat_to_string(csp));
        
    frame = read_png(argv[1], &format, csp);

    if(csp == GAVL_RGBA_64)
      {
      write_png("test.png", &format, frame);
      }

#if 0
    /* Write test frame */
    sprintf(filename_buffer, "%s-test.png", gavl_pixelformat_to_string(csp));
    write_png(filename_buffer, &format, frame);
#endif   
    
    gavl_video_options_set_defaults(opt);
    
    gavl_video_options_set_deinterlace_mode(opt,
                                            GAVL_DEINTERLACE_BLEND);
    gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_MMXEXT);
    
    gavl_video_deinterlacer_init(deinterlacer, &format);
    
    frame_1 = gavl_video_frame_create(&format);

    gavl_video_frame_clear(frame_1, &format);
    
    gavl_video_deinterlacer_deinterlace(deinterlacer, frame, frame_1);

    sprintf(filename_buffer, "%s-deinterlaced.png",
            gavl_pixelformat_to_string(csp));
    
    write_png(filename_buffer, &format, frame_1);
    fprintf(stderr, "Wrote %s\n", filename_buffer);
    gavl_video_frame_destroy(frame);
    gavl_video_frame_destroy(frame_1);
    }
  gavl_video_deinterlacer_destroy(deinterlacer);
  return 0;
  }
