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




#include <config.h>
#include <gavl.h>
#include <gavldsp.h>
#include <accel.h> /* Private header */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/time.h>
#include <time.h>
#include <errno.h>

#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif

#ifdef HAVE_SCHED_SETAFFINITY
#define __USE_GNU
#include <sched.h>
#endif

// #undef ARCH_X86

#define OUT_PFMT GAVL_RGB_24
#define IN_PFMT GAVL_PIXELFORMAT_NONE

#define SCALE_X (1<<0)
#define SCALE_Y (1<<1)

static int opt_scaledir = 0;

// #define INIT_RUNS 5
// #define NUM_RUNS 10

#define INIT_RUNS 10
#define NUM_RUNS  200

int do_html = 0;
gavl_pixelformat_t opt_pfmt1 = GAVL_PIXELFORMAT_NONE;
gavl_pixelformat_t opt_pfmt2 = GAVL_PIXELFORMAT_NONE;

typedef struct
  {
  uint64_t times[NUM_RUNS];
  uint64_t min;
  uint64_t max;
  uint64_t avg;

  void (*func)(void*);
  void (*init)(void*);
  void *data;

  int accel_supported;
  int num_discard;
  
  } gavl_benchmark_t;

/* Fill frames with random numbers */

static void init_video_frame(gavl_video_format_t * format, gavl_video_frame_t * f)
  {
  int i, j, num;
  int num_planes;
  int32_t * pixels_i;
  float * pixels_f;
  int sub_h, sub_v;

  switch(format->pixelformat)
    {
    case GAVL_RGB_FLOAT:
      for(i = 0; i < format->image_height; i++)
        {
        pixels_f = (float*)(f->planes[0] + i * f->strides[0]);
        for(j = 0; j < format->image_width; j++)
          {
          pixels_f[0] = (float)rand() / RAND_MAX;
          pixels_f[1] = (float)rand() / RAND_MAX;
          pixels_f[2] = (float)rand() / RAND_MAX;
          pixels_f+=3;
          }
        }
      break;
    case GAVL_RGBA_FLOAT:
      for(i = 0; i < format->image_height; i++)
        {
        pixels_f = (float*)(f->planes[0] + i * f->strides[0]);
        for(j = 0; j < format->image_width; j++)
          {
          pixels_f[0] = (float)rand() / RAND_MAX;
          pixels_f[1] = (float)rand() / RAND_MAX;
          pixels_f[2] = (float)rand() / RAND_MAX;
          pixels_f[4] = (float)rand() / RAND_MAX;
          pixels_f+=4;
          }
        }
      break;
    default:
      num_planes = gavl_pixelformat_num_planes(format->pixelformat);
      gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
      num = 0;
      for(i = 0; i < num_planes; i++)
        {
        num += (f->strides[i] * format->image_height)
          / (1 + (!!i) * (sub_v-1));
        }
      
      pixels_i = (int*)(f->planes[0]);
      
      for(i = 0; i < format->image_height/sub_v; i++)
        {
        pixels_i[i] = rand();
        }
      
      break;
    }
  
  }


static void gavl_benchmark_run(gavl_benchmark_t * b)
  {
  int i;
  uint64_t time_before;
  uint64_t time_after;
  b->accel_supported = gavl_accel_supported();
  b->avg = 0;
  
  for(i = 0; i < INIT_RUNS + NUM_RUNS; i++)
    {
    if(b->init)
      b->init(b->data);
    time_before = gavl_benchmark_get_time(b->accel_supported);
    b->func(b->data);
    time_after = gavl_benchmark_get_time(b->accel_supported);

    if(i >= INIT_RUNS)
      {
      b->times[i-INIT_RUNS] = time_after - time_before;
      b->avg += b->times[i-INIT_RUNS];
      }
    }
  
  b->avg /= NUM_RUNS;
  b->num_discard = 0;
  
  for(i = 0; i < NUM_RUNS; i++)
    {
    if(b->times[i] > (3 * b->avg)/2)
      {
      b->times[i] = 0;
      b->num_discard++;
      }
    }
  i = 0;
  while(!b->times[i])
    i++;

  b->min = b->times[i];
  b->max = b->times[i];
  b->avg = b->times[i];
  i++;
  
  while(i < NUM_RUNS)
    {
    if(!b->times[i])
      {
      i++;
      continue;
      }
    if(b->times[i] > b->max)
      b->max = b->times[i];
    if(b->times[i] < b->min)
      b->min = b->times[i];

    b->avg += b->times[i];
    i++;
    }
  b->avg /= (NUM_RUNS-b->num_discard);
  }

static void gavl_benchmark_print_header(gavl_benchmark_t * b)
  {
  if(do_html)
    {
    printf("<td align=\"right\">Average</td><td align=\"right\">Minimum</td><td align=\"right\">Maximum</td><td align=\"right\">Discarded</td>");
    }
  else
    {
    printf(" Average  Minimum  Maximum  Discarded");
    }
  }

static void gavl_benchmark_print_results(gavl_benchmark_t * b)
  {
  if(do_html)
    {
    printf("<td align=\"right\">%"PRId64"</td><td align=\"right\">%"PRId64"</td><td align=\"right\">%"PRId64"</td><td align=\"right\">%d</td>",
         b->avg, b->min, b->max, b->num_discard);
    }
  else
    printf("%8"PRId64" %8"PRId64" %8"PRId64"       %4d",
           b->avg, b->min, b->max, b->num_discard);
  }

/* Audio conversions */

typedef struct
  {
  gavl_audio_format_t in_format;
  gavl_audio_format_t out_format;
  
  gavl_audio_converter_t * cnv;
  gavl_audio_options_t * opt;
  
  gavl_audio_frame_t * in_frame;
  gavl_audio_frame_t * out_frame;
  
  } audio_convert_context_t;

static void audio_convert_context_create(audio_convert_context_t * ctx)
  {
  ctx->cnv = gavl_audio_converter_create();
  ctx->opt = gavl_audio_converter_get_options(ctx->cnv);
  }


static void audio_convert_context_init(audio_convert_context_t * ctx)
  {
  ctx->in_frame = gavl_audio_frame_create(&ctx->in_format);
  ctx->out_frame = gavl_audio_frame_create(&ctx->out_format);
  gavl_audio_converter_init(ctx->cnv, &ctx->in_format, &ctx->out_format);
  }

static void audio_convert_context_cleanup(audio_convert_context_t * ctx)
  {
  gavl_audio_frame_destroy(ctx->in_frame);
  gavl_audio_frame_destroy(ctx->out_frame);
  }

static void audio_convert_context_destroy(audio_convert_context_t * ctx)
  {
  gavl_audio_converter_destroy(ctx->cnv);
  }

static void audio_convert_init(void * data)
  {
  audio_convert_context_t * ctx = data;
  ctx->in_frame->valid_samples = ctx->in_format.samples_per_frame;
  }

static void audio_convert_func(void * data)
  {
  audio_convert_context_t * ctx = data;
  gavl_audio_convert(ctx->cnv, ctx->in_frame, ctx->out_frame);
  }

static const gavl_sample_format_t sampleformats[] =
  {
    GAVL_SAMPLE_U8, /*!< Unsigned 8 bit */
    GAVL_SAMPLE_S8, /*!< Signed 8 bit */
    GAVL_SAMPLE_U16, /*!< Unsigned 16 bit */
    GAVL_SAMPLE_S16, /*!< Signed 16 bit */
    GAVL_SAMPLE_S32, /*!< Signed 32 bit */
    GAVL_SAMPLE_FLOAT,  /*!< Floating point (-1.0 .. 1.0) */
    GAVL_SAMPLE_DOUBLE  /*!< Double (-1.0 .. 1.0) */
  };

