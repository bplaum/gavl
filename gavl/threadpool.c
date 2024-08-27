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



#include <pthread.h>
#include <stdlib.h>

#include <gavl/gavl.h>
#include <gavl/threadpool.h>
#include <gavl/utils.h>

//#include <gmerlin/parameter.h>
//#include <gmerlin/bggavl.h>
//#include <gmerlin/bg_sem.h>

#include <semaphore.h>

typedef struct
  {
  pthread_t t;
  sem_t run_sem;
  sem_t done_sem;
  pthread_mutex_t stop_mutex;
  int do_stop;
  void (*func)(void*, int, int);
  void * data;
  int start;
  int len;
  } thread_t;

struct gavl_thread_pool_s
  {
  int num_threads;
  thread_t * threads;
  };

static void * thread_func(void * data)
  {
  thread_t * t = data;
  int do_stop;
  while(1)
    {
    //    fprintf(stderr, "thread_func %p\n", t->func);
    
    sem_wait(&t->run_sem);

    pthread_mutex_lock(&t->stop_mutex);
    do_stop = t->do_stop;
    pthread_mutex_unlock(&t->stop_mutex);

    if(do_stop)
      break;
    t->func(t->data, t->start, t->len);
    sem_post(&t->done_sem);
    }
  return NULL;
  }

gavl_thread_pool_t * gavl_thread_pool_create(int num_threads)
  {
  int i;
  gavl_thread_pool_t * ret = calloc(1, sizeof(*ret));

  if(num_threads < 1)
    num_threads = gavl_num_cpus();

  ret->num_threads = num_threads;
  ret->threads = calloc(num_threads, sizeof(*ret->threads));

  for(i = 0; i < ret->num_threads; i++)
    {
    pthread_mutex_init(&ret->threads[i].stop_mutex, NULL);
    sem_init(&ret->threads[i].run_sem, 0, 0);
    sem_init(&ret->threads[i].done_sem, 0, 0);
    pthread_create(&ret->threads[i].t,
                   NULL,
                   thread_func, &ret->threads[i]);
    }
  return ret;
  }

int gavl_thread_pool_get_num_threads(gavl_thread_pool_t * p)
  {
  return p->num_threads;
  }
  

void gavl_thread_pool_destroy(gavl_thread_pool_t * p)
  {
  int i;
  for(i = 0; i < p->num_threads; i++)
    {
    pthread_mutex_lock(&p->threads[i].stop_mutex);
    p->threads[i].do_stop = 1;
    pthread_mutex_unlock(&p->threads[i].stop_mutex);
    
    sem_post(&p->threads[i].run_sem);
    
    pthread_join(p->threads[i].t, NULL);
    pthread_mutex_destroy(&p->threads[i].stop_mutex);
    sem_destroy(&p->threads[i].run_sem);
    sem_destroy(&p->threads[i].done_sem);
    }
  free(p->threads);
  free(p);
  }

void gavl_thread_pool_run(void (*func)(void*,int start, int len),
                        void * gavl_data,
                        int start, int len,
                        void * client_data, int thread)
  {
  gavl_thread_pool_t * p     = client_data;

  p->threads[thread].func  = func;
  p->threads[thread].data  = gavl_data;
  p->threads[thread].start = start;
  p->threads[thread].len   = len;
  
  sem_post(&p->threads[thread].run_sem);
  }

void gavl_thread_pool_stop(void * client_data, int thread)
  {
  gavl_thread_pool_t * p     = client_data;
  sem_wait(&p->threads[thread].done_sem);
  }

