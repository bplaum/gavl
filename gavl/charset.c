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

#include <iconv.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <config.h>
#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/log.h>
#define LOG_DOMAIN "charset"


struct gavl_charset_converter_s
  {
  iconv_t cd;
  };

GAVL_PUBLIC
gavl_charset_converter_t * gavl_charset_converter_create(const char * from, const char * to)
  {
  gavl_charset_converter_t * ret = calloc(1, sizeof(*ret));
  ret->cd = iconv_open(to, from);
  return ret;
  }

#define BYTES_INCREMENT 32
#define ZEROTERMINATE_SIZE    4

static char * do_convert(iconv_t cd, char * in_string, int len, int * out_len)
  {
  char * ret;
  
  char *inbuf;
  char *outbuf;
  int alloc_size;
  int output_pos;
  
  size_t inbytesleft;
  size_t outbytesleft;
  int done = 0;
  int i;
  
  alloc_size = len + BYTES_INCREMENT;

  inbytesleft  = len;
  outbytesleft = alloc_size;

  ret    = malloc(alloc_size);
  inbuf  = in_string;
  outbuf = ret;
  
  while(!done)
    {
    if(iconv(cd, &inbuf, &inbytesleft,
             &outbuf, &outbytesleft) == (size_t)-1)
      {
      switch(errno)
        {
        case E2BIG:
          output_pos = (int)(outbuf - ret);

          alloc_size   += (len + BYTES_INCREMENT);
          outbytesleft += (len + BYTES_INCREMENT);
          
          ret = realloc(ret, alloc_size + ZEROTERMINATE_SIZE); // Make sure we can zero pad after
          outbuf = &ret[output_pos];
          break;
        case EILSEQ:
          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Invalid multibyte sequence");
          done = 1;
          break;
        case EINVAL:
          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Incomplete multibyte sequence");
          done = 1;
          break;
        default:
          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got error: %s", strerror(errno));
          done = 1;
          break;
        }
      }
    if(!inbytesleft)
      break;
    }
  /* Zero terminate */
  for(i = 0; i < ZEROTERMINATE_SIZE; i++)
    outbuf[i] = '\0';
  
  if(out_len)
    *out_len = outbuf - ret;
  return ret;
  }


char * gavl_convert_string(gavl_charset_converter_t * cnv,
                           const char * str, int len,
                           int * out_len)
  {
  char * ret;
  char * tmp_string;

  if(len < 0)
    len = strlen(str);

  tmp_string = malloc(len+1);
  memcpy(tmp_string, str, len);
  tmp_string[len] = '\0';
  ret = do_convert(cnv->cd, tmp_string, len, out_len);
  free(tmp_string);
  return ret;
  }

void gavl_charset_converter_destroy(gavl_charset_converter_t * cnv)
  {
  iconv_close(cnv->cd);
  free(cnv);
  }

