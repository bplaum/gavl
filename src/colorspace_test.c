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
#include <gavl.h>
//#include "colorspace.h" // Common routines
#include <stdio.h>
#include <png.h>
#include <unistd.h>
#include <stdlib.h>

#include <accel.h>

// #define ALL_PIXELFORMATS
#define IN_PIXELFORMAT GAVL_RGBA_FLOAT
#define OUT_PIXELFORMAT GAVL_YUVA_FLOAT

// Masks for BGR16 and RGB16 formats

#define RGB16_LOWER_MASK  0x001f
#define RGB16_MIDDLE_MASK 0x07e0
#define RGB16_UPPER_MASK  0xf800

// Extract unsigned char RGB values from 15 bit pixels

#define RGB16_TO_R(pixel) ((pixel & RGB16_UPPER_MASK)>>8)
#define RGB16_TO_G(pixel) ((pixel & RGB16_MIDDLE_MASK)>>3) 
#define RGB16_TO_B(pixel) ((pixel & RGB16_LOWER_MASK)<<3)

#define RGB15_LOWER_MASK  0x001f
#define RGB15_MIDDLE_MASK 0x3e0
#define RGB15_UPPER_MASK  0x7C00

#define RGB15_TO_R(pixel) ((pixel & RGB15_UPPER_MASK)>>7)
#define RGB15_TO_G(pixel) ((pixel & RGB15_MIDDLE_MASK)>>2) 
#define RGB15_TO_B(pixel) ((pixel & RGB15_LOWER_MASK)<<3)

// ((((((src[2]<<5)&0xff00)|src[1])<<6)&0xfff00)|src[0])>>3;

#define TEST_PICTURE_WIDTH  320
#define TEST_PICTURE_HEIGHT 256

/*
 *   Some braindead YUV conversion (but works at least :-)
 */

static int * r_to_y = NULL;
static int * g_to_y = NULL;
static int * b_to_y = NULL;

static int * r_to_u = NULL;
static int * g_to_u = NULL;
static int * b_to_u = NULL;

static int * r_to_v = NULL;
static int * g_to_v = NULL;
static int * b_to_v = NULL;

static int * y_to_rgb = NULL;
static int * v_to_r = NULL;
static int * u_to_g = NULL;
static int * v_to_g = NULL;
static int * u_to_b = NULL;

/* JPEG Quantization */

static int * r_to_yj = NULL;
static int * g_to_yj = NULL;
static int * b_to_yj = NULL;

static int * r_to_uj = NULL;
static int * g_to_uj = NULL;
static int * b_to_uj = NULL;

static int * r_to_vj = NULL;
static int * g_to_vj = NULL;
static int * b_to_vj = NULL;

static int * yj_to_rgb = NULL;
static int * vj_to_r = NULL;
static int * uj_to_g = NULL;
static int * vj_to_g = NULL;
static int * uj_to_b = NULL;



static void init_yuv()
  {
  int i;
  
  r_to_y = malloc(0x100 * sizeof(int));
  g_to_y = malloc(0x100 * sizeof(int));
  b_to_y = malloc(0x100 * sizeof(int));

  r_to_u = malloc(0x100 * sizeof(int));
  g_to_u = malloc(0x100 * sizeof(int));
  b_to_u = malloc(0x100 * sizeof(int));

  r_to_v = malloc(0x100 * sizeof(int));
  g_to_v = malloc(0x100 * sizeof(int));
  b_to_v = malloc(0x100 * sizeof(int));

  y_to_rgb = malloc(0x100 * sizeof(int));
  v_to_r   = malloc(0x100 * sizeof(int));
  u_to_g   = malloc(0x100 * sizeof(int));
  v_to_g   = malloc(0x100 * sizeof(int));
  u_to_b   = malloc(0x100 * sizeof(int));

  /* JPEG Quantization */

  r_to_yj = malloc(0x100 * sizeof(int));
  g_to_yj = malloc(0x100 * sizeof(int));
  b_to_yj = malloc(0x100 * sizeof(int));

  r_to_uj = malloc(0x100 * sizeof(int));
  g_to_uj = malloc(0x100 * sizeof(int));
  b_to_uj = malloc(0x100 * sizeof(int));

  r_to_vj = malloc(0x100 * sizeof(int));
  g_to_vj = malloc(0x100 * sizeof(int));
  b_to_vj = malloc(0x100 * sizeof(int));

  yj_to_rgb = malloc(0x100 * sizeof(int));
  vj_to_r   = malloc(0x100 * sizeof(int));
  uj_to_g   = malloc(0x100 * sizeof(int));
  vj_to_g   = malloc(0x100 * sizeof(int));
  uj_to_b   = malloc(0x100 * sizeof(int));
  
  
  for(i = 0; i < 0x100; i++)
    {
    // RGB to YUV conversion

    r_to_y[i] = (int)(0.257*0x10000 * i + 16 * 0x10000);
    g_to_y[i] = (int)(0.504*0x10000 * i);
    b_to_y[i] = (int)(0.098*0x10000 * i);
    
    r_to_u[i] = (int)(-0.148*0x10000 * i);
    g_to_u[i] = (int)(-0.291*0x10000 * i);
    b_to_u[i] = (int)( 0.439*0x10000 * i + 0x800000);
    
    r_to_v[i] = (int)( 0.439*0x10000 * i);
    g_to_v[i] = (int)(-0.368*0x10000 * i);
    b_to_v[i] = (int)(-0.071*0x10000 * i + 0x800000);

    r_to_yj[i] = (int)((0.29900)*0x10000 * i);
    g_to_yj[i] = (int)((0.58700)*0x10000 * i);
    b_to_yj[i] = (int)((0.11400)*0x10000 * i);
    
    r_to_uj[i] = (int)(-(0.16874)*0x10000 * i);
    g_to_uj[i] = (int)(-(0.33126)*0x10000 * i);
    b_to_uj[i] = (int)( (0.50000)*0x10000 * i + 0x800000);
    
    r_to_vj[i] = (int)( (0.50000)*0x10000 * i);
    g_to_vj[i] = (int)(-(0.41869)*0x10000 * i);
    b_to_vj[i] = (int)(-(0.08131)*0x10000 * i + 0x800000);


    // YUV to RGB conversion

    y_to_rgb[i] = (int)(1.164*(i-16)) * 0x10000;
    
    v_to_r[i]   = (int)( 1.596  * (i - 0x80) * 0x10000);
    u_to_g[i]   = (int)(-0.392  * (i - 0x80) * 0x10000);
    v_to_g[i]   = (int)(-0.813  * (i - 0x80) * 0x10000);
    u_to_b[i]   = (int)( 2.017 * (i - 0x80) * 0x10000);

    /* JPEG Quantization */

    yj_to_rgb[i] = (int)(i * 0x10000);
    
    vj_to_r[i]   = (int)( 1.40200 * (i - 0x80) * 0x10000);
    uj_to_g[i]   = (int)(-0.34414 * (i - 0x80) * 0x10000);
    vj_to_g[i]   = (int)(-0.71414 * (i - 0x80) * 0x10000);
    uj_to_b[i]   = (int)( 1.77200 * (i - 0x80) * 0x10000);
    
    }
  }

#define RECLIP(color) (uint8_t)((color>0xFF)?0xff:((color<0)?0:color))

#define RECLIP_16(color) (uint16_t)((color>0xFFFF)?0xFFFF:((color<0)?0:color))

#define RECLIP_FLOAT(color) (float)((color>1.0)?1.0:((color<0.0)?0.0:color))

#define YUV_2_RGB(y,u,v,r,g,b) i_tmp=(y_to_rgb[y]+v_to_r[v])>>16;\
                                r=RECLIP(i_tmp);\
                                i_tmp=(y_to_rgb[y]+u_to_g[u]+v_to_g[v])>>16;\
                                g=RECLIP(i_tmp);\
                                i_tmp=(y_to_rgb[y]+u_to_b[u])>>16;\
                                b=RECLIP(i_tmp);

#define YUVJ_2_RGB(y,u,v,r,g,b) i_tmp=(yj_to_rgb[y]+vj_to_r[v])>>16;\
                                r=RECLIP(i_tmp);\
                                i_tmp=(yj_to_rgb[y]+uj_to_g[u]+vj_to_g[v])>>16;\
                                g=RECLIP(i_tmp);\
                                i_tmp=(yj_to_rgb[y]+uj_to_b[u])>>16;\
                                b=RECLIP(i_tmp);

