#ifndef GAVL_SHM_H_INCLUDED
#define GAVL_SHM_H_INCLUDED


#include <limits.h>
#include <inttypes.h>

typedef struct gavl_shm_s gavl_shm_t;

gavl_shm_t * gavl_shm_alloc_write(int size);
gavl_shm_t * gavl_shm_alloc_read(int pid, int id, int size);

uint8_t * gavl_shm_get_buffer(gavl_shm_t * s, int * size);
int gavl_shm_get_id(gavl_shm_t * s);

void gavl_shm_ref(gavl_shm_t * s);
void gavl_shm_unref(gavl_shm_t * s);
int gavl_shm_refcount(gavl_shm_t * s);

void gavl_shm_free(gavl_shm_t*);

/* Shared memory pool */

typedef struct gavl_shm_pool_s gavl_shm_pool_t;

gavl_shm_pool_t * gavl_shm_pool_create_write(int seg_size);
gavl_shm_pool_t * gavl_shm_pool_create_read(int seg_size, int pid);

void gavl_shm_pool_destroy(gavl_shm_pool_t *);


/* Get a shared memory segment for reading. */
gavl_shm_t * gavl_shm_pool_get_read(gavl_shm_pool_t *, int id);
gavl_shm_t * gavl_shm_pool_get_write(gavl_shm_pool_t *);

#endif // GAVL_SHM_H_INCLUDED
