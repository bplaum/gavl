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


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <config.h>
#include <gavl.h>
#include <accel.h>
#include <bswap.h>

#include <memalign.h>

#define ALIGNMENT_BYTES 16

gavl_audio_frame_t *
gavl_audio_frame_create(const gavl_audio_format_t * format)
  {
  gavl_audio_frame_t * ret;
  int num_samples;
  int i;
  ret = calloc(1, sizeof(*ret));

  if(!format)
    return ret;
  
  num_samples = ALIGNMENT_BYTES *
    ((format->samples_per_frame + ALIGNMENT_BYTES - 1) / ALIGNMENT_BYTES);
  
  switch(format->sample_format)
    {
    case GAVL_SAMPLE_U8:
      ret->channel_stride = num_samples;
      ret->samples.u_8 =
        gavl_memalign(ALIGNMENT_BYTES, num_samples * format->num_channels);

      for(i = 0; i < format->num_channels; i++)
        ret->channels.u_8[i] = &ret->samples.u_8[i*num_samples];

      break;
    case GAVL_SAMPLE_S8:
      ret->channel_stride = num_samples;
      ret->samples.s_8 =
        gavl_memalign(ALIGNMENT_BYTES, num_samples * format->num_channels);

      for(i = 0; i < format->num_channels; i++)
        ret->channels.s_8[i] = &ret->samples.s_8[i*num_samples];

      break;
    case GAVL_SAMPLE_U16:
      ret->channel_stride = num_samples * 2;
      ret->samples.u_16 =
        gavl_memalign(ALIGNMENT_BYTES, 2 * num_samples * format->num_channels);
      for(i = 0; i < format->num_channels; i++)
        ret->channels.u_16[i] = &ret->samples.u_16[i*num_samples];

      break;
    case GAVL_SAMPLE_S16:
      ret->channel_stride = num_samples * 2;
      ret->samples.s_16 =
        gavl_memalign(ALIGNMENT_BYTES, 2 * num_samples * format->num_channels);
      for(i = 0; i < format->num_channels; i++)
        ret->channels.s_16[i] = &ret->samples.s_16[i*num_samples];

      break;

    case GAVL_SAMPLE_S32:
      ret->channel_stride = num_samples * 4;
      ret->samples.s_32 =
        gavl_memalign(ALIGNMENT_BYTES, 4 * num_samples * format->num_channels);
      for(i = 0; i < format->num_channels; i++)
        ret->channels.s_32[i] = &ret->samples.s_32[i*num_samples];

      break;

    case GAVL_SAMPLE_FLOAT:
      ret->channel_stride = num_samples * sizeof(float);
      ret->samples.f =
        gavl_memalign(ALIGNMENT_BYTES, sizeof(float) * num_samples * format->num_channels);

      for(i = 0; i < format->num_channels; i++)
        ret->channels.f[i] = &ret->samples.f[i*num_samples];

      break;
    case GAVL_SAMPLE_DOUBLE:
      ret->channel_stride = num_samples * sizeof(double);
      ret->samples.d =
        gavl_memalign(ALIGNMENT_BYTES, sizeof(double) * num_samples * format->num_channels);

      for(i = 0; i < format->num_channels; i++)
        ret->channels.d[i] = &ret->samples.d[i*num_samples];

      break;
    case GAVL_SAMPLE_NONE:
      {
      fprintf(stderr, "Sample format not specified for audio frame\n");
      return ret;
      }
    }
  return ret;
  }

void gavl_audio_frame_copy_ptrs(const gavl_audio_format_t * format,
                                gavl_audio_frame_t * dst,
                                const gavl_audio_frame_t * src)
  {
  int i;
  dst->samples.s_8 = src->samples.s_8;
  for(i = 0; i < format->num_channels; i++)
    dst->channels.s_8[i] = src->channels.s_8[i];
  dst->timestamp = src->timestamp;
  dst->valid_samples = src->valid_samples;
  }


