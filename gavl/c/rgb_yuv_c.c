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

#include <gavl/gavl.h>
#include <video.h>
#include <colorspace.h>

#include "colorspace_tables.h"
#include "colorspace_macros.h"


#define INIT_RGBA_FLOAT float anti_alpha;                               \
  float background_r=ctx->options->background_float[0];                \
  float background_g=ctx->options->background_float[1];                 \
  float background_b=ctx->options->background_float[2];

/****************************************************
 * Conversions to YUY2
 ****************************************************/

#ifndef HQ

/* rgb_15_to_yuy2_c */

#define FUNC_NAME   rgb_15_to_yuy2_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(RGB15_TO_R_8(src[0]),\
               RGB15_TO_G_8(src[0]),\
               RGB15_TO_B_8(src[0]),dst[0],dst[1],dst[3])\
    RGB_24_TO_Y_8(RGB15_TO_R_8(src[1]),RGB15_TO_G_8(src[1]),RGB15_TO_B_8(src[1]),dst[2])\

#include "../csp_packed_packed.h"

/* bgr_15_to_yuy2_c */

#define FUNC_NAME   bgr_15_to_yuy2_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(BGR15_TO_R_8(src[0]),\
               BGR15_TO_G_8(src[0]),\
               BGR15_TO_B_8(src[0]),dst[0],dst[1],dst[3])\
    RGB_24_TO_Y_8(BGR15_TO_R_8(src[1]),BGR15_TO_G_8(src[1]),BGR15_TO_B_8(src[1]),dst[2])\

#include "../csp_packed_packed.h"

/* rgb_16_to_yuy2_c */

#define FUNC_NAME   rgb_16_to_yuy2_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(RGB16_TO_R_8(src[0]),\
               RGB16_TO_G_8(src[0]),\
               RGB16_TO_B_8(src[0]),dst[0],dst[1],dst[3])\
    RGB_24_TO_Y_8(RGB16_TO_R_8(src[1]),RGB16_TO_G_8(src[1]),RGB16_TO_B_8(src[1]),dst[2])\

#include "../csp_packed_packed.h"

/* bgr_16_to_yuy2_c */

#define FUNC_NAME   bgr_16_to_yuy2_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(BGR16_TO_R_8(src[0]),\
               BGR16_TO_G_8(src[0]),\
               BGR16_TO_B_8(src[0]),dst[0],dst[1],dst[3])\
    RGB_24_TO_Y_8(BGR16_TO_R_8(src[1]),BGR16_TO_G_8(src[1]),BGR16_TO_B_8(src[1]),dst[2])\

#include "../csp_packed_packed.h"

/* rgb_24_to_yuy2_c */

#define FUNC_NAME   rgb_24_to_yuy2_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  6
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(src[0],src[1],src[2],dst[0],dst[1],dst[3]) \
    RGB_24_TO_Y_8(src[3],src[4],src[5],dst[2])

#include "../csp_packed_packed.h"

#endif // !HQ

/* rgb_48_to_yuy2_c */

#define FUNC_NAME   rgb_48_to_yuy2_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  6
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_48_TO_YUV_8(src[0],src[1],src[2],dst[0],dst[1],dst[3]) \
    RGB_48_TO_Y_8(src[3],src[4],src[5],dst[2])

#include "../csp_packed_packed.h"


/* rgb_float_to_yuy2_c */

#define FUNC_NAME   rgb_float_to_yuy2_c
#define IN_TYPE     float
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  6
#define OUT_ADVANCE 4
#define NUM_PIXELS  2

#define INIT INIT_RGB_FLOAT_TO_YUV

#define CONVERT                                                   \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2],dst[0],dst[1],dst[3]) \
    RGB_FLOAT_TO_Y_8(src[3],src[4],src[5],dst[2])

#include "../csp_packed_packed.h"

#ifndef HQ

/* bgr_24_to_yuy2_c */

#define FUNC_NAME   bgr_24_to_yuy2_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  6
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(src[2],src[1],src[0],dst[0],dst[1],dst[3]) \
    RGB_24_TO_Y_8(src[5],src[4],src[3],dst[2])

#include "../csp_packed_packed.h"

/* rgb_32_to_yuy2_c */

#define FUNC_NAME   rgb_32_to_yuy2_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(src[0],src[1],src[2],dst[0],dst[1],dst[3]) \
    RGB_24_TO_Y_8(src[4],src[5],src[6],dst[2])

#include "../csp_packed_packed.h"

/* bgr_32_to_yuy2_c */

#define FUNC_NAME   bgr_32_to_yuy2_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(src[2],src[1],src[0],dst[0],dst[1],dst[3]) \
    RGB_24_TO_Y_8(src[6],src[5],src[4],dst[2])

#include "../csp_packed_packed.h"

/****************************************************
 * Conversions to YUVA 32
 ****************************************************/

/* rgb_15_to_yuva_32_c */

#define FUNC_NAME   rgb_15_to_yuva_32_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_8(RGB15_TO_R_8(src[0]),\
                    RGB15_TO_G_8(src[0]),                       \
                    RGB15_TO_B_8(src[0]),dst[0],dst[1],dst[2])  \
    dst[3] = 0xff;

#include "../csp_packed_packed.h"

/* rgb_15_to_yuva_64_c */

#define FUNC_NAME   rgb_15_to_yuva_64_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_16(RGB15_TO_R_8(src[0]),\
                    RGB15_TO_G_8(src[0]),                       \
                    RGB15_TO_B_8(src[0]),dst[0],dst[1],dst[2])  \
    dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* rgb_15_to_yuva_float_c */

#define FUNC_NAME   rgb_15_to_yuva_float_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(RGB15_TO_R_8(src[0]),\
                    RGB15_TO_G_8(src[0]),                       \
                    RGB15_TO_B_8(src[0]),dst[0],dst[1],dst[2])  \
    dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* rgb_15_to_yuv_float_c */

#define FUNC_NAME   rgb_15_to_yuv_float_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(RGB15_TO_R_8(src[0]),\
                    RGB15_TO_G_8(src[0]),                       \
                    RGB15_TO_B_8(src[0]),dst[0],dst[1],dst[2])

#include "../csp_packed_packed.h"


/* bgr_15_to_yuva_32_c */

#define FUNC_NAME   bgr_15_to_yuva_32_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_8(BGR15_TO_R_8(src[0]),\
               BGR15_TO_G_8(src[0]),\
               BGR15_TO_B_8(src[0]),dst[0],dst[1],dst[2])  \
    dst[3] = 0xff;


#include "../csp_packed_packed.h"

/* bgr_15_to_yuva_64_c */

#define FUNC_NAME   bgr_15_to_yuva_64_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_16(BGR15_TO_R_8(src[0]),\
                    BGR15_TO_G_8(src[0]),                       \
                    BGR15_TO_B_8(src[0]),dst[0],dst[1],dst[2])  \
    dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* bgr_15_to_yuva_float_c */

#define FUNC_NAME   bgr_15_to_yuva_float_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(BGR15_TO_R_8(src[0]),\
                    BGR15_TO_G_8(src[0]),                       \
                    BGR15_TO_B_8(src[0]),dst[0],dst[1],dst[2])  \
    dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* bgr_15_to_yuv_float_c */

#define FUNC_NAME   bgr_15_to_yuv_float_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(BGR15_TO_R_8(src[0]),\
                    BGR15_TO_G_8(src[0]),                       \
                    BGR15_TO_B_8(src[0]),dst[0],dst[1],dst[2])

#include "../csp_packed_packed.h"

/* rgb_16_to_yuva_32_c */

#define FUNC_NAME   rgb_16_to_yuva_32_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_8(RGB16_TO_R_8(src[0]),\
               RGB16_TO_G_8(src[0]),\
               RGB16_TO_B_8(src[0]),dst[0],dst[1],dst[2])  \
    dst[3] = 0xff;


#include "../csp_packed_packed.h"

/* rgb_16_to_yuva_64_c */

#define FUNC_NAME   rgb_16_to_yuva_64_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_16(RGB16_TO_R_8(src[0]),\
                    RGB16_TO_G_8(src[0]),                       \
                    RGB16_TO_B_8(src[0]),dst[0],dst[1],dst[2])  \
    dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* rgb_16_to_yuva_float_c */

#define FUNC_NAME   rgb_16_to_yuva_float_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(RGB16_TO_R_8(src[0]),\
                    RGB16_TO_G_8(src[0]),                       \
                    RGB16_TO_B_8(src[0]),dst[0],dst[1],dst[2])  \
    dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* rgb_16_to_yuv_float_c */

#define FUNC_NAME   rgb_16_to_yuv_float_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(RGB16_TO_R_8(src[0]),\
                    RGB16_TO_G_8(src[0]),                       \
                    RGB16_TO_B_8(src[0]),dst[0],dst[1],dst[2])

#include "../csp_packed_packed.h"

/* bgr_16_to_yuva_32_c */

#define FUNC_NAME   bgr_16_to_yuva_32_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_8(BGR16_TO_R_8(src[0]),\
               BGR16_TO_G_8(src[0]),\
               BGR16_TO_B_8(src[0]),dst[0],dst[1],dst[2])  \
    dst[3] = 0xff;


#include "../csp_packed_packed.h"


/* bgr_16_to_yuva_64_c */

#define FUNC_NAME   bgr_16_to_yuva_64_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_16(BGR16_TO_R_8(src[0]),\
                    BGR16_TO_G_8(src[0]),                       \
                    BGR16_TO_B_8(src[0]),dst[0],dst[1],dst[2])  \
    dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* bgr_16_to_yuva_float_c */

#define FUNC_NAME   bgr_16_to_yuva_float_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(BGR16_TO_R_8(src[0]),\
                    BGR16_TO_G_8(src[0]),                       \
                    BGR16_TO_B_8(src[0]),dst[0],dst[1],dst[2])  \
    dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* bgr_16_to_yuv_float_c */

#define FUNC_NAME   bgr_16_to_yuv_float_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(BGR16_TO_R_8(src[0]),\
                    BGR16_TO_G_8(src[0]),                       \
                    BGR16_TO_B_8(src[0]),dst[0],dst[1],dst[2])

#include "../csp_packed_packed.h"

/* rgb_24_to_yuva_32_c */

#define FUNC_NAME   rgb_24_to_yuva_32_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_8(src[0],src[1],src[2],dst[0],dst[1],dst[2])  \
    dst[3] = 0xff;


#include "../csp_packed_packed.h"

/* rgb_24_to_yuva_64_c */

#define FUNC_NAME   rgb_24_to_yuva_64_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_16(src[0],src[1],src[2],dst[0],dst[1],dst[2])  \
    dst[3] = 0xffff;


#include "../csp_packed_packed.h"

/* rgb_24_to_yuva_float_c */

#define FUNC_NAME   rgb_24_to_yuva_float_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    float
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(src[0],src[1],src[2],dst[0],dst[1],dst[2])  \
    dst[3] = 1.0;


#include "../csp_packed_packed.h"

/* rgb_24_to_yuv_float_c */

#define FUNC_NAME   rgb_24_to_yuv_float_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    float
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(src[0],src[1],src[2],dst[0],dst[1],dst[2])

#include "../csp_packed_packed.h"


#endif // !HQ

/* rgb_48_to_yuva_32_c */

#define FUNC_NAME   rgb_48_to_yuva_32_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_48_TO_YUV_8(src[0],src[1],src[2],dst[0],dst[1],dst[2])  \
    dst[3] = 0xff;


#include "../csp_packed_packed.h"

#ifndef HQ

/* rgb_48_to_yuva_64_c */

#define FUNC_NAME   rgb_48_to_yuva_64_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_48_TO_YUV_16(src[0],src[1],src[2],dst[0],dst[1],dst[2])  \
    dst[3] = 0xffff;


#include "../csp_packed_packed.h"

/* rgb_48_to_yuva_float_c */

#define FUNC_NAME   rgb_48_to_yuva_float_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    float
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_48_TO_YUV_FLOAT(src[0],src[1],src[2],dst[0],dst[1],dst[2])  \
    dst[3] = 1.0;


#include "../csp_packed_packed.h"

/* rgb_48_to_yuv_float_c */

#define FUNC_NAME   rgb_48_to_yuv_float_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    float
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_48_TO_YUV_FLOAT(src[0],src[1],src[2],dst[0],dst[1],dst[2])

#include "../csp_packed_packed.h"

#endif // !HQ

/* rgb_float_to_yuva_32_c */

#define FUNC_NAME   rgb_float_to_yuva_32_c
#define IN_TYPE     float
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1

#define INIT INIT_RGB_FLOAT_TO_YUV

#define CONVERT                                                   \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2],dst[0],dst[1],dst[2])  \
    dst[3] = 0xff;

#include "../csp_packed_packed.h"


/* rgb_float_to_yuva_64_c */

#define FUNC_NAME   rgb_float_to_yuva_64_c
#define IN_TYPE     float
#define OUT_TYPE    uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define INIT INIT_RGB_FLOAT_TO_YUV
#define CONVERT     \
    RGB_FLOAT_TO_YUV_16(src[0],src[1],src[2],dst[0],dst[1],dst[2])  \
    dst[3] = 0xffff;

#include "../csp_packed_packed.h"

#ifndef HQ

/* rgb_float_to_yuva_float_c */

#define FUNC_NAME   rgb_float_to_yuva_float_c
#define IN_TYPE     float
#define OUT_TYPE    float
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_FLOAT_TO_YUV_FLOAT(src[0],src[1],src[2],dst[0],dst[1],dst[2])  \
    dst[3] = 1.0;


#include "../csp_packed_packed.h"

/* rgb_float_to_yuv_float_c */

#define FUNC_NAME   rgb_float_to_yuv_float_c
#define IN_TYPE     float
#define OUT_TYPE    float
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_FLOAT_TO_YUV_FLOAT(src[0],src[1],src[2],dst[0],dst[1],dst[2])

#include "../csp_packed_packed.h"




/* bgr_24_to_yuva_32_c */

#define FUNC_NAME   bgr_24_to_yuva_32_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_8(src[2],src[1],src[0],dst[0],dst[1],dst[2])  \
    dst[3] = 0xff;

#include "../csp_packed_packed.h"


/* bgr_24_to_yuva_64_c */

#define FUNC_NAME   bgr_24_to_yuva_64_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_16(src[2],src[1],src[0],dst[0],dst[1],dst[2])  \
    dst[3] = 0xffff;


#include "../csp_packed_packed.h"

/* bgr_24_to_yuva_float_c */

#define FUNC_NAME   bgr_24_to_yuva_float_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    float
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(src[2],src[1],src[0],dst[0],dst[1],dst[2])  \
    dst[3] = 1.0;


#include "../csp_packed_packed.h"

/* bgr_24_to_yuv_float_c */

#define FUNC_NAME   bgr_24_to_yuv_float_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    float
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(src[2],src[1],src[0],dst[0],dst[1],dst[2])

