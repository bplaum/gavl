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

#include <gavl/gavl.h>
#include <video.h>
#include <transform.h>
#include <scale.h>

#define ROUND(val) (val >= 0.0) ? (int)(val+0.5):(int)(val-0.5)

/* 1234 */
/*  S340  */
/*   S400 */

static void shift_right(gavl_transform_pixel_t * p, int factors_per_pixel,
                        int delta)
  {
  int i, j;
  for(i = 0; i < factors_per_pixel; i++)
    {
    /* Sum up */
    for(j = 1; j < delta; j++)
      p->factors[i][0] += p->factors[i][j];
    /* Shift */
    for(j = 1; j < factors_per_pixel - delta; j++)
      p->factors[i][j] = p->factors[i][j + delta];
    /* Clear */
    for(j = factors_per_pixel - delta; j < factors_per_pixel; j++)
      p->factors[i][j] = 0.0;
    }
  p->index_x += delta;
  }

/*   1234 */
/* 001S */

static void shift_left(gavl_transform_pixel_t * p, int factors_per_pixel,
                       int delta)
  {
  int i, j;

  for(i = 0; i < factors_per_pixel; i++)
    {
    /* Sum up */
    for(j = factors_per_pixel - delta; j < factors_per_pixel; j++)
      p->factors[i][factors_per_pixel - 1 - delta] += p->factors[i][j];
    /* Shift */
 
    for(j = factors_per_pixel - 1; j >= delta; j--)
      p->factors[i][j] = p->factors[i][j - delta];
    /* Clear */
    for(j = 0; j < delta; j++)
      p->factors[i][j] = 0.0;
    }
  p->index_x -= delta;
  }

static void shift_down(gavl_transform_pixel_t * p, int factors_per_pixel,
                       int delta)
  {
  int i, j;

  for(i = 0; i < factors_per_pixel; i++)
    {
    /* Sum up */
    for(j = 1; j < delta; j++)
      p->factors[0][i] += p->factors[j][i];
    /* Shift */
    for(j = 1; j < factors_per_pixel - delta; j++)
      p->factors[j][i] = p->factors[j + delta][i];
    /* Clear */
    for(j = factors_per_pixel - delta; j < factors_per_pixel; j++)
      p->factors[j][i] = 0.0;
    
    }
  p->index_y += delta;
  }

static void shift_up(gavl_transform_pixel_t * p, int factors_per_pixel,
                     int delta)
  {
  int i, j;

  for(i = 0; i < factors_per_pixel; i++)
    {
    /* Sum up */
    for(j = factors_per_pixel - delta; j < factors_per_pixel; j++)
      p->factors[factors_per_pixel - 1 - delta][i] += p->factors[j][i];
    /* Shift */
 
    for(j = factors_per_pixel - 1; j >= delta; j--)
      p->factors[j][i] = p->factors[j - delta][i];
    /* Clear */
    for(j = 0; j < delta; j++)
      p->factors[j][i] = 0.0;
    }
  p->index_y -= delta;
  }

static void shift_borders(gavl_transform_table_t * tab,
                          int width, int i, int height)
  {
  int j;

  for(j = 0; j < width; j++)
    {
    /* Check left overshot */
    if(tab->pixels[i][j].index_x < 0)
      shift_right(&tab->pixels[i][j], tab->factors_per_pixel,
                  -tab->pixels[i][j].index_x);
      
    /* Check right overshot */
    if(tab->pixels[i][j].index_x + tab->factors_per_pixel > width)
      shift_left(&tab->pixels[i][j], tab->factors_per_pixel,
                 tab->pixels[i][j].index_x +
                 tab->factors_per_pixel - width);
      
    /* Check top overshot */
    if(tab->pixels[i][j].index_y < 0)
      shift_down(&tab->pixels[i][j], tab->factors_per_pixel,
                 -tab->pixels[i][j].index_y);
      
    /* Check bottom overshot */
    if(tab->pixels[i][j].index_y + tab->factors_per_pixel > height)
      shift_up(&tab->pixels[i][j], tab->factors_per_pixel,
               tab->pixels[i][j].index_y +
               tab->factors_per_pixel - height);
    }
  }

