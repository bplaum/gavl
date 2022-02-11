/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2012 Members of the Gmerlin project
 * gmerlin-general@lists.sourceforge.net
 * http://gmerlin.sourceforge.net
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
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif
#include <sys/time.h>

#include <gavl.h>

#ifdef HAVE_CLOCK_MONOTONIC

/* High precision version */
static void get_time(gavl_time_t * ret)
  {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  *ret = (int64_t)(time.tv_sec)*1000000LL + (time.tv_nsec)/1000;
  }

#else
static void get_time(gavl_time_t * ret)
  {
  struct timeval time;
  gettimeofday(&time, NULL);
  *ret = (int64_t)(time.tv_sec)*1000000LL + time.tv_usec;
  }
#endif

struct gavl_timer_s
  {
  int64_t start_time_real;
  int64_t start_time;
  int is_running;
  };

gavl_timer_t * gavl_timer_create()
  {
  gavl_timer_t * ret = calloc(1, sizeof(*ret));
  return ret;
  }

void gavl_timer_destroy(gavl_timer_t * t)
  {
  free(t);
  }

void gavl_timer_start(gavl_timer_t * t)
  {
  get_time(&t->start_time_real);
  t->is_running = 1;
  }

int gavl_timer_is_running(gavl_timer_t * t)
  {
  return t->is_running;
  }

void gavl_timer_stop(gavl_timer_t * t)
  {
  gavl_time_t tmp;
  tmp = gavl_timer_get(t);
  t->start_time = tmp;
  t->is_running = 0;
  }

gavl_time_t gavl_timer_get(gavl_timer_t * t)
  {
  gavl_time_t ret;
  if(t->is_running)
    {
    get_time(&ret);
    ret -= t->start_time_real;
    ret += t->start_time;
    return ret;
    }
  else
    return t->start_time;
  }

void gavl_timer_set(gavl_timer_t * t, gavl_time_t v)
  {
  int was_running = t->is_running;

  if(was_running)
    gavl_timer_stop(t);

  t->start_time = v;

  if(was_running)
    gavl_timer_start(t);
  }

/* */

#ifdef HAVE_CLOCK_GETTIME

uint64_t gavl_benchmark_get_time(int config_flags)
  {
  struct timespec ts;
#if defined(CLOCK_PROCESS_CPUTIME_ID)
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
#elif defined(CLOCK_PROF)
  clock_gettime(CLOCK_PROF, &ts);
#else
  clockgettime(CLOCK_REALTIME, &ts);
#endif

  return (uint64_t)(ts.tv_sec) * 1000000000 + ts.tv_nsec;
  }

const char * gavl_benchmark_get_desc(int flags)
  {
  return "nanoseconds returned by clock_gettime with CLOCK_PROCESS_CPUTIME_ID";
  }

#elif defined(ARCH_X86) || defined(HAVE_SYS_TIMES_H)

uint64_t gavl_benchmark_get_time(int config_flags)
  {
  struct timeval tv;
#ifdef ARCH_X86
  uint64_t x;
  /* that should prevent us from trying cpuid with old cpus */
  if( config_flags & GAVL_ACCEL_MMX )
    {
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
    }
  else
    {
#endif
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000000 + tv.tv_usec;
#ifdef ARCH_X86
  }
#endif
  }

const char * gavl_benchmark_get_desc(int config_flags)
  {
#ifdef ARCH_X86
  /* that should prevent us from trying cpuid with old cpus */
  if( config_flags & GAVL_ACCEL_MMX )
    {
    return "Units returned by rdtsc";
    }
  else
    {
#endif
    return "microseconds returned by gettimeofday";
#ifdef ARCH_X86
    }
#endif
  }

#else /* gettimeofday */

uint64_t gavl_benchmark_get_time(int config_flags)
  {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t)(tv.tv_sec) * 1000000 + tv.tv_usec;
  /* FIXME: implement an equivalent for using optimized memcpy on other
     architectures */
  }

const char * gavl_benchmark_get_desc(int flags)
  {
  return "microseconds returned by gettimeofday";
  }

#endif