#include "../csp_packed_packed.h"

/* rgb_32_to_yuva_32_c */

#define FUNC_NAME   rgb_32_to_yuva_32_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_8(src[0],src[1],src[2],dst[0],dst[1],dst[2])  \
    dst[3] = 0xff;

#include "../csp_packed_packed.h"

/* rgb_32_to_yuva_64_c */

#define FUNC_NAME   rgb_32_to_yuva_64_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_16(src[0],src[1],src[2],dst[0],dst[1],dst[2])  \
    dst[3] = 0xffff;


#include "../csp_packed_packed.h"

/* rgb_32_to_yuva_float_c */

#define FUNC_NAME   rgb_32_to_yuva_float_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    float
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(src[0],src[1],src[2],dst[0],dst[1],dst[2])  \
    dst[3] = 1.0;


#include "../csp_packed_packed.h"

/* rgb_32_to_yuv_float_c */

#define FUNC_NAME   rgb_32_to_yuv_float_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(src[0],src[1],src[2],dst[0],dst[1],dst[2])

#include "../csp_packed_packed.h"

/* bgr_32_to_yuva_32_c */

#define FUNC_NAME   bgr_32_to_yuva_32_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_8(src[2],src[1],src[0],dst[0],dst[1],dst[2])  \
    dst[3] = 0xff;

#include "../csp_packed_packed.h"


/* bgr_32_to_yuva_64_c */

#define FUNC_NAME   bgr_32_to_yuva_64_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_16(src[2],src[1],src[0],dst[0],dst[1],dst[2])  \
    dst[3] = 0xffff;


#include "../csp_packed_packed.h"

/* bgr_32_to_yuva_float_c */

#define FUNC_NAME   bgr_32_to_yuva_float_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    float
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(src[2],src[1],src[0],dst[0],dst[1],dst[2])  \
    dst[3] = 1.0;


#include "../csp_packed_packed.h"

/* bgr_32_to_yuv_float_c */

#define FUNC_NAME   bgr_32_to_yuv_float_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(src[2],src[1],src[0],dst[0],dst[1],dst[2])

#include "../csp_packed_packed.h"

/****************************************************
 * Conversions to UYVY
 ****************************************************/

/* rgb_15_to_uyvy_c */

#define FUNC_NAME   rgb_15_to_uyvy_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(RGB15_TO_R_8(src[0]),\
               RGB15_TO_G_8(src[0]),\
               RGB15_TO_B_8(src[0]),dst[1],dst[0],dst[2])\
    RGB_24_TO_Y_8(RGB15_TO_R_8(src[1]),RGB15_TO_G_8(src[1]),RGB15_TO_B_8(src[1]),dst[3])\

#include "../csp_packed_packed.h"

/* bgr_15_to_uyvy_c */

#define FUNC_NAME   bgr_15_to_uyvy_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(BGR15_TO_R_8(src[0]),\
               BGR15_TO_G_8(src[0]),\
               BGR15_TO_B_8(src[0]),dst[1],dst[0],dst[2])\
    RGB_24_TO_Y_8(BGR15_TO_R_8(src[1]),BGR15_TO_G_8(src[1]),BGR15_TO_B_8(src[1]),dst[3])\

#include "../csp_packed_packed.h"

/* rgb_16_to_uyvy_c */

#define FUNC_NAME   rgb_16_to_uyvy_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(RGB16_TO_R_8(src[0]),\
               RGB16_TO_G_8(src[0]),\
               RGB16_TO_B_8(src[0]),dst[1],dst[0],dst[2])\
    RGB_24_TO_Y_8(RGB16_TO_R_8(src[1]),RGB16_TO_G_8(src[1]),RGB16_TO_B_8(src[1]),dst[3])\

#include "../csp_packed_packed.h"

/* bgr_16_to_uyvy_c */

#define FUNC_NAME   bgr_16_to_uyvy_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(BGR16_TO_R_8(src[0]),\
               BGR16_TO_G_8(src[0]),\
               BGR16_TO_B_8(src[0]),dst[1],dst[0],dst[2])\
    RGB_24_TO_Y_8(BGR16_TO_R_8(src[1]),BGR16_TO_G_8(src[1]),BGR16_TO_B_8(src[1]),dst[3])\

#include "../csp_packed_packed.h"

/* rgb_24_to_uyvy_c */

#define FUNC_NAME   rgb_24_to_uyvy_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  6
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(src[0],src[1],src[2],dst[1],dst[0],dst[2]) \
    RGB_24_TO_Y_8(src[3],src[4],src[5],dst[3])

#include "../csp_packed_packed.h"

#endif // !HQ

/* rgb_48_to_uyvy_c */

#define FUNC_NAME   rgb_48_to_uyvy_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  6
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_48_TO_YUV_8(src[0],src[1],src[2],dst[1],dst[0],dst[2]) \
    RGB_48_TO_Y_8(src[3],src[4],src[5],dst[3])

#include "../csp_packed_packed.h"

/* rgb_float_to_uyvy_c */

#define FUNC_NAME   rgb_float_to_uyvy_c
#define IN_TYPE     float
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  6
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define INIT INIT_RGB_FLOAT_TO_YUV
#define CONVERT     \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2],dst[1],dst[0],dst[2]) \
    RGB_FLOAT_TO_Y_8(src[3],src[4],src[5],dst[3])

#include "../csp_packed_packed.h"

#ifndef HQ

/* bgr_24_to_uyvy_c */

#define FUNC_NAME   bgr_24_to_uyvy_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  6
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(src[2],src[1],src[0],dst[1],dst[0],dst[2]) \
    RGB_24_TO_Y_8(src[5],src[4],src[3],dst[3])

#include "../csp_packed_packed.h"

/* rgb_32_to_uyvy_c */

#define FUNC_NAME   rgb_32_to_uyvy_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(src[0],src[1],src[2],dst[1],dst[0],dst[2]) \
    RGB_24_TO_Y_8(src[4],src[5],src[6],dst[3])

#include "../csp_packed_packed.h"

/* bgr_32_to_uyvy_c */

#define FUNC_NAME   bgr_32_to_uyvy_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_24_TO_YUV_8(src[2],src[1],src[0],dst[1],dst[0],dst[2]) \
    RGB_24_TO_Y_8(src[6],src[5],src[4],dst[3])

#include "../csp_packed_packed.h"

/****************************************************
 * Conversions to YUV 422 P
 ****************************************************/

/* rgb_15_to_yuv_422_p_c */

#define FUNC_NAME      rgb_15_to_yuv_422_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(RGB15_TO_R_8(src[0]), \
           RGB15_TO_G_8(src[0]), \
           RGB15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[1]), \
         RGB15_TO_G_8(src[1]), \
         RGB15_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_15_to_yuv_422_p_c */

#define FUNC_NAME      bgr_15_to_yuv_422_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(BGR15_TO_R_8(src[0]), \
           BGR15_TO_G_8(src[0]), \
           BGR15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[1]), \
         BGR15_TO_G_8(src[1]), \
         BGR15_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_16_to_yuv_422_p_c */

#define FUNC_NAME      rgb_16_to_yuv_422_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(RGB16_TO_R_8(src[0]), \
           RGB16_TO_G_8(src[0]), \
           RGB16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[1]), \
         RGB16_TO_G_8(src[1]), \
         RGB16_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_16_to_yuv_422_p_c */

#define FUNC_NAME      bgr_16_to_yuv_422_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(BGR16_TO_R_8(src[0]), \
           BGR16_TO_G_8(src[0]), \
           BGR16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[1]), \
         BGR16_TO_G_8(src[1]), \
         BGR16_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_24_to_yuv_422_p_c */

#define FUNC_NAME      rgb_24_to_yuv_422_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[3],src[4],src[5],dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#endif // !HQ

/* rgb_48_to_yuv_422_p_c */

#define FUNC_NAME      rgb_48_to_yuv_422_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_48_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_48_TO_Y_8(src[3],src[4],src[5],dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"


/* rgb_float_to_yuv_422_p_c */

#define FUNC_NAME      rgb_float_to_yuv_422_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2

#define INIT INIT_RGB_FLOAT_TO_YUV

#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_Y_8(src[3],src[4],src[5],dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#ifndef HQ

/* bgr_24_to_yuv_422_p_c */

#define FUNC_NAME      bgr_24_to_yuv_422_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[5],src[4],src[3],dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_32_to_yuv_422_p_c */

#define FUNC_NAME      rgb_32_to_yuv_422_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[4],src[5],src[6],dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_32_to_yuv_422_p_c */

#define FUNC_NAME      bgr_32_to_yuv_422_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[6],src[5],src[4],dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/****************************************************
 * Conversions to YUV 422 P 16 bit
 ****************************************************/

/* rgb_15_to_yuv_422_p_16_c */

#define FUNC_NAME      rgb_15_to_yuv_422_p_16_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUV_16(RGB15_TO_R_8(src[0]), \
           RGB15_TO_G_8(src[0]), \
           RGB15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_16(RGB15_TO_R_8(src[1]), \
         RGB15_TO_G_8(src[1]), \
         RGB15_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_15_to_yuv_422_p_16_c */

#define FUNC_NAME      bgr_15_to_yuv_422_p_16_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUV_16(BGR15_TO_R_8(src[0]), \
           BGR15_TO_G_8(src[0]), \
           BGR15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_16(BGR15_TO_R_8(src[1]), \
         BGR15_TO_G_8(src[1]), \
         BGR15_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_16_to_yuv_422_p_16_c */

#define FUNC_NAME      rgb_16_to_yuv_422_p_16_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUV_16(RGB16_TO_R_8(src[0]), \
           RGB16_TO_G_8(src[0]), \
           RGB16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_16(RGB16_TO_R_8(src[1]), \
         RGB16_TO_G_8(src[1]), \
         RGB16_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_16_to_yuv_422_p_16_c */

#define FUNC_NAME      bgr_16_to_yuv_422_p_16_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUV_16(BGR16_TO_R_8(src[0]), \
           BGR16_TO_G_8(src[0]), \
           BGR16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_16(BGR16_TO_R_8(src[1]), \
         BGR16_TO_G_8(src[1]), \
         BGR16_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_24_to_yuv_422_p_16_c */

#define FUNC_NAME      rgb_24_to_yuv_422_p_16_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUV_16(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_16(src[3],src[4],src[5],dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_48_to_yuv_422_p_16_c */

#define FUNC_NAME      rgb_48_to_yuv_422_p_16_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_48_TO_YUV_16(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_48_TO_Y_16(src[3],src[4],src[5],dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#endif // !HQ

/* rgb_float_to_yuv_422_p_16_c */

#define FUNC_NAME      rgb_float_to_yuv_422_p_16_c
#define IN_TYPE        float
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2

#define INIT INIT_RGB_FLOAT_TO_YUV

#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_16(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_Y_16(src[3],src[4],src[5],dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#ifndef HQ

/* bgr_24_to_yuv_422_p_16_c */

#define FUNC_NAME      bgr_24_to_yuv_422_p_16_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUV_16(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_16(src[5],src[4],src[3],dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_32_to_yuv_422_p_16_c */

#define FUNC_NAME      rgb_32_to_yuv_422_p_16_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUV_16(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_16(src[4],src[5],src[6],dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_32_to_yuv_422_p_16_c */

#define FUNC_NAME      bgr_32_to_yuv_422_p_16_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUV_16(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_16(src[6],src[5],src[4],dst_y[1])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"


/****************************************************
 * Conversions to YUV 411 P
 ****************************************************/

/* rgb_15_to_yuv_411_p_c */

#define FUNC_NAME      rgb_15_to_yuv_411_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(RGB15_TO_R_8(src[0]), \
           RGB15_TO_G_8(src[0]), \
           RGB15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[1]), \
         RGB15_TO_G_8(src[1]), \
         RGB15_TO_B_8(src[1]), \
         dst_y[1]) \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[2]), \
         RGB15_TO_G_8(src[2]), \
         RGB15_TO_B_8(src[2]), \
         dst_y[2]) \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[3]), \
         RGB15_TO_G_8(src[3]), \
         RGB15_TO_B_8(src[3]), \
         dst_y[3])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_15_to_yuv_411_p_c */

#define FUNC_NAME      bgr_15_to_yuv_411_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(BGR15_TO_R_8(src[0]), \
           BGR15_TO_G_8(src[0]), \
           BGR15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[1]), \
         BGR15_TO_G_8(src[1]), \
         BGR15_TO_B_8(src[1]), \
         dst_y[1]) \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[2]), \
         BGR15_TO_G_8(src[2]), \
         BGR15_TO_B_8(src[2]), \
         dst_y[2]) \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[3]), \
         BGR15_TO_G_8(src[3]), \
         BGR15_TO_B_8(src[3]), \
         dst_y[3])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_16_to_yuv_411_p_c */

#define FUNC_NAME      rgb_16_to_yuv_411_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(RGB16_TO_R_8(src[0]), \
           RGB16_TO_G_8(src[0]), \
           RGB16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[1]), \
         RGB16_TO_G_8(src[1]), \
         RGB16_TO_B_8(src[1]), \
         dst_y[1]) \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[2]), \
         RGB16_TO_G_8(src[2]), \
         RGB16_TO_B_8(src[2]), \
         dst_y[2]) \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[3]), \
         RGB16_TO_G_8(src[3]), \
         RGB16_TO_B_8(src[3]), \
         dst_y[3])


#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_16_to_yuv_411_p_c */

#define FUNC_NAME      bgr_16_to_yuv_411_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(BGR16_TO_R_8(src[0]), \
           BGR16_TO_G_8(src[0]), \
           BGR16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[1]), \
         BGR16_TO_G_8(src[1]), \
         BGR16_TO_B_8(src[1]), \
         dst_y[1]) \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[2]), \
         BGR16_TO_G_8(src[2]), \
         BGR16_TO_B_8(src[2]), \
         dst_y[2]) \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[3]), \
         BGR16_TO_G_8(src[3]), \
         BGR16_TO_B_8(src[3]), \
         dst_y[3]) \

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_24_to_yuv_411_p_c */

#define FUNC_NAME      rgb_24_to_yuv_411_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     12
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[3],src[4],src[5],dst_y[1]) \
    RGB_24_TO_Y_8(src[6],src[7],src[8],dst_y[2]) \
    RGB_24_TO_Y_8(src[9],src[10],src[11],dst_y[3])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#endif // !HQ

/* rgb_48_to_yuv_411_p_c */

#define FUNC_NAME      rgb_48_to_yuv_411_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     12
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_48_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_48_TO_Y_8(src[3],src[4],src[5],dst_y[1]) \
    RGB_48_TO_Y_8(src[6],src[7],src[8],dst_y[2]) \
    RGB_48_TO_Y_8(src[9],src[10],src[11],dst_y[3])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_float_to_yuv_411_p_c */