static const struct
  {
  gavl_audio_dither_mode_t mode;
  const char * name;
  }
dither_modes[] =
  {
    { GAVL_AUDIO_DITHER_NONE, "None" },
    { GAVL_AUDIO_DITHER_RECT, "Rect"}, 
    { GAVL_AUDIO_DITHER_TRI,  "Triangular" },
    { GAVL_AUDIO_DITHER_SHAPED, "Shaped" }
  };

static void benchmark_sampleformat()
  {
  int num_sampleformats;
  int num_dither_modes;
  gavl_sample_format_t in_format;
  gavl_sample_format_t out_format;

  int i, j, k;

  audio_convert_context_t ctx;
  gavl_benchmark_t b;
  memset(&ctx, 0, sizeof(ctx));
  memset(&b, 0, sizeof(b));

  b.init = audio_convert_init;
  b.func = audio_convert_func;
  b.data = &ctx;
  
  ctx.in_format.num_channels = 2;
  ctx.in_format.samplerate = 48000;
  ctx.in_format.interleave_mode = GAVL_INTERLEAVE_NONE;
  ctx.in_format.channel_locations[0] = GAVL_CHID_NONE;
  ctx.in_format.samples_per_frame = 10240;

  gavl_set_channel_setup(&ctx.in_format);
  
  gavl_audio_format_copy(&ctx.out_format, &ctx.in_format);

  num_sampleformats = sizeof(sampleformats)/sizeof(sampleformats[0]);
  num_dither_modes = sizeof(dither_modes)/sizeof(dither_modes[0]);

  audio_convert_context_create(&ctx);

  printf("Conversion of %d samples, %d channels\n", ctx.in_format.samples_per_frame,
         ctx.in_format.num_channels);
  
  if(do_html)
    {
    printf("<p><table border=\"1\" width=\"100%%\"><tr><td>Conversion</td><td>Dithering</td>");
    gavl_benchmark_print_header(&b);
    printf("</tr>\n");
    }
  else
    {
    printf("\nConversion                           Dithering  ");
    gavl_benchmark_print_header(&b);
    printf("\n");
    }
  
  for(i = 0; i < num_sampleformats; i++)
    {
    in_format = sampleformats[i];
    ctx.in_format.sample_format = in_format;
    for(j = 0; j < num_sampleformats; j++)
      {
      out_format = sampleformats[j];
      if(in_format == out_format)
        continue;
      
      ctx.out_format.sample_format = out_format;

      if((gavl_bytes_per_sample(out_format) > 2) ||
         (in_format < GAVL_SAMPLE_FLOAT))
        {
        if(do_html)
          {
          printf("<td>%s -> %s</td><td>Not available</td>",
                 gavl_sample_format_to_string(in_format),
                 gavl_sample_format_to_string(out_format));
          }
        else
          printf("%-16s -> %-16s -          ",
                 gavl_sample_format_to_string(in_format),
                 gavl_sample_format_to_string(out_format));
        
        gavl_audio_options_set_dither_mode(ctx.opt, GAVL_AUDIO_DITHER_NONE);
        audio_convert_context_init(&ctx);
        gavl_benchmark_run(&b);
        audio_convert_context_cleanup(&ctx);
        gavl_benchmark_print_results(&b);

        if(do_html)
          {
          printf("</tr>");
          }
        printf("\n");
        fflush(stdout);
        }
      else
        {
        for(k = 0; k < num_dither_modes; k++)
          {
          if(do_html)
            {
            printf("<td>%s -> %s</td><td>%s</td>",
                   gavl_sample_format_to_string(in_format),
                   gavl_sample_format_to_string(out_format),
                   dither_modes[k].name);
            }
          else
            printf("%-16s -> %-16s %-10s ",
                   gavl_sample_format_to_string(in_format),
                   gavl_sample_format_to_string(out_format),
                   dither_modes[k].name);
           
          gavl_audio_options_set_dither_mode(ctx.opt, dither_modes[k].mode);
          audio_convert_context_init(&ctx);
          gavl_benchmark_run(&b);
          audio_convert_context_cleanup(&ctx);
          gavl_benchmark_print_results(&b);
          
          if(do_html)
            {
            printf("</tr>");
            }
          printf("\n");
          fflush(stdout);
          }
        }
      }
    }
  audio_convert_context_destroy(&ctx);

  if(do_html)
    printf("</table>\n");
  
  }

static void benchmark_mix()
  {
  int num_sampleformats;
  gavl_sample_format_t in_format;

  int i;

  audio_convert_context_t ctx;
  gavl_benchmark_t b;
  memset(&ctx, 0, sizeof(ctx));
  memset(&b, 0, sizeof(b));

  b.init = audio_convert_init;
  b.func = audio_convert_func;
  b.data = &ctx;
  
  ctx.in_format.num_channels = 6;
  ctx.in_format.samplerate = 48000;
  ctx.in_format.interleave_mode = GAVL_INTERLEAVE_NONE;
  ctx.in_format.channel_locations[0] = GAVL_CHID_NONE;
  ctx.in_format.samples_per_frame = 102400;

  gavl_set_channel_setup(&ctx.in_format);
  
  gavl_audio_format_copy(&ctx.out_format, &ctx.in_format);

  ctx.out_format.num_channels = 2;
  ctx.out_format.channel_locations[0] = GAVL_CHID_NONE;
  gavl_set_channel_setup(&ctx.out_format);

  num_sampleformats = sizeof(sampleformats)/sizeof(sampleformats[0]);
  audio_convert_context_create(&ctx);

  printf("Mixing of %d samples, from %d to %d channels\n",
         ctx.in_format.samples_per_frame,
         ctx.in_format.num_channels,
         ctx.out_format.num_channels);
  
  if(do_html)
    {
    printf("<p>\n<table border=\"1\" width=\"100%%\"><tr><td>Sampleformat</td>");
    gavl_benchmark_print_header(&b);
    printf("</tr>\n");
    }
  else
    {
    printf("\nSampleformat     ");
    gavl_benchmark_print_header(&b);
    printf("\n");
    }
  
  
  for(i = 0; i < num_sampleformats; i++)
    {
    in_format = sampleformats[i];
    ctx.in_format.sample_format = in_format;
    ctx.out_format.sample_format = in_format;
    if(do_html)
      {
      printf("<td>%s</td>", gavl_sample_format_to_string(in_format));
      }
    else
      printf("%-16s ", gavl_sample_format_to_string(in_format));
    audio_convert_context_init(&ctx);
    gavl_benchmark_run(&b);
    audio_convert_context_cleanup(&ctx);
    gavl_benchmark_print_results(&b);

    if(do_html)
      {
      printf("</tr>");
      }
    printf("\n");
    fflush(stdout);
    }
  
  audio_convert_context_destroy(&ctx);
  
  if(do_html)
    printf("</table>\n");
  
  }

static const struct
  {
  gavl_resample_mode_t mode;
  char * name;
  }
resample_modes[] =
  {
    { GAVL_RESAMPLE_ZOH,         "Zero order hold" },
    { GAVL_RESAMPLE_LINEAR,      "Linear" },
    { GAVL_RESAMPLE_SINC_FAST,   "Sinc fast" },
    { GAVL_RESAMPLE_SINC_MEDIUM, "Sinc medium" },
    { GAVL_RESAMPLE_SINC_BEST,   "Sinc best" } 
  };

