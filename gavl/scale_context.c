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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <config.h>
#include <gavl/gavl.h>
#include <scale.h>

#include <accel.h>
#include <memalign.h>

#define SCALE_X        0
#define SCALE_Y        1
#define SCALE_XY       2
#define SCALE_X_THEN_Y 3
#define SCALE_Y_THEN_X 4

#define ALIGNMENT_BYTES 16
#define ALIGN(a) a=((a+ALIGNMENT_BYTES-1)/ALIGNMENT_BYTES)*ALIGNMENT_BYTES

#define EPS 1e-4 /* Needed to determine if a double and an int are equal. */

static void copy_scanline_noadvance(gavl_video_scale_context_t * ctx, int scanline,
                                    uint8_t * dest_start)
  {
  //  fprintf(stderr, "copy_scanline_noadvance %d\n", scanline);
  gavl_memcpy(dest_start, ctx->src + ctx->src_stride * scanline, ctx->bytes_per_line);
  //  ctx->src += ctx->src_stride;
  }

static void copy_scanline_advance(gavl_video_scale_context_t * ctx, int scanline,
                                  uint8_t * dest_start)
  {
  int i;
  uint8_t * src = ctx->src + ctx->src_stride * scanline;
  //  fprintf(stderr, "copy_scanline_advance\n");
  for(i = 0; i < ctx->dst_rect.w; i++)
    {
    *dest_start = *src;

    dest_start += ctx->offset->dst_advance;
    src += ctx->offset->src_advance;
    }
  //  ctx->src += ctx->src_stride;
  }

/* Debugging utility */
#if 0
static void dump_offset(gavl_video_scale_offsets_t*off)
  {
  fprintf(stderr, "Scale offset:\n");
  fprintf(stderr, "  src_advance: %d\n", off->src_advance);
  fprintf(stderr, "  src_offset:  %d\n", off->src_offset);
  fprintf(stderr, "  dst_advance: %d\n", off->dst_advance);
  fprintf(stderr, "  dst_offset:  %d\n", off->dst_offset);
  }
#endif

static gavl_video_scale_scanline_func get_func(gavl_scale_func_tab_t * tab,
                                               gavl_pixelformat_t pixelformat,
                                               int * bits)
  {
  switch(pixelformat)
    {
    case GAVL_PIXELFORMAT_NONE:
      break;
    case GAVL_RGB_15:
    case GAVL_BGR_15:
      *bits = tab->bits_rgb_15;
      return tab->scale_rgb_15;
      break;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
      *bits = tab->bits_rgb_16;
      return tab->scale_rgb_16;
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
      *bits = tab->bits_uint8_noadvance;
      return tab->scale_uint8_x_3;
      break;
    case GAVL_RGB_32:
    case GAVL_BGR_32:
      *bits = tab->bits_uint8_noadvance;
      return tab->scale_uint8_x_3;
      break;
    case GAVL_GRAYA_16:
      *bits = tab->bits_uint8_noadvance;
      return tab->scale_uint8_x_2;
      break;
    case GAVL_GRAY_16:
      *bits = tab->bits_uint16;
      return tab->scale_uint16_x_1;
      break;
    case GAVL_GRAYA_32:
      *bits = tab->bits_uint16;
      return tab->scale_uint16_x_2;
      break;
    case GAVL_YUVA_32:
    case GAVL_RGBA_32:
      *bits = tab->bits_uint8_noadvance;
      return tab->scale_uint8_x_4;
      break;
    case GAVL_YUY2:
      *bits = tab->bits_uint8_advance;
      return tab->scale_uint8_x_1_noadvance;
      break;
    case GAVL_UYVY:
      *bits = tab->bits_uint8_advance;
      return tab->scale_uint8_x_1_advance;
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_444_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_410_P:
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
    case GAVL_GRAY_8:
      *bits = tab->bits_uint8_noadvance;
      return tab->scale_uint8_x_1_noadvance;
      break;
    case GAVL_YUV_444_P_16:
    case GAVL_YUV_422_P_16:
      *bits = tab->bits_uint16;
      return tab->scale_uint16_x_1;
      break;
    case GAVL_RGB_48:
      *bits = tab->bits_uint16;
      return tab->scale_uint16_x_3;
      break;
    case GAVL_RGBA_64:
    case GAVL_YUVA_64:
      *bits = tab->bits_uint16;
      return tab->scale_uint16_x_4;
      break;
    case GAVL_GRAY_FLOAT:
      *bits = 0;
      return tab->scale_float_x_1;
      break;
    case GAVL_GRAYA_FLOAT:
      *bits = 0;
      return tab->scale_float_x_2;
      break;
    case GAVL_YUV_FLOAT:
    case GAVL_RGB_FLOAT:
      *bits = 0;
      return tab->scale_float_x_3;
      break;
    case GAVL_RGBA_FLOAT:
    case GAVL_YUVA_FLOAT:
      *bits = 0;
      return tab->scale_float_x_4;
      break;
    }
  return NULL;
  }