#define FUNC_NAME      rgb_float_to_yuv_411_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     12
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4

#define INIT INIT_RGB_FLOAT_TO_YUV

#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_Y_8(src[3],src[4],src[5],dst_y[1]) \
    RGB_FLOAT_TO_Y_8(src[6],src[7],src[8],dst_y[2]) \
    RGB_FLOAT_TO_Y_8(src[9],src[10],src[11],dst_y[3])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#ifndef HQ

/* bgr_24_to_yuv_411_p_c */

#define FUNC_NAME      bgr_24_to_yuv_411_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     12
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[5],src[4],src[3],dst_y[1]) \
    RGB_24_TO_Y_8(src[8],src[7],src[6],dst_y[2]) \
    RGB_24_TO_Y_8(src[11],src[10],src[9],dst_y[3])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_32_to_yuv_411_p_c */

#define FUNC_NAME      rgb_32_to_yuv_411_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[4],src[5],src[6],dst_y[1]) \
    RGB_24_TO_Y_8(src[8],src[9],src[10],dst_y[2]) \
    RGB_24_TO_Y_8(src[12],src[13],src[14],dst_y[3])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_32_to_yuv_411_p_c */

#define FUNC_NAME      bgr_32_to_yuv_411_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[6],src[5],src[4],dst_y[1]) \
    RGB_24_TO_Y_8(src[10],src[9],src[8],dst_y[2]) \
    RGB_24_TO_Y_8(src[14],src[13],src[12],dst_y[3])

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/****************************************************
 * Conversions to YUV 444 P
 ****************************************************/

/* rgb_15_to_yuv_444_p_c */

#define FUNC_NAME      rgb_15_to_yuv_444_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     1
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(RGB15_TO_R_8(src[0]), \
           RGB15_TO_G_8(src[0]), \
           RGB15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_15_to_yuv_444_p_c */

#define FUNC_NAME      bgr_15_to_yuv_444_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     1
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(BGR15_TO_R_8(src[0]), \
           BGR15_TO_G_8(src[0]), \
           BGR15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v)
      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_16_to_yuv_444_p_c */

#define FUNC_NAME      rgb_16_to_yuv_444_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     1
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(RGB16_TO_R_8(src[0]), \
           RGB16_TO_G_8(src[0]), \
           RGB16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_16_to_yuv_444_p_c */

#define FUNC_NAME      bgr_16_to_yuv_444_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     1
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(BGR16_TO_R_8(src[0]), \
           BGR16_TO_G_8(src[0]), \
           BGR16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_24_to_yuv_444_p_c */

#define FUNC_NAME      rgb_24_to_yuv_444_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     3
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#endif // !HQ

/* rgb_48_to_yuv_444_p_c */

#define FUNC_NAME      rgb_48_to_yuv_444_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     3
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_48_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_float_to_yuv_444_p_c */

#define FUNC_NAME      rgb_float_to_yuv_444_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     3
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1

#define INIT INIT_RGB_FLOAT_TO_YUV

#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2], \
                       dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#ifndef HQ

/* bgr_24_to_yuv_444_p_c */

#define FUNC_NAME      bgr_24_to_yuv_444_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     3
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_32_to_yuv_444_p_c */

#define FUNC_NAME      rgb_32_to_yuv_444_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_32_to_yuv_444_p_c */

#define FUNC_NAME      bgr_32_to_yuv_444_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/****************************************************
 * Conversions to YUV 444 P 16 bit
 ****************************************************/

/* rgb_15_to_yuv_444_p_16_c */

#define FUNC_NAME      rgb_15_to_yuv_444_p_16_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     1
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
RGB_24_TO_YUV_16(RGB15_TO_R_8(src[0]), \
           RGB15_TO_G_8(src[0]), \
           RGB15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_15_to_yuv_444_p_16_c */

#define FUNC_NAME      bgr_15_to_yuv_444_p_16_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     1
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
RGB_24_TO_YUV_16(BGR15_TO_R_8(src[0]), \
           BGR15_TO_G_8(src[0]), \
           BGR15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v)
      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_16_to_yuv_444_p_16_c */

#define FUNC_NAME      rgb_16_to_yuv_444_p_16_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     1
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
RGB_24_TO_YUV_16(RGB16_TO_R_8(src[0]), \
           RGB16_TO_G_8(src[0]), \
           RGB16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_16_to_yuv_444_p_16_c */

#define FUNC_NAME      bgr_16_to_yuv_444_p_16_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     1
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
RGB_24_TO_YUV_16(BGR16_TO_R_8(src[0]), \
           BGR16_TO_G_8(src[0]), \
           BGR16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_24_to_yuv_444_p_16_c */

#define FUNC_NAME      rgb_24_to_yuv_444_p_16_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     3
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_24_TO_YUV_16(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_48_to_yuv_444_p_16_c */

#define FUNC_NAME      rgb_48_to_yuv_444_p_16_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     3
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_48_TO_YUV_16(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#endif // !HQ

/* rgb_float_to_yuv_444_p_16_c */

#define FUNC_NAME      rgb_float_to_yuv_444_p_16_c
#define IN_TYPE        float
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     3
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1

#define INIT INIT_RGB_FLOAT_TO_YUV

#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_16(src[0],src[1],src[2], \
                       dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#ifndef HQ

/* bgr_24_to_yuv_444_p_16_c */

#define FUNC_NAME      bgr_24_to_yuv_444_p_16_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     3
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_24_TO_YUV_16(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_32_to_yuv_444_p_16_c */

#define FUNC_NAME      rgb_32_to_yuv_444_p_16_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_24_TO_YUV_16(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_32_to_yuv_444_p_16_c */

#define FUNC_NAME      bgr_32_to_yuv_444_p_16_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_24_TO_YUV_16(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"


/****************************************************
 * Conversions to YUV 420 P
 ****************************************************/

/* rgb_15_to_yuv_420_p_c */

#define FUNC_NAME      rgb_15_to_yuv_420_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(RGB15_TO_R_8(src[0]), \
           RGB15_TO_G_8(src[0]), \
           RGB15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[1]), \
         RGB15_TO_G_8(src[1]), \
         RGB15_TO_B_8(src[1]), \
         dst_y[1])

#define CONVERT_Y \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[0]), \
         RGB15_TO_G_8(src[0]), \
         RGB15_TO_B_8(src[0]), \
         dst_y[0]) \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[1]), \
         RGB15_TO_G_8(src[1]), \
         RGB15_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* bgr_15_to_yuv_420_p_c */

#define FUNC_NAME      bgr_15_to_yuv_420_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(BGR15_TO_R_8(src[0]), \
           BGR15_TO_G_8(src[0]), \
           BGR15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[1]), \
         BGR15_TO_G_8(src[1]), \
         BGR15_TO_B_8(src[1]), \
         dst_y[1])

#define CONVERT_Y \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[0]), \
         BGR15_TO_G_8(src[0]), \
         BGR15_TO_B_8(src[0]), \
         dst_y[0]) \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[1]), \
         BGR15_TO_G_8(src[1]), \
         BGR15_TO_B_8(src[1]), \
         dst_y[1])
#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgb_16_to_yuv_420_p_c */

#define FUNC_NAME      rgb_16_to_yuv_420_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(RGB16_TO_R_8(src[0]), \
           RGB16_TO_G_8(src[0]), \
           RGB16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[1]), \
         RGB16_TO_G_8(src[1]), \
         RGB16_TO_B_8(src[1]), \
         dst_y[1])

#define CONVERT_Y    \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[0]), \
         RGB16_TO_G_8(src[0]), \
         RGB16_TO_B_8(src[0]), \
         dst_y[0]) \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[1]), \
         RGB16_TO_G_8(src[1]), \
         RGB16_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* bgr_16_to_yuv_420_p_c */

#define FUNC_NAME      bgr_16_to_yuv_420_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(BGR16_TO_R_8(src[0]), \
           BGR16_TO_G_8(src[0]), \
           BGR16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[1]), \
         BGR16_TO_G_8(src[1]), \
         BGR16_TO_B_8(src[1]), \
         dst_y[1])

#define CONVERT_Y \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[0]), \
         BGR16_TO_G_8(src[0]), \
         BGR16_TO_B_8(src[0]), \
         dst_y[0]) \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[1]), \
         BGR16_TO_G_8(src[1]), \
         BGR16_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgb_24_to_yuv_420_p_c */

#define FUNC_NAME      rgb_24_to_yuv_420_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[3],src[4],src[5],dst_y[1])

#define CONVERT_Y      \
    RGB_24_TO_Y_8(src[0],src[1],src[2],dst_y[0]) \
    RGB_24_TO_Y_8(src[3],src[4],src[5],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

#endif // !HQ

/* rgb_48_to_yuv_420_p_c */

#define FUNC_NAME      rgb_48_to_yuv_420_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_48_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_48_TO_Y_8(src[3],src[4],src[5],dst_y[1])

#define CONVERT_Y      \
    RGB_48_TO_Y_8(src[0],src[1],src[2],dst_y[0]) \
    RGB_48_TO_Y_8(src[3],src[4],src[5],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgb_float_to_yuv_420_p_c */

#define FUNC_NAME      rgb_float_to_yuv_420_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2

#define INIT INIT_RGB_FLOAT_TO_YUV

#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_Y_8(src[3],src[4],src[5],dst_y[1])

#define CONVERT_Y      \
    RGB_FLOAT_TO_Y_8(src[0],src[1],src[2],dst_y[0]) \
    RGB_FLOAT_TO_Y_8(src[3],src[4],src[5],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

#ifndef HQ

/* bgr_24_to_yuv_420_p_c */

#define FUNC_NAME      bgr_24_to_yuv_420_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[5],src[4],src[3],dst_y[1])

#define CONVERT_Y      \
    RGB_24_TO_Y_8(src[2],src[1],src[0],dst_y[0]) \
    RGB_24_TO_Y_8(src[5],src[4],src[3],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgb_32_to_yuv_420_p_c */

#define FUNC_NAME      rgb_32_to_yuv_420_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[4],src[5],src[6],dst_y[1])

#define CONVERT_Y      \
    RGB_24_TO_Y_8(src[0],src[1],src[2], \
               dst_y[0]) \
    RGB_24_TO_Y_8(src[4],src[5],src[6],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* bgr_32_to_yuv_420_p_c */

#define FUNC_NAME      bgr_32_to_yuv_420_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[6],src[5],src[4],dst_y[1])

#define CONVERT_Y      \
    RGB_24_TO_Y_8(src[2],src[1],src[0], \
             dst_y[0]) \
    RGB_24_TO_Y_8(src[6],src[5],src[4],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/****************************************************
 * Conversions to YUV 410 P
 ****************************************************/

/* rgb_15_to_yuv_410_p_c */

#define FUNC_NAME      rgb_15_to_yuv_410_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(RGB15_TO_R_8(src[0]), \
           RGB15_TO_G_8(src[0]), \
           RGB15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[1]), \
         RGB15_TO_G_8(src[1]), \
         RGB15_TO_B_8(src[1]), \
         dst_y[1]) \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[2]), \
         RGB15_TO_G_8(src[2]), \
         RGB15_TO_B_8(src[2]), \
         dst_y[2]) \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[3]), \
         RGB15_TO_G_8(src[3]), \
         RGB15_TO_B_8(src[3]), \
         dst_y[3])

#define CONVERT_Y \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[0]), \
         RGB15_TO_G_8(src[0]), \
         RGB15_TO_B_8(src[0]), \
         dst_y[0]) \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[1]), \
         RGB15_TO_G_8(src[1]), \
         RGB15_TO_B_8(src[1]), \
         dst_y[1]) \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[2]), \
         RGB15_TO_G_8(src[2]), \
         RGB15_TO_B_8(src[2]), \
         dst_y[2]) \
RGB_24_TO_Y_8(RGB15_TO_R_8(src[3]), \
         RGB15_TO_G_8(src[3]), \
         RGB15_TO_B_8(src[3]), \
         dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

/* bgr_15_to_yuv_410_p_c */

#define FUNC_NAME      bgr_15_to_yuv_410_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(BGR15_TO_R_8(src[0]), \
           BGR15_TO_G_8(src[0]), \
           BGR15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[1]), \
         BGR15_TO_G_8(src[1]), \
         BGR15_TO_B_8(src[1]), \
         dst_y[1]) \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[2]), \
         BGR15_TO_G_8(src[2]), \
         BGR15_TO_B_8(src[2]), \
         dst_y[2]) \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[3]), \
         BGR15_TO_G_8(src[3]), \
         BGR15_TO_B_8(src[3]), \
         dst_y[3])

#define CONVERT_Y \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[0]), \
         BGR15_TO_G_8(src[0]), \
         BGR15_TO_B_8(src[0]), \
         dst_y[0]) \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[1]), \
         BGR15_TO_G_8(src[1]), \
         BGR15_TO_B_8(src[1]), \
         dst_y[1]) \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[2]), \
         BGR15_TO_G_8(src[2]), \
         BGR15_TO_B_8(src[2]), \
         dst_y[2]) \
RGB_24_TO_Y_8(BGR15_TO_R_8(src[3]), \
         BGR15_TO_G_8(src[3]), \
         BGR15_TO_B_8(src[3]), \
         dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

/* rgb_16_to_yuv_410_p_c */

#define FUNC_NAME      rgb_16_to_yuv_410_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(RGB16_TO_R_8(src[0]), \
           RGB16_TO_G_8(src[0]), \
           RGB16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[1]), \
         RGB16_TO_G_8(src[1]), \
         RGB16_TO_B_8(src[1]), \
         dst_y[1]) \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[2]), \
         RGB16_TO_G_8(src[2]), \
         RGB16_TO_B_8(src[2]), \
         dst_y[2]) \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[3]), \
         RGB16_TO_G_8(src[3]), \
         RGB16_TO_B_8(src[3]), \
         dst_y[3])

#define CONVERT_Y    \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[0]), \
         RGB16_TO_G_8(src[0]), \
         RGB16_TO_B_8(src[0]), \
         dst_y[0]) \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[1]), \
         RGB16_TO_G_8(src[1]), \
         RGB16_TO_B_8(src[1]), \
         dst_y[1]) \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[2]), \
         RGB16_TO_G_8(src[2]), \
         RGB16_TO_B_8(src[2]), \
         dst_y[2]) \
RGB_24_TO_Y_8(RGB16_TO_R_8(src[3]), \
         RGB16_TO_G_8(src[3]), \
         RGB16_TO_B_8(src[3]), \
         dst_y[3])


#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

/* bgr_16_to_yuv_410_p_c */

