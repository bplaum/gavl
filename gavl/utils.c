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



#include <config.h>
#define _GNU_SOURCE

#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/http.h>

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
  char * new_string = NULL;
  int i;
  char * dst_pos;
  
  int new_len;
  int old_len = strlen(old_string);

  new_len = old_len;
  
  /* 1st round: Count replacements */

  for(i = 0; i < old_len; i++)
    {
    if(strchr(escape_chars, old_string[i]))
      new_len++;
    }
  
  if(new_len == old_len)
    return old_string;

  new_string = malloc(new_len+1);
  
  dst_pos = new_string;
  
  for(i = 0; i < old_len; i++)
    {
    if(strchr(escape_chars, old_string[i]))
      *(dst_pos++) = '\\';

    *(dst_pos++) = old_string[i];
    }
  *dst_pos = '\0';

  free(old_string);
  return new_string;
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
  else
    ret = gavl_strdup(rel_uri);
  
  return ret;
  }

// https://stackoverflow.com/questions/44668774/reduce-fractions-in-c

static int gcd(int x, int y)
  {
  int temp;
  
  while (y != 0)
    {
    temp = y;
    y = x % y;
    x = temp;
    }
  return x;
  }

void gavl_simplify_rational(int * num, int * den)
  {
  int fac = gcd(*num, *den);
  
  *num /= fac;
  *den /= fac;
  }

#undef LOG_DOMAIN
#define LOG_DOMAIN "cpus"

int gavl_num_cpus()
  {
  cpu_set_t mask;

  if(sched_getaffinity(0, sizeof(cpu_set_t), &mask) == -1)
    {
    gavl_log(GAVL_LOG_WARNING, LOG_DOMAIN,
             "Could not obtain the CPU affinity: %s (switching to single threaded mode)",
           strerror(errno));
    return 1;
    }
  return CPU_COUNT(&mask);
  }

#undef LOG_DOMAIN


#define LOG_DOMAIN "utils"

int gavl_fd_can_read(int fd, int milliseconds)
  {
  int result;
  fd_set set;
  struct timeval timeout;
  FD_ZERO (&set);
  FD_SET  (fd, &set);

  timeout.tv_sec  = milliseconds / 1000;
  timeout.tv_usec = (milliseconds % 1000) * 1000;
    
  if((result = select(fd+1, &set, NULL, NULL, &timeout) <= 0))
    {
    if(result < 0 && (errno == EINVAL))
      {
      fprintf(stderr, "EINVAL %d\n", fd);
      }
    
    if(result < 0)
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Select for reading failed: %s", strerror(errno));
    return 0;
    }

  
  return 1;
  }


int gavl_fd_can_write(int fd, int milliseconds)
  {
  int result;
  fd_set set;
  struct timeval timeout;
  FD_ZERO (&set);
  FD_SET  (fd, &set);

  timeout.tv_sec  = milliseconds / 1000;
  timeout.tv_usec = (milliseconds % 1000) * 1000;
    
  if((result = select(fd+1, NULL, &set, NULL, &timeout) <= 0))
    {
    if(result < 0)
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Select for writing failed: %s", strerror(errno));
    // gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got read timeout");
    return 0;
    }
  return 1;
  }

int gavl_fd_set_block(int fd, int block)
  {
  int flags;

  if(!block)
    flags = O_NONBLOCK;
  else
    flags = 0;

  if(fcntl(fd, F_SETFL, flags) < 0)
    {
    if(block)
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot set blocking mode: %s", strerror(errno));
    else
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot set non-blocking mode: %s", strerror(errno));
    return 0;
    }
  return 1;
  }

int gavl_fd_get_block(int fd)
  {
  return !(fcntl(fd, F_GETFL, 0) & O_NONBLOCK);
  }


int gavl_host_is_us(const char * hostname)
  {
  char str[HOST_NAME_MAX+1];

  gethostname(str, HOST_NAME_MAX+1);

  if(!strcmp(hostname, str))
    return 1;
  else
    return 0;
  }


int gavl_is_directory(const char * dir, int wr)
  {
  
  struct stat st;

  if(stat(dir, &st) || !S_ISDIR(st.st_mode))
    return 0;


  if(wr)
    {
    if(!access(dir, R_OK|W_OK|X_OK))
      return 1;
    }
  else if(!access(dir, R_OK|X_OK))
    return 1;
  
  return 0;
  }

