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
#include <config.h>
#include <video.h>
#include <colorspace.h>
#include <accel.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WORDS_BIGENDIAN
static const uint32_t rgb32_masks[4]  = { 0xff000000, 0x00ff0000, 0x0000ff00, 0x00000000 };
static const uint32_t rgba32_masks[4] = { 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff };
static const uint32_t bgr32_masks[4]  = { 0x0000ff00, 0x00ff0000, 0xff000000, 0x00000000 };
#else
static const uint32_t rgb32_masks[4]  = { 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000 };
static const uint32_t rgba32_masks[4] = { 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };
static const uint32_t bgr32_masks[4]  = { 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };
#endif

typedef struct
  {
  gavl_pixelformat_t pixelformat;
  const char * name;
  const char * short_name;
  const uint32_t * masks;
  } pixelformat_tab_t;

const pixelformat_tab_t pixelformat_tab[] =
  {
    { GAVL_PIXELFORMAT_NONE, "Undefined",           "unknown"   }, // Must be first
    { GAVL_GRAY_8,  "8 bpp gray",                   "gray8"     },
    { GAVL_GRAY_16, "16 bpp gray",                  "gray16"    },
    { GAVL_GRAY_FLOAT, "Float gray",                "grayf"     },
    { GAVL_GRAYA_16,  "16 bpp gray + alpha",        "graya16"   },
    { GAVL_GRAYA_32, "32 bpp gray + alpha",         "graya32"   },
    { GAVL_GRAYA_FLOAT, "Float gray + alpha",       "grayaf"    },
    { GAVL_RGB_15, "15 bpp RGB",                    "rgb15"     },
    { GAVL_BGR_15, "15 bpp BGR",                    "bgr15"     },
    { GAVL_RGB_16, "16 bpp RGB",                    "rgb16"     },
    { GAVL_BGR_16, "16 bpp BGR",                    "bgr16"     },
    { GAVL_RGB_24, "24 bpp RGB",                    "rgb24"     },
    { GAVL_BGR_24, "24 bpp BGR",                    "bgr24"     },
    { GAVL_RGB_32, "32 bpp RGB",                    "rgb32",    rgb32_masks },
    { GAVL_BGR_32, "32 bpp BGR",                    "bgr32",    bgr32_masks },
    { GAVL_RGBA_32, "32 bpp RGBA",                  "rgba32",   rgba32_masks },
    { GAVL_RGB_48, "48 bpp RGB",                    "rgb48"     },
    { GAVL_RGBA_64, "64 bpp RGBA",                  "rgba64"    },
    { GAVL_RGB_FLOAT, "Float RGB",                  "rgbf"      },
    { GAVL_RGBA_FLOAT, "Float RGBA",                "rgbaf"     },
    { GAVL_YUY2, "YUV 422 (YUY2)",                  "yuy2"      },
    { GAVL_UYVY, "YUV 422 (UYVY)",                  "uyvy"      },
    { GAVL_YUVA_32, "YUVA 4444 (8 bit)",            "yuva32"    },
    { GAVL_YUVA_64, "YUVA 4444 (16 bit)",           "yuva64"    },
    { GAVL_YUVA_FLOAT, "YUVA 4444 (float)",         "yuvaf"     },
    { GAVL_YUV_FLOAT, "YUV 444 (float)",            "yuv444pf"  },
    { GAVL_YUV_420_P, "YUV 420 Planar",             "yuv420p8"  },
    { GAVL_YUV_410_P, "YUV 410 Planar",             "yuv410p8"  },
    { GAVL_YUV_411_P, "YUV 411 Planar",             "yuv411p8"  },
    { GAVL_YUV_422_P, "YUV 422 Planar",             "yuv422p8"  },
    { GAVL_YUV_422_P_16, "YUV 422 Planar (16 bit)", "yuv422p16" },
    { GAVL_YUV_444_P, "YUV 444 Planar",             "yuv444p8"  },
    { GAVL_YUV_444_P_16, "YUV 444 Planar (16 bit)", "yuv444p16" },
    { GAVL_YUVJ_420_P, "YUVJ 420 Planar",           "yuvj420p8" },
    { GAVL_YUVJ_422_P, "YUVJ 422 Planar",           "yuvj422p8" },
    { GAVL_YUVJ_444_P, "YUVJ 444 Planar",           "yuvj444p8" }
  };

static const int num_pixelformats =
  sizeof(pixelformat_tab)/sizeof(pixelformat_tab[0]);

int gavl_pixelformat_num_planes(gavl_pixelformat_t csp)
  {
  switch(csp)
    {
    case GAVL_GRAY_8:
    case GAVL_GRAY_16:
    case GAVL_GRAY_FLOAT:
    case GAVL_GRAYA_16:
    case GAVL_GRAYA_32:
    case GAVL_GRAYA_FLOAT:
    case GAVL_RGB_15:
    case GAVL_BGR_15:
    case GAVL_RGB_16:
    case GAVL_BGR_16:
    case GAVL_RGB_24:
    case GAVL_BGR_24:
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_RGBA_32:
    case GAVL_YUY2:
    case GAVL_UYVY:
    case GAVL_YUVA_32:
    case GAVL_YUVA_64:
    case GAVL_RGB_48:
    case GAVL_RGBA_64:
    case GAVL_RGB_FLOAT:
    case GAVL_RGBA_FLOAT:
    case GAVL_YUV_FLOAT:
    case GAVL_YUVA_FLOAT:
      return 1;
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUV_410_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_444_P:
    case GAVL_YUV_422_P_16:
    case GAVL_YUV_444_P_16:
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
      return 3;
      break;
    case GAVL_PIXELFORMAT_NONE:
      return 0;
      break;
    }
  return 0;
  }

void gavl_pixelformat_chroma_sub(gavl_pixelformat_t csp, int * sub_hp,
                                int * sub_vp)
  {
  int sub_h = 0;
  int sub_v = 0;
  
  switch(csp)
    {
    case GAVL_RGB_15:
    case GAVL_BGR_15:
    case GAVL_RGB_16:
    case GAVL_BGR_16:
    case GAVL_RGB_24:
    case GAVL_BGR_24:
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_RGBA_32:
    case GAVL_YUV_444_P:
    case GAVL_YUV_444_P_16:
    case GAVL_YUVJ_444_P:
    case GAVL_YUVA_32:
    case GAVL_YUVA_64:
    case GAVL_RGB_48:
    case GAVL_RGBA_64:
    case GAVL_RGB_FLOAT:
    case GAVL_RGBA_FLOAT:
    case GAVL_YUV_FLOAT:
    case GAVL_YUVA_FLOAT:
    case GAVL_GRAY_8:
    case GAVL_GRAY_16:
    case GAVL_GRAY_FLOAT:
    case GAVL_GRAYA_16:
    case GAVL_GRAYA_32:
    case GAVL_GRAYA_FLOAT:
      sub_h = 1;
      sub_v = 1;
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUVJ_420_P:
      sub_h = 2;
      sub_v = 2;
      break;
    case GAVL_YUV_422_P:
    case GAVL_YUV_422_P_16:
    case GAVL_YUVJ_422_P:
    case GAVL_YUY2:
    case GAVL_UYVY:
      sub_h = 2;
      sub_v = 1;
      break;
    case GAVL_YUV_411_P:
      sub_h = 4;
      sub_v = 1;
      break;
    case GAVL_YUV_410_P:
      sub_h = 4;
      sub_v = 4;
      break;
    case GAVL_PIXELFORMAT_NONE:
      break;
    }

  if(sub_hp)
    *sub_hp = sub_h;

  if(sub_vp)
    *sub_vp = sub_v;
  
  }



int gavl_num_pixelformats()
  {
  return num_pixelformats - 1;
  }

gavl_pixelformat_t gavl_get_pixelformat(int index)
  {
  return pixelformat_tab[index].pixelformat;
  }

const char * gavl_pixelformat_to_string(gavl_pixelformat_t pixelformat)
  {
  int i;
  for(i = 0; i < num_pixelformats; i++)
    {
    if(pixelformat_tab[i].pixelformat == pixelformat)
      return pixelformat_tab[i].name;
    }
  return NULL;
  }

const char * gavl_pixelformat_to_short_string(gavl_pixelformat_t pixelformat)
  {
  int i;
  for(i = 0; i < num_pixelformats; i++)
    {
    if(pixelformat_tab[i].pixelformat == pixelformat)
      return pixelformat_tab[i].short_name;
    }
  return pixelformat_tab[0].short_name;
  }

gavl_pixelformat_t gavl_string_to_pixelformat(const char * name)
  {
  int i;
  for(i = 0; i < num_pixelformats; i++)
    {
    if(!strcmp(pixelformat_tab[i].name, name))
      return pixelformat_tab[i].pixelformat;
    }
  return GAVL_PIXELFORMAT_NONE;
  }

gavl_pixelformat_t gavl_short_string_to_pixelformat(const char * name)
  {
  int i;
  for(i = 0; i < num_pixelformats; i++)
    {
    if(!strcmp(pixelformat_tab[i].short_name, name))
      return pixelformat_tab[i].pixelformat;
    }
  return GAVL_PIXELFORMAT_NONE;
  }

const uint32_t * gavl_pixelformat_get_masks(gavl_pixelformat_t pixelformat)
  {
  int i;
  for(i = 0; i < num_pixelformats; i++)
    {
    if(pixelformat_tab[i].pixelformat == pixelformat)
      return pixelformat_tab[i].masks;
    }
  return NULL;
  }

