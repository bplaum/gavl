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

#include <gavl/gavl.h>
#include <video.h>
#include <transform.h>
#include <stdio.h>
#include <accel.h>


static gavl_transform_scanline_func get_func(gavl_transform_funcs_t * tab,
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
      return tab->transform_rgb_15;
      break;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
      *bits = tab->bits_rgb_16;
      return tab->transform_rgb_16;
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
      *bits = tab->bits_uint8_noadvance;
      return tab->transform_uint8_x_3;
      break;
    case GAVL_RGB_32:
    case GAVL_BGR_32:
      *bits = tab->bits_uint8_noadvance;
      return tab->transform_uint8_x_3;
      break;
    case GAVL_GRAYA_16:
      *bits = tab->bits_uint8_noadvance;
      return tab->transform_uint8_x_2;
      break;
    case GAVL_GRAY_16:
      *bits = tab->bits_uint16_x_1;
      return tab->transform_uint16_x_1;
      break;
    case GAVL_GRAYA_32:
      *bits = tab->bits_uint16_x_2;
      return tab->transform_uint16_x_2;
      break;
    case GAVL_YUVA_32:
    case GAVL_RGBA_32:
      *bits = tab->bits_uint8_noadvance;
      return tab->transform_uint8_x_4;
      break;
    case GAVL_YUY2:
      *bits = tab->bits_uint8_advance;
      return tab->transform_uint8_x_1_advance;
      break;
    case GAVL_UYVY:
      *bits = tab->bits_uint8_advance;
      return tab->transform_uint8_x_1_advance;
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
      return tab->transform_uint8_x_1_noadvance;
      break;
    case GAVL_YUV_444_P_16:
    case GAVL_YUV_422_P_16:
      *bits = tab->bits_uint16_x_1;
      return tab->transform_uint16_x_1;
      break;
    case GAVL_RGB_48:
      *bits = tab->bits_uint16_x_3;
      return tab->transform_uint16_x_3;
      break;
    case GAVL_RGBA_64:
    case GAVL_YUVA_64:
      *bits = tab->bits_uint16_x_4;
      return tab->transform_uint16_x_4;
      break;
    case GAVL_GRAY_FLOAT:
      *bits = 0;
      return tab->transform_float_x_1;
      break;
    case GAVL_GRAYA_FLOAT:
      *bits = 0;
      return tab->transform_float_x_2;
      break;
    case GAVL_YUV_FLOAT:
    case GAVL_RGB_FLOAT:
      *bits = 0;
      return tab->transform_float_x_3;
      break;
    case GAVL_RGBA_FLOAT:
    case GAVL_YUVA_FLOAT:
      *bits = 0;
      return tab->transform_float_x_4;
      break;
    }
  return NULL;
  }


static void init_func_tab(gavl_video_options_t * opt,
                          gavl_transform_context_t * ctx,
                          gavl_transform_funcs_t * func_tab)
  {
  if(!opt->quality)
    memset(func_tab, 0, sizeof(*func_tab));
  
  switch(ctx->tab.factors_per_pixel)
    {
    case 1:
      if((opt->quality > 0) || (opt->accel_flags & GAVL_ACCEL_C))
        gavl_init_transform_funcs_nearest_c(func_tab, ctx->advance);
      break;
    case 2:
      if((opt->quality > 0) || (opt->accel_flags & GAVL_ACCEL_C))
        gavl_init_transform_funcs_bilinear_c(func_tab, ctx->advance);
#ifdef HAVE_MMX
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMX))
        gavl_init_transform_funcs_bilinear_mmx(func_tab, ctx->advance);
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMXEXT))
        gavl_init_transform_funcs_bilinear_mmxext(func_tab, ctx->advance);
      
#endif
      break;
    case 3:
      if((opt->quality > 0) || (opt->accel_flags & GAVL_ACCEL_C))
        gavl_init_transform_funcs_quadratic_c(func_tab, ctx->advance);
#ifdef HAVE_MMX
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMX))
        gavl_init_transform_funcs_quadratic_mmx(func_tab, ctx->advance);
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMXEXT))
        gavl_init_transform_funcs_quadratic_mmxext(func_tab, ctx->advance);
#endif
      break;
    case 4:
      if((opt->quality > 0) || (opt->accel_flags & GAVL_ACCEL_C))
        gavl_init_transform_funcs_bicubic_c(func_tab, ctx->advance);
#ifdef HAVE_MMX
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMX))
        gavl_init_transform_funcs_bicubic_mmx(func_tab, ctx->advance);
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMXEXT))
        gavl_init_transform_funcs_bicubic_mmxext(func_tab, ctx->advance);
#endif
      break;
    default:
      fprintf(stderr, "BUG: Filter taps > 4 in image transform\n");
      break;
    }
  }