#define FUNC_NAME      bgr_16_to_yuv_410_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
RGB_24_TO_YUV_8(BGR16_TO_R_8(src[0]), \
           BGR16_TO_G_8(src[0]), \
           BGR16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[1]), \
         BGR16_TO_G_8(src[1]), \
         BGR16_TO_B_8(src[1]), \
         dst_y[1]) \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[2]), \
         BGR16_TO_G_8(src[2]), \
         BGR16_TO_B_8(src[2]), \
         dst_y[2]) \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[3]), \
         BGR16_TO_G_8(src[3]), \
         BGR16_TO_B_8(src[3]), \
         dst_y[3])

#define CONVERT_Y \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[0]), \
         BGR16_TO_G_8(src[0]), \
         BGR16_TO_B_8(src[0]), \
         dst_y[0]) \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[1]), \
         BGR16_TO_G_8(src[1]), \
         BGR16_TO_B_8(src[1]), \
         dst_y[1]) \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[2]), \
         BGR16_TO_G_8(src[2]), \
         BGR16_TO_B_8(src[2]), \
         dst_y[2]) \
RGB_24_TO_Y_8(BGR16_TO_R_8(src[3]), \
         BGR16_TO_G_8(src[3]), \
         BGR16_TO_B_8(src[3]), \
         dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

/* rgb_24_to_yuv_410_p_c */

#define FUNC_NAME      rgb_24_to_yuv_410_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     12
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[3],src[4],src[5],dst_y[1]) \
    RGB_24_TO_Y_8(src[6],src[7],src[8],dst_y[2]) \
    RGB_24_TO_Y_8(src[9],src[10],src[11],dst_y[3])
         

