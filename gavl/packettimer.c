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

#include <stdlib.h>

#include <packettimer.h>
#include <gavltime.h>



struct gavl_packet_timer_s
  {
  int timescale;
  int low_delay;

  gavl_time_t start_time;
  int64_t start_dts;
  int64_t dts;
  
  };

gavl_packet_timer_t * gavl_packet_timer_create(int low_delay, int timescale)
  {
  gavl_packet_timer_t * ret;
  ret = calloc(1, sizeof(*ret));
  ret->timescale = timescale;
  ret->low_delay = low_delay;
  ret->start_time = GAVL_TIME_UNDEFINED;
  
  return ret;
  }

void gavl_packet_timer_destroy(gavl_packet_timer_t * pt)
  {
  free(pt);
  
  }

static int64_t get_dts(gavl_packet_timer_t * pt, const gavl_packet_t * p)
  {
  if(pt->low_delay)
    return p->pts;
  else if(p->dts != GAVL_TIME_UNDEFINED)
    return p->dts;
  else // Reconstruct dts from duration
    {
    int64_t ret = pt->dts;
    pt->dts += p->duration;
    return ret;
    }
  }

void gavl_packet_timer_wait(gavl_packet_timer_t * pt, const gavl_packet_t * p)
  {
  if(pt->start_time == GAVL_TIME_UNDEFINED)
    {
    pt->start_time = gavl_time_get_monotonic();
    pt->start_dts  = get_dts(pt, p);
    return;
    }
  else
    {
    gavl_time_t diff_time =
      pt->start_time + gavl_time_unscale(pt->timescale, get_dts(pt, p) - pt->start_dts) -
      gavl_time_get_monotonic();
    if(diff_time > 0)
      gavl_time_delay(&diff_time);
    }
  }