static void get_minmax(gavl_pixelformat_t pixelformat,
                       int * min, int * max, float * min_f, float * max_f)
  {
  int i;
  for(i = 0; i < GAVL_MAX_PLANES; i++)
    {
    min[i] = 0;
    max[i] = 0;
    min_f[i] = 0.0;
    max_f[i] = 0.0;
    }

  switch(pixelformat)
    {
    case GAVL_PIXELFORMAT_NONE:
      break;
    case GAVL_RGB_15:
    case GAVL_BGR_15:
      min[0] = 0;
      min[1] = 0;
      min[2] = 0;
      max[0] = (1<<5)-1;
      max[1] = (1<<5)-1;
      max[2] = (1<<5)-1;
      break;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
      min[0] = 0;
      min[1] = 0;
      min[2] = 0;
      max[0] = (1<<5)-1;
      max[1] = (1<<6)-1;
      max[2] = (1<<5)-1;
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
    case GAVL_RGBA_32:
    case GAVL_GRAY_8:
    case GAVL_GRAYA_16:
      min[0] = 0;
      min[1] = 0;
      min[2] = 0;
      min[3] = 0;
      max[0] = (1<<8)-1;
      max[1] = (1<<8)-1;
      max[2] = (1<<8)-1;
      max[3] = (1<<8)-1;
      break;
    case GAVL_YUVA_32:
      min[0] = 16;
      min[1] = 16;
      min[2] = 16;
      min[3] = 0;
      max[0] = 235;
      max[1] = 240;
      max[2] = 240;
      max[3] = 255;
      break;
    case GAVL_YUY2:
    case GAVL_UYVY:
    case GAVL_YUV_420_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_444_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_410_P:
      min[0] = 16;
      min[1] = 16;
      min[2] = 16;
      max[0] = 235;
      max[1] = 240;
      max[2] = 240;
      break;
    case GAVL_YUV_444_P_16:
    case GAVL_YUV_422_P_16:
    case GAVL_YUVA_64:
      min[0] = 16<<8;
      min[1] = 16<<8;
      min[2] = 16<<8;
      max[0] = 235<<8;
      max[1] = 240<<8;
      max[2] = 240<<8;
      max[3] = (1<<16)-1;
      break;
    case GAVL_GRAY_16:
    case GAVL_GRAYA_32:
    case GAVL_RGB_48:
    case GAVL_RGBA_64:
      min[0] = 0;
      min[1] = 0;
      min[2] = 0;
      min[3] = 0;
      max[0] = (1<<16)-1;
      max[1] = (1<<16)-1;
      max[2] = (1<<16)-1;
      max[3] = (1<<16)-1;
      break;
    case GAVL_RGB_FLOAT:
    case GAVL_RGBA_FLOAT:
    case GAVL_GRAY_FLOAT:
    case GAVL_GRAYA_FLOAT:
      max_f[0] = 1.0;
      max_f[1] = 1.0;
      max_f[2] = 1.0;
      max_f[3] = 1.0;
      break;
    case GAVL_YUV_FLOAT:
    case GAVL_YUVA_FLOAT:
      max_f[0] =  1.0;
      min_f[1] = -0.5;
      max_f[1] =  0.5;
      min_f[2] = -0.5;
      max_f[2] =  0.5;
      max_f[3] =  1.0;
      break;
    }
  }

static void alloc_temp(gavl_video_scale_context_t * ctx, gavl_pixelformat_t pixelformat)
  {
  int size;
  if((pixelformat == GAVL_YUY2) || (pixelformat == GAVL_UYVY))
    ctx->buffer_stride = ctx->buffer_width;
  else if(gavl_pixelformat_is_planar(pixelformat))
    ctx->buffer_stride = ctx->buffer_width *
      gavl_pixelformat_bytes_per_component(pixelformat);
  else
    ctx->buffer_stride = ctx->buffer_width *
      gavl_pixelformat_bytes_per_pixel(pixelformat);
  
  ALIGN(ctx->buffer_stride);

  size = ctx->buffer_stride * ctx->buffer_height;

  if(ctx->buffer_alloc < size)
    {
    if(ctx->buffer)
      free(ctx->buffer);
    ctx->buffer_alloc = size + 8192;
    ctx->buffer = gavl_memalign(ALIGNMENT_BYTES, ctx->buffer_alloc);
    }
  }

/*
 * Scale offset is the position of the first dst pixel in source
 * coordinates. First, we calculate the distances from the image border
 * of the first source and dst pixels respectively. dst_dist is scaled to
 * source coordinates and the offset becomes dst_dist - src_dist.
 */

static
float get_scale_offset(int src_field, int dst_field,
                       int src_fields, int dst_fields,
                       float scale_fac, int sub_in, int sub_out,
                       float chroma_offset_src, float chroma_offset_dst)
  {
  float result;
  
  float src_dist; /* Distance of the first source sample from the picture border in destination
                     coordinates */
  
  float dst_dist; /* Distance of the first destination sample from the picture border in
                     destination coordinates */
  
  float chroma_scale_fac = (float)sub_in / (float)sub_out;
#if 0
  fprintf(stderr, "get_scale_offset:\n\
  src_field: %d\n  dst_field: %d\n\
  src_fields: %d\n  dst_fields: %d\n\
  scale_fac: %f\n  sub_in: %d\n\
  sub_out: %d\n  chroma_offset_src: %f\n\
  chroma_offset_dst: %f\n\
  chroma_scale_fac: %f\n",
          src_field, dst_field,
          src_fields, dst_fields,
          scale_fac, sub_in, sub_out,
          chroma_offset_src, chroma_offset_dst,
          chroma_scale_fac);
#endif

  scale_fac *= chroma_scale_fac;
  
  /* Different calculation for diffrent interlacing options */
  
  if(src_fields == 1) /* Progressive input -> Progressive */
    {
    //    fprintf(stderr, "Progressive -> Progressive\n");

    src_dist = (0.5 + chroma_offset_src) / sub_in;
    dst_dist = (0.5 + chroma_offset_dst) / (sub_out * scale_fac);
    }
  else if(dst_fields == 1) /* Interlaced -> Progressive */
    {
    if(src_field == 0) /* Top field -> Progressive */
      {
      //      fprintf(stderr, "Top field -> Progressive\n");

      src_dist = (0.25 + chroma_offset_src) / (sub_in);
      dst_dist = (0.5 + chroma_offset_dst) / (2.0*scale_fac * sub_out);
      }
    else /* Bottom field -> Progressive */
      {
      //      fprintf(stderr, "Bottom field -> Progressive\n");
      
      src_dist = (0.75 + chroma_offset_src) / (sub_in);
      dst_dist = (0.5 + chroma_offset_dst) / (2.0*scale_fac * sub_out);
      }
    }
  else /* Interlaced -> Interlaced */
    {
    if(dst_field == src_field)
      {
      if(!dst_field) /* Top field -> Top field */
        {
        //        fprintf(stderr, "Top field -> Top field\n");

        src_dist = (0.25 + chroma_offset_src) / (sub_in);
        dst_dist = (0.25 + chroma_offset_dst) / (scale_fac * sub_out);
        
        }
      else /* Bottom field -> Bottom field */
        {
        //        fprintf(stderr, "Bottom field -> Bottom field\n");

        src_dist = (0.75 + chroma_offset_src) / (sub_in);
        dst_dist = (0.75 + chroma_offset_dst) / (scale_fac * sub_out);
        }
      }
    else
      {
      fprintf(stderr, "BUG: scaler cannot swap fields\n");
      return 0;
      }
    }

  result = dst_dist - src_dist;
#if 0
  fprintf(stderr, "Src dist: %f, dst_dist: %f\n", src_dist, dst_dist);
  fprintf(stderr, "result: %f\n", result);
#endif
  return result;
  }

