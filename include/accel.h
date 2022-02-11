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

/*
  Acceleration flags are NOT part of the public API
  because what the user wants in the end is to choose between
  speed and quality.

  The only place, where these functions are needed from outside
  the library, are the test programs, which must select
  individual routines.
*/

#include <string.h> /* We want the memcpy prototype anyway */

/* Acceleration flags (CPU Specific flags are in the public API) */

#define GAVL_ACCEL_C        (1<<16)
#define GAVL_ACCEL_C_HQ     (1<<17)
#define GAVL_ACCEL_C_SHQ    (1<<18) /* Super high quality, damn slow */

/* CPU Specific flags */

  

void gavl_audio_options_set_accel_flags(gavl_audio_options_t * opt,
                                        int accel_flags)
  __attribute__ ((visibility("default")));
int gavl_audio_options_get_accel_flags(gavl_audio_options_t * opt);

void gavl_video_options_set_accel_flags(gavl_video_options_t * opt,
                                        int accel_flags)
  __attribute__ ((visibility("default")));
int gavl_video_options_get_accel_flags(gavl_video_options_t * opt);

/* Optimized memcpy versions */

void gavl_init_memcpy();

extern void * (*gavl_memcpy)(void *to, const void *from, size_t len);

/* Branch prediction */

#if __GNUC__ >= 3

#define GAVL_UNLIKELY(exp) __builtin_expect((exp),0)
#define GAVL_LIKELY(exp)   __builtin_expect((exp),1)

#else

#define GAVL_UNLIKELY(exp) exp
#define GAVL_LIKELY(exp)   exp

#endif
