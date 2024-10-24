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

#include <gavl/gavl.h>

#include <gavl/io.h>
#include <io_private.h>

typedef struct
  {
  uint8_t * buf;
  const uint8_t * buf_const;
  int pos;

  int len;
  int len_alloc;
  
  } mem_t;

static int read_mem(void * priv, uint8_t * data, int len)
  {
  mem_t * m = priv;
  int bytes = len;

  if(m->pos + bytes > m->len)
    bytes = m->len - m->pos;

  if(!bytes)
    return 0;
  memcpy(data, m->buf_const + m->pos, bytes);
  m->pos += bytes;
  return bytes;
  }

#define PAD_ALLOC(i) (((i)/512)+1)*512

static int write_mem(void * priv, const uint8_t * data, int len)
  {
  mem_t * m = priv;

  if(m->pos + len > m->len_alloc)
    {
    m->len_alloc = PAD_ALLOC(m->len + len);
    m->buf = realloc(m->buf, m->len_alloc);
    }

  memcpy(m->buf + m->pos, data, len);
  m->pos += len;
  if(m->len < m->pos)
    m->len = m->pos;
  
  //  if(ret < len)
  //    fprintf(stderr, "write failed: %d < %d\n", ret, len);
  return len;
  }

static int64_t seek_mem(void * priv, int64_t pos, int whence)
  {
  mem_t * m = priv;
  int64_t real_pos = 0;
  switch(whence)
    {
    case SEEK_SET:
      real_pos = pos;
      break;
    case SEEK_CUR:
      real_pos = m->pos + pos;
      break;
    case SEEK_END:
      real_pos = m->len + pos;
      break;
    }

  if(real_pos < 0)
    real_pos = 0;
  if(real_pos > m->len)
    real_pos = m->len;

  m->pos = real_pos;
  return m->pos;
  }

static void close_mem(void * priv)
  {
  mem_t * m = priv;
  if(m->buf)
    free(m->buf);
  free(m);
  }

GAVL_PUBLIC
uint8_t * gavl_io_mem_get_buf(gavl_io_t * io, int * len)
  {
  uint8_t * ret;
  mem_t * m = io->priv;

  *len = m->len;
  ret = m->buf;

  /* Cleanup */
  memset(m, 0, sizeof(*m));
  return ret;
  }

GAVL_PUBLIC
gavl_io_t * gavl_io_create_mem_read(const uint8_t * ptr, int len)
  {
  mem_t * m = calloc(1, sizeof(*m));
  m->buf_const = ptr;
  m->len = len;
  return gavl_io_create(read_mem, NULL, seek_mem, close_mem, NULL, GAVL_IO_CAN_READ | GAVL_IO_CAN_SEEK, m);
  }

GAVL_PUBLIC
gavl_io_t * gavl_io_create_mem_write()
  {
  mem_t * m = calloc(1, sizeof(*m));
  return gavl_io_create(NULL, write_mem, seek_mem, close_mem, NULL, GAVL_IO_CAN_WRITE | GAVL_IO_CAN_SEEK, m);
  }