void gavl_audio_frame_destroy(gavl_audio_frame_t * frame)
  {
  if(frame->samples.s_8)
    free(frame->samples.s_8);
  free(frame);
  }

void gavl_audio_frame_mute_samples(gavl_audio_frame_t * frame,
                                   const gavl_audio_format_t * format,
                                   int num_samples)
  {
  int i;
  int imax;
  imax = format->num_channels * num_samples;
  
  switch(format->sample_format)
    {
    case GAVL_SAMPLE_NONE:
      break;
    case GAVL_SAMPLE_U8:
      for(i = 0; i < imax; i++)
        frame->samples.u_8[i] = 0x80;
      break;
    case GAVL_SAMPLE_S8:
      for(i = 0; i < imax; i++)
        frame->samples.u_8[i] = 0x00;
      break;
    case GAVL_SAMPLE_U16:
      for(i = 0; i < imax; i++)
        frame->samples.u_16[i] = 0x8000;
      break;
    case GAVL_SAMPLE_S16:
      for(i = 0; i < imax; i++)
        frame->samples.s_16[i] = 0x0000;
      break;
    case GAVL_SAMPLE_S32:
      for(i = 0; i < imax; i++)
        frame->samples.s_32[i] = 0x00000000;
      break;
    case GAVL_SAMPLE_FLOAT:
      for(i = 0; i < imax; i++)
        frame->samples.f[i] = 0.0;
      break;
    case GAVL_SAMPLE_DOUBLE:
      for(i = 0; i < imax; i++)
        frame->samples.d[i] = 0.0;
      break;
    }
  frame->valid_samples = num_samples;
  }

void gavl_audio_frame_mute(gavl_audio_frame_t * frame,
                           const gavl_audio_format_t * format)
  {
  gavl_audio_frame_mute_samples(frame,
                                format,
                                format->samples_per_frame);
  }

void gavl_audio_frame_mute_channel(gavl_audio_frame_t * frame,
                                   const gavl_audio_format_t * format,
                                   int channel)
  {
  int i;
  int imax;
  int offset = 0;
  int advance = 0;

  switch(format->interleave_mode)
    {
    case GAVL_INTERLEAVE_ALL:
      offset = channel;
      advance = format->num_channels;
      break;
    case GAVL_INTERLEAVE_NONE:
      offset = channel * format->samples_per_frame;
      advance = 1;
      break;
    case GAVL_INTERLEAVE_2:
      if(channel & 1)
        offset = (channel-1) * format->samples_per_frame + 1;
      else
        offset = channel * format->samples_per_frame;

      if((channel == format->num_channels - 1) &&
         (format->num_channels & 1))
        advance = 1;
      else
        advance = 2;
      
      break;
    }
  
  imax = format->samples_per_frame;
  
  switch(format->sample_format)
    {
    case GAVL_SAMPLE_NONE:
      break;
    case GAVL_SAMPLE_U8:
      for(i = 0; i < imax; i++)
        frame->samples.u_8[offset + advance * i] = 0x80;
      break;
    case GAVL_SAMPLE_S8:
      for(i = 0; i < imax; i++)
        frame->samples.u_8[offset + advance * i] = 0x00;
      break;
    case GAVL_SAMPLE_U16:
      for(i = 0; i < imax; i++)
        frame->samples.u_16[offset + advance * i] = 0x8000;
      break;
    case GAVL_SAMPLE_S16:
      for(i = 0; i < imax; i++)
        frame->samples.s_16[offset + advance * i] = 0x0000;
      break;
    case GAVL_SAMPLE_S32:
      for(i = 0; i < imax; i++)
        frame->samples.s_32[offset + advance * i] = 0x00000000;
      break;
    case GAVL_SAMPLE_FLOAT:
      for(i = 0; i < imax; i++)
        frame->samples.f[offset + advance * i] = 0.0;
      break;
    case GAVL_SAMPLE_DOUBLE:
      for(i = 0; i < imax; i++)
        frame->samples.d[offset + advance * i] = 0.0;
      break;
    }
  }


