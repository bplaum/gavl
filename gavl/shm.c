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
static int shm_id = 0;

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

static void gen_name(int pid, int shmid, char * ret)
  {
  snprintf(ret, BG_SHM_NAME_MAX, "/gavl-%08x-%08x", pid, shmid);
  }

gavl_shm_t * gavl_shm_alloc_write(int size)
  {
  int shm_fd = -1;
  void * addr;
  gavl_shm_t * ret = NULL;
  char name[BG_SHM_NAME_MAX];
  //  pthread_mutexattr_t attr;
  int pid = getpid();
  
  pthread_mutex_lock(&id_mutex);
    
  while(1)
    {
    shm_id++;
    
    gen_name(pid, shm_id, name);
    
    if((shm_fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL,
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
      {
      if(errno != EEXIST)
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
               "shm_open of %s failed: %s", name, strerror(errno));
        return NULL;
        }
      }
    else
      break;
    }

  pthread_mutex_unlock(&id_mutex);
  
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
    return NULL;
    }

  ret = calloc(1, sizeof(*ret));
  ret->addr = addr;
  ret->size = size;

  strncpy(ret->name, name, BG_SHM_NAME_MAX);
  
  ret->wr = 1;
  //  ret->rc = (refcounter_t*)(ret->addr + align_size(size));

  /* Initialize process shared mutex */
#if 0  
  pthread_mutexattr_init(&attr);
  if(pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
           "cannot create process shared mutex: %s", strerror(errno));
    goto fail;
    }
  //  pthread_mutex_init(&ret->rc->mutex, &attr);


  ret->rc->refcount = 0;
#endif
  gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN,
         "created shm segment (write) %s", name);

  fail:
  
  close(shm_fd);
  
  return ret;
  }

gavl_shm_t * gavl_shm_alloc_read(const char * name, int size)
  {
  void * addr;
  gavl_shm_t * ret = NULL;
  int shm_fd;
  //  int real_size = get_real_size(size);
  
  shm_fd = shm_open(name, O_RDWR, 0);
  if(shm_fd < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
             "shm_open of %s failed: %s", name, strerror(errno));
    goto fail;
    }
  if((addr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED,
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

void gavl_shm_free(gavl_shm_t * shm)
  {
  munmap(shm->addr, shm->size);
  
  if(shm->wr)
    {
    shm_unlink(shm->name);
    gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN,
           "destroyed shm segment %s", shm->name);
    }
  free(shm);
  }

#if 0
void gavl_shm_ref(gavl_shm_t * s)
  {
  pthread_mutex_lock(&s->rc->mutex);
  s->rc->refcount++;
  //  fprintf(stderr, "gavl_shm_ref %08x %d\n", s->id, s->rc->refcount);
  pthread_mutex_unlock(&s->rc->mutex);

  }

void gavl_shm_unref(gavl_shm_t * s)
  {
  pthread_mutex_lock(&s->rc->mutex);
  s->rc->refcount--;
  //  fprintf(stderr, "gavl_shm_unref %08x %d\n", s->id, s->rc->refcount);
  pthread_mutex_unlock(&s->rc->mutex);
  }

int gavl_shm_refcount(gavl_shm_t * s)
  {
  int ret;
  pthread_mutex_lock(&s->rc->mutex);
  ret = s->rc->refcount;
  pthread_mutex_unlock(&s->rc->mutex);
  return ret;
  }

/* Shared memory pool */

struct gavl_shm_pool_s
  {
  int wr;
  };

gavl_shm_pool_t * gavl_shm_pool_create_write(int seg_size)
  {
  gavl_shm_pool_t * ret = calloc(1, sizeof(*ret));
  ret->wr = 1;
  return ret;
  }

gavl_shm_pool_t * gavl_shm_pool_create_read(int seg_size, int pid)
  {
  gavl_shm_pool_t * ret = calloc(1, sizeof(*ret));
  return ret;
  }

#endif


#if 0
gavl_shm_t * gavl_shm_pool_get_read(gavl_shm_pool_t * p, int id)
  {
  int i;
  gavl_shm_t * ret;
  for(i = 0; i < p->num_segments; i++)
    {
    if(p->segments[i]->id == id)
      return p->segments[i];
    }

  /* Segment isn't in the pool yet, map a new one */
  if(p->num_segments + 1 >= p->segments_alloc)
    {
    p->segments_alloc += 16;
    p->segments = realloc(p->segments,
                          p->segments_alloc * sizeof(*p->segments));
    }
  
  ret = gavl_shm_alloc_read(p->pid, id, p->segment_size);

  if(!ret)
    return ret;
  p->segments[p->num_segments] = ret;
  p->num_segments++;
  return ret;
  }

gavl_shm_t * gavl_shm_pool_get_write(gavl_shm_pool_t * p)
  {
  int i;
  gavl_shm_t * ret = NULL;
  for(i = 0; i < p->num_segments; i++)
    {
    if(!gavl_shm_refcount(p->segments[i]))
      {
      ret = p->segments[i];
      break;
      }
    }

  if(!ret)
    {
    /* No unused segment available, create a new one */
    if(p->num_segments + 1 >= p->segments_alloc)
      {
      p->segments_alloc += 16;
      p->segments = realloc(p->segments,
                            p->segments_alloc * sizeof(*p->segments));
      }
    p->segments[p->num_segments] =
      gavl_shm_alloc_write(p->segment_size);
    ret = p->segments[p->num_segments];
    p->num_segments++;
    }

  /* Set refcount to 1 */
  gavl_shm_ref(ret);
  return ret;
  }
void gavl_shm_pool_destroy(gavl_shm_pool_t * p)
  {
  free(p);
  }
#endif