static void normalize(gavl_transform_table_t * tab,
                      int width, int i)
  {
  int j, k, l;
  float sum;
  
  for(j = 0; j < width; j++)
    {
    sum = 0.0;
    for(k = 0; k < tab->factors_per_pixel; k++)
      {
      for(l = 0; l < tab->factors_per_pixel; l++)
        {
        sum += tab->pixels[i][j].factors[k][l];
        }
      }
    for(k = 0; k < tab->factors_per_pixel; k++)
      {
      for(l = 0; l < tab->factors_per_pixel; l++)
        tab->pixels[i][j].factors[k][l] /= sum;
      }
    }
  }

typedef struct
  {
  float off_x;
  float off_y;
  float scale_x, scale_y;
  int width;
  int height;
  gavl_image_transform_func func;
  gavl_video_scale_get_weight weight_func;
  gavl_transform_table_t * tab;
  void * func_priv;
  gavl_video_options_t * opt;
  } slice_data_t;

static void init_slice(void* p, int start, int end)
  {
  int i, j, k, l;
  slice_data_t * sd = p;

  double x_src_f, y_src_f, x_dst_f, y_dst_f;
  int x_src_nearest, y_src_nearest;
  double t;
  double w_x[MAX_TRANSFORM_FILTER];
  double w_y;
  
  for(i = start; i < end; i++)
    {
    y_dst_f = sd->scale_y * (double)i + sd->off_y;
    for(j = 0; j < sd->width; j++)
      {
      x_dst_f = sd->scale_x * (double)j + sd->off_x;
      
      sd->func(sd->func_priv, x_dst_f, y_dst_f, &x_src_f, &y_src_f);

      //      fprintf(stderr, "Transform: src: %f %f -> dst: %f %f\n",
      //              x_src_f, y_src_f, x_dst_f, y_dst_f);

      //      x_src_f = (x_src_f - sd->off_x) / sd->scale_x;
      //      y_src_f = (y_src_f - sd->off_y) / sd->scale_y;

      x_src_f = (x_src_f) / sd->scale_x;
      y_src_f = (y_src_f) / sd->scale_y;
      
      if((x_src_f < 0.0) || (x_src_f > (double)sd->width) || 
         (y_src_f < 0.0) || (y_src_f > (double)sd->height))
        {
        sd->tab->pixels[i][j].outside = 1;
        //        fprintf(stderr, "Outside\n");
        continue;
        }

      /* ... */
      x_src_nearest = ROUND(x_src_f);
      y_src_nearest = ROUND(y_src_f);

      sd->tab->pixels[i][j].index_x =
        x_src_nearest - sd->tab->factors_per_pixel/2;
      sd->tab->pixels[i][j].index_y =
        y_src_nearest - sd->tab->factors_per_pixel/2;
      
      if(sd->tab->factors_per_pixel == 1)
        {
        if(sd->tab->pixels[i][j].index_x < 0)
          sd->tab->pixels[i][j].index_x = 0;
        if(sd->tab->pixels[i][j].index_x > sd->width - 1)
          sd->tab->pixels[i][j].index_x = sd->width - 1;
        if(sd->tab->pixels[i][j].index_y < 0)
          sd->tab->pixels[i][j].index_y = 0;
        if(sd->tab->pixels[i][j].index_y > sd->height - 1)
          sd->tab->pixels[i][j].index_y = sd->height - 1;
        continue;
        }
      
      /* x weights */
      t = (x_src_f - 0.5 - sd->tab->pixels[i][j].index_x);
      //      t = (x_src_f - sd->tab->pixels[i][j].index_x);
      for(k = 0; k < sd->tab->factors_per_pixel; k++)
        {
        //        fprintf(stderr, "%d w_x(%f)\n", k, t);
        
        w_x[k] = sd->weight_func(sd->opt, t);


        t -= 1.0;
        }

      t = (y_src_f - 0.5 - sd->tab->pixels[i][j].index_y);
      //      t = (y_src_f - sd->tab->pixels[i][j].index_y);
      
      for(k = 0; k < sd->tab->factors_per_pixel; k++)
        {
        /* Y-weight */
        w_y = sd->weight_func(sd->opt, t);
        
        //        fprintf(stderr, "%d w_y(%f)\n", k, t);
        
        for(l = 0; l < sd->tab->factors_per_pixel; l++)
          sd->tab->pixels[i][j].factors[k][l] = w_y * w_x[l];
        t -= 1.0;
        }
      
      }
    shift_borders(sd->tab, sd->width, i, sd->height);
    normalize(sd->tab, sd->width, i);
    }
  
  }
     
