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
#include <string.h>
#include <stdio.h>

#include <audio.h>
#include <libsamplerate/common.h>
#include <mix.h>
#include <accel.h>

struct gavl_audio_converter_s
  {
  gavl_audio_format_t input_format;
  gavl_audio_format_t output_format;
  gavl_audio_options_t opt;
    
  int num_conversions;
  
  gavl_audio_convert_context_t * contexts;
  gavl_audio_convert_context_t * last_context;
  
  gavl_audio_format_t * current_format;
  };

static void destroy_context(gavl_audio_convert_context_t * ctx)
  {
  if(ctx->mix_matrix)
    gavl_destroy_mix_matrix(ctx->mix_matrix);

  if(ctx->samplerate_converter)
    gavl_samplerate_converter_destroy(ctx->samplerate_converter);

  if(ctx->dither_context)
    gavl_audio_dither_context_destroy(ctx->dither_context);

  free(ctx);
  }

static void audio_converter_cleanup(gavl_audio_converter_t* cnv)
  {
  gavl_audio_convert_context_t * ctx;
  
  ctx = cnv->contexts;

  while(ctx)
    {
    ctx = cnv->contexts->next;
    if(ctx && cnv->contexts->output_frame)
      gavl_audio_frame_destroy(cnv->contexts->output_frame);
    destroy_context(cnv->contexts);
    cnv->contexts = ctx;
    }
  cnv->num_conversions = 0;
  cnv->contexts     = NULL;
  cnv->last_context = NULL;
  }

void gavl_audio_converter_destroy(gavl_audio_converter_t* cnv)
  {
  audio_converter_cleanup(cnv);
  free(cnv);
  }

gavl_audio_convert_context_t *
gavl_audio_convert_context_create(gavl_audio_format_t  * input_format,
                                  gavl_audio_format_t  * output_format)
  {
  gavl_audio_convert_context_t * ret;
  ret = calloc(1, sizeof(*ret));
  gavl_audio_format_copy(&ret->input_format,  input_format);
  gavl_audio_format_copy(&ret->output_format, output_format);

  return ret;
  }

static void adjust_format(gavl_audio_format_t * f)
  {
  if(f->num_channels == 1)
    f->interleave_mode = GAVL_INTERLEAVE_NONE;
  
  if((f->num_channels == 2) &&
     (f->interleave_mode == GAVL_INTERLEAVE_2))
    f->interleave_mode = GAVL_INTERLEAVE_ALL;
  }

static void add_context(gavl_audio_converter_t* cnv,
                        gavl_audio_convert_context_t * ctx)
  {
  if(cnv->last_context)
    {
    cnv->last_context->next = ctx;
    cnv->last_context = cnv->last_context->next;
    }
  else
    {
    cnv->last_context = ctx;
    cnv->contexts     = ctx;
    }
  ctx->output_format.samples_per_frame = 0;
  cnv->current_format = &ctx->output_format;
  cnv->num_conversions++;
  }
#if 0
static void dump_context(gavl_audio_convert_context_t * ctx)
  {
  fprintf(stderr, "==== Conversion context ====\n");
  fprintf(stderr, "Input format:\n");
  gavl_audio_format_dump(&ctx->input_format);
  fprintf(stderr, "Output format:\n");
  gavl_audio_format_dump(&ctx->output_format);
  fprintf(stderr, "Func: %p\n", ctx->func);
  
  }
#endif

static void put_samplerate_context(gavl_audio_converter_t* cnv,
                                   gavl_audio_format_t * tmp_format,
                                   int out_samplerate)
  {
  gavl_audio_convert_context_t * ctx;

  /* Sampleformat conversion with GAVL_INTERLEAVE_2 is not supported */
  if(cnv->current_format->interleave_mode == GAVL_INTERLEAVE_2)
    {
    tmp_format->interleave_mode = GAVL_INTERLEAVE_NONE;
    ctx = gavl_interleave_context_create(&cnv->opt,
                                         cnv->current_format,
                                         tmp_format);
    add_context(cnv, ctx);
    }
  
  if(cnv->current_format->sample_format < GAVL_SAMPLE_FLOAT)
    {
    tmp_format->sample_format = GAVL_SAMPLE_FLOAT;
    ctx = gavl_sampleformat_context_create(&cnv->opt,
                                           cnv->current_format,
                                           tmp_format);
    add_context(cnv, ctx);
    }
  
  tmp_format->samplerate = out_samplerate;
  ctx = gavl_samplerate_context_create(&cnv->opt,
                                       cnv->current_format,
                                       tmp_format);
  add_context(cnv, ctx);
  }