static gavl_pixelformat_function_table_t *
create_pixelformat_function_table(const gavl_video_options_t * opt,
                                 int width, int height)
  {
  gavl_pixelformat_function_table_t * csp_tab;
    
  csp_tab =
    calloc(1,sizeof(gavl_pixelformat_function_table_t));

  //  fprintf(stderr, "create_pixelformat_function_table, flags: %08x, q: %d\n",
  //          opt->accel_flags, opt->quality);
  
  /* Standard C-routines, always complete */
  if(opt->quality || (opt->accel_flags & GAVL_ACCEL_C))
    {
    gavl_init_rgb_rgb_funcs_c(csp_tab, opt);
    gavl_init_rgb_yuv_funcs_c(csp_tab, opt);
    gavl_init_yuv_rgb_funcs_c(csp_tab, opt);
    gavl_init_yuv_yuv_funcs_c(csp_tab, opt);
    gavl_init_rgb_gray_funcs_c(csp_tab, opt);
    gavl_init_gray_rgb_funcs_c(csp_tab, opt);
    gavl_init_yuv_gray_funcs_c(csp_tab, opt);
    gavl_init_gray_yuv_funcs_c(csp_tab, opt);
    gavl_init_gray_gray_funcs_c(csp_tab, opt);
    }
  
#ifdef HAVE_MMX
  if(opt->accel_flags & GAVL_ACCEL_MMX)
    {
    //    fprintf(stderr, "Init MMX functions %08x\n", real_accel_flags);
    gavl_init_rgb_rgb_funcs_mmx(csp_tab, width, opt);
    gavl_init_rgb_yuv_funcs_mmx(csp_tab, width, opt);
    gavl_init_yuv_yuv_funcs_mmx(csp_tab, width, opt);
    gavl_init_yuv_rgb_funcs_mmx(csp_tab, width, opt);
    }
  if(opt->accel_flags & GAVL_ACCEL_MMXEXT)
    {
    //    fprintf(stderr, "Init MMXEXT functions %08x\n", real_accel_flags);
    gavl_init_rgb_rgb_funcs_mmxext(csp_tab, width, opt);
    gavl_init_rgb_yuv_funcs_mmxext(csp_tab, width, opt);
    gavl_init_yuv_yuv_funcs_mmxext(csp_tab, width, opt);
    gavl_init_yuv_rgb_funcs_mmxext(csp_tab, width, opt);
    }
#endif
#ifdef HAVE_SSE
  if(opt->accel_flags & GAVL_ACCEL_SSE)
    {
    //    fprintf(stderr, "Init MMXEXT functions %08x\n", real_accel_flags);
    //    gavl_init_rgb_rgb_funcs_sse(csp_tab, opt);
    //    gavl_init_rgb_yuv_funcs_sse(csp_tab, opt);
    //    gavl_init_yuv_yuv_funcs_sse(csp_tab, opt);
    //    gavl_init_yuv_rgb_funcs_sse(csp_tab, opt);
    }
#endif
#ifdef HAVE_SSE3
  if(opt->accel_flags & GAVL_ACCEL_SSE3)
    {
    //    fprintf(stderr, "Init MMXEXT functions %08x\n", real_accel_flags);
    //    gavl_init_rgb_rgb_funcs_sse(csp_tab, opt);
    gavl_init_rgb_yuv_funcs_sse3(csp_tab, opt);
    //    gavl_init_yuv_yuv_funcs_sse(csp_tab, opt);
    //    gavl_init_yuv_rgb_funcs_sse(csp_tab, opt);
    }
#endif
  /* High quality */
  
  if((opt->quality > 3) ||
     ((!opt->quality) && (opt->accel_flags & GAVL_ACCEL_C_HQ)))
    {
    //    fprintf(stderr, "Init HQ\n");
    gavl_init_rgb_rgb_funcs_hq(csp_tab, opt);
    gavl_init_rgb_yuv_funcs_hq(csp_tab, opt);
    gavl_init_yuv_rgb_funcs_hq(csp_tab, opt);
    gavl_init_yuv_yuv_funcs_hq(csp_tab, opt);
    }


  return csp_tab;
  }

