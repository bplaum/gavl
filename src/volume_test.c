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
#include <stdio.h>
#include <math.h>
#include <string.h>

static struct
  {
  char * suffix;
  gavl_sample_format_t sampleformat;
  }
sampleformats[] =
  {
    { "_u8", GAVL_SAMPLE_U8 },
    { "_s8", GAVL_SAMPLE_S8 },
    { "_u16", GAVL_SAMPLE_U16  },
    { "_s16", GAVL_SAMPLE_S16 },
    { "_s32", GAVL_SAMPLE_S32 },
    { "_f", GAVL_SAMPLE_FLOAT },
  };

#define NUM_SAMPLES 1024

static gavl_audio_frame_t * create_ref_frame(gavl_audio_format_t * format)
  {
  int i;
  float t;
  gavl_audio_frame_t * ret;
  
  memset(format, 0, sizeof(*format));
  format->num_channels = 1;
  format->interleave_mode = GAVL_INTERLEAVE_ALL;
  format->sample_format = GAVL_SAMPLE_FLOAT;
  format->samples_per_frame = NUM_SAMPLES;
  format->samplerate = 48000;
  
  gavl_set_channel_setup(format);
  
  ret = gavl_audio_frame_create(format);
  ret->valid_samples = NUM_SAMPLES;
  
  for(i = 0; i < NUM_SAMPLES; i++)
    {
    t = (float)i * 2.0 * M_PI / (float)(NUM_SAMPLES-1);
    ret->samples.f[i] = sin(t);
    }
  return ret;
  }

static void write_frames(gavl_audio_frame_t * ref_frame,
                         gavl_audio_frame_t * out_frame,
                         const char * suffix)
  {
  FILE * out;
  char filename[1024];
  int i;

  sprintf(filename, "volume%s.dat", suffix);
  out = fopen(filename, "w");
  for(i = 0; i < NUM_SAMPLES; i++)
    {
    fprintf(out, "%d %f %f\n", i,
            ref_frame->samples.f[i],
            out_frame->samples.f[i]);
    }
  fclose(out);

  fprintf(stderr, "Wrote %s\n", filename);
  
  sprintf(filename, "volume%s.gnu", suffix);
  out = fopen(filename, "w");

  fprintf(out, "plot \"volume%s.dat\" using 1:2 title \"Before\"\n",
          suffix);

  fprintf(out, "replot \"volume%s.dat\" using 1:3 title \"After\"\n",
          suffix);
  fclose(out);
  fprintf(stderr, "Wrote %s\n", filename);
  }


int main(int argc, char ** argv)
  {
  int do_convert, i;
  gavl_audio_frame_t * ref_frame;
  gavl_audio_converter_t * cnv_in;
  gavl_audio_converter_t * cnv_out;
  
  gavl_audio_frame_t * frame_1;
  gavl_audio_frame_t * frame_2;

  gavl_audio_frame_t * out_frame;
  
  gavl_audio_format_t ref_format;
  gavl_audio_format_t format;
  gavl_volume_control_t * vc;
  
  cnv_in = gavl_audio_converter_create();
  cnv_out = gavl_audio_converter_create();
  
  vc = gavl_volume_control_create();
  
  ref_frame = create_ref_frame(&ref_format);

  out_frame = gavl_audio_frame_create(&ref_format);
  
  /* -6.0 dB (factor 0.5) */
  gavl_volume_control_set_volume(vc, -6.0);
  
  for(i = 0; i < sizeof(sampleformats) / sizeof(sampleformats[0]); i++)
    {
    gavl_audio_format_copy(&format, &ref_format);
    format.sample_format = sampleformats[i].sampleformat;
    
    gavl_volume_control_set_format(vc, &format);
    
    frame_1 = gavl_audio_frame_create(&format);
    frame_2 = gavl_audio_frame_create(&format);

    do_convert = gavl_audio_converter_init(cnv_in, &ref_format, &format);
    if(do_convert)
      gavl_audio_converter_init(cnv_out, &format, &ref_format);
    
    if(do_convert)
      gavl_audio_convert(cnv_in, ref_frame, frame_1);
    else
      {
      gavl_audio_frame_copy(&format, frame_1, ref_frame, 0, 0, NUM_SAMPLES, NUM_SAMPLES);
      frame_1->valid_samples = NUM_SAMPLES;
      }
    
    gavl_audio_frame_copy(&format, frame_2, frame_1, 0, 0, NUM_SAMPLES, NUM_SAMPLES);
    frame_2->valid_samples = NUM_SAMPLES;
    
    gavl_volume_control_apply(vc, frame_2);

    if(do_convert)
      gavl_audio_convert(cnv_out, frame_2, out_frame);
    else
      {
      gavl_audio_frame_copy(&format, out_frame, frame_2, 0, 0, NUM_SAMPLES, NUM_SAMPLES);
      out_frame->valid_samples = NUM_SAMPLES;
      }
    write_frames(ref_frame, out_frame, sampleformats[i].suffix);
    
    gavl_audio_frame_destroy(frame_1);
    gavl_audio_frame_destroy(frame_2);
    }

  gavl_audio_frame_destroy(ref_frame);
  gavl_audio_frame_destroy(out_frame);
  gavl_volume_control_destroy(vc);
  gavl_audio_converter_destroy(cnv_in);
  gavl_audio_converter_destroy(cnv_out);
  return 0;
  }