static const gavl_sample_format_t resample_sampleformats[] =
  {
    // GAVL_SAMPLE_U8, /*!< Unsigned 8 bit */
    // GAVL_SAMPLE_S8, /*!< Signed 8 bit */
    // GAVL_SAMPLE_U16, /*!< Unsigned 16 bit */
    // GAVL_SAMPLE_S16, /*!< Signed 16 bit */
    // GAVL_SAMPLE_S32, /*!< Signed 32 bit */
    GAVL_SAMPLE_FLOAT,  /*!< Floating point (-1.0 .. 1.0) */
    GAVL_SAMPLE_DOUBLE  /*!< Double (-1.0 .. 1.0) */
  };

static void benchmark_resample()
  {
  int num_sampleformats;
  int num_resample_modes;
  gavl_sample_format_t in_format;

  int i, j;

  audio_convert_context_t ctx;
  gavl_benchmark_t b;
  memset(&ctx, 0, sizeof(ctx));
  memset(&b, 0, sizeof(b));

  b.init = audio_convert_init;
  b.func = audio_convert_func;
  b.data = &ctx;
  
  ctx.in_format.num_channels = 2;
  ctx.in_format.samplerate = 48000;
  ctx.in_format.interleave_mode = GAVL_INTERLEAVE_NONE;
  ctx.in_format.channel_locations[0] = GAVL_CHID_NONE;
  ctx.in_format.samples_per_frame = 48000;
  gavl_set_channel_setup(&ctx.in_format);
  
  gavl_audio_format_copy(&ctx.out_format, &ctx.in_format);
  
  ctx.out_format.samplerate = 44100;
  
  num_sampleformats =
    sizeof(resample_sampleformats)/sizeof(resample_sampleformats[0]);
  num_resample_modes =
    sizeof(resample_modes)/sizeof(resample_modes[0]);
  
  audio_convert_context_create(&ctx);

  printf("Resampling of %d samples (%d channels), from %d to %d\n",
         ctx.in_format.samples_per_frame,
         ctx.in_format.num_channels,
         ctx.in_format.samplerate,
         ctx.out_format.samplerate);

  if(do_html)
    {
    printf("<p>\n<table border=\"1\" width=\"100%%\"><tr><td>Sampleformat</td><td>Method</td>");
    gavl_benchmark_print_header(&b);
    printf("</tr>\n");
    }
  else
    {
    printf("\nSampleformat     Method          ");
    gavl_benchmark_print_header(&b);
    printf("\n");
    
    }
  
  for(i = 0; i < num_sampleformats; i++)
    {
    in_format = resample_sampleformats[i];
    ctx.in_format.sample_format = in_format;
    ctx.out_format.sample_format = in_format;

    for(j = 0; j < num_resample_modes; j++)
      {
      gavl_audio_options_set_resample_mode(ctx.opt, resample_modes[j].mode);
  
      if(do_html)
        {
        printf("<td>%s</td><td>%s</td>",
               gavl_sample_format_to_string(in_format),
               resample_modes[j].name);
        }
      else
        {
        printf("%-16s %-15s ",
               gavl_sample_format_to_string(in_format),
               resample_modes[j].name);
        
        }
      
      audio_convert_context_init(&ctx);
      gavl_benchmark_run(&b);
      audio_convert_context_cleanup(&ctx);
      gavl_benchmark_print_results(&b);
      
      if(do_html)
        printf("</tr>");
      printf("\n");
      fflush(stdout);
      }
    }
  
  audio_convert_context_destroy(&ctx);
  
  if(do_html)
    printf("</table>\n");
  
  }


/* Video converter */

typedef struct
  {
  gavl_video_format_t in_format;
  gavl_video_format_t out_format;
  
  gavl_video_converter_t * cnv;
  gavl_video_options_t * opt;
  
  gavl_video_frame_t * in_frame;
  gavl_video_frame_t * out_frame;
  
  } video_convert_context_t;

static void video_convert_context_create(video_convert_context_t * ctx)
  {
  ctx->cnv = gavl_video_converter_create();
  ctx->opt = gavl_video_converter_get_options(ctx->cnv);
  }

static int video_convert_context_init(video_convert_context_t * ctx)
  {
  int result;
  ctx->in_frame = gavl_video_frame_create(&ctx->in_format);
  ctx->out_frame = gavl_video_frame_create(&ctx->out_format);
  result = gavl_video_converter_init(ctx->cnv, &ctx->in_format, &ctx->out_format);
  //  fprintf(stderr, "Result: %d\n", result);
  return result <= 0 ? 0 : 1;
  }

static void video_convert_context_cleanup(video_convert_context_t * ctx)
  {
  gavl_video_frame_destroy(ctx->in_frame);
  gavl_video_frame_destroy(ctx->out_frame);
  }

static void video_convert_context_destroy(video_convert_context_t * ctx)
  {
  gavl_video_converter_destroy(ctx->cnv);
  }


static void video_convert_init(void * data)
  {
  
  video_convert_context_t * ctx = data;
  /* Fill frame with random numbers */
  init_video_frame(&ctx->in_format, ctx->in_frame);
  }

static void video_convert_func(void * data)
  {
  video_convert_context_t * ctx = data;
  gavl_video_convert(ctx->cnv, ctx->in_frame, ctx->out_frame);
  }

static const struct
  {
  gavl_scale_mode_t mode;
  const char * name;
  }
scale_modes[] =
  {
    { GAVL_SCALE_NEAREST, "Nearest" },
    { GAVL_SCALE_BILINEAR, "Linear"}, 
    { GAVL_SCALE_QUADRATIC, "Quadratic" },
    { GAVL_SCALE_CUBIC_BSPLINE, "Cubic B-Spline" },
    { GAVL_SCALE_CUBIC_CATMULL, "Cubic Catmull-Rom" },
    { GAVL_SCALE_CUBIC_MITCHELL, "Cubic Mitchell-Netravali" },
    { GAVL_SCALE_SINC_LANCZOS, "Sinc" },
  };

static void do_pixelformat(video_convert_context_t * ctx,
                           gavl_benchmark_t * b, gavl_pixelformat_t in_format,
                           gavl_pixelformat_t out_format, char * name)
  {
  int flags;
  int in_sub_h;
  int in_sub_v;
  int out_sub_h;
  int out_sub_v;
  int i;

  gavl_pixelformat_chroma_sub(in_format, &in_sub_h, &in_sub_v);
  gavl_pixelformat_chroma_sub(out_format, &out_sub_h, &out_sub_v);
  ctx->in_format.pixelformat = in_format;
  ctx->out_format.pixelformat = out_format;
  
  if((in_sub_h == out_sub_h) && (in_sub_v == out_sub_v))
    {
    flags = gavl_video_options_get_conversion_flags(ctx->opt);
    flags &= ~GAVL_RESAMPLE_CHROMA;
    gavl_video_options_set_conversion_flags(ctx->opt, flags);
    
    if(video_convert_context_init(ctx))
      {
      if(do_html)
        {
        printf("<tr><td>%s</td><td>Not needed</td>", name);
        }
      else
        printf("%-6s -                         ", name);
      
      gavl_benchmark_run(b);
      gavl_benchmark_print_results(b);
      if(do_html)
        printf("</tr>\n");
      else
        printf("\n");
      }
    video_convert_context_cleanup(ctx);
    }
  else
    {
    flags = gavl_video_options_get_conversion_flags(ctx->opt);
    flags &= ~GAVL_RESAMPLE_CHROMA;
    gavl_video_options_set_conversion_flags(ctx->opt, flags);
    
    if(video_convert_context_init(ctx))
      {
      if(do_html)
        {
        printf("<tr><td>%s<td>Off</td>", name);
        }
      else
        printf( "%-6s Off                       ", name);
      gavl_benchmark_run(b);
      gavl_benchmark_print_results(b);
      
      if(do_html)
        printf("</tr>\n");
      else
        printf("\n");
      }
    video_convert_context_cleanup(ctx);

    flags = gavl_video_options_get_conversion_flags(ctx->opt);
    flags |= GAVL_RESAMPLE_CHROMA;
    gavl_video_options_set_conversion_flags(ctx->opt, flags);

    for(i = 1; i < sizeof(scale_modes)/sizeof(scale_modes[0]); i++)
      {
      gavl_video_options_set_scale_mode(ctx->opt, scale_modes[i].mode);
      
      if(video_convert_context_init(ctx))
        {
        if(do_html)
          {
          printf("<tr><td>%s</td><td>%s</td>", name, scale_modes[i].name);
          }
        else
          printf("%-6s %-24s  ", name, scale_modes[i].name);
        gavl_benchmark_run(b);
        gavl_benchmark_print_results(b);
        if(do_html)
          printf("</tr>\n");
        else
          printf("\n");
        }
      video_convert_context_cleanup(ctx);
      }
    }
  }