gavl_video_func_t
gavl_find_pixelformat_converter(const gavl_video_options_t * opt,
                               gavl_pixelformat_t input_pixelformat,
                               gavl_pixelformat_t output_pixelformat,
                               int width,
                               int height)
  {
  gavl_video_func_t ret = NULL;
  gavl_pixelformat_function_table_t * tab =
    create_pixelformat_function_table(opt, width, height);

  switch(input_pixelformat)
    {
    case GAVL_GRAY_8:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_16:
          ret = tab->gray_8_to_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->gray_8_to_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->gray_8_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->gray_8_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->gray_8_to_graya_float;
          break;
        case GAVL_RGB_15:
        case GAVL_BGR_15:
          ret = tab->gray_8_to_rgb_15;
          break;
        case GAVL_RGB_16:
        case GAVL_BGR_16:
          ret = tab->gray_8_to_rgb_16;
          break;
        case GAVL_RGB_24:
        case GAVL_BGR_24:
          ret = tab->gray_8_to_rgb_24;
          break;
        case GAVL_RGB_32:
        case GAVL_BGR_32:
          ret = tab->gray_8_to_rgb_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->gray_8_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->gray_8_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->gray_8_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->gray_8_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->gray_8_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->gray_8_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->gray_8_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->gray_8_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->gray_8_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->gray_8_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->gray_8_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_422_P:
        case GAVL_YUV_411_P:
        case GAVL_YUV_444_P:
          ret = tab->gray_8_to_y_8;
          break;
        case GAVL_YUV_422_P_16:
        case GAVL_YUV_444_P_16:
          ret = tab->gray_8_to_y_16;
          break;
        case GAVL_YUVJ_420_P:
        case GAVL_YUVJ_422_P:
        case GAVL_YUVJ_444_P:
          ret = tab->gray_8_to_yj_8;
          break;
          /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_GRAY_8:
          break;
        }
      break;
    case GAVL_GRAYA_16:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->graya_16_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->graya_16_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->graya_16_to_gray_float;
          break;
        case GAVL_GRAYA_32:
          ret = tab->graya_16_to_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->graya_16_to_float;
          break;
        case GAVL_RGB_15:
        case GAVL_BGR_15:
          ret = tab->graya_16_to_rgb_15;
          break;
        case GAVL_RGB_16:
        case GAVL_BGR_16:
          ret = tab->graya_16_to_rgb_16;
          break;
        case GAVL_RGB_24:
        case GAVL_BGR_24:
          ret = tab->graya_16_to_rgb_24;
          break;
        case GAVL_RGB_32:
        case GAVL_BGR_32:
          ret = tab->graya_16_to_rgb_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->graya_16_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->graya_16_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->graya_16_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->graya_16_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->graya_16_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->graya_16_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->graya_16_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->graya_16_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->graya_16_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->graya_16_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->graya_16_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_422_P:
        case GAVL_YUV_411_P:
        case GAVL_YUV_444_P:
          ret = tab->graya_16_to_y_8;
          break;
        case GAVL_YUV_422_P_16:
        case GAVL_YUV_444_P_16:
          ret = tab->graya_16_to_y_16;
          break;
        case GAVL_YUVJ_420_P:
        case GAVL_YUVJ_422_P:
        case GAVL_YUVJ_444_P:
          ret = tab->graya_16_to_yj_8;
          break;
          /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_GRAYA_16:
          break;
        }
      break;


    case GAVL_GRAY_16:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->gray_16_to_8;
          break;
        case GAVL_GRAY_FLOAT:
          ret  = tab->gray_16_to_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->gray_16_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->gray_16_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->gray_16_to_graya_float;
          break;
        case GAVL_RGB_15:
        case GAVL_BGR_15:
          ret = tab->gray_16_to_rgb_15;
          break;
        case GAVL_RGB_16:
        case GAVL_BGR_16:
          ret = tab->gray_16_to_rgb_16;
          break;
        case GAVL_RGB_24:
        case GAVL_BGR_24:
          ret = tab->gray_16_to_rgb_24;
          break;
        case GAVL_RGB_32:
        case GAVL_BGR_32:
          ret = tab->gray_16_to_rgb_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->gray_16_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->gray_16_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->gray_16_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->gray_16_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->gray_16_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->gray_16_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->gray_16_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->gray_16_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->gray_16_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->gray_16_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->gray_16_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_422_P:
        case GAVL_YUV_411_P:
        case GAVL_YUV_444_P:
          ret = tab->gray_16_to_y_8;
          break;
        case GAVL_YUV_422_P_16:
        case GAVL_YUV_444_P_16:
          ret = tab->gray_16_to_y_16;
          break;
        case GAVL_YUVJ_420_P:
        case GAVL_YUVJ_422_P:
        case GAVL_YUVJ_444_P:
          ret = tab->gray_16_to_yj_8;
          break;
          /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_GRAY_16:
          break;
        }
      break;
    case GAVL_GRAYA_32:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->graya_32_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->graya_32_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->graya_32_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->graya_32_to_16;
          break;
        case GAVL_GRAYA_FLOAT:
          ret  = tab->graya_32_to_float;
          break;
        case GAVL_RGB_15:
        case GAVL_BGR_15:
          ret = tab->graya_32_to_rgb_15;
          break;
        case GAVL_RGB_16:
        case GAVL_BGR_16:
          ret = tab->graya_32_to_rgb_16;
          break;
        case GAVL_RGB_24:
        case GAVL_BGR_24:
          ret = tab->graya_32_to_rgb_24;
          break;
        case GAVL_RGB_32:
        case GAVL_BGR_32:
          ret = tab->graya_32_to_rgb_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->graya_32_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->graya_32_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->graya_32_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->graya_32_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->graya_32_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->graya_32_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->graya_32_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->graya_32_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->graya_32_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->graya_32_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->graya_32_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_422_P:
        case GAVL_YUV_411_P:
        case GAVL_YUV_444_P:
          ret = tab->graya_32_to_y_8;
          break;
        case GAVL_YUV_422_P_16:
        case GAVL_YUV_444_P_16:
          ret = tab->graya_32_to_y_16;
          break;
        case GAVL_YUVJ_420_P:
        case GAVL_YUVJ_422_P:
        case GAVL_YUVJ_444_P:
          ret = tab->graya_32_to_yj_8;
          break;
          /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_GRAYA_32:
          break;
        }
      break;


    case GAVL_GRAY_FLOAT:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->gray_float_to_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->gray_float_to_16;
          break;
        case GAVL_GRAYA_16:
          ret = tab->gray_float_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->gray_float_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->gray_float_to_graya_float;
          break;
        case GAVL_RGB_15:
        case GAVL_BGR_15:
          ret = tab->gray_float_to_rgb_15;
          break;
        case GAVL_RGB_16:
        case GAVL_BGR_16:
          ret = tab->gray_float_to_rgb_16;
          break;
        case GAVL_RGB_24:
        case GAVL_BGR_24:
          ret = tab->gray_float_to_rgb_24;
          break;
        case GAVL_RGB_32:
        case GAVL_BGR_32:
          ret = tab->gray_float_to_rgb_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->gray_float_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->gray_float_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->gray_float_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->gray_float_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->gray_float_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->gray_float_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->gray_float_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->gray_float_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->gray_float_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->gray_float_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->gray_float_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_422_P:
        case GAVL_YUV_411_P:
        case GAVL_YUV_444_P:
          ret = tab->gray_float_to_y_8;
          break;
        case GAVL_YUV_422_P_16:
        case GAVL_YUV_444_P_16:
          ret = tab->gray_float_to_y_16;
          break;
        case GAVL_YUVJ_420_P:
        case GAVL_YUVJ_422_P:
        case GAVL_YUVJ_444_P:
          ret = tab->gray_float_to_yj_8;
          break;
          /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_GRAY_FLOAT:
          break;
        }
      break;

    case GAVL_GRAYA_FLOAT:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->graya_float_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->graya_float_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->graya_float_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->graya_float_to_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->graya_float_to_32;
          break;
        case GAVL_RGB_15:
        case GAVL_BGR_15:
          ret = tab->graya_float_to_rgb_15;
          break;
        case GAVL_RGB_16:
        case GAVL_BGR_16:
          ret = tab->graya_float_to_rgb_16;
          break;
        case GAVL_RGB_24:
        case GAVL_BGR_24:
          ret = tab->graya_float_to_rgb_24;
          break;
        case GAVL_RGB_32:
        case GAVL_BGR_32:
          ret = tab->graya_float_to_rgb_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->graya_float_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->graya_float_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->graya_float_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->graya_float_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->graya_float_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->graya_float_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->graya_float_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->graya_float_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->graya_float_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->graya_float_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->graya_float_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_422_P:
        case GAVL_YUV_411_P:
        case GAVL_YUV_444_P:
          ret = tab->graya_float_to_y_8;
          break;
        case GAVL_YUV_422_P_16:
        case GAVL_YUV_444_P_16:
          ret = tab->graya_float_to_y_16;
          break;
        case GAVL_YUVJ_420_P:
        case GAVL_YUVJ_422_P:
        case GAVL_YUVJ_444_P:
          ret = tab->graya_float_to_yj_8;
          break;
          /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_GRAYA_FLOAT:
          break;
        }
      break;
      
    case GAVL_RGB_15:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->rgb_15_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->rgb_15_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->rgb_15_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->rgb_15_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->rgb_15_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->rgb_15_to_graya_float;
          break;
        case GAVL_BGR_15:
          ret = tab->swap_rgb_15;
          break;
        case GAVL_RGB_16:
          ret = tab->rgb_15_to_16;
          break;
        case GAVL_BGR_16:
          ret = tab->rgb_15_to_16_swap;
          break;
        case GAVL_RGB_24:
          ret = tab->rgb_15_to_24;
          break;
        case GAVL_BGR_24:
          ret = tab->rgb_15_to_24_swap;
          break;
        case GAVL_RGB_32:
          ret = tab->rgb_15_to_32;
          break;
        case GAVL_BGR_32:
          ret = tab->rgb_15_to_32_swap;
          break;
        case GAVL_RGBA_32:
          ret = tab->rgb_15_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->rgb_15_to_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->rgb_15_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->rgb_15_to_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->rgb_15_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->rgb_15_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->rgb_15_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->rgb_15_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->rgb_15_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->rgb_15_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->rgb_15_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->rgb_15_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->rgb_15_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->rgb_15_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->rgb_15_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->rgb_15_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->rgb_15_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->rgb_15_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->rgb_15_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->rgb_15_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->rgb_15_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_RGB_15:
          break;
        }
      break;
    case GAVL_BGR_15:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->bgr_15_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->bgr_15_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->bgr_15_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->bgr_15_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->bgr_15_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->bgr_15_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->swap_rgb_15;
          break;
        case GAVL_RGB_16:
          ret = tab->rgb_15_to_16_swap;
          break;
        case GAVL_BGR_16:
          ret = tab->rgb_15_to_16;
          break;
        case GAVL_RGB_24:
          ret = tab->rgb_15_to_24_swap;
          break;
        case GAVL_BGR_24:
          ret = tab->rgb_15_to_24;
          break;
        case GAVL_RGB_32:
          ret = tab->rgb_15_to_32_swap;
          break;
        case GAVL_BGR_32:
          ret = tab->rgb_15_to_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->bgr_15_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->rgb_15_to_48_swap;
          break;
        case GAVL_RGBA_64:
          ret = tab->bgr_15_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->rgb_15_to_float_swap;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->bgr_15_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->bgr_15_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->bgr_15_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->bgr_15_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->bgr_15_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->bgr_15_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->bgr_15_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->bgr_15_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->bgr_15_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->bgr_15_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->bgr_15_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->bgr_15_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->bgr_15_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->bgr_15_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->bgr_15_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->bgr_15_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->bgr_15_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_BGR_15:
          break;
        }
      break;
    case GAVL_RGB_16:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->rgb_16_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->rgb_16_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->rgb_16_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->rgb_16_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->rgb_16_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->rgb_16_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->rgb_16_to_15;
          break;
        case GAVL_BGR_15:
          ret = tab->rgb_16_to_15_swap;
          break;
        case GAVL_BGR_16:
          ret = tab->swap_rgb_16;
          break;
        case GAVL_RGB_24:
          ret = tab->rgb_16_to_24;
          break;
        case GAVL_BGR_24:
          ret = tab->rgb_16_to_24_swap;
          break;
        case GAVL_RGB_32:
          ret = tab->rgb_16_to_32;
          break;
        case GAVL_BGR_32:
          ret = tab->rgb_16_to_32_swap;
          break;
        case GAVL_RGBA_32:
          ret = tab->rgb_16_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->rgb_16_to_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->rgb_16_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->rgb_16_to_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->rgb_16_to_rgba_float;
          break;

        case GAVL_YUY2:
          ret = tab->rgb_16_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->rgb_16_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->rgb_16_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->rgb_16_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->rgb_16_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->rgb_16_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->rgb_16_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->rgb_16_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->rgb_16_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->rgb_16_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->rgb_16_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->rgb_16_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->rgb_16_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->rgb_16_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->rgb_16_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->rgb_16_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_RGB_16:
          break;
        }
      break;
    case GAVL_BGR_16:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->bgr_16_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->bgr_16_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->bgr_16_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->bgr_16_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->bgr_16_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->bgr_16_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->rgb_16_to_15_swap;
          break;
        case GAVL_BGR_15:
          ret = tab->rgb_16_to_15;
          break;
        case GAVL_RGB_16:
          ret = tab->swap_rgb_16;
          break;
        case GAVL_RGB_24:
          ret = tab->rgb_16_to_24_swap;
          break;
        case GAVL_BGR_24:
          ret = tab->rgb_16_to_24;
          break;
        case GAVL_RGB_32:
          ret = tab->rgb_16_to_32_swap;
          break;
        case GAVL_BGR_32:
          ret = tab->rgb_16_to_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->bgr_16_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->rgb_16_to_48_swap;
          break;
        case GAVL_RGBA_64:
          ret = tab->bgr_16_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->rgb_16_to_float_swap;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->bgr_16_to_rgba_float;
          break;

        case GAVL_YUY2:
          ret = tab->bgr_16_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->bgr_16_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->bgr_16_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->bgr_16_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->bgr_16_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->bgr_16_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->bgr_16_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->bgr_16_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->bgr_16_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->bgr_16_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->bgr_16_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->bgr_16_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->bgr_16_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->bgr_16_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->bgr_16_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->bgr_16_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_BGR_16:
          break;
        }
      break;
    case GAVL_RGB_24:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->rgb_24_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->rgb_24_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->rgb_24_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->rgb_24_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->rgb_24_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->rgb_24_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->rgb_24_to_15;
          break;
        case GAVL_BGR_15:
          ret = tab->rgb_24_to_15_swap;
          break;
        case GAVL_RGB_16:
          ret = tab->rgb_24_to_16;
          break;
        case GAVL_BGR_16:
          ret = tab->rgb_24_to_16_swap;
          break;
        case GAVL_BGR_24:
          ret = tab->swap_rgb_24;
          break;
        case GAVL_RGB_32:
          ret = tab->rgb_24_to_32;
          break;
        case GAVL_BGR_32:
          ret = tab->rgb_24_to_32_swap;
          break;
        case GAVL_RGBA_32:
          ret = tab->rgb_24_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->rgb_24_to_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->rgb_24_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->rgb_24_to_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->rgb_24_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->rgb_24_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->rgb_24_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->rgb_24_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->rgb_24_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->rgb_24_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->rgb_24_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->rgb_24_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->rgb_24_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->rgb_24_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->rgb_24_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->rgb_24_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->rgb_24_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->rgb_24_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->rgb_24_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->rgb_24_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->rgb_24_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_RGB_24:
          break;
        }
      break;
    case GAVL_BGR_24:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->bgr_24_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->bgr_24_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->bgr_24_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->bgr_24_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->bgr_24_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->bgr_24_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->rgb_24_to_15_swap;
          break;
        case GAVL_BGR_15:
          ret = tab->rgb_24_to_15;
          break;
        case GAVL_RGB_16:
          ret = tab->rgb_24_to_16_swap;
          break;
        case GAVL_BGR_16:
          ret = tab->rgb_24_to_16;
          break;
        case GAVL_RGB_24:
          ret = tab->swap_rgb_24;
          break;
        case GAVL_RGB_32:
          ret = tab->rgb_24_to_32_swap;
          break;
        case GAVL_BGR_32:
          ret = tab->rgb_24_to_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->bgr_24_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->rgb_24_to_48_swap;
          break;
        case GAVL_RGBA_64:
          ret = tab->bgr_24_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->rgb_24_to_float_swap;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->bgr_24_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->bgr_24_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->bgr_24_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->bgr_24_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->bgr_24_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->bgr_24_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->bgr_24_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->bgr_24_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->bgr_24_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->bgr_24_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->bgr_24_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->bgr_24_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->bgr_24_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->bgr_24_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->bgr_24_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->bgr_24_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->bgr_24_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_BGR_24:
          break;
        }
      break;
    case GAVL_RGB_32:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->rgb_32_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->rgb_32_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->rgb_32_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->rgb_32_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->rgb_32_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->rgb_32_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->rgb_32_to_15;
          break;
        case GAVL_BGR_15:
          ret = tab->rgb_32_to_15_swap;
          break;
        case GAVL_RGB_16:
          ret = tab->rgb_32_to_16;
          break;
        case GAVL_BGR_16:
          ret = tab->rgb_32_to_16_swap;
          break;
        case GAVL_RGB_24:
          ret = tab->rgb_32_to_24;
          break;
        case GAVL_BGR_24:
          ret = tab->rgb_32_to_24_swap;
          break;
        case GAVL_BGR_32:
          ret = tab->swap_rgb_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->rgb_32_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->rgb_32_to_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->rgb_32_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->rgb_32_to_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->rgb_32_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->rgb_32_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->rgb_32_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->rgb_32_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->rgb_32_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->rgb_32_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->rgb_32_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->rgb_32_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->rgb_32_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->rgb_32_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->rgb_32_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->rgb_32_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->rgb_32_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->rgb_32_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->rgb_32_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->rgb_32_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->rgb_32_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_RGB_32:
          break;
        }
      break;
    case GAVL_BGR_32:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->bgr_32_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->bgr_32_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->bgr_32_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->bgr_32_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->bgr_32_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->bgr_32_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->rgb_32_to_15_swap;
          break;
        case GAVL_BGR_15:
          ret = tab->rgb_32_to_15;
          break;
        case GAVL_RGB_16:
          ret = tab->rgb_32_to_16_swap;
          break;
        case GAVL_BGR_16:
          ret = tab->rgb_32_to_16;
          break;
        case GAVL_RGB_24:
          ret = tab->rgb_32_to_24_swap;
          break;
        case GAVL_BGR_24:
          ret = tab->rgb_32_to_24;
          break;
        case GAVL_RGB_32:
          ret = tab->swap_rgb_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->bgr_32_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->rgb_32_to_48_swap;
          break;
        case GAVL_RGBA_64:
          ret = tab->bgr_32_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->rgb_32_to_float_swap;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->bgr_32_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->bgr_32_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->bgr_32_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->bgr_32_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->bgr_32_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->bgr_32_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->bgr_32_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->bgr_32_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->bgr_32_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->bgr_32_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->bgr_32_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->bgr_32_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->bgr_32_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->bgr_32_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->bgr_32_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->bgr_32_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->bgr_32_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_BGR_32:
          break;
        }
      break;
    case GAVL_RGBA_32:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->rgba_32_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->rgba_32_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->rgba_32_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->rgba_32_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->rgba_32_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->rgba_32_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->rgba_32_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->rgba_32_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->rgba_32_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->rgba_32_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->rgba_32_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->rgba_32_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->rgba_32_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->rgba_32_to_bgr_32;
          break;
        case GAVL_RGB_48:
          ret = tab->rgba_32_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->rgba_32_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->rgba_32_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->rgba_32_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->rgba_32_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->rgba_32_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->rgba_32_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->rgba_32_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->rgba_32_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->rgba_32_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->rgba_32_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->rgba_32_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->rgba_32_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->rgba_32_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->rgba_32_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->rgba_32_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->rgba_32_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->rgba_32_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->rgba_32_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->rgba_32_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_RGBA_32:
          break;
        }
      break;

    case GAVL_RGBA_64:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->rgba_64_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->rgba_64_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->rgba_64_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->rgba_64_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->rgba_64_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->rgba_64_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->rgba_64_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->rgba_64_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->rgba_64_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->rgba_64_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->rgba_64_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->rgba_64_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->rgba_64_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->rgba_64_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->rgba_64_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->rgba_64_to_rgb_48;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->rgba_64_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->rgba_64_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->rgba_64_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->rgba_64_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->rgba_64_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->rgba_64_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->rgba_64_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->rgba_64_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->rgba_64_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->rgba_64_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->rgba_64_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->rgba_64_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->rgba_64_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->rgba_64_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->rgba_64_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->rgba_64_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->rgba_64_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->rgba_64_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_RGBA_64:
          break;
        }
      break;

    case GAVL_RGBA_FLOAT:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->rgba_float_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->rgba_float_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->rgba_float_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->rgba_float_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->rgba_float_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->rgba_float_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->rgba_float_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->rgba_float_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->rgba_float_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->rgba_float_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->rgba_float_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->rgba_float_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->rgba_float_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->rgba_float_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->rgba_float_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->rgba_float_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->rgba_float_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->rgba_float_to_rgb_float;
          break;
        case GAVL_YUY2:
          ret = tab->rgba_float_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->rgba_float_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->rgba_float_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->rgba_float_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->rgba_float_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->rgba_float_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->rgba_float_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->rgba_float_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->rgba_float_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->rgba_float_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->rgba_float_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->rgba_float_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->rgba_float_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->rgba_float_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->rgba_float_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->rgba_float_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_RGBA_FLOAT:
          break;
        }
      break;

      
    case GAVL_RGB_48:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->rgb_48_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->rgb_48_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->rgb_48_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->rgb_48_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->rgb_48_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->rgb_48_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->rgb_48_to_15;
          break;
        case GAVL_BGR_15:
          ret = tab->rgb_48_to_15_swap;
          break;
        case GAVL_RGB_16:
          ret = tab->rgb_48_to_16;
          break;
        case GAVL_BGR_16:
          ret = tab->rgb_48_to_16_swap;
          break;
        case GAVL_RGB_24:
          ret = tab->rgb_48_to_24;
          break;
        case GAVL_BGR_24:
          ret = tab->rgb_48_to_24_swap;
          break;
        case GAVL_RGB_32:
          ret = tab->rgb_48_to_32;
          break;
        case GAVL_BGR_32:
          ret = tab->rgb_48_to_32_swap;
          break;
        case GAVL_RGBA_32:
          ret = tab->rgb_48_to_rgba_32;
          break;
        case GAVL_RGBA_64:
          ret = tab->rgb_48_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->rgb_48_to_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->rgb_48_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->rgb_48_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->rgb_48_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->rgb_48_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->rgb_48_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->rgb_48_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->rgb_48_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->rgb_48_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->rgb_48_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->rgb_48_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->rgb_48_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->rgb_48_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->rgb_48_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->rgb_48_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->rgb_48_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->rgb_48_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->rgb_48_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_RGB_48:
          break;
        }
      break;
    case GAVL_RGB_FLOAT:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->rgb_float_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->rgb_float_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->rgb_float_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->rgb_float_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->rgb_float_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->rgb_float_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->rgb_float_to_15;
          break;
        case GAVL_BGR_15:
          ret = tab->rgb_float_to_15_swap;
          break;
        case GAVL_RGB_16:
          ret = tab->rgb_float_to_16;
          break;
        case GAVL_BGR_16:
          ret = tab->rgb_float_to_16_swap;
          break;
        case GAVL_RGB_24:
          ret = tab->rgb_float_to_24;
          break;
        case GAVL_BGR_24:
          ret = tab->rgb_float_to_24_swap;
          break;
        case GAVL_RGB_32:
          ret = tab->rgb_float_to_32;
          break;
        case GAVL_BGR_32:
          ret = tab->rgb_float_to_32_swap;
          break;
        case GAVL_RGBA_32:
          ret = tab->rgb_float_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->rgb_float_to_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->rgb_float_to_rgba_64;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->rgb_float_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->rgb_float_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->rgb_float_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->rgb_float_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->rgb_float_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->rgb_float_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->rgb_float_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->rgb_float_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->rgb_float_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->rgb_float_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->rgb_float_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->rgb_float_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->rgb_float_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->rgb_float_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->rgb_float_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->rgb_float_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->rgb_float_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_RGB_FLOAT:
          break;
        }
      break;

      

    case GAVL_YUY2:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->yuy2_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->yuy2_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->yuy2_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->yuy2_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->yuy2_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->yuy2_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuy2_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuy2_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuy2_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuy2_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuy2_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuy2_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuy2_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuy2_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuy2_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuy2_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuy2_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuy2_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuy2_to_rgba_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuy2_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuy2_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuy2_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuy2_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuy2_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuy2_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuy2_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuy2_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuy2_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuy2_to_yuvj_444_p;
          break;
        case GAVL_UYVY:
          ret = tab->uyvy_to_yuy2;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuy2_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuy2_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuy2_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuy2_to_yuv_float;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUY2:
          break;
        }
      break;
    case GAVL_UYVY:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->uyvy_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->uyvy_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->uyvy_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->uyvy_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->uyvy_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->uyvy_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->uyvy_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->uyvy_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->uyvy_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->uyvy_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->uyvy_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->uyvy_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->uyvy_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->uyvy_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->uyvy_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->uyvy_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->uyvy_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->uyvy_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->uyvy_to_rgba_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->uyvy_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->uyvy_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->uyvy_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->uyvy_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->uyvy_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->uyvy_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->uyvy_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->uyvy_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->uyvy_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->uyvy_to_yuvj_444_p;
          break;
        case GAVL_YUY2:
          ret = tab->uyvy_to_yuy2;
          break;
        case GAVL_YUVA_32:
          ret = tab->uyvy_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->uyvy_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->uyvy_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->uyvy_to_yuv_float;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_UYVY:
          break;
        }
      break;

     case GAVL_YUVA_32:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->yuva_32_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->yuva_32_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->yuva_32_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->yuva_32_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->yuva_32_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->yuva_32_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuva_32_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuva_32_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuva_32_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuva_32_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuva_32_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuva_32_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuva_32_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuva_32_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuva_32_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuva_32_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuva_32_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuva_32_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuva_32_to_rgba_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuva_32_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuva_32_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuva_32_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuva_32_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuva_32_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuva_32_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuva_32_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuva_32_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuva_32_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuva_32_to_yuvj_444_p;
          break;
        case GAVL_YUY2:
          ret = tab->yuva_32_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuva_32_to_uyvy;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuva_32_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuva_32_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuva_32_to_yuv_float;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUVA_32:
          break;
        }
      break;

    case GAVL_YUVA_64:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->yuva_64_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->yuva_64_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->yuva_64_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->yuva_64_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->yuva_64_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->yuva_64_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuva_64_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuva_64_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuva_64_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuva_64_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuva_64_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuva_64_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuva_64_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuva_64_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuva_64_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuva_64_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuva_64_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuva_64_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuva_64_to_rgba_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuva_64_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuva_64_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuva_64_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuva_64_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuva_64_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuva_64_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuva_64_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuva_64_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuva_64_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuva_64_to_yuvj_444_p;
          break;
        case GAVL_YUY2:
          ret = tab->yuva_64_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuva_64_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuva_64_to_yuva_32;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuva_64_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuva_64_to_yuv_float;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUVA_64:
          break;
        }
      break;

    case GAVL_YUVA_FLOAT:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->yuva_float_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->yuva_float_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->yuva_float_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->yuva_float_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->yuva_float_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->yuva_float_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuva_float_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuva_float_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuva_float_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuva_float_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuva_float_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuva_float_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuva_float_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuva_float_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuva_float_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuva_float_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuva_float_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuva_float_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuva_float_to_rgba_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuva_float_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuva_float_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuva_float_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuva_float_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuva_float_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuva_float_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuva_float_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuva_float_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuva_float_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuva_float_to_yuvj_444_p;
          break;
        case GAVL_YUY2:
          ret = tab->yuva_float_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuva_float_to_uyvy;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuva_float_to_yuva_64;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuva_float_to_yuva_32;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuva_float_to_yuv_float;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUVA_FLOAT:
          break;
        }
      break;

    case GAVL_YUV_FLOAT:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->yuv_float_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->yuv_float_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->yuv_float_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->yuv_float_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->yuv_float_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->yuv_float_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuv_float_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuv_float_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuv_float_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuv_float_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuv_float_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuv_float_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuv_float_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuv_float_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuv_float_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuv_float_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuv_float_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuv_float_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuv_float_to_rgba_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuv_float_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuv_float_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuv_float_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuv_float_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuv_float_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuv_float_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuv_float_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuv_float_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuv_float_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuv_float_to_yuvj_444_p;
          break;
        case GAVL_YUY2:
          ret = tab->yuv_float_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuv_float_to_uyvy;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuv_float_to_yuva_64;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuv_float_to_yuva_32;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuv_float_to_yuva_float;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUV_FLOAT:
          break;
        }
      break;

    case GAVL_YUV_420_P:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->y_8_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->y_8_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->y_8_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->y_8_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->y_8_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->y_8_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuv_420_p_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuv_420_p_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuv_420_p_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuv_420_p_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuv_420_p_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuv_420_p_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuv_420_p_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuv_420_p_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuv_420_p_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuv_420_p_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuv_420_p_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuv_420_p_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuv_420_p_to_rgba_float;
          break;

        case GAVL_YUY2:
          ret = tab->yuv_420_p_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuv_420_p_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuv_420_p_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuv_420_p_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuv_420_p_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuv_420_p_to_yuv_float;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuv_420_p_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuv_420_p_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuv_420_p_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuv_420_p_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuv_420_p_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuv_420_p_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuv_420_p_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuv_420_p_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuv_420_p_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUV_420_P:
          break;
        }
      break;
    case GAVL_YUV_410_P:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->y_8_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->y_8_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->y_8_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->y_8_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->y_8_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->y_8_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuv_410_p_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuv_410_p_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuv_410_p_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuv_410_p_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuv_410_p_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuv_410_p_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuv_410_p_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuv_410_p_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuv_410_p_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuv_410_p_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuv_410_p_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuv_410_p_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuv_410_p_to_rgba_float;
          break;

        case GAVL_YUY2:
          ret = tab->yuv_410_p_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuv_410_p_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuv_410_p_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuv_410_p_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuv_410_p_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuv_410_p_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuv_410_p_to_yuv_420_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuv_410_p_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuv_410_p_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuv_410_p_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuv_410_p_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuv_410_p_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuv_410_p_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuv_410_p_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuv_410_p_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUV_410_P:
          break;
        }
      break;
    case GAVL_YUV_422_P:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->y_8_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->y_8_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->y_8_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->y_8_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->y_8_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->y_8_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuv_422_p_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuv_422_p_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuv_422_p_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuv_422_p_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuv_422_p_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuv_422_p_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuv_422_p_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuv_422_p_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuv_422_p_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuv_422_p_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuv_422_p_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuv_422_p_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuv_422_p_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->yuv_422_p_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuv_422_p_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuv_422_p_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuv_422_p_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuv_422_p_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuv_422_p_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuv_422_p_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuv_422_p_to_yuv_410_p;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuv_422_p_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuv_422_p_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuv_422_p_to_yuv_444_p_16;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuv_422_p_to_yuv_422_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuv_422_p_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuv_422_p_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuv_422_p_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUV_422_P:
          break;
        }
      break;

    case GAVL_YUV_422_P_16:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->y_16_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->y_16_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->y_16_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->y_16_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->y_16_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->y_16_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuv_422_p_16_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuv_422_p_16_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuv_422_p_16_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuv_422_p_16_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuv_422_p_16_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuv_422_p_16_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuv_422_p_16_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuv_422_p_16_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuv_422_p_16_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuv_422_p_16_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuv_422_p_16_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuv_422_p_16_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuv_422_p_16_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->yuv_422_p_16_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuv_422_p_16_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuv_422_p_16_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuv_422_p_16_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuv_422_p_16_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuv_422_p_16_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuv_422_p_16_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuv_422_p_16_to_yuv_410_p;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuv_422_p_16_to_yuv_411_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuv_422_p_16_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuv_422_p_16_to_yuv_444_p_16;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuv_422_p_16_to_yuv_422_p;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuv_422_p_16_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuv_422_p_16_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuv_422_p_16_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUV_422_P_16:
          break;
        }
      break;

    case GAVL_YUV_411_P:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->y_8_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->y_8_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->y_8_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->y_8_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->y_8_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->y_8_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuv_411_p_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuv_411_p_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuv_411_p_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuv_411_p_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuv_411_p_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuv_411_p_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuv_411_p_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuv_411_p_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuv_411_p_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuv_411_p_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuv_411_p_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuv_411_p_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuv_411_p_to_rgba_float;
          break;

        case GAVL_YUY2:
          ret = tab->yuv_411_p_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuv_411_p_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuv_411_p_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuv_411_p_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuv_411_p_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuv_411_p_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuv_411_p_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuv_411_p_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuv_411_p_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuv_411_p_to_yuv_422_p_16;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuv_411_p_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuv_411_p_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuv_411_p_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuv_411_p_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuv_411_p_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUV_411_P:
          break;
        }
      break;
    case GAVL_YUV_444_P:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->y_8_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->y_8_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->y_8_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->y_8_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->y_8_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->y_8_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuv_444_p_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuv_444_p_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuv_444_p_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuv_444_p_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuv_444_p_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuv_444_p_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuv_444_p_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuv_444_p_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuv_444_p_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuv_444_p_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuv_444_p_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuv_444_p_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuv_444_p_to_rgba_float;
          break;
         
        case GAVL_YUY2:
          ret = tab->yuv_444_p_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuv_444_p_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuv_444_p_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuv_444_p_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuv_444_p_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuv_444_p_to_yuv_float;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuv_444_p_to_yuv_410_p;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuv_444_p_to_yuv_420_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuv_444_p_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuv_444_p_to_yuv_422_p_16;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuv_444_p_to_yuv_444_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuv_444_p_to_yuv_411_p;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuv_444_p_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuv_444_p_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuv_444_p_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUV_444_P:
          break;
        }
      break;

    case GAVL_YUV_444_P_16:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->y_16_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->y_16_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->y_16_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->y_16_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->y_16_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->y_16_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuv_444_p_16_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuv_444_p_16_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuv_444_p_16_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuv_444_p_16_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuv_444_p_16_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuv_444_p_16_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuv_444_p_16_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuv_444_p_16_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuv_444_p_16_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuv_444_p_16_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuv_444_p_16_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuv_444_p_16_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuv_444_p_16_to_rgba_float;
          break;
         
        case GAVL_YUY2:
          ret = tab->yuv_444_p_16_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuv_444_p_16_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuv_444_p_16_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuv_444_p_16_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuv_444_p_16_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuv_444_p_16_to_yuv_float;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuv_444_p_16_to_yuv_410_p;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuv_444_p_16_to_yuv_420_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuv_444_p_16_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuv_444_p_16_to_yuv_422_p_16;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuv_444_p_16_to_yuv_444_p;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuv_444_p_16_to_yuv_411_p;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuv_444_p_16_to_yuvj_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuv_444_p_16_to_yuvj_422_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuv_444_p_16_to_yuvj_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUV_444_P_16:
          break;
        }
      break;

    case GAVL_YUVJ_420_P:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->yj_8_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->yj_8_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->yj_8_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->yj_8_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->yj_8_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->yj_8_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuvj_420_p_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuvj_420_p_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuvj_420_p_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuvj_420_p_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuvj_420_p_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuvj_420_p_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuvj_420_p_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuvj_420_p_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuvj_420_p_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuvj_420_p_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuvj_420_p_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuvj_420_p_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuvj_420_p_to_rgba_float;
          break;

        case GAVL_YUY2:
          ret = tab->yuvj_420_p_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuvj_420_p_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuvj_420_p_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuvj_420_p_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuvj_420_p_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuvj_420_p_to_yuv_float;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuvj_420_p_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuvj_420_p_to_yuv_422_p_16;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuvj_420_p_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuvj_420_p_to_yuv_444_p_16;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuvj_420_p_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuvj_420_p_to_yuv_410_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuv_420_p_to_yuv_422_p;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuv_420_p_to_yuv_411_p;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuv_420_p_to_yuv_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUVJ_420_P:
          break;
        }
      break;
    case GAVL_YUVJ_422_P:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->yj_8_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->yj_8_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->yj_8_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->yj_8_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->yj_8_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->yj_8_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuvj_422_p_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuvj_422_p_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuvj_422_p_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuvj_422_p_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuvj_422_p_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuvj_422_p_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuvj_422_p_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuvj_422_p_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuvj_422_p_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuvj_422_p_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuvj_422_p_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuvj_422_p_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuvj_422_p_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->yuvj_422_p_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuvj_422_p_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuvj_422_p_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuvj_422_p_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuvj_422_p_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuvj_422_p_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuvj_422_p_to_yuv_420_p;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuvj_422_p_to_yuv_411_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuvj_422_p_to_yuv_410_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuvj_422_p_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuvj_422_p_to_yuv_444_p_16;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuv_422_p_to_yuv_420_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuvj_422_p_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuvj_422_p_to_yuv_422_p_16;
          break;
        case GAVL_YUVJ_444_P:
          ret = tab->yuv_422_p_to_yuv_444_p;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUVJ_422_P:
          break;
        }
      break;
    case GAVL_YUVJ_444_P:
      switch(output_pixelformat)
        {
        case GAVL_GRAY_8:
          ret = tab->yj_8_to_gray_8;
          break;
        case GAVL_GRAY_16:
          ret = tab->yj_8_to_gray_16;
          break;
        case GAVL_GRAY_FLOAT:
          ret = tab->yj_8_to_gray_float;
          break;
        case GAVL_GRAYA_16:
          ret = tab->yj_8_to_graya_16;
          break;
        case GAVL_GRAYA_32:
          ret = tab->yj_8_to_graya_32;
          break;
        case GAVL_GRAYA_FLOAT:
          ret = tab->yj_8_to_graya_float;
          break;
        case GAVL_RGB_15:
          ret = tab->yuvj_444_p_to_rgb_15;
          break;
        case GAVL_BGR_15:
          ret = tab->yuvj_444_p_to_bgr_15;
          break;
        case GAVL_RGB_16:
          ret = tab->yuvj_444_p_to_rgb_16;
          break;
        case GAVL_BGR_16:
          ret = tab->yuvj_444_p_to_bgr_16;
          break;
        case GAVL_RGB_24:
          ret = tab->yuvj_444_p_to_rgb_24;
          break;
        case GAVL_BGR_24:
          ret = tab->yuvj_444_p_to_bgr_24;
          break;
        case GAVL_RGB_32:
          ret = tab->yuvj_444_p_to_rgb_32;
          break;
        case GAVL_BGR_32:
          ret = tab->yuvj_444_p_to_bgr_32;
          break;
        case GAVL_RGBA_32:
          ret = tab->yuvj_444_p_to_rgba_32;
          break;
        case GAVL_RGB_48:
          ret = tab->yuvj_444_p_to_rgb_48;
          break;
        case GAVL_RGBA_64:
          ret = tab->yuvj_444_p_to_rgba_64;
          break;
        case GAVL_RGB_FLOAT:
          ret = tab->yuvj_444_p_to_rgb_float;
          break;
        case GAVL_RGBA_FLOAT:
          ret = tab->yuvj_444_p_to_rgba_float;
          break;
        case GAVL_YUY2:
          ret = tab->yuvj_444_p_to_yuy2;
          break;
        case GAVL_UYVY:
          ret = tab->yuvj_444_p_to_uyvy;
          break;
        case GAVL_YUVA_32:
          ret = tab->yuvj_444_p_to_yuva_32;
          break;
        case GAVL_YUVA_64:
          ret = tab->yuvj_444_p_to_yuva_64;
          break;
        case GAVL_YUVA_FLOAT:
          ret = tab->yuvj_444_p_to_yuva_float;
          break;
        case GAVL_YUV_FLOAT:
          ret = tab->yuvj_444_p_to_yuv_float;
          break;
        case GAVL_YUV_420_P:
          ret = tab->yuvj_444_p_to_yuv_420_p;
          break;
        case GAVL_YUV_410_P:
          ret = tab->yuvj_444_p_to_yuv_410_p;
          break;
        case GAVL_YUV_422_P:
          ret = tab->yuvj_444_p_to_yuv_422_p;
          break;
        case GAVL_YUV_422_P_16:
          ret = tab->yuvj_444_p_to_yuv_422_p_16;
          break;
        case GAVL_YUV_411_P:
          ret = tab->yuvj_444_p_to_yuv_411_p;
          break;
        case GAVL_YUVJ_420_P:
          ret = tab->yuv_444_p_to_yuv_420_p;
          break;
        case GAVL_YUVJ_422_P:
          ret = tab->yuv_444_p_to_yuv_422_p;
          break;
        case GAVL_YUV_444_P:
          ret = tab->yuvj_444_p_to_yuv_444_p;
          break;
        case GAVL_YUV_444_P_16:
          ret = tab->yuvj_444_p_to_yuv_444_p_16;
          break;
        /* Keep GCC happy */
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_YUVJ_444_P:
          break;
        }
      break;

      
    case GAVL_PIXELFORMAT_NONE:
      break;
    }
  free(tab);  
  return ret;
  }

