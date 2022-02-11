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
#include <stdio.h>
#include <float.h>
#include <math.h>

#include <gavl/gavl.h>
#include <scale.h>

#define BLUR_THRESHOLD 5.0e-3

// #define DUMP_TABLE

/* Conversion between src and dst coordinates */

#define DST_TO_SRC(c) ((double)c)/scale_factor+src_off

#define ROUND(val) (val >= 0.0) ? (int)(val+0.5):(int)(val-0.5)

static void shift_borders(gavl_video_scale_table_t * tab, int src_width);

static void normalize_table(gavl_video_scale_table_t * tab);

static void alloc_table(gavl_video_scale_table_t * tab,
                        int num_pixels);

static void check_clip(gavl_video_scale_table_t * tab);

static void get_preblur_coeffs(double scale_factor,
                               gavl_video_options_t * opt,
                               int * num_ret,
                               float ** coeffs_ret);

static void convolve_preblur(float * src_1, int src_len_1,
                             float * src_2, int src_len_2,
                             float * dst);

/*
 * Creation of the scale tables, the most ugly part.
 * The tables are for one dimension only, for 2D scaling, we'll have 2 tables.
 * 
 * We have 3 values: src_size (double), src_off (double)
 * and dst_size (int).
 *
 * The scale factor is dst_size / src_size. src_off is the coordinate of the
 * first destination pixel in source coordinates.
 *
 * When src_offset is zero, we'll have a pattern like this
 * (for upscaling by a factor of 2):
 *
 * src: | x     | x     |
 * dst: | x | x | x | x |
 * 
 * For a src_offset of -0.25, we'll have
 *
 * src: |   x   |   x   |
 * dst: | x | x | x | x |
 *
 * The table is created in the follwing steps:
 *
 * - Calculate the floating point filter coefficiens for each destination pixel
 *   
 * - Shift the indixes of the source pixels such that no source pixels are outside
 *   the image. The nonexistant pixels beyond the borders (whose values are needed
 *   for scaling) are assumed to have the same color as the nearest pixel at the
 *   image border
 *
 * - Convert the floating point coefficients to integer with the specified bit
 *   resolution requested by the scaling routine.
 *
 */
                    