int gavl_video_scale_context_init(gavl_video_scale_context_t*ctx,
                                  gavl_video_options_t * opt, int plane,
                                  const gavl_video_format_t * src_format,
                                  const gavl_video_format_t * dst_format,
                                  int src_field, int dst_field,
                                  int src_fields, int dst_fields)
  {
  int bits_h = 0, bits_v = 0;
  int sub_h_in = 1, sub_v_in = 1, sub_h_out = 1, sub_v_out = 1;
  int scale_x, scale_y;
  gavl_video_options_t tmp_opt, tmp_opt_y;
  double scale_factor_x, scale_factor_y;
  float src_chroma_offset_x, src_chroma_offset_y, dst_chroma_offset_x, dst_chroma_offset_y;
  gavl_rectangle_i_t src_rect_i;

  
  int src_width, src_height; /* Needed for generating the scale table */
  float offset_x, offset_y;
  
  gavl_scale_funcs_t funcs;

  ctx->first_scanline = 0;

#if 0  
  fprintf(stderr, "scale_context_init: src_field: %d, dst_field: %d plane: %d\n",
          src_field, dst_field, plane);
#endif  
  gavl_rectangle_f_copy(&ctx->src_rect, &opt->src_rect);
  gavl_rectangle_i_copy(&ctx->dst_rect, &opt->dst_rect);
  
  scale_factor_x = (float)ctx->dst_rect.w / ctx->src_rect.w;
  scale_factor_y = (float)ctx->dst_rect.h / ctx->src_rect.h;
    
  ctx->plane = plane;

  
  
  if(plane)
    {
    /* Get chroma subsampling factors for source and destination */
    gavl_pixelformat_chroma_sub(src_format->pixelformat, &sub_h_in, &sub_v_in);
    gavl_pixelformat_chroma_sub(dst_format->pixelformat, &sub_h_out, &sub_v_out);
    
    ctx->src_rect.w /= sub_h_in;
    ctx->src_rect.x /= sub_h_in;

    ctx->src_rect.h /= sub_v_in;
    ctx->src_rect.y /= sub_v_in;
    
    ctx->dst_rect.w /= sub_h_out;
    ctx->dst_rect.x /= sub_h_out;

    ctx->dst_rect.h /= sub_v_out;
    ctx->dst_rect.y /= sub_v_out;

    src_width = src_format->image_width / sub_h_in;
    src_height = src_format->image_height / sub_v_in;

    }
  else
    {
    src_width = src_format->image_width;
    src_height = src_format->image_height;
    }
  
  if(src_fields == 2)
    {
    ctx->src_rect.h /= 2.0;
    ctx->src_rect.y /= 2.0;
    src_height /= 2;
    }
  if(dst_fields == 2)
    {
    ctx->dst_rect.h /= 2;
    ctx->dst_rect.y /= 2;
    }
  
#if 0
  fprintf(stderr, "gavl_video_scale_context_init\n");
  gavl_rectangle_f_dump(&ctx->src_rect);
  fprintf(stderr, "\n");
  gavl_rectangle_i_dump(&ctx->dst_rect);
  fprintf(stderr, "\n");
#endif
  

  /* Calculate chroma offsets  */
    
  gavl_video_format_get_chroma_offset(src_format, src_field, plane,
                                      &src_chroma_offset_x,
                                      &src_chroma_offset_y);
  
  gavl_video_format_get_chroma_offset(dst_format, dst_field, plane,
                                      &dst_chroma_offset_x,
                                      &dst_chroma_offset_y);
  
  offset_x = get_scale_offset(0, 0, 1, 1,
                              scale_factor_x, sub_h_in, sub_h_out,
                              src_chroma_offset_x, dst_chroma_offset_x);
  offset_y = get_scale_offset(src_field, dst_field, src_fields, dst_fields,
                              scale_factor_y, sub_v_in, sub_v_out,
                              src_chroma_offset_y, dst_chroma_offset_y);
  
  if((fabs(ctx->src_rect.w - ctx->dst_rect.w) > EPS) ||
     (fabs(ctx->src_rect.x) > EPS) || (fabs(offset_x) > EPS))
    scale_x = 1;
  else
    scale_x = 0;
  
  if((fabs(ctx->src_rect.h - ctx->dst_rect.h) > EPS) ||
     (fabs(ctx->src_rect.y) > EPS) || (fabs(offset_y) > EPS))
    scale_y = 1;
  else
    scale_y = 0;
#if 0
  fprintf(stderr, "Offsets: %f %f, scale_factors: %f %f\n",
          offset_x, offset_y, ctx->dst_rect.w / ctx->src_rect.w,
          ctx->dst_rect.h / ctx->src_rect.h);
#endif  
  offset_x += ctx->src_rect.x;
  offset_y += ctx->src_rect.y;

  ctx->func1 = NULL;
  ctx->func2 = NULL;
  
  ctx->num_directions = 0;
  if(scale_x)
    ctx->num_directions++;
  if(scale_y)
    ctx->num_directions++;

  /* Set source and destination frame planes */

  if(gavl_pixelformat_is_planar(src_format->pixelformat))
    ctx->src_frame_plane = plane;
  else
    ctx->src_frame_plane = 0;

  if(gavl_pixelformat_is_planar(dst_format->pixelformat))
    ctx->dst_frame_plane = plane;
  else
    ctx->dst_frame_plane = 0;

  /* Set bytes per line. We need this for copying */
  ctx->bytes_per_line = gavl_pixelformat_is_planar(src_format->pixelformat) ?
    ctx->dst_rect.w * gavl_pixelformat_bytes_per_component(src_format->pixelformat) :
    ctx->dst_rect.w * gavl_pixelformat_bytes_per_pixel(src_format->pixelformat);
  
  /* Set source and destination offsets */
  
  if(ctx->num_directions == 1)
    {
    gavl_pixelformat_get_offset(src_format->pixelformat,
                plane, &ctx->offset1.src_advance, &ctx->offset1.src_offset);
    gavl_pixelformat_get_offset(dst_format->pixelformat,
                plane, &ctx->offset1.dst_advance, &ctx->offset1.dst_offset);

    /* We set this once here */

    ctx->offset = &ctx->offset1;
    ctx->dst_size = ctx->dst_rect.w;
    }
  else if(ctx->num_directions == 2)
    {
    gavl_pixelformat_get_offset(src_format->pixelformat,
                plane, &ctx->offset1.src_advance, &ctx->offset1.src_offset);
    gavl_pixelformat_get_offset(dst_format->pixelformat,
                plane, &ctx->offset2.dst_advance, &ctx->offset2.dst_offset);

    ctx->offset1.dst_offset = 0;
    ctx->offset1.dst_advance = ctx->offset1.src_advance;

    if((src_format->pixelformat == GAVL_YUY2) || 
       (src_format->pixelformat == GAVL_UYVY))
      {
      ctx->offset1.dst_advance = 1;
      }
    ctx->offset2.src_advance = ctx->offset1.dst_advance;
    ctx->offset2.src_offset  = ctx->offset1.dst_offset;
    }
  else if(!ctx->num_directions)
    {
    if((src_format->pixelformat == GAVL_YUY2) ||
       (src_format->pixelformat == GAVL_UYVY) ||
       (dst_format->pixelformat == GAVL_YUY2) ||
       (dst_format->pixelformat == GAVL_UYVY))
      ctx->func1 = copy_scanline_advance;
    else
      ctx->func1 = copy_scanline_noadvance;

    /* Set source and destination offsets */
    gavl_pixelformat_get_offset(src_format->pixelformat,
                plane, &ctx->offset1.src_advance, &ctx->offset1.src_offset);
    gavl_pixelformat_get_offset(dst_format->pixelformat,
                plane, &ctx->offset1.dst_advance, &ctx->offset1.dst_offset);

    /* We set this once here */

    ctx->offset = &ctx->offset1;
    ctx->dst_size = ctx->dst_rect.w;

    ctx->num_directions = 1;
    return 1;
    }

  if(scale_x && scale_y)
    {
    // fprintf(stderr, "Initializing x table %f\n", ctx->src_rect.x + offset_x);
    gavl_video_options_copy(&tmp_opt, opt);
    gavl_video_scale_table_init(&ctx->table_h, &tmp_opt,
                                offset_x,
                                ctx->src_rect.w, ctx->dst_rect.w, src_width);
    //    fprintf(stderr, "Initializing x table done\n");

    //    fprintf(stderr, "Initializing y table %f\n",
    //            ctx->src_rect.y + offset_y);
    gavl_video_options_copy(&tmp_opt_y, opt);
    gavl_video_scale_table_init(&ctx->table_v, &tmp_opt_y,
                                offset_y,
                                ctx->src_rect.h, ctx->dst_rect.h, src_height);
    
    //    fprintf(stderr, "Initializing y table done\n");
    
    /* Check if we can scale in x and y-directions at once */

    if((tmp_opt.scale_mode == tmp_opt_y.scale_mode) &&
       (tmp_opt.scale_order == tmp_opt_y.scale_order))
      {
      gavl_init_scale_funcs(&funcs, &tmp_opt, ctx->offset1.src_advance,
                            ctx->offset2.dst_advance, &ctx->table_h,
                            &ctx->table_v);
      ctx->func1 = get_func(&funcs.funcs_xy, src_format->pixelformat, &bits_h);
      //      fprintf(stderr, "X AND Y %d\n");
      }
    
    if(ctx->func1) /* Scaling routines for x-y are there, good */
      {
      ctx->num_directions = 1;
      if(bits_h)
        {
        gavl_video_scale_table_init_int(&ctx->table_h, bits_h);
      /* Must be bits_h since we have only one function (and thus one accuracy) */
        gavl_video_scale_table_init_int(&ctx->table_v, bits_h);
        }
      ctx->offset = &ctx->offset1;
      ctx->dst_size = ctx->dst_rect.w;
      
      /* Switch offsets back */
      ctx->offset1.dst_advance = ctx->offset2.dst_advance;
      ctx->offset1.dst_offset  = ctx->offset2.dst_offset;
      }
    else
      {
      gavl_video_scale_table_get_src_indices(&ctx->table_h,
                                             &src_rect_i.x, &src_rect_i.w);
      gavl_video_scale_table_get_src_indices(&ctx->table_v,
                                             &src_rect_i.y, &src_rect_i.h);

      /* Decide the scale order depending on whats computationally less expensive */
      /* We calculate the sizes (in pixels) of the temporary frame for both options and
         take the smaller one */
#if 0 
      fprintf(stderr, "Src rect: ");
      gavl_rectangle_i_dump(&src_rect_i);
      fprintf(stderr, "\nDst rect: ");
      gavl_rectangle_i_dump(&ctx->dst_rect);
      fprintf(stderr, "\n");
#endif
      if(src_rect_i.h * ctx->dst_rect.w < ctx->dst_rect.h * src_rect_i.w)
        {
        //        fprintf(stderr, "X then Y\n");
        /* X then Y */

        ctx->buffer_width  = ctx->dst_rect.w;
        ctx->buffer_height = src_rect_i.h;
        
        gavl_video_scale_table_shift_indices(&ctx->table_v, -src_rect_i.y);
        ctx->first_scanline = src_rect_i.y;
        
            gavl_init_scale_funcs(&funcs, &tmp_opt,
                              ctx->offset1.src_advance,
                              ctx->offset1.dst_advance,
                              &ctx->table_h, NULL);
        ctx->func1 = get_func(&funcs.funcs_x, src_format->pixelformat, &bits_h);
        if(bits_h)
          gavl_video_scale_table_init_int(&ctx->table_h, bits_h);
        
        gavl_init_scale_funcs(&funcs, &tmp_opt_y,
                              ctx->offset2.src_advance,
                              ctx->offset2.dst_advance,
                              NULL, &ctx->table_v);
        ctx->func2 = get_func(&funcs.funcs_y,
                              src_format->pixelformat, &bits_v);

        if(bits_v)
          gavl_video_scale_table_init_int(&ctx->table_v, bits_v);
        }
      else
        {
        //        fprintf(stderr, "Y then X\n");
        /* Y then X */

        ctx->buffer_width = src_rect_i.w;
        ctx->buffer_height = ctx->dst_rect.h;
        
        ctx->offset1.src_offset += src_rect_i.x * ctx->offset1.src_advance;
        
        gavl_video_scale_table_shift_indices(&ctx->table_h, -src_rect_i.x);

        gavl_init_scale_funcs(&funcs, &tmp_opt_y,
                              ctx->offset1.src_advance,
                              ctx->offset1.dst_advance,
                              NULL, &ctx->table_v);
        ctx->func1 = get_func(&funcs.funcs_y, src_format->pixelformat, &bits_v);

        if(bits_v)
          gavl_video_scale_table_init_int(&ctx->table_v, bits_v);

        gavl_init_scale_funcs(&funcs, &tmp_opt,
                              ctx->offset2.src_advance,
                              ctx->offset2.dst_advance,
                              &ctx->table_h, NULL);
        ctx->func2 = get_func(&funcs.funcs_x,
                              src_format->pixelformat, &bits_h);
        if(bits_h)
          gavl_video_scale_table_init_int(&ctx->table_h, bits_h);
        }
      
      /* Allocate temporary buffer */
      alloc_temp(ctx, src_format->pixelformat);
      }
    }
  else if(scale_x)
    {
    gavl_video_options_copy(&tmp_opt, opt);
    gavl_video_scale_table_init(&ctx->table_h, &tmp_opt,
                                offset_x,
                                ctx->src_rect.w, ctx->dst_rect.w, src_width);
    //    fprintf(stderr, "Initializing x table done\n");

    gavl_init_scale_funcs(&funcs, &tmp_opt,
                          ctx->offset1.src_advance,
                          ctx->offset1.dst_advance,
                          &ctx->table_h, NULL);
    ctx->func1 = get_func(&funcs.funcs_x, src_format->pixelformat, &bits_h);

    if(bits_h)
      gavl_video_scale_table_init_int(&ctx->table_h, bits_h);
    }
  else if(scale_y)
    {
    //    fprintf(stderr, "Initializing y table\n");
    gavl_video_options_copy(&tmp_opt, opt);

    gavl_video_scale_table_init(&ctx->table_v, &tmp_opt, offset_y,
                                ctx->src_rect.h, ctx->dst_rect.h, src_height);
    //    fprintf(stderr, "Initializing y table done\n");
    gavl_init_scale_funcs(&funcs, &tmp_opt,
                          ctx->offset1.src_advance,
                          ctx->offset1.dst_advance,
                          NULL, &ctx->table_v);
    ctx->func1 = get_func(&funcs.funcs_y, src_format->pixelformat, &bits_v);
    
    if(bits_v)
      gavl_video_scale_table_init_int(&ctx->table_v, bits_v);
    }

  if(!ctx->func1 || ((ctx->num_directions == 2) && !ctx->func2))
    {
    //    fprintf(stderr, "Initializing scale context failed: %d %p %p\n",
    //            ctx->num_directions,
    //            ctx->func1, ctx->func2);
    return 0;
    }
#if 0  
  /* Dump final scale tables */
  fprintf(stderr, "Horizontal table:\n");
  gavl_video_scale_table_dump(&ctx->table_h);
  fprintf(stderr, "Vertical table:\n");
  gavl_video_scale_table_dump(&ctx->table_v);
#endif
  
  get_minmax(src_format->pixelformat, ctx->min_values_h,
             ctx->max_values_h, ctx->min_values_f, ctx->max_values_f);
  
  get_minmax(src_format->pixelformat, ctx->min_values_v,
             ctx->max_values_v, ctx->min_values_f, ctx->max_values_f);
  
#if 0
  fprintf(stderr, "Min: %d %d %d, max: %d %d %d\n",
          ctx->min_values_h[0],
          ctx->min_values_h[1],
          ctx->min_values_h[2],
          ctx->max_values_h[0],
          ctx->max_values_h[1],
          ctx->max_values_h[2]);
#endif
    
  return 1;
  }

