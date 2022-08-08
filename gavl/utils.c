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

#include <config.h>
#define _GNU_SOURCE

#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/http.h>


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <gavl/log.h>
#define LOG_DOMAIN "utils"

void gavl_dprintf(const char * format, ...)
  {
  va_list argp; /* arg ptr */
  va_start( argp, format);
  vfprintf(stderr, format, argp);
  va_end(argp);
  }

void gavl_diprintf(int indent, const char * format, ...)
  {
  int i;
  va_list argp; /* arg ptr */
  for(i = 0; i < indent; i++)
    gavl_dprintf( " ");
  
  va_start(argp, format);
  vfprintf(stderr, format, argp);
  va_end(argp);
  }

void gavl_hexdumpi(const uint8_t * data, int len, int linebreak, int indent)
  {
  int i;
  int bytes_written = 0;
  int imax;
  
  while(bytes_written < len)
    {
    for(i = 0; i < indent; i++)
      fprintf(stderr, " ");
    imax = (bytes_written + linebreak > len) ? len - bytes_written : linebreak;
    for(i = 0; i < imax; i++)
      fprintf(stderr, "%02x ", data[bytes_written + i]);
    for(i = imax; i < linebreak; i++)
      fprintf(stderr, "   ");
    for(i = 0; i < imax; i++)
      {
      if((data[bytes_written + i] < 0x7f) && (data[bytes_written + i] >= 32))
        fprintf(stderr, "%c", data[bytes_written + i]);
      else
        fprintf(stderr, ".");
      }
    bytes_written += imax;
    fprintf(stderr, "\n");
    }
  }

void gavl_hexdump(const uint8_t * data, int len, int linebreak)
  {
  gavl_hexdumpi(data, len, linebreak, 0);
  }

char * gavl_strrep(char * old_string,
                   const char * new_string)
  {
  char * ret;
  int len;
  if(!new_string || (*new_string == '\0'))
    {
    if(old_string)
      free(old_string);
    return NULL;
    }

  if(old_string)
    {
    if(!strcmp(old_string, new_string))
      return old_string;
    else
      free(old_string);
    }

  len = ((strlen(new_string)+1 + 3) / 4) * 4 ;
  
  ret = malloc(len);
  strcpy(ret, new_string);
  return ret;
  }

char * gavl_strnrep(char * old_string,
                    const char * new_string_start,
                    const char * new_string_end)
  {
  char * ret;
  if(!new_string_start || (*new_string_start == '\0'))
    {
    if(old_string)
      free(old_string);
    return NULL;
    }

  if(old_string)
    {
    if(!strncmp(old_string, new_string_start,
                new_string_end - new_string_start))
      return old_string;
    else
      free(old_string);
    }
  ret = malloc(new_string_end - new_string_start + 1);
  strncpy(ret, new_string_start, new_string_end - new_string_start);
  ret[new_string_end - new_string_start] = '\0';
  return ret;
  }

char * gavl_strdup(const char * new_string)
  {
  return gavl_strrep(NULL, new_string);
  }

char * gavl_strndup(const char * new_string,
                    const char * new_string_end)
  {
  return gavl_strnrep(NULL, new_string, new_string_end);
  }

char * gavl_strncat(char * old, const char * start, const char * end)
  {
  int len, old_len;
  old_len = old ? strlen(old) : 0;
  
  len = (end) ? (end - start) : strlen(start);
  old = realloc(old, len + old_len + 1);
  strncpy(old + old_len, start, len);
  old[old_len + len] = '\0';
  return old;
  }

char * gavl_strcat(char * old, const char * tail)
  {
  return gavl_strncat(old, tail, NULL);
  }

