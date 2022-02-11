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

#include <stdlib.h> /* calloc, free */
#include <string.h> 

#ifdef DEBUG
#include <stdio.h>  
#endif

#include "gavl.h"
#include "config.h"
#include "audio.h"
#include <accel.h>

#define SET_INT(p) opt->p = p

void gavl_audio_options_set_quality(gavl_audio_options_t * opt, int quality)
  {
  SET_INT(quality);
  }

void gavl_audio_options_set_dither_mode(gavl_audio_options_t * opt, gavl_audio_dither_mode_t dither_mode)
  {
  SET_INT(dither_mode);
  }

void gavl_audio_options_set_resample_mode(gavl_audio_options_t * opt, gavl_resample_mode_t resample_mode)
  {
  SET_INT(resample_mode);
  }

void gavl_audio_options_set_accel_flags(gavl_audio_options_t * opt,
                                        int accel_flags)
  {
  SET_INT(accel_flags);
  }

void gavl_audio_options_set_conversion_flags(gavl_audio_options_t * opt,
                                             int conversion_flags)
  {
  SET_INT(conversion_flags);
  }

#undef SET_INT

int gavl_audio_options_get_accel_flags(gavl_audio_options_t * opt)
  {
  return opt->accel_flags;
  }

int gavl_audio_options_get_conversion_flags(const gavl_audio_options_t * opt)
  {
  return opt->conversion_flags;
  }

int gavl_audio_options_get_quality(const gavl_audio_options_t * opt)
  {
  return opt->quality;
  }

gavl_audio_dither_mode_t
gavl_audio_options_get_dither_mode(const gavl_audio_options_t * opt)
  {
  return opt->dither_mode;
  }

gavl_resample_mode_t
gavl_audio_options_get_resample_mode(const gavl_audio_options_t * opt)
  {
  return opt->resample_mode;
  }

void gavl_audio_options_set_defaults(gavl_audio_options_t * opt)
  {
  memset(opt, 0, sizeof(*opt));
  
  opt->conversion_flags =
    GAVL_AUDIO_FRONT_TO_REAR_COPY |
    GAVL_AUDIO_STEREO_TO_MONO_MIX |
    GAVL_AUDIO_NORMALIZE_MIX_MATRIX;
  
  opt->accel_flags = gavl_accel_supported();
  opt->quality = GAVL_QUALITY_DEFAULT;
  gavl_init_memcpy();
  }

gavl_audio_options_t * gavl_audio_options_create()
  {
  gavl_audio_options_t * ret = calloc(1, sizeof(*ret));
  gavl_audio_options_set_defaults(ret);
  return ret;
  }

void gavl_audio_options_copy(gavl_audio_options_t * dst,
                             const gavl_audio_options_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  }

void gavl_audio_options_destroy(gavl_audio_options_t * opt)
  {
  free(opt);
  }

void gavl_audio_options_set_mix_matrix(gavl_audio_options_t * opt,
                                       const double ** matrix)
  {
  opt->mix_matrix = matrix;
  }
  
const double ** gavl_audio_options_get_mix_matrix(const gavl_audio_options_t * opt)
  {
  return opt->mix_matrix;
  }