/* Factor must be a power of 2 */

static void downsample_coeffs(int factor,
                              const float * coeffs, int num_coeffs,
                              float ** coeffs_ret, 
                              int * num_coeffs_ret)
  {
  int i;
  int fac = 1;
  int src_index;
  float * coeffs_1;
  float * coeffs_2;
  float * swp;
  int coeffs_total;
  int last_coeffs_total;

  *num_coeffs_ret = num_coeffs;
  coeffs_total = *num_coeffs_ret * 2 + 1;

  coeffs_1 = malloc(coeffs_total * sizeof(*coeffs_1));
  coeffs_2 = malloc(coeffs_total * sizeof(*coeffs_2));

  memcpy(coeffs_1, coeffs, coeffs_total * sizeof(*coeffs));

  //  fprintf(stderr, "downsample_coeffs: num_coeffs: %d, factor: %d\n",
  //          num_coeffs, factor);
  
  while(fac < factor)
    {
    *num_coeffs_ret = (*num_coeffs_ret + 1)/2;
    last_coeffs_total = coeffs_total;
    coeffs_total = *num_coeffs_ret * 2 + 1;

    if(!(*num_coeffs_ret & 1))
      {
      coeffs_2[0] = 0.5 * coeffs_1[0] + coeffs_1[1];
      src_index = 2;
      for(i = 1; i < coeffs_total - 1; i++)
        {
        coeffs_2[i] = 0.5 * (coeffs_1[src_index-1] + 
                             coeffs_1[src_index+1]) +
                      coeffs_1[src_index];
        src_index += 2;
        }
      coeffs_2[coeffs_total - 1] = 
        0.5 * coeffs_1[last_coeffs_total - 1] + 
              coeffs_1[last_coeffs_total - 2];
      }
    else
      {
      coeffs_2[0] =  0.5 * coeffs_1[0];
      src_index = 1;
      for(i = 1; i < coeffs_total - 1; i++)
        {
        coeffs_2[i] = 0.5 * (coeffs_1[src_index-1] + 
                             coeffs_1[src_index+1]) +
          coeffs_1[src_index];
        src_index += 2;
        }
      coeffs_2[coeffs_total-1] = 
        0.5 * coeffs_1[last_coeffs_total-1];
      }
    swp = coeffs_1;
    coeffs_1 = coeffs_2;
    coeffs_2 = swp;
    fac *= 2;
    }
  *coeffs_ret = coeffs_1;
  free(coeffs_2);
  }

