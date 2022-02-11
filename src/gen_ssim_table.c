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

/* Compile with -lm */

#define SSIM_GAUSS_TAPS 11
#define SSIM_GAUSS_DEV  1.5

static void dump_array(double * arr, int start, int end)
  {
  int i;
  double tmp;
  double arr_norm[SSIM_GAUSS_TAPS];

  /* Normalize array */
  tmp = 0.0;
  for(i = start; i <= end; i++)
    tmp += arr[i];
  for(i = start; i <= end; i++)
    arr_norm[i] = arr[i] / tmp;

  /* Dump array */
  printf("    { ");

  /* Nonzero numbers come first */
  for(i = start; i <= end; i++)
    printf("%.16f, ", arr_norm[i]);
  
  for(i = 0; i < start; i++)
    printf("%.16f, ", 0.0);
  for(i = end+1; i < SSIM_GAUSS_TAPS; i++)
    printf("%.16f, ", 0.0);
  printf("},\n");
  }

int main(int argc, char ** argv)
  {
  int i, j;
  double tmp;
  double arr[SSIM_GAUSS_TAPS];

  /* Make unnormalized array */
  for(i = 0; i < SSIM_GAUSS_TAPS; i++)
    {
    tmp = (double)(i - SSIM_GAUSS_TAPS/2) / SSIM_GAUSS_DEV;
    tmp *= -0.5 * tmp;
    arr[i] = exp(tmp);
    }

  printf("#define SSIM_GAUSS_TAPS %d\n", SSIM_GAUSS_TAPS);
  printf("static const double ssim_gauss_coeffs[%d][%d] = \n\
  {\n", SSIM_GAUSS_TAPS, SSIM_GAUSS_TAPS);
  
  for(i = 0; i < SSIM_GAUSS_TAPS / 2; i++)
    {
    dump_array(arr, SSIM_GAUSS_TAPS/2 - i, SSIM_GAUSS_TAPS-1);
    }
  dump_array(arr, 0, SSIM_GAUSS_TAPS-1);

  for(i = 0; i < SSIM_GAUSS_TAPS / 2; i++)
    {
    dump_array(arr, 0, SSIM_GAUSS_TAPS - 2 - i);
    }

  printf("  };\n");
  
  return 0;
  }
