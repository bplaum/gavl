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

#include <stdlib.h> /* calloc, free */

#include <string.h>

//#define DEBUG

//#ifdef DEBUG
#include <stdio.h>  
//#endif

#include "gavl.h"
#include "config.h"
#include "video.h"

/***************************************************
 * Create and destroy video converters
 ***************************************************/

gavl_video_converter_t * gavl_video_converter_create()
  {
  gavl_video_converter_t * ret = calloc(1,sizeof(gavl_video_converter_t));
  gavl_video_options_set_defaults(&ret->options);
  return ret;
  }

static void video_converter_cleanup(gavl_video_converter_t* cnv)
  {
  gavl_video_convert_context_t * ctx;
  while(cnv->first_context)
    {
    ctx = cnv->first_context->next;
    
    if(cnv->first_context->scaler)
      gavl_video_scaler_destroy(cnv->first_context->scaler);
    if(cnv->first_context->output_frame && cnv->first_context->next)
      gavl_video_frame_destroy(cnv->first_context->output_frame);
    free(cnv->first_context);
    cnv->first_context = ctx;
    }
  cnv->last_context = NULL;
  cnv->num_contexts = 0;
  }

void gavl_video_converter_destroy(gavl_video_converter_t* cnv)
  {
  video_converter_cleanup(cnv);
  free(cnv);
  }

/* Add a context to the converter */

static gavl_video_convert_context_t *
add_context(gavl_video_converter_t * cnv,
            const gavl_video_format_t * input_format,
            const gavl_video_format_t * output_format)
  {
  gavl_video_convert_context_t * ctx;
  ctx = calloc(1, sizeof(*ctx));
  ctx->options = &cnv->options;
  gavl_video_format_copy(&ctx->input_format,
                         input_format);
  
  gavl_video_format_copy(&ctx->output_format,
                         output_format);
  
  if(cnv->last_context)
    {
    cnv->last_context->next = ctx;
    cnv->last_context = cnv->last_context->next;
    }
  else
    {
    cnv->first_context = ctx;
    cnv->last_context = ctx;
    }
  cnv->num_contexts++;
  return ctx;
  }

static int add_context_csp(gavl_video_converter_t * cnv,
                     const gavl_video_format_t * input_format,
                     const gavl_video_format_t * output_format)
  {
  gavl_video_convert_context_t * ctx;
  ctx = add_context(cnv, input_format, output_format);

  ctx->func = gavl_find_pixelformat_converter(&cnv->options,
                                             input_format->pixelformat,
                                             output_format->pixelformat,
                                             input_format->frame_width,
                                             input_format->frame_height);

  if(!ctx->func)
    {
#if 0
    fprintf(stderr, "Found no conversion from %s to %s\n",
            gavl_pixelformat_to_string(input_format->pixelformat),
            gavl_pixelformat_to_string(output_format->pixelformat));
#endif
    return 0;
    }
#if 0
  fprintf(stderr, "Doing pixelformat conversion from %s to %s\n",
          gavl_pixelformat_to_string(input_format->pixelformat),
          gavl_pixelformat_to_string(output_format->pixelformat));
  
#endif
  return 1;
  }

static void scale_func(gavl_video_convert_context_t * ctx)
  {
  gavl_video_scaler_scale(ctx->scaler,
                          ctx->input_frame,
                          ctx->output_frame);
  }

static int add_context_scale(gavl_video_converter_t * cnv,
                             const gavl_video_format_t * input_format,
                             const gavl_video_format_t * output_format)
  {
  gavl_video_options_t * scaler_options;
  
  gavl_video_convert_context_t * ctx;
  ctx = add_context(cnv, input_format, output_format);

  ctx->scaler = gavl_video_scaler_create();

  scaler_options = gavl_video_scaler_get_options(ctx->scaler);

  gavl_video_options_copy(scaler_options, &cnv->options);
#if 0
  fprintf(stderr, "gavl_video_scaler_init:\n");
  fprintf(stderr, "src_format:\n");
  gavl_video_format_dump(input_format);
  fprintf(stderr, "dst_format:\n");
  gavl_video_format_dump(output_format);

  fprintf(stderr, "src_rectangle: ");
  gavl_rectangle_f_dump(&cnv->options.src_rect);
  fprintf(stderr, "\n");

  fprintf(stderr, "dst_rectangle: ");
  gavl_rectangle_i_dump(&cnv->options.dst_rect);
  fprintf(stderr, "\n");
#endif
  
  if(!gavl_video_scaler_init(ctx->scaler,
                             input_format,
                             output_format))
    {
    //    fprintf(stderr, "Initializing scaler failed\n");
    return 0;
    }
  ctx->func = scale_func;
  return 1;
  }

