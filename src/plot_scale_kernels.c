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
#include <stdio.h>

#include <gavl/gavl.h>
#include "../include/scale.h"

static struct
  {
  char * name;
  gavl_scale_mode_t mode;
  int order;
  gavl_video_scale_get_weight func;
  }
scale_modes[] = 
  {
    { "Nearest",                  GAVL_SCALE_NEAREST },
    { "Bilinear",                 GAVL_SCALE_BILINEAR },
    { "Quadratic",                GAVL_SCALE_QUADRATIC },
    { "Cubic B-Spline",           GAVL_SCALE_CUBIC_BSPLINE },
    { "Cubic Mitchel",            GAVL_SCALE_CUBIC_MITCHELL },
    { "Cubic Catmull",            GAVL_SCALE_CUBIC_CATMULL },
    { "Sinc Lanczos (4th order)", GAVL_SCALE_SINC_LANCZOS, 4 },
    { "Sinc Lanczos (5th order)", GAVL_SCALE_SINC_LANCZOS, 5 },
  };

static int num_scale_modes = sizeof(scale_modes)/sizeof(scale_modes[0]);

#define DATA_NAME "kernels.dat"
#define COMMAND_NAME "kernels.gnu"

#define TOTAL_POINTS 1000

int main(int argc, char ** argv)
  {
  FILE * data_file;
  FILE * command_file;
  
  float t, tmax;
  int i, j, num_points, num_points_test;

  gavl_video_options_t opt;
  memset(&opt, 0, sizeof(opt));

  /* Open output files */

  data_file = fopen(DATA_NAME, "w");
  command_file = fopen(COMMAND_NAME, "w");
  
  /* Get the scale functions */

  num_points = 0;
  for(i = 0; i < num_scale_modes; i++)
    {
    opt.scale_mode = scale_modes[i].mode;
    opt.scale_order = scale_modes[i].order;
    
    scale_modes[i].func = gavl_video_scale_get_weight_func(&opt, &num_points_test);
    if(num_points < num_points_test)
      num_points = num_points_test;
    }

  tmax = num_points/2 + 0.5;

  fprintf(command_file, "plot ");
  
  for(j = 0; j < TOTAL_POINTS; j++)
    {
    t = (float)(j)/(float)(TOTAL_POINTS-1) * tmax;

    fprintf(data_file, "%f ", t);
    
    for(i = 0; i < num_scale_modes; i++)
      {
      opt.scale_mode = scale_modes[i].mode;
      opt.scale_order = scale_modes[i].order;

      fprintf(data_file, "%f", scale_modes[i].func(&opt, t));
      fprintf(command_file, "\"%s\" using 1:%d title \"%s\" with lines", DATA_NAME, i+2,
              scale_modes[i].name);

      if(i == num_scale_modes -1)
        {
        fprintf(data_file, "\n");
        fprintf(command_file, "\n");
        }
      else
        {
        fprintf(data_file, " ");
        fprintf(command_file, ", ");
        }
      }
    }
  fclose(data_file);
  fclose(command_file);

  printf("Created scale kernels plot. To watch it,\n");
  printf("type\n\nload \"%s\"\n\nfrom within gnuplot\n", COMMAND_NAME);

  
  return 0;
  }
