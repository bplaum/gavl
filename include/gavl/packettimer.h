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

#ifndef PACKETTIMER_H_INCLUDED
#define PACKETTIMER_H_INCLUDED

#include <gavl/gavl.h>
#include <gavl/compression.h>


typedef struct gavl_packet_timer_s gavl_packet_timer_t;

GAVL_PUBLIC
gavl_packet_timer_t * gavl_packet_timer_create(int low_delay, int timescale);

GAVL_PUBLIC
void  gavl_packet_timer_destroy(gavl_packet_timer_t * pt);

GAVL_PUBLIC
void gavl_packet_timer_wait(gavl_packet_timer_t * pt, const gavl_packet_t * p);


#endif
