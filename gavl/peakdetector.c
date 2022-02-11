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
#include <math.h>
#include <string.h>

#include <gavl/peakdetector.h>

struct gavl_peak_detector_s
  {
  int64_t min_i[GAVL_MAX_CHANNELS];
  int64_t max_i[GAVL_MAX_CHANNELS];
  double min_d[GAVL_MAX_CHANNELS];
  double max_d[GAVL_MAX_CHANNELS];
  double abs_d[GAVL_MAX_CHANNELS];
  
  gavl_audio_format_t format;
  void (*update_channel)(gavl_peak_detector_t*,void*,int num, int offset,
                         int advance, int channel);
  void (*update)(gavl_peak_detector_t*, gavl_audio_frame_t*);

  gavl_audio_sink_t * sink;

  /* Callbacks */
  gavl_update_peak_callback peak_callback;
  gavl_update_peaks_callback peaks_callback;
  void * callback_priv;
  };

static void update_none(gavl_peak_detector_t*pd, gavl_audio_frame_t*f)
  {
  int i;
  for(i = 0; i < pd->format.num_channels; i++)
    {
    pd->update_channel(pd, f->channels.s_8[i], f->valid_samples,
                       0, 1, i);
    }
  }

static void update_all(gavl_peak_detector_t*pd, gavl_audio_frame_t*f)
  {
  int i;
  for(i = 0; i < pd->format.num_channels; i++)
    {
    pd->update_channel(pd, f->samples.s_8,
                       f->valid_samples,
                       i, pd->format.num_channels, i);
    }
  }

static void update_2(gavl_peak_detector_t*pd, gavl_audio_frame_t*f)
  {
  int i;
  for(i = 0; i < pd->format.num_channels/2; i++)
    {
    pd->update_channel(pd, f->samples.s_8,
                       f->valid_samples,
                       0, 2, 2*i);
    pd->update_channel(pd, f->samples.s_8,
                       f->valid_samples,
                       1, 2, 2*i+1);
    }
  if(pd->format.num_channels % 2)
    pd->update_channel(pd, f->channels.s_8[pd->format.num_channels-1],
                       f->valid_samples, 0, 1, pd->format.num_channels-1);
  }

static void update_channel_u8(gavl_peak_detector_t * pd, void * _samples,
                              int num, int offset,
                              int advance, int channel)
  {
  int i;
  uint8_t * samples = (uint8_t *)_samples;
  samples += offset;
  for(i = 0; i < num; i++)
    {
    if(*samples > pd->max_i[channel]) pd->max_i[channel] = *samples;
    if(*samples < pd->min_i[channel]) pd->min_i[channel] = *samples;
    samples += advance;
    }
  pd->min_d[channel] = (double)((int)pd->min_i[channel]-0x80) / 128.0;
  pd->max_d[channel] = (double)((int)pd->max_i[channel]-0x80) / 127.0;
  }

static void update_channel_s8(gavl_peak_detector_t * pd, void * _samples,
                              int num, int offset,
                              int advance, int channel)
  {
  int i;
  int8_t * samples = (int8_t *)_samples;
  samples += offset;
  for(i = 0; i < num; i++)
    {
    if(*samples > pd->max_i[channel]) pd->max_i[channel] = *samples;
    if(*samples < pd->min_i[channel]) pd->min_i[channel] = *samples;
    samples += advance;
    }
  pd->min_d[channel] = (double)((int)pd->min_i[channel]) / 128.0;
  pd->max_d[channel] = (double)((int)pd->max_i[channel]) / 127.0;
  }

static void update_channel_u16(gavl_peak_detector_t * pd, void * _samples,
                               int num, int offset,
                               int advance, int channel)
  {
  int i;
  uint16_t * samples = (uint16_t *)_samples;
  samples += offset;
  for(i = 0; i < num; i++)
    {
    if(*samples > pd->max_i[channel]) pd->max_i[channel] = *samples;
    if(*samples < pd->min_i[channel]) pd->min_i[channel] = *samples;
    samples += advance;
    }
  pd->min_d[channel] = (double)((int)pd->min_i[channel]-0x8000) / 32768.0;
  pd->max_d[channel] = (double)((int)pd->max_i[channel]-0x8000) / 32767.0;
  }

