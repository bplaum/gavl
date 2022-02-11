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

struct gavl_framerate_converter_s
  {
  gavl_time_t input_time_1;
  gavl_time_t input_time_2;
  gavl_time_t next_output_time;

  gavl_video_frame_t * input_frame_1;
  gavl_video_frame_t * input_frame_2;
  
  double output_framerate;
  int64_t output_frame_count;
  int do_resync;
  };

gavl_framerate_converter_t * gavl_framerate_converter_create()
  {
  gavl_framerate_converter_t * ret;
  ret = calloc(1, sizeof(*ret));
  return ret;
  }

void
gavl_framerate_converter_init(gavl_framerate_converter_t * cnv,
                              double output_framerate)
  {
  cnv->output_framerate = output_framerate;
  cnv->output_frame_count = 0;
  cnv->do_resync = 1;
  }

void gavl_framerate_converter_reset(gavl_framerate_converter_t * cnv)
  {
  cnv->input_frame_1 = NULL;
  cnv->input_frame_2 = NULL;
  cnv->do_resync = 1;
  }

/*
 * Convert framerate:
 * input are 2 subsequent frames with valid timestamps
 * output is one of the input frames with corrected timestamps
 *
 * The timestamps of the input frames should be considered
 * undefined afterwards
 */

/*
#define GAVL_FRAMERATE_INPUT_1_DONE (1<<0)
#define GAVL_FRAMERATE_INPUT_2_DONE (1<<1)
#define GAVL_FRAMERATE_OUTPUT_DONE  (1<<2)
*/


int gavl_framerate_convert(gavl_framerate_converter_t * cnv,
                           gavl_video_frame_t * input_frame_1,
                           gavl_video_frame_t * input_frame_2,
                           gavl_video_frame_t ** output_frame)
  {
  int ret = 0;

  /* Correct times */

  if(cnv->input_frame_1 != input_frame_1)
    {
    cnv->input_frame_1 = input_frame_1;
    cnv->input_time_1 = cnv->input_frame_1->time;
    }
  if(cnv->input_frame_2 != input_frame_2)
    {
    cnv->input_frame_2 = input_frame_2;
    cnv->input_time_2 = cnv->input_frame_2->time;
    }

  if(cnv->do_resync)
    {
    cnv->output_frame_count = gavl_time_to_frames(cnv->output_framerate,
                                                  cnv->input_time_1);
    cnv->next_output_time = gavl_frames_to_time(cnv->output_framerate,
                                                cnv->output_frame_count);
    cnv->do_resync = 0;
    }

  if(cnv->next_output_time > cnv->input_time_2)
    {
    ret |= GAVL_FRAMERATE_INPUT_DONE;
    }
  /* Ill conditioned case: Repeat first frame, until
     our time reaches timestamp of first first frame */
  
  else if(cnv->next_output_time < cnv->input_time_1)
    {
    *output_frame = input_frame_1;
    (*output_frame)->time = cnv->next_output_time;
    ret |= GAVL_FRAMERATE_OUTPUT_DONE;
    }
  /* Our time is between the input time, take the nearest frame */
  else
    {
    if(cnv->next_output_time - cnv->input_time_1 <
       cnv->input_time_2 - cnv->next_output_time)
      *output_frame = input_frame_1;
    else
      *output_frame = input_frame_1;
    (*output_frame)->time = cnv->next_output_time;
    ret |= GAVL_FRAMERATE_OUTPUT_DONE;
    }
  
  cnv->output_frame_count++;

  cnv->next_output_time = gavl_frames_to_time(cnv->output_framerate,
                                              cnv->output_frame_count);
  return ret;

  }

void gavl_framerate_converter_destroy(gavl_framerate_converter_t * cnv)
  {
  free(cnv);
  }
