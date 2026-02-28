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


#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <nettle/md5.h>
#include <nettle/version.h>

#include <string.h>

void *
gavl_md5_buffer(const void *buffer, int len, void *resblock)
  {
  struct md5_ctx ctx;
  md5_init(&ctx);
  md5_update(&ctx, len, buffer);

#if NETTLE_VERSION_MAJOR > 3
    /* nettle >= 4.0 */
    md5_digest(&ctx, resblock);
#else
    /* nettle 3.x */
    md5_digest(&ctx, GAVL_MD5_SIZE, resblock);
#endif

  return resblock;
  }

char * gavl_md5_buffer_str(const void *buffer, int len, char * ret)
  {
  uint8_t md5[GAVL_MD5_SIZE];
  gavl_md5_buffer(buffer, len, md5);
  gavl_md5_2_string(md5, ret);
  return ret;
  }

static const char * hex_digits = "0123456789abcdef";

char * gavl_md5_2_string(const void * md5v, char * str)
  {
  int i;
  const uint8_t * md5 = md5v;
  for(i = 0; i < GAVL_MD5_SIZE; i++)
    {
    str[0] = hex_digits[ (*md5 >> 4) & 0xf  ];
    str[1] = hex_digits[ *md5 & 0xf ];
    str += 2;
    md5++;
    }
  *str = '\0';
  return str;
  }

int gavl_string_2_md5(const char * str, void * md5v)
  {
  const char * pos1;
  const char * pos2;
  int i;

  char * md5 = md5v;
  
  for(i = 0; i < GAVL_MD5_SIZE; i++)
    {
    pos1 = strchr(hex_digits, str[2*i]);
    pos2 = strchr(hex_digits, str[2*i+1]);

    if(!pos1 || !pos2)
      return 0;

    *md5 = ((int)(pos1 - hex_digits) << 4) | (int)(pos2 - hex_digits);
    md5++;
    }
  
  return 1;
  }