int gavl_ensure_directory(const char * dir, int priv)
  {
  char ** directories;
  char * subpath = NULL;
  int i, ret;
  int absolute;
  
  /* Return early */

  if(gavl_is_directory(dir, 1))
    return 1;
  
  if(dir[0] == '/')
    absolute = 1;
  else
    absolute = 0;
  
  /* We omit the first slash */
  
  if(absolute)
    directories = gavl_strbreak(dir+1, '/');
  else
    directories = gavl_strbreak(dir, '/');
  
  i = 0;
  ret = 1;
  while(directories[i])
    {
    if(i || absolute)
      subpath = gavl_strcat(subpath, "/");

    subpath = gavl_strcat(subpath, directories[i]);

    if(access(subpath, R_OK) && (errno == ENOENT))
      {
      mode_t mode = S_IRUSR|S_IWUSR|S_IXUSR;
      if(!priv)
        mode |= S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH;
      
      if(mkdir(subpath, mode) == -1)
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Creating directory %s failed: %s",
               subpath, strerror(errno));
        ret = 0;
        break;
        }
      else
        gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Created directory %s", subpath);
      }
    i++;
    }
  if(subpath)
    free(subpath);
  gavl_strbreak_free(directories);
  return ret;
  }

char * gavl_search_cache_dir(const char * package, const char * app, const char * directory)
  {
  const char * var;
  char * ret;
  
  if((var = getenv("XDG_CACHE_HOME")))
    ret = gavl_sprintf("%s/%s", var, package);
  else if((var = getenv("HOME")))
    ret = gavl_sprintf("%s/.cache/%s", var, package);
  else
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot get cache directory: neither HOME nor XDG_CONFIG_HOME is defined");
    return NULL;
    }

  if(app)
    {
    ret = gavl_strcat(ret, "/");
    ret = gavl_strcat(ret, app);
    }
  if(directory)
    {
    ret = gavl_strcat(ret, "/");
    ret = gavl_strcat(ret, directory);
    }
  
  if(!gavl_ensure_directory(ret, 1))
    {
    free(ret);
    return NULL;
    }
  return ret;
  }

char * gavl_search_config_dir(const char * package, const char * app, const char * directory)
  {
  const char * var;
  char * ret;
  
  if((var = getenv("XDG_CONFIG_HOME")))
    ret = gavl_sprintf("%s/%s", var, package);
  else if((var = getenv("HOME")))
    ret = gavl_sprintf("%s/.config/%s", var, package);
  else
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot get configuration directory: neither HOME nor XDG_CONFIG_HOME is defined");
    return NULL;
    }

  if(app)
    {
    ret = gavl_strcat(ret, "/");
    ret = gavl_strcat(ret, app);
    }
  if(directory)
    {
    ret = gavl_strcat(ret, "/");
    ret = gavl_strcat(ret, directory);
    }
  
  if(!gavl_ensure_directory(ret, 0))
    {
    free(ret);
    return NULL;
    }
  return ret;
  }

char * gavl_search_state_dir(const char * package, const char * app, const char * directory)
  {
  const char * var;
  char * ret;

  if((var = getenv("XDG_STATE_HOME")))
    ret = gavl_sprintf("%s/%s", var, package);
  else if((var = getenv("HOME")))
    ret = gavl_sprintf("%s/.local/state/%s", var, package);
  else
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot get state directory: neither HOME nor XDG_CONFIG_HOME is defined");
    return NULL;
    }
  
  if(app)
    {
    ret = gavl_strcat(ret, "/");
    ret = gavl_strcat(ret, app);
    }
  
  if(directory)
    {
    ret = gavl_strcat(ret, "/");
    ret = gavl_strcat(ret, directory);
    }
  
  if(!gavl_ensure_directory(ret, 0))
    {
    free(ret);
    return NULL;
    }
  return ret;
  }

char * gavl_search_data_dir_nocreate(const char * package, const char * app, const char * directory)
  {
  const char * var;
  char * ret;

  if((var = getenv("XDG_DATA_HOME")))
    ret = gavl_sprintf("%s/%s", var, package);
  else if((var = getenv("HOME")))
    ret = gavl_sprintf("%s/.local/share/%s", var, package);
  else
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot get data directory: neither HOME nor XDG_CONFIG_HOME is defined");
    return NULL;
    }
  
  if(app)
    {
    ret = gavl_strcat(ret, "/");
    ret = gavl_strcat(ret, app);
    }
  
  if(directory)
    {
    ret = gavl_strcat(ret, "/");
    ret = gavl_strcat(ret, directory);
    }
  return ret;
  }