int gavl_audio_converter_reinit(gavl_audio_converter_t* cnv)
  {
  int do_mix, do_resample;
  int i;
  gavl_audio_convert_context_t * ctx;

  gavl_audio_format_t * input_format, * output_format;
  
  gavl_audio_format_t tmp_format;

  input_format = &cnv->input_format;
  output_format = &cnv->output_format;
  
#if 0
  fprintf(stderr, "Initializing audio converter, quality: %d, Flags: 0x%08x\n",
          cnv->opt.quality, cnv->opt.accel_flags);
#endif
  
  /* Delete previous conversions */
  audio_converter_cleanup(cnv);

  /* Copy formats and options */
  
  memset(&tmp_format, 0, sizeof(tmp_format));
  gavl_audio_format_copy(&tmp_format, &cnv->input_format);
  
  cnv->current_format = &cnv->input_format;
    
  /* Check if we must mix */

  do_mix = 0;
  
  if((input_format->num_channels != output_format->num_channels) ||
     (gavl_front_channels(input_format) != gavl_front_channels(output_format)) ||
     (gavl_rear_channels(input_format) != gavl_rear_channels(output_format)) ||
     (gavl_side_channels(input_format) != gavl_side_channels(output_format)))
    {
    do_mix = 1;
    }
  if(!do_mix)
    {
    i = (input_format->num_channels < output_format->num_channels) ?
      input_format->num_channels : output_format->num_channels;
    
    while(i--)
      {
      if(input_format->channel_locations[i] != output_format->channel_locations[i])
        {
        do_mix = 1;
        break;
        }
      }
    }

  /* Check if we must resample */

  do_resample = (input_format->samplerate != output_format->samplerate) ? 1 : 0;

  /* Check for resampling. We take care, that we do resampling for the least possible channels */

  if(do_resample &&
     (!do_mix ||
      (do_mix && (input_format->num_channels <= output_format->num_channels))))
    put_samplerate_context(cnv, &tmp_format, output_format->samplerate);
  
  /* Check for mixing */
    
  if(do_mix)
    {
    if(cnv->current_format->interleave_mode != GAVL_INTERLEAVE_NONE)
      {
      tmp_format.interleave_mode = GAVL_INTERLEAVE_NONE;
      ctx = gavl_interleave_context_create(&cnv->opt,
                                           cnv->current_format,
                                           &tmp_format);
      add_context(cnv, ctx);
      }
    
    if((cnv->current_format->sample_format < GAVL_SAMPLE_FLOAT) &&
       ((cnv->opt.quality > 3) ||
        (cnv->output_format.sample_format == GAVL_SAMPLE_FLOAT)))
      {
      tmp_format.sample_format = GAVL_SAMPLE_FLOAT;
      ctx = gavl_sampleformat_context_create(&cnv->opt,
                                             cnv->current_format,
                                             &tmp_format);
      add_context(cnv, ctx);
      }
    else if((cnv->current_format->sample_format < GAVL_SAMPLE_DOUBLE) &&
       ((cnv->opt.quality > 4) ||
        (cnv->output_format.sample_format == GAVL_SAMPLE_DOUBLE)))
      {
      tmp_format.sample_format = GAVL_SAMPLE_DOUBLE;
      ctx = gavl_sampleformat_context_create(&cnv->opt,
                                             cnv->current_format,
                                             &tmp_format);
      add_context(cnv, ctx);
      }

    else if(gavl_bytes_per_sample(cnv->current_format->sample_format) <
            gavl_bytes_per_sample(cnv->output_format.sample_format))
      {
      tmp_format.sample_format = cnv->output_format.sample_format;
      ctx = gavl_sampleformat_context_create(&cnv->opt,
                                             cnv->current_format,
                                             &tmp_format);
      add_context(cnv, ctx);
      }

    tmp_format.num_channels = cnv->output_format.num_channels;
    memcpy(tmp_format.channel_locations, cnv->output_format.channel_locations,
           GAVL_MAX_CHANNELS * sizeof(tmp_format.channel_locations[0]));

    ctx = gavl_mix_context_create(&cnv->opt, cnv->current_format,
                                  &tmp_format);
    add_context(cnv, ctx);
    }

  if(do_resample && do_mix && (input_format->num_channels > output_format->num_channels))
    {
    put_samplerate_context(cnv, &tmp_format, output_format->samplerate);
    }
  
  /* Check, if we must change the sample format */
  
  if(cnv->current_format->sample_format != cnv->output_format.sample_format)
    {
    if(cnv->current_format->interleave_mode == GAVL_INTERLEAVE_2)
      {
      tmp_format.interleave_mode = GAVL_INTERLEAVE_NONE;
      ctx = gavl_interleave_context_create(&cnv->opt,
                                           cnv->current_format,
                                           &tmp_format);
      add_context(cnv, ctx);
      }

    tmp_format.sample_format = cnv->output_format.sample_format;
    ctx = gavl_sampleformat_context_create(&cnv->opt,
                                           cnv->current_format,
                                           &tmp_format);
    add_context(cnv, ctx);
    
    }
     
  /* Final interleaving */

  if(cnv->current_format->interleave_mode != cnv->output_format.interleave_mode)
    {
    tmp_format.interleave_mode = cnv->output_format.interleave_mode;
    ctx = gavl_interleave_context_create(&cnv->opt,
                                         cnv->current_format,
                                         &tmp_format);
    add_context(cnv, ctx);
    }
#if 0
  fprintf(stderr, "Audio converter initialized, %d conversions\n", cnv->num_conversions);
  gavl_audio_format_dump(&cnv->input_format);
  gavl_audio_format_dump(&cnv->output_format);
#endif  
  //  gavl_audio_format_dump(cnv->current_format);

  /* Set samples_per_frame of the first context
     to zero to enable automatic allocation later on */
  
  cnv->input_format.samples_per_frame = 0;
  return cnv->num_conversions;
  }