/* bytes_per_component is only valid for planar formats */
  
int gavl_pixelformat_bytes_per_component(gavl_pixelformat_t csp)
  {
  switch(csp)
    {
    case GAVL_PIXELFORMAT_NONE:
    case GAVL_RGB_15:
    case GAVL_BGR_15:
    case GAVL_RGB_16:
    case GAVL_BGR_16:
    case GAVL_RGB_24:
    case GAVL_BGR_24:
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_RGBA_32:
    case GAVL_RGB_48:
    case GAVL_RGBA_64:
    case GAVL_RGB_FLOAT:
    case GAVL_RGBA_FLOAT:
    case GAVL_YUY2:
    case GAVL_UYVY:
    case GAVL_YUVA_32:
    case GAVL_YUVA_64:
    case GAVL_YUVA_FLOAT:
    case GAVL_YUV_FLOAT:
    case GAVL_GRAY_8:
    case GAVL_GRAY_16:
    case GAVL_GRAY_FLOAT:
    case GAVL_GRAYA_16:
    case GAVL_GRAYA_32:
    case GAVL_GRAYA_FLOAT:
      return 0;
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_444_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_410_P:
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
      return 1;
      break;
    case GAVL_YUV_444_P_16:
    case GAVL_YUV_422_P_16:
      return 2;
    }
  return 0;
  }

