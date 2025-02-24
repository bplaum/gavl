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

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/utils.h>


#define PAD_SIZE(s, m) \
  s = ((s + m - 1) / m) * m

void gavl_buffer_init(gavl_buffer_t * buf)
  {
  memset(buf, 0, sizeof(*buf));
  }

void
gavl_buffer_init_static(gavl_buffer_t * buf, uint8_t * data, int size)
  {
  gavl_buffer_init(buf);
  buf->alloc = size;
  buf->alloc_static = size;
  buf->buf = data;
  }

void gavl_buffer_reset(gavl_buffer_t * buf)
  {
  buf->len = 0;
  buf->pos = 0;
  }

int gavl_buffer_alloc(gavl_buffer_t * buf,
                      int size)
  {
  size++; // Zero terminate
  
  if(buf->alloc < size)
    {
    if(buf->alloc_static)
      return 0;
    
    buf->alloc = size;
    PAD_SIZE(buf->alloc, 1024);
    buf->buf = realloc(buf->buf, buf->alloc);
    if(!buf->buf)
      return 0;
    }
  
  // Zero terminate
  buf->buf[size - 1] = '\0';
  return 1;
  }

void gavl_buffer_free(gavl_buffer_t * buf)
  {
  if(buf->buf && !buf->alloc_static)
    free(buf->buf);
  }

void gavl_buffer_copy(gavl_buffer_t * dst, const gavl_buffer_t * src)
  {
  gavl_buffer_alloc(dst, src->alloc);
  memcpy(dst->buf, src->buf, src->alloc);
  dst->len = src->len;
  dst->pos = 0;
  }

void gavl_buffer_append_data_pad(gavl_buffer_t * dst, const uint8_t * data, int len, int padding)
  {
  gavl_buffer_alloc(dst, dst->len + len + padding);
  memcpy(dst->buf + dst->len, data, len);
  dst->len += len;
  if(padding)
    memset(dst->buf + dst->len, 0, padding);
  }

void gavl_buffer_append(gavl_buffer_t * dst, const gavl_buffer_t * src)
  {
  gavl_buffer_append_data_pad(dst, src->buf, src->len, 0);
  }

void gavl_buffer_append_pad(gavl_buffer_t * dst, const gavl_buffer_t * src, int padding)
  {
  gavl_buffer_append_data_pad(dst, src->buf, src->len, padding);
  }

void gavl_buffer_append_data(gavl_buffer_t * dst, const uint8_t * data, int len)
  {
  gavl_buffer_append_data_pad(dst, data, len, 0);
  }

void gavl_buffer_prepend_data(gavl_buffer_t * dst, const uint8_t * data, int len)
  {
  if(!len)
    return;
  
  gavl_buffer_alloc(dst, dst->len + len);
  if(dst->len)
    memmove(dst->buf + len, dst->buf, dst->len);
  memcpy(dst->buf, data, len);
  dst->len += len;
  }

void gavl_buffer_flush(gavl_buffer_t * buf, int len)
  {
  if(len < 0)
    return;
  
  if(len > buf->len)
    len = buf->len;
  
  if(buf->len > len)
    memmove(buf->buf, buf->buf + len, buf->len - len);
  buf->len -= len;
  
  buf->pos -= len;
  if(buf->pos < 0)
    buf->pos = 0;
  }
