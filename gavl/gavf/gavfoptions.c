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


#include <gavfprivate.h>
#include <string.h>
#include <stdlib.h>


void
gavf_options_set_flags(gavf_options_t * opt, int flags)
  {
  opt->flags = flags;
  }

int gavf_options_get_flags(gavf_options_t * opt)
  {
  return opt->flags;
  }

void gavf_options_copy(gavf_options_t * dst, const gavf_options_t * src)
  {
  memcpy(dst, src, sizeof(*src));
  }

gavf_options_t * gavf_options_create()
  {
  gavf_options_t * ret = calloc(1, sizeof(*ret));
  return ret;
  }

void gavf_options_destroy(gavf_options_t * opt)
  {
  free(opt);
  }
