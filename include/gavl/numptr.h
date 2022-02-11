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

#include <gavl/gavldefs.h>

/* ptr -> integer */

#define GAVL_PTR_2_16LE(p) \
  (((p)[1] << 8) |         \
   (p)[0])

#define GAVL_PTR_2_24LE(p) \
  (((p)[2] << 16) |        \
   ((p)[1] << 8) |         \
   (p)[0])

#define GAVL_PTR_2_32LE(p) \
  (((p)[3] << 24) |        \
   ((p)[2] << 16) |        \
   ((p)[1] << 8) |         \
   (p)[0])

#define GAVL_PTR_2_64LE(p)      \
  (((uint64_t)((p)[7]) << 56) | \
   ((uint64_t)((p)[6]) << 48) | \
   ((uint64_t)((p)[5]) << 40) | \
   ((uint64_t)((p)[4]) << 32) | \
   ((uint64_t)((p)[3]) << 24) | \
   ((uint64_t)((p)[2]) << 16) | \
   ((uint64_t)((p)[1]) << 8) |  \
   (p)[0])

#define GAVL_PTR_2_16BE(p) \
  (((p)[0] << 8) |         \
   (p)[1])

#define GAVL_PTR_2_32BE(p) \
  (((p)[0] << 24) |        \
   ((p)[1] << 16) |        \
   ((p)[2] << 8) |         \
   (p)[3])

#define GAVL_PTR_2_24BE(p) \
  (((p)[0] << 16) |        \
   ((p)[1] << 8) |         \
   ((p)[2]))

#define GAVL_PTR_2_64BE(p)  \
  (((uint64_t)p[0] << 56) | \
   ((uint64_t)p[1] << 48) | \
   ((uint64_t)p[2] << 40) | \
   ((uint64_t)p[3] << 32) | \
   ((uint64_t)p[4] << 24) | \
   ((uint64_t)p[5] << 16) | \
   ((uint64_t)p[6] << 8) |  \
   (p)[7])

/* integer -> ptr */

#define GAVL_16LE_2_PTR(i, p) \
  (p)[0] = (i) & 0xff;        \
  (p)[1] = ((i)>>8) & 0xff

#define GAVL_24LE_2_PTR(i, p) \
  (p)[0] = (i) & 0xff;        \
  (p)[1] = ((i)>>8) & 0xff;   \
  (p)[2] = ((i)>>16) & 0xff;

#define GAVL_32LE_2_PTR(i, p) \
  (p)[0] = (i) & 0xff;        \
  (p)[1] = ((i)>>8) & 0xff;   \
  (p)[2] = ((i)>>16) & 0xff;  \
  (p)[3] = ((i)>>24) & 0xff

#define GAVL_64LE_2_PTR(i, p) \
  (p)[0] = (i) & 0xff;        \
  (p)[1] = ((i)>>8) & 0xff;   \
  (p)[2] = ((i)>>16) & 0xff;  \
  (p)[3] = ((i)>>24) & 0xff;  \
  (p)[4] = ((i)>>32) & 0xff;  \
  (p)[5] = ((i)>>40) & 0xff;  \
  (p)[6] = ((i)>>48) & 0xff;  \
  (p)[7] = ((i)>>56) & 0xff

#define GAVL_16BE_2_PTR(i, p) \
  (p)[1] = (i) & 0xff;        \
  (p)[0] = ((i)>>8) & 0xff

#define GAVL_24BE_2_PTR(i, p) \
  (p)[2] = (i) & 0xff;        \
  (p)[1] = ((i)>>8) & 0xff;   \
  (p)[0] = ((i)>>16) & 0xff;

#define GAVL_32BE_2_PTR(i, p) \
  (p)[3] = (i) & 0xff;        \
  (p)[2] = ((i)>>8) & 0xff;   \
  (p)[1] = ((i)>>16) & 0xff;  \
  (p)[0] = ((i)>>24) & 0xff;

#define GAVL_64BE_2_PTR(i, p) \
  (p)[7] = (i) & 0xff;        \
  (p)[6] = ((i)>>8) & 0xff;   \
  (p)[5] = ((i)>>16) & 0xff;  \
  (p)[4] = ((i)>>24) & 0xff;  \
  (p)[3] = ((i)>>32) & 0xff;  \
  (p)[2] = ((i)>>40) & 0xff;  \
  (p)[1] = ((i)>>48) & 0xff;  \
  (p)[0] = ((i)>>56) & 0xff

GAVL_PUBLIC void
GAVL_FLOAT32BE_2_PTR(float f, void * p);

GAVL_PUBLIC void
GAVL_FLOAT32LE_2_PTR(float f, void * p);

GAVL_PUBLIC void
GAVL_DOUBLE64BE_2_PTR(double d, void * p);

GAVL_PUBLIC void
GAVL_DOUBLE64LE_2_PTR(double d, void * p);

GAVL_PUBLIC float
GAVL_PTR_2_FLOAT32BE(const void * p);

GAVL_PUBLIC float
GAVL_PTR_2_FLOAT32LE(const void * p);

GAVL_PUBLIC double
GAVL_PTR_2_DOUBLE64BE(const void * p);

GAVL_PUBLIC double
GAVL_PTR_2_DOUBLE64LE(const void * p);