int gavl_audio_frame_copy(const gavl_audio_format_t * format,
                          gavl_audio_frame_t * dst,
                          const gavl_audio_frame_t * src,
                          int out_pos,
                          int in_pos,
                          int out_size,
                          int in_size)
  {
  int i;
  int bytes_per_sample;
  int samples_to_copy;
  gavl_init_memcpy();
  samples_to_copy = (in_size < out_size) ? in_size : out_size;

  if(!dst)
    return samples_to_copy;

  bytes_per_sample = gavl_bytes_per_sample(format->sample_format);
  
  switch(format->interleave_mode)
    {
    case GAVL_INTERLEAVE_NONE:
      for(i = 0; i < format->num_channels; i++)
        {
        gavl_memcpy(&dst->channels.s_8[i][out_pos * bytes_per_sample],
                    &src->channels.s_8[i][in_pos * bytes_per_sample],
                    samples_to_copy * bytes_per_sample);
        }
      break;
    case GAVL_INTERLEAVE_2:
      for(i = 0; i < format->num_channels/2; i++)
        {
        gavl_memcpy(&dst->channels.s_8[i*2][2 * out_pos * bytes_per_sample],
                    &src->channels.s_8[i*2][2 * in_pos * bytes_per_sample],
                    2*samples_to_copy * bytes_per_sample);
        }
      /* Last channel is not interleaved */
      if(format->num_channels & 1)
        {
        gavl_memcpy(&dst->channels.s_8[format->num_channels-1][2 * out_pos * bytes_per_sample],
                    &src->channels.s_8[format->num_channels-1][2 * in_pos * bytes_per_sample],
                    2*samples_to_copy * bytes_per_sample);
        }
      break;
    case GAVL_INTERLEAVE_ALL:
      gavl_memcpy(&dst->samples.s_8[format->num_channels * out_pos * bytes_per_sample],
                  &src->samples.s_8[format->num_channels * in_pos * bytes_per_sample],
                  format->num_channels * samples_to_copy * bytes_per_sample);
      break;
    }
  return samples_to_copy;
  }

void gavl_audio_frame_null(gavl_audio_frame_t * f)
  {
  memset(f, 0, sizeof(*f));
  }

GAVL_PUBLIC
void gavl_audio_frame_get_subframe(const gavl_audio_format_t * format,
                                   gavl_audio_frame_t * src,
                                   gavl_audio_frame_t * dst,
                                   int start, int len)
  {
  int i;
  int bytes_per_sample = gavl_bytes_per_sample(format->sample_format);

  switch(format->interleave_mode)
    {
    case GAVL_INTERLEAVE_ALL:
      dst->samples.s_8 = src->samples.s_8 +
        bytes_per_sample * start * format->num_channels;
      break;
    case GAVL_INTERLEAVE_NONE:
      for(i = 0; i < format->num_channels; i++)
        dst->channels.s_8[i] =
          src->channels.s_8[i] + bytes_per_sample * start;
      break;
    case GAVL_INTERLEAVE_2:
      for(i = 0; i < format->num_channels/2; i++)
        dst->channels.s_8[i*2] =
          src->channels.s_8[i*2] + bytes_per_sample * start * 2;
      if(format->num_channels & 1)
        {
        dst->channels.s_8[format->num_channels-1] =
          src->channels.s_8[format->num_channels-1] +
          bytes_per_sample * start;
        }
      break;
    }
  dst->valid_samples = len;
  }

