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



#include <stdlib.h>
#include <stdio.h>

#include <audio.h>
#include <sampleformat.h>
#include <libgdither/gdither.h>
#include <accel.h>


/* Dither context */

struct gavl_audio_dither_context_s
  {
  GDither dither;
  };

void gavl_audio_dither_context_destroy(gavl_audio_dither_context_t* ctx)
  {
  gdither_free(ctx->dither);
  free(ctx);
  }

static void convert_gdither_ni_f(gavl_audio_convert_context_t * ctx)
  {
  int i;
  //  fprintf(stderr, "Convert gdither\n");
  for(i = 0; i < ctx->input_format.num_channels; i++)
    {
    gdither_runf(ctx->dither_context->dither, 0, ctx->input_frame->valid_samples,
                 ctx->input_frame->channels.f[i],
                 ctx->output_frame->channels.u_8[i]);
    }
  }

static void convert_gdither_s8_ni_f(gavl_audio_convert_context_t * ctx)
  {
  int i, j;
  for(i = 0; i < ctx->input_format.num_channels; i++)
    {
    gdither_runf(ctx->dither_context->dither, 0, ctx->input_frame->valid_samples,
                 ctx->input_frame->channels.f[i],
                 ctx->output_frame->channels.s_8[i]);

    for(j = 0; j < ctx->input_frame->valid_samples; j++)
      ctx->output_frame->channels.s_8[i][j] ^= 0x80;
    }
  }

static void convert_gdither_u16_ni_f(gavl_audio_convert_context_t * ctx)
  {
  int i, j;
  for(i = 0; i < ctx->input_format.num_channels; i++)
    {
    gdither_runf(ctx->dither_context->dither, 0, ctx->input_frame->valid_samples,
                 ctx->input_frame->channels.f[i],
                 ctx->output_frame->channels.u_16[i]);

    for(j = 0; j < ctx->input_frame->valid_samples; j++)
      ctx->output_frame->channels.u_16[i][j] ^= 0x8000;
    }
  }

static void convert_gdither_i_f(gavl_audio_convert_context_t * ctx)
  {
  gdither_runf(ctx->dither_context->dither, 0,
               ctx->input_format.num_channels * ctx->input_frame->valid_samples,
               ctx->input_frame->samples.f,
               ctx->output_frame->samples.u_8);
  }

static void convert_gdither_s8_i_f(gavl_audio_convert_context_t * ctx)
  {
  int j, jmax;
  jmax = ctx->input_format.num_channels * ctx->input_frame->valid_samples;
  gdither_runf(ctx->dither_context->dither, 0,
               jmax,
               ctx->input_frame->samples.f,
               ctx->output_frame->samples.s_8);
  for(j = 0; j < jmax; j++)
    ctx->output_frame->samples.s_8[j] ^= 0x80;
  }

static void convert_gdither_u16_i_f(gavl_audio_convert_context_t * ctx)
  {
  int j, jmax;
  jmax = ctx->input_format.num_channels * ctx->input_frame->valid_samples;
  
  gdither_runf(ctx->dither_context->dither, 0,
               jmax,
               ctx->input_frame->samples.f,
               ctx->output_frame->samples.u_16);
  
  for(j = 0; j < jmax; j++)
    ctx->output_frame->samples.u_16[j] ^= 0x8000;
  }

/* */

static void convert_gdither_ni_d(gavl_audio_convert_context_t * ctx)
  {
  int i;
  //  fprintf(stderr, "Convert gdither\n");
  for(i = 0; i < ctx->input_format.num_channels; i++)
    {
    gdither_run(ctx->dither_context->dither, 0, ctx->input_frame->valid_samples,
                ctx->input_frame->channels.d[i],
                ctx->output_frame->channels.u_8[i]);
    }
  }

static void convert_gdither_s8_ni_d(gavl_audio_convert_context_t * ctx)
  {
  int i, j;
  for(i = 0; i < ctx->input_format.num_channels; i++)
    {
    gdither_run(ctx->dither_context->dither, 0, ctx->input_frame->valid_samples,
                 ctx->input_frame->channels.d[i],
                 ctx->output_frame->channels.s_8[i]);

    for(j = 0; j < ctx->input_frame->valid_samples; j++)
      ctx->output_frame->channels.s_8[i][j] ^= 0x80;
    }
  }

static void convert_gdither_u16_ni_d(gavl_audio_convert_context_t * ctx)
  {
  int i, j;
  for(i = 0; i < ctx->input_format.num_channels; i++)
    {
    gdither_run(ctx->dither_context->dither, 0, ctx->input_frame->valid_samples,
                 ctx->input_frame->channels.d[i],
                 ctx->output_frame->channels.u_16[i]);

    for(j = 0; j < ctx->input_frame->valid_samples; j++)
      ctx->output_frame->channels.u_16[i][j] ^= 0x8000;
    }
  }