/* bytes_per_pixel is only valid for packed formats */

int gavl_pixelformat_bytes_per_pixel(gavl_pixelformat_t csp)
  {
  switch(csp)
    {
    case GAVL_PIXELFORMAT_NONE:
      return 0;
      break;
    case GAVL_GRAY_8:
      return 1;
    case GAVL_RGB_15:
    case GAVL_BGR_15:
    case GAVL_RGB_16:
    case GAVL_BGR_16:
    case GAVL_GRAY_16:
    case GAVL_GRAYA_16:
      return 2;
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
      return 3;
      break;
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_RGBA_32:
    case GAVL_YUVA_32:
    case GAVL_GRAYA_32:
      return 4;
      break;
    case GAVL_GRAY_FLOAT:
      return sizeof(float);
      break;
    case GAVL_GRAYA_FLOAT:
      return 2*sizeof(float);
      break;
    case GAVL_RGB_48:
      return 6;
      break;
    case GAVL_RGBA_64:
    case GAVL_YUVA_64:
      return 8;
      break;
    case GAVL_RGB_FLOAT:
    case GAVL_YUV_FLOAT:
      return 3*sizeof(float);
      break;
    case GAVL_RGBA_FLOAT:
    case GAVL_YUVA_FLOAT:
      return 4*sizeof(float);
      break;
    case GAVL_YUY2:
    case GAVL_UYVY:
      return 2;
    case GAVL_YUV_420_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_422_P_16:
    case GAVL_YUV_444_P:
    case GAVL_YUV_444_P_16:
    case GAVL_YUV_411_P:
    case GAVL_YUV_410_P:
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
      return 0;
    }
  return 0;
  }