int 
gavl_video_scale_context_init_convolve(gavl_video_scale_context_t* ctx,
                                       gavl_video_options_t * opt,
                                       int plane,
                                       const gavl_video_format_t * format,
                                       int num_fields,
                                       int h_radius, const float * h_coeffs,
                                       int v_radius, const float * v_coeffs)
  {
  int bits_h = 1, bits_v = 1;
  int sub_h = 1, sub_v = 1;
  int scale_x, scale_y;

  int fac_h = 1;
  int fac_v = 1;

  gavl_video_options_t tmp_opt, tmp_opt_y;
  gavl_rectangle_i_t src_rect_i;

  int h_radius_real;
  float *h_c = NULL;
  int v_radius_real;
  float *v_c = NULL;

  int src_width, src_height; /* Needed for generating the scale table */
 
  gavl_scale_funcs_t funcs;
  
#if 0  
  fprintf(stderr, "scale_context_init: src_field: %d, dst_field: %d plane: %d\n",
          src_field, dst_field, plane);
#endif  

  gavl_rectangle_f_set_all(&ctx->src_rect, format);
  gavl_rectangle_i_set_all(&ctx->dst_rect, format);

  ctx->first_scanline = 0;
  
  ctx->plane = plane;
  
  if(plane)
    {
    /* Get chroma subsampling factors for source and destination */
    gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
    
    ctx->src_rect.w /= sub_h;
    ctx->src_rect.h /= sub_v;
    ctx->dst_rect.w /= sub_h;
    ctx->dst_rect.h /= sub_v;

    fac_h *= sub_h;
    fac_v *= sub_v;
    
    src_width = format->image_width / sub_h;
    src_height = format->image_height / sub_v;
    }
  else
    {
    src_width =  format->image_width;
    src_height = format->image_height;
    }
  
  if(num_fields == 2)
    {
    ctx->src_rect.h /= 2.0;
    ctx->dst_rect.h /= 2;
    src_height /= 2;
    fac_v *= 2;
    }
  
#if 0
  fprintf(stderr, "gavl_video_scale_context_init\n");
  gavl_rectangle_f_dump(&ctx->src_rect);
  fprintf(stderr, "\n");
  gavl_rectangle_i_dump(&ctx->dst_rect);
  fprintf(stderr, "\n");
#endif
  
  /* Calculate chroma offsets  */
  if(plane && !(opt->conversion_flags & GAVL_CONVOLVE_CHROMA))
    {
    scale_x = 0;
    scale_y = 0;
    }
  else
    {
    if(h_radius > 0)
      scale_x = 1;
    else
      scale_x = 0;

    if(v_radius > 0)
      scale_y = 1;
    else
      scale_y = 0;
    }

  /* Downsample coefficients */

  h_radius_real = h_radius;
  v_radius_real = v_radius;

  if(scale_x)
    {
    if(fac_h > 1)
      {
      downsample_coeffs(fac_h, h_coeffs, h_radius,
                        &h_c, &h_radius_real);
      }
    }
  if(scale_y)
    {
    if(fac_v > 1)
      {
      downsample_coeffs(fac_v, v_coeffs, v_radius,
                        &v_c, &v_radius_real);
      }
    }
 
  ctx->func1 = NULL;
  ctx->func2 = NULL;
  
  ctx->num_directions = 0;
  if(scale_x)
    ctx->num_directions++;
  if(scale_y)
    ctx->num_directions++;
  
  /* Set source and destination frame planes */
  
  if(gavl_pixelformat_is_planar(format->pixelformat))
    {
    ctx->src_frame_plane = plane;
    ctx->dst_frame_plane = plane;
    }
  else
    {
    ctx->src_frame_plane = 0;
    ctx->dst_frame_plane = 0;
    }

  
  ctx->bytes_per_line = 
    gavl_pixelformat_is_planar(format->pixelformat) ?
    ctx->dst_rect.w * gavl_pixelformat_bytes_per_component(format->pixelformat) :
    ctx->dst_rect.w * gavl_pixelformat_bytes_per_pixel(format->pixelformat);
  
  /* Set source and destination offsets */

  if(ctx->num_directions == 1)
    {
    gavl_pixelformat_get_offset(format->pixelformat,
                plane, &ctx->offset1.src_advance, &ctx->offset1.src_offset);
    gavl_pixelformat_get_offset(format->pixelformat,
                plane, &ctx->offset1.dst_advance, &ctx->offset1.dst_offset);

    /* We set this once here */

    ctx->offset = &ctx->offset1;
    ctx->dst_size = ctx->dst_rect.w;
    }
  else if(ctx->num_directions == 2)
    {
    gavl_pixelformat_get_offset(format->pixelformat,
                plane, &ctx->offset1.src_advance, &ctx->offset1.src_offset);

    gavl_pixelformat_get_offset(format->pixelformat,
                plane, &ctx->offset2.dst_advance, &ctx->offset2.dst_offset);

    ctx->offset1.dst_offset = 0;
    ctx->offset1.dst_advance = ctx->offset1.src_advance;

    if((format->pixelformat == GAVL_YUY2) || 
       (format->pixelformat == GAVL_UYVY))
      {
      ctx->offset1.dst_advance = 1;
      }
    ctx->offset2.src_advance = ctx->offset1.dst_advance;
    ctx->offset2.src_offset  = ctx->offset1.dst_offset;
    
    }
  
  /* Set functions */
  
  if(!ctx->num_directions)
    {
    if((format->pixelformat == GAVL_YUY2) ||
       (format->pixelformat == GAVL_UYVY))
      ctx->func1 = copy_scanline_advance;
    else
      ctx->func1 = copy_scanline_noadvance;
    
    /* Set source and destination offsets */
    gavl_pixelformat_get_offset(format->pixelformat,
                plane, &ctx->offset1.src_advance, &ctx->offset1.src_offset);
    gavl_pixelformat_get_offset(format->pixelformat,
                plane, &ctx->offset1.dst_advance, &ctx->offset1.dst_offset);

    /* We set this once here */

    ctx->offset = &ctx->offset1;
    ctx->dst_size = ctx->dst_rect.w;

    ctx->num_directions = 1;
    return 0;
    }

  else if(scale_x && scale_y)
    {
    //    fprintf(stderr, "Initializing x table\n");
    gavl_video_options_copy(&tmp_opt, opt);
    tmp_opt.scale_mode = GAVL_SCALE_NONE;
    gavl_video_scale_table_init_convolve(&ctx->table_h,
                                         &tmp_opt,
                                         h_radius, h_coeffs,
                                         src_width);
    //    fprintf(stderr, "Initializing x table done\n");

    //    fprintf(stderr, "Initializing y table\n");
    gavl_video_options_copy(&tmp_opt_y, opt);
    tmp_opt_y.scale_mode = GAVL_SCALE_NONE;
    gavl_video_scale_table_init_convolve(&ctx->table_v,
                                         &tmp_opt_y,
                                         v_radius, v_coeffs,
                                         src_height);
   
    //    fprintf(stderr, "Initializing y table done\n");
    
    /* Check if we can scale in x and y-directions at once */

    if((tmp_opt.scale_mode == tmp_opt_y.scale_mode) &&
       (tmp_opt.scale_order == tmp_opt_y.scale_order))
      {
      gavl_init_scale_funcs(&funcs, &tmp_opt,
                            ctx->offset1.src_advance,
                            ctx->offset2.dst_advance,
                            &ctx->table_h, &ctx->table_v);
      ctx->func1 = get_func(&funcs.funcs_xy, format->pixelformat, &bits_h);
      //      fprintf(stderr, "X AND Y\n");
      }
    
    if(ctx->func1) /* Scaling routines for x-y are there, good */
      {
      ctx->num_directions = 1;
            
      gavl_video_scale_table_init_int(&ctx->table_h, bits_h);
      gavl_video_scale_table_init_int(&ctx->table_v, bits_h);
      }
    else
      {
      gavl_video_scale_table_get_src_indices(&ctx->table_h,
                                             &src_rect_i.x,
                                             &src_rect_i.w);
      
      gavl_video_scale_table_get_src_indices(&ctx->table_v,
                                             &src_rect_i.y,
                                             &src_rect_i.h);
      
      //        fprintf(stderr, "X then Y\n");
      /* X then Y */
      
      ctx->buffer_width  = ctx->dst_rect.w;
      ctx->buffer_height = src_rect_i.h;
        
      gavl_video_scale_table_shift_indices(&ctx->table_v,
                                           -src_rect_i.y);
        gavl_init_scale_funcs(&funcs, &tmp_opt,
                            ctx->offset1.src_advance,
                            ctx->offset1.dst_advance,
                            &ctx->table_h, NULL);
      ctx->func1 = get_func(&funcs.funcs_x, format->pixelformat, &bits_h);

      gavl_video_scale_table_init_int(&ctx->table_h, bits_h);

        gavl_init_scale_funcs(&funcs, &tmp_opt_y,
                            ctx->offset2.src_advance,
                            ctx->offset2.dst_advance,
                            NULL, &ctx->table_v);
      ctx->func2 = get_func(&funcs.funcs_y, format->pixelformat, &bits_v);


      gavl_video_scale_table_init_int(&ctx->table_v, bits_v);
     
      /* Allocate temporary buffer */
      alloc_temp(ctx, format->pixelformat);
      }
    }
  else if(scale_x)
    {
    //    fprintf(stderr, "Initializing x table\n");
    gavl_video_options_copy(&tmp_opt, opt);
    tmp_opt.scale_mode = GAVL_SCALE_NONE;

    gavl_video_scale_table_init_convolve(&ctx->table_h,
                                         &tmp_opt,
                                         h_radius, h_coeffs,
                                         src_width);
    //    fprintf(stderr, "Initializing x table done\n");
    gavl_init_scale_funcs(&funcs, &tmp_opt,
                          ctx->offset1.src_advance,
                          ctx->offset1.dst_advance,
                          &ctx->table_h, NULL);
    ctx->func1 = get_func(&funcs.funcs_x, format->pixelformat, &bits_h);
    
    gavl_video_scale_table_init_int(&ctx->table_h, bits_h);
    }
  else if(scale_y)
    {
    //    fprintf(stderr, "Initializing y table\n");
    gavl_video_options_copy(&tmp_opt, opt);
    tmp_opt.scale_mode = GAVL_SCALE_NONE;
    gavl_video_scale_table_init_convolve(&ctx->table_v,
                                         &tmp_opt,
                                         v_radius, v_coeffs,
                                         src_height);
    
    //    fprintf(stderr, "Initializing y table done\n");
    gavl_init_scale_funcs(&funcs, &tmp_opt,
                          ctx->offset1.src_advance,
                          ctx->offset1.dst_advance,
                          NULL, &ctx->table_v);
    ctx->func1 = get_func(&funcs.funcs_y, format->pixelformat, &bits_v);
    
    gavl_video_scale_table_init_int(&ctx->table_v, bits_v);
    }
  
#if 0
  /* Dump final scale tables */
  fprintf(stderr, "Horizontal table:\n");
  gavl_video_scale_table_dump(&ctx->table_h);
  fprintf(stderr, "Vertical table:\n");
  gavl_video_scale_table_dump(&ctx->table_v);
#endif

  
  get_minmax(format->pixelformat, ctx->min_values_h, ctx->max_values_h, ctx->min_values_f, ctx->max_values_f);
  get_minmax(format->pixelformat, ctx->min_values_v, ctx->max_values_v, ctx->min_values_f, ctx->max_values_f);

  if(h_c) free(h_c);
  if(v_c) free(v_c);
  
  return 1;
  }