static void alloc_frames(gavl_audio_converter_t* cnv,
                         int in_samples, double new_ratio)
  {
  gavl_audio_convert_context_t * ctx;
  int out_samples_needed;  
  if((cnv->input_format.samples_per_frame >= in_samples) && (new_ratio < 0.0))
    return;
 
  cnv->input_format.samples_per_frame = in_samples;
  
  /* Set the samples_per_frame member of all intermediate formats */
  
  ctx = cnv->contexts;
  out_samples_needed = in_samples;  

  while(ctx->next)
    {
    ctx->input_format.samples_per_frame = out_samples_needed;
    
    if(ctx->samplerate_converter)
      {
      /* Varispeed */
      if(new_ratio > 0.0)
        {
        out_samples_needed = 
          (int)(0.5 * (ctx->samplerate_converter->ratio + new_ratio) * out_samples_needed) + 10;
        }
      /* Constant ratio */
      else
        {
        out_samples_needed =
          (out_samples_needed * ctx->output_format.samplerate) /
          ctx->input_format.samplerate + 10;
        }
      }
    if(ctx->output_format.samples_per_frame < out_samples_needed)
      {
      ctx->output_format.samples_per_frame = out_samples_needed + 1024;
      if(ctx->output_frame)
        gavl_audio_frame_destroy(ctx->output_frame);
      ctx->output_frame = gavl_audio_frame_create(&ctx->output_format);
      gavl_audio_frame_mute(ctx->output_frame, &ctx->output_format);
      ctx->next->input_frame = ctx->output_frame;
      }
    ctx = ctx->next;
    }
  }

int gavl_audio_converter_init(gavl_audio_converter_t* cnv,
                              const gavl_audio_format_t * input_format,
                              const gavl_audio_format_t * output_format)
  {
  gavl_audio_format_copy(&cnv->input_format, input_format);
  gavl_audio_format_copy(&cnv->output_format, output_format);

  adjust_format(&cnv->input_format);
  adjust_format(&cnv->output_format);
  return gavl_audio_converter_reinit(cnv);
  }

/* Convert audio */

void gavl_audio_convert(gavl_audio_converter_t * cnv,
                        const gavl_audio_frame_t * input_frame,
                        gavl_audio_frame_t * output_frame)
  {
  int i;
  gavl_audio_convert_context_t * ctx;

  /* return early */
  if(!input_frame->valid_samples)
    {
    output_frame->valid_samples = 0;
    return;
    }
  cnv->contexts->input_frame = input_frame;
  cnv->last_context->output_frame = output_frame;
  
  alloc_frames(cnv, input_frame->valid_samples, -1.0);
  
  ctx = cnv->contexts;
  
  for(i = 0; i < cnv->num_conversions; i++)
    {
    ctx->output_frame->valid_samples = 0;
    if(ctx->func)
      {
      ctx->func(ctx);
      if(!ctx->output_frame->valid_samples)
        ctx->output_frame->valid_samples = ctx->input_frame->valid_samples;
      
      if(ctx->output_format.samplerate != ctx->input_format.samplerate)
        ctx->output_frame->timestamp =
          gavl_time_rescale(ctx->input_format.samplerate,
                            ctx->output_format.samplerate,
                            ctx->input_frame->timestamp);
      else
        ctx->output_frame->timestamp = ctx->input_frame->timestamp;
      }
    ctx = ctx->next;
    }
  }