#define YUV_16_2_RGB(y,u,v,r,g,b) i_tmp=(y_to_rgb[y>>8]+v_to_r[v>>8])>>16;\
                                r=RECLIP(i_tmp);\
                                i_tmp=(y_to_rgb[y>>8]+u_to_g[u>>8]+v_to_g[v>>8])>>16;\
                                g=RECLIP(i_tmp);\
                                i_tmp=(y_to_rgb[y>>8]+u_to_b[u>>8])>>16;\
                                b=RECLIP(i_tmp);

#define Y_F_TO_8(x) ((int)(x * 255.0 +0.5))
#define UV_F_TO_8(x) ((int)((x+0.5) * 255.0 +0.5))

#define YUV_FLOAT_2_RGB(y,u,v,r,g,b) i_tmp=(yj_to_rgb[Y_F_TO_8(y)]+vj_to_r[UV_F_TO_8(v)])>>16;\
                                            r=RECLIP(i_tmp);\
                                            i_tmp=(yj_to_rgb[Y_F_TO_8(y)]+uj_to_g[UV_F_TO_8(u)]+vj_to_g[UV_F_TO_8(v)])>>16;\
                                            g=RECLIP(i_tmp);\
                                            i_tmp=(yj_to_rgb[Y_F_TO_8(y)]+uj_to_b[UV_F_TO_8(u)])>>16;\
                                            b=RECLIP(i_tmp);


