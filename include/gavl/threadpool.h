/* Thread pool */
#ifndef GAVL_THREADPOOL_H_INCLUDED
#define GAVL_THREADPOOL_H_INCLUDED

/* Forward declaration */
// typedef struct gavl_thread_pool_s gavl_thread_pool_t;

typedef struct gavl_thread_pool_s gavl_thread_pool_t;

GAVL_PUBLIC gavl_thread_pool_t * gavl_thread_pool_create(int num_threads);
GAVL_PUBLIC void gavl_thread_pool_destroy(gavl_thread_pool_t *);

GAVL_PUBLIC int gavl_thread_pool_get_num_threads(gavl_thread_pool_t *);

GAVL_PUBLIC void gavl_thread_pool_run(void (*func)(void*,int start, int len),
                        void * gavl_data,
                        int start, int len,
                        void * client_data, int thread);

GAVL_PUBLIC void gavl_thread_pool_stop(void * client_data, int thread);

#endif // GAVL_THREADPOOL_H_INCLUDED
