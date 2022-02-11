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
#include <math.h>

#include <config.h>

#include <gavl/gavl.h>
#include <scale.h>
#include <accel.h>

// #define DUMP_TABLES

#define SCALE_ACTION_NOOP  0
#define SCALE_ACTION_COPY  1
#define SCALE_ACTION_SCALE 2

/* Public functions */

void gavl_video_scaler_destroy(gavl_video_scaler_t * s)
  {
  int i, j;

  gavl_video_frame_null(s->src);
  gavl_video_frame_null(s->dst);

  gavl_video_frame_destroy(s->src);
  gavl_video_frame_destroy(s->dst);

  for(i = 0; i < 3; i++)
    {
    for(j = 0; j < GAVL_MAX_PLANES; j++)
      {
      gavl_video_scale_context_cleanup(&s->contexts[i][j]);
      }
    }
  free(s);  
  }

gavl_video_scaler_t * gavl_video_scaler_create()
  {
  gavl_video_scaler_t * ret;
  int i, j;
  ret = calloc(1, sizeof(*ret));

  ret->src = gavl_video_frame_create(NULL);
  ret->dst = gavl_video_frame_create(NULL);

  gavl_video_options_set_defaults(&ret->opt);

  for(i = 0; i < 3; i++)
    {
    for(j = 0; j < GAVL_MAX_PLANES; j++)
      ret->contexts[i][j].opt = &ret->opt;
    }
  
  return ret;
  }

void gavl_init_scale_funcs(gavl_scale_funcs_t * tab,
                           gavl_video_options_t * opt,
                           int src_advance, int dst_advance,
                           gavl_video_scale_table_t * tab_h,
                           gavl_video_scale_table_t * tab_v)
  {
  gavl_video_scale_table_t * scale_table;

  memset(tab, 0, sizeof(*tab));
  /* Only nearest is faster for x && y */
  if(tab_h && tab_v)
    {
    if((tab_h->factors_per_pixel == 1) &&
       (tab_v->factors_per_pixel == 1) &&
       ((opt->quality > 0) || (opt->accel_flags & GAVL_ACCEL_C)))
      gavl_init_scale_funcs_nearest_c(tab, src_advance, dst_advance);
    return;
    }
  
  scale_table = (tab_h ? tab_h : tab_v);

  if(scale_table->factors_per_pixel < 1)
    return;
  
  /* Get scale functions */
  switch(scale_table->factors_per_pixel)
    {
    case 1:
      if((opt->quality > 0) || (opt->accel_flags & GAVL_ACCEL_C))
        gavl_init_scale_funcs_nearest_c(tab, src_advance, dst_advance);
      break;
    case 2:
      if((opt->quality > 0) || (opt->accel_flags & GAVL_ACCEL_C))
        {
        if(scale_table->do_clip)
          gavl_init_scale_funcs_bilinear_c(tab);
        else
          {
          gavl_init_scale_funcs_bilinear_noclip_c(tab);
          if(scale_table->normalized)
            gavl_init_scale_funcs_bilinear_fast_c(tab);
          }
        }
#ifdef HAVE_MMX
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMX))
        {
        gavl_init_scale_funcs_bilinear_y_mmx(tab, src_advance, dst_advance);
        gavl_init_scale_funcs_bilinear_x_mmx(tab, src_advance, dst_advance);
        }
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMXEXT))
        {
        gavl_init_scale_funcs_bilinear_y_mmxext(tab, src_advance, dst_advance);
        gavl_init_scale_funcs_bilinear_x_mmxext(tab, src_advance, dst_advance);
        }
#endif
#ifdef HAVE_SSE
      if(opt->accel_flags & GAVL_ACCEL_SSE)
        {
        gavl_init_scale_funcs_bilinear_y_sse(tab, src_advance, dst_advance);
        gavl_init_scale_funcs_bilinear_x_sse(tab);
        }
