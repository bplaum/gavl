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

/* /gavl-12345678-12345670 */
#define BG_SHM_NAME_MAX 24

typedef struct gavl_shm_s
  {
  uint8_t * addr;
  int size;
  int wr;

  char name[BG_SHM_NAME_MAX];
  
  //  refcounter_t * rc;
  } gavl_shm_t;


gavl_shm_t * gavl_shm_alloc_write(int size);
gavl_shm_t * gavl_shm_alloc_read(const char * name, int size);

uint8_t * gavl_shm_get_buffer(gavl_shm_t * s, int * size);
int gavl_shm_get_id(gavl_shm_t * s);

void gavl_shm_free(gavl_shm_t*);


#if 0
void gavl_shm_ref(gavl_shm_t * s);
void gavl_shm_unref(gavl_shm_t * s);
int gavl_shm_refcount(gavl_shm_t * s);


/* Shared memory pool */

typedef struct gavl_shm_pool_s gavl_shm_pool_t;

gavl_shm_pool_t * gavl_shm_pool_create_write(int seg_size);
gavl_shm_pool_t * gavl_shm_pool_create_read(int seg_size, int pid);

void gavl_shm_pool_destroy(gavl_shm_pool_t *);


/* Get a shared memory segment for reading. */
gavl_shm_t * gavl_shm_pool_get_read(gavl_shm_pool_t *, int id);
gavl_shm_t * gavl_shm_pool_get_write(gavl_shm_pool_t *);
#endif

#endif // GAVL_SHM_H_INCLUDED
