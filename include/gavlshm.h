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