#endif
#ifdef HAVE_SSE2
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_SSE2))
        {
        gavl_init_scale_funcs_bilinear_y_sse2(tab, src_advance, dst_advance);
        //        gavl_init_scale_funcs_bilinear_x_sse2(tab, src_advance, dst_advance);
        }
#endif
      break;
    case 3:
      if((opt->quality > 0) || (opt->accel_flags & GAVL_ACCEL_C))
        {
        if(scale_table->do_clip)
          gavl_init_scale_funcs_quadratic_c(tab);
        else
          gavl_init_scale_funcs_quadratic_noclip_c(tab);
        }
#ifdef HAVE_MMX
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMX))
        {
        gavl_init_scale_funcs_quadratic_y_mmx(tab, src_advance, dst_advance);
        gavl_init_scale_funcs_quadratic_x_mmx(tab, src_advance, dst_advance);
        }
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMXEXT))
        {
        gavl_init_scale_funcs_quadratic_y_mmxext(tab, src_advance, dst_advance);
        gavl_init_scale_funcs_quadratic_x_mmxext(tab, src_advance, dst_advance);
        }
#endif
#ifdef HAVE_SSE
      if(opt->accel_flags & GAVL_ACCEL_SSE)
        {
        gavl_init_scale_funcs_quadratic_y_sse(tab, src_advance, dst_advance);
        gavl_init_scale_funcs_quadratic_x_sse(tab);
        }
#endif
#ifdef HAVE_SSE2
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_SSE2))
        {
        gavl_init_scale_funcs_quadratic_y_sse2(tab, src_advance, dst_advance);
        //        gavl_init_scale_funcs_quadratic_x_sse2(tab, src_advance, dst_advance);
        }
#endif
      break;
    case 4:
      if(!scale_table->do_clip)
        {
        if((opt->quality > 0) || (opt->accel_flags & GAVL_ACCEL_C))
          gavl_init_scale_funcs_bicubic_noclip_c(tab);
#ifdef HAVE_MMX
        if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMX))
          {
          gavl_init_scale_funcs_bicubic_y_mmx(tab, src_advance, dst_advance);
          gavl_init_scale_funcs_bicubic_noclip_x_mmx(tab, src_advance, dst_advance);
          }
        if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMXEXT))
          {
          gavl_init_scale_funcs_bicubic_y_mmxext(tab, src_advance, dst_advance);
          gavl_init_scale_funcs_bicubic_noclip_x_mmxext(tab, src_advance, dst_advance);
          }
#endif
#ifdef HAVE_SSE
        if(opt->accel_flags & GAVL_ACCEL_SSE)
          {
          gavl_init_scale_funcs_bicubic_y_noclip_sse(tab, src_advance, dst_advance);
          gavl_init_scale_funcs_bicubic_x_noclip_sse(tab);
          }
#endif
#ifdef HAVE_SSE2
        if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_SSE2))
          {
          gavl_init_scale_funcs_bicubic_y_noclip_sse2(tab, src_advance, dst_advance);
          //        gavl_init_scale_funcs_bicubic_x_sse2(tab, src_advance, dst_advance);
          }
#endif
#ifdef HAVE_SSE3
        if(opt->accel_flags & GAVL_ACCEL_SSE3)
          {
          gavl_init_scale_funcs_bicubic_x_noclip_sse3(tab);
          }
#endif
        }
      else
        {
        if((opt->quality > 0) || (opt->accel_flags & GAVL_ACCEL_C))
          gavl_init_scale_funcs_bicubic_c(tab);
#ifdef HAVE_MMX
        if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMX))
          {
          gavl_init_scale_funcs_bicubic_y_mmx(tab, src_advance, dst_advance);
          gavl_init_scale_funcs_bicubic_x_mmx(tab, src_advance, dst_advance);
          }
        if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMXEXT))
          {
          gavl_init_scale_funcs_bicubic_y_mmxext(tab, src_advance, dst_advance);
          gavl_init_scale_funcs_bicubic_x_mmxext(tab, src_advance, dst_advance);
          }