void gavl_transform_table_init(gavl_transform_table_t * tab,
                               gavl_video_options_t * opt,
                               gavl_image_transform_func func, void * priv,
                               float off_x, float off_y, float scale_x,
                               float scale_y, int width, int height)
  {
  int delta;
  int scanline;
  int nt;
  int i;
  
  slice_data_t sd;

  sd.off_x = off_x;
  sd.off_y = off_y;
  sd.scale_x = scale_x;
  sd.scale_y = scale_y;
  sd.width = width;
  sd.height = height;
  sd.tab = tab;
  sd.func = func;
  sd.func_priv = priv;
  sd.opt = opt;
  
  /* (re)alloc */
  
  gavl_transform_table_free(tab);
  
  
  /* Get factors per pixel and filter_func */
  sd.weight_func =
    gavl_video_scale_get_weight_func(opt, &tab->factors_per_pixel);
  
  if(tab->factors_per_pixel > MAX_TRANSFORM_FILTER)
    {
    fprintf(stderr, "BUG: tab->factors_per_pixel > MAX_TRANSFORM_FILTER\n");
    return;
    }
  
  tab->pixels = malloc(height * sizeof(*tab->pixels));
  tab->pixels[0] = calloc(width * height, sizeof(**tab->pixels));
  
  for(i = 1; i < height; i++)
    tab->pixels[i] = tab->pixels[0] + i * width;

  nt = opt->num_threads;
  if(nt > height)
    nt = height;
  if(nt < 1)
    nt = 1;
  
  delta = height / nt;
  scanline = 0;

  for(i = 0; i < nt - 1; i++)
    {
    opt->run_func(init_slice, &sd, scanline, scanline+delta,
                  opt->run_data, i);
        
    scanline += delta;
    }
  opt->run_func(init_slice, &sd, scanline, height,
                opt->run_data, nt - 1);
  
  for(i = 0; i < nt; i++)
    opt->stop_func(opt->stop_data, i);
  }

void gavl_transform_table_init_int(gavl_transform_table_t * tab,
                                   int bits, int width, int height)
  {
  int fac_max_i, i, j, k, l;
  float fac_max_f, sum_f;
  int sum_i;
  int min_index_k, max_index_k;
  int min_index_l, max_index_l;
  
  //  fac_max_i = (1<<bits) - 1;
  //  fprintf(stderr, "Init int %d\n", bits);
  fac_max_i = (1<<bits);
  fac_max_f = (float)(fac_max_i);
  
  for(i = 0; i < height; i++)
    {
    for(j = 0; j < width; j++)
      {
      //      if(tab->pixels[i][j].outside)
      //       continue;

      if(tab->pixels[i][j].outside)
        continue;
      
      min_index_k = 0;
      max_index_k = 0;
      min_index_l = 0;
      max_index_l = 0;
      
      sum_i = 0;
      sum_f = 0.0;
      
      for(k = 0; k < tab->factors_per_pixel; k++)
        {
        for(l = 0; l < tab->factors_per_pixel; l++)
          {
          tab->pixels[i][j].factors_i[k][l] =
            (int)(fac_max_f * tab->pixels[i][j].factors[k][l]+0.5);
          sum_i += tab->pixels[i][j].factors_i[k][l];
          sum_f += tab->pixels[i][j].factors[k][l];

          if(k || l)
            {
            if(tab->pixels[i][j].factors_i[k][l] >
               tab->pixels[i][j].factors_i[max_index_k][max_index_l])
              {
              max_index_k = k;
              max_index_l = l;
              }
            if(tab->pixels[i][j].factors_i[k][l] <
               tab->pixels[i][j].factors_i[min_index_k][min_index_l])
              {
              min_index_k = k;
              min_index_l = l;
              }
            }
          }
        }
      
      if(sum_i > fac_max_i)
        tab->pixels[i][j].factors_i[max_index_k][max_index_l]
          -= (sum_i - fac_max_i);
      else if(sum_i < fac_max_i)
        tab->pixels[i][j].factors_i[min_index_k][min_index_l]
          += (fac_max_i - sum_i);
      //  gavl_video_scale_table_dump_int(tab);
      }
    }
  }

void
gavl_transform_table_free(gavl_transform_table_t * tab)
  {
  if(tab->pixels)
    {
    if(tab->pixels[0])
      free(tab->pixels[0]);
    free(tab->pixels);
    tab->pixels = NULL;
    }
  }