char * gavl_escape_string(char * old_string, const char * escape_chars)
  {
  char escape_seq[3];
  char * new_string = NULL;
  int i, done;

  const char * start;
  const char * end;
  const char * pos;

  int escape_len = strlen(escape_chars);

  /* 1st round: Check if the string can be passed unchanged */

  done = 1;
  for(i = 0; i < escape_len; i++)
    {
    if(strchr(old_string, escape_chars[i]))
      {
      done = 0;
      break;
      }
    }
  if(done)
    return old_string;

  /* 2nd round: Escape characters */

  escape_seq[0] = '\\';
  escape_seq[2] = '\0';

  start = old_string;
  end = start;

  done = 0;

  while(1)
    {
    /* Copy unescaped stuff */
    while(!strchr(escape_chars, *end) && (*end != '\0'))
      end++;

    if(end - start)
      {
      new_string = gavl_strncat(new_string, start, end);
      start = end;
      }

    if(*end == '\0')
      {
      free(old_string);
      return new_string;
      }
    /* Escape stuff */

    while((pos = strchr(escape_chars, *start)))
      {
      escape_seq[1] = *pos;
      new_string = gavl_strcat(new_string, escape_seq);
      start++;
      }
    end = start;
    if(*end == '\0')
      {
      free(old_string);
      return new_string;
      }
    }
  return NULL; // Never get here
  }

char * gavl_unescape_string(char * old_string, const char * escape_chars)
  {
  char * pos = old_string;
  int len = strlen(old_string);
  
  while(*pos != '\0')
    {
    if((*pos == '\\') && (*(pos+1) != '\0') && strchr(escape_chars, *(pos+1)))
      {
      /* len - 1 + 1 (don't count backslash but count '\0') */
      memmove(pos, pos+1, len);
      }
    else
      {
      pos++;
      }

    len--;
    }
  return old_string;
  }

int gavl_string_starts_with(const char * str, const char * start)
  {
  return !strncmp(str, start, strlen(start)) ? 1 : 0;
  }

int gavl_string_starts_with_i(const char * str, const char * start)
  {
  return !strncasecmp(str, start, strlen(start)) ? 1 : 0;
  }

int gavl_string_ends_with(const char * str, const char * end)
  {
  int slen = strlen(str);
  int elen = strlen(end);

  if(slen < elen)
    return 0;
  return !strcmp(str + slen - elen, end) ? 1 : 0;
  }

int gavl_string_ends_with_i(const char * str, const char * end)
  {
  int slen = strlen(str);
  int elen = strlen(end);

  if(slen < elen)
    return 0;
  return !strcasecmp(str + slen - elen, end) ? 1 : 0;
  }

char * gavl_strtrim(char * str)
  {
  int len;
  char * pos = str;

  //  fprintf(stderr, "gavl_strtrim 1 %s\n", str);
  
  while(isspace(*pos) && (*pos != '\0'))
    pos++;

  if(*pos == '\0') // String consists of whitespace
    {
    *str = '\0';
    return str;
    }

  if(pos > str)
    memmove(str, pos, strlen(pos) + 1);

  /* Trailing */

  len = strlen(str);

  if(!len)
    return str;
  
  pos = str + (len-1);

  //  fprintf(stderr, "gavl_strtrim 2 %s %s\n", str, pos);

  while(pos >= str)
    {
    if(isspace(*pos))
      *pos = '\0';
    else
      break;
    pos--;
    }
  return str;
  }


/* TODO */
char *
gavl_sprintf(const char * format,...)
  {
  va_list argp; /* arg ptr */
#ifndef HAVE_VASPRINTF
  int len;
#endif
  char * ret;
  va_start( argp, format);

#ifndef HAVE_VASPRINTF
  len = vsnprintf(NULL, 0, format, argp);
  ret = malloc(len+1);
  vsnprintf(ret, len+1, format, argp);
#else
  vasprintf(&ret, format, argp);
#endif
  va_end(argp);
  return ret;
  }