static void benchmark_pixelformat()
  {
  int num_pixelformats;
  gavl_pixelformat_t in_format;
  gavl_pixelformat_t out_format;
  int i, j;

  video_convert_context_t ctx;
  gavl_benchmark_t b;
  
  memset(&ctx, 0, sizeof(ctx));
  memset(&b, 0, sizeof(b));

  b.init = video_convert_init;
  b.func = video_convert_func;
  b.data = &ctx;
  
  ctx.in_format.image_width = 720;
  ctx.in_format.image_height = 576;

  ctx.in_format.frame_width = 720;
  ctx.in_format.frame_height = 576;
  ctx.in_format.pixel_width = 1;
  ctx.in_format.pixel_height = 1;
  
  gavl_video_format_copy(&ctx.out_format, &ctx.in_format);
  printf("Image size: %d x %d\n",
         ctx.in_format.image_width,
         ctx.in_format.image_height);

  if(do_html)
    {
    printf("<p>\n<table border=\"1\" width=\"100%%\"><tr><td>Flavour</td><td>Chroma resampling</td>");
    gavl_benchmark_print_header(&b);
    printf("</tr>\n");
    }
  else
    {
    printf("\nFlavour Chroma resampling        ");
    gavl_benchmark_print_header(&b);
    printf("\n");
    
    }
  
  num_pixelformats = gavl_num_pixelformats();
  
  video_convert_context_create(&ctx);
  /* Disable autoselection */
  gavl_video_options_set_quality(ctx.opt, 0);

  /* Must set this to prevent a crash (not in gavl but due to
   *  the benchmarking logic)
   */
  gavl_video_options_set_alpha_mode(ctx.opt, GAVL_ALPHA_BLEND_COLOR);

  for(i = 0; i < num_pixelformats; i++)
    {
    in_format = gavl_get_pixelformat(i);

    if((opt_pfmt1 != GAVL_PIXELFORMAT_NONE) && (opt_pfmt1 != in_format))
      continue;
    
    for(j = 0; j < num_pixelformats; j++)
      {
      out_format = gavl_get_pixelformat(j);

      if((opt_pfmt2 != GAVL_PIXELFORMAT_NONE) && (opt_pfmt2 != out_format))
        continue;
      

      if(in_format == out_format)
        continue;

      if(do_html)
        printf("<tr><td colspan=\"6\"><b>");
      printf("%s -> %s", gavl_pixelformat_to_string(in_format),
             gavl_pixelformat_to_string(out_format));
      if(do_html)
        printf("<b></td></tr>");
      printf("\n");
      
      /* C-Version */
      
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_C);
      do_pixelformat(&ctx, &b, in_format, out_format, "C");
      fflush(stdout);
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_MMX);
      do_pixelformat(&ctx, &b, in_format, out_format, "MMX");

      fflush(stdout);
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_MMXEXT);
      do_pixelformat(&ctx, &b, in_format, out_format, "MMXEXT");

      fflush(stdout);
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_C_HQ);
      do_pixelformat(&ctx, &b, in_format, out_format, "HQ");
      fflush(stdout);

      fflush(stdout);
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_SSE);
      do_pixelformat(&ctx, &b, in_format, out_format, "SSE");
      fflush(stdout);

      fflush(stdout);
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_SSE3);
      do_pixelformat(&ctx, &b, in_format, out_format, "SSE3");
      fflush(stdout);
      
      }
    }
  
  if(do_html)
    printf("</table>\n");

  video_convert_context_destroy(&ctx);
  
  }


static void do_scale(video_convert_context_t * ctx,
                     gavl_benchmark_t * b, gavl_pixelformat_t in_format,
                     const char * name, const char * dirs)
  {
  int num_scale_modes, i;
  num_scale_modes = sizeof(scale_modes)/sizeof(scale_modes[0]);

  ctx->in_format.pixelformat = in_format;
  ctx->out_format.pixelformat = in_format;

  for(i = 0; i < num_scale_modes; i++)
    {
    gavl_video_options_set_scale_mode(ctx->opt, scale_modes[i].mode);

    if(video_convert_context_init(ctx))
      {
      if(do_html)
        printf("<tr><td>%s</td><td>%s</td><td>%s</td>", dirs, name, scale_modes[i].name);
      else
        printf("%-2s        %-6s  %-24s ", dirs, name, scale_modes[i].name);
      
      gavl_benchmark_run(b);
      gavl_benchmark_print_results(b);
      if(do_html)
        printf("</tr>");

      printf("\n");
      
      }
    
    fflush(stdout);
    video_convert_context_cleanup(ctx);
    }
  }

