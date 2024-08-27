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
#include <string.h>

#include <gavl/metatags.h>

#include <gavfprivate.h>

/*
 *  Layout of a gavi file:
 *
 *  GAVLIMAG: Signature
 *  metadata
 *  format
 *  frame
 */

static const uint8_t sig[8] = "GAVLIMAG";

int gavl_image_write_header(gavl_io_t * io,
                            const gavl_dictionary_t * m,
                            const gavl_video_format_t * v)
  {
  int result;
  gavl_dictionary_t s;
  gavl_dictionary_init(&s);

  if((gavl_io_write_data(io, sig, 8) < 8))
    return 0;

  if(m)
    {
    if(!gavl_dictionary_set_dictionary(&s, GAVL_META_METADATA, m))
      return 0;
    }
  else
    gavl_dictionary_get_dictionary_create(&s, GAVL_META_METADATA);
  
  gavl_dictionary_set_video_format(&s, GAVL_META_FORMAT, v);
 
  result = gavl_dictionary_write(io, &s);
  gavl_dictionary_free(&s);
  return result;
  }

int gavl_image_write_image(gavl_io_t * io,
                           const gavl_video_format_t * v,
                           gavl_video_frame_t * f)
  {
  int len = gavl_video_format_get_image_size(v);
  if((gavl_io_write_data(io, f->planes[0], len) < len))
    return 0;
  return 1;
  }

int gavl_image_read_header(gavl_io_t * io,
                           gavl_dictionary_t * m,
                           gavl_video_format_t * v)
  {
  uint8_t sig_test[8];
  const gavl_video_format_t * fmt;
  gavl_dictionary_t s;
  gavl_dictionary_init(&s);
  
  if((gavl_io_read_data(io, sig_test, 8) < 8) ||
     memcmp(sig_test, sig, 8) ||
     !gavl_dictionary_read(io, &s) ||
     !(fmt = gavl_dictionary_get_video_format(&s, GAVL_META_FORMAT)))
    {
    gavl_dictionary_free(&s);
    return 0;
    }

  gavl_dictionary_free(&s);
  
  return 1;
  }

int gavl_image_read_image(gavl_io_t * io,
                          gavl_video_format_t * v,
                          gavl_video_frame_t * f)
  {
  int len = gavl_video_format_get_image_size(v);
  if(gavl_io_read_data(io, f->planes[0], len) < len)
    return 0;
  return 1;
  }