int gavl_pixelformat_bits_per_pixel(gavl_pixelformat_t pixelformat)
  {
  switch(pixelformat)
    {
    case GAVL_PIXELFORMAT_NONE:
      return 0;
      break;
    case GAVL_GRAY_8:
      return 8;
      break;
    case GAVL_RGB_15:
    case GAVL_BGR_15:
      return 15;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
    case GAVL_GRAY_16:
    case GAVL_GRAYA_16:
      return 16;
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
    case GAVL_RGB_32:
    case GAVL_BGR_32:
      return 24;
      break;
    case GAVL_RGBA_32:
    case GAVL_YUVA_32:
    case GAVL_GRAYA_32:
      return 32;
      break;
    case GAVL_RGB_48:
      return 48;
      break;
    case GAVL_RGBA_64:
    case GAVL_YUVA_64:
      return 64;
      break;
    case GAVL_GRAY_FLOAT:
      return 8*sizeof(float);
      break;
    case GAVL_GRAYA_FLOAT:
      return 8*2*sizeof(float);
      break;
    case GAVL_RGB_FLOAT:
    case GAVL_YUV_FLOAT:
      return 8*3*sizeof(float);
      break;
    case GAVL_RGBA_FLOAT:
    case GAVL_YUVA_FLOAT:
      return 8*4*sizeof(float);
      break;
    case GAVL_YUY2:
    case GAVL_UYVY:
    case GAVL_YUV_422_P:
    case GAVL_YUVJ_422_P:
      return 16;
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUVJ_420_P:
    case GAVL_YUV_411_P:
      return 12;
      break;
    case GAVL_YUV_444_P:
    case GAVL_YUVJ_444_P:
      return 24;
      break;
    case GAVL_YUV_422_P_16:
      return 32;
      break;
    case GAVL_YUV_444_P_16:
      return 48;
      break;
    case GAVL_YUV_410_P:
      return 9;
      break;
    }
  return 0;
  
  }

static int effective_bits_per_component(gavl_pixelformat_t fmt)
  {
  switch(fmt)
    {
    case GAVL_PIXELFORMAT_NONE:
      return 0;
      break;
    case GAVL_RGB_15:
    case GAVL_BGR_15:
      return 5;
      break;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
      return 6; // Rounded up
      break;
    case GAVL_GRAY_8:
    case GAVL_GRAYA_16:
    case GAVL_RGB_24:
    case GAVL_BGR_24:
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_RGBA_32:
    case GAVL_YUVA_32:
    case GAVL_YUY2:
    case GAVL_UYVY:
    case GAVL_YUV_422_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUV_420_P:
    case GAVL_YUVJ_420_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_444_P:
    case GAVL_YUVJ_444_P:
    case GAVL_YUV_410_P:
      return 8;
      break;
    case GAVL_GRAY_16:
    case GAVL_GRAYA_32:
    case GAVL_RGB_48:
    case GAVL_RGBA_64:
    case GAVL_YUVA_64:
    case GAVL_YUV_422_P_16:
    case GAVL_YUV_444_P_16:
      return 16;
      break;
    case GAVL_GRAY_FLOAT:
    case GAVL_GRAYA_FLOAT:
    case GAVL_RGB_FLOAT:
    case GAVL_YUV_FLOAT:
    case GAVL_RGBA_FLOAT:
    case GAVL_YUVA_FLOAT:
      return 8*sizeof(float);
      break;
    }
  return 0;

  } 

int gavl_pixelformat_conversion_penalty(gavl_pixelformat_t src,
                                        gavl_pixelformat_t dst)
  {
  int ret;
  int sub_h_src, sub_h_dst;
  int sub_v_src, sub_v_dst;
  int src_bits, dst_bits;
  
  if(src == dst)
    return 0;

  gavl_pixelformat_chroma_sub(src, &sub_h_src, &sub_v_src);
  gavl_pixelformat_chroma_sub(dst, &sub_h_dst, &sub_v_dst);

  ret = 0;
  
  /* Loosing the color is the worst */
  if(!gavl_pixelformat_is_gray(src) &&
     gavl_pixelformat_is_gray(dst))
    ret += 1;
  
  /* Loosing the alpha channel is second worst */
  ret <<= 1;
  if(gavl_pixelformat_has_alpha(src) &&
     !gavl_pixelformat_has_alpha(dst))
    ret += 1;

  /* Converting gray to colored format is bloat */
  ret <<= 1;
  if(gavl_pixelformat_is_gray(src) &&
     !gavl_pixelformat_is_gray(dst))
    ret += 1;
  
  /* Colorspace conversions aren't good either */

  ret <<= 1;
  if((gavl_pixelformat_is_rgb(src) && gavl_pixelformat_is_yuv(dst)) ||
     (gavl_pixelformat_is_yuv(src) && gavl_pixelformat_is_rgb(dst)))
    ret += 1;
  
  /* Adding an alpha channel bloat */
  ret <<= 1;
  if(!gavl_pixelformat_has_alpha(src) &&
     gavl_pixelformat_has_alpha(dst))
    ret += 1;
  
  /* Chroma subsampling is next */
  ret <<= 1;
  if((sub_h_src != sub_h_dst) || (sub_v_src != sub_v_dst))
    ret += 1;

  /* Bits per pixel difference is maximum 256
     (one extra bit for src_bits > dst_bits) */
  ret <<= 9;
  
  src_bits = effective_bits_per_component(src);
  dst_bits = effective_bits_per_component(dst);
  
  /* Increasing precision is bad... */
  if(src_bits < dst_bits)
    ret += (dst_bits - src_bits);
  /* ... but descreasing precision is worse */
  else if(src_bits > dst_bits)
    ret += (src_bits - dst_bits) * 2;

  /*
   *   Converting between YUV and YUVJ is done via lookup tables,
   *   (efficient but lossy)
   */

  ret <<= 1;
  if(gavl_pixelformat_is_yuv(src) &&
     gavl_pixelformat_is_yuv(dst) &&
     (gavl_pixelformat_is_jpeg_scaled(src) !=
      gavl_pixelformat_is_jpeg_scaled(dst)))
    ret += 1;
  
  /* Remaining differences should be packing format.
     Conversions here are lossless and efficient */
  ret <<= 1;
  ret += 1;
  return ret;
  }

gavl_pixelformat_t 
gavl_pixelformat_get_best(gavl_pixelformat_t src,
                          const gavl_pixelformat_t * dst_supported,
                          int *penalty)
  {
  int min_penalty;
  int min_index;
  int i, test;

  if(!dst_supported || (dst_supported[0] == GAVL_PIXELFORMAT_NONE))
    return GAVL_PIXELFORMAT_NONE;
  
  min_penalty =
    gavl_pixelformat_conversion_penalty(src,
                                        dst_supported[0]);
  min_index = 0;

  i = 1;
  while(dst_supported[i] != GAVL_PIXELFORMAT_NONE)
    {
    test = gavl_pixelformat_conversion_penalty(src,
                                               dst_supported[i]);
    if(test < min_penalty)
      {
      min_penalty = test;
      min_index = i;
      }
    i++;
    }
  if(penalty)
    *penalty = min_penalty;
  return dst_supported[min_index];
  }



/* Check if a pixelformat can be converted by simple scaling */

int gavl_pixelformat_can_scale(gavl_pixelformat_t in_csp, gavl_pixelformat_t out_csp)
  {
    int sub_v_in,  sub_h_in;
  int sub_v_out, sub_h_out;
  if(gavl_pixelformat_is_rgb(in_csp) ||
     gavl_pixelformat_is_rgb(out_csp))
    {
    return 0;
    }

  if(gavl_pixelformat_is_jpeg_scaled(in_csp) !=
     gavl_pixelformat_is_jpeg_scaled(out_csp))
    {
    return 0;
    }
  if(gavl_pixelformat_has_alpha(in_csp) !=
     gavl_pixelformat_has_alpha(out_csp))
    {
    return 0;
    }


  
  gavl_pixelformat_chroma_sub(in_csp, &sub_h_in, &sub_v_in);
  gavl_pixelformat_chroma_sub(out_csp, &sub_h_out, &sub_v_out);
  
  if((sub_h_in == sub_h_out) && (sub_v_in == sub_v_out))
    {
    return 0;
    }

  if(!gavl_pixelformat_is_planar(in_csp))
    {
    // fprintf(stderr, "BLUPPPP: %d %d\n", gavl_pixelformat_is_planar(out_csp),
    //         gavl_pixelformat_bytes_per_component(out_csp));
    
    if(gavl_pixelformat_is_planar(out_csp) &&
       (gavl_pixelformat_bytes_per_component(out_csp) == 1))
      return 1;
    else
      return 0;
    }
  else
    {
    if(!gavl_pixelformat_is_planar(out_csp) &&
       (gavl_pixelformat_bytes_per_component(in_csp) == 1))
      return 1;
    else if(gavl_pixelformat_bytes_per_component(in_csp) ==
            gavl_pixelformat_bytes_per_component(out_csp))
      return 1;
    else
      return 0;
    }
  return 0;
  }

