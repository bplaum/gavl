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


#include <config.h>
#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/log.h>
#include <gavl/io.h>

#define LOG_DOMAIN "charset-test"


int main(int argc, char ** argv)
  {
  gavl_buffer_t buf;
  gavl_io_t * io;
  gavl_charset_converter_t * cnv;
  int in_len;
  char * out;
  int out_len;
  
  if(argc < 4)
    {
    fprintf(stderr, "Usage: %s <from> <to> <file>\n", argv[0]);
    return EXIT_SUCCESS;
    }

  if(!(cnv = gavl_charset_converter_create(argv[1], argv[2])))
    return EXIT_FAILURE;
  
  if(!(io = gavl_io_from_filename(argv[3], 0)))
    return EXIT_FAILURE;

  gavl_buffer_init(&buf);

  in_len = gavl_io_total_bytes(io);
  gavl_buffer_alloc(&buf, in_len);

  if((buf.len = gavl_io_read_data(io, buf.buf, in_len)) < in_len)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Loading file %s failed", argv[3]);
    return EXIT_FAILURE;
    }
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Loaded %s %d bytes", argv[3], in_len);
  
  out = gavl_convert_string(cnv, (char*)buf.buf, buf.len, &out_len);
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Converted to %s (%d bytes)", argv[2], out_len);
  
  fwrite(out, out_len, 1, stdout);

  free(out);
  gavl_charset_converter_destroy(cnv);
  gavl_io_destroy(io);

  gavl_buffer_free(&buf);
  
  }