void gavl_video_scale_table_init(gavl_video_scale_table_t * tab,
                                 gavl_video_options_t * opt,
                                 double src_off, double src_size,
                                 int dst_size, int src_width)
  {
  int widen;

  double t;
  int i, j, src_index_min, src_index_nearest;
  double src_index_f;
  float widen_factor;
  float * preblur_factors = NULL;
  int num_preblur_factors = 0;
  int num_tmp_factors = 0;
  float * tmp_factors = NULL;
  
  gavl_video_scale_get_weight weight_func;
  
  double scale_factor;
  scale_factor = (double)(dst_size) / src_size;

  widen = 0;

  if(!dst_size)
    return;
  
  if(scale_factor < 1.0)
    {
    switch(opt->downscale_filter)
      {
      case GAVL_DOWNSCALE_FILTER_AUTO: //!< Auto selection based on quality
        if(opt->quality < 2)
          break;
        else
          {
          get_preblur_coeffs(scale_factor,
                             opt,
                             &num_preblur_factors,
                             &preblur_factors);
          }
        break;
      case GAVL_DOWNSCALE_FILTER_NONE: //!< Fastest method, might produce heavy aliasing artifacts
        break;
      case GAVL_DOWNSCALE_FILTER_WIDE: //!< Widen the filter curve according to the scaling ratio. 
        if(opt->downscale_blur > BLUR_THRESHOLD)
          widen = 1;
        break;
      case GAVL_DOWNSCALE_FILTER_GAUSS: //!< Do a Gaussian preblur
        get_preblur_coeffs(scale_factor,
                           opt,
                           &num_preblur_factors,
                           &preblur_factors);
        break;
      }
    }
  
  //  src_off = -0.25;
  
  /* Get the kernel generator */

  weight_func =
    gavl_video_scale_get_weight_func(opt, &tab->factors_per_pixel);

  num_tmp_factors = tab->factors_per_pixel;
  
  if(num_preblur_factors)
    {
    tmp_factors = malloc(sizeof(*tmp_factors) * num_tmp_factors);
    
    tab->factors_per_pixel += num_preblur_factors - 1;
    }
  
  if(widen)
    {
    widen_factor = ceil(opt->downscale_blur / scale_factor);
    tab->factors_per_pixel *= (int)(widen_factor);
    }
  else
    widen_factor = 1.0;

  
  //  fprintf(stderr, "tab->factors_per_pixel: %d, src_width: %d\n",
  //          tab->factors_per_pixel, src_width);
  
  
  if(tab->factors_per_pixel > src_width)
    {
    switch(src_width)
      {
      case 1:
        opt->scale_mode = GAVL_SCALE_NEAREST;
        //        fprintf(stderr, "gavl: Changing scale mode to nearest (image too small)\n");
        break;
      case 2:
      case 3:
        opt->scale_mode = GAVL_SCALE_BILINEAR;
        //        fprintf(stderr, "gavl: Changing scale mode to bilinear (image too small)\n");
        break;
      default:
        opt->scale_mode = GAVL_SCALE_CUBIC_BSPLINE;
        //        fprintf(stderr, "gavl: Changing scale mode to bspline (image too small)\n");
        break;
      }
    weight_func =
      gavl_video_scale_get_weight_func(opt, &tab->factors_per_pixel);
    }
  
  //  fprintf(stderr, "gavl_video_scale_table_init: %f %f %d %d\n", 
  //          src_off, src_size, dst_size, tab->factors_per_pixel);
  
  /* (Re)allocate memory */

  alloc_table(tab, dst_size);
 
  
  for(i = 0; i < dst_size; i++)
    {

    /* src_index_f:       Fractional source index */
    /* src_index_nearest: Nearest integer source index */
    
    src_index_f = DST_TO_SRC((double)i);

    //    fprintf(stderr, "dst: %d -> src: %f (offset: s: %.2f)\n", i, src_index_f,
    //            src_off);
    
    //    if(src_index_f > src_size - 1.0)
    //      src_index_f = src_size - 1.0;
    
    src_index_nearest = ROUND(src_index_f);

    tab->pixels[i].index = src_index_nearest - tab->factors_per_pixel/2;
    src_index_min = src_index_nearest - num_tmp_factors/2;
#if 0
    if(((double)src_index_nearest < src_index_f) && !(tab->factors_per_pixel % 2))
      {
      src_index_min++;
      tab->pixels[i].index++;
      }
#endif
    //    fprintf(stderr, "src_index_f: %f, src_index_nearest: %d, src_index_min: %d, pixel.index: %d, dst_index: %d\n",
    //            src_index_f, src_index_nearest, src_index_min,
    //            tab->pixels[i].index,
    //            i);
    
    /* For nearest neighbour, we don't need any factors */
    if(tab->factors_per_pixel == 1)
      {
      if(tab->pixels[i].index < 0)
        tab->pixels[i].index = 0;
      if(tab->pixels[i].index > src_width - 1)
        tab->pixels[i].index = src_width - 1;
      continue;
      }
    
    /* Normalized distance of the destination pixel to the first source pixel
       in src coordinates */    

    if(num_preblur_factors)
      {
      t = (src_index_f - src_index_min)/widen_factor;
      for(j = 0; j < num_tmp_factors; j++)
        {
        tmp_factors[j] = weight_func(opt, t);
        //        fprintf(stderr, "j: %d, t: %f, w: %f\n", j, t, weight_func(opt, t));
        t -= 1.0;
        }
      
      convolve_preblur(tmp_factors, num_tmp_factors,
                       preblur_factors, num_preblur_factors,
                       tab->pixels[i].factor_f);
      }
    else
      {
      t = (src_index_f - tab->pixels[i].index)/widen_factor;
      for(j = 0; j < tab->factors_per_pixel; j++)
        {
        tab->pixels[i].factor_f[j] = weight_func(opt, t);
        //      fprintf(stderr, "j: %d, t: %f, w: %f\n", j, t, weight_func(opt, t));
        t -= 1.0 /widen_factor;
        }
      }
#if 0
    for(j = 0; j < tab->factors_per_pixel; j++)
      {
      fprintf(stderr, "%d %f\n", j,
              tab->pixels[i].factor_f[j]);
      }
#endif
    }

  //  fprintf(stderr, "Before shift\n");
  //  gavl_video_scale_table_dump(tab);
  
  shift_borders(tab, src_width);
  // if(opt->scale_mode == GAVL_SCALE_SINC_LANCZOS)
  normalize_table(tab);
  
  check_clip(tab);

  if(tmp_factors)
    free(tmp_factors);
  if(preblur_factors)
    free(preblur_factors);
    
  //  fprintf(stderr, "After shift %d\n", src_width);
  //if(deinterlace || (total_fields == 2))
#ifdef DUMP_TABLE
  gavl_video_scale_table_dump(tab);
#endif  
  }

