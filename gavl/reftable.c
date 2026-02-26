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

#define _GNU_SOURCE

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>
#include <string.h>


#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/log.h>
#define LOG_DOMAIN "sharedpool"
#include <hw_private.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int num_pools = 0;

// #define FLAG_LOCAL (1<<0)

char * gavl_hw_reftable_create_name(void)
  {
  char * ret;

  pthread_mutex_lock(&mutex);
  num_pools++;
  ret = gavl_sprintf("/gavl-reftable-%08x-%08x", getpid(), num_pools);
  pthread_mutex_unlock(&mutex);
  return ret;
  }

static void reftable_initialize(reftable_t * tab, int shared)
  {
  sem_init(&tab->free_buffers, shared, 0);
  }

static int reftable_size(gavl_hw_context_t * ctx)
  {
  return sizeof(reftable_t) + ctx->max_frames * sizeof(reftable_frame_t);
  }


reftable_t * gavl_hw_reftable_create_local(gavl_hw_context_t * ctx)
  {
  int size;
  reftable_t * ret;

  size = reftable_size(ctx);

  ret = calloc(1, size);
  
  ctx->shm_fd = -1;

  reftable_initialize(ret, 0);
  
  return ret;
  }

reftable_t * gavl_hw_reftable_create_shared(gavl_hw_context_t * ctx)
  {
  int size;
  reftable_t * ret;
  
  size = reftable_size(ctx);
  
  if((ctx->shm_fd = shm_open(ctx->shm_name, O_RDWR | O_CREAT | O_EXCL,
                             S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
             "shm_open of %s failed: %s", ctx->shm_name, strerror(errno));
    goto fail;
    }

  if(ftruncate(ctx->shm_fd, size))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
           "ftruncate failed: %s", strerror(errno));
    goto fail;
    }

  if((ret = mmap(0, size, PROT_READ | PROT_WRITE,
                  MAP_SHARED, ctx->shm_fd, 0)) == MAP_FAILED)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
           "mmap failed: %s", strerror(errno));
    goto fail;
    }

  reftable_initialize(ret, 1);
  
  return ret;

  fail:
  return NULL;
  }

reftable_t * gavl_hw_reftable_create_remote(gavl_hw_context_t * ctx)
  {
  int size;
  reftable_t * ret;

  size = reftable_size(ctx);

  if((ctx->shm_fd = shm_open(ctx->shm_name, O_RDWR,
                             S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
             "shm_open of %s failed: %s", ctx->shm_name, strerror(errno));
    goto fail;
    }
  
  if((ret = mmap(0, size, PROT_READ | PROT_WRITE,
                  MAP_SHARED, ctx->shm_fd, 0)) == MAP_FAILED)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
           "mmap failed: %s", strerror(errno));
    goto fail;
    }
  
  return ret;

  fail:
  return NULL;

  }

void gavl_hw_reftable_destroy(gavl_hw_context_t * ctx)
  {
  if(!ctx->reftab_priv)
    return;
  
  if(ctx->shm_name)
    {
    close(ctx->shm_fd);
    if(ctx->reftab_priv)
      munmap(ctx->reftab_priv, reftable_size(ctx));
    }
  else
    {
    if(ctx->reftab_priv)
      free(ctx->reftab_priv);
    }
  
  if((ctx->flags & HW_CTX_FLAG_CREATOR) && ctx->shm_name)
    shm_unlink(ctx->shm_name);
  
  ctx->reftab = NULL;
  ctx->reftab_priv = NULL;
  
  }


