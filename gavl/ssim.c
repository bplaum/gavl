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
#include <stdio.h>

#include <gavl/gavl.h>

#include "ssim_tab.h"



/* Constants for suppressing instabilities for almost equal images */
static const double K1 = 0.01;
static const double K2 = 0.03;


/*
 *  Note: SSIM calculation is optimized for *accuracy*, not for *speed* !!
 */

/* Range for iterating through a frame */
typedef struct
  {
  int start;
  int len;
  const double * w;
  int coeffs_index;
  } range_t;

/* Wang, eq. 14 */

static double get_mu(range_t * ri, range_t * rj,
                     const float * data, int stride)
  {
  int i, j;
  double ret = 0.0;
  
  for(i = 0; i < ri->len; i++)
    {
    for(j = 0; j < rj->len; j++)
      {
      ret += ri->w[i] * rj->w[j] * data[j];
      }
    data += stride;
    }
  return ret;
  }

/* Wang, eq. 15 */

static double get_sigma(range_t * ri, range_t * rj,
                        const float * data, int stride,
                        double mu)
  {
  int i, j;
  double ret = 0.0;
  double diff;
  
  for(i = 0; i < ri->len; i++)
    {
    for(j = 0; j < rj->len; j++)
      {
      diff = data[j] - mu;
      ret += ri->w[i] * rj->w[j] * diff * diff;
      }
    data += stride;
    }
  return sqrt(ret);
  }

/* Wang, eq. 16 */

static double get_sigma_xy(range_t * ri, range_t * rj,
                           const float * data_x, int stride_x,
                           const float * data_y, int stride_y,
                           double mu_x, double mu_y)
  {
  int i, j;
  double ret = 0.0;
  
  for(i = 0; i < ri->len; i++)
    {
    for(j = 0; j < rj->len; j++)
      {
      ret += ri->w[i]*rj->w[j]*(data_x[j]-mu_x)*(data_y[j]-mu_y);
      }
    data_x += stride_x;
    data_y += stride_y;
    }
  return ret;
  }

static void setup_range(range_t * r, int center, int size)
  {
  int diff;
  
  r->start = center - SSIM_GAUSS_TAPS/2;
  r->len   = SSIM_GAUSS_TAPS;
  
  r->coeffs_index = SSIM_GAUSS_TAPS/2;
  
  if(r->start < 0)
    {
    diff = -r->start;

    r->len -= diff;
    r->coeffs_index -= diff;
    r->start = 0;
    }
  else if(r->start + r->len > size)
    {
    diff = r->start + r->len - size;
    
    r->len -= diff;
    r->coeffs_index += diff;
    }

  r->w = ssim_gauss_coeffs[r->coeffs_index];

  }

int gavl_video_frame_ssim(const gavl_video_frame_t * x,
                          const gavl_video_frame_t * y,
                          gavl_video_frame_t * dst,
                          const gavl_video_format_t * format)
  {
  int i, j;

  double sigma_x, sigma_y, mu_x, mu_y, sigma_xy;
  range_t ri, rj;
  float * dst_ptr;
  
  const float * data_x;
  const float * data_y;
  const float * data_start_x;
  const float * data_start_y;

  /* Dynamic range is 1.0 for our grayscale images */
  const float C1 = K1 * K1;
  const float C2 = K2 * K2;

  int stride_x, stride_y;

  stride_x = x->strides[0] / sizeof(float);
  stride_y = y->strides[0] / sizeof(float);
  
  if(format->pixelformat != GAVL_GRAY_FLOAT)
    return 0;

  if((format->image_width < SSIM_GAUSS_TAPS) ||
     (format->image_height < SSIM_GAUSS_TAPS))
    return 0;
  
  for(i = 0; i < format->image_height; i++)
    {
    dst_ptr = (float*)(dst->planes[0] + i * dst->strides[0]);
    setup_range(&ri, i, format->image_height);
    
    data_start_x = (float*)(x->planes[0] + ri.start * x->strides[0]);
    data_start_y = (float*)(y->planes[0] + ri.start * y->strides[0]);
    
    for(j = 0; j < format->image_width; j++)
      {
      setup_range(&rj, j, format->image_width);

      data_x = data_start_x + rj.start;
      data_y = data_start_y + rj.start;
      
      mu_x = get_mu(&ri, &rj, data_x, stride_x);
      mu_y = get_mu(&ri, &rj, data_y, stride_y);
      sigma_x =  get_sigma(&ri, &rj, data_x, stride_x, mu_x);
      sigma_y =  get_sigma(&ri, &rj, data_y, stride_y, mu_y);
      sigma_xy = get_sigma_xy(&ri, &rj,
                              data_x, stride_x,
                              data_y, stride_y,
                              mu_x, mu_y);
      
      /* Wang, eq. 13 */
      
      *dst_ptr = (2.0*mu_x*mu_y+C1)*(2.0*sigma_xy+C2) /
        ((mu_x*mu_x+mu_y*mu_y+C1)*(sigma_x*sigma_x+sigma_y*sigma_y+C2));
      dst_ptr++;
      }
    }
  
  return 1;
  }