/*
 *  Return a pixelformat (or GAVL_PIXELFORMAT_NONE) as an intermediate pixelformat
 *  for which the conversion quality can be improved. E.g. instead of
 *  RGB -> YUV420P, we can do RGB -> YUV444P -> YUV420P with proper chroma scaling
 */

gavl_pixelformat_t gavl_pixelformat_get_intermediate(gavl_pixelformat_t in_csp,
                                                   gavl_pixelformat_t out_csp)
  {
  switch(in_csp)
    {
    case GAVL_PIXELFORMAT_NONE: return GAVL_PIXELFORMAT_NONE; break;
    case GAVL_GRAY_8:
    case GAVL_GRAY_16:
    case GAVL_GRAY_FLOAT:
    case GAVL_GRAYA_16:
    case GAVL_GRAYA_32:
    case GAVL_GRAYA_FLOAT:
    case GAVL_RGB_15:
    case GAVL_BGR_15:
    case GAVL_RGB_16:
    case GAVL_BGR_16:
    case GAVL_RGB_24:
    case GAVL_BGR_24:
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_RGBA_32:
    case GAVL_RGB_48:
    case GAVL_RGBA_64:
    case GAVL_RGB_FLOAT:
    case GAVL_RGBA_FLOAT:
    case GAVL_YUVA_32:
    case GAVL_YUVA_64:
    case GAVL_YUVA_FLOAT:
    case GAVL_YUV_FLOAT:
      /*4:4:4 -> */
      switch(out_csp)
        {
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_RGB_15:
        case GAVL_BGR_15:
        case GAVL_RGB_16:
        case GAVL_BGR_16:
        case GAVL_RGB_24:
        case GAVL_BGR_24:
        case GAVL_RGB_32:
        case GAVL_BGR_32:
        case GAVL_RGB_48:
        case GAVL_RGBA_64:
        case GAVL_RGB_FLOAT:
        case GAVL_RGBA_FLOAT:
        case GAVL_RGBA_32:
        case GAVL_YUVA_32:
        case GAVL_YUVA_64:
        case GAVL_YUVA_FLOAT:
        case GAVL_YUV_FLOAT:
        case GAVL_YUV_444_P:
        case GAVL_YUVJ_444_P:
        case GAVL_YUV_444_P_16:
        case GAVL_GRAY_8:
        case GAVL_GRAY_16:
        case GAVL_GRAY_FLOAT:
        case GAVL_GRAYA_16:
        case GAVL_GRAYA_32:
        case GAVL_GRAYA_FLOAT:
          return GAVL_PIXELFORMAT_NONE; break;
        case GAVL_YUY2:
        case GAVL_UYVY:
        case GAVL_YUV_420_P:
        case GAVL_YUV_422_P:
        case GAVL_YUV_411_P:
        case GAVL_YUV_410_P:
          return GAVL_YUV_444_P; break;
        case GAVL_YUVJ_420_P:
        case GAVL_YUVJ_422_P:
          return GAVL_YUVJ_444_P; break;
        case GAVL_YUV_422_P_16:
          return GAVL_YUV_444_P_16; break;
        }
      break;
    case GAVL_YUY2:
    case GAVL_UYVY:
    case GAVL_YUV_422_P:
      switch(out_csp)
        {
        case GAVL_PIXELFORMAT_NONE:
          return GAVL_PIXELFORMAT_NONE; break;
          /* YUV422 -> RGB */
        case GAVL_RGB_15:
        case GAVL_BGR_15:
        case GAVL_RGB_16:
        case GAVL_BGR_16:
        case GAVL_RGB_24:
        case GAVL_BGR_24:
        case GAVL_RGB_32:
        case GAVL_BGR_32:
        case GAVL_RGB_48:
        case GAVL_RGBA_64:
        case GAVL_RGB_FLOAT:
        case GAVL_RGBA_FLOAT:
        case GAVL_RGBA_32:
        case GAVL_YUVA_32:
          return GAVL_YUV_444_P; break;
        case GAVL_YUV_422_P:
        case GAVL_YUV_422_P_16:
        case GAVL_YUVJ_422_P:
        case GAVL_YUY2:
        case GAVL_UYVY:
        case GAVL_YUV_420_P:
        case GAVL_YUV_411_P:
        case GAVL_YUV_410_P:
        case GAVL_GRAY_8:
        case GAVL_GRAY_16:
        case GAVL_GRAY_FLOAT:
        case GAVL_GRAYA_16:
        case GAVL_GRAYA_32:
        case GAVL_GRAYA_FLOAT:
          return GAVL_PIXELFORMAT_NONE; break;
        case GAVL_YUV_444_P_16:
        case GAVL_YUVA_64:
        case GAVL_YUVA_FLOAT:
        case GAVL_YUV_FLOAT:
          return GAVL_YUV_422_P_16; break;
        case GAVL_YUV_444_P:
          return GAVL_YUV_422_P; break;
        case GAVL_YUVJ_444_P:
          return GAVL_YUVJ_422_P; break;
        case GAVL_YUVJ_420_P:
          return GAVL_YUVJ_422_P; break;
        }
      break;
    case GAVL_YUV_420_P:
      switch(out_csp)
        {
        case GAVL_PIXELFORMAT_NONE:
          return GAVL_PIXELFORMAT_NONE; break;
          /* YUV420 -> RGB */
        case GAVL_RGB_15:
        case GAVL_BGR_15:
        case GAVL_RGB_16:
        case GAVL_BGR_16:
        case GAVL_RGB_24:
        case GAVL_BGR_24:
        case GAVL_RGB_32:
        case GAVL_BGR_32:
        case GAVL_RGB_48:
        case GAVL_RGBA_64:
        case GAVL_RGB_FLOAT:
        case GAVL_RGBA_FLOAT:
        case GAVL_RGBA_32:
        case GAVL_YUVA_32:
          return GAVL_YUV_444_P; break;
        case GAVL_YUV_422_P:
        case GAVL_YUY2:
        case GAVL_UYVY:
        case GAVL_YUV_420_P:
        case GAVL_YUV_411_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_444_P:
        case GAVL_YUVJ_420_P:
        case GAVL_GRAY_8:
        case GAVL_GRAY_16:
        case GAVL_GRAY_FLOAT:
        case GAVL_GRAYA_16:
        case GAVL_GRAYA_32:
        case GAVL_GRAYA_FLOAT:
          return GAVL_PIXELFORMAT_NONE; break;
        case GAVL_YUV_422_P_16:
          return GAVL_YUV_422_P; break;
        case GAVL_YUVJ_422_P:
          return GAVL_YUVJ_420_P; break;
        case GAVL_YUV_444_P_16:
        case GAVL_YUVA_64:
        case GAVL_YUVA_FLOAT:
        case GAVL_YUV_FLOAT:
          return GAVL_YUV_444_P; break;
        case GAVL_YUVJ_444_P:
          return GAVL_YUVJ_420_P; break;
        }
      break;
    case GAVL_YUV_444_P:
      switch(out_csp)
        {
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_RGB_15:
        case GAVL_BGR_15:
        case GAVL_RGB_16:
        case GAVL_BGR_16:
        case GAVL_RGB_24:
        case GAVL_BGR_24:
        case GAVL_RGB_32:
        case GAVL_BGR_32:
        case GAVL_RGB_48:
        case GAVL_RGBA_64:
        case GAVL_RGB_FLOAT:
        case GAVL_RGBA_FLOAT:
        case GAVL_RGBA_32:
        case GAVL_YUVA_32:
        case GAVL_GRAY_8:
        case GAVL_GRAY_16:
        case GAVL_GRAY_FLOAT:
        case GAVL_GRAYA_16:
        case GAVL_GRAYA_32:
        case GAVL_GRAYA_FLOAT:
          return GAVL_PIXELFORMAT_NONE; break;
        case GAVL_YUV_422_P:
        case GAVL_YUY2:
        case GAVL_UYVY:
        case GAVL_YUV_420_P:
        case GAVL_YUV_411_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_444_P:
        case GAVL_YUVJ_444_P:
        case GAVL_YUV_444_P_16:
          return GAVL_PIXELFORMAT_NONE; break;
        case GAVL_YUV_422_P_16:
        case GAVL_YUVA_64:
        case GAVL_YUVA_FLOAT:
        case GAVL_YUV_FLOAT:
          return GAVL_YUV_422_P; break;
        case GAVL_YUVJ_422_P:
          return GAVL_YUVJ_420_P; break;
        case GAVL_YUVJ_420_P:
          return GAVL_YUVJ_444_P; break;
        }
      break;
      
    case GAVL_YUV_411_P:
    case GAVL_YUV_410_P:
      switch(out_csp)
        {
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_GRAY_8:
        case GAVL_GRAY_16:
        case GAVL_GRAY_FLOAT:
        case GAVL_GRAYA_16:
        case GAVL_GRAYA_32:
        case GAVL_GRAYA_FLOAT:
          return GAVL_PIXELFORMAT_NONE; break;
        case GAVL_RGB_15:
        case GAVL_BGR_15:
        case GAVL_RGB_16:
        case GAVL_BGR_16:
        case GAVL_RGB_24:
        case GAVL_BGR_24:
        case GAVL_RGB_32:
        case GAVL_BGR_32:
        case GAVL_RGB_48:
        case GAVL_RGBA_64:
        case GAVL_RGB_FLOAT:
        case GAVL_RGBA_FLOAT:
        case GAVL_RGBA_32:
        case GAVL_YUVA_32:
          return GAVL_YUV_444_P; break;
        case GAVL_YUV_422_P:
        case GAVL_YUY2:
        case GAVL_UYVY:
        case GAVL_YUV_420_P:
        case GAVL_YUV_411_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_444_P:
          return GAVL_PIXELFORMAT_NONE; break;

        case GAVL_YUVJ_444_P:
          return GAVL_YUV_444_P; break;
        case GAVL_YUV_422_P_16:
          return GAVL_YUV_422_P; break;
        case GAVL_YUVJ_422_P:
          return GAVL_YUV_422_P; break;
        case GAVL_YUV_444_P_16:
        case GAVL_YUVA_64:
        case GAVL_YUVA_FLOAT:
        case GAVL_YUV_FLOAT:
          return GAVL_YUV_444_P; break;
        case GAVL_YUVJ_420_P:
          return GAVL_YUV_420_P; break;
        }
      break;
    case GAVL_YUVJ_420_P:
      switch(out_csp)
        {
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_GRAY_8:
        case GAVL_GRAY_16:
        case GAVL_GRAY_FLOAT:
        case GAVL_GRAYA_16:
        case GAVL_GRAYA_32:
        case GAVL_GRAYA_FLOAT:
          return GAVL_PIXELFORMAT_NONE; break;
        case GAVL_RGB_15:
        case GAVL_BGR_15:
        case GAVL_RGB_16:
        case GAVL_BGR_16:
        case GAVL_RGB_24:
        case GAVL_BGR_24:
        case GAVL_RGB_32:
        case GAVL_BGR_32:
        case GAVL_RGB_48:
        case GAVL_RGBA_64:
        case GAVL_RGB_FLOAT:
        case GAVL_RGBA_FLOAT:
        case GAVL_RGBA_32:
        case GAVL_YUVA_32:
          return GAVL_YUVJ_444_P; break;
        case GAVL_YUV_422_P:
        case GAVL_YUY2:
        case GAVL_UYVY:
        case GAVL_YUV_411_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_444_P:
          return GAVL_YUV_420_P; break;
        case GAVL_YUV_420_P:
        case GAVL_YUVJ_444_P:
        case GAVL_YUVJ_422_P:
        case GAVL_YUVJ_420_P:
          return GAVL_PIXELFORMAT_NONE; break;

        case GAVL_YUV_422_P_16:
          return GAVL_YUVJ_422_P; break;
        case GAVL_YUV_444_P_16:
        case GAVL_YUVA_64:
        case GAVL_YUVA_FLOAT:
        case GAVL_YUV_FLOAT:
          return GAVL_YUVJ_444_P; break;
        }
      break;
      
    case GAVL_YUVJ_422_P:
      switch(out_csp)
        {
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_GRAY_8:
        case GAVL_GRAY_16:
        case GAVL_GRAY_FLOAT:
        case GAVL_GRAYA_16:
        case GAVL_GRAYA_32:
        case GAVL_GRAYA_FLOAT:
          return GAVL_PIXELFORMAT_NONE; break;
        case GAVL_RGB_15:
        case GAVL_BGR_15:
        case GAVL_RGB_16:
        case GAVL_BGR_16:
        case GAVL_RGB_24:
        case GAVL_BGR_24:
        case GAVL_RGB_32:
        case GAVL_BGR_32:
        case GAVL_RGB_48:
        case GAVL_RGBA_64:
        case GAVL_RGB_FLOAT:
        case GAVL_RGBA_FLOAT:
        case GAVL_RGBA_32:
        case GAVL_YUVA_32:
          return GAVL_YUVJ_444_P; break;
        case GAVL_YUV_411_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_444_P:
        case GAVL_YUV_420_P:
          return GAVL_YUV_422_P; break;
        case GAVL_YUV_422_P:
        case GAVL_YUY2:
        case GAVL_UYVY:
        case GAVL_YUVJ_444_P:
        case GAVL_YUVJ_422_P:
        case GAVL_YUVJ_420_P:
        case GAVL_YUV_422_P_16:
          return GAVL_PIXELFORMAT_NONE; break;
        case GAVL_YUV_444_P_16:
        case GAVL_YUVA_64:
        case GAVL_YUVA_FLOAT:
        case GAVL_YUV_FLOAT:
          return GAVL_YUV_422_P_16; break;
        }
      break;
    case GAVL_YUVJ_444_P:
      switch(out_csp)
        {
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_GRAY_8:
        case GAVL_GRAY_16:
        case GAVL_GRAY_FLOAT:
        case GAVL_GRAYA_16:
        case GAVL_GRAYA_32:
        case GAVL_GRAYA_FLOAT:
        case GAVL_RGB_15:
        case GAVL_BGR_15:
        case GAVL_RGB_16:
        case GAVL_BGR_16:
        case GAVL_RGB_24:
        case GAVL_BGR_24:
        case GAVL_RGB_32:
        case GAVL_BGR_32:
        case GAVL_RGB_48:
        case GAVL_RGBA_64:
        case GAVL_RGB_FLOAT:
        case GAVL_RGBA_FLOAT:
        case GAVL_RGBA_32:
        case GAVL_YUVA_32:
        case GAVL_YUV_444_P:
        case GAVL_YUV_444_P_16:
        case GAVL_YUVA_64:
        case GAVL_YUVA_FLOAT:
        case GAVL_YUV_FLOAT:
          return GAVL_PIXELFORMAT_NONE; break;
        case GAVL_YUV_411_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_420_P:
          return GAVL_YUV_444_P; break;
        case GAVL_YUV_422_P:
        case GAVL_YUY2:
        case GAVL_UYVY:
        case GAVL_YUVJ_444_P:
        case GAVL_YUVJ_422_P:
        case GAVL_YUVJ_420_P:
          return GAVL_PIXELFORMAT_NONE; break;

        case GAVL_YUV_422_P_16:
          return GAVL_YUVJ_422_P; break;
          return GAVL_YUV_422_P_16; break;
        }
      break;
    case GAVL_YUV_444_P_16:
      switch(out_csp)
        {
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_GRAY_8:
        case GAVL_GRAY_16:
        case GAVL_GRAY_FLOAT:
        case GAVL_GRAYA_16:
        case GAVL_GRAYA_32:
        case GAVL_GRAYA_FLOAT:
        case GAVL_RGB_15:
        case GAVL_BGR_15:
        case GAVL_RGB_16:
        case GAVL_BGR_16:
        case GAVL_RGB_24:
        case GAVL_BGR_24:
        case GAVL_RGB_32:
        case GAVL_BGR_32:
        case GAVL_RGB_48:
        case GAVL_RGBA_64:
        case GAVL_RGB_FLOAT:
        case GAVL_RGBA_FLOAT:
        case GAVL_RGBA_32:
        case GAVL_YUVA_32:
        case GAVL_YUV_444_P:
        case GAVL_YUV_444_P_16:
        case GAVL_YUVJ_444_P:
        case GAVL_YUV_422_P_16:
        case GAVL_YUVA_64:
        case GAVL_YUVA_FLOAT:
        case GAVL_YUV_FLOAT:
          return GAVL_PIXELFORMAT_NONE; break;
        case GAVL_YUV_422_P:
        case GAVL_YUY2:
        case GAVL_UYVY:
        case GAVL_YUV_411_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_420_P:
          return GAVL_YUV_444_P; break;
        case GAVL_YUVJ_422_P:
        case GAVL_YUVJ_420_P:
          return GAVL_YUVJ_444_P; break;
        }
      break;
    case GAVL_YUV_422_P_16:
      switch(out_csp)
        {
        case GAVL_PIXELFORMAT_NONE:
        case GAVL_RGB_15:
        case GAVL_BGR_15:
        case GAVL_RGB_16:
        case GAVL_BGR_16:
        case GAVL_RGB_24:
        case GAVL_BGR_24:
        case GAVL_RGB_32:
        case GAVL_BGR_32:
        case GAVL_RGB_48:
        case GAVL_RGBA_64:
        case GAVL_RGB_FLOAT:
        case GAVL_RGBA_FLOAT:
        case GAVL_RGBA_32:
        case GAVL_YUVA_32:
        case GAVL_YUV_444_P:
        case GAVL_YUVJ_444_P:
        case GAVL_YUVA_64:
        case GAVL_YUVA_FLOAT:
        case GAVL_YUV_FLOAT:
          return GAVL_YUV_444_P_16; break;
        case GAVL_YUV_444_P_16:
        case GAVL_YUV_422_P_16:
        case GAVL_YUV_422_P:
        case GAVL_YUY2:
        case GAVL_UYVY:
        case GAVL_YUVJ_422_P:
        case GAVL_GRAY_8:
        case GAVL_GRAY_16:
        case GAVL_GRAY_FLOAT:
        case GAVL_GRAYA_16:
        case GAVL_GRAYA_32:
        case GAVL_GRAYA_FLOAT:
          return GAVL_PIXELFORMAT_NONE; break;
        case GAVL_YUV_411_P:
        case GAVL_YUV_410_P:
        case GAVL_YUV_420_P:
          return GAVL_YUV_422_P; break;
        case GAVL_YUVJ_420_P:
          return GAVL_YUVJ_422_P; break;
        }

      break;
    }
  return GAVL_PIXELFORMAT_NONE;
  }