GAVL_PUBLIC
int gavl_audio_frame_skip(const gavl_audio_format_t * format,
                          gavl_audio_frame_t * f,
                          int num_samples)
  {
  int i;
  int samples_left;
  int bytes_per_sample = gavl_bytes_per_sample(format->sample_format);

  if(num_samples > f->valid_samples)
    num_samples = f->valid_samples;
  
  samples_left = f->valid_samples - num_samples;

  if(!samples_left)
    {
    f->valid_samples = 0;
    return num_samples;
    }
  
  switch(format->interleave_mode)
    {
    case GAVL_INTERLEAVE_ALL:
      memmove(f->samples.s_8, f->samples.s_8 +
              bytes_per_sample * num_samples * format->num_channels,
              bytes_per_sample * samples_left * format->num_channels);
      break;
    case GAVL_INTERLEAVE_NONE:
      for(i = 0; i < format->num_channels; i++)
        memmove(f->channels.s_8[i], f->channels.s_8[i] +
                bytes_per_sample * num_samples,
                bytes_per_sample * samples_left);
      break;
    case GAVL_INTERLEAVE_2:
      for(i = 0; i < format->num_channels/2; i++)
        memmove(f->channels.s_8[2*i], f->channels.s_8[2*i] +
                bytes_per_sample * num_samples * 2,
                bytes_per_sample * samples_left * 2);
      if(format->num_channels & 1)
        {
        memmove(f->channels.s_8[format->num_channels-1],
                f->channels.s_8[format->num_channels-1] +
                bytes_per_sample * num_samples,
                bytes_per_sample * samples_left);
        }
      break;
    }
  f->valid_samples = samples_left;
  f->timestamp += num_samples;
  return num_samples;
  }


int gavl_audio_frames_equal(const gavl_audio_format_t * format,
                            const gavl_audio_frame_t * f1,
                            const gavl_audio_frame_t * f2)
  {
  int i;
  int bytes;

  if(f1->valid_samples != f2->valid_samples)
    return 0;
  
  switch(format->interleave_mode)
    {
    case GAVL_INTERLEAVE_ALL:
      bytes = f1->valid_samples * format->num_channels *
        gavl_bytes_per_sample(format->sample_format);
      if(memcmp(f1->samples.s_8, f2->samples.s_8, bytes))
        return 0;
      break;
    case GAVL_INTERLEAVE_2:
      bytes = f1->valid_samples * 2 *
        gavl_bytes_per_sample(format->sample_format);

      for(i = 0; i < format->num_channels/2; i++)
        {
        if(memcmp(f1->channels.s_8[i*2], f2->channels.s_8[i*2], bytes))
          return 0;
        }
      if(format->num_channels % 2)
        {
        if(memcmp(f1->channels.s_8[format->num_channels-1],
                  f2->channels.s_8[format->num_channels-1], bytes/2))
          return 0;
        }
      break;
    case GAVL_INTERLEAVE_NONE:
      bytes = f1->valid_samples *
        gavl_bytes_per_sample(format->sample_format);
      for(i = 0; i < format->num_channels/2; i++)
        {
        if(memcmp(f1->channels.s_8[i], f2->channels.s_8[i], bytes))
          return 0;
        }
      break;
    }
  return 1;
  }

static void do_plot(const gavl_audio_format_t * format,
                    const gavl_audio_frame_t * frame,
                    FILE * out)
  {
  int i, j;
  for(j = 0; j < frame->valid_samples; j++)
    {
    fprintf(out, "%d", j);
    switch(format->sample_format)
      {
      case GAVL_SAMPLE_U8:
        for(i = 0; i < format->num_channels; i++)
          fprintf(out, " %d", frame->channels.u_8[i][j]);
        break;
      case GAVL_SAMPLE_S8:
        for(i = 0; i < format->num_channels; i++)
          fprintf(out, " %d", frame->channels.s_8[i][j]);

        break;
      case GAVL_SAMPLE_U16:
        for(i = 0; i < format->num_channels; i++)
          fprintf(out, " %d", frame->channels.u_16[i][j]);

        break;
      case GAVL_SAMPLE_S16:
        for(i = 0; i < format->num_channels; i++)
          fprintf(out, " %d", frame->channels.s_16[i][j]);

        break;

      case GAVL_SAMPLE_S32:
        for(i = 0; i < format->num_channels; i++)
          fprintf(out, " %d", frame->channels.s_32[i][j]);

        break;

      case GAVL_SAMPLE_FLOAT:
        for(i = 0; i < format->num_channels; i++)
          fprintf(out, " %f", frame->channels.f[i][j]);

        break;
      case GAVL_SAMPLE_DOUBLE:
        for(i = 0; i < format->num_channels; i++)
          fprintf(out, " %f", frame->channels.d[i][j]);

        break;
      case GAVL_SAMPLE_NONE:
        break;
      }
    fprintf(out, "\n");
    }
  }