static char * strip_space(char * str, int do_free)
  {
  char * pos = str;

  while(isspace(*pos) && *pos != '\0')
    pos++;

  if((*pos == '\0'))
    {
    if(do_free)
      {
      free(str);
      return NULL;
      }
    else
      {
      *str = '\0';
      return str;
      }
    }
  
  if(pos > str)
    memmove(str, pos, strlen(pos)+1);

  pos = str + (strlen(str)-1);
  while(isspace(*pos))
    pos--;

  pos++;
  *pos = '\0';
  return str;
  }


char * gavl_strip_space(char * str)
  {
  return strip_space(str, 1);
  }

void gavl_strip_space_inplace(char * str)
  {
  strip_space(str, 0);
  }

/* Support escaped delimiters (with backslash) */

const char * gavl_find_char_c(const char * start, char delim)
  {
  const char * pos = start;

  while(*pos != '\0')
    {
    if(*pos == delim)
      {
      if(pos > start)
        {
        if(*(pos-1) != '\\')
          return pos;
        }
      else
        return pos;
      }
    pos++;
    }
  return NULL;
  }

char * gavl_find_char(char * start, char delim)
  {
  char * pos = start;

  while(*pos != '\0')
    {
    if(*pos == delim)
      {
      if(pos > start)
        {
        if(*(pos-1) != '\\')
          return pos;
        }
      else
        return pos;
      }
    pos++;
    }
  return NULL;
  }



char ** gavl_strbreak(const char * str, char delim)
  {
  int num_entries;
  char *pos, *end = NULL;
  const char *pos_c;
  char ** ret;
  int i;

  char escape_chars[2];
  escape_chars[0] = delim;
  escape_chars[1] = '\0';
  
  if(!str || (*str == '\0'))
    return NULL;
    
  pos_c = str;
  
  num_entries = 1;
  while((pos_c = gavl_find_char_c(pos_c, delim)))
    {
    num_entries++;
    pos_c++;
    }
  ret = calloc(num_entries+1, sizeof(char*));

  ret[0] = gavl_strdup(str);
  
  pos = ret[0];
  for(i = 0; i < num_entries; i++)
    {
    if(i)
      {
      ret[i] = pos;
      }
    if(i < num_entries-1)
      {
      end = gavl_find_char(pos, delim);
      *end = '\0';
      }
    end++;
    pos = end;
    }

  for(i = 0; i < num_entries; i++)
    {
    strip_space(ret[i], 0);
    gavl_unescape_string(ret[i], escape_chars);
    }
  return ret;
  }

void gavl_strbreak_free(char ** retval)
  {
  free(retval[0]);
  free(retval);
  }

const char * gavl_tempdir()
  {
  char * ret = getenv("TMPDIR");

  if(!ret)
    ret = getenv("TEMP");

  if(!ret)
    ret = getenv("TMP");

  if(ret)
    return ret;

  if(!access("/tmp", R_OK|W_OK))
    return "/tmp";
  return ".";
  }

int gavl_url_split(const char * url,
                 char ** protocol,
                 char ** user,
                 char ** password,
                 char ** hostname,
                 int * port,
                 char ** path)
  {
  const char * pos1;
  const char * pos2;

  /* For detecting user:pass@blabla.com/file */

  const char * colon_pos;
  const char * at_pos;
  const char * slash_pos;
  
  pos1 = url;

  /* Sanity check */
  
  pos2 = strstr(url, "://");
  if(!pos2)
    return 0;

  /* Protocol */
    
  if(protocol)
    *protocol = gavl_strndup( pos1, pos2);

  pos2 += 3;
  pos1 = pos2;

  /* Check for user and password */

  colon_pos = strchr(pos1, ':');
  at_pos = strchr(pos1, '@');
  slash_pos = strchr(pos1, '/');

  if(!slash_pos)
    slash_pos = pos1 + strlen(pos1);
  
  if(colon_pos && at_pos && at_pos &&
     (colon_pos < at_pos) && 
     (at_pos < slash_pos))
    {
    if(user)
      *user = gavl_strndup( pos1, colon_pos);
    pos1 = colon_pos + 1;
    if(password)
      *password = gavl_strndup( pos1, at_pos);
    pos1 = at_pos + 1;
    pos2 = pos1;
    }
  
  /* Hostname */

  if(*pos1 == '[') // IPV6
    {
    pos1++;
    pos2 = strchr(pos1, ']');
    if(!pos2)
      return 0;

    if(hostname)
      *hostname = gavl_strndup( pos1, pos2);
    pos2++;
    }
  else
    {
    while((*pos2 != '\0') && (*pos2 != ':') && (*pos2 != '/'))
      pos2++;
    if(hostname)
      *hostname = gavl_strndup( pos1, pos2);
    }
  
  switch(*pos2)
    {
    case '\0':
      if(port)
        *port = -1;
      return 1;
      break;
    case ':':
      /* Port */
      pos2++;
      if(port)
        *port = atoi(pos2);
      while(isdigit(*pos2))
        pos2++;
      break;
    default:
      if(port)
        *port = -1;
      break;
    }

  if(path)
    {
    pos1 = pos2;
    pos2 = pos1 + strlen(pos1);
    if(pos1 != pos2)
      *path = gavl_strndup( pos1, pos2);
    else
      *path = NULL;
    }
  return 1;
  }

