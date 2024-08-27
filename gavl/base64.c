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
#include <gavl/gavl.h>
#include <gavl/utils.h>

#include <nettle/base64.h>

static char * 
base64_encode_data_internal(const void * data, int length, int urlsafe)
  {
  char * ret;
  struct base64_encode_ctx ctx;
  int len_enc;
  if(urlsafe)
    base64url_encode_init(&ctx);
  else
    base64_encode_init(&ctx);

  if(length < 0)
    length = strlen(data);
  
  ret = malloc(BASE64_ENCODE_LENGTH(length) + BASE64_ENCODE_FINAL_LENGTH + 1);

  len_enc = base64_encode_update(&ctx, ret, length, data);
  len_enc += base64_encode_final(&ctx, ret + len_enc);
  ret[len_enc] = '\0';
  return ret;

  }

static int
base64_decode_data_internal(const char * str, gavl_buffer_t * ret, int urlsafe)
  {
  struct base64_decode_ctx ctx;
  size_t dst_length;
  int len = strlen(str);
  if(urlsafe)
    base64url_decode_init(&ctx);
  else
    base64_decode_init(&ctx);
  
  gavl_buffer_alloc(ret, BASE64_DECODE_LENGTH(len));

  base64_decode_update(&ctx,
                       &dst_length,
                       ret->buf,
                       len,
                       str);

  ret->len = dst_length;
  ret->pos = 0;
  
  base64_decode_final(&ctx);
  return 1;
  }

char * 
gavl_base64_encode_data(const void * data, int length)
  {
  return base64_encode_data_internal(data, length, 0);
  }

int
gavl_base64_decode_data(const char * str, gavl_buffer_t * ret)
  {
  return base64_decode_data_internal(str, ret, 0);
  }

char * 
gavl_base64_encode_data_urlsafe(const void * data, int length)
  {
  return base64_encode_data_internal(data, length, 1);
  }

int
gavl_base64_decode_data_urlsafe(const char * str, gavl_buffer_t * ret)
  {
  return base64_decode_data_internal(str, ret, 1);
  }