void gavl_pixelformat_get_offset(gavl_pixelformat_t pixelformat,
                                 int plane,
                                 int * advance, int * offset)
  {
  switch(pixelformat)
    {
    case GAVL_PIXELFORMAT_NONE:
      break;
    case GAVL_RGB_15:
    case GAVL_BGR_15:
      *advance = 2;
      *offset = 0;
      break;
    case GAVL_GRAY_8:
      *advance = 1;
      *offset = 0;
      break;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
    case GAVL_GRAY_16:
    case GAVL_GRAYA_16:
      *advance = 2;
      *offset = 0;
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
      *advance = 3;
      *offset = 0;
      break;
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_YUVA_32:
    case GAVL_RGBA_32:
    case GAVL_GRAYA_32:
      *advance = 4;
      *offset = 0;
      break;
    case GAVL_YUY2:
      switch(plane)
        {
        /* YUYV */
        case 0:
          *advance = 2;
          *offset = 0;
          break;
        case 1:
          *advance = 4;
          *offset = 1;
          break;
        case 2:
          *advance = 4;
          *offset = 3;
          break;
        }
      break;
    case GAVL_UYVY:
      switch(plane)
        {
        /* UYVY */
        case 0:
          *advance = 2;
          *offset = 1;
          break;
        case 1:
          *advance = 4;
          *offset = 0;
          break;
        case 2:
          *advance = 4;
          *offset = 2;
          break;
        }
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_444_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_410_P:
      *advance = 1;
      *offset = 0;
      break;
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
      *advance = 1;
      *offset = 0;
      break;
    case GAVL_YUV_444_P_16:
    case GAVL_YUV_422_P_16:
      *advance = 2;
      *offset = 0;
      break;
    case GAVL_RGB_48:
      *advance = 6;
      *offset = 0;
      break;
    case GAVL_RGBA_64:
    case GAVL_YUVA_64:
      *advance = 8;
      *offset = 0;
      break;
    case GAVL_GRAY_FLOAT:
      *advance = sizeof(float);
      *offset = 0;
      break;
    case GAVL_GRAYA_FLOAT:
      *advance = 2 * sizeof(float);
      *offset = 0;
      break;
    case GAVL_RGB_FLOAT:
    case GAVL_YUV_FLOAT:
      *advance = 3 * sizeof(float);
      *offset = 0;
      break;
    case GAVL_RGBA_FLOAT:
    case GAVL_YUVA_FLOAT:
      *advance = 4 * sizeof(float);
      *offset = 0;
      break;
    }

  }


GAVL_PUBLIC
int gavl_pixelformat_num_channels(gavl_pixelformat_t pixelformat)
  {
  int ret = gavl_pixelformat_is_gray(pixelformat) ? 1 : 3;
  if(gavl_pixelformat_has_alpha(pixelformat))
    ret++;
  return ret;
  }



GAVL_PUBLIC gavl_color_channel_t
gavl_pixelformat_get_channel(gavl_pixelformat_t pixelformat, int index)
  {
  int num = gavl_pixelformat_num_channels(pixelformat);
  if((index < 0) || (index >= num))
    return GAVL_CCH_NONE;
  
  if(gavl_pixelformat_has_alpha(pixelformat) &&
     (index == num-1))
    return GAVL_CCH_ALPHA;
  
  if(gavl_pixelformat_is_gray(pixelformat))
    {
    return GAVL_CCH_Y;
    }
  else if(gavl_pixelformat_is_rgb(pixelformat))
    {
    switch(index)
      {
      case 0:
        return GAVL_CCH_RED;
        break;
      case 1:
        return GAVL_CCH_GREEN;
        break;
      case 2:
        return GAVL_CCH_BLUE;
        break;
      }
    }
  else if(gavl_pixelformat_is_yuv(pixelformat))
    {
    switch(index)
      {
      case 0:
        return GAVL_CCH_Y;
        break;
      case 1:
        return GAVL_CCH_CB;
        break;
      case 2:
        return GAVL_CCH_CR;
        break;
      }
    }
  return GAVL_CCH_NONE;
  }