void 
gavl_video_scale_table_init_convolve(gavl_video_scale_table_t * tab,
                                     gavl_video_options_t * opt,
                                     int num_coeffs, const float * coeffs,
                                     int size)
  {
  int i, j;
  tab->factors_per_pixel = num_coeffs * 2 + 1;
  alloc_table(tab, size);
  
  for(i = 0; i < size; i++)
    {
    tab->pixels[i].index = i - num_coeffs;
    for(j = 0; j < tab->factors_per_pixel; j++)
      tab->pixels[i].factor_f[j] = coeffs[j];
    }

  shift_borders(tab, size);

  if(opt->conversion_flags & GAVL_CONVOLVE_NORMALIZE)
    normalize_table(tab);   
  else
    tab->normalized = 0;
  
  check_clip(tab);

  //  gavl_video_scale_table_dump(tab);
  }

static void alloc_table(gavl_video_scale_table_t * tab,
                        int dst_size)
  {
  int i;
  if(tab->pixels_alloc < dst_size)
    {
    tab->pixels_alloc = dst_size + 128;
    tab->pixels = realloc(tab->pixels, tab->pixels_alloc * sizeof(*(tab->pixels)));
    }

  tab->num_pixels = dst_size;
  
  if(tab->factors_alloc < dst_size * tab->factors_per_pixel)
    {
    tab->factors_alloc =  dst_size * tab->factors_per_pixel + 128;
    tab->factors_f = realloc(tab->factors_f, tab->factors_alloc * sizeof(*(tab->factors_f)));
    tab->factors_i = realloc(tab->factors_i, tab->factors_alloc * sizeof(*(tab->factors_i)));
    }

  for(i = 0; i < dst_size; i++)
    {
    /* Set the factor pointers */
    tab->pixels[i].factor_f = tab->factors_f + i * tab->factors_per_pixel;
    tab->pixels[i].factor_i = tab->factors_i + i * tab->factors_per_pixel;
    }
  }



static void normalize_table(gavl_video_scale_table_t * tab)
  {
  int i, j;
  float sum;
  
  for(i = 0; i < tab->num_pixels; i++)
    {
    sum = 0.0;
    for(j = 0; j < tab->factors_per_pixel; j++)
      sum += tab->pixels[i].factor_f[j];

    /* This is to prevent accidental setting of clipping functions due to
       rounding errors. For usual applications, it should not be noticable
    */
    sum += FLT_EPSILON;
    
    // fprintf(stderr, "sum: %f\n", sum);
    
    for(j = 0; j < tab->factors_per_pixel; j++)
      tab->pixels[i].factor_f[j] /= sum;
    }
  tab->normalized = 1;
  }

/* Shift the borders of the table such that no indices are out of range.
   The values in the pixels beyond the border are assumed to be the same as
   the border pixel. */

/* shift_up() for shift = 2, num = 5
   
   | f0 | f1 | f2 | f3 | f4 |

   ->

   | S  | f3 | f4 |  0 |  0 |

   with S = f0 + f1 + f2
   
*/
   
static void shift_up(float * fac, int num, int shift)
  {
  int j;
  /* Add all out-of-range coefficients up into the first
     coefficient of the new array */
  for(j = 0; j < shift; j++)
    fac[shift] += fac[j];

  /* Move Coefficients to new locations */
  for(j = 0; j < num - shift; j++)
    fac[j] = fac[j + shift];
  
  /* Zero unused entries */
  for(j = num - shift; j < num; j++)
    fac[j] = 0.0;
  }

