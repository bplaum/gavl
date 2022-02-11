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
#include <gavl.h>
#include <volume.h>
#include <float_cast.h>

#define CLAMP(val, min, max) if(val < min)val=min;if(val>max)val=max

static void set_volume_s8_c(gavl_volume_control_t * v, void * samples, 
                            int num_samples)
  {
  int i;
  int32_t sample;
  int8_t * s = (int8_t*)samples;
  
  for(i = 0; i < num_samples; i++)
    {
    sample = (s[i] * v->factor_i) >> 8;
    CLAMP(sample, -128, 127);
    s[i] = sample;
    }

  }

static void set_volume_u8_c(gavl_volume_control_t * v, void * samples, 
                            int num_samples)
  {
  int i;
  int32_t sample;
  uint8_t * s = (uint8_t*)samples;
  
  for(i = 0; i < num_samples; i++)
    {
    sample = ((((int)s[i] - 0x80) * v->factor_i) >> 8) + 0x80;
    CLAMP(sample, 0, 255);
    s[i] = sample;
    }
  
  }

static void set_volume_s16_c(gavl_volume_control_t * v, void * samples, 
                            int num_samples)
  {
  int i;
  int64_t sample;
  int16_t * s = (int16_t*)samples;
  
  for(i = 0; i < num_samples; i++)
    {
    sample = ((int64_t)s[i] * v->factor_i) >> 16;
    CLAMP(sample, -32768, 32767);
    s[i] = sample;
    }
  }

static void set_volume_u16_c(gavl_volume_control_t * v, void * samples, 
                            int num_samples)
  {
  int i;
  int64_t sample;
  uint16_t * s = (uint16_t*)samples;
  
  for(i = 0; i < num_samples; i++)
    {
    sample = ((((int64_t)s[i] - 0x8000) * v->factor_i) >> 16) + 0x8000;
    CLAMP(sample, 0, 65535);
    s[i] = sample;
    }
  }

static void set_volume_s32_c(gavl_volume_control_t * v, void * samples,
                              int num_samples)
  {
  int i;
  int64_t sample;
  int32_t * s = (int32_t*)samples;
  
  for(i = 0; i < num_samples; i++)
    {
    sample = ((((int64_t)s[i]) * v->factor_i) >> 31);
    CLAMP(sample, -2147483648LL, 2147483647LL);
    s[i] = sample;
    }
  }

static void set_volume_float_c(gavl_volume_control_t * v,
                               void * samples,
                               int num_samples)
  {
  int i;
  float * s = (float*)samples;
  for(i = 0; i < num_samples; i++)
    s[i] *= v->factor_f;
  }

static void set_volume_double_c(gavl_volume_control_t * v,
                                void * samples,
                                int num_samples)
  {
  int i;
  double * s = (double*)samples;
  for(i = 0; i < num_samples; i++)
    s[i] *= v->factor_f;
  }

void gavl_init_volume_funcs_c(gavl_volume_funcs_t * v)
  {
  v->set_volume_s8 = set_volume_s8_c;
  v->set_volume_u8 = set_volume_u8_c;

  v->set_volume_s16 = set_volume_s16_c;
  v->set_volume_u16 = set_volume_u16_c;
  
  v->set_volume_s32 = set_volume_s32_c;

  v->set_volume_float = set_volume_float_c;
  v->set_volume_double = set_volume_double_c;
  }