gavl_audio_converter_t * gavl_audio_converter_create()
  {
  gavl_audio_converter_t * ret = calloc(1, sizeof(*ret));
  gavl_audio_options_set_defaults(&ret->opt);
  return ret;
  }

gavl_audio_options_t *
gavl_audio_converter_get_options(gavl_audio_converter_t * cnv)
  {
  return &cnv->opt;
  }


int gavl_audio_converter_init_resample(gavl_audio_converter_t * cnv,
                                   const gavl_audio_format_t * format)
  {
  gavl_audio_format_t tmp_format;
  gavl_audio_convert_context_t * ctx;
  
  gavl_audio_format_copy(&cnv->input_format, format);
  gavl_audio_format_copy(&cnv->output_format, format);
  gavl_audio_format_copy(&tmp_format, format);

  adjust_format(&cnv->input_format);
  adjust_format(&cnv->output_format);

  // Delete previous conversions 
  audio_converter_cleanup(cnv);

  cnv->current_format = &cnv->input_format;

  put_samplerate_context(cnv, &tmp_format, cnv->output_format.samplerate);

  /* put_samplerate will automatically convert sample format and interleave format 
	* we need to check to see if it did or not and add contexts to convert back */
  if(cnv->current_format->sample_format != cnv->output_format.sample_format)
    {
    if(cnv->current_format->interleave_mode == GAVL_INTERLEAVE_2)
      {
      tmp_format.interleave_mode = GAVL_INTERLEAVE_NONE;
      ctx = gavl_interleave_context_create(&cnv->opt,
                                           cnv->current_format,
                                           &tmp_format);
      add_context(cnv, ctx);
      }

    tmp_format.sample_format = cnv->output_format.sample_format;
    ctx = gavl_sampleformat_context_create(&cnv->opt,
                                           cnv->current_format,
                                           &tmp_format);
    add_context(cnv, ctx);
    }

  /* Final interleaving */

  if(cnv->current_format->interleave_mode != cnv->output_format.interleave_mode)
    {
    tmp_format.interleave_mode = cnv->output_format.interleave_mode;
    ctx = gavl_interleave_context_create(&cnv->opt,
                                         cnv->current_format,
                                         &tmp_format);
    add_context(cnv, ctx);
    }

  cnv->input_format.samples_per_frame = 0;

  return cnv->num_conversions;
  }

int gavl_audio_converter_set_resample_ratio(gavl_audio_converter_t * cnv, 
                                            double ratio ) 
  {
  int j;
  gavl_audio_convert_context_t * ctx;

  ctx = cnv->contexts;

  if (ratio > SRC_MAX_RATIO  || ratio < 1/SRC_MAX_RATIO)
    return 0;
	
  while(ctx) 
    {
    if (ctx->samplerate_converter != NULL)
      {
      for (j=0; j < ctx->samplerate_converter->num_resamplers; j++)
        gavl_src_set_ratio( ctx->samplerate_converter->resamplers[j], ratio);
      }
    ctx->samplerate_converter->ratio = ratio;
    ctx = ctx->next;
    }
  return 1;
  }

void gavl_audio_converter_resample(gavl_audio_converter_t * cnv,
		gavl_audio_frame_t * input_frame,
		gavl_audio_frame_t * output_frame,
		double ratio)
  {
  gavl_audio_convert_context_t * ctx;

  cnv->contexts->input_frame = input_frame;
  cnv->last_context->output_frame = output_frame;

  alloc_frames(cnv, input_frame->valid_samples, ratio);

  ctx = cnv->contexts;

  while(ctx) 
    {
    ctx->output_frame->valid_samples = 0;
    if (ctx->samplerate_converter != NULL)
      {
      if (ctx->samplerate_converter->ratio != ratio )
        {
        //ctx->output_format.samplerate = ctx->input_format.samplerate * ratio;
        ctx->samplerate_converter->ratio = ratio;
        ctx->samplerate_converter->data.src_ratio = ratio;
        //for (j=0; j < ctx->samplerate_converter->num_resamplers; j++)
        //	gavl_src_set_ratio( ctx->samplerate_converter->resamplers[j], ratio);
        }
      }

    if(ctx->func)
      {
      ctx->func(ctx);
      // DO WE NEED THIS HERE???
      if(!ctx->output_frame->valid_samples)
        ctx->output_frame->valid_samples = ctx->input_frame->valid_samples;

      ctx->output_frame->timestamp = ctx->input_frame->timestamp;

      }
    ctx = ctx->next;
    }
  }