#endif
#ifdef HAVE_SSE
        if(opt->accel_flags & GAVL_ACCEL_SSE)
          {
          gavl_init_scale_funcs_bicubic_y_sse(tab, src_advance, dst_advance);
          gavl_init_scale_funcs_bicubic_x_sse(tab);
          }
#endif
#ifdef HAVE_SSE2
        if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_SSE2))
          {
          gavl_init_scale_funcs_bicubic_y_sse2(tab, src_advance, dst_advance);
          //        gavl_init_scale_funcs_bicubic_x_sse2(tab, src_advance, dst_advance);
          }
#endif
#ifdef HAVE_SSE3
        if(opt->accel_flags & GAVL_ACCEL_SSE3)
          {
          gavl_init_scale_funcs_bicubic_x_sse3(tab);
          }
#endif
        }
      break;
    default:
      if((opt->quality > 0) || (opt->accel_flags & GAVL_ACCEL_C))
        gavl_init_scale_funcs_generic_c(tab);
#ifdef HAVE_MMX
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMX))
        {
        gavl_init_scale_funcs_generic_y_mmx(tab, src_advance, dst_advance);
        gavl_init_scale_funcs_generic_x_mmx(tab, src_advance, dst_advance);
        }
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_MMXEXT))
        {
        gavl_init_scale_funcs_generic_y_mmxext(tab, src_advance, dst_advance);
        gavl_init_scale_funcs_generic_x_mmxext(tab, src_advance, dst_advance);
        }
#endif
#ifdef HAVE_SSE
      if(opt->accel_flags & GAVL_ACCEL_SSE)
        {
        gavl_init_scale_funcs_generic_y_sse(tab, src_advance, dst_advance);
        gavl_init_scale_funcs_generic_x_sse(tab);
        }
#endif
#ifdef HAVE_SSE2
      if((opt->quality < 3) && (opt->accel_flags & GAVL_ACCEL_SSE2))
        {
        gavl_init_scale_funcs_generic_y_sse2(tab, src_advance, dst_advance);
        //        gavl_init_scale_funcs_generic_x_sse2(tab, src_advance, dst_advance);
        }
#endif
#ifdef HAVE_SSE3
      if(opt->accel_flags & GAVL_ACCEL_SSE3)
        {
        gavl_init_scale_funcs_generic_x_sse3(tab);
        }
#endif
      break;
    }
  }


