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

// #define FLAG_HAS_BOM (1<<0)

struct gavl_charset_converter_s
  {
  iconv_t cd;
  char * to_charset;
  int flags;
  };

GAVL_PUBLIC
gavl_charset_converter_t * gavl_charset_converter_create(const char * from, const char * to)
  {
  gavl_charset_converter_t * ret = calloc(1, sizeof(*ret));

  if(!strcmp(from, GAVL_UTF_BOM))
    {
    ret->cd = (iconv_t)-1;
    ret->to_charset = gavl_strdup(to);
    }
  else
    {
    ret->cd = iconv_open(to, from);
    if(ret->cd == (iconv_t)-1)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "iconv_open failed: %s", strerror(errno));
      free(ret);
      return NULL;
      }
    }
  return ret;
  }

#define BYTES_INCREMENT 32
#define ZEROTERMINATE_SIZE    4

char * gavl_convert_string_to_buffer(gavl_charset_converter_t * cnv,
                                     const char * in_string1, int len, gavl_buffer_t * buf)
  
  {
  char *inbuf;
  char *outbuf;
  int alloc_size;
  int output_pos;
  char *in_string;
  
  size_t inbytesleft;
  size_t outbytesleft;
  int done = 0;
  int i;
  
  if(len < 0)
    len = strlen(in_string1);

  in_string = malloc(len + 4);
  memcpy(in_string, in_string1, len);
  memset(in_string + len, 0, 4);
  
  if(cnv->cd == (iconv_t)-1)
    {
    if((len > 1) &&
       ((uint8_t)in_string[0] == 0xff) &&
       ((uint8_t)in_string[1] == 0xfe))
      cnv->cd = iconv_open(cnv->to_charset, "UTF-16LE");
    /* Byte order Big Endian */
    else if((len > 1) &&
            ((uint8_t)in_string[0] == 0xfe) &&
            ((uint8_t)in_string[1] == 0xff))
      cnv->cd = iconv_open(cnv->to_charset, "UTF-16BE");
    /* UTF-8 */
    else if(!strcmp(cnv->to_charset, "UTF-8"))
      {
      buf->len = 0;
      gavl_buffer_append_data_pad(buf, (const uint8_t *)in_string, len, 1);
      free(in_string);
      return (char*)buf->buf;
      }
    else
      cnv->cd = iconv_open(cnv->to_charset, "UTF-8");
    }
    
  alloc_size = len + BYTES_INCREMENT;

  inbytesleft  = len;
  outbytesleft = alloc_size;
  
  gavl_buffer_alloc(buf, alloc_size);

  //  ret    = malloc(alloc_size);
  inbuf  = in_string;
  outbuf = (char*)buf->buf;
  
  while(!done)
    {
    if(iconv(cnv->cd, &inbuf, &inbytesleft,
             &outbuf, &outbytesleft) == (size_t)-1)
      {
      switch(errno)
        {
        case E2BIG:
          output_pos = (int)(outbuf - (char*)buf->buf);

          alloc_size   += (len + BYTES_INCREMENT);
          outbytesleft += (len + BYTES_INCREMENT);

          gavl_buffer_alloc(buf, alloc_size + ZEROTERMINATE_SIZE); // Make sure we can zero pad after
          
          outbuf = (char*)buf->buf + output_pos;
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

  buf->len = outbuf - (char*)buf->buf;

  free(in_string);
  
  return (char*)buf->buf;
  }


char * gavl_convert_string(gavl_charset_converter_t * cnv,
                           const char * str, int len,
                           int * out_len)
  {
  char * ret;
  gavl_buffer_t buf;
  gavl_buffer_init(&buf);
  ret = gavl_convert_string_to_buffer(cnv, str, len, &buf);
  if(out_len)
    *out_len = buf.len;
  return ret;
  }

void gavl_charset_converter_destroy(gavl_charset_converter_t * cnv)
  {
  if(cnv->cd != (iconv_t)-1)
    iconv_close(cnv->cd);

  if(cnv->to_charset)
    free(cnv->to_charset);
  
  free(cnv);
  }