/* shift_down() for shift = 2, num = 5
   
   | f0 | f1 | f2 | f3 | f4 |

   ->

   | 0  |  0 | f0 | f1 | S  |

   with S = f2 + f3 + f4
   
*/

static void shift_down(float * fac, int num, int shift)
  {
  int j;
  //  fprintf(stderr, "Shift down 1 %d\n", shift);
  /* Add all out-of-range coefficients up into the first
     coefficient of the new array */
  for(j = num - shift; j < num; j++)
    fac[num - shift - 1] += fac[j];

  /* Move Coefficients to new locations */
  for(j = num - 1; j >= shift; j--)
    fac[j] = fac[j-shift];
  
  /* Zero unused entries */
  for(j = 0; j < shift; j++)
    fac[j] = 0.0;
  }

static void shift_borders(gavl_video_scale_table_t * tab, int src_width)
  {
  int i, shift;
  
  for(i = 0; i < tab->num_pixels; i++)
    {
    //    fprintf(stderr, "Shift borders1 %d %d %d\n", tab->pixels[i].index,
    //            tab->factors_per_pixel, src_width);
    if(tab->pixels[i].index < 0)
      {
      shift = -tab->pixels[i].index;
      shift_up(tab->pixels[i].factor_f, tab->factors_per_pixel, shift);
      tab->pixels[i].index = 0;
      }

    if(tab->pixels[i].index + tab->factors_per_pixel > src_width)
      {
      shift = tab->pixels[i].index + tab->factors_per_pixel - src_width;
      
      shift_down(tab->pixels[i].factor_f, tab->factors_per_pixel, shift);
      tab->pixels[i].index = src_width - tab->factors_per_pixel;
      }
    //    fprintf(stderr, "Shift borders2 %d %d %d\n", tab->pixels[i].index,
    //            tab->factors_per_pixel, src_width);
    
    }
  
  }

void gavl_video_scale_table_dump(gavl_video_scale_table_t * tab)
  {
  int i, j;
  float sum;
  fprintf(stderr, "Scale table:\n");
  for(i = 0; i < tab->num_pixels; i++)
    {
    sum = 0.0;
    fprintf(stderr, " dst: %d", i);

    for(j = 0; j < tab->factors_per_pixel; j++)
      {
      //      fprintf(stderr, ", fac[%d]: %f [%d]", tab->pixels[i].index + j,
      //              tab->pixels[i].factor[j].fac_f,
      //              tab->pixels[i].factor[j].fac_i);

      fprintf(stderr, ", fac[%d]: %f (%d) ", tab->pixels[i].index + j,
              tab->pixels[i].factor_f[j], tab->pixels[i].factor_i[j]);
      
      sum += tab->pixels[i].factor_f[j];
      }
    
    fprintf(stderr, ", sum: %f\n", sum);
    }
  }

#if 0
static void gavl_video_scale_table_dump_int(gavl_video_scale_table_t * tab)
  {
  int i, j;
  int sum;
  fprintf(stderr, "Scale table:\n");
  for(i = 0; i < tab->num_pixels; i++)
    {
    sum = 0.0;
    fprintf(stderr, " dst: %d", i);

    for(j = 0; j < tab->factors_per_pixel; j++)
      {
      //      fprintf(stderr, ", fac[%d]: %f [%d]", tab->pixels[i].index + j,
      //              tab->pixels[i].factor[j].fac_f,
      //              tab->pixels[i].factor[j].fac_i);

      fprintf(stderr, ", fac[%d]: %d ", tab->pixels[i].index + j,
              tab->pixels[i].factor_i[j]);
      
      sum += tab->pixels[i].factor_i[j];
      }
    
    fprintf(stderr, ", sum: %d\n", sum);
    }
  }
#endif