static void convert_15_to_24(gavl_video_frame_t * in_frame,
                      gavl_video_frame_t * out_frame,
                      int width, int height)
  {
  int i, j;

  uint16_t * in_pixel;
  uint8_t  * out_pixel;

  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (uint16_t*)in_pixel_save;
    out_pixel = out_pixel_save;
    
    for(j = 0; j < width; j++)
      {
      out_pixel[0] = RGB15_TO_R(*in_pixel);
      out_pixel[1] = RGB15_TO_G(*in_pixel);
      out_pixel[2] = RGB15_TO_B(*in_pixel);
      
      in_pixel++;
      out_pixel += 3;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

static void convert_16_to_24(gavl_video_frame_t * in_frame,
                      gavl_video_frame_t * out_frame,
                      int width, int height)
  {
  int i, j;
  uint16_t * in_pixel;
  uint8_t  * out_pixel;

  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (uint16_t*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      out_pixel[0] = RGB16_TO_R(*in_pixel);
      out_pixel[1] = RGB16_TO_G(*in_pixel);
      out_pixel[2] = RGB16_TO_B(*in_pixel);

      in_pixel++;
      out_pixel += 3;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

static void convert_32_to_24(gavl_video_frame_t * in_frame,
                      gavl_video_frame_t * out_frame,
                      int width, int height)
  {
  int i, j;
  uint8_t * in_pixel;
  uint8_t  * out_pixel;
  
  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (uint8_t*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      out_pixel[0] = in_pixel[0];
      out_pixel[1] = in_pixel[1];
      out_pixel[2] = in_pixel[2];
      in_pixel+=4;
      out_pixel+=3;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

static void convert_48_to_24(gavl_video_frame_t * in_frame,
                      gavl_video_frame_t * out_frame,
                      int width, int height)
  {
  int i, j;
  uint16_t * in_pixel;
  uint8_t  * out_pixel;
  
  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (uint16_t*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      out_pixel[0] = in_pixel[0] >> 8;
      out_pixel[1] = in_pixel[1] >> 8;
      out_pixel[2] = in_pixel[2] >> 8;
      in_pixel+=3;
      out_pixel+=3;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

static void convert_graya_32_to_graya_16(gavl_video_frame_t * in_frame,
                                         gavl_video_frame_t * out_frame,
                                         int width, int height)
  {
  int i, j;
  uint16_t * in_pixel;
  uint8_t  * out_pixel;
  
  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (uint16_t*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      out_pixel[0] = in_pixel[0] >> 8;
      out_pixel[1] = in_pixel[1] >> 8;
      in_pixel+=2;
      out_pixel+=2;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

static void convert_gray_16_to_gray_8(gavl_video_frame_t * in_frame,
                                      gavl_video_frame_t * out_frame,
                                      int width, int height)
  {
  int i, j;
  uint16_t * in_pixel;
  uint8_t  * out_pixel;
  
  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (uint16_t*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      out_pixel[0] = in_pixel[0] >> 8;
      in_pixel++;
      out_pixel++;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

static void convert_gray_float_to_gray_8(gavl_video_frame_t * in_frame,
                                         gavl_video_frame_t * out_frame,
                                         int width, int height)
  {
  int i, j;
  float * in_pixel;
  uint8_t  * out_pixel;
  int i_tmp;
  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (float*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      i_tmp = (int)(in_pixel[0] * 255.0 + 0.5);
      out_pixel[0] = RECLIP(i_tmp);
      in_pixel++;
      out_pixel++;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

static void convert_graya_float_to_graya_16(gavl_video_frame_t * in_frame,
                                           gavl_video_frame_t * out_frame,
                                           int width, int height)
  {
  int i, j;
  float * in_pixel;
  uint8_t  * out_pixel;
  int i_tmp;
  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (float*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      i_tmp = (int)(in_pixel[0] * 255.0 + 0.5);
      out_pixel[0] = RECLIP(i_tmp);
      i_tmp = (int)(in_pixel[1] * 255.0 + 0.5);
      out_pixel[1] = RECLIP(i_tmp);
      in_pixel+=2;
      out_pixel+=2;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }


static void convert_64_to_32(gavl_video_frame_t * in_frame,
                      gavl_video_frame_t * out_frame,
                      int width, int height)
  {
  int i, j;
  uint16_t * in_pixel;
  uint8_t  * out_pixel;
  
  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (uint16_t*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      out_pixel[0] = in_pixel[0] >> 8;
      out_pixel[1] = in_pixel[1] >> 8;
      out_pixel[2] = in_pixel[2] >> 8;
      out_pixel[3] = in_pixel[3] >> 8;
      in_pixel+=4;
      out_pixel+=4;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

static void convert_YUVA_32_to_RGBA_32(gavl_video_frame_t * in_frame,
                      gavl_video_frame_t * out_frame,
                      int width, int height)
  {
  int i, j, i_tmp;
  uint8_t * in_pixel;
  uint8_t  * out_pixel;
  
  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      YUV_2_RGB(in_pixel[0], in_pixel[1], in_pixel[2], out_pixel[0], out_pixel[1], out_pixel[2]);
      out_pixel[3] = in_pixel[3];
      in_pixel+=4;
      out_pixel+=4;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

static void convert_YUVA_64_to_RGBA_32(gavl_video_frame_t * in_frame,
                                       gavl_video_frame_t * out_frame,
                                       int width, int height)
  {
  int i, j, i_tmp;
  uint16_t * in_pixel;
  uint8_t * out_pixel;
  
  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (uint16_t *)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      YUV_16_2_RGB(in_pixel[0], in_pixel[1], in_pixel[2], out_pixel[0], out_pixel[1], out_pixel[2]);
      out_pixel[3] = in_pixel[3] >> 8;
      in_pixel+=4;
      out_pixel+=4;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

static void convert_YUVA_FLOAT_to_RGBA_32(gavl_video_frame_t * in_frame,
                                          gavl_video_frame_t * out_frame,
                                          int width, int height)
  {
  int i, j, i_tmp;
  float * in_pixel;
  uint8_t * out_pixel;
  
  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (float*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      YUV_FLOAT_2_RGB(in_pixel[0], in_pixel[1], in_pixel[2], out_pixel[0], out_pixel[1], out_pixel[2]);
      i_tmp = (int)(in_pixel[3]*255.0);
      out_pixel[3] = RECLIP(i_tmp);
      in_pixel+=4;
      out_pixel+=4;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

static void convert_YUV_FLOAT_to_RGB_24(gavl_video_frame_t * in_frame,
                                        gavl_video_frame_t * out_frame,
                                        int width, int height)
  {
  int i, j, i_tmp;
  float * in_pixel;
  uint8_t * out_pixel;
  
  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (float*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      YUV_FLOAT_2_RGB(in_pixel[0], in_pixel[1], in_pixel[2], out_pixel[0], out_pixel[1], out_pixel[2]);
      in_pixel+=3;
      out_pixel+=3;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }



static void convert_float_to_24(gavl_video_frame_t * in_frame,
                         gavl_video_frame_t * out_frame,
                         int width, int height)
  {
  int i, j, i_tmp;
  float * in_pixel;
  uint8_t  * out_pixel;
  
  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (float*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      i_tmp = (int)(in_pixel[0] * 255.0 + 0.5);
      out_pixel[0] = RECLIP(i_tmp);
      i_tmp = (int)(in_pixel[1] * 255.0 + 0.5);
      out_pixel[1] = RECLIP(i_tmp);
      i_tmp = (int)(in_pixel[2] * 255.0 + 0.5);
      out_pixel[2] = RECLIP(i_tmp);
      in_pixel+=3;
      out_pixel+=3;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

static void convert_float_to_32(gavl_video_frame_t * in_frame,
                         gavl_video_frame_t * out_frame,
                         int width, int height)
  {
  int i, j, i_tmp;
  float * in_pixel;
  uint8_t  * out_pixel;
  
  uint8_t  * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];

  //  fprintf(stderr, "convert_float_to_32: %d %d\n", 
  //           in_frame->strides[0], out_frame->strides[0]);
  for(i = 0; i < height; i++)
    {
    in_pixel = (float*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      i_tmp = (int)(in_pixel[0] * 255.0 + 0.5);
      out_pixel[0] = RECLIP(i_tmp);
      i_tmp = (int)(in_pixel[1] * 255.0 + 0.5);
      out_pixel[1] = RECLIP(i_tmp);
      i_tmp = (int)(in_pixel[2] * 255.0 + 0.5);
      out_pixel[2] = RECLIP(i_tmp);
      i_tmp = (int)(in_pixel[3] * 255.0 + 0.5);
      out_pixel[3] = RECLIP(i_tmp);
      in_pixel+=4;
      out_pixel+=4;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }


static void convert_YUV_420_P_to_RGB24(gavl_video_frame_t * in_frame,
                                gavl_video_frame_t * out_frame,
                                int width, int height)
  {
  int i, j, i_tmp;
  uint8_t * in_y;
  uint8_t * in_u;
  uint8_t * in_v;

  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->planes[0];
  uint8_t * in_y_save = in_frame->planes[0];
  uint8_t * in_u_save = in_frame->planes[1];
  uint8_t * in_v_save = in_frame->planes[2];

  for(i = 0; i < height/2; i++)
    {

    /* Even Rows */

    out_pixel = out_pixel_save;
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    for(j = 0; j < width/2; j++)
      {
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];

    /* Odd Rows */

    out_pixel = out_pixel_save;
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    for(j = 0; j < width/2; j++)
      {
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];

    in_u_save += in_frame->strides[1];
    in_v_save += in_frame->strides[2];
    }
  }

static void convert_YUV_410_P_to_RGB24(gavl_video_frame_t * in_frame,
                                gavl_video_frame_t * out_frame,
                                int width, int height)
  {
  int i, j, i_tmp;
  uint8_t * in_y;
  uint8_t * in_u;
  uint8_t * in_v;

  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->planes[0];
  uint8_t * in_y_save = in_frame->planes[0];
  uint8_t * in_u_save = in_frame->planes[1];
  uint8_t * in_v_save = in_frame->planes[2];

  for(i = 0; i < height/4; i++)
    {

    /* Even Rows */

    out_pixel = out_pixel_save;
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    for(j = 0; j < width/4; j++)
      {
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];

    /* Odd Rows */

    out_pixel = out_pixel_save;
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    for(j = 0; j < width/4; j++)
      {
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];

    out_pixel = out_pixel_save;
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    for(j = 0; j < width/4; j++)
      {
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];

    out_pixel = out_pixel_save;
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    for(j = 0; j < width/4; j++)
      {
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];

    
    in_u_save += in_frame->strides[1];
    in_v_save += in_frame->strides[2];
    }
  }

static void convert_YUVJ_420_P_to_RGB24(gavl_video_frame_t * in_frame,
                                 gavl_video_frame_t * out_frame,
                                 int width, int height)
  {
  int i, j, i_tmp;
  uint8_t * in_y;
  uint8_t * in_u;
  uint8_t * in_v;

  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->planes[0];
  uint8_t * in_y_save = in_frame->planes[0];
  uint8_t * in_u_save = in_frame->planes[1];
  uint8_t * in_v_save = in_frame->planes[2];

  for(i = 0; i < height/2; i++)
    {

    /* Even Rows */

    out_pixel = out_pixel_save;
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    for(j = 0; j < width/2; j++)
      {
      YUVJ_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUVJ_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];

    /* Odd Rows */

    out_pixel = out_pixel_save;
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    for(j = 0; j < width/2; j++)
      {
      YUVJ_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUVJ_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];

    in_u_save += in_frame->strides[1];
    in_v_save += in_frame->strides[2];
    }
  }

static void convert_YUV_422_P_to_RGB24(gavl_video_frame_t * in_frame,
                               gavl_video_frame_t * out_frame,
                               int width, int height)
  {
  int i, j, i_tmp;

  uint8_t * in_y;
  uint8_t * in_u;
  uint8_t * in_v;

  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->planes[0];
  uint8_t * in_y_save = in_frame->planes[0];
  uint8_t * in_u_save = in_frame->planes[1];
  uint8_t * in_v_save = in_frame->planes[2];

  for(i = 0; i < height; i++)
    {
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width/2; j++)
      {
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];
    in_u_save += in_frame->strides[1];
    in_v_save += in_frame->strides[2];
    }
  }

static void convert_YUV_422_P_16_to_RGB24(gavl_video_frame_t * in_frame,
                               gavl_video_frame_t * out_frame,
                               int width, int height)
  {
  int i, j, i_tmp;

  uint16_t * in_y;
  uint16_t * in_u;
  uint16_t * in_v;

  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->planes[0];
  uint8_t * in_y_save = in_frame->planes[0];
  uint8_t * in_u_save = in_frame->planes[1];
  uint8_t * in_v_save = in_frame->planes[2];

  for(i = 0; i < height; i++)
    {
    in_y = (uint16_t*)in_y_save;
    in_u = (uint16_t*)in_u_save;
    in_v = (uint16_t*)in_v_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width/2; j++)
      {
      YUV_2_RGB(((*in_y)>>8), ((*in_u)>>8), ((*in_v)>>8),
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      
      YUV_2_RGB(((*in_y)>>8), ((*in_u)>>8), ((*in_v)>>8),
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];
    in_u_save += in_frame->strides[1];
    in_v_save += in_frame->strides[2];
    }
  }

static void convert_YUV_411_P_to_RGB24(gavl_video_frame_t * in_frame,
                               gavl_video_frame_t * out_frame,
                               int width, int height)
  {
  int i, j, i_tmp;

  uint8_t * in_y;
  uint8_t * in_u;
  uint8_t * in_v;

  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->planes[0];
  uint8_t * in_y_save = in_frame->planes[0];
  uint8_t * in_u_save = in_frame->planes[1];
  uint8_t * in_v_save = in_frame->planes[2];

  for(i = 0; i < height; i++)
    {
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width/4; j++)
      {
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;

      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;

      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];
    in_u_save += in_frame->strides[1];
    in_v_save += in_frame->strides[2];
    }
  }


static void convert_YUVJ_422_P_to_RGB24(gavl_video_frame_t * in_frame,
                                 gavl_video_frame_t * out_frame,
                                 int width, int height)
  {
  int i, j, i_tmp;

  uint8_t * in_y;
  uint8_t * in_u;
  uint8_t * in_v;

  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->planes[0];
  uint8_t * in_y_save = in_frame->planes[0];
  uint8_t * in_u_save = in_frame->planes[1];
  uint8_t * in_v_save = in_frame->planes[2];

  for(i = 0; i < height; i++)
    {
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width/2; j++)
      {
      YUVJ_2_RGB(*in_y, *in_u, *in_v,
                 out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      
      YUVJ_2_RGB(*in_y, *in_u, *in_v,
                 out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];
    in_u_save += in_frame->strides[1];
    in_v_save += in_frame->strides[2];
    }
  }

static void convert_YUV_444_P_to_RGB24(gavl_video_frame_t * in_frame,
                                gavl_video_frame_t * out_frame,
                                int width, int height)
  {
  int i, j, i_tmp;

  uint8_t * in_y;
  uint8_t * in_u;
  uint8_t * in_v;

  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->planes[0];
  uint8_t * in_y_save = in_frame->planes[0];
  uint8_t * in_u_save = in_frame->planes[1];
  uint8_t * in_v_save = in_frame->planes[2];

  for(i = 0; i < height; i++)
    {
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];
    in_u_save += in_frame->strides[1];
    in_v_save += in_frame->strides[2];
    }
  }

static void convert_YUV_444_P_16_to_RGB24(gavl_video_frame_t * in_frame,
                                gavl_video_frame_t * out_frame,
                                int width, int height)
  {
  int i, j, i_tmp;

  uint16_t * in_y;
  uint16_t * in_u;
  uint16_t * in_v;

  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->planes[0];
  uint8_t * in_y_save = in_frame->planes[0];
  uint8_t * in_u_save = in_frame->planes[1];
  uint8_t * in_v_save = in_frame->planes[2];

  for(i = 0; i < height; i++)
    {
    in_y = (uint16_t*)in_y_save;
    in_u = (uint16_t*)in_u_save;
    in_v = (uint16_t*)in_v_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      YUV_2_RGB(((*in_y)>>8), ((*in_u)>>8), ((*in_v)>>8),
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];
    in_u_save += in_frame->strides[1];
    in_v_save += in_frame->strides[2];
    }
  }


static void convert_YUVJ_444_P_to_RGB24(gavl_video_frame_t * in_frame,
                                 gavl_video_frame_t * out_frame,
                                 int width, int height)
  {
  int i, j, i_tmp;

  uint8_t * in_y;
  uint8_t * in_u;
  uint8_t * in_v;

  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->planes[0];
  uint8_t * in_y_save = in_frame->planes[0];
  uint8_t * in_u_save = in_frame->planes[1];
  uint8_t * in_v_save = in_frame->planes[2];

  for(i = 0; i < height; i++)
    {
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      YUVJ_2_RGB(*in_y, *in_u, *in_v,
                 out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->strides[0];
    in_y_save += in_frame->strides[0];
    in_u_save += in_frame->strides[1];
    in_v_save += in_frame->strides[2];
    }
  }

static void convert_YUY2_to_RGB24(gavl_video_frame_t * in_frame,
                           gavl_video_frame_t * out_frame,
                           int width, int height)
  {
  int i, j, i_tmp;

  uint8_t * in_pixel;
  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];

  for(i = 0; i < height; i++)
    {
    in_pixel = in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width/2; j++)
      {
      YUV_2_RGB(in_pixel[0], in_pixel[1], in_pixel[3],
                out_pixel[0], out_pixel[1], out_pixel[2]);
      out_pixel+=3;
      YUV_2_RGB(in_pixel[2], in_pixel[1], in_pixel[3],
                out_pixel[0], out_pixel[1], out_pixel[2]);
      out_pixel+=3;
      in_pixel += 4;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

static void convert_UYVY_to_RGB24(gavl_video_frame_t * in_frame,
                           gavl_video_frame_t * out_frame,
                           int width, int height)
  {
  int i, j, i_tmp;

  uint8_t * in_pixel;
  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->planes[0];
  uint8_t * in_pixel_save = in_frame->planes[0];

  for(i = 0; i < height; i++)
    {
    in_pixel = in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width/2; j++)
      {
      YUV_2_RGB(in_pixel[1], in_pixel[0], in_pixel[2],
                out_pixel[0], out_pixel[1], out_pixel[2]);
      out_pixel+=3;
      YUV_2_RGB(in_pixel[3], in_pixel[0], in_pixel[2],
                out_pixel[0], out_pixel[1], out_pixel[2]);
      out_pixel+=3;
      in_pixel += 4;
      }
    in_pixel_save += in_frame->strides[0];
    out_pixel_save +=  out_frame->strides[0];
    }
  }

/*
 *  This function writes a png file of the video frame in the given format
 *  The format can have all supported colorspaces, so we'll convert them
 *  without the use of gavl (i.e. in less optimized but bugfree code)
 */

/* On error, FALSE is returned */

static int write_file(const char * name,
               gavl_video_frame_t * frame, gavl_video_format_t * format)
  {
  int color_type;  
  int png_transforms;
  gavl_video_frame_t * tmp_frame;
  gavl_video_frame_t * out_frame = NULL;
  gavl_video_format_t tmp_format;

  int i;
  char ** row_pointers;  
  png_structp png_ptr;  png_infop info_ptr;
  
  FILE *fp = fopen(name, "wb");
  if (!fp)
    {
    fprintf(stderr, "Cannot open file %s, exiting \n", name);
    _exit(-1);
    return 0;
    }

  png_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr)
    return 0;
  
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    {
    png_destroy_write_struct(&png_ptr,
                             (png_infopp)NULL);
    return 0;
    }
  
  setjmp(png_jmpbuf(png_ptr));

  png_init_io(png_ptr, fp);
  
  if(gavl_pixelformat_is_gray(format->pixelformat))
    {
    if(gavl_pixelformat_has_alpha(format->pixelformat))
      color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
    else
      color_type = PNG_COLOR_TYPE_GRAY;
    }
  else
    {
    if(gavl_pixelformat_has_alpha(format->pixelformat))
      color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    else
      color_type = PNG_COLOR_TYPE_RGB;
    }
  
  png_set_IHDR(png_ptr, info_ptr, format->image_width, format->image_height,
               8, color_type, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  /* Set up the temporary video frame */

  tmp_format.image_width  = format->image_width;
  tmp_format.image_height = format->image_height;

  tmp_format.frame_width  = format->frame_width;
  tmp_format.frame_height = format->frame_height;
  
  /* We convert everything to either RGB24, BGR24 or RGBA */
  
  switch(format->pixelformat)
    {
    case GAVL_BGR_15:
    case GAVL_BGR_16:
    case GAVL_BGR_24:
    case GAVL_BGR_32:
      png_transforms = PNG_TRANSFORM_BGR;
      tmp_format.pixelformat = GAVL_BGR_24;
      break;
    case GAVL_RGBA_64:
    case GAVL_RGBA_FLOAT:
    case GAVL_YUVA_32:
    case GAVL_YUVA_64:
    case GAVL_YUVA_FLOAT:
      png_transforms = PNG_TRANSFORM_IDENTITY;
      tmp_format.pixelformat = GAVL_RGBA_32;
      break;
    default:
      png_transforms = PNG_TRANSFORM_IDENTITY;
      tmp_format.pixelformat = GAVL_RGB_24;
    }

  /* Allocate the video frame structure */

  tmp_frame = NULL;

  switch(format->pixelformat)
    {
    case GAVL_RGB_15:
    case GAVL_BGR_15:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_15_to_24(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_16_to_24(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_GRAY_16:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_gray_16_to_gray_8(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_GRAY_FLOAT:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_gray_float_to_gray_8(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_GRAYA_FLOAT:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_graya_float_to_graya_16(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_GRAYA_32:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_graya_32_to_graya_16(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_GRAY_8:
    case GAVL_GRAYA_16:
    case GAVL_RGB_24:
    case GAVL_BGR_24:
    case GAVL_RGBA_32:
      out_frame = frame;
      break;
    case GAVL_RGB_48:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_48_to_24(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_RGBA_64:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_64_to_32(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_RGB_FLOAT:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_float_to_24(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_RGBA_FLOAT:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_float_to_32(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_RGB_32:
    case GAVL_BGR_32:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_32_to_24(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUY2:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUY2_to_RGB24(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUVA_32:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUVA_32_to_RGBA_32(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUVA_64:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUVA_64_to_RGBA_32(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUVA_FLOAT:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUVA_FLOAT_to_RGBA_32(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUV_FLOAT:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUV_FLOAT_to_RGB_24(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_UYVY:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_UYVY_to_RGB24(frame, tmp_frame, format->image_width, format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUV_420_P:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUV_420_P_to_RGB24(frame, tmp_frame, format->image_width,
                                 format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUV_410_P:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUV_410_P_to_RGB24(frame, tmp_frame, format->image_width,
                                 format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUV_422_P:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUV_422_P_to_RGB24(frame, tmp_frame, format->image_width,
                                 format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUV_422_P_16:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUV_422_P_16_to_RGB24(frame, tmp_frame, format->image_width,
                                 format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUV_411_P:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUV_411_P_to_RGB24(frame, tmp_frame, format->image_width,
                                 format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUV_444_P:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUV_444_P_to_RGB24(frame, tmp_frame, format->image_width,
                                 format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUV_444_P_16:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUV_444_P_16_to_RGB24(frame, tmp_frame, format->image_width,
                                 format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUVJ_420_P:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUVJ_420_P_to_RGB24(frame, tmp_frame, format->image_width,
                                 format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUVJ_422_P:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUVJ_422_P_to_RGB24(frame, tmp_frame, format->image_width,
                                 format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUVJ_444_P:
      tmp_frame = gavl_video_frame_create(&tmp_format);
      convert_YUVJ_444_P_to_RGB24(frame, tmp_frame, format->image_width,
                                 format->image_height);
      out_frame = tmp_frame;
      break;
    case GAVL_PIXELFORMAT_NONE:
      break;
    }

  /* Set up the row pointers */

  row_pointers = malloc(format->image_height * sizeof(char*));
 
  for(i = 0; i < format->image_height; i++)
    row_pointers[i] =
      (char*)(out_frame->planes[0] + i * out_frame->strides[0]);
  
  png_set_rows(png_ptr, info_ptr, (png_bytep*)row_pointers);

  png_write_png(png_ptr, info_ptr, png_transforms, NULL);

  free(row_pointers);

  if(tmp_frame)
    gavl_video_frame_destroy(tmp_frame);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(fp);
    
  return 1;
  }

/**********************************************************
 * Test picture generator
 **********************************************************/

static uint8_t colorbar_colors[16][2][3] =
  {
    {
      { 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, } /* White */
    },
    {
      { 1.0, 1.0, 1.0 }, { 1.0, 1.0, 1.0, }
    },
    {
      { 0.0, 0.0, 0.0 }, { 1.0, 1.0, 0.0, }  /* Yellow */
    },
    {
      { 1.0, 1.0, 1.0 }, { 1.0, 1.0, 0.0, }
    },
    {
      { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 1.0, } /* Cyan */
    },
    {
      { 1.0, 1.0, 1.0 }, { 0.0, 1.0, 1.0, } 
    },
    {
      { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0, } /* Green */
    },
    {
      { 1.0, 1.0, 1.0 }, { 0.0, 1.0, 0.0, } 
    },
    {
      { 0.0, 0.0, 0.0 }, { 1.0, 0.0, 1.0, } /* Magenta */
    },
    {
      { 1.0, 1.0, 1.0 }, { 1.0, 0.0, 1.0, }
    },
    {
      { 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0, } /* Red */
    },
    {
      { 1.0, 1.0, 1.0 }, { 1.0, 0.0, 0.0, }
    },
    {
      { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0, }  /* Blue */
    },
    {
      { 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0, } 
    },
    {
      { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0, }, /* Black */
    },
    {
      { 1.0, 1.0, 1.0 }, { 0.0, 0.0, 0.0, }
    }
  };

// #define COLORBAR_HORIZONTAL

static void get_pixel_colorbar(int x, int y,
                        float * ret)
  {
  int color_index;
  float r_tmp, g_tmp, b_tmp, alpha;

#ifdef COLORBAR_HORIZONTAL
  alpha = (float)(TEST_PICTURE_WIDTH - x) / (float)(TEST_PICTURE_WIDTH);
  color_index = (y * 16) / (TEST_PICTURE_HEIGHT - 1);
#else
  alpha = (float)(TEST_PICTURE_HEIGHT - y) / (float)(TEST_PICTURE_HEIGHT);
  color_index = (x * 16) / (TEST_PICTURE_WIDTH - 1);
#endif
  if(color_index >= 16)
    color_index = 15;

  alpha = RECLIP_FLOAT(alpha);
  
  /*  fprintf(stderr, "y: %d alpha: %d\n", y ,(int)alpha); */

  // colorbar_colors[16][2][3]
  
  r_tmp = (colorbar_colors[color_index][0][0] * (1.0 - alpha) + 
           colorbar_colors[color_index][1][0] * alpha); 
  g_tmp = (colorbar_colors[color_index][0][1] * (1.0 - alpha) +
           colorbar_colors[color_index][1][1] * alpha); 
  b_tmp = (colorbar_colors[color_index][0][2] * (1.0 - alpha) +
           colorbar_colors[color_index][1][2] * alpha); 

/*   r_tmp =  colorbar_colors[color_index][1][0]; */
/*   g_tmp =  colorbar_colors[color_index][1][1]; */
/*   b_tmp =  colorbar_colors[color_index][1][2]; */
  
//  a_tmp = 0xFF; /* Not used for now */
  
  ret[0] = RECLIP_FLOAT(r_tmp);
  ret[1] = RECLIP_FLOAT(g_tmp);
  ret[2] = RECLIP_FLOAT(b_tmp);
  ret[3] = RECLIP_FLOAT(alpha);
  
  }

#define PACK_RGB16(_r,_g,_b,_pixel) \
_pixel=((((((_r<<5)&0xff00)|_g)<<6)&0xfff00)|_b)>>3

#define PACK_RGB15(_r,_g,_b,_pixel) \
_pixel=((((((_r<<5)&0xff00)|_g)<<5)&0xfff00)|_b)>>3

#define FLOAT_TO_8(f) (int)(f * 255.0 + 0.5)
#define FLOAT_TO_16(f) (int)(f * 65535.0 + 0.5)

#define RGB_TO_Y() yuv_f[0] = (0.299*tmp_f[0]+0.587*tmp_f[1]+0.114*tmp_f[2])

#define RGB_TO_YUV() RGB_TO_Y();                                        \
  yuv_f[1] = (-0.168736 * tmp_f[0] -0.331264 * tmp_f[1] + 0.500    * tmp_f[2]); \
    yuv_f[2] = (0.500     * tmp_f[0] -0.418688 * tmp_f[1] - 0.081312 * tmp_f[2])

#define Y_TO_8(d) d=(int)(yuv_f[0]*219.0+0.5)+16
#define U_TO_8(d) d=(int)(yuv_f[1]*224.0+0.5)+128
#define V_TO_8(d) d=(int)(yuv_f[2]*224.0+0.5)+128

#define Y_TO_16(d) d=((int)(yuv_f[0]*219.0+0.5)+16) << 8;
#define U_TO_16(d) d=((int)(yuv_f[1]*224.0+0.5)+128) << 8;
#define V_TO_16(d) d=((int)(yuv_f[2]*224.0+0.5)+128) << 8;

#define YJ_TO_8(d) d=RECLIP((int)(yuv_f[0]*255.0+0.5))
#define UJ_TO_8(d) d=RECLIP((int)(yuv_f[1]*255.0+0.5)+128)
#define VJ_TO_8(d) d=RECLIP((int)(yuv_f[2]*255.0+0.5)+128)


static gavl_video_frame_t * create_picture(gavl_pixelformat_t pixelformat,
                                    void (*get_pixel)(int x, int y,
                                                      float * ret))
  
  {
  int r_tmp;
  int g_tmp;
  int b_tmp;
  int a_tmp;
  
  float tmp_f[4];
  float yuv_f[4];
  
  uint8_t  * pixel;
  uint16_t * pixel_16;
  float * pixel_float;
  uint8_t * y;
  uint8_t * u;
  uint8_t * v;

  uint16_t * y_16;
  uint16_t * u_16;
  uint16_t * v_16;
    
  int row, col;
  gavl_video_frame_t * ret;
  gavl_video_format_t format;
  format.pixelformat = pixelformat;
  format.image_width = TEST_PICTURE_WIDTH;
  format.image_height = TEST_PICTURE_HEIGHT;

  format.frame_width = TEST_PICTURE_WIDTH;
  format.frame_height = TEST_PICTURE_HEIGHT;

  format.pixel_width = 1;
  format.pixel_height = 1;
  
  
  ret = gavl_video_frame_create(&format);

  switch(pixelformat)
    {
    case GAVL_RGB_15:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_16 = (uint16_t*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          r_tmp = FLOAT_TO_8(tmp_f[0]);
          r_tmp = RECLIP(r_tmp);
          g_tmp = FLOAT_TO_8(tmp_f[1]);
          g_tmp = RECLIP(g_tmp);
          b_tmp = FLOAT_TO_8(tmp_f[2]);
          b_tmp = RECLIP(b_tmp);
                    
          PACK_RGB15(r_tmp, g_tmp, b_tmp, *pixel_16);
          pixel_16++;
          }
        }
      break;
    case GAVL_BGR_15:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_16 = (uint16_t*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          r_tmp = FLOAT_TO_8(tmp_f[0]);
          r_tmp = RECLIP(r_tmp);
          g_tmp = FLOAT_TO_8(tmp_f[1]);
          g_tmp = RECLIP(g_tmp);
          b_tmp = FLOAT_TO_8(tmp_f[2]);
          b_tmp = RECLIP(b_tmp);

          PACK_RGB15(b_tmp, g_tmp, r_tmp, *pixel_16);
          pixel_16++;
          }
        }
      break;
    case GAVL_RGB_16:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_16 = (uint16_t*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          r_tmp = FLOAT_TO_8(tmp_f[0]);
          r_tmp = RECLIP(r_tmp);
          g_tmp = FLOAT_TO_8(tmp_f[1]);
          g_tmp = RECLIP(g_tmp);
          b_tmp = FLOAT_TO_8(tmp_f[2]);
          b_tmp = RECLIP(b_tmp);

          PACK_RGB16(r_tmp, g_tmp, b_tmp, *pixel_16);
          pixel_16++;
          }
        }
      break;
    case GAVL_BGR_16:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_16 = (uint16_t*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);
          
          r_tmp = FLOAT_TO_8(tmp_f[0]);
          r_tmp = RECLIP(r_tmp);
          g_tmp = FLOAT_TO_8(tmp_f[1]);
          g_tmp = RECLIP(g_tmp);
          b_tmp = FLOAT_TO_8(tmp_f[2]);
          b_tmp = RECLIP(b_tmp);
          PACK_RGB16(b_tmp, g_tmp, r_tmp, *pixel_16);
          pixel_16++;
          }
        }
      break;
    case GAVL_RGB_24:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->planes[0] + row * ret->strides[0];
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);
          
          r_tmp = FLOAT_TO_8(tmp_f[0]);
          r_tmp = RECLIP(r_tmp);
          g_tmp = FLOAT_TO_8(tmp_f[1]);
          g_tmp = RECLIP(g_tmp);
          b_tmp = FLOAT_TO_8(tmp_f[2]);
          b_tmp = RECLIP(b_tmp);

          pixel[0] = r_tmp;
          pixel[1] = g_tmp;
          pixel[2] = b_tmp;

          pixel += 3;
          }
        }
      break;
    case GAVL_BGR_24:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->planes[0] + row * ret->strides[0];
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);
          
          r_tmp = FLOAT_TO_8(tmp_f[0]);
          r_tmp = RECLIP(r_tmp);
          g_tmp = FLOAT_TO_8(tmp_f[1]);
          g_tmp = RECLIP(g_tmp);
          b_tmp = FLOAT_TO_8(tmp_f[2]);
          b_tmp = RECLIP(b_tmp);

          pixel[2] = r_tmp;
          pixel[1] = g_tmp;
          pixel[0] = b_tmp;
          pixel += 3;
          }
        }
      break;
    case GAVL_RGB_32:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->planes[0] + row * ret->strides[0];
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);
          
          r_tmp = FLOAT_TO_8(tmp_f[0]);
          r_tmp = RECLIP(r_tmp);
          g_tmp = FLOAT_TO_8(tmp_f[1]);
          g_tmp = RECLIP(g_tmp);
          b_tmp = FLOAT_TO_8(tmp_f[2]);
          b_tmp = RECLIP(b_tmp);

          pixel[0] = r_tmp;
          pixel[1] = g_tmp;
          pixel[2] = b_tmp;
          pixel += 4;
          }
        }
      break;
    case GAVL_BGR_32:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->planes[0] + row * ret->strides[0];
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);
          
          r_tmp = FLOAT_TO_8(tmp_f[0]);
          r_tmp = RECLIP(r_tmp);
          g_tmp = FLOAT_TO_8(tmp_f[1]);
          g_tmp = RECLIP(g_tmp);
          b_tmp = FLOAT_TO_8(tmp_f[2]);
          b_tmp = RECLIP(b_tmp);

          pixel[2] = r_tmp;
          pixel[1] = g_tmp;
          pixel[0] = b_tmp;
          pixel += 4;
          }
        }
      break;
    case GAVL_RGBA_32:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->planes[0] + row * ret->strides[0];
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);
          
          r_tmp = FLOAT_TO_8(tmp_f[0]);
          r_tmp = RECLIP(r_tmp);
          g_tmp = FLOAT_TO_8(tmp_f[1]);
          g_tmp = RECLIP(g_tmp);
          b_tmp = FLOAT_TO_8(tmp_f[2]);
          b_tmp = RECLIP(b_tmp);
          a_tmp = FLOAT_TO_8(tmp_f[3]);
          a_tmp = RECLIP(a_tmp);

          pixel[0] = r_tmp;
          pixel[1] = g_tmp;
          pixel[2] = b_tmp;
          pixel[3] = a_tmp;
          pixel += 4;
          }
        }
      break;
    case GAVL_RGB_48:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_16 = (uint16_t*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);
          
          r_tmp = FLOAT_TO_16(tmp_f[0]);
          r_tmp = RECLIP_16(r_tmp);
          g_tmp = FLOAT_TO_16(tmp_f[1]);
          g_tmp = RECLIP_16(g_tmp);
          b_tmp = FLOAT_TO_16(tmp_f[2]);
          b_tmp = RECLIP_16(b_tmp);

          pixel_16[0] = r_tmp;
          pixel_16[1] = g_tmp;
          pixel_16[2] = b_tmp;
          pixel_16 += 3;
          }
        }
      break;
    case GAVL_RGBA_64:
      //      fprintf(stderr, "STRIDES: %d\n", ret->strides[0]);
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_16 = (uint16_t*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);
          
          r_tmp = FLOAT_TO_16(tmp_f[0]);
          r_tmp = RECLIP_16(r_tmp);

          g_tmp = FLOAT_TO_16(tmp_f[1]);
          g_tmp = RECLIP_16(g_tmp);

          b_tmp = FLOAT_TO_16(tmp_f[2]);
          b_tmp = RECLIP_16(b_tmp);

          a_tmp = FLOAT_TO_16(tmp_f[3]);
          a_tmp = RECLIP_16(a_tmp);

          pixel_16[0] = r_tmp;
          pixel_16[1] = g_tmp;
          pixel_16[2] = b_tmp;
          pixel_16[3] = a_tmp;
          pixel_16 += 4;
          }
        }
      //      gavl_video_frame_dump(ret, &format, "TEST");
      break;
    case GAVL_RGB_FLOAT:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_float = (float*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          pixel_float[0] = tmp_f[0];
          pixel_float[1] = tmp_f[1];
          pixel_float[2] = tmp_f[2];
          pixel_float += 3;
          }
        }
      break;
    case GAVL_RGBA_FLOAT:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_float = (float*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);
          pixel_float[0] = tmp_f[0];
          pixel_float[1] = tmp_f[1];
          pixel_float[2] = tmp_f[2];
          pixel_float[3] = tmp_f[3];
          
          pixel_float += 4;
          }
        }
      break;
    case GAVL_YUY2:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->planes[0] + row * ret->strides[0];
        for(col = 0; col < TEST_PICTURE_WIDTH/2; col++)
          {
          get_pixel(2*col, row, tmp_f);

          RGB_TO_YUV();
          Y_TO_8(pixel[0]);
          U_TO_8(pixel[1]);
          V_TO_8(pixel[3]);
          
          get_pixel(2*col+1, row, tmp_f);
          RGB_TO_Y();
          Y_TO_8(pixel[2]);

          pixel+= 4;
          }
        }
      break;
    case GAVL_YUVA_32:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->planes[0] + row * ret->strides[0];
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          RGB_TO_YUV();
          Y_TO_8(pixel[0]);
          U_TO_8(pixel[1]);
          V_TO_8(pixel[2]);
          a_tmp = FLOAT_TO_8(tmp_f[3]);
          a_tmp = RECLIP(a_tmp);
          pixel[3] = a_tmp;          
          pixel+= 4;
          }
        }
      break;
    case GAVL_YUVA_64:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_16 = (uint16_t*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          RGB_TO_YUV();
          Y_TO_16(pixel_16[0]);
          U_TO_16(pixel_16[1]);
          V_TO_16(pixel_16[2]);
          a_tmp = FLOAT_TO_16(tmp_f[3]);
          a_tmp = RECLIP_16(a_tmp);
          pixel_16[3] = a_tmp;          
          pixel_16+= 4;
          }
        }
      break;
    case GAVL_YUVA_FLOAT:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_float = (float*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          RGB_TO_YUV();
          pixel_float[0] = yuv_f[0];
          pixel_float[1] = yuv_f[1];
          pixel_float[2] = yuv_f[2];
          pixel_float[3] = tmp_f[3];          
          pixel_float+= 4;
          }
        }
      break;
    case GAVL_GRAYA_FLOAT:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_float = (float*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          RGB_TO_YUV();
          pixel_float[0] = yuv_f[0];
          pixel_float[1] = tmp_f[3];          
          pixel_float+= 2;
          }
        }
      break;
    case GAVL_GRAY_FLOAT:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_float = (float*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          RGB_TO_YUV();
          pixel_float[0] = yuv_f[0];
          pixel_float++;
          }
        }
      break;
    case GAVL_GRAYA_32:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_16 = (uint16_t*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          RGB_TO_YUV();
          pixel_16[0] = FLOAT_TO_16(yuv_f[0]);
          pixel_16[1] = FLOAT_TO_16(tmp_f[3]);
          pixel_16+= 2;
          }
        }
      break;
    case GAVL_GRAY_16:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_16 = (uint16_t*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          RGB_TO_YUV();
          pixel_16[0] = FLOAT_TO_16(yuv_f[0]);
          pixel_16++;
          }
        }
      break;
    case GAVL_GRAYA_16:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = (ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          RGB_TO_YUV();
          pixel[0] = FLOAT_TO_8(yuv_f[0]);
          pixel[1] = FLOAT_TO_8(tmp_f[3]);
          pixel+= 2;
          }
        }
      break;
    case GAVL_GRAY_8:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = (ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          RGB_TO_YUV();
          pixel[0] = FLOAT_TO_8(yuv_f[0]);
          pixel++;
          }
        }
      break;
    case GAVL_YUV_FLOAT:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_float = (float*)(ret->planes[0] + row * ret->strides[0]);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          RGB_TO_YUV();
          pixel_float[0] = yuv_f[0];
          pixel_float[1] = yuv_f[1];
          pixel_float[2] = yuv_f[2];
          pixel_float+=3;
          }
        }
      break;
    case GAVL_UYVY:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->planes[0] + row * ret->strides[0];
        for(col = 0; col < TEST_PICTURE_WIDTH/2; col++)
          {
          get_pixel(2*col, row, tmp_f);

          RGB_TO_YUV();
          Y_TO_8(pixel[1]);
          U_TO_8(pixel[0]);
          V_TO_8(pixel[2]);

          
          get_pixel(2*col+1, row, tmp_f);
          RGB_TO_Y();
          Y_TO_8(pixel[3]);
          pixel+= 4;
          }
        }
      break;
    case GAVL_YUV_420_P:
      for(row = 0; row < TEST_PICTURE_HEIGHT/2; row++)
        {
        y = ret->planes[0] + 2 * row * ret->strides[0];
        u = ret->planes[1] + row * ret->strides[1];
        v = ret->planes[2] + row * ret->strides[2];

        for(col = 0; col < TEST_PICTURE_WIDTH/2; col++)
          {
          get_pixel(2*col, 2*row, tmp_f);

          RGB_TO_YUV();
          Y_TO_8(*y);
          U_TO_8(*u);
          V_TO_8(*v);

          y++;
          
          get_pixel(2*col+1, 2*row, tmp_f);
          RGB_TO_YUV();
          Y_TO_8(*y);
          U_TO_8(*u);
          V_TO_8(*v);
          
          y++;
          u++;
          v++;
          }

        y = ret->planes[0] + (2 * row + 1) * ret->strides[0];

        for(col = 0; col < TEST_PICTURE_WIDTH/2; col++)
          {
          get_pixel(2*col, 2*row+1, tmp_f);

          RGB_TO_Y();
          Y_TO_8(*y);

          y++;
          
          get_pixel(2*col+1, 2*row+1, tmp_f);

          RGB_TO_Y();
          Y_TO_8(*y);
          
          y++;
          }
        }
      break;
    case GAVL_YUV_410_P:
      for(row = 0; row < TEST_PICTURE_HEIGHT/4; row++)
        {
        y = ret->planes[0] + 4 * row * ret->strides[0];
        u = ret->planes[1] + row * ret->strides[1];
        v = ret->planes[2] + row * ret->strides[2];

        for(col = 0; col < TEST_PICTURE_WIDTH/4; col++)
          {
          get_pixel(4*col, 4*row, tmp_f);
          RGB_TO_YUV();
          Y_TO_8(*y);
          U_TO_8(*u);
          V_TO_8(*v);
          y++;
          
          get_pixel(4*col+1, 4*row, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);
          
          y++;
          get_pixel(4*col+2, 4*row, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);

          
          y++;
          get_pixel(4*col+3, 4*row, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);
          
          y++;

          u++;
          v++;
          }

        y = ret->planes[0] + (4 * row + 1) * ret->strides[0];

        for(col = 0; col < TEST_PICTURE_WIDTH/4; col++)
          {
          get_pixel(4*col, 4*row+1, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);

          y++;
          
          get_pixel(4*col+1, 4*row+1, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);

          y++;

          get_pixel(4*col+2, 4*row+1, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);

          y++;

          get_pixel(4*col+3, 4*row+1, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);

          y++;
          }

        y = ret->planes[0] + (4 * row + 2) * ret->strides[0];

        for(col = 0; col < TEST_PICTURE_WIDTH/4; col++)
          {
          get_pixel(4*col, 4*row+2, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);

          y++;
          
          get_pixel(4*col+1, 4*row+2, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);
          y++;

          get_pixel(4*col+2, 4*row+2, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);

          y++;

          get_pixel(4*col+3, 4*row+2, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);

          y++;
          }

        y = ret->planes[0] + (4 * row + 3) * ret->strides[0];

        for(col = 0; col < TEST_PICTURE_WIDTH/4; col++)
          {
          get_pixel(4*col, 4*row+3, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);

          y++;
          
          get_pixel(4*col+1, 4*row+3, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);

          y++;

          get_pixel(4*col+2, 4*row+3, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);

          y++;

          get_pixel(4*col+3, 4*row+3, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);

          y++;
          }

        }
      break;
    case GAVL_YUV_422_P:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        y = ret->planes[0] + row * ret->strides[0];
        u = ret->planes[1] + row * ret->strides[1];
        v = ret->planes[2] + row * ret->strides[2];

        for(col = 0; col < TEST_PICTURE_WIDTH/2; col++)
          {
          get_pixel(2* col, row, tmp_f);

          RGB_TO_YUV();
          Y_TO_8(*y);
          U_TO_8(*u);
          V_TO_8(*v);

          y++;
          
          get_pixel(2* col + 1, row, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);
          
          y++;
          u++;
          v++;
          }
        }
      break;
    case GAVL_YUV_422_P_16:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        y_16 = (uint16_t*)(ret->planes[0] + row * ret->strides[0]);
        u_16 = (uint16_t*)(ret->planes[1] + row * ret->strides[1]);
        v_16 = (uint16_t*)(ret->planes[2] + row * ret->strides[2]);

        for(col = 0; col < TEST_PICTURE_WIDTH/2; col++)
          {
          get_pixel(2* col, row, tmp_f);

          RGB_TO_YUV();
          Y_TO_16(*y_16);
          U_TO_16(*u_16);
          V_TO_16(*v_16);

          y_16++;
          
          get_pixel(2* col + 1, row, tmp_f);
          RGB_TO_Y();
          Y_TO_16(*y_16);
          
          y_16++;
          u_16++;
          v_16++;
          }
        }
      break;
    case GAVL_YUV_411_P:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        y = ret->planes[0] + row * ret->strides[0];
        u = ret->planes[1] + row * ret->strides[1];
        v = ret->planes[2] + row * ret->strides[2];

        for(col = 0; col < TEST_PICTURE_WIDTH/4; col++)
          {
          get_pixel(4* col, row, tmp_f);

          RGB_TO_YUV();
          Y_TO_8(*y);
          U_TO_8(*u);
          V_TO_8(*v);

          y++;
          
          get_pixel(4* col + 1, row, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);

          
          y++;
          get_pixel(4* col + 2, row, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);
          
          y++;
          get_pixel(4* col + 3, row, tmp_f);
          RGB_TO_Y();
          Y_TO_8(*y);
          
          y++;

          u++;
          v++;
          }
        }
      break;
    case GAVL_YUV_444_P:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        y = ret->planes[0] + row * ret->strides[0];
        u = ret->planes[1] + row * ret->strides[1];
        v = ret->planes[2] + row * ret->strides[2];

        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          RGB_TO_YUV();
          Y_TO_8(*y);
          U_TO_8(*u);
          V_TO_8(*v);
          
          y++;
          u++;
          v++;
          }
        }
      break;
    case GAVL_YUV_444_P_16:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        y_16 = (uint16_t*)(ret->planes[0] + row * ret->strides[0]);
        u_16 = (uint16_t*)(ret->planes[1] + row * ret->strides[1]);
        v_16 = (uint16_t*)(ret->planes[2] + row * ret->strides[2]);

        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);

          RGB_TO_YUV();
          Y_TO_16(*y_16);
          U_TO_16(*u_16);
          V_TO_16(*v_16);
          
          y_16++;
          u_16++;
          v_16++;
          }
        }
      break;
    case GAVL_YUVJ_420_P:
      for(row = 0; row < TEST_PICTURE_HEIGHT/2; row++)
        {
        y = ret->planes[0] + 2 * row * ret->strides[0];
        u = ret->planes[1] + row * ret->strides[1];
        v = ret->planes[2] + row * ret->strides[2];

        for(col = 0; col < TEST_PICTURE_WIDTH/2; col++)
          {
          get_pixel(2*col, 2*row, tmp_f);

          RGB_TO_YUV();
          YJ_TO_8(*y);
          UJ_TO_8(*u);
          VJ_TO_8(*v);

          y++;
          
          get_pixel(2*col+1, 2*row, tmp_f);

          RGB_TO_Y();
          YJ_TO_8(*y);
          
          y++;
          u++;
          v++;
          }

        y = ret->planes[0] + (2 * row + 1) * ret->strides[0];

        for(col = 0; col < TEST_PICTURE_WIDTH/2; col++)
          {
          get_pixel(2*col, 2*row+1, tmp_f);
          RGB_TO_Y();
          YJ_TO_8(*y);

          y++;
          
          get_pixel(2*col+1, 2*row+1, tmp_f);
          RGB_TO_Y();
          YJ_TO_8(*y);
          
          y++;
          }
        }
      break;
    case GAVL_YUVJ_422_P:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        y = ret->planes[0] + row * ret->strides[0];
        u = ret->planes[1] + row * ret->strides[1];
        v = ret->planes[2] + row * ret->strides[2];

        for(col = 0; col < TEST_PICTURE_WIDTH/2; col++)
          {
          get_pixel(2* col, row, tmp_f);

          RGB_TO_YUV();
          YJ_TO_8(*y);
          UJ_TO_8(*u);
          VJ_TO_8(*v);

          y++;
          
          get_pixel(2* col + 1, row, tmp_f);
          RGB_TO_Y();
          YJ_TO_8(*y);

          
          y++;
          u++;
          v++;
          }
        }
      break;
    case GAVL_YUVJ_444_P:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        y = ret->planes[0] + row * ret->strides[0];
        u = ret->planes[1] + row * ret->strides[1];
        v = ret->planes[2] + row * ret->strides[2];

        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, tmp_f);
          RGB_TO_YUV();
          YJ_TO_8(*y);
          UJ_TO_8(*u);
          VJ_TO_8(*v);
          y++;
          u++;
          v++;
          }
        }
      break;
    case GAVL_PIXELFORMAT_NONE:
      break;
    }
  
  return  ret;
  }