static void deinterlace_func(gavl_video_convert_context_t * ctx)
  {
  gavl_video_deinterlacer_deinterlace(ctx->deinterlacer,
                                      ctx->input_frame,
                                      ctx->output_frame);
  }

static int add_context_deinterlace(gavl_video_converter_t * cnv,
                                    const gavl_video_format_t * in_format,
                                    const gavl_video_format_t * out_format)
  {
  gavl_video_options_t * deinterlacer_options;
  gavl_video_convert_context_t * ctx;
  ctx = add_context(cnv, in_format, out_format);

  ctx->deinterlacer = gavl_video_deinterlacer_create();
  deinterlacer_options = gavl_video_deinterlacer_get_options(ctx->deinterlacer);
  gavl_video_options_copy(deinterlacer_options, &cnv->options);
  
  if(!gavl_video_deinterlacer_init(ctx->deinterlacer,
                                   in_format))
    return 0;
  
  ctx->func = deinterlace_func;
  return 1;
  }

int gavl_video_converter_reinit(gavl_video_converter_t * cnv)
  {
  int csp_then_scale = 0;
  gavl_pixelformat_t tmp_csp = GAVL_PIXELFORMAT_NONE;
  
  int do_csp = 0;
  int do_scale = 0;
  int do_deinterlace = 0;
  
  int in_sub;
  int out_sub;

  int sub_h;
  int sub_v;

  gavl_video_format_t tmp_format;
  gavl_video_format_t tmp_format1;

  gavl_video_format_t * input_format;
  gavl_video_format_t * output_format;

  input_format = &cnv->input_format;
  output_format = &cnv->output_format;
  
  // #ifdef DEBUG
#if 0
  fprintf(stderr, "Initializing video converter, quality: %d, Flags: 0x%08x\n",
          cnv->options.quality, cnv->options.accel_flags);
  gavl_video_format_dump(input_format);
  gavl_video_format_dump(output_format);

#endif
  
  video_converter_cleanup(cnv);
  
  gavl_video_format_copy(&tmp_format, input_format);
    
  /* Adjust pixelformat */
  
  if((cnv->options.alpha_mode == GAVL_ALPHA_IGNORE) &&
     (tmp_format.pixelformat == GAVL_RGBA_32) &&
     (output_format->pixelformat == GAVL_RGB_32))
    tmp_format.pixelformat = GAVL_RGB_32;
  
  /* Check for pixelformat conversion */

  if(tmp_format.pixelformat != output_format->pixelformat)
    {
    do_csp = 1;
    }
  
  if(cnv->options.src_rect.x  || cnv->options.src_rect.y ||
     cnv->options.dst_rect.x  || cnv->options.dst_rect.y ||
     (cnv->options.src_rect.w &&
      (cnv->options.src_rect.w != tmp_format.image_width)) ||
     (cnv->options.src_rect.h &&
      (cnv->options.src_rect.h != tmp_format.image_height)) ||
     (cnv->options.dst_rect.w &&
      (cnv->options.dst_rect.w != output_format->image_width)) ||
     (cnv->options.dst_rect.h &&
      (cnv->options.dst_rect.h != output_format->image_height)) ||
     (tmp_format.image_width  != output_format->image_width) ||
     (tmp_format.image_height != output_format->image_height) ||
     (tmp_format.pixel_width  != output_format->pixel_width) ||
     (tmp_format.pixel_height != output_format->pixel_height))
    {
    do_scale = 1;
    }
    
  /* For quality levels above 3, we switch on scaling, if it provides a more
     accurate conversion. This is especially true if the chroma subsampling
     ratios change or when the chroma placement becomes different */
    
  if(((cnv->options.quality > 3) ||
      (cnv->options.conversion_flags & GAVL_RESAMPLE_CHROMA) ||
      do_scale))
    {
    if(do_csp)
      {
      /* Check, if pixelformat conversion can be replaced by simple scaling
         (True if only the subsampling changes) */
      if(gavl_pixelformat_can_scale(tmp_format.pixelformat, output_format->pixelformat))
        {
        do_scale = 1;
        do_csp = 0;
        }
      else
        {
        tmp_csp = gavl_pixelformat_get_intermediate(tmp_format.pixelformat,
                                                    output_format->pixelformat);
        if(tmp_csp != GAVL_PIXELFORMAT_NONE)
          do_scale = 1;
        }
      }
    /* Having different chroma placements also switches on scaling */
    else if(tmp_format.chroma_placement != output_format->chroma_placement)
      {
      do_scale = 1;
      }
    }

  /* Check if we must deinterlace */

  if(((input_format->interlace_mode != GAVL_INTERLACE_NONE) &&
      (output_format->interlace_mode == GAVL_INTERLACE_NONE)) ||
     (cnv->options.conversion_flags & GAVL_FORCE_DEINTERLACE))
    {
    // fprintf(stderr, "Forcing deinterlacing\n");
    if(cnv->options.deinterlace_mode == GAVL_DEINTERLACE_SCALE)
      do_scale = 1;
    else if(cnv->options.deinterlace_mode != GAVL_DEINTERLACE_NONE)
      do_deinterlace = 1;
    }
  
  /* Deinterlacing must always be the first step */

  if(do_deinterlace)
    {
    gavl_video_format_copy(&tmp_format1, &tmp_format);

    tmp_format1.interlace_mode = GAVL_INTERLACE_NONE;
    if(!add_context_deinterlace(cnv, &tmp_format, &tmp_format1))
      return -1;
    gavl_video_format_copy(&tmp_format, &tmp_format1);
    }
  
  if(do_csp && do_scale)
    {
    /* For qualities below 3, we scale in the pixelformat with the
       smaller subsampling */

    if(tmp_csp == GAVL_PIXELFORMAT_NONE)
      {
      gavl_pixelformat_chroma_sub(tmp_format.pixelformat, &sub_h, &sub_v);
      in_sub = sub_h * sub_v;
      
      gavl_pixelformat_chroma_sub(output_format->pixelformat, &sub_h, &sub_v);
      out_sub = sub_h * sub_v;

      if(((in_sub < out_sub) && cnv->options.quality < 3) ||
         ((in_sub >= out_sub) && cnv->options.quality >= 3))
        csp_then_scale = 1;
      }
    else
      {
      if(!gavl_pixelformat_can_scale(input_format->pixelformat, tmp_csp))
        csp_then_scale = 1;
#if 0
      fprintf(stderr, "converting %s -> %s -> %s (%d, %d)\n",
              gavl_pixelformat_to_string(input_format->pixelformat),
              gavl_pixelformat_to_string(tmp_csp),
              gavl_pixelformat_to_string(output_format->pixelformat),
              gavl_pixelformat_can_scale(input_format->pixelformat, tmp_csp),
              gavl_pixelformat_can_scale(tmp_csp, output_format->pixelformat));
#endif
      }
    
    if(csp_then_scale) /* csp then scale */
      {
#if 0
      fprintf(stderr, "csp then scale\n");
#endif
      /* csp (tmp_format -> tmp_format1) */
      
      gavl_video_format_copy(&tmp_format1, &tmp_format);

      if(tmp_csp != GAVL_PIXELFORMAT_NONE)
        tmp_format1.pixelformat = tmp_csp;
      else
        tmp_format1.pixelformat = output_format->pixelformat;

      if(!add_context_csp(cnv, &tmp_format, &tmp_format1))
        return -1;
      
      gavl_video_format_copy(&tmp_format, &tmp_format1);

      /* scale (tmp_format -> tmp_format1) */
      
      tmp_format1.pixelformat = output_format->pixelformat;

      tmp_format1.image_width  = output_format->image_width;
      tmp_format1.image_height = output_format->image_height;

      tmp_format1.pixel_width  = output_format->pixel_width;
      tmp_format1.pixel_height = output_format->pixel_height;

      tmp_format1.frame_width  = output_format->image_width;
      tmp_format1.frame_height = output_format->image_height;
      tmp_format1.chroma_placement = output_format->chroma_placement;
      tmp_format1.interlace_mode = output_format->interlace_mode;
      
      if(!add_context_scale(cnv, &tmp_format, &tmp_format1))
        return -1;

      gavl_video_format_copy(&tmp_format, &tmp_format1);
      
      }
    /* scale then csp */
    else
      {
#if 0
      fprintf(stderr, "scale then csp\n");
#endif
      /* scale (tmp_format -> tmp_format1) */

      gavl_video_format_copy(&tmp_format1, &tmp_format);

      tmp_format1.image_width  = output_format->image_width;
      tmp_format1.image_height = output_format->image_height;

      tmp_format1.pixel_width  = output_format->pixel_width;
      tmp_format1.pixel_height = output_format->pixel_height;

      tmp_format1.frame_width  = output_format->image_width;
      tmp_format1.frame_height = output_format->image_height;
      tmp_format1.interlace_mode = output_format->interlace_mode;
      
      if(tmp_csp != GAVL_PIXELFORMAT_NONE)
        {
        tmp_format1.pixelformat = tmp_csp;
        }
      tmp_format1.chroma_placement = output_format->chroma_placement;
      
      if(!add_context_scale(cnv, &tmp_format, &tmp_format1))
        return -1;

      gavl_video_format_copy(&tmp_format, &tmp_format1);

      /* csp (tmp_format -> tmp_format1) */

      tmp_format1.pixelformat = output_format->pixelformat;
      if(!add_context_csp(cnv, &tmp_format, &tmp_format1))
        return -1;

      gavl_video_format_copy(&tmp_format, &tmp_format1);
      }
    
    }

  else if(do_csp)
    {
    if(!add_context_csp(cnv, &tmp_format,
                        output_format))
      return -1;
    }

  else if(do_scale)
    {
    if(!add_context_scale(cnv, &tmp_format,
                          output_format))
      return -1;
    }

  /* Now, create temporary frames for the contexts */

  cnv->have_frames = 0;
  
  return cnv->num_contexts;
  }