static void convert_gdither_i_d(gavl_audio_convert_context_t * ctx)
  {
  gdither_run(ctx->dither_context->dither, 0,
               ctx->input_format.num_channels * ctx->input_frame->valid_samples,
               ctx->input_frame->samples.d,
               ctx->output_frame->samples.u_8);
  }

static void convert_gdither_s8_i_d(gavl_audio_convert_context_t * ctx)
  {
  int j, jmax;
  jmax = ctx->input_format.num_channels * ctx->input_frame->valid_samples;
  gdither_run(ctx->dither_context->dither, 0,
               jmax,
               ctx->input_frame->samples.d,
               ctx->output_frame->samples.s_8);
  for(j = 0; j < jmax; j++)
    ctx->output_frame->samples.s_8[j] ^= 0x80;
  }

static void convert_gdither_u16_i_d(gavl_audio_convert_context_t * ctx)
  {
  int j, jmax;
  jmax = ctx->input_format.num_channels * ctx->input_frame->valid_samples;
  
  gdither_run(ctx->dither_context->dither, 0,
              jmax,
              ctx->input_frame->samples.d,
              ctx->output_frame->samples.u_16);
  
  for(j = 0; j < jmax; j++)
    ctx->output_frame->samples.u_16[j] ^= 0x8000;
  }

gavl_sampleformat_table_t * gavl_create_sampleformat_table(gavl_audio_options_t * opt,
                                                           gavl_interleave_mode_t interleave_mode)
  {
  gavl_sampleformat_table_t * ret = calloc(1, sizeof(*ret));

  if(opt->quality || (opt->accel_flags & GAVL_ACCEL_C))
    gavl_init_sampleformat_funcs_c(ret, interleave_mode);
  return ret;
  }

static GDitherType get_dither_type(gavl_audio_options_t * opt)
  {
  switch(opt->dither_mode)
    {
    case GAVL_AUDIO_DITHER_NONE:
      return GDitherNone;
      break;
    case GAVL_AUDIO_DITHER_RECT:
      return GDitherRect;
      break;
    case GAVL_AUDIO_DITHER_TRI:
      return GDitherTri;
      break;
    case GAVL_AUDIO_DITHER_SHAPED:
      return GDitherShaped;
      break;
    case GAVL_AUDIO_DITHER_AUTO:
      switch(opt->quality)
        {
        case 1:
        case 2:
          return GDitherNone;
          break;
        case 3:
          return GDitherRect;
          break;
        case 4:
          return GDitherTri;
          break;
        case 5:
          return GDitherShaped;
          break;
        }
      break;
    }
  return GDitherNone;
  }

/* Create sampleformat converter. Samples are interleaved or non interleaved */