static void do_scale_direction(video_convert_context_t * ctx,
                               gavl_benchmark_t * b)
  {
  int num_pixelformats;
  gavl_pixelformat_t in_format;
  int i;

  const char * dir;

  printf("\nDestination size: 1024 x 1024, source size 512 for each scaled direction\n");
  
  if(do_html)
    {
    printf("<p>\n<table border=\"1\" width=\"100%%\"><tr><td>Direction</td><td>Flavour</td><td>Scale method</td>");
    gavl_benchmark_print_header(b);
    printf("</tr>\n");
    }
  else
    {
    printf("Direction Flavour Scale method             ");
    gavl_benchmark_print_header(b);
    printf("\n");
    }
  
  num_pixelformats = gavl_num_pixelformats();
  

  for(i = 0; i < num_pixelformats; i++)
  //  for(i = 0; i < 3; i++)
    {
    in_format = gavl_get_pixelformat(i);
    if((opt_pfmt1 != GAVL_PIXELFORMAT_NONE) &&
       (opt_pfmt1 != in_format))
      continue;
    
    if(do_html)
      printf("<tr><td colspan=\"7\"><b>%s<b></td></tr>\n",
             gavl_pixelformat_to_string(in_format));
    else
      printf("%s\n", gavl_pixelformat_to_string(in_format));
    
    ctx->out_format.image_width =  1024;
    ctx->out_format.image_height = 1024;
    ctx->out_format.frame_width =  1024;
    ctx->out_format.frame_height = 1024;
    
    ctx->out_format.pixel_width = 1;
    ctx->out_format.pixel_height = 1;

    /* X */

    if(!opt_scaledir || (opt_scaledir == SCALE_X))
      {
      dir = "x";
    
      gavl_video_format_copy(&ctx->in_format, &ctx->out_format);
    
      ctx->in_format.image_width =   512;
      ctx->in_format.frame_width =   512;
    
      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_C);
      do_scale(ctx, b, in_format, "C", dir);
    
      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_MMX);
      do_scale(ctx, b, in_format, "MMX", dir);

      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_MMXEXT);
      do_scale(ctx, b, in_format, "MMXEXT", dir);

      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_SSE);
      do_scale(ctx, b, in_format, "SSE", dir);

      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_SSE2);
      do_scale(ctx, b, in_format, "SSE2", dir);

      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_SSE3);
      do_scale(ctx, b, in_format, "SSE3", dir);
    
      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_C_HQ);
      do_scale(ctx, b, in_format, "HQ", dir);
      }
    

    /* Y */
    if(!opt_scaledir || (opt_scaledir == SCALE_Y))
      {

      dir = "y";
    
      gavl_video_format_copy(&ctx->in_format, &ctx->out_format);
    
      ctx->in_format.image_height = 512;
      ctx->in_format.frame_height = 512;
    
      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_C);
      do_scale(ctx, b, in_format, "C", dir);
    
      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_MMX);
      do_scale(ctx, b, in_format, "MMX", dir);

      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_MMXEXT);
      do_scale(ctx, b, in_format, "MMXEXT", dir);

      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_SSE);
      do_scale(ctx, b, in_format, "SSE", dir);

      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_SSE2);
      do_scale(ctx, b, in_format, "SSE2", dir);

      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_SSE3);
      do_scale(ctx, b, in_format, "SSE3", dir);
    
      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_C_HQ);
      do_scale(ctx, b, in_format, "HQ", dir);
      }
    /* X+Y */

    if(!opt_scaledir || (opt_scaledir == (SCALE_X|SCALE_Y)))
      {
      //      fprintf(stderr, "%d\n", opt_scaledir);
      dir = "x+y";
      gavl_video_format_copy(&ctx->in_format, &ctx->out_format);
    
      ctx->in_format.image_width =  512;
      ctx->in_format.frame_width =  512;
      ctx->in_format.image_height = 512;
      ctx->in_format.frame_height = 512;
    
      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_C);
      do_scale(ctx, b, in_format, "C", dir);
    
      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_MMX);
      do_scale(ctx, b, in_format, "MMX", dir);

      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_MMXEXT);
      do_scale(ctx, b, in_format, "MMXEXT", dir);

      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_SSE);
      do_scale(ctx, b, in_format, "SSE", dir);

      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_SSE2);
      do_scale(ctx, b, in_format, "SSE2", dir);

      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_SSE3);
      do_scale(ctx, b, in_format, "SSE3", dir);
    
      gavl_video_options_set_accel_flags(ctx->opt, GAVL_ACCEL_C_HQ);
      do_scale(ctx, b, in_format, "HQ", dir);
      }
    }
  if(do_html)
    printf("</table>\n");
  
  }

static void benchmark_scale()
  {
  video_convert_context_t ctx;
  gavl_benchmark_t b;
  
  memset(&ctx, 0, sizeof(ctx));
  memset(&b, 0, sizeof(b));

  b.init = video_convert_init;
  b.func = video_convert_func;
  b.data = &ctx;

  video_convert_context_create(&ctx);

  /* Disable autoselection */
  gavl_video_options_set_quality(ctx.opt, 0);
  
  do_scale_direction(&ctx, &b);
  
  video_convert_context_destroy(&ctx);
  }

static const struct
  {
  const char * name;
  gavl_deinterlace_mode_t mode;
  }
deinterlace_modes[] =
  {
    { "Scanline doubler", GAVL_DEINTERLACE_COPY },
    { "Upscale",          GAVL_DEINTERLACE_SCALE },
    { "Blend",            GAVL_DEINTERLACE_BLEND },
  };

static void benchmark_deinterlace()
  {
  video_convert_context_t ctx;
  gavl_benchmark_t b;
  int num_pixelformats;
  int num_modes;
  int num_scale_modes;
  int i, j;
  memset(&ctx, 0, sizeof(ctx));
  memset(&b, 0, sizeof(b));
  
  b.init = video_convert_init;
  b.func = video_convert_func;
  b.data = &ctx;

  ctx.in_format.image_width  = 720;
  ctx.in_format.image_height = 576;
  ctx.in_format.frame_width  = 720;
  ctx.in_format.frame_height = 576;
  ctx.in_format.pixel_width  = 1;
  ctx.in_format.pixel_height = 1;
  ctx.in_format.interlace_mode = GAVL_INTERLACE_TOP_FIRST;

  printf("Size: %d x %d\n",
         ctx.in_format.image_width,
         ctx.in_format.image_height);
  
  if(do_html)
    {
    printf("<p><table border=\"1\" width=\"100%%\"><tr><td>Method</td><td>Flavour</td><td>Scale method</td>");
    gavl_benchmark_print_header(&b);
    printf("</tr>\n");
    }
  else
    {
    printf("Method           Flavour Scale method             ");
    gavl_benchmark_print_header(&b);
    printf("\n");
    }
  
  gavl_video_format_copy(&ctx.out_format, &ctx.in_format);
  ctx.out_format.interlace_mode = GAVL_INTERLACE_NONE;
  
  video_convert_context_create(&ctx);

  /* Disable autoselection */
  gavl_video_options_set_quality(ctx.opt, 0);

  num_pixelformats = gavl_num_pixelformats();
  num_modes = sizeof(deinterlace_modes)/sizeof(deinterlace_modes[0]);
  num_scale_modes = sizeof(scale_modes)/sizeof(scale_modes[0]);
  
  for(i = 0; i < num_pixelformats; i++)
    {
    ctx.in_format.pixelformat = gavl_get_pixelformat(i);
    ctx.out_format.pixelformat = ctx.in_format.pixelformat;

    if(do_html)
      printf("<tr><td colspan=\"7\"><b>%s<b></td></tr>\n",
             gavl_pixelformat_to_string(ctx.in_format.pixelformat));
    else
      printf("%s\n",
             gavl_pixelformat_to_string(ctx.in_format.pixelformat));
    
    for(j = 0; j < num_modes; j++)
      {
      if(deinterlace_modes[j].mode == GAVL_DEINTERLACE_SCALE)
        continue;
      
      gavl_video_options_set_deinterlace_mode(ctx.opt, deinterlace_modes[j].mode);

      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_C);

      if(video_convert_context_init(&ctx))
        {
        if(do_html)
          printf("<tr><td>%s</td><td>C</td><td></td>",
                 deinterlace_modes[j].name);
        else
          printf("%-16s C       -                        ",
                 deinterlace_modes[j].name);
        
        gavl_benchmark_run(&b);
        gavl_benchmark_print_results(&b);
        if(do_html)
          printf("</tr>");
        printf("\n");
        }
      fflush(stdout);
      video_convert_context_cleanup(&ctx);

      if(deinterlace_modes[j].mode == GAVL_DEINTERLACE_COPY)
        continue;
      
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_MMX);

      if(video_convert_context_init(&ctx))
        {
        if(do_html)
          printf("<tr><td>%s</td><td>MMX</td><td></td>",
                 deinterlace_modes[j].name);
        else
          printf("%-16s MMX     -                        ",
                 deinterlace_modes[j].name);
        
        gavl_benchmark_run(&b);
        gavl_benchmark_print_results(&b);
        if(do_html)
          printf("</tr>");
        printf("\n");
        }
      fflush(stdout);
      video_convert_context_cleanup(&ctx);


      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_MMXEXT);

      if(video_convert_context_init(&ctx))
        {
        if(do_html)
          printf("<tr><td>%s</td><td>MMXEXT</td><td></td>",
                 deinterlace_modes[j].name);
        else
          printf("%-16s MMXEXT  -                        ",
                 deinterlace_modes[j].name);
        
        gavl_benchmark_run(&b);
        gavl_benchmark_print_results(&b);
        if(do_html)
          printf("</tr>");
        printf("\n");
        }
      fflush(stdout);
      video_convert_context_cleanup(&ctx);
      }
    
    gavl_video_options_set_deinterlace_mode(ctx.opt, GAVL_DEINTERLACE_SCALE);
    for(j = 1; j < num_scale_modes; j++)
      {
      gavl_video_options_set_scale_mode(ctx.opt, scale_modes[j].mode);

      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_C);

      if(video_convert_context_init(&ctx))
        {
        if(do_html)
          printf("<tr><td>Upscale</td><td>C</td><td>%s</td>",
                 scale_modes[j].name);
        else
          printf("Upscale          C       %-24s ", scale_modes[j].name);
        
        gavl_benchmark_run(&b);
        gavl_benchmark_print_results(&b);
        if(do_html)
          printf("</tr>");
        printf("\n");
        }
      fflush(stdout);
      video_convert_context_cleanup(&ctx);

      if(deinterlace_modes[j].mode == GAVL_DEINTERLACE_COPY)
        continue;
      
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_MMX);

      if(video_convert_context_init(&ctx))
        {
        if(do_html)
          printf("<tr><td>Upscale</td><td>MMX</td><td>%s</td>",
                 scale_modes[j].name);
        else
          printf("Upscale          MMX     %-24s ", scale_modes[j].name);

        
        gavl_benchmark_run(&b);
        gavl_benchmark_print_results(&b);
        if(do_html)
          printf("</tr>");
        printf("\n");
        }
      fflush(stdout);
      video_convert_context_cleanup(&ctx);
      }
    }
  if(do_html)
    printf("</table>\n");
  
  video_convert_context_destroy(&ctx);
  }

