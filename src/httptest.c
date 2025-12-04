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
#include <gavl/http.h>
#include <gavl/log.h>
#define LOG_DOMAIN "httptest"

int main(int argc, char ** argv)
  {
  gavl_io_t * c;
  gavl_buffer_t buf;
  
  if(argc == 1)
    {
    fprintf(stderr, "Usage: %s uri\n", argv[0]);
    return EXIT_FAILURE;
    }

  gavl_buffer_init(&buf);
  
  c = gavl_http_client_create();

  gavl_http_client_set_response_body(c, &buf);
  
  if(!gavl_http_client_open(c, "GET", argv[1]))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "http GET failed");
    return EXIT_FAILURE;
    }
  
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Downloaded http body %d bytes", buf.len);
  gavl_buffer_free(&buf);
  
  }
