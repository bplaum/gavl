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

#include <stdlib.h>
#include <string.h>
// #include <stdio.h>

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/compression.h>

// #include <gavl/log.h>
// #define LOG_DOMAIN "palette"

gavl_palette_t *
gavl_palette_create(void)
  {
  gavl_palette_t * pal = calloc(1, sizeof(*pal));
  return pal;
  }

void
gavl_palette_alloc(gavl_palette_t * pal, int num_colors)
  {
  pal->entries = malloc(num_colors * sizeof(*pal->entries));
  pal->num_entries = num_colors;
  }

void gavl_palette_free(gavl_palette_t * pal)
  {
  if(pal->entries)
    free(pal->entries);
  }

void gavl_palette_destroy(gavl_palette_t * pal)
  {
  gavl_palette_free(pal);
  free(pal);
  }

void
gavl_palette_init(gavl_palette_t * pal)
  {
  memset(pal, 0, sizeof(*pal));
  }

void gavl_palette_move(gavl_palette_t * dst, gavl_palette_t * src)
  {
  memmove(dst, src, sizeof(*dst));
  gavl_palette_init(src);
  }