char * gavl_search_data_dir(const char * package, const char * app, const char * directory)
  {
  char *  ret =
    gavl_search_data_dir_nocreate(package, app, directory);
  
  if(!gavl_ensure_directory(ret, 0))
    {
    free(ret);
    return NULL;
    }
  return ret;
  }

/* 2D coordinate transforms */

void gavl_set_orient_matrix_inv(gavl_image_orientation_t orient, double mat[2][3])
  {
  mat[0][2] = 0.0;
  mat[1][2] = 0.0;
  
  switch(orient)
    {
    case GAVL_IMAGE_ORIENT_NORMAL:  // EXIF: 1
      mat[0][0] = 1.0; mat[0][1] = 0.0;
      mat[1][0] = 0.0; mat[1][1] = 1.0;
      break;
    case GAVL_IMAGE_ORIENT_ROT90_CW:  // EXIF: 8
      mat[0][0] =  0.0; mat[0][1] = -1.0;
      mat[1][0] =  1.0; mat[1][1] =  0.0;
      break;
    case GAVL_IMAGE_ORIENT_ROT180_CW: // EXIF: 3
      mat[0][0] = -1.0; mat[0][1] =  0.0;
      mat[1][0] =  0.0; mat[1][1] = -1.0;
      break;
    case GAVL_IMAGE_ORIENT_ROT270_CW: // EXIF: 6
      mat[0][0] =  0.0; mat[0][1] =  1.0;
      mat[1][0] = -1.0; mat[1][1] =  0.0;
      break;
    case GAVL_IMAGE_ORIENT_FH:       // EXIF: 2
      mat[0][0] = -1.0; mat[0][1] = 0.0;
      mat[1][0] = 0.0;  mat[1][1] = 1.0;
      break;
    case GAVL_IMAGE_ORIENT_FH_ROT90_CW:  // EXIF: 7
      mat[0][0] =  0.0;  mat[0][1] =  1.0;
      mat[1][0] =  1.0;  mat[1][1] =  0.0;
      break;
    case GAVL_IMAGE_ORIENT_FH_ROT180_CW: // EXIF: 4
      mat[0][0] =  1.0;  mat[0][1] =  0.0;
      mat[1][0] =  0.0;  mat[1][1] = -1.0;
      break;
    case GAVL_IMAGE_ORIENT_FH_ROT270_CW: // EXIF: 5
      mat[0][0] =  0.0;  mat[0][1] =  -1.0;
      mat[1][0] =  -1.0; mat[1][1] =  0.0;
      break;
    default: // Keeps gcc quiet
      mat[0][0] = 1.0;   mat[0][1] = 0.0;
      mat[1][0] = 0.0;   mat[1][1] = 1.0;
      break;
    }
  }

void gavl_set_orient_matrix(gavl_image_orientation_t orient, double mat[2][3])
  {
  double tmp[2][3];
  gavl_set_orient_matrix_inv(orient, tmp);
  gavl_2d_transform_invert(tmp, mat);
  
  }

void 
gavl_2d_transform_invert(const double matrix[2][3], double inverse[2][3])
  {
  // Extrahiere die 2x2 Transformationsmatrix
  double a = matrix[0][0];
  double b = matrix[0][1];
  double c = matrix[1][0];
  double d = matrix[1][1];
    
  // Extrahiere die Translationskomponenten
  double tx = matrix[0][2];
  double ty = matrix[1][2];
    
  // Berechne die Determinante der 2x2 Matrix
  double det = a * d - b * c;

  double inv_det;
  double inv_a;
  double inv_b;
  double inv_c;
  double inv_d;
  
  double inv_tx;
  double inv_ty;

  
  // Test for singularity
  if(det == 0.0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Matrix is singular");
    return;
    }
  
  // Berechne die Inverse der 2x2 Matrix
  inv_det = 1.0 / det;
  inv_a = d * inv_det;
  inv_b = -b * inv_det;
  inv_c = -c * inv_det;
  inv_d = a * inv_det;
    
  // Berechne die inverse Translation
  inv_tx = -(inv_a * tx + inv_b * ty);
  inv_ty = -(inv_c * tx + inv_d * ty);
  
  // Setze die inverse Matrix zusammen
  inverse[0][0] = inv_a;
  inverse[0][1] = inv_b;
  inverse[0][2] = inv_tx;
    
  inverse[1][0] = inv_c;
  inverse[1][1] = inv_d;
  inverse[1][2] = inv_ty;
  }


