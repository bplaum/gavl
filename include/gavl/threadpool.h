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
