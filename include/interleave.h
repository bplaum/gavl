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

/* Interleaving functions */

typedef struct
  {
  /* 8 bit versions */
  
  gavl_audio_func_t interleave_none_to_all_8;
  gavl_audio_func_t interleave_none_to_all_stereo_8;

  gavl_audio_func_t interleave_all_to_none_8;
  gavl_audio_func_t interleave_all_to_none_stereo_8;

  gavl_audio_func_t interleave_2_to_all_8;
  gavl_audio_func_t interleave_2_to_none_8;

  gavl_audio_func_t interleave_all_to_2_8;
  gavl_audio_func_t interleave_none_to_2_8;

  /* 16 bit versions */
  
  gavl_audio_func_t interleave_none_to_all_16;
  gavl_audio_func_t interleave_none_to_all_stereo_16;

  gavl_audio_func_t interleave_all_to_none_16;
  gavl_audio_func_t interleave_all_to_none_stereo_16;

  gavl_audio_func_t interleave_2_to_all_16;
  gavl_audio_func_t interleave_2_to_none_16;

  gavl_audio_func_t interleave_all_to_2_16;
  gavl_audio_func_t interleave_none_to_2_16;

  /* 32 bit versions */
  
  gavl_audio_func_t interleave_none_to_all_32;
  gavl_audio_func_t interleave_none_to_all_stereo_32;

  gavl_audio_func_t interleave_all_to_none_32;
  gavl_audio_func_t interleave_all_to_none_stereo_32;

  gavl_audio_func_t interleave_2_to_all_32;
  gavl_audio_func_t interleave_2_to_none_32;

  gavl_audio_func_t interleave_all_to_2_32;
  gavl_audio_func_t interleave_none_to_2_32;

  /* 64 bit versions */
  
  gavl_audio_func_t interleave_none_to_all_64;
  gavl_audio_func_t interleave_none_to_all_stereo_64;

  gavl_audio_func_t interleave_all_to_none_64;
  gavl_audio_func_t interleave_all_to_none_stereo_64;

  gavl_audio_func_t interleave_2_to_all_64;
  gavl_audio_func_t interleave_2_to_none_64;

  gavl_audio_func_t interleave_all_to_2_64;
  gavl_audio_func_t interleave_none_to_2_64;

  
  } gavl_interleave_table_t;

gavl_interleave_table_t * gavl_create_interleave_table(gavl_audio_options_t*);
void gavl_destroy_interleave_table(gavl_interleave_table_t *);

/* Find conversion functions */

gavl_audio_func_t
gavl_find_interleave_converter(gavl_interleave_table_t *,
                               gavl_audio_format_t * in,
                               gavl_audio_format_t * out);

void gavl_init_interleave_funcs_c(gavl_interleave_table_t * t);