void gavl_video_scale_context_cleanup(gavl_video_scale_context_t * ctx)
  {
  gavl_video_scale_table_cleanup(&ctx->table_h);
  gavl_video_scale_table_cleanup(&ctx->table_v);
  if(ctx->buffer)
    free(ctx->buffer);
  }

static void func_1(void* p, int start, int end)
  {
  int i;
  uint8_t * dst_save;
  
  gavl_video_scale_context_t * ctx = p;

  dst_save = ctx->dst_frame->planes[ctx->dst_frame_plane] + ctx->offset->dst_offset +
    start * ctx->dst_frame->strides[ctx->dst_frame_plane];

  for(i = start; i < end; i++)
    {
    ctx->func1(ctx, i, dst_save);
    dst_save += ctx->dst_frame->strides[ctx->dst_frame_plane];
    }
#ifdef HAVE_MMX
  __asm__ __volatile__ ("emms");
#endif

  }

static void func_1_of_2(void* p, int start, int end)
  {
  int i;
  uint8_t * dst_save;
  gavl_video_scale_context_t * ctx = p;
  
  dst_save = ctx->buffer + start * ctx->buffer_stride;
  for(i = start; i < end; i++)
    {
    ctx->func1(ctx, i, dst_save);
    dst_save += ctx->buffer_stride;
    }
#ifdef HAVE_MMX
  __asm__ __volatile__ ("emms");
#endif
  
  }