gavl_audio_convert_context_t *
gavl_sampleformat_context_create(gavl_audio_options_t * opt,
                                 gavl_audio_format_t * in_format,
                                 gavl_audio_format_t * out_format)
  {
  gavl_audio_convert_context_t * ret;

  /* Native sampleformat conversion routines */
  gavl_sampleformat_table_t * table;

  /* libgdither support */
  GDitherType dither_type;
  GDitherSize dither_bit_depth;
  int dither_depth;
#if 0    
  fprintf(stderr, "Gavl: initializing sampleformat converter %s -> %s\n",
          gavl_sample_format_to_string(in_format->sample_format),
          gavl_sample_format_to_string(out_format->sample_format));
#endif  
  ret = gavl_audio_convert_context_create(in_format, out_format);
  ret->output_format.sample_format = out_format->sample_format;

  dither_type = get_dither_type(opt);
  
  if((dither_type == GDitherNone) ||
     (gavl_bytes_per_sample(out_format->sample_format) > 2) ||
     (in_format->sample_format < GAVL_SAMPLE_FLOAT))
    {
    table = gavl_create_sampleformat_table(opt, in_format->interleave_mode);
    
    
    ret->func =
      gavl_find_sampleformat_converter(table, &ret->input_format,
                                       &ret->output_format);
    /*   if(!ret->func) */
    /*     fprintf(stderr, "No function found\n"); */
    gavl_destroy_sampleformat_table(table);
    }
  else
    {
    switch(out_format->sample_format)
      {
      case GAVL_SAMPLE_U8:
        dither_bit_depth = GDither8bit;
        dither_depth = 8;

        if(in_format->sample_format == GAVL_SAMPLE_DOUBLE)
          {
          if(in_format->interleave_mode == GAVL_INTERLEAVE_NONE)
            ret->func = convert_gdither_ni_d;
          else if(in_format->interleave_mode == GAVL_INTERLEAVE_ALL)
            ret->func = convert_gdither_i_d;
          }
        else
          {
          if(in_format->interleave_mode == GAVL_INTERLEAVE_NONE)
          ret->func = convert_gdither_ni_f;
          else if(in_format->interleave_mode == GAVL_INTERLEAVE_ALL)
            ret->func = convert_gdither_i_f;
          }
        
        break;
      case GAVL_SAMPLE_S8:
        dither_bit_depth = GDither8bit;
        dither_depth = 8;

        if(in_format->sample_format == GAVL_SAMPLE_DOUBLE)
          {
          if(in_format->interleave_mode == GAVL_INTERLEAVE_NONE)
            ret->func = convert_gdither_s8_ni_d;
          else if(in_format->interleave_mode == GAVL_INTERLEAVE_ALL)
            ret->func = convert_gdither_s8_i_d;
          }
        else
          {
          if(in_format->interleave_mode == GAVL_INTERLEAVE_NONE)
            ret->func = convert_gdither_s8_ni_f;
          else if(in_format->interleave_mode == GAVL_INTERLEAVE_ALL)
            ret->func = convert_gdither_s8_i_f;
          }
        break;
      case GAVL_SAMPLE_U16:
        dither_bit_depth = GDither16bit;
        dither_depth = 16;

        if(in_format->sample_format == GAVL_SAMPLE_DOUBLE)
          {
          if(in_format->interleave_mode == GAVL_INTERLEAVE_NONE)
            ret->func = convert_gdither_u16_ni_d;
          else if(in_format->interleave_mode == GAVL_INTERLEAVE_ALL)
            ret->func = convert_gdither_u16_i_d;
          }
        else
          {
          if(in_format->interleave_mode == GAVL_INTERLEAVE_NONE)
            ret->func = convert_gdither_u16_ni_f;
          else if(in_format->interleave_mode == GAVL_INTERLEAVE_ALL)
            ret->func = convert_gdither_u16_i_f;
          }
        break;
      case GAVL_SAMPLE_S16:
        dither_bit_depth = GDither16bit;
        dither_depth = 16;

        if(in_format->sample_format == GAVL_SAMPLE_DOUBLE)
          {
          if(in_format->interleave_mode == GAVL_INTERLEAVE_NONE)
            ret->func = convert_gdither_ni_d;
          else if(in_format->interleave_mode == GAVL_INTERLEAVE_ALL)
            ret->func = convert_gdither_i_d;
          }
        else
          {
          if(in_format->interleave_mode == GAVL_INTERLEAVE_NONE)
            ret->func = convert_gdither_ni_f;
          else if(in_format->interleave_mode == GAVL_INTERLEAVE_ALL)
            ret->func = convert_gdither_i_f;
          
          }
        break;
      default:
        fprintf(stderr, "BUG: Invalid dither initialization\n");
        fprintf(stderr, "Input format\n");
        gavl_audio_format_dump(&ret->input_format);
        fprintf(stderr, "Output format\n");
        gavl_audio_format_dump(&ret->output_format);
        return NULL;
      }
    
    /* Dither (float -> 8/16 bit) */
    
    ret->dither_context = calloc(1, sizeof(*(ret->dither_context)));

    //    fprintf(stderr, "Gdither, %d %d %d\n", dither_type,
    //            dither_bit_depth, dither_depth);
    
    ret->dither_context->dither =  gdither_new(dither_type, 1,
                                               dither_bit_depth, dither_depth);
    
    }

  
  return ret;
  }


