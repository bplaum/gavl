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


#ifndef GAVL_SHM_H_INCLUDED
#define GAVL_SHM_H_INCLUDED

#include <limits.h>
#include <inttypes.h>

/* /gavl-12345678-12345670-12345670 */
#define BG_SHM_NAME_MAX 64

typedef struct gavl_shm_s gavl_shm_t;

#define BG_SHM_FLAG_CREATOR (1<<0)

struct gavl_shm_s
  {
  int flags;
  uint8_t * addr;
  int size;
  char name[BG_SHM_NAME_MAX];
  };

gavl_shm_t * gavl_shm_create(int size, int * ctx, int * idx);

gavl_shm_t * gavl_shm_map(const char * name, int size, int wr);
void gavl_shm_unmap(gavl_shm_t *);

uint8_t * gavl_shm_get_buffer(gavl_shm_t * s, int * size);
void gavl_shm_destroy(gavl_shm_t*);

#endif // GAVL_SHM_H_INCLUDED