static void alloc_frames(gavl_video_converter_t * cnv)
  {
  gavl_video_convert_context_t * tmp_ctx;

  if(cnv->have_frames)
    return;

  tmp_ctx = cnv->first_context;
  while(tmp_ctx && tmp_ctx->next)
    {
    tmp_ctx->output_frame =
      gavl_video_frame_create(&tmp_ctx->output_format);
    gavl_video_frame_clear(tmp_ctx->output_frame, &tmp_ctx->output_format);
    
    tmp_ctx->next->input_frame = tmp_ctx->output_frame;
    tmp_ctx = tmp_ctx->next;
    }

  cnv->have_frames = 1;
  }

int gavl_video_converter_init(gavl_video_converter_t * cnv,
                              const gavl_video_format_t * input_format,
                              const gavl_video_format_t * output_format)
  {
  gavl_video_format_copy(&cnv->input_format, input_format);
  gavl_video_format_copy(&cnv->output_format, output_format);
  return gavl_video_converter_reinit(cnv);
  }


gavl_video_options_t *
gavl_video_converter_get_options(gavl_video_converter_t * cnv)
  {
  return &cnv->options;
  }

/***************************************************
 * Convert a frame
 ***************************************************/

void gavl_video_convert(gavl_video_converter_t * cnv,
                        const gavl_video_frame_t * input_frame,
                        gavl_video_frame_t * output_frame)
  {
  gavl_video_convert_context_t * tmp_ctx;

  alloc_frames(cnv);
  
  cnv->first_context->input_frame = input_frame;
  cnv->last_context->output_frame = output_frame;
  
  tmp_ctx = cnv->first_context;
  
  while(tmp_ctx)
    {
    gavl_video_frame_copy_metadata(tmp_ctx->output_frame,
                                   tmp_ctx->input_frame);
    tmp_ctx->func(tmp_ctx);
    tmp_ctx = tmp_ctx->next;
    }

  }
