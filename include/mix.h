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

typedef struct gavl_mix_output_channel_s gavl_mix_output_channel_t;

typedef void (*gavl_mix_func_t)(gavl_mix_output_channel_t * channel,
                                const gavl_audio_frame_t * input_frame,
                                gavl_audio_frame_t * output_frame);
                                
typedef struct
  {
  gavl_mix_func_t copy_func;
  gavl_mix_func_t mix_1_to_1;
  gavl_mix_func_t mix_2_to_1;
  gavl_mix_func_t mix_3_to_1;
  gavl_mix_func_t mix_4_to_1;
  gavl_mix_func_t mix_5_to_1;
  gavl_mix_func_t mix_6_to_1;
  gavl_mix_func_t mix_all_to_1;
  } gavl_mixer_table_t;

typedef struct gavl_mix_input_channel_s
  {
  int index; /* Which input channel */
  union      /* Weighing factor     */
    {
    double  f_float;
    int     f_int;
    } factor;
  } gavl_mix_input_channel_t;

struct gavl_mix_output_channel_s
  {
  int num_inputs;
  int index;
  gavl_mix_input_channel_t inputs[GAVL_MAX_CHANNELS];
  gavl_mix_func_t func;
  };

struct gavl_mix_matrix_s
  {
  gavl_mix_output_channel_t output_channels[GAVL_MAX_CHANNELS];
  gavl_mixer_table_t mixer_table;
  };

gavl_mix_matrix_t *
gavl_create_mix_matrix(gavl_audio_options_t * opt,
                       gavl_audio_format_t * in,
                       gavl_audio_format_t * out);

void gavl_destroy_mix_matrix(gavl_mix_matrix_t *);

void gavl_mix_audio(gavl_audio_convert_context_t * ctx);

void gavl_setup_mix_funcs_c(gavl_mixer_table_t * c,
                            gavl_audio_format_t * f);
