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


#ifndef GAVL_LOG_H_INCLUDED
#define GAVL_LOG_H_INCLUDED

#include <stdarg.h>

#include <gavl/gavl.h>
#include <gavl/msg.h>

typedef enum
  {
    GAVL_LOG_ERROR   = (1<<0),
    GAVL_LOG_WARNING = (1<<1),
    GAVL_LOG_INFO    = (1<<2),
    GAVL_LOG_DEBUG   = (1<<3),
    
  } gavl_log_level_t;

/* Logging is done via a single callback function, which does nice stderr console
   logging by default and can be set to any other function once during initialization
   before any other threads are launched */

GAVL_PUBLIC
void gavl_log_translate(const char * translation_domain,
                        gavl_log_level_t level, const char * domain,
                        const char * format, ...) __attribute__ ((format (printf, 4, 5)));

GAVL_PUBLIC
void gavl_logv_translate(const char * translation_domain,
                         gavl_log_level_t level, const char * domain, 
                         const char * format, va_list argp);


#define gavl_log(level, domain, ...)                     \
   gavl_log_translate(PACKAGE, level, domain, __VA_ARGS__)

#define gavl_logv(level, domain, fmt, valist)            \
   gavl_logv_translate(PACKAGE, level, domain, fmt, valist)


GAVL_PUBLIC
int gavl_log_stderr(void * data, gavl_msg_t * msg);

GAVL_PUBLIC
int gavl_log_msg_get(const gavl_msg_t * msg,
                     gavl_log_level_t * level,
                     const char ** domain,
                     const char ** message);

GAVL_PUBLIC
void gavl_log_msg_set(gavl_msg_t * msg,
                      gavl_log_level_t level,
                      const char * domain,
                      const char * message);

GAVL_PUBLIC
const char * gavl_log_level_to_string(gavl_log_level_t level);

GAVL_PUBLIC
void gavl_set_log_callback(gavl_handle_msg_func handler, void * priv);

GAVL_PUBLIC
void gavl_set_log_verbose(int level);

GAVL_PUBLIC
void gavl_set_log_mask(int mask);

GAVL_PUBLIC
void gavl_set_log_verbose(int level);

#endif // GAVL_LOG_H_INCLUDED