gavl_audio_func_t gavl_find_sampleformat_converter(gavl_sampleformat_table_t * t,
                                                   gavl_audio_format_t * in,
                                                   gavl_audio_format_t * out)
  {
  switch(in->sample_format)
    {
    case GAVL_SAMPLE_U8:
      switch(out->sample_format)
        {
        case GAVL_SAMPLE_U8:
          // Nothing
          break;
        case GAVL_SAMPLE_S8:
          return t->swap_sign_8;
          break;
        case GAVL_SAMPLE_U16:
          return t->u_8_to_u_16;
          break;
        case GAVL_SAMPLE_S16:
          return t->u_8_to_s_16;
          break;
        case GAVL_SAMPLE_S32:
          return t->u_8_to_s_32;
          break;
        case GAVL_SAMPLE_FLOAT:
          return t->convert_u8_to_float;
          break;
        case GAVL_SAMPLE_DOUBLE:
          return t->convert_u8_to_double;
          break;
        case GAVL_SAMPLE_NONE:
          break;
        }
      break;
    case GAVL_SAMPLE_S8:
      switch(out->sample_format)
        {
        case GAVL_SAMPLE_U8:
          return t->swap_sign_8;
          break;
        case GAVL_SAMPLE_S8:
          // Nothing
          break;
        case GAVL_SAMPLE_U16:
          return t->s_8_to_u_16;
          break;
        case GAVL_SAMPLE_S16:
          return t->s_8_to_s_16;
          break;
        case GAVL_SAMPLE_S32:
          return t->s_8_to_s_32;
          break;
        case GAVL_SAMPLE_FLOAT:
          return t->convert_s8_to_float;
          break;
        case GAVL_SAMPLE_DOUBLE:
          return t->convert_s8_to_double;
          break;
        case GAVL_SAMPLE_NONE:
          break;
        }
      break;
    case GAVL_SAMPLE_U16:
      switch(out->sample_format)
        {
        case GAVL_SAMPLE_U8:
          return t->convert_16_to_8;
          break;
        case GAVL_SAMPLE_S8:
          return t->convert_16_to_8_swap;
          break;
        case GAVL_SAMPLE_U16:
          // Nothing
          break;
        case GAVL_SAMPLE_S16:
          return t->swap_sign_16;
          break;
        case GAVL_SAMPLE_S32:
          return t->u_16_to_s_32;
          break;
        case GAVL_SAMPLE_FLOAT:
          return t->convert_u16_to_float;
          break;
        case GAVL_SAMPLE_DOUBLE:
          return t->convert_u16_to_double;
          break;
          
        case GAVL_SAMPLE_NONE:
          break;
        }
      break;
    case GAVL_SAMPLE_S16:
      switch(out->sample_format)
        {
        case GAVL_SAMPLE_U8:
          return t->convert_16_to_8_swap;
          break;
        case GAVL_SAMPLE_S8:
          return t->convert_16_to_8;
          break;
        case GAVL_SAMPLE_U16:
          return t->swap_sign_16;
          break;
        case GAVL_SAMPLE_S16:
          // Nothing
          break;
        case GAVL_SAMPLE_S32:
          return t->s_16_to_s_32;
          break;
        case GAVL_SAMPLE_FLOAT:
          return t->convert_s16_to_float;
          break;
        case GAVL_SAMPLE_DOUBLE:
          return t->convert_s16_to_double;
          break;
        case GAVL_SAMPLE_NONE:
          break;
        }
      break;
    case GAVL_SAMPLE_S32:
      switch(out->sample_format)
        {
        case GAVL_SAMPLE_U8:
          return t->convert_32_to_8_swap;
          break;
        case GAVL_SAMPLE_S8:
          return t->convert_32_to_8;
          break;
        case GAVL_SAMPLE_U16:
          return t->convert_32_to_16_swap;
          break;
        case GAVL_SAMPLE_S16:
          return t->convert_32_to_16;
          break;
        case GAVL_SAMPLE_S32:
          // Nothing
          break;
        case GAVL_SAMPLE_FLOAT:
          return t->convert_s32_to_float;
          break;
        case GAVL_SAMPLE_DOUBLE:
          return t->convert_s32_to_double;
          break;

        case GAVL_SAMPLE_NONE:
          break;
        }
      break;
    case GAVL_SAMPLE_FLOAT:
      switch(out->sample_format)
        {
        case GAVL_SAMPLE_U8:
          return t->convert_float_to_u8;
          break;
        case GAVL_SAMPLE_S8:
          return t->convert_float_to_s8;
          break;
        case GAVL_SAMPLE_U16:
          return t->convert_float_to_u16;
          break;
        case GAVL_SAMPLE_S16:
          return t->convert_float_to_s16;
          break;
        case GAVL_SAMPLE_S32:
          return t->convert_float_to_s32;
          break;
        case GAVL_SAMPLE_FLOAT:
          // Nothing
          break;
        case GAVL_SAMPLE_DOUBLE:
          return t->convert_float_to_double;
          break;
        case GAVL_SAMPLE_NONE:
          break;
        }
      break;
    case GAVL_SAMPLE_DOUBLE:
      switch(out->sample_format)
        {
        case GAVL_SAMPLE_U8:
          return t->convert_double_to_u8;
          break;
        case GAVL_SAMPLE_S8:
          return t->convert_double_to_s8;
          break;
        case GAVL_SAMPLE_U16:
          return t->convert_double_to_u16;
          break;
        case GAVL_SAMPLE_S16:
          return t->convert_double_to_s16;
          break;
        case GAVL_SAMPLE_S32:
          return t->convert_double_to_s32;
          break;
        case GAVL_SAMPLE_FLOAT:
          return t->convert_double_to_float;
          break;
        case GAVL_SAMPLE_DOUBLE:
          // Nothing
          break;
        case GAVL_SAMPLE_NONE:
          break;
        }
      break;
    case GAVL_SAMPLE_NONE:
      break;
    }
  return NULL;
  }

void gavl_destroy_sampleformat_table(gavl_sampleformat_table_t * t)
  {
  free(t);
  }