/*
 *  Split off vars like path?var1=val1&var2=val2#fragment
 */

static void url_vars_to_metadata(const char * pos, gavl_dictionary_t * vars)
  {
  int i;
  char ** str;
  i = 0;
  
  str = gavl_strbreak(pos, '&');

  if(!str)
    return;
  
  while(str[i])
    {
    char * key;
    pos = strchr(str[i], '=');
    if(!pos)
      gavl_dictionary_set_int(vars, str[i], 1);
    else
      {
      key = gavl_strndup(str[i], pos);
      pos++;
      
      if(*key != '\0')
        gavl_dictionary_set_string(vars, key, pos);
      
      free(key);
      }
    i++;
    }
  gavl_strbreak_free(str);
  }


void gavl_url_get_vars_c(const char * path,
                         gavl_dictionary_t * vars)
  {
  const char * pos = strrchr(path, '?');
  if(!pos)
    return;
  pos++;

  url_vars_to_metadata(pos, vars);
  }
 
void gavl_url_get_vars(char * path,
                       gavl_dictionary_t * vars)
  {
  char * pos;
  
  pos = strrchr(path, '?');
  if(!pos)
    return;

  *pos = '\0';
  
  if(!vars)
    return;
  
  pos++;

  if(vars)
    url_vars_to_metadata(pos, vars);
  }

char * gavl_url_append_vars(char * path,
                            const gavl_dictionary_t * vars)
  {
  char sep;
  char * tmp_string;
  char * val_string;
  int i;
  int idx = 0;
  
  for(i = 0; i < vars->num_entries; i++)
    {
    if(!strchr(path, '?'))
      sep = '?';
    else
      sep = '&';

    if(!(val_string = gavl_value_to_string(&vars->entries[i].v)))
      {
      if(vars->entries[i].v.type != GAVL_TYPE_STRING)
        {
        gavl_log(GAVL_LOG_WARNING, LOG_DOMAIN, "Value type %s of var %s not supported in URL variables",
                 gavl_type_to_string(vars->entries[i].v.type), vars->entries[i].name);
        continue;
        }
      }
    
    if(!val_string)
      tmp_string = gavl_sprintf("%c%s=", sep, vars->entries[i].name);
    else
      tmp_string = gavl_sprintf("%c%s=%s", sep, vars->entries[i].name, val_string);
    idx++;
    
    path = gavl_strcat(path, tmp_string);
    
    free(tmp_string);

    if(val_string)
      free(val_string);
    }
  return path;
  }

#define HTTP_VARS_KEY "gavlhttpvars"