int gavl_video_scaler_init(gavl_video_scaler_t * scaler,
                           const gavl_video_format_t * src_format,
                           const gavl_video_format_t * dst_format)
  {
  gavl_rectangle_f_t src_rect;
  gavl_rectangle_i_t  dst_rect;
  gavl_video_options_t opt;

  int field, plane;
 
  int sub_h_out = 1, sub_v_out = 1;
  
  /* Copy options because we want to change them */

  gavl_video_options_copy(&opt, &scaler->opt);

  /* TODO: If the image is smaller than the number of filter taps,
     reduce scaling algorithm */
  
  /* Copy formats */
  
  gavl_video_format_copy(&scaler->src_format, src_format);
  gavl_video_format_copy(&scaler->dst_format, dst_format);
  
  /* Check if we have rectangles */

  if(!opt.have_rectangles)
    {
    gavl_rectangle_f_set_all(&src_rect, &scaler->src_format);
    gavl_rectangle_i_set_all(&dst_rect, &scaler->dst_format);
    gavl_video_options_set_rectangles(&opt, &src_rect, &dst_rect);
    }
  
  /* Check how many fields we must handle */

  if((opt.deinterlace_mode == GAVL_DEINTERLACE_SCALE) &&
     (opt.conversion_flags & GAVL_FORCE_DEINTERLACE))
    {
    /* Deinterlacing mode */
    scaler->src_fields = 2;
    scaler->dst_fields = 1;

    /* Fake formats for scale context */
    if(scaler->src_format.interlace_mode == GAVL_INTERLACE_NONE)
      scaler->src_format.interlace_mode = GAVL_INTERLACE_TOP_FIRST;
    scaler->dst_format.interlace_mode = GAVL_INTERLACE_NONE;
    }
  else if((opt.deinterlace_mode == GAVL_DEINTERLACE_SCALE) &&
          (scaler->dst_format.interlace_mode == GAVL_INTERLACE_NONE) &&
          (scaler->src_format.interlace_mode != GAVL_INTERLACE_NONE))
    {
    /* Deinterlacing mode */
    scaler->src_fields = 2;
    scaler->dst_fields = 1;
    }
  else if(scaler->src_format.interlace_mode != GAVL_INTERLACE_NONE)
    {
    /* Interlaced scaling */
    scaler->src_fields = 2;
    scaler->dst_fields = 2;
    }
  else
    {
    /* Progressive scaling */
    scaler->src_fields = 1;
    scaler->dst_fields = 1;
    }
  
  /* Copy destination rectangle so we know, which subframe to take */
  gavl_rectangle_i_copy(&scaler->dst_rect, &opt.dst_rect);
  
#if 0
  fprintf(stderr, "gavl_video_scaler_init:\n");
  gavl_rectangle_f_dump(&scaler->opt.src_rect);
  fprintf(stderr, "\n");
  gavl_rectangle_i_dump(&scaler->dst_rect);
  fprintf(stderr, "\n");
#endif                      
  
  /* Crop source and destination rectangles to the formats */

  
  
  /* Align the destination rectangle to the output formtat */

  gavl_pixelformat_chroma_sub(scaler->dst_format.pixelformat, &sub_h_out, &sub_v_out);
  gavl_rectangle_i_align(&opt.dst_rect, sub_h_out, sub_v_out);
  
#if 0
  fprintf(stderr, "Initializing scaler:\n");
  fprintf(stderr, "Src format:\n");
  gavl_video_format_dump(&scaler->src_format);
  fprintf(stderr, "Dst format:\n");
  gavl_video_format_dump(&scaler->dst_format);

  fprintf(stderr, "Src rectangle:\n");
  gavl_rectangle_f_dump(&opt.src_rect);
  fprintf(stderr, "\nDst rectangle:\n");
  gavl_rectangle_i_dump(&scaler->dst_rect);
  fprintf(stderr, "\n");
#endif
  
  /* Check how many planes we have */
  
  if((scaler->src_format.pixelformat == GAVL_YUY2) ||
     (scaler->src_format.pixelformat == GAVL_UYVY))
    scaler->num_planes = 3;
  else
    scaler->num_planes = gavl_pixelformat_num_planes(scaler->src_format.pixelformat);
  
  if((scaler->src_fields == 2) && (!scaler->src_field))
    scaler->src_field = gavl_video_frame_create(NULL);
  
  if((scaler->dst_fields == 2) && (!scaler->dst_field))
    scaler->dst_field = gavl_video_frame_create(NULL);
  
  
#if 0
  fprintf(stderr, "src_fields: %d, dst_fields: %d, planes: %d\n",
          scaler->src_fields, scaler->dst_fields, scaler->num_planes);
#endif    

  /* Handle automatic mode selection */

  if(opt.scale_mode == GAVL_SCALE_AUTO)
    {
    if(opt.quality < 2)
      opt.scale_mode = GAVL_SCALE_NEAREST;
    else if(opt.quality <= 3)
      opt.scale_mode = GAVL_SCALE_BILINEAR;
    else
      opt.scale_mode = GAVL_SCALE_CUBIC_BSPLINE;
    }
  
  
  /* Now, initialize all fields and planes */

  if(scaler->src_fields > scaler->dst_fields)
    {
    /* Deinterlace mode */
    field = (scaler->opt.deinterlace_drop_mode == GAVL_DEINTERLACE_DROP_BOTTOM) ? 0 : 1;
    for(plane = 0; plane < scaler->num_planes; plane++)
      {
      if(!gavl_video_scale_context_init(&scaler->contexts[field][plane],
                                    &opt,
                                    plane, &scaler->src_format, &scaler->dst_format, field, 0,
                                    scaler->src_fields, scaler->dst_fields))
        return 0;
      }
    if(scaler->src_format.interlace_mode == GAVL_INTERLACE_MIXED)
      {
      for(plane = 0; plane < scaler->num_planes; plane++)
        {
        if(!gavl_video_scale_context_init(&scaler->contexts[2][plane],
                                          &opt,
                                          plane, &scaler->src_format, &scaler->dst_format, 0, 0, 1, 1))
          return 0;
        }
      }
    }
  else
    {
    /* src_fields == dst_fields */
    for(field = 0; field < scaler->src_fields; field++)
      {
      for(plane = 0; plane < scaler->num_planes; plane++)
        {
        if(!gavl_video_scale_context_init(&scaler->contexts[field][plane],
                                          &opt,
                                          plane, &scaler->src_format, &scaler->dst_format, field, field,
                                          scaler->src_fields, scaler->dst_fields))
          return 0;
        }
      }

    if(scaler->src_format.interlace_mode == GAVL_INTERLACE_MIXED)
      {
      for(plane = 0; plane < scaler->num_planes; plane++)
        {
        if(!gavl_video_scale_context_init(&scaler->contexts[2][plane],
                                          &opt,
                                          plane, &scaler->src_format, &scaler->dst_format, 0, 0, 1, 1))
          return 0;
        }
      }
    }
  return 1;
  }


