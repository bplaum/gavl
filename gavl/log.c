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


#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>


#include <gavl/log.h>
#include <gavl/utils.h>


static gavl_handle_msg_func gavl_log_handler = gavl_log_stderr;
static void * gavl_log_priv = NULL;

static int gavl_log_mask = GAVL_LOG_ERROR | GAVL_LOG_WARNING | GAVL_LOG_INFO;

static const struct
  {
  gavl_log_level_t level;
  const char * name;
  }
level_names[] =
  {
    { GAVL_LOG_DEBUG,   "Debug"   },
    { GAVL_LOG_WARNING, "Warning" },
    { GAVL_LOG_ERROR,   "Error"   },
    { GAVL_LOG_INFO,    "Info"    },
    { 0,                 NULL     }
  };
  
const char * gavl_log_level_to_string(gavl_log_level_t level)
  {
  int index = 0;
  while(level_names[index].name)
    {
    if(level_names[index].level == level)
      return level_names[index].name;
    index++;
    }
  return NULL;
  }


static void logs_internal(gavl_log_level_t level, const char * domain,
                          const char *  message)
  {
  char ** lines;
  int i;

  if(level & gavl_log_mask)
    {
    lines = gavl_strbreak(message, '\n');
    i = 0;
    while(lines[i])
      {
      gavl_msg_t msg;

      gavl_strtrim(lines[i]);
      if(*(lines[i]) == '\0')
        {
        i++;
        continue;
        }
      
      gavl_msg_init(&msg);
      gavl_log_msg_set(&msg, level, domain, lines[i]);
      gavl_log_handler(gavl_log_priv, &msg);
      gavl_msg_free(&msg);
      i++;
      }
    gavl_strbreak_free(lines);
    }
  }


void gavl_logv_translate(const char * translation_domain,
                         gavl_log_level_t level, const char * domain, 
                         const char * format, va_list argp)
  {
  char * msg_string;

#ifndef HAVE_VASPRINTF
  int len;
  len = vsnprintf(NULL, 0, format, argp);
  msg_string = malloc(len+1);
  vsnprintf(msg_string, len+1, format, argp);
#else
  if(vasprintf(&msg_string, format, argp) < 0)
    return; // Should never happen
#endif
  logs_internal(level, domain, msg_string);
  free(msg_string);
  }

void gavl_log_translate(const char * translation_domain,
                        gavl_log_level_t level, const char * domain,
                        const char * format, ...)
  {
  va_list argp; /* arg ptr */

  va_start( argp, format);
  gavl_logv_translate(translation_domain, level, domain, format, argp);
  va_end(argp);
  }


int gavl_log_msg_get(const gavl_msg_t * msg,
                     gavl_log_level_t * level,
                     const char ** domain,
                     const char ** message)
  {
  switch(msg->ID)
    {
    case GAVL_LOG_ERROR:
    case GAVL_LOG_DEBUG:
    case GAVL_LOG_INFO:
    case GAVL_LOG_WARNING:
      *level = msg->ID;
      *domain = gavl_msg_get_arg_string_c(msg, 0);
      *message = gavl_msg_get_arg_string_c(msg, 1);
      break;
    default:
      return 0;
    }
  return 1;
  }


void gavl_log_msg_set(gavl_msg_t * msg,
                      gavl_log_level_t level,
                      const char * domain,
                      const char * message)
  {
  gavl_msg_set_id_ns(msg, level, GAVL_MSG_NS_LOG);
  gavl_msg_set_arg_string(msg, 0, domain);
  gavl_msg_set_arg_string(msg, 1, message);
  }

/* stderr logging */

/* Default log functions */


static void log_stderr_nocolor(gavl_log_level_t level,
                               const char * domain, const char * msg)
  {
  fprintf(stderr, "[%s] %s: %s\n",
          domain,
          gavl_log_level_to_string(level),
          msg);
  }

#ifdef HAVE_ISATTY

static char term_error[]  = "\33[31m";
static char term_warn[]   = "\33[33m";
static char term_debug[]  = "\33[32m";
static char term_reset[]  = "\33[0m";

static void log_stderr(gavl_log_level_t level, const char * domain, const char * msg)
  {
  int use_color;
  if(isatty(fileno(stderr)) && getenv("TERM"))
    use_color = 1;
  else
    use_color = 0;
  
  if(use_color)
    {
    switch(level)
      {
      case GAVL_LOG_INFO:
        fprintf(stderr, "[%s] %s\n",
                domain,
                msg);
        break;
      case GAVL_LOG_WARNING:
        fprintf(stderr, "%s[%s] %s%s\n",
                term_warn,
                domain,
                msg,
                term_reset);
        break;
      case GAVL_LOG_ERROR:
        fprintf(stderr, "%s[%s] %s%s\n",
                term_error,
                domain,
                msg,
                term_reset);
        break;
      case GAVL_LOG_DEBUG:
        fprintf(stderr, "%s[%s] %s%s\n",
                term_debug,
                domain,
                msg,
                term_reset);
        break;
      }
    
    }
  else
    {
    log_stderr_nocolor(level, domain, msg);
    }
  }

#else
#define log_stderr log_stderr_nocolor
#endif

int gavl_log_stderr(void * data, gavl_msg_t * msg)
  {
  const char * domain = NULL;
  const char * message = NULL;
  gavl_log_level_t level;

  if(gavl_log_msg_get(msg, &level, &domain, &message))
    {
    log_stderr(level, domain, message);
    }
     
  return 1;
  }

void gavl_set_log_callback(gavl_handle_msg_func handler, void * priv)
  {
  gavl_log_handler = handler;
  gavl_log_priv    = priv;
  
  }

void gavl_set_log_mask(int mask)
  {
  gavl_log_mask = mask;
  }

void gavl_set_log_verbose(int level)
  {
  gavl_log_mask = 0;
  
  if(level >= 1)
    gavl_log_mask |= GAVL_LOG_ERROR;
  if(level >= 2)
    gavl_log_mask |= GAVL_LOG_WARNING;
  if(level >= 3)
    gavl_log_mask |= GAVL_LOG_INFO;
  if(level >= 4)
    gavl_log_mask |= GAVL_LOG_DEBUG;
  
  }
