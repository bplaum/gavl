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

#ifndef GAVL_STATE_H_INCLUDED
#define GAVL_STATE_H_INCLUDED


#include <gavl/msg.h>
/* State */

// GAVL_MSG_NS_STATE

/* 
 *  arg0: last (int)
 *  arg1: context (string)
 *  arg2: var (string)
 *  arg3: val (value)
 */

#define GAVL_MSG_STATE_CHANGED        1

/* 
 *  arg0: last (int)
 *  arg1: context (string)
 *  arg2: var (string)
 *  arg3: val (value)
 *
 *  context can be a path into nested subdirectories like /ctx/dict/subdict
 */

#define GAVL_CMD_SET_STATE            100

/* 
 *  arg0: last (int)
 *  arg1: context (string)
 *  arg2: var (string)
 *  arg3: difference (value)
 *
 *  context can be a path into nested subdirectories like /ctx/dict/subdict
 */

#define GAVL_CMD_SET_STATE_REL        101

/* Standardized state contexts and variables */

#define GAVL_STATE_CTX_SRC "src"
#define GAVL_STATE_SRC_METADATA      GAVL_META_METADATA
#define GAVL_STATE_SRC_SEEK_WINDOW         "seekwindow"
#define GAVL_STATE_SRC_SEEK_WINDOW_START   "start"
#define GAVL_STATE_SRC_SEEK_WINDOW_END     "end"
#define GAVL_STATE_SRC_START_TIME_ABSOLUTE "st"

GAVL_PUBLIC
void gavl_msg_set_state(gavl_msg_t * msg,
                      int id,
                      int last,
                      const char * ctx,
                      const char * var,
                      const gavl_value_t * val);

GAVL_PUBLIC
void gavl_msg_set_state_nocopy(gavl_msg_t * msg,
                             int id,
                             int last,
                             const char * ctx,
                             const char * var,
                             gavl_value_t * val);

GAVL_PUBLIC
void gavl_msg_get_state(const gavl_msg_t * msg,
                        int * last,
                        const char ** ctx_p,
                        const char ** var_p,
                        gavl_value_t * val,
                        gavl_dictionary_t * dict);

#endif // GAVL_STATE_H_INCLUDED