int
gavl_transform_context_init(gavl_image_transform_t * t,
                            gavl_video_options_t * opt,
                            int field_index, int plane_index,
                            gavl_image_transform_func func, void * priv)
  {
  gavl_transform_funcs_t func_tab;
  int bits = 0;
  
  float off_x, off_y;
  float scale_x, scale_y;
  int sub_h, sub_v;
  gavl_transform_context_t * ctx;
  ctx = &t->contexts[field_index][plane_index];
  
  if(field_index == 1)
    {
    ctx->field = 1;
    ctx->num_fields = 2;
    }
  else
    {
    ctx->field = 0;
    if((t->format.interlace_mode == GAVL_INTERLACE_TOP_FIRST) ||
       (t->format.interlace_mode == GAVL_INTERLACE_BOTTOM_FIRST))
      ctx->num_fields = 2;
    else
      ctx->num_fields = 1;
    }
  if((t->format.pixelformat == GAVL_YUY2) ||
     (t->format.pixelformat == GAVL_UYVY))
    ctx->plane = 0;
  else
    ctx->plane = plane_index;
  
  gavl_pixelformat_get_offset(t->format.pixelformat,
                              plane_index,
                              &ctx->advance,
                              &ctx->offset);

  ctx->dst_width  = t->format.image_width;
  ctx->dst_height = t->format.image_height;

  scale_x = 1.0;
  scale_y = 1.0;

  ctx->dst_height /= ctx->num_fields;
  scale_y *= ctx->num_fields;

  off_x = 0.5;
  off_y = 0.5 + (float)(ctx->field);
  //  off_x = 0.0;
  //  off_y = (float)(ctx->field);
  
  if(plane_index)
    {
    gavl_pixelformat_chroma_sub(t->format.pixelformat,
                                &sub_h, &sub_v);
    ctx->dst_width  /= sub_h;
    ctx->dst_height /= sub_v;
    scale_x *= sub_h;
    scale_y *= sub_v;

    if((sub_h == 2) && (sub_v == 2))
      {
      switch(t->format.chroma_placement)
        {
        case GAVL_CHROMA_PLACEMENT_DEFAULT:
          off_x += 0.5;
          off_y += 0.5;
          break;
        case GAVL_CHROMA_PLACEMENT_MPEG2:
          if(ctx->num_fields == 1)
            off_y += 0.5;
          else if(ctx->field == 0) /* Top field */
            off_y += 0.5; /* In FRAME coordinates */
          else /* Bottom field */
            off_y += 1.5; /* In FRAME coordinates */
          break;
        case GAVL_CHROMA_PLACEMENT_DVPAL:
          if(ctx->plane == 1) /* Cb */
            off_y += 2.0; /* In FRAME coordinates */
          break;
        }
      }
    }

  gavl_transform_table_init(&ctx->tab, opt,
                            func, priv,
                            off_x, off_y, scale_x,
                            scale_y,
                            ctx->dst_width, ctx->dst_height);

  /* Get function */

  init_func_tab(opt, ctx, &func_tab);

  ctx->func = get_func(&func_tab,
                       t->format.pixelformat,
                       &bits);

  if(!ctx->func)
    return 0;
  
  /* Now we know the bits, convert to int */
  if(bits)
    gavl_transform_table_init_int(&ctx->tab,
                                  bits, ctx->dst_width, ctx->dst_height);
  return 1;
  }

static void func_1(void* p, int start, int end)
  {
  int i;
  uint8_t * dst_save;
  int dst_stride;
  
  gavl_transform_context_t * ctx = p;
  dst_stride =
    ctx->dst_frame->strides[ctx->plane] *
    ctx->num_fields;
  
  dst_save = ctx->dst_frame->planes[ctx->plane] +
    ctx->offset + ctx->field * ctx->dst_frame->strides[ctx->plane] +
    start * dst_stride;
  
  for(i = start; i < end; i++)
    {
    ctx->func(ctx, ctx->tab.pixels[i], dst_save);
    dst_save += dst_stride;
    }
#ifdef HAVE_MMX
  if(ctx->need_emms)
    {
    __asm__ __volatile__ ("emms");
    }
#endif
  }


void gavl_transform_context_transform(gavl_transform_context_t * ctx,
                                 const gavl_video_frame_t * src,
                                 gavl_video_frame_t * dst)
  {
  int i;
  /* Things set while transforming */
  //  uint8_t * src; /* Beginning of plane */
  //  int src_stride;
  
  //  uint8_t * dst; /* Current scanline */
  //  int dst_stride;
  
  ctx->src = src->planes[ctx->plane] +
    ctx->offset + ctx->field * src->strides[ctx->plane];
  
  ctx->src_stride = src->strides[ctx->plane] * ctx->num_fields;
  
  if(ctx->opt->num_threads > 1)
    {
    int delta;
    int scanline;
    int nt;
    
    ctx->dst_frame = dst;
    nt = ctx->opt->num_threads;
    if(nt > ctx->dst_height)
      nt = ctx->dst_height;
    
    delta = ctx->dst_height / nt;
    scanline = 0;
    for(i = 0; i < nt - 1; i++)
      {
      ctx->opt->run_func(func_1, ctx, scanline,
                         scanline+delta, ctx->opt->run_data, i);
      scanline += delta;
      }
    ctx->opt->run_func(func_1, ctx, scanline, ctx->dst_height,
                       ctx->opt->run_data, nt - 1);
    
    for(i = 0; i < nt; i++)
      ctx->opt->stop_func(ctx->opt->stop_data, i);
    }
  else
    {
    int dst_stride;
    uint8_t * dst_save;

    dst_save = dst->planes[ctx->plane] +
      ctx->offset + ctx->field * dst->strides[ctx->plane];
    
    dst_stride = dst->strides[ctx->plane] * ctx->num_fields;
    for(i = 0; i < ctx->dst_height; i++)
      {
      ctx->func(ctx, ctx->tab.pixels[i], dst_save);
      dst_save += dst_stride;
      }
#ifdef HAVE_MMX
    if(ctx->need_emms)
      {
      __asm__ __volatile__ ("emms");
      // fprintf(stderr, "emms");
      }
#endif
    }
  
  }

void
gavl_transform_context_free(gavl_transform_context_t * ctx)
  {
  gavl_transform_table_free(&ctx->tab);
  }