char * gavl_url_extract_http_vars(char * url, gavl_dictionary_t * vars)
  {
  const char * str;
  gavl_dictionary_t url_vars;
  gavl_dictionary_init(&url_vars);
  gavl_url_get_vars(url, &url_vars);

  if((str = gavl_dictionary_get_string(&url_vars, HTTP_VARS_KEY)))
    {
    gavl_buffer_t buf;
    gavl_buffer_init(&buf);
    gavl_base64_decode_data_urlsafe(str, &buf);

    if(!gavl_dictionary_from_buffer(buf.buf, buf.len, vars))
      gavl_dictionary_reset(vars);
    
    gavl_buffer_free(&buf);

    gavl_dictionary_set(&url_vars, HTTP_VARS_KEY, NULL);
    }

  url = gavl_url_append_vars(url, &url_vars);
  
  gavl_dictionary_free(&url_vars);
  return url;
  }

char * gavl_url_append_http_vars(char * url, const gavl_dictionary_t * vars)
  {
  gavl_dictionary_t url_vars;
  gavl_dictionary_t http_vars;
  gavl_buffer_t buf;
  const char * str;
  uint8_t * buf_new = NULL;
  int len_new;
  char * str_new;

  if(!vars->num_entries)
    return url;
  
  gavl_dictionary_init(&url_vars);
  gavl_dictionary_init(&http_vars);
  gavl_buffer_init(&buf);
  
  gavl_url_get_vars(url, &url_vars);

  if((str = gavl_dictionary_get_string(&url_vars, HTTP_VARS_KEY)))
    {
    gavl_base64_decode_data_urlsafe(str, &buf);
    
    if(!gavl_dictionary_from_buffer(buf.buf, buf.len, &http_vars))
      gavl_dictionary_reset(&http_vars);
    
    gavl_buffer_reset(&buf);
    
    }
  
  gavl_dictionary_merge2(&http_vars, vars);

  buf_new = gavl_dictionary_to_buffer(&len_new, &http_vars);
  str_new = gavl_base64_encode_data_urlsafe(buf_new, len_new);

  gavl_dictionary_set_string_nocopy(&url_vars, HTTP_VARS_KEY, str_new);

  url = gavl_url_append_vars(url, &url_vars);
  
  /* Cleanup */
  gavl_buffer_free(&buf);
  free(buf_new);

  gavl_dictionary_free(&url_vars);
  gavl_dictionary_free(&http_vars);
  
  return url;
  }

char * gavl_get_absolute_uri(const char * rel_uri, const char * abs_uri)
  {
  const char * end;
  char * tmp_str;
  char * ret = NULL;
  const char * pos;

  if(strstr(rel_uri, "://"))
    return gavl_strdup(rel_uri);
  
  if((pos = strstr(abs_uri, "://")))
    {
    pos += 3;
    
    if(rel_uri[0] == '/')
      {
      end = strchr(pos, '/');
      if(!end)
        end = strchr(pos, '?');
      if(!end)
        end = strchr(pos, '#');

      if(!end)
        ret = gavl_sprintf("%s%s", abs_uri, rel_uri);
      else
        {
        tmp_str = gavl_strndup(abs_uri, end);
        ret = gavl_sprintf("%s%s", tmp_str, rel_uri);
        free(tmp_str);
        }
      }
    else
      {
      end = strrchr(pos, '/');

      if(!end)
        ret = gavl_sprintf("%s/%s", abs_uri, rel_uri);
      else
        {
        tmp_str = gavl_strndup(abs_uri, end);
        ret = gavl_sprintf("%s/%s", tmp_str, rel_uri);
        free(tmp_str);
        }
      }
    
    }
  else if(abs_uri[0] == '/')
    {
    if(rel_uri[0] == '/')
      return gavl_strdup(rel_uri);
    
    if((end = strrchr(abs_uri, '/')))
      {
      tmp_str = gavl_strndup(abs_uri, end);
      ret = gavl_sprintf("%s/%s", tmp_str, rel_uri);
      free(tmp_str);
      }
    else
      ret = gavl_sprintf("%s/%s", abs_uri, rel_uri);
    }
  
  return ret;

  }