void gavl_video_scale_table_init_int(gavl_video_scale_table_t * tab,
                                     int bits)
  {
  int fac_max_i, i, j;
  float fac_max_f, sum_f;
  int sum_i, index;
  int min_index, max_index, fac_i_norm = 0;
  
  //  fac_max_i = (1<<bits) - 1;
  //  fprintf(stderr, "Init int %d\n", bits);
  fac_max_i = (1<<bits);
  fac_max_f = (float)(fac_max_i);

  index = 0;
  
  for(i = 0; i < tab->num_pixels; i++)
    {
    min_index = index;
    max_index = index;
    
    sum_i = 0;
    sum_f = 0.0;
    
    for(j = 0; j < tab->factors_per_pixel; j++)
      {
      tab->factors_i[index] =
        (int)(fac_max_f * tab->factors_f[index]+0.5);
      sum_i += tab->factors_i[index];
      sum_f += tab->factors_f[index];
      
      if(j)
        {
        if(tab->factors_i[index] > tab->factors_i[max_index])
          max_index = index;
        if(tab->factors_i[index] < tab->factors_i[min_index])
          min_index = index;
        }
      index++;
      }
   if(!i)
      fac_i_norm = (int)(sum_f * fac_max_i + 0.5);
    
    if(sum_i > fac_i_norm)
      tab->factors_i[max_index] -= (sum_i - fac_i_norm);
    else if(sum_i < fac_i_norm)
      tab->factors_i[min_index] += (fac_i_norm - sum_i);
    }
  //  gavl_video_scale_table_dump_int(tab);
  }

void gavl_video_scale_table_cleanup(gavl_video_scale_table_t * tab)
  {
  if(tab->pixels)
    free(tab->pixels);
  if(tab->factors_f)
    free(tab->factors_f);
  if(tab->factors_i)
    free(tab->factors_i);
  }

void gavl_video_scale_table_get_src_indices(gavl_video_scale_table_t * tab,
                                            int * start, int * size)
  {
  if(!tab->pixels)
    {
    *start = 0;
    *size = 0;
    }
  else
    {
    *start = tab->pixels[0].index;
    *size = tab->pixels[tab->num_pixels-1].index + tab->factors_per_pixel -
      *start;
    }
  }

void gavl_video_scale_table_shift_indices(gavl_video_scale_table_t * tab,
                                          int shift)
  {
  int i;
  if(!shift)
    return;
  for(i = 0; i < tab->num_pixels; i++)
    {
    tab->pixels[i].index += shift;
    }
  }

static void check_clip(gavl_video_scale_table_t * tab)
  {
  int i, j;
  float sum;

  tab->do_clip = 0;
  
  for(i = 0; i < tab->num_pixels; i++)
    {
    sum = 0.0;
    for(j = 0; j < tab->factors_per_pixel; j++)
      {
      sum += tab->pixels[i].factor_f[j];
      
      if(tab->pixels[i].factor_f[j] < 0.0 ||
         tab->pixels[i].factor_f[j] > 1.0)
        {
        tab->do_clip = 1;
        return;
        }
      }
    if(sum > 1.0)
      {
      tab->do_clip = 1;
      return;
      }
    }
  }

static void get_preblur_coeffs(double scale_factor,
                               gavl_video_options_t * opt,
                               int * num_ret,
                               float ** coeffs_ret)
  {
  *num_ret = 0;

  /* Gaussian lowpass */
  if(opt->downscale_filter == GAVL_DOWNSCALE_FILTER_GAUSS)
    {
    int i;
    float tmp;
    float f_co = 0.25 * scale_factor;
    int n = (int)(ceil(0.398 / f_co));

    if(n && (opt->downscale_blur >= BLUR_THRESHOLD))
      {
      *num_ret = 2 * n + 1;
      *coeffs_ret = malloc(*num_ret * sizeof(**coeffs_ret));
      
      for(i = -n; i <= n; i++)
        {
        tmp = 3.011 * f_co * (float)i / opt->downscale_blur;
        (*coeffs_ret)[i+n] = exp(-M_PI * tmp * tmp);
        }
      }
    }
  
  if(!(*num_ret))
    *coeffs_ret = NULL;
  
  }

static void convolve_preblur(float * src_1, int src_len_1,
                             float * src_2, int src_len_2,
                             float * dst)
  {
  int m, n, nmax;
  
  nmax = src_len_1 + src_len_2 - 1;
  
  for(n = 0; n < nmax; n++)
    {
    dst[n] = 0.0;

    for(m = 0; m < src_len_1; m++)
      {
      if((n - m >= 0) && (n - m < src_len_2))
        dst[n] += src_1[m] * src_2[n-m];
      }
    
    }
  
  
  }
