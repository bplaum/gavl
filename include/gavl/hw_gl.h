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



#ifndef GAVL_HW_GL_H_INCLUDED
#define GAVL_HW_GL_H_INCLUDED



// #include <gavl/gavldefs.h>

typedef struct
  {
  int num_textures;
  GLuint textures[GAVL_MAX_PLANES];
  GLenum texture_target;
  } gavl_gl_frame_info_t;

GAVL_PUBLIC int gavl_get_gl_format(gavl_pixelformat_t fmt, GLenum * format,
                                   GLenum * internalformat, GLenum * type);

GAVL_PUBLIC gavl_pixelformat_t * gavl_gl_get_image_formats(gavl_hw_context_t * ctx,
                                                           int * num);

GAVL_PUBLIC void gavl_gl_adjust_video_format(gavl_hw_context_t * ctx,
                                             gavl_video_format_t * fmt);


/* The following functions require a current GL context */
GAVL_PUBLIC gavl_video_frame_t * gavl_gl_create_frame(const gavl_video_format_t * fmt);

GAVL_PUBLIC void gavl_gl_destroy_frame(gavl_video_frame_t * f, int destroy_resource);


GAVL_PUBLIC void gavl_gl_frame_to_ram(const gavl_video_format_t * fmt,
                                      gavl_video_frame_t * dst,
                                      const gavl_video_frame_t * src);

GAVL_PUBLIC void gavl_gl_frame_to_hw(const gavl_video_format_t * fmt,
                                     gavl_video_frame_t * dst,
                                     const gavl_video_frame_t * src);


/* Generic utilities */

GAVL_PUBLIC const char * gavl_gl_get_error_string(GLenum err);
GAVL_PUBLIC int gavl_gl_log_error(const char * funcname);
GAVL_PUBLIC void gavl_gl_flush_errors(void);

#endif // GAVL_HW_GL_H_INCLUDED
