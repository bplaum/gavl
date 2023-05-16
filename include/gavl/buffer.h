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

#ifndef GAVL_BUFFER_H_INCLUDED
#define GAVL_BUFFER_H_INCLUDED


/* Buffer */

typedef struct
  {
  uint8_t * buf;
  int len;
  int alloc;
  int alloc_static;
  int pos;
  } gavl_buffer_t;

GAVL_PUBLIC
void gavl_buffer_init(gavl_buffer_t * buf);

GAVL_PUBLIC
void gavl_buffer_init_static(gavl_buffer_t * buf, uint8_t * data, int size);

GAVL_PUBLIC
int gavl_buffer_alloc(gavl_buffer_t * buf,
                      int size);

GAVL_PUBLIC
void gavl_buffer_free(gavl_buffer_t * buf);

GAVL_PUBLIC
void gavl_buffer_reset(gavl_buffer_t * buf);

GAVL_PUBLIC
void gavl_buffer_copy(gavl_buffer_t * dst, const gavl_buffer_t * src);

GAVL_PUBLIC
void gavl_buffer_append(gavl_buffer_t * dst, const gavl_buffer_t * src);

GAVL_PUBLIC
void gavl_buffer_append_pad(gavl_buffer_t * dst, const gavl_buffer_t * src, int padding);

GAVL_PUBLIC
void gavl_buffer_append_data(gavl_buffer_t * dst, const uint8_t * data, int len);

GAVL_PUBLIC
void gavl_buffer_append_data_pad(gavl_buffer_t * dst, const uint8_t * data, int len,
                                 int padding);


GAVL_PUBLIC
void gavl_buffer_prepend_data(gavl_buffer_t * dst, const uint8_t * data, int len);

GAVL_PUBLIC
void gavl_buffer_flush(gavl_buffer_t * buf, int len);

#endif // GAVL_BUFFER_H_INCLUDED