int main(int argc, char ** argv)
  {
#ifdef ALL_PIXELFORMATS
  int i, j;
#endif
  const char * tmp1, * tmp2;
  char filename_buffer[128];

  //  float background[3] = { 1.0, 0.0, 0.0 };
    
  gavl_video_format_t input_format;
  gavl_video_format_t output_format;

  gavl_video_frame_t * input_frame;
  gavl_video_frame_t * output_frame;
  
  gavl_video_options_t * opt;

  gavl_video_converter_t * cnv = gavl_video_converter_create();
  opt = gavl_video_converter_get_options(cnv);

  memset(&input_format, 0, sizeof(input_format));
  memset(&output_format, 0, sizeof(output_format));
  
  input_format.image_width = TEST_PICTURE_WIDTH;
  input_format.image_height = TEST_PICTURE_HEIGHT;
  
  input_format.frame_width = TEST_PICTURE_WIDTH;
  input_format.frame_height = TEST_PICTURE_HEIGHT;
  
  input_format.pixel_width = 1;
  input_format.pixel_height = 1;
  
  output_format.frame_width = TEST_PICTURE_WIDTH;
  output_format.frame_height = TEST_PICTURE_HEIGHT;

  output_format.image_width = TEST_PICTURE_WIDTH;
  output_format.image_height = TEST_PICTURE_HEIGHT;

  output_format.pixel_width = 1;
  output_format.pixel_height = 1;
 
  init_yuv();
 
#ifdef ALL_PIXELFORMATS
  for(i = 0; i < gavl_num_pixelformats(); i++)
    {
    input_format.pixelformat = gavl_get_pixelformat(i);
#else
    input_format.pixelformat = IN_PIXELFORMAT;
#endif
    input_frame = create_picture(input_format.pixelformat,
                                 get_pixel_colorbar);

    tmp1 = gavl_pixelformat_to_string(input_format.pixelformat);
    sprintf(filename_buffer, "0test_%s.png", tmp1);
    
    write_file(filename_buffer,
                 input_frame, &input_format);

#ifdef ALL_PIXELFORMATS
    for(j = 0; j < gavl_num_pixelformats(); j++)
      {
      output_format.pixelformat = gavl_get_pixelformat(j);
#else
      output_format.pixelformat = OUT_PIXELFORMAT;
#endif

      gavl_video_options_set_defaults(opt);

      gavl_video_options_set_quality(opt, 0);
      //      gavl_video_options_set_alpha_mode(opt, GAVL_ALPHA_BLEND_COLOR);
      // gavl_video_options_set_background_color(opt, background);

      gavl_video_options_set_alpha_mode(opt, GAVL_ALPHA_IGNORE);
      
#ifdef ALL_PIXELFORMATS
      if(input_format.pixelformat == output_format.pixelformat)
        continue;
#endif
      
      output_frame = gavl_video_frame_create(&output_format);
      fprintf(stderr, "************* Pixelformat conversion ");
      fprintf(stderr, "%s -> ", tmp1);
      tmp2 = gavl_pixelformat_to_string(output_format.pixelformat);
      
      fprintf(stderr, "%s *************\n", tmp2);

      gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_C);
      //      gavl_video_options_set_quality(opt, 5);
      gavl_video_frame_clear(output_frame, &output_format);
      
      if(gavl_video_converter_init(cnv, &input_format, &output_format) <= 0)
        {
        fprintf(stderr, "No Conversion defined yet or not necessary\n");
        }
      else
        {
        fprintf(stderr, "ANSI C Version: ");
        gavl_video_convert(cnv, input_frame, output_frame);
        }
      
      /* Now, do some conversions */

      sprintf(filename_buffer, "%s_to_%s_c.png", tmp1, tmp2);
      write_file(filename_buffer,
                 output_frame, &output_format);
      fprintf(stderr, "Wrote %s\n", filename_buffer);
#if 1
      /* HQ */
      
      gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_C_HQ);
      
      gavl_video_frame_clear(output_frame, &output_format);
      sprintf(filename_buffer, "%s_to_%s_hq.png", tmp1, tmp2);
      if(gavl_video_converter_init(cnv, &input_format, &output_format) <= 0)
        fprintf(stderr, "No High Quality Conversion defined yet\n");
      else
        {
        fprintf(stderr, "High Quality Version:    ");
        gavl_video_convert(cnv, input_frame, output_frame);
        write_file(filename_buffer,
                   output_frame, &output_format);
        fprintf(stderr, "Wrote %s\n", filename_buffer);
        }

      /* Now, initialize with MMX */
#endif
      
#if 1
      gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_MMX);
      
      gavl_video_frame_clear(output_frame, &output_format);
      sprintf(filename_buffer, "%s_to_%s_mmx.png", tmp1, tmp2);
      if(gavl_video_converter_init(cnv, &input_format, &output_format) <= 0)
        fprintf(stderr, "No MMX Conversion defined yet\n");
      else
        {
        fprintf(stderr, "MMX Version:    ");
        gavl_video_convert(cnv, input_frame, output_frame);
        write_file(filename_buffer,
                   output_frame, &output_format);
        fprintf(stderr, "Wrote %s\n", filename_buffer);
        }

      gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_MMXEXT);
      gavl_video_frame_clear(output_frame, &output_format);
      sprintf(filename_buffer, "%s_to_%s_mmxext.png", tmp1, tmp2);
      if(gavl_video_converter_init(cnv, &input_format, &output_format) <= 0)
        fprintf(stderr, "No MMXEXT Conversion defined yet\n");
      else
        {
        fprintf(stderr, "MMXEXT Version:    ");
        gavl_video_convert(cnv, input_frame, output_frame);
        write_file(filename_buffer,
                   output_frame, &output_format);
        fprintf(stderr, "Wrote %s\n", filename_buffer);
        }

      gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_SSE);
      gavl_video_frame_clear(output_frame, &output_format);
      sprintf(filename_buffer, "%s_to_%s_sse.png", tmp1, tmp2);
      if(gavl_video_converter_init(cnv, &input_format, &output_format) <= 0)
        fprintf(stderr, "No SSE Conversion defined yet\n");
      else
        {
        fprintf(stderr, "SSE Version:    ");
        gavl_video_convert(cnv, input_frame, output_frame);
        write_file(filename_buffer,
                   output_frame, &output_format);
        fprintf(stderr, "Wrote %s\n", filename_buffer);
        }

      gavl_video_options_set_accel_flags(opt, GAVL_ACCEL_SSE3);
      gavl_video_frame_clear(output_frame, &output_format);
      sprintf(filename_buffer, "%s_to_%s_sse3.png", tmp1, tmp2);
      if(gavl_video_converter_init(cnv, &input_format, &output_format) <= 0)
        fprintf(stderr, "No SSE3 Conversion defined yet\n");
      else
        {
        fprintf(stderr, "SSE3 Version:    ");
        gavl_video_convert(cnv, input_frame, output_frame);
        write_file(filename_buffer,
                   output_frame, &output_format);
        fprintf(stderr, "Wrote %s\n", filename_buffer);
        }
#endif
      
      gavl_video_frame_destroy(output_frame);

#ifdef ALL_PIXELFORMATS
      }
    }
#endif  
  
  return 0;
  }
