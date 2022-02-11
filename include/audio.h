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

/* Private structures for the audio converter */

#include <gavl.h>
#include <samplerate.h>

#include "config.h"

struct gavl_audio_options_s
  {
  /*
   *  Quality setting from 1 to 5 (0 means undefined).
   *  3 means Standard C routines or accellerated version with
   *  equal quality. Lower numbers mean accellerated versions with lower
   *  quality.
   */
  int quality;         

  /* Explicit accel_flags are mainly for debugging purposes */
  uint32_t accel_flags;     /* CPU Acceleration flags */

  /* #defines from above */
    
  uint32_t conversion_flags;

  gavl_audio_dither_mode_t dither_mode;
  gavl_resample_mode_t resample_mode;
  
  const double ** mix_matrix;
  };

typedef struct gavl_audio_convert_context_s gavl_audio_convert_context_t;
typedef struct gavl_mix_matrix_s gavl_mix_matrix_t;

typedef void (*gavl_audio_func_t)(struct gavl_audio_convert_context_s * ctx);

typedef struct gavl_samplerate_converter_s gavl_samplerate_converter_t;

typedef struct gavl_audio_dither_context_s gavl_audio_dither_context_t;

struct gavl_samplerate_converter_s
  {
  int num_resamplers;
  SRC_STATE ** resamplers;
  SRC_DATA data;
  double ratio;
  };

struct gavl_audio_convert_context_s
  {
  const gavl_audio_frame_t * input_frame;
  gavl_audio_frame_t * output_frame;

  gavl_audio_format_t input_format;
  gavl_audio_format_t output_format;
  
  /* Conversion function to be called */

  gavl_audio_func_t func;

  /* Private data */
  
  gavl_mix_matrix_t * mix_matrix;
  gavl_samplerate_converter_t * samplerate_converter;
  gavl_audio_dither_context_t * dither_context;
    
  /* For chaining */
  
  struct gavl_audio_convert_context_s * next;
  };

gavl_audio_convert_context_t *
gavl_audio_convert_context_create(gavl_audio_format_t  * input_format,
                                  gavl_audio_format_t  * output_format);

gavl_audio_convert_context_t *
gavl_mix_context_create(gavl_audio_options_t * opt,
                        gavl_audio_format_t  * input_format,
                        gavl_audio_format_t  * output_format);

gavl_audio_convert_context_t *
gavl_interleave_context_create(gavl_audio_options_t * opt,
                               gavl_audio_format_t  * input_format,
                               gavl_audio_format_t  * output_format);

gavl_audio_convert_context_t *
gavl_sampleformat_context_create(gavl_audio_options_t * opt,
                                 gavl_audio_format_t  * input_format,
                                 gavl_audio_format_t  * output_format);

gavl_audio_convert_context_t *
gavl_samplerate_context_create(gavl_audio_options_t * opt,
                               gavl_audio_format_t  * input_format,
                               gavl_audio_format_t  * output_format);

/* Resampling support */

gavl_audio_convert_context_t *
gavl_samplerate_context_create(gavl_audio_options_t * opt,
                               gavl_audio_format_t  * input_format,
                               gavl_audio_format_t  * output_format);


/* Destroy samplerate converter */

void gavl_samplerate_converter_destroy(gavl_samplerate_converter_t * s);

/* Destroy dither context */

void gavl_audio_dither_context_destroy(gavl_audio_dither_context_t * s);


