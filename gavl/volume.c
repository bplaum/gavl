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



#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <config.h>
#include <gavl.h>
#include <volume.h>

static void set_factor_i(gavl_volume_control_t * v)
  {
  switch(v->format.sample_format)
    {
    case GAVL_SAMPLE_S8:
    case GAVL_SAMPLE_U8:
      v->factor_i = (int64_t)(v->factor_f * 0x100+0.5);
      break;
    case GAVL_SAMPLE_S16:
    case GAVL_SAMPLE_U16:
      v->factor_i = (int64_t)(v->factor_f * 0x10000+0.5);
      break;
    case GAVL_SAMPLE_S32:
      v->factor_i = (int64_t)(v->factor_f * 0x80000000LL+0.5);
      break;
    case GAVL_SAMPLE_NONE:
    case GAVL_SAMPLE_FLOAT:
    case GAVL_SAMPLE_DOUBLE:
      break;
    }
  }

static void set_volume_interleave_none(gavl_volume_control_t * v,
                                       gavl_audio_frame_t * frame)
  {
  int i;
  /* All channels separate */

  for(i = 0; i < v->format.num_channels; i++)
    {
    v->set_volume_channel(v, frame->channels.s_8[i],
                          frame->valid_samples);
    }
  }

static void set_volume_interleave_2(gavl_volume_control_t * v,
                                    gavl_audio_frame_t * frame)
  {
  int i;
  int imax;

  imax = v->format.num_channels/2;
  
  for(i = 0; i < imax; i++)
    {
    v->set_volume_channel(v, frame->channels.s_8[2*i],
                          frame->valid_samples * 2);
    }

  if(v->format.num_channels % 2)
    {
    v->set_volume_channel(v, frame->channels.s_8[2*imax],
                          frame->valid_samples);
    }
  }

static void set_volume_interleave_all(gavl_volume_control_t * v,
                                      gavl_audio_frame_t * frame)
  {
  v->set_volume_channel(v, frame->samples.s_8,
                        frame->valid_samples * v->format.num_channels);
  }


/* Create / destroy */
  
gavl_volume_control_t * gavl_volume_control_create()
  {
  gavl_volume_control_t * v;
  v = calloc(1, sizeof(*v));
  return v;
  }

void gavl_volume_control_destroy(gavl_volume_control_t * v)
  {
  free(v);
  }

/* Set format: can be called multiple times with one instance */

void gavl_volume_control_set_format(gavl_volume_control_t * v,
                                    const gavl_audio_format_t * format)
  {
  gavl_volume_funcs_t * funcs;

  gavl_audio_format_copy(&v->format, format);
  
  funcs = gavl_volume_funcs_create();
  
  switch(format->sample_format)
    {
    case GAVL_SAMPLE_S8:
      v->set_volume_channel = funcs->set_volume_s8;
      break;

    case GAVL_SAMPLE_U8:
      v->set_volume_channel = funcs->set_volume_u8;
      break;

    case GAVL_SAMPLE_S16:
      v->set_volume_channel = funcs->set_volume_s16;
      break;

    case GAVL_SAMPLE_U16:
      v->set_volume_channel = funcs->set_volume_u16;
      break;

    case GAVL_SAMPLE_S32:
      v->set_volume_channel = funcs->set_volume_s32;
      break;

    case GAVL_SAMPLE_FLOAT:
      v->set_volume_channel = funcs->set_volume_float;
      break;
    case GAVL_SAMPLE_DOUBLE:
      v->set_volume_channel = funcs->set_volume_double;
      break;

    case GAVL_SAMPLE_NONE:
      break;
    }

  gavl_volume_funcs_destroy(funcs);

  switch(format->interleave_mode)
    {
    case GAVL_INTERLEAVE_NONE:
      v->set_volume = set_volume_interleave_none;
      break;
    case GAVL_INTERLEAVE_2:
      v->set_volume = set_volume_interleave_2;
      break;
    case GAVL_INTERLEAVE_ALL:
      v->set_volume = set_volume_interleave_all;
      break;
    }
  set_factor_i(v);
  }

/* Apply the volume control to one audio frame */
  
void gavl_volume_control_apply(gavl_volume_control_t * v,
                               gavl_audio_frame_t * frame)
  {
  v->set_volume(v, frame);
  }


void gavl_volume_control_set_volume(gavl_volume_control_t * v,
                                    float volume)
  {
  v->factor_f = pow(10, volume/20.0);
  set_factor_i(v);
  }


gavl_volume_funcs_t * gavl_volume_funcs_create()
  {
  gavl_volume_funcs_t * ret;
  ret = calloc(1, sizeof(*ret));
  
  gavl_init_volume_funcs_c(ret);

#ifdef ARCH_X86
  //  gavl_init_volume_funcs_mmx(ret);
#endif
  
  return ret;
  }

void gavl_volume_funcs_destroy(gavl_volume_funcs_t * funcs)
  {
  free(funcs);
  }

/*
 *   one e.g. is -40.0
 *
 *   < -40.0 -> 0
 *     -40.0 -> 1
 *       0.0 -> steps - 1
 *   > one -> (int)((volume - one)/(0.0 - one) * (steps - 2)+0.5) + 1;
 */

int gavl_volume_to_int(float volume, float one, int max)
  {
  float volume_norm;
  float volume_int;

  if(volume < one)
    return 0;

  volume_norm = (volume - one)/(0.0 - one); // one -> 0, 0 -> 1

  volume_int = (int)(volume_norm * (max - 1)+0.5) + 1;
  if(volume_int > max)
    volume_int = max;
  return volume_int;
  }

float gavl_volume_to_float(int volume, float one, int max)
  {
  float volume_norm;
  if(volume < 1)
    return -10000.0; // Should be small enough even for double precision

  volume_norm = (float)(volume - 1) / ((float)(max - 1));
  
  return one - volume_norm * one;
  }