static void func_2_of_2(void* p, int start, int end)
  {
  int i;
  uint8_t * dst_save;
  gavl_video_scale_context_t * ctx = p;
  dst_save =
    ctx->dst_frame->planes[ctx->dst_frame_plane] + ctx->offset->dst_offset +
    start * ctx->dst_frame->strides[ctx->dst_frame_plane];

  for(i = start; i < end; i++)
    {
    ctx->func2(ctx, i, dst_save);
    dst_save += ctx->dst_frame->strides[ctx->dst_frame_plane];
    }
#ifdef HAVE_MMX
  __asm__ __volatile__ ("emms");
#endif
  }


static void scale_context_scale_mt(gavl_video_scale_context_t * ctx,
                                   const gavl_video_frame_t * src,
                                   gavl_video_frame_t * dst)
  {
  int delta;
  int scanline;
  int nt;
  int i;
  switch(ctx->num_directions)
    {
    case 1:
      /* Only step */
      ctx->src = src->planes[ctx->src_frame_plane] + ctx->offset->src_offset;
      ctx->src_stride = src->strides[ctx->src_frame_plane];
      ctx->dst_frame = dst;
      
      nt = ctx->opt->num_threads;
      if(nt > ctx->dst_rect.h)
        nt = ctx->dst_rect.h;
      
      delta = ctx->dst_rect.h / nt;
      scanline = 0;
      for(i = 0; i < nt - 1; i++)
        {
        ctx->opt->run_func(func_1, ctx, scanline, scanline+delta, ctx->opt->run_data, i);
        
        //        fprintf(stderr, "scanline: %d (%d)\n", ctx->scanline, ctx->dst_rect.h);
        scanline += delta;
        }
      ctx->opt->run_func(func_1, ctx, scanline, ctx->dst_rect.h, ctx->opt->run_data, nt - 1);
      
      for(i = 0; i < nt; i++)
        ctx->opt->stop_func(ctx->opt->stop_data, i);
      break;
    case 2:
      /* First step */
      ctx->offset = &ctx->offset1;
      
      ctx->src = src->planes[ctx->src_frame_plane] +
        ctx->offset->src_offset +
        src->strides[ctx->src_frame_plane] * ctx->first_scanline;
      
      ctx->src_stride = src->strides[ctx->src_frame_plane];
      ctx->dst_size = ctx->buffer_width;

#if 0
      fprintf(stderr, "First direction\n");
      dump_offset(ctx->offset);
#endif

      nt = ctx->opt->num_threads;
      if(nt > ctx->buffer_height)
        nt = ctx->buffer_height;

      delta = ctx->buffer_height / nt;
      scanline = 0;
      for(i = 0; i < nt - 1; i++)
        {
        ctx->opt->run_func(func_1_of_2, ctx, scanline, scanline+delta, ctx->opt->run_data, i);
        
        //        fprintf(stderr, "scanline: %d (%d)\n", ctx->scanline, ctx->dst_rect.h);
        scanline += delta;
        }
      ctx->opt->run_func(func_1_of_2, ctx, scanline, ctx->buffer_height,
                         ctx->opt->run_data, nt - 1);
      
      for(i = 0; i < nt; i++)
        ctx->opt->stop_func(ctx->opt->stop_data, i);
      
      //      fprintf(stderr, "done\n");

      /* Second step */
      ctx->offset = &ctx->offset2;
#if 0
      fprintf(stderr, "Second direction\n");
      dump_offset(ctx->offset);
#endif
      ctx->src = ctx->buffer;
      ctx->src_stride = ctx->buffer_stride;
      ctx->dst_size = ctx->dst_rect.w;
      ctx->dst_frame = dst;
      
      nt = ctx->opt->num_threads;
      if(nt > ctx->dst_rect.h)
        nt = ctx->dst_rect.h;
      
      delta = ctx->dst_rect.h / nt;

      scanline = 0;
      for(i = 0; i < nt - 1; i++)
        {
        ctx->opt->run_func(func_2_of_2, ctx, scanline, scanline+delta, ctx->opt->run_data, i);
        
        //        fprintf(stderr, "scanline: %d (%d)\n", ctx->scanline, ctx->dst_rect.h);
        scanline += delta;
        }
      ctx->opt->run_func(func_2_of_2, ctx, scanline, ctx->dst_rect.h,
                         ctx->opt->run_data, nt - 1);
      
      for(i = 0; i < nt; i++)
        ctx->opt->stop_func(ctx->opt->stop_data, i);
      //      fprintf(stderr, "done\n");
      break;
    }
  }