/* Video DSP */

typedef struct
  {
  gavl_video_format_t format;
  
  gavl_dsp_context_t * ctx;
  
  gavl_video_frame_t * frames[3];
  int num_frames;
  
  } video_dsp_context_t;

static void video_dsp_context_create(video_dsp_context_t * ctx, int num_frames)
  {
  ctx->ctx = gavl_dsp_context_create();
  ctx->num_frames = num_frames;
  }

static void video_dsp_context_init(video_dsp_context_t * ctx)
  {
  int i;

  for(i = 0; i < ctx->num_frames; i++)
    ctx->frames[i] = gavl_video_frame_create(&ctx->format);
  return;
  }

static void video_dsp_context_cleanup(video_dsp_context_t * ctx)
  {
  int i;
  for(i = 0; i < ctx->num_frames; i++)
    gavl_video_frame_destroy(ctx->frames[i]);
  }

static void video_dsp_context_destroy(video_dsp_context_t * ctx)
  {
  gavl_dsp_context_destroy(ctx->ctx);
  }

static void video_dsp_init(void * data)
  {
  int i;
  video_dsp_context_t * ctx = data;
  /* Fill frame with random numbers */
  for(i = 0; i < ctx->num_frames; i++)
    {
    init_video_frame(&ctx->format, ctx->frames[0]);
    }
  }

static void interpolate_func(void * data)
  {
  video_dsp_context_t * ctx = data;

  gavl_dsp_interpolate_video_frame(ctx->ctx,
                                   &ctx->format,
                                   ctx->frames[0],
                                   ctx->frames[1],
                                   ctx->frames[2],
                                   0.5);
  }

/* Video interpolation */

static void benchmark_dsp_interpolate()
  {
  video_dsp_context_t ctx;
  gavl_benchmark_t b;
  int num_pixelformats;
  int i;
  memset(&ctx, 0, sizeof(ctx));
  memset(&b, 0, sizeof(b));
  
  b.init = video_dsp_init;
  b.func = interpolate_func;
  b.data = &ctx;

  ctx.format.image_width  = 720;
  ctx.format.image_height = 576;
  ctx.format.frame_width  = 720;
  ctx.format.frame_height = 576;
  ctx.format.pixel_width  = 1;
  ctx.format.pixel_height = 1;

  printf("Size: %d x %d\n",
         ctx.format.image_width,
         ctx.format.image_height);
  
  if(do_html)
    {
    printf("<p><table border=\"1\" width=\"100%%\"><tr><td>Format</td><td>Flavour</td>");
    gavl_benchmark_print_header(&b);
    printf("</tr>\n");
    }
  else
    {
    printf("Format                  Flavour ");
    gavl_benchmark_print_header(&b);
    printf("\n");
    }
  
  video_dsp_context_create(&ctx, 3);

  /* Disable autoselection */
  gavl_dsp_context_set_quality(ctx.ctx, 0);

  num_pixelformats = gavl_num_pixelformats();
  
  for(i = 0; i < num_pixelformats; i++)
    {
    ctx.format.pixelformat = gavl_get_pixelformat(i);

    /* C */

    gavl_dsp_context_set_accel_flags(ctx.ctx, GAVL_ACCEL_C);
    
    video_dsp_context_init(&ctx);

    if(gavl_dsp_interpolate_video_frame(ctx.ctx,
                                        &ctx.format,
                                        ctx.frames[0],
                                        ctx.frames[1],
                                        ctx.frames[2],
                                        0.5))
      {
      if(do_html)
        printf("<tr><td>%s</td><td>C</td>",
               gavl_pixelformat_to_string(ctx.format.pixelformat));
      else
        printf("%-23s C       ",
               gavl_pixelformat_to_string(ctx.format.pixelformat));
      
      gavl_benchmark_run(&b);
      gavl_benchmark_print_results(&b);
      if(do_html)
        printf("</tr>");
      printf("\n");
      fflush(stdout);
      }
    video_dsp_context_cleanup(&ctx);

    /* MMX */

    gavl_dsp_context_set_accel_flags(ctx.ctx, GAVL_ACCEL_MMX);
    
    video_dsp_context_init(&ctx);

    if(gavl_dsp_interpolate_video_frame(ctx.ctx,
                                        &ctx.format,
                                        ctx.frames[0],
                                        ctx.frames[1],
                                        ctx.frames[2],
                                        0.5))
      {
      if(do_html)
        printf("<tr><td>%s</td><td>MMX</td>",
               gavl_pixelformat_to_string(ctx.format.pixelformat));
      else
        printf("%-23s MMX     ",
               gavl_pixelformat_to_string(ctx.format.pixelformat));
      
      gavl_benchmark_run(&b);
      gavl_benchmark_print_results(&b);
      if(do_html)
        printf("</tr>");
      printf("\n");
      fflush(stdout);
      }
    video_dsp_context_cleanup(&ctx);

    }
  
  video_dsp_context_destroy(&ctx);
  if(do_html)
    printf("</table>\n");
  
  }

/* Image transformation */

static const struct
  {
  gavl_scale_mode_t mode;
  const char * name;
  }
transform_modes[] =
  {
    { GAVL_SCALE_NEAREST, "Nearest" },
    { GAVL_SCALE_BILINEAR, "Linear"}, 
    { GAVL_SCALE_QUADRATIC, "Quadratic" },
    { GAVL_SCALE_CUBIC_BSPLINE, "Cubic B-Spline" },
  };

