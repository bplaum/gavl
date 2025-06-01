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

#include <inttypes.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <pthread.h>

#include <unistd.h>
#include <sys/types.h>

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include <gavlshm.h>

// #include <gmerlin/translation.h>
#include <gavl/log.h>
#define LOG_DOMAIN "shm"

#define ALIGN_BYTES 16


#if 0
typedef struct
  {
  pthread_mutex_t mutex;
  int refcount;
  } refcounter_t;
#endif


static pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;
static int shm_ctx = 0;


/*
static int align_size(int size)
  {
  size = ((size + ALIGN_BYTES - 1) / ALIGN_BYTES) * ALIGN_BYTES;
  return size;
  }

static int get_real_size(int size)
  {
  return align_size(size) + sizeof(refcounter_t);
  }
*/
  
uint8_t * gavl_shm_get_buffer(gavl_shm_t * s, int * size)
  {
  if(size)
    *size = s->size;
  return s->addr;
  }

/*
static void gen_name(int pid, int shmid, char * ret)
  {
  snprintf(ret, BG_SHM_NAME_MAX, "/gavl-%08x-%08x", pid, shmid);
  }
*/
  
gavl_shm_t * gavl_shm_create(int size, int * ctx, int * idx)
  {
  int shm_fd = -1;
  void * addr;
  gavl_shm_t * ret = NULL;
  char name[BG_SHM_NAME_MAX];
  //  pthread_mutexattr_t attr;
  //  int pid = getpid();
  
  if(!(*ctx))
    {
    pthread_mutex_lock(&id_mutex);
    shm_ctx++;
    *ctx = shm_ctx;
    pthread_mutex_unlock(&id_mutex);
    }

  snprintf(name, BG_SHM_NAME_MAX, "/gavl-%08x-%08x-%08x",
           getpid(), *ctx, *idx);

  (*idx)++;

  if((shm_fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
             "shm_open of %s failed: %s", name, strerror(errno));
    goto fail;
    }
  
  if(ftruncate(shm_fd, size))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
           "ftruncate failed: %s", strerror(errno));
    goto fail;
    }
  
  if((addr = mmap(0, size, PROT_READ | PROT_WRITE,
                  MAP_SHARED, shm_fd, 0)) == MAP_FAILED)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
           "mmap failed: %s", strerror(errno));
    goto fail;
    }
  
  ret = calloc(1, sizeof(*ret));
  ret->addr = addr;
  ret->size = size;
  strncpy(ret->name, name, BG_SHM_NAME_MAX);
  ret->flags |= BG_SHM_FLAG_CREATOR;
  
  gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN,
           "created shm segment %s", name);
  fail:
  
  if(shm_fd >= 0)
    close(shm_fd);
  return ret;
  }

gavl_shm_t * gavl_shm_map(const char * name, int size, int wr)
  {
  void * addr;
  gavl_shm_t * ret = NULL;
  int shm_fd;
  //  int real_size = get_real_size(size);

  int arg;

  arg = O_RDONLY;
  if(wr)
    arg = O_RDWR;
  
  shm_fd = shm_open(name, arg, 0);
  if(shm_fd < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
             "shm_open of %s failed: %s", name, strerror(errno));
    goto fail;
    }

  arg = PROT_READ;
  if(wr)
    arg |= PROT_WRITE;
  if((addr = mmap(0, size, arg, MAP_SHARED,
                  shm_fd, 0)) == MAP_FAILED)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
           "mmap failed: %s", strerror(errno));
    goto fail;
    }
  
  ret = calloc(1, sizeof(*ret));
  ret->addr = addr;
  ret->size = size;
  //  ret->rc = (refcounter_t*)(ret->addr + align_size(size));
  
  gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN,
           "created shm segment (read) %s", name);
  
  fail:
  if(shm_fd >= 0)
    close(shm_fd);
  
  return ret;
  }

void gavl_shm_unmap(gavl_shm_t * shm)
  {
  if(!(shm->flags | BG_SHM_FLAG_CREATOR))
    {
    munmap(shm->addr, shm->size);
    shm->addr = NULL;
    }
  }

void gavl_shm_destroy(gavl_shm_t * shm)
  {
  if(shm->addr)
    munmap(shm->addr, shm->size);
  
  if(shm->flags | BG_SHM_FLAG_CREATOR)
    {
    shm_unlink(shm->name);
    gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN,
           "destroyed shm segment %s", shm->name);
    }
  free(shm);
  }