#ifdef HAVE_MMX
#define EMMS                       \
  if(ctx->need_emms)               \
    {                              \
    __asm__ __volatile__ ("emms"); \
    ctx->need_emms = 0;            \
    }
#else
#define EMMS
#endif


void gavl_video_scale_context_scale(gavl_video_scale_context_t * ctx,
                                    const gavl_video_frame_t * src,
                                    gavl_video_frame_t * dst)
  {
  uint8_t * dst_save;
  int i;
  
  if(ctx->opt->num_threads > 1)
    {
    scale_context_scale_mt(ctx, src, dst);
    return;
    }
  
  //  fprintf(stderr, "gavl_video_scale_context_scale, plane %d\n",
  //          ctx->dst_frame_plane);
  switch(ctx->num_directions)
    {
    case 1:
      /* Only step */
      ctx->src = src->planes[ctx->src_frame_plane] + ctx->offset->src_offset;
      ctx->src_stride = src->strides[ctx->src_frame_plane];

      dst_save = dst->planes[ctx->dst_frame_plane] + ctx->offset->dst_offset;
            
      for(i = 0; i < ctx->dst_rect.h; i++)
        {
        //        fprintf(stderr, "scanline: %d (%d)\n", ctx->scanline, ctx->dst_rect.h);
        ctx->func1(ctx, i, dst_save);
        dst_save += dst->strides[ctx->dst_frame_plane];
        }
      EMMS
      break;
    case 2:
      /* First step */
      ctx->offset = &ctx->offset1;
      
      ctx->src = src->planes[ctx->src_frame_plane] +
        ctx->offset->src_offset +
        src->strides[ctx->src_frame_plane] * ctx->first_scanline;
      
      ctx->src_stride = src->strides[ctx->src_frame_plane];
      ctx->dst_size = ctx->buffer_width;

      dst_save = ctx->buffer;
#if 0
      fprintf(stderr, "First direction\n");
      dump_offset(ctx->offset);
#endif
      
      for(i = 0; i < ctx->buffer_height; i++)
        {
        ctx->func1(ctx, i, dst_save);
        dst_save += ctx->buffer_stride;
        }
      EMMS
      //      fprintf(stderr, "done\n");

      /* Second step */
      ctx->offset = &ctx->offset2;
#if 0
      fprintf(stderr, "Second direction\n");
      dump_offset(ctx->offset);
#endif
      ctx->src = ctx->buffer;
      ctx->src_stride = ctx->buffer_stride;
      ctx->dst_size = ctx->dst_rect.w;
      dst_save =
        dst->planes[ctx->dst_frame_plane] + ctx->offset->dst_offset;

      for(i = 0; i < ctx->dst_rect.h; i++)
        {
        //        fprintf(stderr, "i: %d\n", ctx->scanline);
        ctx->func2(ctx, i, dst_save);
        dst_save += dst->strides[ctx->dst_frame_plane];
        }
      EMMS
      //      fprintf(stderr, "done\n");
      break;
    }
  }