static const gavl_pixelformat_t transform_pixelformats[] =
  {
    GAVL_GRAY_8,
    GAVL_GRAY_16,
    GAVL_GRAY_FLOAT,
    GAVL_GRAYA_16,
    GAVL_GRAYA_32,
    GAVL_GRAYA_FLOAT,
    GAVL_RGB_15,
    GAVL_RGB_16,
    GAVL_RGB_24,
    GAVL_RGB_32,
    GAVL_RGBA_32,
    GAVL_RGB_48,
    GAVL_RGBA_64,
    GAVL_RGB_FLOAT,
    GAVL_RGBA_FLOAT,
    GAVL_YUY2,
    GAVL_UYVY,
    GAVL_YUV_420_P,
    GAVL_YUV_422_P,
    GAVL_YUV_444_P,
    GAVL_YUV_411_P,
    GAVL_YUV_410_P,
    GAVL_YUV_444_P_16,
    GAVL_YUV_422_P_16,
    GAVL_PIXELFORMAT_NONE,
  };

typedef struct
  {
  gavl_video_format_t format;
  
  gavl_image_transform_t * transform;
  gavl_video_options_t * opt;
  
  gavl_video_frame_t * in_frame;
  gavl_video_frame_t * out_frame;
  } image_transform_context_t;

static void image_transform_context_create(image_transform_context_t * ctx)
  {
  ctx->transform = gavl_image_transform_create();
  ctx->opt = gavl_image_transform_get_options(ctx->transform);
  }

static void transform_func(void *priv, double xdst,
                           double ydst, double *xsrc, double *ysrc)
  {
  *xsrc = xdst + 0.5;
  *ysrc = ydst + 0.5;
  }

static int image_transform_context_init(image_transform_context_t * ctx)
  {
  ctx->in_frame = gavl_video_frame_create(&ctx->format);
  ctx->out_frame = gavl_video_frame_create(&ctx->format);

  return gavl_image_transform_init(ctx->transform, &ctx->format, transform_func, NULL);
  }

static void image_transform_context_cleanup(image_transform_context_t * ctx)
  {
  gavl_video_frame_destroy(ctx->in_frame);
  gavl_video_frame_destroy(ctx->out_frame);
  }

static void image_transform_context_destroy(image_transform_context_t * ctx)
  {
  gavl_image_transform_destroy(ctx->transform);
  }

static void image_transform_init(void * data)
  {
  image_transform_context_t * ctx = data;
  /* Fill frame with random numbers */
  init_video_frame(&ctx->format, ctx->in_frame);
  }

static void image_transform_func(void * data)
  {
  image_transform_context_t * ctx = data;

  gavl_image_transform_transform(ctx->transform,
                                 ctx->in_frame,
                                 ctx->out_frame);
  }


static void benchmark_image_transform()
  {
  image_transform_context_t ctx;
  gavl_benchmark_t b;
  int i, j;
  memset(&ctx, 0, sizeof(ctx));
  memset(&b, 0, sizeof(b));
  
  b.init = image_transform_init;
  b.func = image_transform_func;
  b.data = &ctx;

  ctx.format.image_width  = 720;
  ctx.format.image_height = 576;
  ctx.format.frame_width  = 720;
  ctx.format.frame_height = 576;
  ctx.format.pixel_width  = 1;
  ctx.format.pixel_height = 1;

  printf("Size: %d x %d\n",
         ctx.format.image_width,
         ctx.format.image_height);
  
  if(do_html)
    {
    printf("<p><table border=\"1\" width=\"100%%\"><tr><td>Format</td><td>Flavour</td>");
    gavl_benchmark_print_header(&b);
    printf("</tr>\n");
    }
  else
    {
    printf("Format                  Mode           Flavour ");
    gavl_benchmark_print_header(&b);
    printf("\n");
    }
  
  image_transform_context_create(&ctx);
  
  /* Disable autoselection */
  gavl_video_options_set_quality(ctx.opt, 0);

  for(j = 0; j < sizeof(transform_modes)/sizeof(transform_modes[0]); j++)
    {
    gavl_video_options_set_scale_mode(ctx.opt, transform_modes[j].mode);

    i = 0;
    
    while(transform_pixelformats[i] != GAVL_PIXELFORMAT_NONE)
      {
      ctx.format.pixelformat = transform_pixelformats[i];
      
      /* C */

      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_C);

      image_transform_context_init(&ctx);
      
      if(do_html)
        printf("<tr><td>%s</td><td>%s</td><td>C</td>",
               gavl_pixelformat_to_string(ctx.format.pixelformat), transform_modes[j].name);
      else
        printf("%-23s %-14s C       ",
               gavl_pixelformat_to_string(ctx.format.pixelformat), transform_modes[j].name);
    
      gavl_benchmark_run(&b);
      gavl_benchmark_print_results(&b);
      if(do_html)
        printf("</tr>");
      printf("\n");
      fflush(stdout);
      image_transform_context_cleanup(&ctx);

      /* MMX */
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_MMX);
      
      if(image_transform_context_init(&ctx))
        {
        if(do_html)
          printf("<tr><td>%s</td><td>%s</td><td>MMX</td>",
                 gavl_pixelformat_to_string(ctx.format.pixelformat), transform_modes[j].name);
        else
          printf("%-23s %-14s MMX     ",
                 gavl_pixelformat_to_string(ctx.format.pixelformat), transform_modes[j].name);
        
        gavl_benchmark_run(&b);
        gavl_benchmark_print_results(&b);
        if(do_html)
          printf("</tr>");
        printf("\n");
        fflush(stdout);
        image_transform_context_cleanup(&ctx);
        }

      /* MMXEXT */
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_MMXEXT);
      
      if(image_transform_context_init(&ctx))
        {
        if(do_html)
          printf("<tr><td>%s</td><td>%s</td><td>MMXEXT</td>",
                 gavl_pixelformat_to_string(ctx.format.pixelformat), transform_modes[j].name);
        else
          printf("%-23s %-14s MMXEXT  ",
                 gavl_pixelformat_to_string(ctx.format.pixelformat), transform_modes[j].name);
        
        gavl_benchmark_run(&b);
        gavl_benchmark_print_results(&b);
        if(do_html)
          printf("</tr>");
        printf("\n");
        fflush(stdout);
        image_transform_context_cleanup(&ctx);
        }

      //      else
      //        fprintf(stderr, "No MMX version\n");
      
      i++;
      }
    }
  
  image_transform_context_destroy(&ctx);
  if(do_html)
    printf("</table>\n");
  
  }



#define BENCHMARK_SAMPLEFORMAT (1<<0)
#define BENCHMARK_MIX          (1<<1)
#define BENCHMARK_VOLUME       (1<<2)
#define BENCHMARK_PEAK_DETECT  (1<<3)
#define BENCHMARK_INTERLEAVE   (1<<4)
#define BENCHMARK_RESAMPLE     (1<<5)

#define BENCHMARK_PIXELFORMAT  (1<<6)
#define BENCHMARK_SCALE        (1<<7)
#define BENCHMARK_DEINTERLACE  (1<<8)
#define BENCHMARK_INTERPOLATE  (1<<9)
#define BENCHMARK_SAD          (1<<10)
#define BENCHMARK_TRANSFORM    (1<<11)

static const struct
  {
  const char * option;
  const char * help;
  int flag;
  }
benchmark_flags[] =
  {
    { "-sfmt", "Sampleformat conversions", BENCHMARK_SAMPLEFORMAT },
    { "-mix", "Audio mixing",              BENCHMARK_MIX },
    { "-rs", "Resample routines",          BENCHMARK_RESAMPLE},
    //    { "-vol", "Volume control",            BENCHMARK_VOLUME},
    //    { "-pd", "Peak detection",             BENCHMARK_PEAK_DETECT},
    //    { "-il", "Interleaving conversion",    BENCHMARK_INTERLEAVE},
    { "-pfmt", "Pixelformat conversions",  BENCHMARK_PIXELFORMAT },
    { "-scale", "Scale",                   BENCHMARK_SCALE},
    { "-deint", "Deinterlacing",           BENCHMARK_DEINTERLACE},
    { "-ip", "Video frame interpolation",  BENCHMARK_INTERPOLATE},
    { "-it", "Image transformation",  BENCHMARK_TRANSFORM},
    //    { "-sad", "SAD routines",              BENCHMARK_SAD},
  };