void gavl_2d_transform_mult(const double mat1[2][3], const double mat2[2][3], double ret[2][3])
  {
  ret[0][0] = mat1[0][0] * mat2[0][0] + mat1[0][1] * mat2[1][0];
  ret[0][1] = mat1[0][0] * mat2[0][1] + mat1[0][1] * mat2[1][1];
  ret[1][0] = mat1[1][0] * mat2[0][0] + mat1[1][1] * mat2[1][0];
  ret[1][1] = mat1[1][0] * mat2[0][1] + mat1[1][1] * mat2[1][1];

  ret[0][2] = mat1[0][0] * mat2[0][2] + mat1[0][1] * mat2[1][2] + mat1[0][2]; 
  ret[1][2] = mat1[1][0] * mat2[0][2] + mat1[1][1] * mat2[1][2] + mat1[1][2];
  }

static void transform_2d_copy(double dst[2][3], const double src[2][3])
  {
  int i, j;

  for(i = 0; i < 2; i++)
    {
    for(j = 0; j < 3; j++)
      {
      dst[i][j] = src[i][j];
      }
    }
    
  }

void gavl_2d_transform_prepend(double dst[2][3], const double src[2][3])
  {
  double tmp[2][3];
  gavl_2d_transform_mult(src, dst, tmp);
  transform_2d_copy(dst, tmp);
  }

void gavl_2d_transform_transform(const double mat[2][3], const float * src, float * dst)
  {
  dst[0] = src[0] * mat[0][0] + src[1] * mat[0][1] + mat[0][2];
  dst[1] = src[0] * mat[1][0] + src[1] * mat[1][1] + mat[1][2];
  }

void gavl_2d_transform_transform_inplace(const double mat[2][3], float * vec)
  {
  float tmp[2];
  gavl_2d_transform_transform(mat, vec, tmp);
  memcpy(vec, tmp, 2*sizeof(vec[0]));
  }

char * gavl_filename_get_dir(const char * filename)
  {
  const char * pos = strrchr(filename, '/');
  return gavl_strndup(filename, pos);
  }

char * gavl_filename_get_base(const char * filename)
  {
  const char * start;
  const char * end;

  if((start = strrchr(filename, '/')))
    start++;
  else
    start = filename;

  if(!(end = strrchr(start, '.')))
    end = start + strlen(start);
  
  return gavl_strndup(start, end);
  }


char * gavl_filename_ensure_extension(const char * filename,
                                    const char * ext)
  {
  const char * pos;

  if((pos = strrchr(filename, '.')) &&
     (!strcasecmp(pos+1, ext)))
    return gavl_strdup(filename);
  else
    return gavl_sprintf("%s.%s", filename, ext);
  }


int gavl_read_file_range(const char * filename, gavl_buffer_t * buf, int64_t start, int64_t len)
  {
  FILE * file;
  
  file = fopen(filename, "r");
  if(!file)
    return 0;

  if(len < 1)
    len = gavl_file_size(file) - start;

  gavl_buffer_alloc(buf, len + 1);
  
  if(start > 0)
    fseek(file, start, SEEK_SET);
  
  if(fread(buf->buf, 1, len, file) < len)
    {
    fclose(file);
    gavl_buffer_free(buf);
    return 0;
    }
  buf->len = len;
  buf->buf[buf->len] = '\0';
  fclose(file);
  return 1;
  }

int gavl_read_file(const char * filename, gavl_buffer_t * buf)
  {
  return gavl_read_file_range(filename, buf, 0, 0);
  }

size_t gavl_file_size(FILE * f)
  {
  size_t ret;
  size_t oldpos;

  oldpos = ftell(f);

  fseek(f, 0, SEEK_END);
  ret = ftell(f);
  fseek(f, oldpos, SEEK_SET);
  return ret;
  }

int gavl_write_file(const char * filename, void * data, int len)
  {
  FILE * file;
  
  file = fopen(filename, "w");
  if(!file)
    return 0;
  
  if(fwrite(data, 1, len, file) < len)
    {
    fclose(file);
    return 0;
    }
  fclose(file);
  return 1;
  }
