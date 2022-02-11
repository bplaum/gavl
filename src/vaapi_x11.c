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

#include <string.h>
#include <X11/Xlib.h>
#include <va/va.h>

#include <gavl/gavl.h>
#include <gavl/hw_vaapi_x11.h>

int main(int argc, char ** argv)
  {
  gavl_video_frame_t * f;
  gavl_video_format_t fmt;
  gavl_hw_context_t * ctx;

  ctx = gavl_hw_ctx_create_vaapi_x11(NULL);

  memset(&fmt, 0, sizeof(fmt));
  fmt.image_width  = 320;
  fmt.image_height = 240;
  fmt.frame_width  = 320;
  fmt.frame_height = 240;
  fmt.pixel_width  = 1;
  fmt.pixel_height = 1;
  fmt.pixelformat = GAVL_YUV_420_P;

  f = gavl_hw_video_frame_create_ram(ctx, &fmt);

  gavl_video_frame_destroy(f);
  
  gavl_hw_ctx_destroy(ctx);
  return 0;
  }
