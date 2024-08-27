/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2024 Members of the Gmerlin project
 * http://github.com/bplaum
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
#include <stdio.h>
#include <string.h>

#include <config.h>


#include <gavl.h>
#include <hw_v4l2.h>
#include <log.h>

#define LOG_DOMAIN "v4l_m2m_converter"


int main(int argc, char ** argv)
  {
  gavl_hw_context_t * hwctx;
  gavl_array_t devices;
  const gavl_dictionary_t * device_info;
  gavl_v4l2_device_t * dev;

  gavl_video_format_t in_fmt;
  gavl_video_format_t out_fmt;
  
  gavl_array_init(&devices);
  gavl_v4l2_devices_scan_by_type(GAVL_V4L2_DEVICE_CONVERTER, &devices);

  if(!devices.num_entries)
    return EXIT_FAILURE;

  if(!(device_info = gavl_value_get_dictionary(&devices.entries[0])))
    return EXIT_FAILURE;

  fprintf(stderr, "Got device:\n");
  gavl_dictionary_dump(device_info, 2);
  
  hwctx = gavl_hw_ctx_create_v4l2(device_info);
  dev = gavl_hw_ctx_v4l2_get_device(hwctx);

  memset(&in_fmt, 0, sizeof(in_fmt));
  memset(&out_fmt, 0, sizeof(out_fmt));

  in_fmt.image_width = 1920;
  in_fmt.image_height = 1080;
  in_fmt.pixel_width = 1;
  in_fmt.pixel_height = 1;
  in_fmt.pixelformat = GAVL_YUV_420_P;
  in_fmt.framerate_mode = GAVL_FRAMERATE_STILL;

  gavl_video_format_copy(&out_fmt, &in_fmt);

  out_fmt.image_width = 1280;
  out_fmt.image_height = 720;

  if(!gavl_v4l2_device_init_converter(dev, &in_fmt, &out_fmt))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Initializing converter failed");
    }
  
  }


