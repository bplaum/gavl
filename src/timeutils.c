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

#include <sys/time.h>
#include <stdlib.h>

#include <config.h>

#include <inttypes.h>
#include <timeutils.h>

static struct timeval time_before;
static struct timeval time_after;

void timer_init()
  {
  gettimeofday(&time_before, NULL);
  }

uint64_t timer_stop()
  {
  uint64_t before, after, diff;
  
  gettimeofday(&time_after, NULL);

  before = ((uint64_t)time_before.tv_sec)*1000000 + time_before.tv_usec;
  after  = ((uint64_t)time_after.tv_sec)*1000000  + time_after.tv_usec;
  
  /*   fprintf(stderr, "Before: %f After: %f\n", before, after); */
  
  diff = after - before;

  return diff;
  
  }