static void update_channel_s16(gavl_peak_detector_t * pd, void * _samples,
                               int num, int offset,
                               int advance, int channel)
  {
  int i;
  int16_t * samples = (int16_t *)_samples;
  samples += offset;
  for(i = 0; i < num; i++)
    {
    if(*samples > pd->max_i[channel]) pd->max_i[channel] = *samples;
    if(*samples < pd->min_i[channel]) pd->min_i[channel] = *samples;
    samples += advance;
    }
  pd->min_d[channel] = (double)((int)pd->min_i[channel]) / 32768.0;
  pd->max_d[channel] = (double)((int)pd->max_i[channel]) / 32767.0;
  }

static void update_channel_s32(gavl_peak_detector_t * pd, void * _samples,
                               int num, int offset,
                               int advance, int channel)
  {
  int i;
  int32_t * samples = (int32_t *)_samples;
  samples += offset;
  for(i = 0; i < num; i++)
    {
    if(*samples > pd->max_i[channel]) pd->max_i[channel] = *samples;
    if(*samples < pd->min_i[channel]) pd->min_i[channel] = *samples;
    samples += advance;
    }
  pd->min_d[channel] = (double)((int)pd->min_i[channel]) / 2147483648.0;
  pd->max_d[channel] = (double)((int)pd->max_i[channel]) / 2147483647.0;
  }

static void update_channel_float(gavl_peak_detector_t * pd, void * _samples,
                                 int num, int offset,
                                 int advance, int channel)
  {
  int i;
  float * samples = (float*)_samples;
  samples += offset;
  for(i = 0; i < num; i++)
    {
    if(*samples > pd->max_d[channel]) pd->max_d[channel] = *samples;
    if(*samples < pd->min_d[channel]) pd->min_d[channel] = *samples;
    samples += advance;
    }
  }

static void update_channel_double(gavl_peak_detector_t * pd, void * _samples,
                                  int num, int offset,
                                  int advance, int channel)
  {
  int i;
  double * samples = (double*)_samples;
  samples += offset;
  for(i = 0; i < num; i++)
    {
    if(*samples > pd->max_d[channel]) pd->max_d[channel] = *samples;
    if(*samples < pd->min_d[channel]) pd->min_d[channel] = *samples;
    samples += advance;
    }
  }

gavl_peak_detector_t * gavl_peak_detector_create()
  {
  gavl_peak_detector_t * ret;
  ret = calloc(1, sizeof(*ret));
  return ret;
  }


void gavl_peak_detector_destroy(gavl_peak_detector_t *pd)
  {
  if(pd->sink)
    gavl_audio_sink_destroy(pd->sink);
  free(pd);
  }

static gavl_sink_status_t put_frame_func(void * priv,
                                         gavl_audio_frame_t * frame)
  {
  int i;
  gavl_peak_detector_t *pd = priv;
  pd->update(pd, frame);

  for(i = 0; i < pd->format.num_channels; i++)
    {
    pd->abs_d[i] = (pd->max_d[i] > fabs(pd->min_d[i])) ? 
      pd->max_d[i] : fabs(pd->min_d[i]);
    }

  if(pd->peaks_callback)
    pd->peaks_callback(pd->callback_priv, frame->valid_samples,
                       pd->min_d, pd->max_d, pd->abs_d);
  if(pd->peak_callback)
    {
    double min, max, abs;
    gavl_peak_detector_get_peak(pd, &min, &max, &abs);
    pd->peak_callback(pd->callback_priv, frame->valid_samples,
                      min, max, abs);
    }
  
  return GAVL_SINK_OK;
  }

const gavl_audio_format_t *
gavl_peak_detector_get_format(gavl_peak_detector_t * pd)
  {
  return &pd->format;
  }

