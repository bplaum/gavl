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
#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/io.h>

#include <gavl/log.h>
#define LOG_DOMAIN "urlvars"


/* URI variables */

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


/* Append url variables */
char * gavl_url_add_var_string(char * uri, const char * var, const char * val)
  {
  gavl_dictionary_t vars;
  gavl_dictionary_init(&vars);
  
  gavl_url_get_vars(uri, &vars);
  gavl_dictionary_set_string(&vars, var, val);
  uri =  gavl_url_append_vars(uri, &vars);
  gavl_dictionary_free(&vars);

  return uri;
  }

char * gavl_url_add_var_long(char * uri, const char * var, int64_t val)
  {
  char * ret;
  char * str = gavl_sprintf("%"PRId64, val);
  ret = gavl_url_add_var_string(uri, var, str);
  free(str);
  return ret;
  }


char * gavl_url_add_var_double(char * uri, const char * var, double val)
  {
  char * ret;
  char * str = gavl_sprintf("%f", val);
  ret = gavl_url_add_var_string(uri, var, str);
  free(str);
  return ret;
  }


/* Extract url variables */

/* *val = NULL if variable doesn't exist */
char * gavl_url_extract_var_string(char * uri, const char * var, char ** val)
  {
  gavl_dictionary_t vars;
  gavl_dictionary_init(&vars);
  
  gavl_url_get_vars(uri, &vars);

  *val = gavl_strdup(gavl_dictionary_get_string(&vars, var));
  gavl_dictionary_set(&vars, var, NULL);
  
  uri =  gavl_url_append_vars(uri, &vars);
  gavl_dictionary_free(&vars);
  
  return uri;
  }


char * gavl_url_extract_var_long(char * uri, const char * var, int64_t * val)
  {
  char * str = NULL;
  char * ret = gavl_url_extract_var_string(uri, var, &str);

  if(str)
    {
    *val = strtoll(str, NULL, 10);
    free(str);
    }
  else
    *val = 0;
  return ret;
  }

/* *val = -1 if variable doesn't exist */
char * gavl_url_extract_var_double(char * uri, const char * var, double * val)
  {
  char * str = NULL;
  char * ret = gavl_url_extract_var_string(uri, var, &str);

  if(str)
    {
    *val = strtod(str, NULL);
    free(str);
    }
  else
    *val = 0.0;
  return ret;
  
  }