int gavl_video_scaler_init_convolve(gavl_video_scaler_t * scaler,
                                    const gavl_video_format_t * format,
                                    int h_radius, const float * h_coeffs,
                                    int v_radius, const float * v_coeffs)
  {
  gavl_rectangle_f_t src_rect;
  gavl_rectangle_i_t  dst_rect;
  gavl_video_options_t opt;

  int field, plane;
 
  /* Copy options because we want to change them */

  gavl_video_options_copy(&opt, &scaler->opt);
  
  /* Copy formats */
  
  gavl_video_format_copy(&scaler->src_format, format);
  gavl_video_format_copy(&scaler->dst_format, format);
  
  gavl_rectangle_f_set_all(&src_rect, &scaler->src_format);
  gavl_rectangle_i_set_all(&dst_rect, &scaler->dst_format);
  gavl_video_options_set_rectangles(&opt, &src_rect, &dst_rect);
    
  /* Check how many fields we must handle */

  if(format->interlace_mode != GAVL_INTERLACE_NONE)
    {
    scaler->src_fields = 2;
    scaler->dst_fields = 2;
    }
  else
    {
    scaler->src_fields = 1;
    scaler->dst_fields = 1;
    }
  
  /* Copy destination rectangle so we know, which subframe to take */
  gavl_rectangle_i_copy(&scaler->dst_rect, &opt.dst_rect);
  
  /* Check how many planes we have */
  
  if((scaler->src_format.pixelformat == GAVL_YUY2) ||
     (scaler->src_format.pixelformat == GAVL_UYVY))
    scaler->num_planes = 3;
  else
    scaler->num_planes = 
      gavl_pixelformat_num_planes(scaler->src_format.pixelformat);
  
  if((scaler->src_fields == 2) && (!scaler->src_field))
    scaler->src_field = gavl_video_frame_create(NULL);
  
  if((scaler->dst_fields == 2) && (!scaler->dst_field))
    scaler->dst_field = gavl_video_frame_create(NULL);
  
  /* Now, initialize all fields and planes */
  
  for(field = 0; field < scaler->src_fields; field++)
    {
    for(plane = 0; plane < scaler->num_planes; plane++)
      {
      gavl_video_scale_context_init_convolve(&scaler->contexts[field][plane],
                                             &opt,
                                             plane, format, 
                                             scaler->src_fields,
                                             h_radius, h_coeffs,
                                             v_radius, v_coeffs);
      }
    
    if(scaler->src_format.interlace_mode == GAVL_INTERLACE_MIXED)
      {
      for(plane = 0; plane < scaler->num_planes; plane++)
        {
        gavl_video_scale_context_init_convolve(&scaler->contexts[2][plane],
                                               &opt,
                                               plane, format, 
                                               1,
                                               h_radius, h_coeffs,
                                               v_radius, v_coeffs);
        }
      }
    
    }
  return 1;
  }