void gavl_peak_detector_set_format(gavl_peak_detector_t *pd,
                                   const gavl_audio_format_t * format)
  {
  gavl_audio_format_copy(&pd->format, format);
  switch(pd->format.interleave_mode)
    {
    case GAVL_INTERLEAVE_NONE:
      pd->update = update_none;
      break;
    case GAVL_INTERLEAVE_ALL:
      pd->update = update_all;
      break;
    case GAVL_INTERLEAVE_2:
      pd->update = update_2;
      break;
    }
  switch(pd->format.sample_format)
    {
    case GAVL_SAMPLE_U8:
      pd->update_channel = update_channel_u8;
      break;
    case GAVL_SAMPLE_S8:
      pd->update_channel = update_channel_s8;
      break;
    case GAVL_SAMPLE_U16:
      pd->update_channel = update_channel_u16;
      break;
    case GAVL_SAMPLE_S16:
      pd->update_channel = update_channel_s16;
      break;
    case GAVL_SAMPLE_S32:
      pd->update_channel = update_channel_s32;
      break;
    case GAVL_SAMPLE_FLOAT:
      pd->update_channel = update_channel_float;
      break;
    case GAVL_SAMPLE_DOUBLE:
      pd->update_channel = update_channel_double;
      break;
    case GAVL_SAMPLE_NONE:
      break;
    }
  gavl_peak_detector_reset(pd);

  if(pd->sink)
    gavl_audio_sink_destroy(pd->sink);
  
  pd->sink = gavl_audio_sink_create(NULL, put_frame_func, pd, format);
  }

void gavl_peak_detector_update(gavl_peak_detector_t *pd,
                               gavl_audio_frame_t * frame)
  {
  gavl_audio_sink_put_frame(pd->sink, frame);
  }



void gavl_peak_detector_get_peak(gavl_peak_detector_t * pd,
                                 double * min, double * max,
                                 double * abs)
  {
  int i;
  double min1 = 0.0, max1 = 0.0, abs1 = 0.0;
  
  for(i = 0; i < pd->format.num_channels; i++)
    {
    if(pd->min_d[i] < min1)
      min1 = pd->min_d[i];

    if(pd->max_d[i] > max1)
      max1 = pd->max_d[i];

    if(pd->abs_d[i] > abs1)
      abs1 = pd->abs_d[i];
    }
  if(min)
    *min = min1;
  if(max)
    *max = max1;
  if(abs)
    *abs = abs1;
  }

void gavl_peak_detector_get_peaks(gavl_peak_detector_t * pd,
                                 double * min, double * max,
                                 double * abs)
  {
  if(min)
    memcpy(min, pd->min_d, pd->format.num_channels * sizeof(*min));
  if(max)
    memcpy(max, pd->max_d, pd->format.num_channels * sizeof(*max));
  if(abs)
    memcpy(abs, pd->abs_d, pd->format.num_channels * sizeof(*max));
  }

void gavl_peak_detector_reset(gavl_peak_detector_t * pd)
  {
  int i;
  switch(pd->format.sample_format)
    {
    case GAVL_SAMPLE_U8:
      for(i = 0; i < pd->format.num_channels; i++)
        {
        pd->min_i[i] = 0x80;
        pd->max_i[i] = 0x80;
        }
      break;
    case GAVL_SAMPLE_U16:
      for(i = 0; i < pd->format.num_channels; i++)
        {
        pd->min_i[i] = 0x8000;
        pd->max_i[i] = 0x8000;
        }
      break;
    case GAVL_SAMPLE_S8:
    case GAVL_SAMPLE_S16:
    case GAVL_SAMPLE_S32:
      for(i = 0; i < pd->format.num_channels; i++)
        {
        pd->min_i[i] = 0x0;
        pd->max_i[i] = 0x0;
        }
      break;
    case GAVL_SAMPLE_FLOAT:
    case GAVL_SAMPLE_DOUBLE:
    case GAVL_SAMPLE_NONE:
      break;
    }
  for(i = 0; i < pd->format.num_channels; i++)
    {
    pd->min_d[i] = 0.0;
    pd->max_d[i] = 0.0;
    pd->abs_d[i] = 0.0;
    }
  }

GAVL_PUBLIC
gavl_audio_sink_t * gavl_peak_detector_get_sink(gavl_peak_detector_t *pd)
  {
  return pd->sink;
  }

GAVL_PUBLIC
void gavl_peak_detector_set_callbacks(gavl_peak_detector_t * pd,
                                      gavl_update_peak_callback peak_callback,
                                      gavl_update_peaks_callback peaks_callback,
                                      void * priv)
  {
  pd->peak_callback = peak_callback;
  pd->peaks_callback = peaks_callback;
  pd->callback_priv = priv;
  }