static int get_flag(char * opt, int * flag_ret)
  {
  int i;
  for(i = 0; i < sizeof(benchmark_flags)/sizeof(benchmark_flags[0]); i++)
    {
    if(!strcmp(benchmark_flags[i].option, opt))
      {
      *flag_ret = benchmark_flags[i].flag;
      return 1;
      }
    }
  return 0;
  }

static void print_header(const char * title)
  {
  if(do_html)
    printf("<h2>%s</h2>\n", title);
  else
    printf("\n%s\n\n", title);
  }

static void print_help()
  {
  int i;
  printf("Usage: benchmark [-scaledir <dir>] [-pfmt1 <pfmt>] [-pfmt2 <pfmt>] [function ...] [-html]\n");
  printf("       benchmark -help\n\n");
  printf("       benchmark -listpfmt\n\n");
  printf("-html\n  Produce html output\n");
  printf("-help\n  Print this help and exit\n\n");
  printf("-listpfmt\n  List pixelformats and exit\n\n");
  printf("Function can be any combination of the following\n\n");

  for(i = 0; i < sizeof(benchmark_flags)/sizeof(benchmark_flags[0]); i++)
    {
    printf("%s\n  %s\n", benchmark_flags[i].option, benchmark_flags[i].help);
    }
  printf("-a\n  Do all of the above\n");
  }

static void list_pfmts()
  {
  int i, num;
  num = gavl_num_pixelformats();
  
  for(i = 0; i < num; i++)
    {
    printf("%s\n", gavl_pixelformat_to_string(gavl_get_pixelformat(i)));
    }
  }

int main(int argc, char ** argv)
  {
  int i;
  int flags = 0;
  int flag;


#ifdef HAVE_SCHED_SETAFFINITY
  /* Force ourselves into processor 0 */
  cpu_set_t cpuset;

  CPU_ZERO(&cpuset);
  CPU_SET(0, &cpuset);
  if(!sched_setaffinity(0, sizeof(cpuset), &cpuset))
    fprintf(stderr, "Running on processor 0 exclusively\n");
  else
    fprintf(stderr, "sched_setaffinity failed: %s\n", strerror(errno));
#endif
  
  i = 1;
  
  while(i < argc)
    {
    if(get_flag(argv[i], &flag))
      flags |= flag;
    else if(!strcmp(argv[i], "-a"))
      flags = ~0x0;
    else if(!strcmp(argv[i], "-html"))
      do_html = 1;
    else if(!strcmp(argv[i], "-pfmt1"))
      {
      opt_pfmt1 = gavl_string_to_pixelformat(argv[i+1]);
      i++;
      }
    else if(!strcmp(argv[i], "-scaledir"))
      {
      if(!strcmp(argv[i+1], "x"))
        opt_scaledir = SCALE_X;
      else if(!strcmp(argv[i+1], "y"))
        opt_scaledir = SCALE_Y;
      else if(!strcmp(argv[i+1], "xy"))
        opt_scaledir = SCALE_X | SCALE_Y;
      i++;
      }
    else if(!strcmp(argv[i], "-pfmt2"))
      {
      opt_pfmt2 = gavl_string_to_pixelformat(argv[i+1]);
      i++;
      }
    else if(!strcmp(argv[i], "-help"))
      {
      print_help();
      return 0;
      }
    else if(!strcmp(argv[i], "-listpfmt"))
      {
      list_pfmts();
      return 0;
      }
    i++;
    }

  if(do_html)
    {
    printf("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\n");
    printf("<html>\n");
    printf("<head>\n");
    printf("<title>gavl benchmarks</title>\n");
    printf("<link rel=\"stylesheet\" href=\"css/style.css\">\n");
    printf("</head>\n");
    printf("<body>\n");
    printf("<h1>gavl Benchmarks</h1>");
    printf("These benchmarks are generated with the benchmark tool in the src/ directory of gavl.<br>");
    printf("Number of init runs: %d, number of counted runs: %d<br>\n", INIT_RUNS, NUM_RUNS);
    printf("Times are %s<br>\n", gavl_benchmark_get_desc(gavl_accel_supported()));
    printf("<h2>/proc/cpuinfo</h2>\n");
    printf("<pre>\n");
    fflush(stdout);
    system("cat /proc/cpuinfo");
    printf("</pre>\n");
    
    if(flags & BENCHMARK_SAMPLEFORMAT)
      printf("<a href=\"#sfmt\">Sampleformat conversions</a><br>\n");

    if(flags & BENCHMARK_MIX)
      printf("<a href=\"#mix\">Mixing routines</a><br>\n");
    if(flags & BENCHMARK_RESAMPLE)
      printf("<a href=\"#rs\">Resampling routines</a><br>\n");
    if(flags & BENCHMARK_PIXELFORMAT)
      printf("<a href=\"#pfmt\">Pixelformat conversions</a><br>\n");
    if(flags & BENCHMARK_SCALE)
      printf("<a href=\"#scale\">Scaling routines</a><br>\n");
    if(flags & BENCHMARK_DEINTERLACE)
      printf("<a href=\"#deint\">Deinterlacing routines</a><br>\n");
    if(flags & BENCHMARK_INTERPOLATE)
      printf("<a href=\"#ip\">Video frame interpolation</a><br>\n");
    if(flags & BENCHMARK_TRANSFORM)
      printf("<a href=\"#it\">Video image transformation</a><br>\n");
    }
  else
    printf("Times are %s\n", gavl_benchmark_get_desc(gavl_accel_supported()));
  
  
  if(flags & BENCHMARK_SAMPLEFORMAT)
    {
    if(do_html)
      {
      printf("<a name=\"sfmt\"></a>");
      }
    print_header("Sampleformat conversions");
    benchmark_sampleformat();
    }
  
  if(flags & BENCHMARK_MIX)
    {
    if(do_html)
      {
      printf("<a name=\"mix\"></a>");
      }
    print_header("Mixing routines");
    benchmark_mix();
    }
  if(flags & BENCHMARK_RESAMPLE)
    {
    if(do_html)
      {
      printf("<a name=\"rs\"></a>");
      }
    print_header("Resampling routines");
    benchmark_resample();
    }
  if(flags & BENCHMARK_PIXELFORMAT)
    {
    if(do_html)
      {
      printf("<a name=\"pfmt\"></a>");
      }
    print_header("Pixelformat conversions");
    benchmark_pixelformat();
    }
  if(flags & BENCHMARK_SCALE)
    {
    if(do_html)
      {
      printf("<a name=\"scale\"></a>");
      }
    print_header("Scaling routines");
    benchmark_scale();
    }
  if(flags & BENCHMARK_DEINTERLACE)
    {
    if(do_html)
      {
      printf("<a name=\"deint\"></a>");
      }
    print_header("Deinterlacing routines");
    benchmark_deinterlace();
    }
  if(flags & BENCHMARK_INTERPOLATE)
    {
    if(do_html)
      {
      printf("<a name=\"ip\"></a>");
      }
    print_header("Video frame interpolation");
    benchmark_dsp_interpolate();
    }
  if(flags & BENCHMARK_TRANSFORM)
    {
    if(do_html)
      {
      printf("<a name=\"it\"></a>");
      }
    print_header("Video image transformation");
    benchmark_image_transform();
    }
  
  if(do_html)
    {
    printf("</body>\n</html>\n");
    }
  
  return 0;
  }