int gavl_audio_frame_plot(const gavl_audio_format_t * format,
                          const gavl_audio_frame_t * frame,
                          const char * name_base)
  {
  int i;
  gavl_audio_converter_t * cnv;
  gavl_audio_format_t plot_format;
  int do_convert;
  FILE * out;
  char * filename = malloc(strlen(name_base) + 5);

  /* Write data file */
  strcpy(filename, name_base);
  strcat(filename, ".dat");
  out = fopen(filename, "w");
  if(!out)
    return 0;

  cnv = gavl_audio_converter_create();
  gavl_audio_format_copy(&plot_format, 
                         format);
  plot_format.interleave_mode = GAVL_INTERLEAVE_NONE;

  plot_format.samples_per_frame = frame->valid_samples;
  
  do_convert =
    gavl_audio_converter_init(cnv, format, &plot_format);
  
  if(do_convert)
    {
    gavl_audio_frame_t * plot_frame = gavl_audio_frame_create(&plot_format);
    gavl_audio_convert(cnv, frame, plot_frame);
    
    do_plot(&plot_format, plot_frame, out);
    gavl_audio_frame_destroy(plot_frame);
    }
  else
    do_plot(format, frame, out);

  fclose(out);

  /* Write gnuplot file */
  strcpy(filename, name_base);
  strcat(filename, ".gnu");
  out = fopen(filename, "w");
  if(!out)
    return 0;
  
  fprintf(out, "plot ");

  for(i = 0; i < format->num_channels; i++)
    {
    if(i)
      fprintf(out, ", ");
    fprintf(out, "\"%s.dat\" using 1:%d title \"%s\"",
            name_base, i+2,
            gavl_channel_id_to_string(format->channel_locations[i]));
    }
  fprintf(out, "\n");
  fclose(out);
  return 1;
  }

int gavl_audio_frame_continuous(const gavl_audio_format_t * format,
                                const gavl_audio_frame_t * frame)
  {
  int sample_size;
  int i;
  switch(format->interleave_mode)
    {
    case GAVL_INTERLEAVE_ALL:
      return 1;
      break;
    case GAVL_INTERLEAVE_NONE:
      sample_size = gavl_bytes_per_sample(format->sample_format);
      for(i = 1; i < format->num_channels; i++)
        {
        if(frame->channels.s_8[i] - frame->channels.s_8[i-1] !=
           sample_size * frame->valid_samples)
          return 0;
        }
      return 1;
      break;
    case GAVL_INTERLEAVE_2:
      sample_size = gavl_bytes_per_sample(format->sample_format);
      for(i = 2; i < format->num_channels; i+=2)
        {
        if(frame->channels.s_8[i] - frame->channels.s_8[i-2] !=
           2 * sample_size * frame->valid_samples)
          return 0;
        }
      return 1;
      break;
    }
  return 0;
  }

void gavl_audio_frame_set_channels(gavl_audio_frame_t * f,
                                   const gavl_audio_format_t * format,
                                   uint8_t * data)
  {
  int i;
  int sample_size = gavl_bytes_per_sample(format->sample_format);

  f->samples.u_8 = data;
  for(i = 0; i < format->num_channels; i++)
    f->channels.u_8[i] = data + i * sample_size * f->valid_samples;
  }