#define CONVERT_Y      \
    RGB_24_TO_Y_8(src[0],src[1],src[2],dst_y[0]) \
    RGB_24_TO_Y_8(src[3],src[4],src[5],dst_y[1]) \
    RGB_24_TO_Y_8(src[6],src[7],src[8],dst_y[2]) \
    RGB_24_TO_Y_8(src[9],src[10],src[11],dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

#endif // !HQ

/* rgb_48_to_yuv_410_p_c */

#define FUNC_NAME      rgb_48_to_yuv_410_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     12
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_48_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_48_TO_Y_8(src[3],src[4],src[5],dst_y[1]) \
    RGB_48_TO_Y_8(src[6],src[7],src[8],dst_y[2]) \
    RGB_48_TO_Y_8(src[9],src[10],src[11],dst_y[3])
         

#define CONVERT_Y      \
    RGB_48_TO_Y_8(src[0],src[1],src[2],dst_y[0]) \
    RGB_48_TO_Y_8(src[3],src[4],src[5],dst_y[1]) \
    RGB_48_TO_Y_8(src[6],src[7],src[8],dst_y[2]) \
    RGB_48_TO_Y_8(src[9],src[10],src[11],dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

/* rgb_float_to_yuv_410_p_c */

#define FUNC_NAME      rgb_float_to_yuv_410_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     12
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4

#define INIT INIT_RGB_FLOAT_TO_YUV

#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_Y_8(src[3],src[4],src[5],dst_y[1]) \
    RGB_FLOAT_TO_Y_8(src[6],src[7],src[8],dst_y[2]) \
    RGB_FLOAT_TO_Y_8(src[9],src[10],src[11],dst_y[3])
         

#define CONVERT_Y      \
    RGB_FLOAT_TO_Y_8(src[0],src[1],src[2],dst_y[0]) \
    RGB_FLOAT_TO_Y_8(src[3],src[4],src[5],dst_y[1]) \
    RGB_FLOAT_TO_Y_8(src[6],src[7],src[8],dst_y[2]) \
    RGB_FLOAT_TO_Y_8(src[9],src[10],src[11],dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

#ifndef HQ

/* bgr_24_to_yuv_410_p_c */

#define FUNC_NAME      bgr_24_to_yuv_410_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     12
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[5],src[4],src[3],dst_y[1]) \
    RGB_24_TO_Y_8(src[8],src[7],src[6],dst_y[2]) \
    RGB_24_TO_Y_8(src[11],src[10],src[9],dst_y[3])

#define CONVERT_Y      \
    RGB_24_TO_Y_8(src[2],src[1],src[0],dst_y[0]) \
    RGB_24_TO_Y_8(src[5],src[4],src[3],dst_y[1]) \
    RGB_24_TO_Y_8(src[8],src[7],src[6],dst_y[2]) \
    RGB_24_TO_Y_8(src[11],src[10],src[9],dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

/* rgb_32_to_yuv_410_p_c */

#define FUNC_NAME      rgb_32_to_yuv_410_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[4],src[5],src[6],dst_y[1]) \
    RGB_24_TO_Y_8(src[8],src[9],src[10],dst_y[2]) \
    RGB_24_TO_Y_8(src[12],src[13],src[14],dst_y[3])

#define CONVERT_Y      \
    RGB_24_TO_Y_8(src[0],src[1],src[2],dst_y[0]) \
    RGB_24_TO_Y_8(src[4],src[5],src[6],dst_y[1]) \
    RGB_24_TO_Y_8(src[8],src[9],src[10],dst_y[2]) \
    RGB_24_TO_Y_8(src[12],src[13],src[14],dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

/* bgr_32_to_yuv_410_p_c */

#define FUNC_NAME      bgr_32_to_yuv_410_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_24_TO_YUV_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_Y_8(src[6],src[5],src[4],dst_y[1]) \
    RGB_24_TO_Y_8(src[10],src[9],src[8],dst_y[2]) \
    RGB_24_TO_Y_8(src[14],src[13],src[12],dst_y[3])

#define CONVERT_Y      \
    RGB_24_TO_Y_8(src[2],src[1],src[0], \
             dst_y[0]) \
    RGB_24_TO_Y_8(src[6],src[5],src[4],dst_y[1]) \
    RGB_24_TO_Y_8(src[10],src[9],src[8],dst_y[2]) \
    RGB_24_TO_Y_8(src[14],src[13],src[12],dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

/* RGBA -> ... */

/* rgba_32_to_yuva_32_c */

#define FUNC_NAME   rgba_32_to_yuva_32_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_8(src[0],src[1],src[2],dst[0],dst[1],dst[2]) \
    dst[3] = src[3];

#include "../csp_packed_packed.h"

/* rgba_32_to_yuva_64_c */

#define FUNC_NAME   rgba_32_to_yuva_64_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_16(src[0],src[1],src[2],dst[0],dst[1],dst[2]) \
    dst[3] = RGB_8_TO_16(src[3]);

#include "../csp_packed_packed.h"

/* rgba_32_to_yuva_float_c */

#define FUNC_NAME   rgba_32_to_yuva_float_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    float
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_24_TO_YUV_FLOAT(src[0],src[1],src[2],dst[0],dst[1],dst[2]) \
    dst[3] = RGB_8_TO_FLOAT(src[3]);

#include "../csp_packed_packed.h"

/* rgba_32_to_yuy2_c */

#define FUNC_NAME   rgba_32_to_yuy2_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGBA_32_TO_YUV_8(src[0],src[1],src[2],src[3],dst[0],dst[1],dst[3]) \
    RGBA_32_TO_Y_8(src[4],src[5],src[6],src[7],dst[2])
#define INIT   INIT_RGBA_32                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


#include "../csp_packed_packed.h"

/* rgba_32_to_uyvy_c */

#define FUNC_NAME   rgba_32_to_uyvy_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGBA_32_TO_YUV_8(src[0],src[1],src[2],src[3],dst[1],dst[0],dst[2]) \
    RGBA_32_TO_Y_8(src[4],src[5],src[6],src[7],dst[3])
#define INIT   INIT_RGBA_32                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


#include "../csp_packed_packed.h"

/* rgba_32_to_yuv_422_p_c */

#define FUNC_NAME      rgba_32_to_yuv_422_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_32_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_32_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1])
#define INIT   INIT_RGBA_32                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


      
#define CHROMA_SUB     1
#include "../csp_packed_planar.h"

/* rgba_32_to_yuv_422_p_16_c */

#define FUNC_NAME      rgba_32_to_yuv_422_p_16_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_32_TO_YUV_16(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_32_TO_Y_16(src[4],src[5],src[6],src[7],dst_y[1])
#define INIT   INIT_RGBA_32                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


      
#define CHROMA_SUB     1
#include "../csp_packed_planar.h"


/* rgba_32_to_yuv_411_p_c */

#define FUNC_NAME      rgba_32_to_yuv_411_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGBA_32_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_32_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1]) \
    RGBA_32_TO_Y_8(src[8],src[9],src[10],src[11],dst_y[2]) \
    RGBA_32_TO_Y_8(src[12],src[13],src[14],src[15],dst_y[3])

#define INIT   INIT_RGBA_32                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


      
#define CHROMA_SUB     1


#include "../csp_packed_planar.h"

/* rgba_32_to_yuv_420_p_c */

#define FUNC_NAME      rgba_32_to_yuv_420_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_32_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_32_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1])
#define INIT   INIT_RGBA_32                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


#define CONVERT_Y      \
    RGBA_32_TO_Y_8(src[0],src[1],src[2],src[3], \
               dst_y[0]) \
    RGBA_32_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgba_32_to_yuv_410_p_c */

#define FUNC_NAME      rgba_32_to_yuv_410_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGBA_32_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_32_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1]) \
    RGBA_32_TO_Y_8(src[8],src[9],src[10],src[11],dst_y[2]) \
    RGBA_32_TO_Y_8(src[12],src[13],src[14],src[15],dst_y[3])

#define INIT   INIT_RGBA_32                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


#define CONVERT_Y      \
    RGBA_32_TO_Y_8(src[0],src[1],src[2],src[3],dst_y[0]) \
    RGBA_32_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1]) \
    RGBA_32_TO_Y_8(src[8],src[9],src[10],src[11],dst_y[2]) \
    RGBA_32_TO_Y_8(src[12],src[13],src[14],src[15],dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

/* rgba_32_to_yuv_444_p_c */

#define FUNC_NAME      rgba_32_to_yuv_444_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGBA_32_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v)
#define INIT   INIT_RGBA_32                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgba_32_to_yuv_444_p_16_c */

#define FUNC_NAME      rgba_32_to_yuv_444_p_16_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGBA_32_TO_YUV_16(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v)
#define INIT   INIT_RGBA_32                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgba_32_to_yuv_float_c */

#define FUNC_NAME   rgba_32_to_yuv_float_c
#define IN_TYPE     uint8_t
#define OUT_TYPE    float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGBA_32_TO_YUV_FLOAT(src[0],src[1],src[2],src[3],dst[0],dst[1],dst[2])

#define INIT   INIT_RGBA_32                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;

#include "../csp_packed_packed.h"

#endif // !HQ

/* RGBA 64 -> */

/* rgba_64_to_yuva_32_c */

#define FUNC_NAME   rgba_64_to_yuva_32_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_48_TO_YUV_8(src[0],src[1],src[2],dst[0],dst[1],dst[2]) \
    RGB_16_TO_8(src[3], dst[3]);

#ifdef HQ
#define INIT uint32_t round_tmp;
#endif

#include "../csp_packed_packed.h"

#ifndef HQ

/* rgba_64_to_yuva_64_c */

#define FUNC_NAME   rgba_64_to_yuva_64_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_48_TO_YUV_16(src[0],src[1],src[2],dst[0],dst[1],dst[2]) \
    dst[3] = src[3];

#include "../csp_packed_packed.h"

/* rgba_64_to_yuva_float_c */

#define FUNC_NAME   rgba_64_to_yuva_float_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    float
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_48_TO_YUV_FLOAT(src[0],src[1],src[2],dst[0],dst[1],dst[2]) \
    dst[3] = RGB_16_TO_FLOAT(src[3]);

#include "../csp_packed_packed.h"

#endif // !HQ

/* rgba_64_to_yuy2_c */

#define FUNC_NAME   rgba_64_to_yuy2_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGBA_64_TO_YUV_8(src[0],src[1],src[2],src[3],dst[0],dst[1],dst[3]) \
    RGBA_64_TO_Y_8(src[4],src[5],src[6],src[7],dst[2])

#define INIT   INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;

#include "../csp_packed_packed.h"

/* rgba_64_to_uyvy_c */

#define FUNC_NAME   rgba_64_to_uyvy_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGBA_64_TO_YUV_8(src[0],src[1],src[2],src[3],dst[1],dst[0],dst[2]) \
    RGBA_64_TO_Y_8(src[4],src[5],src[6],src[7],dst[3])
#define INIT   INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


#include "../csp_packed_packed.h"

/* rgba_64_to_yuv_422_p_c */

#define FUNC_NAME      rgba_64_to_yuv_422_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_64_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_64_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1])
#define INIT   INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


      
#define CHROMA_SUB     1
#include "../csp_packed_planar.h"

#ifndef HQ

/* rgba_64_to_yuv_422_p_16_c */

#define FUNC_NAME      rgba_64_to_yuv_422_p_16_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_64_TO_YUV_16(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_64_TO_Y_16(src[4],src[5],src[6],src[7],dst_y[1])
#define INIT   INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


      
#define CHROMA_SUB     1
#include "../csp_packed_planar.h"

#endif // !HQ

/* rgba_64_to_yuv_411_p_c */

#define FUNC_NAME      rgba_64_to_yuv_411_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGBA_64_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_64_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1]) \
    RGBA_64_TO_Y_8(src[8],src[9],src[10],src[11],dst_y[2]) \
    RGBA_64_TO_Y_8(src[12],src[13],src[14],src[15],dst_y[3])

#define INIT   INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


      
#define CHROMA_SUB     1


#include "../csp_packed_planar.h"

/* rgba_64_to_yuv_420_p_c */

#define FUNC_NAME      rgba_64_to_yuv_420_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_64_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_64_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1])
#define INIT   INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


#define CONVERT_Y      \
    RGBA_64_TO_Y_8(src[0],src[1],src[2],src[3], \
               dst_y[0]) \
    RGBA_64_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgba_64_to_yuv_410_p_c */

#define FUNC_NAME      rgba_64_to_yuv_410_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGBA_64_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_64_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1]) \
    RGBA_64_TO_Y_8(src[8],src[9],src[10],src[11],dst_y[2]) \
    RGBA_64_TO_Y_8(src[12],src[13],src[14],src[15],dst_y[3])

#define INIT   INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


#define CONVERT_Y      \
    RGBA_64_TO_Y_8(src[0],src[1],src[2],src[3],dst_y[0]) \
    RGBA_64_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1]) \
    RGBA_64_TO_Y_8(src[8],src[9],src[10],src[11],dst_y[2]) \
    RGBA_64_TO_Y_8(src[12],src[13],src[14],src[15],dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

/* rgba_64_to_yuv_444_p_c */

#define FUNC_NAME      rgba_64_to_yuv_444_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGBA_64_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v)
#define INIT   INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#ifndef HQ

/* rgba_64_to_yuv_444_16_p_c */

#define FUNC_NAME      rgba_64_to_yuv_444_p_16_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGBA_64_TO_YUV_16(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v)
#define INIT   INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;
      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgba_64_to_yuv_float_c */

#define FUNC_NAME   rgba_64_to_yuv_float_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGBA_64_TO_YUV_FLOAT(src[0],src[1],src[2],src[3],dst[0],dst[1],dst[2])

#define INIT   INIT_RGBA_64                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;

#include "../csp_packed_packed.h"

/* rgba_64_to_yuv_float_ia_c */

#define FUNC_NAME   rgba_64_to_yuv_float_ia_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_48_TO_YUV_FLOAT(src[0],src[1],src[2],dst[0],dst[1],dst[3])


#include "../csp_packed_packed.h"


#endif // !HQ

/* RGBA 64 -> (No Alpha) */

/* rgba_64_to_yuy2_ia_c */

#define FUNC_NAME   rgba_64_to_yuy2_ia_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_48_TO_YUV_8(src[0],src[1],src[2],dst[0],dst[1],dst[3]) \
    RGB_48_TO_Y_8(src[4],src[5],src[6],dst[2])

#include "../csp_packed_packed.h"

/* rgba_64_to_uyvy_ia_c */

#define FUNC_NAME   rgba_64_to_uyvy_ia_c
#define IN_TYPE     uint16_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_48_TO_YUV_8(src[0],src[1],src[2],dst[1],dst[0],dst[2]) \
    RGB_48_TO_Y_8(src[4],src[5],src[6],dst[3])

#include "../csp_packed_packed.h"

/* rgba_64_to_yuv_422_p_ia_c */

#define FUNC_NAME      rgba_64_to_yuv_422_p_ia_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_48_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_48_TO_Y_8(src[4],src[5],src[6],dst_y[1])
      
#define CHROMA_SUB     1
#include "../csp_packed_planar.h"

#ifndef HQ

/* rgba_64_to_yuv_422_p_16_ia_c */

#define FUNC_NAME      rgba_64_to_yuv_422_p_16_ia_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_48_TO_YUV_16(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_48_TO_Y_16(src[4],src[5],src[6],dst_y[1])
     
#define CHROMA_SUB     1
#include "../csp_packed_planar.h"

#endif // !HQ

/* rgba_64_to_yuv_411_p_ia_c */

#define FUNC_NAME      rgba_64_to_yuv_411_p_ia_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_48_TO_YUV_8(src[0],src[1],src[2], \
                    dst_y[0],*dst_u,*dst_v)        \
      RGB_48_TO_Y_8(src[4],src[5],src[6],dst_y[1])      \
      RGB_48_TO_Y_8(src[8],src[9],src[10],dst_y[2])     \
      RGB_48_TO_Y_8(src[12],src[13],src[14],dst_y[3])

#define CHROMA_SUB     1


#include "../csp_packed_planar.h"

/* rgba_64_to_yuv_420_p_ia_c */

#define FUNC_NAME      rgba_64_to_yuv_420_p_ia_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_48_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_48_TO_Y_8(src[4],src[5],src[6],dst_y[1])

#define CONVERT_Y      \
    RGB_48_TO_Y_8(src[0],src[1],src[2], \
               dst_y[0]) \
    RGB_48_TO_Y_8(src[4],src[5],src[6],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgba_64_to_yuv_410_p_ia_c */

#define FUNC_NAME      rgba_64_to_yuv_410_p_ia_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_48_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_48_TO_Y_8(src[4],src[5],src[6],dst_y[1]) \
    RGB_48_TO_Y_8(src[8],src[9],src[10],dst_y[2]) \
    RGB_48_TO_Y_8(src[12],src[13],src[14],dst_y[3])

#define CONVERT_Y      \
    RGB_48_TO_Y_8(src[0],src[1],src[2],dst_y[0]) \
    RGB_48_TO_Y_8(src[4],src[5],src[6],dst_y[1]) \
    RGB_48_TO_Y_8(src[8],src[9],src[10],dst_y[2]) \
    RGB_48_TO_Y_8(src[12],src[13],src[14],dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

/* rgba_64_to_yuv_444_p_ia_c */

#define FUNC_NAME      rgba_64_to_yuv_444_p_ia_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_48_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#ifndef HQ

/* rgba_64_to_yuv_444_16_p_ia_c */

#define FUNC_NAME      rgba_64_to_yuv_444_p_16_ia_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_48_TO_YUV_16(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#endif // !HQ

/* RGB Float -> */

/* rgba_float_to_yuva_32_c */

#define FUNC_NAME   rgba_float_to_yuva_32_c
#define IN_TYPE     float
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define INIT   INIT_RGB_FLOAT_TO_YUV
#define CONVERT                                                    \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2],dst[0],dst[1],dst[2]) \
    RGB_FLOAT_TO_8(src[3], dst[3]);

#include "../csp_packed_packed.h"

/* rgba_float_to_yuva_64_c */

#define FUNC_NAME   rgba_float_to_yuva_64_c
#define IN_TYPE     float
#define OUT_TYPE    uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define INIT   INIT_RGB_FLOAT_TO_YUV
#define CONVERT                                                    \
    RGB_FLOAT_TO_YUV_16(src[0],src[1],src[2],dst[0],dst[1],dst[2]) \
    RGB_FLOAT_TO_16(src[3], dst[3]);

#include "../csp_packed_packed.h"

#ifndef HQ

/* rgba_float_to_yuva_float_c */

#define FUNC_NAME   rgba_float_to_yuva_float_c
#define IN_TYPE     float
#define OUT_TYPE    float
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
// #define INIT   INIT_RGB_FLOAT_TO_YUV
#define CONVERT                                                    \
    RGB_FLOAT_TO_YUV_FLOAT(src[0],src[1],src[2],dst[0],dst[1],dst[2]) \
    dst[3] = src[3];

#include "../csp_packed_packed.h"

#endif // !HQ

/* rgba_float_to_yuy2_c */

#define FUNC_NAME   rgba_float_to_yuy2_c
#define IN_TYPE     float
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGBA_FLOAT_TO_YUV_8(src[0],src[1],src[2],src[3],dst[0],dst[1],dst[3]) \
    RGBA_FLOAT_TO_Y_8(src[4],src[5],src[6],src[7],dst[2])
#define INIT   INIT_RGBA_FLOAT \
  float r_tmp;                                                          \
  float g_tmp;                                                          \
  float b_tmp;                                                          \
  INIT_RGB_FLOAT_TO_YUV

#include "../csp_packed_packed.h"

/* rgba_float_to_uyvy_c */

#define FUNC_NAME   rgba_float_to_uyvy_c
#define IN_TYPE     float
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGBA_FLOAT_TO_YUV_8(src[0],src[1],src[2],src[3],dst[1],dst[0],dst[2]) \
    RGBA_FLOAT_TO_Y_8(src[4],src[5],src[6],src[7],dst[3])
#define INIT   INIT_RGBA_FLOAT \
  float r_tmp;                                                          \
  float g_tmp;                                                          \
  float b_tmp;                                                          \
  INIT_RGB_FLOAT_TO_YUV

#include "../csp_packed_packed.h"

/* rgba_float_to_yuv_422_p_c */

#define FUNC_NAME      rgba_float_to_yuv_422_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_FLOAT_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_FLOAT_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1])
#define INIT   INIT_RGBA_FLOAT \
  float r_tmp;                                                          \
  float g_tmp;                                                          \
  float b_tmp;                                                          \
  INIT_RGB_FLOAT_TO_YUV

      
#define CHROMA_SUB     1
#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_422_p_16_c */

#define FUNC_NAME      rgba_float_to_yuv_422_p_16_c
#define IN_TYPE        float
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_FLOAT_TO_YUV_16(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_FLOAT_TO_Y_16(src[4],src[5],src[6],src[7],dst_y[1])

#define INIT   INIT_RGBA_FLOAT \
  float r_tmp;                                                          \
  float g_tmp;                                                          \
  float b_tmp;                                                          \
  INIT_RGB_FLOAT_TO_YUV
      
#define CHROMA_SUB     1
#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_411_p_c */

#define FUNC_NAME      rgba_float_to_yuv_411_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGBA_FLOAT_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_FLOAT_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1]) \
    RGBA_FLOAT_TO_Y_8(src[8],src[9],src[10],src[11],dst_y[2]) \
    RGBA_FLOAT_TO_Y_8(src[12],src[13],src[14],src[15],dst_y[3])

#define INIT   INIT_RGBA_FLOAT \
  float r_tmp;                                                          \
  float g_tmp;                                                          \
  float b_tmp;                                                          \
  INIT_RGB_FLOAT_TO_YUV


      
#define CHROMA_SUB     1


#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_420_p_c */

#define FUNC_NAME      rgba_float_to_yuv_420_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_FLOAT_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_FLOAT_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1])
#define INIT   INIT_RGBA_FLOAT \
  float r_tmp;                                                          \
  float g_tmp;                                                          \
  float b_tmp;                                                          \
               INIT_RGB_FLOAT_TO_YUV


#define CONVERT_Y      \
    RGBA_FLOAT_TO_Y_8(src[0],src[1],src[2],src[3], \
               dst_y[0]) \
    RGBA_FLOAT_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_410_p_c */

#define FUNC_NAME      rgba_float_to_yuv_410_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGBA_FLOAT_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_FLOAT_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1]) \
    RGBA_FLOAT_TO_Y_8(src[8],src[9],src[10],src[11],dst_y[2]) \
    RGBA_FLOAT_TO_Y_8(src[12],src[13],src[14],src[15],dst_y[3])

#define INIT   INIT_RGBA_FLOAT \
  float r_tmp;                                                          \
  float g_tmp;                                                          \
  float b_tmp;                                                          \
               INIT_RGB_FLOAT_TO_YUV


#define CONVERT_Y      \
    RGBA_FLOAT_TO_Y_8(src[0],src[1],src[2],src[3],dst_y[0]) \
    RGBA_FLOAT_TO_Y_8(src[4],src[5],src[6],src[7],dst_y[1]) \
    RGBA_FLOAT_TO_Y_8(src[8],src[9],src[10],src[11],dst_y[2]) \
    RGBA_FLOAT_TO_Y_8(src[12],src[13],src[14],src[15],dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_444_p_c */

#define FUNC_NAME      rgba_float_to_yuv_444_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGBA_FLOAT_TO_YUV_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v)
#define INIT   INIT_RGBA_FLOAT \
  float r_tmp;                                                          \
  float g_tmp;                                                          \
  float b_tmp;                                                          \
               INIT_RGB_FLOAT_TO_YUV


      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_444_p_16_c */

#define FUNC_NAME      rgba_float_to_yuv_444_p_16_c
#define IN_TYPE        float
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGBA_FLOAT_TO_YUV_16(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v)
#define INIT   INIT_RGBA_FLOAT \
  float r_tmp;                                                          \
  float g_tmp;                                                          \
  float b_tmp;                                                          \
               INIT_RGB_FLOAT_TO_YUV

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_float_c */

#define FUNC_NAME   rgba_float_to_yuv_float_c
#define IN_TYPE     float
#define OUT_TYPE    float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGBA_FLOAT_TO_YUV_FLOAT(src[0],src[1],src[2],src[3],dst[0],dst[1],dst[2])

#define INIT   INIT_RGBA_FLOAT                                             \
  float r_tmp;                                                       \
  float g_tmp;                                                       \
  float b_tmp;

#include "../csp_packed_packed.h"


/* rgba_float_to_yuv_float_ia_c */

#define FUNC_NAME   rgba_float_to_yuv_float_ia_c
#define IN_TYPE     float
#define OUT_TYPE    float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define CONVERT     \
    RGB_FLOAT_TO_YUV_FLOAT(src[0],src[1],src[2],dst[0],dst[1],dst[2])

#include "../csp_packed_packed.h"

/* RGB Float -> (No alpha) */

/* rgba_float_to_yuy2_ia_c */

#define FUNC_NAME   rgba_float_to_yuy2_ia_c
#define IN_TYPE     float
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2],dst[0],dst[1],dst[3]) \
    RGB_FLOAT_TO_Y_8(src[4],src[5],src[6],dst[2])
#define INIT \
  INIT_RGB_FLOAT_TO_YUV

#include "../csp_packed_packed.h"

/* rgba_float_to_uyvy_ia_c */

#define FUNC_NAME   rgba_float_to_uyvy_ia_c
#define IN_TYPE     float
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 4
#define NUM_PIXELS  2
#define CONVERT     \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2],dst[1],dst[0],dst[2]) \
    RGB_FLOAT_TO_Y_8(src[4],src[5],src[6],dst[3])
#define INIT  INIT_RGB_FLOAT_TO_YUV

#include "../csp_packed_packed.h"

/* rgba_float_to_yuv_422_p_ia_c */

#define FUNC_NAME      rgba_float_to_yuv_422_p_ia_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_Y_8(src[4],src[5],src[6],dst_y[1])
#define INIT   INIT_RGB_FLOAT_TO_YUV
      
#define CHROMA_SUB     1
#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_422_p_16_ia_c */

#define FUNC_NAME      rgba_float_to_yuv_422_p_16_ia_c
#define IN_TYPE        float
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_16(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_Y_16(src[4],src[5],src[6],dst_y[1])

#define INIT   INIT_RGB_FLOAT_TO_YUV
      
#define CHROMA_SUB     1
#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_411_p_ia_c */

#define FUNC_NAME      rgba_float_to_yuv_411_p_ia_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_Y_8(src[4],src[5],src[6],dst_y[1]) \
    RGB_FLOAT_TO_Y_8(src[8],src[9],src[10],dst_y[2]) \
    RGB_FLOAT_TO_Y_8(src[12],src[13],src[14],dst_y[3])

#define INIT   INIT_RGB_FLOAT_TO_YUV
      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_420_p_ia_c */

#define FUNC_NAME      rgba_float_to_yuv_420_p_ia_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_Y_8(src[4],src[5],src[6],dst_y[1])
#define INIT   INIT_RGB_FLOAT_TO_YUV

#define CONVERT_Y      \
    RGB_FLOAT_TO_Y_8(src[0],src[1],src[2], \
               dst_y[0]) \
    RGB_FLOAT_TO_Y_8(src[4],src[5],src[6],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_410_p_ia_c */

#define FUNC_NAME      rgba_float_to_yuv_410_p_ia_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     4
#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_Y_8(src[4],src[5],src[6],dst_y[1]) \
    RGB_FLOAT_TO_Y_8(src[8],src[9],src[10],dst_y[2]) \
    RGB_FLOAT_TO_Y_8(src[12],src[13],src[14],dst_y[3])

#define INIT INIT_RGB_FLOAT_TO_YUV

#define CONVERT_Y      \
    RGB_FLOAT_TO_Y_8(src[0],src[1],src[2],dst_y[0]) \
    RGB_FLOAT_TO_Y_8(src[4],src[5],src[6],dst_y[1]) \
    RGB_FLOAT_TO_Y_8(src[8],src[9],src[10],dst_y[2]) \
    RGB_FLOAT_TO_Y_8(src[12],src[13],src[14],dst_y[3])

#define CHROMA_SUB     4

#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_444_p_ia_c */

#define FUNC_NAME      rgba_float_to_yuv_444_p_ia_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_8(src[0],src[1],src[2], \
                       dst_y[0],*dst_u,*dst_v)
#define INIT   INIT_RGB_FLOAT_TO_YUV
      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_444_p_16_ia_c */

#define FUNC_NAME      rgba_float_to_yuv_444_p_16_ia_c
#define IN_TYPE        float
#define OUT_TYPE       uint16_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUV_16(src[0],src[1],src[2], \
                        dst_y[0],*dst_u,*dst_v)
#define INIT   INIT_RGB_FLOAT_TO_YUV
      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#ifndef HQ

/* JPEG */

/****************************************************
 * Conversions to YUVJ 422 P
 ****************************************************/

/* rgb_15_to_yuvj_422_p_c */

#define FUNC_NAME      rgb_15_to_yuvj_422_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUVJ_8(RGB15_TO_R_8(src[0]), \
           RGB15_TO_G_8(src[0]), \
           RGB15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_YJ_8(RGB15_TO_R_8(src[1]), \
         RGB15_TO_G_8(src[1]), \
         RGB15_TO_B_8(src[1]), \
         dst_y[1])

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_15_to_yuvj_422_p_c */

#define FUNC_NAME      bgr_15_to_yuvj_422_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUVJ_8(BGR15_TO_R_8(src[0]), \
           BGR15_TO_G_8(src[0]), \
           BGR15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_YJ_8(BGR15_TO_R_8(src[1]), \
         BGR15_TO_G_8(src[1]), \
         BGR15_TO_B_8(src[1]), \
         dst_y[1])

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_16_to_yuvj_422_p_c */

#define FUNC_NAME      rgb_16_to_yuvj_422_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUVJ_8(RGB16_TO_R_8(src[0]), \
           RGB16_TO_G_8(src[0]), \
           RGB16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_YJ_8(RGB16_TO_R_8(src[1]), \
         RGB16_TO_G_8(src[1]), \
         RGB16_TO_B_8(src[1]), \
         dst_y[1])

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_16_to_yuvj_422_p_c */

#define FUNC_NAME      bgr_16_to_yuvj_422_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUVJ_8(BGR16_TO_R_8(src[0]), \
           BGR16_TO_G_8(src[0]), \
           BGR16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_YJ_8(BGR16_TO_R_8(src[1]), \
         BGR16_TO_G_8(src[1]), \
         BGR16_TO_B_8(src[1]), \
         dst_y[1])

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_24_to_yuvj_422_p_c */

#define FUNC_NAME      rgb_24_to_yuvj_422_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_YJ_8(src[3],src[4],src[5],dst_y[1])

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#endif // !HQ

/* rgb_48_to_yuvj_422_p_c */

#define FUNC_NAME      rgb_48_to_yuvj_422_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_48_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_48_TO_YJ_8(src[3],src[4],src[5],dst_y[1])
#ifdef HQ
#define INIT uint32_t round_tmp;
#endif
      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"
               
/* rgb_float_to_yuvj_422_p_c */

#define FUNC_NAME      rgb_float_to_yuvj_422_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2

#define INIT INIT_RGB_FLOAT_TO_YUV

#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_YJ_8(src[3],src[4],src[5],dst_y[1])

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#ifndef HQ

/* bgr_24_to_yuvj_422_p_c */

#define FUNC_NAME      bgr_24_to_yuvj_422_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUVJ_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_YJ_8(src[5],src[4],src[3],dst_y[1])

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_32_to_yuvj_422_p_c */

#define FUNC_NAME      rgb_32_to_yuvj_422_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_YJ_8(src[4],src[5],src[6],dst_y[1])

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_32_to_yuvj_422_p_c */

#define FUNC_NAME      bgr_32_to_yuvj_422_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUVJ_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_YJ_8(src[6],src[5],src[4],dst_y[1])

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/****************************************************
 * Conversions to YUV 444 P
 ****************************************************/

/* rgb_15_to_yuvj_444_p_c */

#define FUNC_NAME      rgb_15_to_yuvj_444_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     1
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
RGB_24_TO_YUVJ_8(RGB15_TO_R_8(src[0]), \
           RGB15_TO_G_8(src[0]), \
           RGB15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_15_to_yuvj_444_p_c */

#define FUNC_NAME      bgr_15_to_yuvj_444_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     1
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
RGB_24_TO_YUVJ_8(BGR15_TO_R_8(src[0]), \
           BGR15_TO_G_8(src[0]), \
           BGR15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v)
      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_16_to_yuvj_444_p_c */

#define FUNC_NAME      rgb_16_to_yuvj_444_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     1
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
RGB_24_TO_YUVJ_8(RGB16_TO_R_8(src[0]), \
           RGB16_TO_G_8(src[0]), \
           RGB16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_16_to_yuvj_444_p_c */

#define FUNC_NAME      bgr_16_to_yuvj_444_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     1
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
RGB_24_TO_YUVJ_8(BGR16_TO_R_8(src[0]), \
           BGR16_TO_G_8(src[0]), \
           BGR16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_24_to_yuvj_444_p_c */

#define FUNC_NAME      rgb_24_to_yuvj_444_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     3
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_24_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#endif // !HQ

/* rgb_48_to_yuvj_444_p_c */

#define FUNC_NAME      rgb_48_to_yuvj_444_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     3
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_48_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)
#ifdef HQ
#define INIT uint32_t round_tmp;
#endif
      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_float_to_yuvj_444_p_c */

#define FUNC_NAME      rgb_float_to_yuvj_444_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     3
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1

#define INIT INIT_RGB_FLOAT_TO_YUV

#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#ifndef HQ
/* bgr_24_to_yuvj_444_p_c */

#define FUNC_NAME      bgr_24_to_yuvj_444_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     3
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_24_TO_YUVJ_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgb_32_to_yuvj_444_p_c */

#define FUNC_NAME      rgb_32_to_yuvj_444_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_24_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* bgr_32_to_yuvj_444_p_c */

#define FUNC_NAME      bgr_32_to_yuvj_444_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_24_TO_YUVJ_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"


/****************************************************
 * Conversions to YUV 420 P
 ****************************************************/

/* rgb_15_to_yuvj_420_p_c */

#define FUNC_NAME      rgb_15_to_yuvj_420_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUVJ_8(RGB15_TO_R_8(src[0]), \
           RGB15_TO_G_8(src[0]), \
           RGB15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_YJ_8(RGB15_TO_R_8(src[1]), \
         RGB15_TO_G_8(src[1]), \
         RGB15_TO_B_8(src[1]), \
         dst_y[1])

#define CONVERT_Y \
RGB_24_TO_YJ_8(RGB15_TO_R_8(src[0]), \
         RGB15_TO_G_8(src[0]), \
         RGB15_TO_B_8(src[0]), \
         dst_y[0]) \
RGB_24_TO_YJ_8(RGB15_TO_R_8(src[1]), \
         RGB15_TO_G_8(src[1]), \
         RGB15_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* bgr_15_to_yuvj_420_p_c */

#define FUNC_NAME      bgr_15_to_yuvj_420_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUVJ_8(BGR15_TO_R_8(src[0]), \
           BGR15_TO_G_8(src[0]), \
           BGR15_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_YJ_8(BGR15_TO_R_8(src[1]), \
         BGR15_TO_G_8(src[1]), \
         BGR15_TO_B_8(src[1]), \
         dst_y[1])

#define CONVERT_Y \
RGB_24_TO_YJ_8(BGR15_TO_R_8(src[0]), \
         BGR15_TO_G_8(src[0]), \
         BGR15_TO_B_8(src[0]), \
         dst_y[0]) \
RGB_24_TO_YJ_8(BGR15_TO_R_8(src[1]), \
         BGR15_TO_G_8(src[1]), \
         BGR15_TO_B_8(src[1]), \
         dst_y[1])
#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgb_16_to_yuvj_420_p_c */

#define FUNC_NAME      rgb_16_to_yuvj_420_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUVJ_8(RGB16_TO_R_8(src[0]), \
           RGB16_TO_G_8(src[0]), \
           RGB16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_YJ_8(RGB16_TO_R_8(src[1]), \
         RGB16_TO_G_8(src[1]), \
         RGB16_TO_B_8(src[1]), \
         dst_y[1])

#define CONVERT_Y    \
RGB_24_TO_YJ_8(RGB16_TO_R_8(src[0]), \
         RGB16_TO_G_8(src[0]), \
         RGB16_TO_B_8(src[0]), \
         dst_y[0]) \
RGB_24_TO_YJ_8(RGB16_TO_R_8(src[1]), \
         RGB16_TO_G_8(src[1]), \
         RGB16_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* bgr_16_to_yuvj_420_p_c */

#define FUNC_NAME      bgr_16_to_yuvj_420_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     2
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
RGB_24_TO_YUVJ_8(BGR16_TO_R_8(src[0]), \
           BGR16_TO_G_8(src[0]), \
           BGR16_TO_B_8(src[0]), \
           dst_y[0],*dst_u,*dst_v) \
RGB_24_TO_YJ_8(BGR16_TO_R_8(src[1]), \
         BGR16_TO_G_8(src[1]), \
         BGR16_TO_B_8(src[1]), \
         dst_y[1])

#define CONVERT_Y \
RGB_24_TO_YJ_8(BGR16_TO_R_8(src[0]), \
         BGR16_TO_G_8(src[0]), \
         BGR16_TO_B_8(src[0]), \
         dst_y[0]) \
RGB_24_TO_YJ_8(BGR16_TO_R_8(src[1]), \
         BGR16_TO_G_8(src[1]), \
         BGR16_TO_B_8(src[1]), \
         dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgb_24_to_yuvj_420_p_c */

#define FUNC_NAME      rgb_24_to_yuvj_420_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_YJ_8(src[3],src[4],src[5],dst_y[1])

#define CONVERT_Y      \
    RGB_24_TO_YJ_8(src[0],src[1],src[2],dst_y[0]) \
    RGB_24_TO_YJ_8(src[3],src[4],src[5],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

#endif // !HQ

/* rgb_48_to_yuvj_420_p_c */

#define FUNC_NAME      rgb_48_to_yuvj_420_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_48_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_48_TO_YJ_8(src[3],src[4],src[5],dst_y[1])

#define CONVERT_Y      \
    RGB_48_TO_YJ_8(src[0],src[1],src[2],dst_y[0]) \
    RGB_48_TO_YJ_8(src[3],src[4],src[5],dst_y[1])

#define CHROMA_SUB     2

#ifdef HQ
#define INIT uint32_t round_tmp;
#endif

#include "../csp_packed_planar.h"

/* rgb_float_to_yuvj_420_p_c */

#define FUNC_NAME      rgb_float_to_yuvj_420_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2

#define INIT INIT_RGB_FLOAT_TO_YUV

#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_YJ_8(src[3],src[4],src[5],dst_y[1])

#define CONVERT_Y      \
    RGB_FLOAT_TO_YJ_8(src[0],src[1],src[2],dst_y[0]) \
    RGB_FLOAT_TO_YJ_8(src[3],src[4],src[5],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

#ifndef HQ
/* bgr_24_to_yuvj_420_p_c */

#define FUNC_NAME      bgr_24_to_yuvj_420_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     6
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUVJ_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_YJ_8(src[5],src[4],src[3],dst_y[1])

#define CONVERT_Y      \
    RGB_24_TO_YJ_8(src[2],src[1],src[0],dst_y[0]) \
    RGB_24_TO_YJ_8(src[5],src[4],src[3],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgb_32_to_yuvj_420_p_c */

#define FUNC_NAME      rgb_32_to_yuvj_420_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_YJ_8(src[4],src[5],src[6],dst_y[1])

#define CONVERT_Y      \
    RGB_24_TO_YJ_8(src[0],src[1],src[2], \
               dst_y[0]) \
    RGB_24_TO_YJ_8(src[4],src[5],src[6],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* bgr_32_to_yuvj_420_p_c */

#define FUNC_NAME      bgr_32_to_yuvj_420_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_24_TO_YUVJ_8(src[2],src[1],src[0], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_24_TO_YJ_8(src[6],src[5],src[4],dst_y[1])

#define CONVERT_Y      \
    RGB_24_TO_YJ_8(src[2],src[1],src[0], \
             dst_y[0]) \
    RGB_24_TO_YJ_8(src[6],src[5],src[4],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* RGBA -> ... */

/* rgba_32_to_yuvj_422_p_c */

#define FUNC_NAME      rgba_32_to_yuvj_422_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_32_TO_YUVJ_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_32_TO_YJ_8(src[4],src[5],src[6],src[7],dst_y[1])
#define INIT   INIT_RGBA_32                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgba_32_to_yuvj_420_p_c */

#define FUNC_NAME      rgba_32_to_yuvj_420_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_32_TO_YUVJ_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_32_TO_YJ_8(src[4],src[5],src[6],src[7],dst_y[1])
#define INIT   INIT_RGBA_32                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;


#define CONVERT_Y      \
    RGBA_32_TO_YJ_8(src[0],src[1],src[2],src[3], \
               dst_y[0]) \
    RGBA_32_TO_YJ_8(src[4],src[5],src[6],src[7],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgba_32_to_yuvj_444_p_c */

#define FUNC_NAME      rgba_32_to_yuvj_444_p_c
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGBA_32_TO_YUVJ_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v)
#define INIT   INIT_RGBA_32                                             \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;
    
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"
#endif // !HQ
/* rgba_64_to_yuvj_422_p_c */

#define FUNC_NAME      rgba_64_to_yuvj_422_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_64_TO_YUVJ_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_64_TO_YJ_8(src[4],src[5],src[6],src[7],dst_y[1])

#ifdef HQ
#define INIT uint32_t round_tmp;\
INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;
#else // !HQ
#define INIT   INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;
#endif

#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgba_64_to_yuvj_420_p_c */

#define FUNC_NAME      rgba_64_to_yuvj_420_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_64_TO_YUVJ_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_64_TO_YJ_8(src[4],src[5],src[6],src[7],dst_y[1])

#ifdef HQ
#define INIT \
  uint32_t round_tmp; \
  INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;
#else
#define INIT   INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;
#endif

#define CONVERT_Y      \
    RGBA_64_TO_YJ_8(src[0],src[1],src[2],src[3], \
               dst_y[0]) \
    RGBA_64_TO_YJ_8(src[4],src[5],src[6],src[7],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgba_64_to_yuvj_444_p_c */

#define FUNC_NAME      rgba_64_to_yuvj_444_p_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGBA_64_TO_YUVJ_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v)
#ifdef HQ
#define INIT   INIT_RGBA_64 \
  uint32_t round_tmp; \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;
#else
#define INIT  \
  INIT_RGBA_64 \
  uint16_t r_tmp;                                                       \
  uint16_t g_tmp;                                                       \
  uint16_t b_tmp;
#endif

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* No Alpha */
/* rgba_64_to_yuvj_422_p_ia_c */

#define FUNC_NAME      rgba_64_to_yuvj_422_p_ia_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_48_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_48_TO_YJ_8(src[4],src[5],src[6],dst_y[1])
     
#define CHROMA_SUB     1

#ifdef HQ
#define INIT uint32_t round_tmp;
#endif

#include "../csp_packed_planar.h"

/* rgba_64_to_yuvj_420_p_ia_c */

#define FUNC_NAME      rgba_64_to_yuvj_420_p_ia_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
               RGB_48_TO_YUVJ_8(src[0],src[1],src[2],   \
               dst_y[0],*dst_u,*dst_v) \
    RGB_48_TO_YJ_8(src[4],src[5],src[6],dst_y[1])

#define CONVERT_Y      \
    RGB_48_TO_YJ_8(src[0],src[1],src[2], \
               dst_y[0]) \
    RGB_48_TO_YJ_8(src[4],src[5],src[6],dst_y[1])

#define CHROMA_SUB     2

#ifdef HQ
#define INIT uint32_t round_tmp;
#endif

#include "../csp_packed_planar.h"

/* rgba_64_to_yuvj_444_p_ia_c */

#define FUNC_NAME      rgba_64_to_yuvj_444_p_ia_c
#define IN_TYPE        uint16_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_48_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)

      
#define CHROMA_SUB     1

#ifdef HQ
#define INIT uint32_t round_tmp;
#endif

#include "../csp_packed_planar.h"
               
/* RGBA float -> */

/* rgba_float_to_yuvj_422_p_c */

#define FUNC_NAME      rgba_float_to_yuvj_422_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_FLOAT_TO_YUVJ_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_FLOAT_TO_YJ_8(src[4],src[5],src[6],src[7],dst_y[1])
#define INIT   INIT_RGBA_FLOAT \
  float r_tmp;                                                          \
  float g_tmp;                                                          \
  float b_tmp;                                                          \
  INIT_RGB_FLOAT_TO_YUV

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgba_float_to_yuvj_420_p_c */

#define FUNC_NAME      rgba_float_to_yuvj_420_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGBA_FLOAT_TO_YUVJ_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v) \
    RGBA_FLOAT_TO_YJ_8(src[4],src[5],src[6],src[7],dst_y[1])
#define INIT   INIT_RGBA_FLOAT \
  float r_tmp;                                                          \
  float g_tmp;                                                          \
  float b_tmp;                                                          \
  INIT_RGB_FLOAT_TO_YUV


#define CONVERT_Y      \
    RGBA_FLOAT_TO_YJ_8(src[0],src[1],src[2],src[3], \
               dst_y[0]) \
    RGBA_FLOAT_TO_YJ_8(src[4],src[5],src[6],src[7],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_444_p_c */

#define FUNC_NAME      rgba_float_to_yuvj_444_p_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGBA_FLOAT_TO_YUVJ_8(src[0],src[1],src[2],src[3], \
               dst_y[0],*dst_u,*dst_v)
#define INIT   INIT_RGBA_FLOAT \
  float r_tmp;                                                          \
  float g_tmp;                                                          \
  float b_tmp;                                                          \
               INIT_RGB_FLOAT_TO_YUV

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"


/* RGBA float -> (No alpha) */

/* rgba_float_to_yuvj_422_p_ia_c */

#define FUNC_NAME      rgba_float_to_yuvj_422_p_ia_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_YJ_8(src[4],src[5],src[6],dst_y[1])
#define INIT   INIT_RGB_FLOAT_TO_YUV

      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

/* rgba_float_to_yuvj_420_p_ia_c */

#define FUNC_NAME      rgba_float_to_yuvj_420_p_ia_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  2
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     2
#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v) \
    RGB_FLOAT_TO_YJ_8(src[4],src[5],src[6],dst_y[1])
#define INIT   INIT_RGB_FLOAT_TO_YUV

#define CONVERT_Y      \
    RGB_FLOAT_TO_YJ_8(src[0],src[1],src[2], \
               dst_y[0]) \
    RGB_FLOAT_TO_YJ_8(src[4],src[5],src[6],dst_y[1])

#define CHROMA_SUB     2

#include "../csp_packed_planar.h"

/* rgba_float_to_yuv_444_p_ia_c */

#define FUNC_NAME      rgba_float_to_yuvj_444_p_ia_c
#define IN_TYPE        float
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     4
#define OUT_ADVANCE_Y  1
#define OUT_ADVANCE_UV 1
#define NUM_PIXELS     1
#define CONVERT_YUV    \
    RGB_FLOAT_TO_YUVJ_8(src[0],src[1],src[2], \
               dst_y[0],*dst_u,*dst_v)
#define INIT   INIT_RGB_FLOAT_TO_YUV
      
#define CHROMA_SUB     1

#include "../csp_packed_planar.h"

#ifdef HQ
void gavl_init_rgb_yuv_funcs_hq(gavl_pixelformat_function_table_t * tab, const gavl_video_options_t * opt)
#else
void gavl_init_rgb_yuv_funcs_c(gavl_pixelformat_function_table_t * tab, const gavl_video_options_t * opt)
#endif // HQ    
  {
  //  _init_rgb_to_yuv_c();
#ifndef HQ
  tab->rgb_15_to_yuy2 =      rgb_15_to_yuy2_c;
  tab->rgb_15_to_uyvy =      rgb_15_to_uyvy_c;
  tab->rgb_15_to_yuva_32 =      rgb_15_to_yuva_32_c;
  tab->rgb_15_to_yuv_420_p = rgb_15_to_yuv_420_p_c;
  tab->rgb_15_to_yuv_410_p = rgb_15_to_yuv_410_p_c;
  tab->rgb_15_to_yuv_422_p = rgb_15_to_yuv_422_p_c;
  tab->rgb_15_to_yuv_422_p_16 = rgb_15_to_yuv_422_p_16_c;
  tab->rgb_15_to_yuv_411_p = rgb_15_to_yuv_411_p_c;
  tab->rgb_15_to_yuv_444_p = rgb_15_to_yuv_444_p_c;
  tab->rgb_15_to_yuv_444_p_16 = rgb_15_to_yuv_444_p_16_c;
  tab->rgb_15_to_yuvj_420_p = rgb_15_to_yuvj_420_p_c;
  tab->rgb_15_to_yuvj_422_p = rgb_15_to_yuvj_422_p_c;
  tab->rgb_15_to_yuvj_444_p = rgb_15_to_yuvj_444_p_c;
  tab->rgb_15_to_yuva_64 = rgb_15_to_yuva_64_c;
  tab->rgb_15_to_yuva_float = rgb_15_to_yuva_float_c;
  tab->rgb_15_to_yuv_float = rgb_15_to_yuv_float_c;

  
  tab->bgr_15_to_yuy2 =      bgr_15_to_yuy2_c;
  tab->bgr_15_to_uyvy =      bgr_15_to_uyvy_c;
  tab->bgr_15_to_yuva_32 =      bgr_15_to_yuva_32_c;
  tab->bgr_15_to_yuv_420_p = bgr_15_to_yuv_420_p_c;
  tab->bgr_15_to_yuv_410_p = bgr_15_to_yuv_410_p_c;
  tab->bgr_15_to_yuv_422_p = bgr_15_to_yuv_422_p_c;
  tab->bgr_15_to_yuv_422_p_16 = bgr_15_to_yuv_422_p_16_c;
  tab->bgr_15_to_yuv_411_p = bgr_15_to_yuv_411_p_c;
  tab->bgr_15_to_yuv_444_p = bgr_15_to_yuv_444_p_c;
  tab->bgr_15_to_yuv_444_p_16 = bgr_15_to_yuv_444_p_16_c;
  tab->bgr_15_to_yuvj_420_p = bgr_15_to_yuvj_420_p_c;
  tab->bgr_15_to_yuvj_422_p = bgr_15_to_yuvj_422_p_c;
  tab->bgr_15_to_yuvj_444_p = bgr_15_to_yuvj_444_p_c;
  tab->bgr_15_to_yuva_64 = bgr_15_to_yuva_64_c;
  tab->bgr_15_to_yuva_float = bgr_15_to_yuva_float_c;
  tab->bgr_15_to_yuv_float = bgr_15_to_yuv_float_c;

  
  tab->rgb_16_to_yuy2 =      rgb_16_to_yuy2_c;
  tab->rgb_16_to_uyvy =      rgb_16_to_uyvy_c;
  tab->rgb_16_to_yuva_32 =      rgb_16_to_yuva_32_c;
  tab->rgb_16_to_yuv_420_p = rgb_16_to_yuv_420_p_c;
  tab->rgb_16_to_yuv_410_p = rgb_16_to_yuv_410_p_c;
  tab->rgb_16_to_yuv_422_p = rgb_16_to_yuv_422_p_c;
  tab->rgb_16_to_yuv_422_p_16 = rgb_16_to_yuv_422_p_16_c;
  tab->rgb_16_to_yuv_411_p = rgb_16_to_yuv_411_p_c;
  tab->rgb_16_to_yuv_444_p = rgb_16_to_yuv_444_p_c;
  tab->rgb_16_to_yuv_444_p_16 = rgb_16_to_yuv_444_p_16_c;
  tab->rgb_16_to_yuvj_420_p = rgb_16_to_yuvj_420_p_c;
  tab->rgb_16_to_yuvj_422_p = rgb_16_to_yuvj_422_p_c;
  tab->rgb_16_to_yuvj_444_p = rgb_16_to_yuvj_444_p_c;
  tab->rgb_16_to_yuva_64 = rgb_16_to_yuva_64_c;
  tab->rgb_16_to_yuva_float = rgb_16_to_yuva_float_c;
  tab->rgb_16_to_yuv_float = rgb_16_to_yuv_float_c;

  
  tab->bgr_16_to_yuy2 =      bgr_16_to_yuy2_c;
  tab->bgr_16_to_uyvy =      bgr_16_to_uyvy_c;
  tab->bgr_16_to_yuva_32 =      bgr_16_to_yuva_32_c;
  tab->bgr_16_to_yuv_420_p = bgr_16_to_yuv_420_p_c;
  tab->bgr_16_to_yuv_410_p = bgr_16_to_yuv_410_p_c;
  tab->bgr_16_to_yuv_422_p = bgr_16_to_yuv_422_p_c;
  tab->bgr_16_to_yuv_422_p_16 = bgr_16_to_yuv_422_p_16_c;
  tab->bgr_16_to_yuv_411_p = bgr_16_to_yuv_411_p_c;
  tab->bgr_16_to_yuv_444_p = bgr_16_to_yuv_444_p_c;
  tab->bgr_16_to_yuv_444_p_16 = bgr_16_to_yuv_444_p_16_c;
  tab->bgr_16_to_yuvj_420_p = bgr_16_to_yuvj_420_p_c;
  tab->bgr_16_to_yuvj_422_p = bgr_16_to_yuvj_422_p_c;
  tab->bgr_16_to_yuvj_444_p = bgr_16_to_yuvj_444_p_c;
  tab->bgr_16_to_yuva_64 = bgr_16_to_yuva_64_c;
  tab->bgr_16_to_yuva_float = bgr_16_to_yuva_float_c;
  tab->bgr_16_to_yuv_float = bgr_16_to_yuv_float_c;

  tab->rgb_24_to_yuy2 =      rgb_24_to_yuy2_c;
  tab->rgb_24_to_uyvy =      rgb_24_to_uyvy_c;
  tab->rgb_24_to_yuva_32 =      rgb_24_to_yuva_32_c;
  tab->rgb_24_to_yuv_420_p = rgb_24_to_yuv_420_p_c;
  tab->rgb_24_to_yuv_410_p = rgb_24_to_yuv_410_p_c;
  tab->rgb_24_to_yuv_422_p = rgb_24_to_yuv_422_p_c;
  tab->rgb_24_to_yuv_422_p_16 = rgb_24_to_yuv_422_p_16_c;
  tab->rgb_24_to_yuv_411_p = rgb_24_to_yuv_411_p_c;
  tab->rgb_24_to_yuv_444_p = rgb_24_to_yuv_444_p_c;
  tab->rgb_24_to_yuv_444_p_16 = rgb_24_to_yuv_444_p_16_c;
  tab->rgb_24_to_yuvj_420_p = rgb_24_to_yuvj_420_p_c;
  tab->rgb_24_to_yuvj_422_p = rgb_24_to_yuvj_422_p_c;
  tab->rgb_24_to_yuvj_444_p = rgb_24_to_yuvj_444_p_c;
  tab->rgb_24_to_yuva_64 = rgb_24_to_yuva_64_c;
  tab->rgb_24_to_yuva_float = rgb_24_to_yuva_float_c;
  tab->rgb_24_to_yuv_float = rgb_24_to_yuv_float_c;

  tab->bgr_24_to_yuy2 =      bgr_24_to_yuy2_c;
  tab->bgr_24_to_uyvy =      bgr_24_to_uyvy_c;
  tab->bgr_24_to_yuva_32 =      bgr_24_to_yuva_32_c;
  tab->bgr_24_to_yuv_420_p = bgr_24_to_yuv_420_p_c;
  tab->bgr_24_to_yuv_410_p = bgr_24_to_yuv_410_p_c;
  tab->bgr_24_to_yuv_422_p = bgr_24_to_yuv_422_p_c;
  tab->bgr_24_to_yuv_422_p_16 = bgr_24_to_yuv_422_p_16_c;
  tab->bgr_24_to_yuv_411_p = bgr_24_to_yuv_411_p_c;
  tab->bgr_24_to_yuv_444_p = bgr_24_to_yuv_444_p_c;
  tab->bgr_24_to_yuv_444_p_16 = bgr_24_to_yuv_444_p_16_c;
  tab->bgr_24_to_yuvj_420_p = bgr_24_to_yuvj_420_p_c;
  tab->bgr_24_to_yuvj_422_p = bgr_24_to_yuvj_422_p_c;
  tab->bgr_24_to_yuvj_444_p = bgr_24_to_yuvj_444_p_c;
  tab->bgr_24_to_yuva_64 = bgr_24_to_yuva_64_c;
  tab->bgr_24_to_yuva_float = bgr_24_to_yuva_float_c;
  tab->bgr_24_to_yuv_float = bgr_24_to_yuv_float_c;

#endif // HQ
  
  tab->rgb_48_to_yuy2 =      rgb_48_to_yuy2_c;
  tab->rgb_48_to_uyvy =      rgb_48_to_uyvy_c;
  tab->rgb_48_to_yuva_32 =      rgb_48_to_yuva_32_c;
  tab->rgb_48_to_yuv_420_p = rgb_48_to_yuv_420_p_c;
  tab->rgb_48_to_yuv_410_p = rgb_48_to_yuv_410_p_c;
  tab->rgb_48_to_yuv_422_p = rgb_48_to_yuv_422_p_c;
  tab->rgb_48_to_yuv_411_p = rgb_48_to_yuv_411_p_c;
  tab->rgb_48_to_yuv_444_p = rgb_48_to_yuv_444_p_c;
  tab->rgb_48_to_yuvj_420_p = rgb_48_to_yuvj_420_p_c;
  tab->rgb_48_to_yuvj_422_p = rgb_48_to_yuvj_422_p_c;
  tab->rgb_48_to_yuvj_444_p = rgb_48_to_yuvj_444_p_c;
#ifndef HQ
  tab->rgb_48_to_yuv_422_p_16 = rgb_48_to_yuv_422_p_16_c;
  tab->rgb_48_to_yuv_444_p_16 = rgb_48_to_yuv_444_p_16_c;
  tab->rgb_48_to_yuva_64 = rgb_48_to_yuva_64_c;
  tab->rgb_48_to_yuva_float = rgb_48_to_yuva_float_c;
  tab->rgb_48_to_yuv_float = rgb_48_to_yuv_float_c;


  
#endif // !HQ

  tab->rgb_float_to_yuy2 =      rgb_float_to_yuy2_c;
  tab->rgb_float_to_uyvy =      rgb_float_to_uyvy_c;
  tab->rgb_float_to_yuva_32 =      rgb_float_to_yuva_32_c;
  tab->rgb_float_to_yuv_420_p = rgb_float_to_yuv_420_p_c;
  tab->rgb_float_to_yuv_410_p = rgb_float_to_yuv_410_p_c;
  tab->rgb_float_to_yuv_422_p = rgb_float_to_yuv_422_p_c;
  tab->rgb_float_to_yuv_422_p_16 = rgb_float_to_yuv_422_p_16_c;
  tab->rgb_float_to_yuv_411_p = rgb_float_to_yuv_411_p_c;
  tab->rgb_float_to_yuv_444_p = rgb_float_to_yuv_444_p_c;
  tab->rgb_float_to_yuv_444_p_16 = rgb_float_to_yuv_444_p_16_c;
  tab->rgb_float_to_yuvj_420_p = rgb_float_to_yuvj_420_p_c;
  tab->rgb_float_to_yuvj_422_p = rgb_float_to_yuvj_422_p_c;
  tab->rgb_float_to_yuvj_444_p = rgb_float_to_yuvj_444_p_c;

  tab->rgb_float_to_yuva_64 = rgb_float_to_yuva_64_c;

  
#ifndef HQ

  tab->rgb_float_to_yuva_float = rgb_float_to_yuva_float_c;
  tab->rgb_float_to_yuv_float = rgb_float_to_yuv_float_c;
  
  tab->rgb_32_to_yuy2 =      rgb_32_to_yuy2_c;
  tab->rgb_32_to_uyvy =      rgb_32_to_uyvy_c;
  tab->rgb_32_to_yuva_32 =      rgb_32_to_yuva_32_c;
  tab->rgb_32_to_yuv_420_p = rgb_32_to_yuv_420_p_c;
  tab->rgb_32_to_yuv_410_p = rgb_32_to_yuv_410_p_c;
  tab->rgb_32_to_yuv_422_p = rgb_32_to_yuv_422_p_c;
  tab->rgb_32_to_yuv_422_p_16 = rgb_32_to_yuv_422_p_16_c;
  tab->rgb_32_to_yuv_411_p = rgb_32_to_yuv_411_p_c;
  tab->rgb_32_to_yuv_444_p = rgb_32_to_yuv_444_p_c;
  tab->rgb_32_to_yuv_444_p_16 = rgb_32_to_yuv_444_p_16_c;
  tab->rgb_32_to_yuvj_420_p = rgb_32_to_yuvj_420_p_c;
  tab->rgb_32_to_yuvj_422_p = rgb_32_to_yuvj_422_p_c;
  tab->rgb_32_to_yuvj_444_p = rgb_32_to_yuvj_444_p_c;
  tab->rgb_32_to_yuva_64 = rgb_32_to_yuva_64_c;
  tab->rgb_32_to_yuva_float = rgb_32_to_yuva_float_c;
  tab->rgb_32_to_yuv_float = rgb_32_to_yuv_float_c;

  
  tab->bgr_32_to_yuy2 =      bgr_32_to_yuy2_c;
  tab->bgr_32_to_uyvy =      bgr_32_to_uyvy_c;
  tab->bgr_32_to_yuva_32 =      bgr_32_to_yuva_32_c;
  tab->bgr_32_to_yuv_420_p = bgr_32_to_yuv_420_p_c;
  tab->bgr_32_to_yuv_410_p = bgr_32_to_yuv_410_p_c;
  tab->bgr_32_to_yuv_422_p = bgr_32_to_yuv_422_p_c;
  tab->bgr_32_to_yuv_422_p_16 = bgr_32_to_yuv_422_p_16_c;
  tab->bgr_32_to_yuv_411_p = bgr_32_to_yuv_411_p_c;
  tab->bgr_32_to_yuv_444_p = bgr_32_to_yuv_444_p_c;
  tab->bgr_32_to_yuv_444_p_16 = bgr_32_to_yuv_444_p_16_c;
  tab->bgr_32_to_yuvj_420_p = bgr_32_to_yuvj_420_p_c;
  tab->bgr_32_to_yuvj_422_p = bgr_32_to_yuvj_422_p_c;
  tab->bgr_32_to_yuvj_444_p = bgr_32_to_yuvj_444_p_c;
  tab->bgr_32_to_yuva_64 = bgr_32_to_yuva_64_c;
  tab->bgr_32_to_yuva_float = bgr_32_to_yuva_float_c;
  tab->bgr_32_to_yuv_float = bgr_32_to_yuv_float_c;

  tab->rgba_32_to_yuva_32 =      rgba_32_to_yuva_32_c;
  tab->rgba_32_to_yuva_float = rgba_32_to_yuva_float_c;
  tab->rgba_64_to_yuva_64 = rgba_64_to_yuva_64_c;
  tab->rgba_64_to_yuva_float = rgba_64_to_yuva_float_c;
  tab->rgba_float_to_yuva_float = rgba_float_to_yuva_float_c;
  tab->rgba_32_to_yuva_64 = rgba_32_to_yuva_64_c;

#endif // !HQ

  tab->rgba_64_to_yuva_32 =   rgba_64_to_yuva_32_c;
  
  tab->rgba_float_to_yuva_32 =      rgba_float_to_yuva_32_c;

  tab->rgba_float_to_yuva_64 = rgba_float_to_yuva_64_c;
  
  if(opt->alpha_mode == GAVL_ALPHA_BLEND_COLOR)
    {
#ifndef HQ
    tab->rgba_32_to_yuy2 =      rgba_32_to_yuy2_c;
    tab->rgba_32_to_uyvy =      rgba_32_to_uyvy_c;
    tab->rgba_32_to_yuv_420_p = rgba_32_to_yuv_420_p_c;
    tab->rgba_32_to_yuv_410_p = rgba_32_to_yuv_410_p_c;
    tab->rgba_32_to_yuv_422_p = rgba_32_to_yuv_422_p_c;
    tab->rgba_32_to_yuv_422_p_16 = rgba_32_to_yuv_422_p_16_c;
    tab->rgba_32_to_yuv_411_p = rgba_32_to_yuv_411_p_c;
    tab->rgba_32_to_yuv_444_p = rgba_32_to_yuv_444_p_c;
    tab->rgba_32_to_yuv_444_p_16 = rgba_32_to_yuv_444_p_16_c;
    tab->rgba_32_to_yuvj_420_p = rgba_32_to_yuvj_420_p_c;
    tab->rgba_32_to_yuvj_422_p = rgba_32_to_yuvj_422_p_c;
    tab->rgba_32_to_yuvj_444_p = rgba_32_to_yuvj_444_p_c;
    tab->rgba_32_to_yuv_float = rgba_32_to_yuv_float_c;
    
#endif // HQ
    
    tab->rgba_64_to_yuy2 =      rgba_64_to_yuy2_c;
    tab->rgba_64_to_uyvy =      rgba_64_to_uyvy_c;
    tab->rgba_64_to_yuv_420_p = rgba_64_to_yuv_420_p_c;
    tab->rgba_64_to_yuv_410_p = rgba_64_to_yuv_410_p_c;
    tab->rgba_64_to_yuv_422_p = rgba_64_to_yuv_422_p_c;
    tab->rgba_64_to_yuv_411_p = rgba_64_to_yuv_411_p_c;
    tab->rgba_64_to_yuv_444_p = rgba_64_to_yuv_444_p_c;
    tab->rgba_64_to_yuvj_420_p = rgba_64_to_yuvj_420_p_c;
    tab->rgba_64_to_yuvj_422_p = rgba_64_to_yuvj_422_p_c;
    tab->rgba_64_to_yuvj_444_p = rgba_64_to_yuvj_444_p_c;
#ifndef HQ
    tab->rgba_64_to_yuv_422_p_16 = rgba_64_to_yuv_422_p_16_c;
    tab->rgba_64_to_yuv_444_p_16 = rgba_64_to_yuv_444_p_16_c;
    tab->rgba_64_to_yuv_float  = rgba_64_to_yuv_float_c;
#endif // HQ

    tab->rgba_float_to_yuy2 =      rgba_float_to_yuy2_c;
    tab->rgba_float_to_uyvy =      rgba_float_to_uyvy_c;
    tab->rgba_float_to_yuv_420_p = rgba_float_to_yuv_420_p_c;
    tab->rgba_float_to_yuv_410_p = rgba_float_to_yuv_410_p_c;
    tab->rgba_float_to_yuv_422_p = rgba_float_to_yuv_422_p_c;
    tab->rgba_float_to_yuv_422_p_16 = rgba_float_to_yuv_422_p_16_c;
    tab->rgba_float_to_yuv_411_p = rgba_float_to_yuv_411_p_c;
    tab->rgba_float_to_yuv_444_p = rgba_float_to_yuv_444_p_c;
    tab->rgba_float_to_yuv_444_p_16 = rgba_float_to_yuv_444_p_16_c;
    tab->rgba_float_to_yuvj_420_p = rgba_float_to_yuvj_420_p_c;
    tab->rgba_float_to_yuvj_422_p = rgba_float_to_yuvj_422_p_c;
    tab->rgba_float_to_yuvj_444_p = rgba_float_to_yuvj_444_p_c;
    tab->rgba_float_to_yuv_float = rgba_float_to_yuv_float_c;
    }
  if(opt->alpha_mode == GAVL_ALPHA_IGNORE)
    {
#ifndef HQ

    tab->rgba_32_to_yuy2 =         rgb_32_to_yuy2_c;
    tab->rgba_32_to_uyvy =         rgb_32_to_uyvy_c;
    tab->rgba_32_to_yuv_420_p    = rgb_32_to_yuv_420_p_c;
    tab->rgba_32_to_yuv_410_p    = rgb_32_to_yuv_410_p_c;
    tab->rgba_32_to_yuv_422_p    = rgb_32_to_yuv_422_p_c;
    tab->rgba_32_to_yuv_422_p_16 = rgb_32_to_yuv_422_p_16_c;
    tab->rgba_32_to_yuv_411_p = rgb_32_to_yuv_411_p_c;
    tab->rgba_32_to_yuv_444_p = rgb_32_to_yuv_444_p_c;
    tab->rgba_32_to_yuv_444_p_16 = rgb_32_to_yuv_444_p_16_c;
    tab->rgba_32_to_yuvj_420_p = rgb_32_to_yuvj_420_p_c;
    tab->rgba_32_to_yuvj_422_p = rgb_32_to_yuvj_422_p_c;
    tab->rgba_32_to_yuvj_444_p = rgb_32_to_yuvj_444_p_c;
    tab->rgba_32_to_yuv_float  = rgb_32_to_yuv_float_c;

#endif // HQ

    tab->rgba_64_to_yuy2 =      rgba_64_to_yuy2_ia_c;
    tab->rgba_64_to_uyvy =      rgba_64_to_uyvy_ia_c;
    tab->rgba_64_to_yuv_420_p = rgba_64_to_yuv_420_p_ia_c;
    tab->rgba_64_to_yuv_410_p = rgba_64_to_yuv_410_p_ia_c;
    tab->rgba_64_to_yuv_422_p = rgba_64_to_yuv_422_p_ia_c;
    tab->rgba_64_to_yuv_411_p = rgba_64_to_yuv_411_p_ia_c;
    tab->rgba_64_to_yuv_444_p = rgba_64_to_yuv_444_p_ia_c;
    tab->rgba_64_to_yuvj_420_p = rgba_64_to_yuvj_420_p_ia_c;
    tab->rgba_64_to_yuvj_422_p = rgba_64_to_yuvj_422_p_ia_c;
    tab->rgba_64_to_yuvj_444_p = rgba_64_to_yuvj_444_p_ia_c;


    
#ifndef HQ
    tab->rgba_64_to_yuv_float = rgba_64_to_yuv_float_ia_c;
    tab->rgba_64_to_yuv_422_p_16 = rgba_64_to_yuv_422_p_16_ia_c;
    tab->rgba_64_to_yuv_444_p_16 = rgba_64_to_yuv_444_p_16_ia_c;
#endif // HQ
    tab->rgba_float_to_yuv_float = rgba_float_to_yuv_float_ia_c;
    
    tab->rgba_float_to_yuy2 =      rgba_float_to_yuy2_ia_c;
    tab->rgba_float_to_uyvy =      rgba_float_to_uyvy_ia_c;
    tab->rgba_float_to_yuv_420_p = rgba_float_to_yuv_420_p_ia_c;
    tab->rgba_float_to_yuv_410_p = rgba_float_to_yuv_410_p_ia_c;
    tab->rgba_float_to_yuv_422_p = rgba_float_to_yuv_422_p_ia_c;
    tab->rgba_float_to_yuv_422_p_16 = rgba_float_to_yuv_422_p_16_ia_c;
    tab->rgba_float_to_yuv_411_p = rgba_float_to_yuv_411_p_ia_c;
    tab->rgba_float_to_yuv_444_p = rgba_float_to_yuv_444_p_ia_c;
    tab->rgba_float_to_yuv_444_p_16 = rgba_float_to_yuv_444_p_16_ia_c;
    tab->rgba_float_to_yuvj_420_p = rgba_float_to_yuvj_420_p_ia_c;
    tab->rgba_float_to_yuvj_422_p = rgba_float_to_yuvj_422_p_ia_c;
    tab->rgba_float_to_yuvj_444_p = rgba_float_to_yuvj_444_p_ia_c;
    }
  }