gavl_video_options_t * 
gavl_video_scaler_get_options(gavl_video_scaler_t * s)
  {
  return &s->opt;
  }

void gavl_video_scaler_scale(gavl_video_scaler_t * s,
                             const gavl_video_frame_t * src,
                             gavl_video_frame_t * dst)
  {
  int i, field;
  /* Set the destination subframe */
  gavl_video_frame_get_subframe(s->dst_format.pixelformat, dst, s->dst, 
                                &s->dst_rect);
#if 0
  fprintf(stderr, "Get subframe\n");
  gavl_rectangle_i_dump(&s->dst_rect);
  fprintf(stderr, "\n");
#endif
  if(s->src_fields > s->dst_fields)
    {
    /* Progressive scaling for mixed mode */
    if((s->src_format.interlace_mode == GAVL_INTERLACE_MIXED) &&
       (src->interlace_mode == GAVL_INTERLACE_NONE) &&
       !(s->opt.conversion_flags & GAVL_FORCE_DEINTERLACE))
      {
      for(i = 0; i < s->num_planes; i++)
        gavl_video_scale_context_scale(&s->contexts[2][i], src, s->dst);
      }
    else /* Deinterlace mode */
      {
      field = (s->opt.deinterlace_drop_mode == GAVL_DEINTERLACE_DROP_BOTTOM) ? 0 : 1;
      gavl_video_frame_get_field(s->src_format.pixelformat, src, 
                                 s->src_field, field);
      
      for(i = 0; i < s->num_planes; i++)
        gavl_video_scale_context_scale(&s->contexts[field][i], 
                                       s->src_field, s->dst);
      }
    }
  else if(s->src_fields == 2)
    {
    /* Progressive scaling for mixed mode */
    if((s->src_format.interlace_mode == GAVL_INTERLACE_MIXED) &&
       (src->interlace_mode == GAVL_INTERLACE_NONE) &&
       !(s->opt.conversion_flags & GAVL_FORCE_DEINTERLACE))
      {
      for(i = 0; i < s->num_planes; i++)
        gavl_video_scale_context_scale(&s->contexts[2][i], src, s->dst);
      }
    else
      {
      /* First field */
      gavl_video_frame_get_field(s->src_format.pixelformat, src,    s->src_field, 0);
      gavl_video_frame_get_field(s->dst_format.pixelformat, s->dst, s->dst_field, 0);
      for(i = 0; i < s->num_planes; i++)
        {
        //      fprintf(stderr, "Field: 0, plane: %d\n", i);
        gavl_video_scale_context_scale(&s->contexts[0][i], s->src_field, s->dst_field);
        }
      
      /* Second field */
      gavl_video_frame_get_field(s->src_format.pixelformat, src,    s->src_field, 1);
      gavl_video_frame_get_field(s->dst_format.pixelformat, s->dst, s->dst_field, 1);
      for(i = 0; i < s->num_planes; i++)
        {
        //      fprintf(stderr, "Field: 1, plane: %d\n", i);
        gavl_video_scale_context_scale(&s->contexts[1][i], s->src_field, s->dst_field);
        }
      }
    }
  else
    {
    for(i = 0; i < s->num_planes; i++)
      {
      //      fprintf(stderr, "Scale %d, %p\n", i, &s->contexts[0][i]);
      //      fprintf(stderr, "Field: 0 (progressive), plane: %d\n", i);
      gavl_video_scale_context_scale(&s->contexts[0][i], src, s->dst);
      }
    }
  }
