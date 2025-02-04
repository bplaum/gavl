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


#include <string.h>
#include <stdlib.h>

#include <config.h>
#define LOG_DOMAIN "io_buffer"

#include <gavl/gavl.h>
#include <gavl/io.h>

typedef struct
  {
  gavl_buffer_t       * buf;
  const gavl_buffer_t * buf_c;
  int64_t pos;
  } buf_t;


static int read_buffer(void * priv, uint8_t * data, int len)
  {
  buf_t * b = priv;

  if(b->pos + len > b->buf_c->len)
    len = b->buf_c->len - b->pos;

  if(len > 0)
    {
    memcpy(data, b->buf_c->buf + b->pos, len);
    b->pos += len;
    return len;
    }
  else
    return 0;
  }

static int write_buffer(void * priv, const uint8_t * data, int len)
  {
  buf_t * b = priv;
  gavl_buffer_append_data(b->buf, data, len);
  return len;
  }

static int64_t seek_buffer(void * priv, int64_t pos, int whence)
  {
  int64_t real_pos = 0;

  buf_t * b = priv;
  
  switch(whence)
    {
    case SEEK_SET:
      real_pos = pos;
      break;
    case SEEK_CUR:
      real_pos = b->pos + pos;
      break;
    case SEEK_END:
      real_pos = b->buf_c->len + pos;
      break;
    }
  b->pos = real_pos;
  return b->pos;
  }

static void close_buffer(void * priv)
  {
  free(priv);
  }

gavl_io_t * gavl_io_create_buffer_write(gavl_buffer_t * buf)
  {
  buf_t * b = calloc(1, sizeof(*b));
  
  b->buf = buf;
  return gavl_io_create(NULL, write_buffer, NULL, close_buffer, NULL, GAVL_IO_CAN_READ, b);
  }

gavl_io_t * gavl_io_create_buffer_read(const gavl_buffer_t * buf)
  {
  buf_t * b = calloc(1, sizeof(*b));
  
  b->buf_c = buf;
  return gavl_io_create(read_buffer, NULL, seek_buffer, close_buffer, NULL, GAVL_IO_CAN_READ | GAVL_IO_CAN_SEEK, b);
  }
