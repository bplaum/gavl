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

#include <stdlib.h>
#include <stdio.h>

#include <audio.h>
#include <interleave.h>
#include <accel.h>

gavl_audio_func_t
gavl_find_interleave_converter(gavl_interleave_table_t * t,
                               gavl_audio_format_t * in,
                               gavl_audio_format_t * out)
  {
  int bytes_per_sample = gavl_bytes_per_sample(in->sample_format);
#if 0
  fprintf(stderr, "gavl_find_interleave_converter %s -> %s\n",
          gavl_interleave_mode_to_string(in->interleave_mode),
          gavl_interleave_mode_to_string(out->interleave_mode));
#endif
  switch(in->interleave_mode)
    {
    case GAVL_INTERLEAVE_ALL:
      switch(out->interleave_mode)
        {
        case GAVL_INTERLEAVE_ALL:
          break;
        case GAVL_INTERLEAVE_2:
          switch(bytes_per_sample)
            {
            case 1:
              return t->interleave_all_to_2_8;
              break;
            case 2:
              return t->interleave_all_to_2_16;
              break;
            case 4:
              return t->interleave_all_to_2_32;
              break;
            case 8:
              return t->interleave_all_to_2_64;
              break;
            }
          break;
        case GAVL_INTERLEAVE_NONE:
          if(in->num_channels == 2)
            {
            switch(bytes_per_sample)
              {
              case 1:
                return t->interleave_all_to_none_stereo_8;
                break;
              case 2:
                return t->interleave_all_to_none_stereo_16;
                break;
              case 4:
                return t->interleave_all_to_none_stereo_32;
                break;
              case 8:
                return t->interleave_all_to_none_stereo_64;
                break;
              }
            }
          else
            {
            switch(bytes_per_sample)
              {
              case 1:
                return t->interleave_all_to_none_8;
                break;
              case 2:
                return t->interleave_all_to_none_16;
                break;
              case 4:
                return t->interleave_all_to_none_32;
                break;
              case 8:
                return t->interleave_all_to_none_64;
                break;
              }
            }
          break;
        }
    case GAVL_INTERLEAVE_2:
      switch(out->interleave_mode)
        {
        case GAVL_INTERLEAVE_ALL:
          switch(bytes_per_sample)
            {
            case 1:
              return t->interleave_2_to_all_8;
              break;
            case 2:
              return t->interleave_2_to_all_16;
              break;
            case 4:
              return t->interleave_2_to_all_32;
              break;
            case 8:
              return t->interleave_2_to_all_64;
              break;
            }
          break;
        case GAVL_INTERLEAVE_2:
          break;
        case GAVL_INTERLEAVE_NONE:
          switch(bytes_per_sample)
            {
            case 1:
              return t->interleave_2_to_none_8;
              break;
            case 2:
              return t->interleave_2_to_none_16;
              break;
            case 4:
              return t->interleave_2_to_none_32;
              break;
            case 8:
              return t->interleave_2_to_none_64;
              break;
            }
          break;
        }
      break;
    case GAVL_INTERLEAVE_NONE:
      switch(out->interleave_mode)
        {
        case GAVL_INTERLEAVE_ALL:
          if(in->num_channels == 2)
            {
            switch(bytes_per_sample)
              {
              case 1:
                return t->interleave_none_to_all_stereo_8;
                break;
              case 2:
                return t->interleave_none_to_all_stereo_16;
                break;
              case 4:
                return t->interleave_none_to_all_stereo_32;
                break;
              case 8:
                return t->interleave_none_to_all_stereo_64;
                break;
              }
            break;
            }
          else
            {
            switch(bytes_per_sample)
              {
              case 1:
                return t->interleave_none_to_all_8;
                break;
              case 2:
                return t->interleave_none_to_all_16;
                break;
              case 4:
                return t->interleave_none_to_all_32;
                break;
              case 8:
                return t->interleave_none_to_all_64;
                break;
              }
            }
          break;
        case GAVL_INTERLEAVE_2:
          switch(bytes_per_sample)
            {
            case 1:
              return t->interleave_none_to_2_8;
              break;
            case 2:
              return t->interleave_none_to_2_16;
              break;
            case 4:
              return t->interleave_none_to_2_32;
              break;
            case 8:
              return t->interleave_none_to_2_64;
              break;
            }
          break;
        case GAVL_INTERLEAVE_NONE:
          break;
        }
      break;
    }
  return NULL;
  }

gavl_interleave_table_t *
gavl_create_interleave_table(gavl_audio_options_t * opt)
  {
  gavl_interleave_table_t * ret = calloc(1, sizeof(*ret));

  if(opt->quality || (opt->accel_flags & GAVL_ACCEL_C))
    gavl_init_interleave_funcs_c(ret);
  return ret;
  }

void gavl_destroy_interleave_table(gavl_interleave_table_t * t)
  {
  free(t);
  }

gavl_audio_convert_context_t *
gavl_interleave_context_create(gavl_audio_options_t * opt,
                               gavl_audio_format_t  * input_format,
                               gavl_audio_format_t  * output_format)
  {
  gavl_interleave_table_t * table;
  gavl_audio_convert_context_t * ret;
  ret = gavl_audio_convert_context_create(input_format, output_format);
  ret->output_format.interleave_mode = output_format->interleave_mode;

  table = gavl_create_interleave_table(opt);
  ret->func =
    gavl_find_interleave_converter(table, &ret->input_format,
                                   &ret->output_format);
  
  gavl_destroy_interleave_table(table);
  return ret;
  }
