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

// #include <stdio.h>
#include <gavl/gavl.h>
#include <video.h>
#include <colorspace.h>

#include "colorspace_tables.h"
#include "colorspace_macros.h"

/*
 * Macros for pixel conversion
 */

#define COPY_24 dst[0]=src[0];dst[1]=src[1];dst[2]=src[2];

#define COPY_24_TO_48 \
  dst[0]=RGB_8_TO_16(src[0]);\
  dst[1]=RGB_8_TO_16(src[1]);\
  dst[2]=RGB_8_TO_16(src[2]);

#define COPY_24_TO_FLOAT                           \
  dst[0]=RGB_8_TO_FLOAT(src[0]);\
  dst[1]=RGB_8_TO_FLOAT(src[1]);\
  dst[2]=RGB_8_TO_FLOAT(src[2]);

#define COPY_48_TO_24 \
  RGB_16_TO_8(src[0], dst[0]);\
  RGB_16_TO_8(src[1], dst[1]);\
  RGB_16_TO_8(src[2], dst[2]);

#define COPY_48_TO_48 \
  dst[0]=src[0];\
  dst[1]=src[1];\
  dst[2]=src[2];


#define COPY_48_TO_FLOAT                        \
  dst[0]=RGB_16_TO_FLOAT(src[0]);\
  dst[1]=RGB_16_TO_FLOAT(src[1]);\
  dst[2]=RGB_16_TO_FLOAT(src[2]);

#define COPY_FLOAT_TO_24 \
  RGB_FLOAT_TO_8(src[0], dst[0]);\
  RGB_FLOAT_TO_8(src[1], dst[1]);\
  RGB_FLOAT_TO_8(src[2], dst[2]);

#define COPY_FLOAT_TO_48 \
  RGB_FLOAT_TO_16(src[0], dst[0]);\
  RGB_FLOAT_TO_16(src[1], dst[1]);\
  RGB_FLOAT_TO_16(src[2], dst[2]);

#define COPY_FLOAT_TO_FLOAT                        \
  dst[0]=src[0];                                   \
  dst[1]=src[1];                                   \
  dst[2]=src[2];

#define SWAP_24 dst[2]=src[0];dst[1]=src[1];dst[0]=src[2];

#define SWAP_24_TO_48 \
  dst[2]=RGB_8_TO_16(src[0]);\
  dst[1]=RGB_8_TO_16(src[1]);\
  dst[0]=RGB_8_TO_16(src[2]);

#define SWAP_24_TO_FLOAT \
  dst[2]=RGB_8_TO_FLOAT(src[0]);\
  dst[1]=RGB_8_TO_FLOAT(src[1]);\
  dst[0]=RGB_8_TO_FLOAT(src[2]);

#define SWAP_48_TO_24 \
  RGB_16_TO_8(src[0], dst[2]);\
  RGB_16_TO_8(src[1], dst[1]);\
  RGB_16_TO_8(src[2], dst[0]);

#define SWAP_48_TO_FLOAT \
  RGB_16_TO_FLOAT(src[0], dst[2]);\
  RGB_16_TO_FLOAT(src[1], dst[1]);\
  RGB_16_TO_FLOAT(src[2], dst[0]);

#define SWAP_FLOAT_TO_24 \
  RGB_FLOAT_TO_8(src[0], dst[2]);\
  RGB_FLOAT_TO_8(src[1], dst[1]);\
  RGB_FLOAT_TO_8(src[2], dst[0]);

/*
 *  Needs the following macros:
 *  IN_TYPE:     Type of the input pointer
 *  OUT_TYPE:    Type of the output pointer
 *  IN_ADVANCE:  Input advance
 *  OUT_ADVANCE: Output advance
 *  FUNC_NAME:   Name of the function
 *  NUM_PIXELS:  The number of pixels the conversion processes at once
 *  CONVERT:     This macro takes no arguments and makes the appropriate conversion
 *               from <src> to <dst>
 *  INIT:        Variable declarations and initialization (can be empty)
 */

#ifndef HQ

/* swap_rgb_24_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME swap_rgb_24_c
#define CONVERT    SWAP_24

#include "../csp_packed_packed.h"

/* swap_rgb_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME swap_rgb_32_c
#define CONVERT    SWAP_24

#include "../csp_packed_packed.h"

/* swap_rgb_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME swap_rgb_16_c
#define CONVERT \
   *dst = (((*src & RGB16_UPPER_MASK) >> 11)|\
           (*src & RGB16_MIDDLE_MASK)|\
          ((*src & RGB16_LOWER_MASK) << 11));

#include "../csp_packed_packed.h"

/* swap_rgb_15_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME swap_rgb_15_c
#define CONVERT \
  *dst = (((*src & RGB15_UPPER_MASK) >> 10)|\
          (*src & RGB15_MIDDLE_MASK)|\
         ((*src & RGB15_LOWER_MASK) << 10));
#include "../csp_packed_packed.h"


/* rgb_15_to_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_16_c
#define CONVERT \
  *dst = *src + (*src & 0xFFE0);
#include "../csp_packed_packed.h"

/* rgb_15_to_24_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_24_c
#define CONVERT \
  dst[0] = RGB15_TO_R_8(*src);\
  dst[1] = RGB15_TO_G_8(*src);\
  dst[2] = RGB15_TO_B_8(*src);

#include "../csp_packed_packed.h"

/* rgb_15_to_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_32_c
#define CONVERT \
  dst[0] = RGB15_TO_R_8(*src);\
  dst[1] = RGB15_TO_G_8(*src);\
  dst[2] = RGB15_TO_B_8(*src);

#include "../csp_packed_packed.h"

/* rgb_15_to_48_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_48_c
#define CONVERT \
  dst[0] = RGB15_TO_R_16(*src);\
  dst[1] = RGB15_TO_G_16(*src);\
  dst[2] = RGB15_TO_B_16(*src);

#include "../csp_packed_packed.h"

/* rgb_15_to_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_float_c
#define CONVERT \
  dst[0] = RGB15_TO_R_FLOAT(*src);\
  dst[1] = RGB15_TO_G_FLOAT(*src);\
  dst[2] = RGB15_TO_B_FLOAT(*src);

#include "../csp_packed_packed.h"

/* rgb_16_to_15_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_15_c
#define CONVERT \
    *dst = (*src & RGB16_LOWER_MASK) | \
   (((*src & (RGB16_MIDDLE_MASK|RGB16_UPPER_MASK)) >> 1) & \
   (RGB15_UPPER_MASK | RGB15_MIDDLE_MASK));

#include "../csp_packed_packed.h"

/* rgb_16_to_24_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_24_c
#define CONVERT \
  dst[0] = RGB16_TO_R_8(*src); \
  dst[1] = RGB16_TO_G_8(*src); \
  dst[2] = RGB16_TO_B_8(*src);

#include "../csp_packed_packed.h"

/* rgb_16_to_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_32_c
#define CONVERT \
  dst[0] = RGB16_TO_R_8(*src); \
  dst[1] = RGB16_TO_G_8(*src); \
  dst[2] = RGB16_TO_B_8(*src);

#include "../csp_packed_packed.h"

/* rgb_16_to_48_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_48_c
#define CONVERT \
  dst[0] = RGB16_TO_R_16(*src); \
  dst[1] = RGB16_TO_G_16(*src); \
  dst[2] = RGB16_TO_B_16(*src);

#include "../csp_packed_packed.h"

/* rgb_16_to_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_float_c
#define CONVERT \
  dst[0] = RGB16_TO_R_FLOAT(*src); \
  dst[1] = RGB16_TO_G_FLOAT(*src); \
  dst[2] = RGB16_TO_B_FLOAT(*src);

#include "../csp_packed_packed.h"

/* rgb_24_to_15_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_15_c
#define CONVERT \
  PACK_8_TO_RGB15(src[0],src[1],src[2],*dst)

#include "../csp_packed_packed.h"

/* rgb_24_to_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_16_c
#define CONVERT \
  PACK_8_TO_RGB16(src[0],src[1],src[2],*dst)

#include "../csp_packed_packed.h"

/* rgb_24_to_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_32_c
#define CONVERT \
  COPY_24

#include "../csp_packed_packed.h"

/* rgb_24_to_48_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_48_c
#define CONVERT \
  COPY_24_TO_48

#include "../csp_packed_packed.h"

/* rgb_24_to_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_float_c
#define CONVERT \
  COPY_24_TO_FLOAT

#include "../csp_packed_packed.h"

/* rgb_32_to_15_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_15_c
#define CONVERT \
  PACK_8_TO_RGB15(src[0],src[1],src[2],*dst)

#include "../csp_packed_packed.h"

/* rgb_32_to_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_16_c
#define CONVERT \
  PACK_8_TO_RGB16(src[0],src[1],src[2],*dst)

#include "../csp_packed_packed.h"

/* rgb_32_to_24_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_24_c
#define CONVERT \
  COPY_24  

#include "../csp_packed_packed.h"

/* rgb_32_to_48_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_48_c
#define CONVERT \
  COPY_24_TO_48

#include "../csp_packed_packed.h"

/* rgb_32_to_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_float_c
#define CONVERT \
  COPY_24_TO_FLOAT

#include "../csp_packed_packed.h"

/* rgb_48_to_15_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_15_c
#define CONVERT \
  PACK_16_TO_RGB15(src[0],src[1],src[2],*dst)

#include "../csp_packed_packed.h"

/* rgb_48_to_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_16_c
#define CONVERT \
  PACK_16_TO_RGB16(src[0],src[1],src[2],*dst)

#include "../csp_packed_packed.h"

#endif // !HQ

/* rgb_48_to_24_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_24_c

#define CONVERT \
  COPY_48_TO_24
#ifdef HQ
#define INIT int32_t round_tmp;
#endif

#include "../csp_packed_packed.h"

/* rgb_48_to_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_32_c
#define CONVERT \
  COPY_48_TO_24

#ifdef HQ
#define INIT int32_t round_tmp;
#endif

#include "../csp_packed_packed.h"

#ifndef HQ

/* rgb_48_to_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_float_c
#define CONVERT \
  COPY_48_TO_FLOAT

#include "../csp_packed_packed.h"

#endif // !HQ

/* rgb_float_to_15_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_15_c

#define INIT uint8_t r_tmp, g_tmp, b_tmp;

#define CONVERT                                 \
  RGB_FLOAT_TO_8(src[0], r_tmp);               \
  RGB_FLOAT_TO_8(src[1], g_tmp);               \
  RGB_FLOAT_TO_8(src[2], b_tmp);               \
  PACK_8_TO_RGB15(r_tmp, g_tmp, b_tmp, *dst)

#include "../csp_packed_packed.h"

/* rgb_float_to_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_16_c

#define INIT uint8_t r_tmp, g_tmp, b_tmp;

#define CONVERT                                 \
  RGB_FLOAT_TO_8(src[0], r_tmp);               \
  RGB_FLOAT_TO_8(src[1], g_tmp);               \
  RGB_FLOAT_TO_8(src[2], b_tmp);               \
  PACK_8_TO_RGB16(r_tmp, g_tmp, b_tmp, *dst)

#include "../csp_packed_packed.h"

/* rgb_float_to_24_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_24_c
#define CONVERT \
  COPY_FLOAT_TO_24

#include "../csp_packed_packed.h"

/* rgb_float_to_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_32_c
#define CONVERT \
  COPY_FLOAT_TO_24

#include "../csp_packed_packed.h"

/* rgb_float_to_48_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_48_c
#define CONVERT \
  COPY_FLOAT_TO_48

#include "../csp_packed_packed.h"

#ifndef HQ // !HQ

/* rgb_15_to_16_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_16_swap_c
#define CONVERT \
  *dst = ((*src & RGB15_UPPER_MASK) >> 10) | \
    ((*src & RGB15_MIDDLE_MASK) << 1) | \
    ((*src & RGB15_LOWER_MASK) << 11);
#include "../csp_packed_packed.h"

/* rgb_15_to_24_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_24_swap_c
#define CONVERT \
  dst[2] = RGB15_TO_R_8(*src); \
  dst[1] = RGB15_TO_G_8(*src); \
  dst[0] = RGB15_TO_B_8(*src);
#include "../csp_packed_packed.h"


/* rgb_15_to_32_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_32_swap_c
#define CONVERT \
  dst[2] = RGB15_TO_R_8(*src); \
  dst[1] = RGB15_TO_G_8(*src); \
  dst[0] = RGB15_TO_B_8(*src);
#include "../csp_packed_packed.h"

/* rgb_15_to_48_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_48_swap_c
#define CONVERT \
  dst[2] = RGB15_TO_R_16(*src);\
  dst[1] = RGB15_TO_G_16(*src);\
  dst[0] = RGB15_TO_B_16(*src);

#include "../csp_packed_packed.h"

/* rgb_15_to_float_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_float_swap_c
#define CONVERT \
  dst[2] = RGB15_TO_R_FLOAT(*src);\
  dst[1] = RGB15_TO_G_FLOAT(*src);\
  dst[0] = RGB15_TO_B_FLOAT(*src);

#include "../csp_packed_packed.h"

/* rgb_16_to_15_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_15_swap_c
#define CONVERT \
  *dst = ((*src & RGB16_UPPER_MASK) >> 11) | \
    (((*src & RGB16_MIDDLE_MASK) >> 1) & RGB15_MIDDLE_MASK) |\
    ((*src & RGB16_LOWER_MASK) << 10);
#include "../csp_packed_packed.h"

/* rgb_16_to_24_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_24_swap_c
#define CONVERT \
  dst[2]=RGB16_TO_R_8(*src);\
  dst[1]=RGB16_TO_G_8(*src);\
  dst[0]=RGB16_TO_B_8(*src);
#include "../csp_packed_packed.h"

/* rgb_16_to_32_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_32_swap_c
#define CONVERT \
  dst[2]=RGB16_TO_R_8(*src);\
  dst[1]=RGB16_TO_G_8(*src);\
  dst[0]=RGB16_TO_B_8(*src);
#include "../csp_packed_packed.h"

/* rgb_16_to_48_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_48_swap_c
#define CONVERT \
  dst[2]=RGB16_TO_R_16(*src);                   \
  dst[1]=RGB16_TO_G_16(*src);                   \
  dst[0]=RGB16_TO_B_16(*src);

#include "../csp_packed_packed.h"

/* rgb_16_to_float_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_float_swap_c
#define CONVERT \
  dst[2]=RGB16_TO_R_FLOAT(*src);                   \
  dst[1]=RGB16_TO_G_FLOAT(*src);                   \
  dst[0]=RGB16_TO_B_FLOAT(*src);

#include "../csp_packed_packed.h"


/* rgb_24_to_15_swap_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_15_swap_c
#define CONVERT \
  PACK_8_TO_BGR15(src[0],src[1],src[2],*dst)
#include "../csp_packed_packed.h"

/* rgb_24_to_16_swap_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_16_swap_c
#define CONVERT \
  PACK_8_TO_BGR16(src[0],src[1],src[2],*dst)
#include "../csp_packed_packed.h"

/* rgb_24_to_32_swap_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_32_swap_c
#define CONVERT \
  SWAP_24
#include "../csp_packed_packed.h"

/* rgb_24_to_48_swap_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_48_swap_c
#define CONVERT \
  SWAP_24_TO_48
#include "../csp_packed_packed.h"

/* rgb_24_to_float_swap_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_float_swap_c
#define CONVERT \
  SWAP_24_TO_FLOAT
#include "../csp_packed_packed.h"


/* rgb_32_to_15_swap_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_15_swap_c
#define CONVERT \
  PACK_8_TO_BGR15(src[0],src[1],src[2],*dst)

#include "../csp_packed_packed.h"

/* rgb_32_to_16_swap_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_16_swap_c
#define CONVERT \
  PACK_8_TO_BGR16(src[0],src[1],src[2],*dst)

#include "../csp_packed_packed.h"

/* rgb_32_to_24_swap_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_24_swap_c
#define CONVERT \
  SWAP_24

#include "../csp_packed_packed.h"

/* rgb_32_to_48_swap_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_48_swap_c
#define CONVERT \
  SWAP_24_TO_48

#include "../csp_packed_packed.h"

/* rgb_32_to_float_swap_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_float_swap_c
#define CONVERT \
  SWAP_24_TO_FLOAT

#include "../csp_packed_packed.h"


/* rgb_48_to_15_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_15_swap_c
#define CONVERT \
  PACK_16_TO_BGR15(src[0],src[1],src[2],*dst)

#include "../csp_packed_packed.h"

/* rgb_48_to_16_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_16_swap_c
#define CONVERT \
  PACK_16_TO_BGR16(src[0],src[1],src[2],*dst)

#include "../csp_packed_packed.h"

#endif // !HQ

/* rgb_48_to_24_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_24_swap_c
#define CONVERT \
  SWAP_48_TO_24

#ifdef HQ
#define INIT int32_t round_tmp;
#endif

#include "../csp_packed_packed.h"

/* rgb_48_to_32_swap_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_32_swap_c
#define CONVERT \
  SWAP_48_TO_24
#ifdef HQ
#define INIT int32_t round_tmp;
#endif

#include "../csp_packed_packed.h"


/* rgb_float_to_15_swap_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_15_swap_c

#define INIT uint8_t r_tmp, g_tmp, b_tmp;

#define CONVERT                                 \
  RGB_FLOAT_TO_8(src[0], r_tmp);               \
  RGB_FLOAT_TO_8(src[1], g_tmp);               \
  RGB_FLOAT_TO_8(src[2], b_tmp);               \
  PACK_8_TO_BGR15(r_tmp, g_tmp, b_tmp, *dst)

#include "../csp_packed_packed.h"

/* rgb_float_to_16_swap_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_16_swap_c

#define INIT uint8_t r_tmp, g_tmp, b_tmp;

#define CONVERT                                 \
  RGB_FLOAT_TO_8(src[0], r_tmp);               \
  RGB_FLOAT_TO_8(src[1], g_tmp);               \
  RGB_FLOAT_TO_8(src[2], b_tmp);               \
  PACK_8_TO_BGR16(r_tmp, g_tmp, b_tmp, *dst)

#include "../csp_packed_packed.h"

/* rgb_float_to_24_swap_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_24_swap_c
#define CONVERT \
  SWAP_FLOAT_TO_24

#include "../csp_packed_packed.h"

/* rgb_float_to_32_swap_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_32_swap_c
#define CONVERT \
 SWAP_FLOAT_TO_24

#include "../csp_packed_packed.h"

#ifndef HQ

/* Conversion from RGBA 32 to RGB */

/* rgba_32_to_rgb_15_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_rgb_15_c
#define CONVERT \
  RGBA_32_TO_RGB_24(src[0], src[1], src[2], src[3], r_tmp, g_tmp, b_tmp)\
  PACK_8_TO_RGB15(r_tmp, g_tmp, b_tmp, *dst)

#define INIT \
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_32

#include "../csp_packed_packed.h"

/* rgba_32_to_bgr_15_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_bgr_15_c
#define CONVERT \
  RGBA_32_TO_RGB_24(src[0], src[1], src[2], src[3], r_tmp, g_tmp, b_tmp) \
  PACK_8_TO_BGR15(r_tmp, g_tmp, b_tmp, *dst)

#define INIT \
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_32

#include "../csp_packed_packed.h"

/* rgba_32_to_rgb_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_rgb_16_c
#define CONVERT \
  RGBA_32_TO_RGB_24(src[0], src[1], src[2], src[3], r_tmp, g_tmp, b_tmp) \
  PACK_8_TO_RGB16(r_tmp, g_tmp, b_tmp, *dst)

#define INIT \
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_32

#include "../csp_packed_packed.h"

/* rgba_32_to_bgr_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_bgr_16_c
#define CONVERT \
  RGBA_32_TO_RGB_24(src[0], src[1], src[2], src[3], r_tmp, g_tmp, b_tmp) \
  PACK_8_TO_BGR16(r_tmp, g_tmp, b_tmp, *dst)

#define INIT \
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_32

#include "../csp_packed_packed.h"

/* rgba_32_to_rgb_24_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_rgb_24_c
#define CONVERT \
  RGBA_32_TO_RGB_24(src[0], src[1], src[2], src[3], dst[0], dst[1], dst[2])

#define INIT \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

/* rgba_32_to_bgr_24_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_bgr_24_c
#define CONVERT \
  RGBA_32_TO_RGB_24(src[0], src[1], src[2], src[3], dst[2], dst[1], dst[0])

#define INIT \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

/* rgba_32_to_rgb_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_rgb_32_c
#define CONVERT \
  RGBA_32_TO_RGB_24(src[0], src[1], src[2], src[3], dst[0], dst[1], dst[2])

#define INIT \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

/* rgba_32_to_bgr_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_bgr_32_c
#define CONVERT \
  RGBA_32_TO_RGB_24(src[0], src[1], src[2], src[3], dst[2], dst[1], dst[0])

#define INIT \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

/* rgba_32_to_rgb_48_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_rgb_48_c
#define CONVERT \
  RGBA_32_TO_RGB_48(src[0], src[1], src[2], src[3], dst[0], dst[1], dst[2])

#define INIT \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

/* rgba_32_to_rgb_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_rgb_float_c

#define CONVERT                                                         \
  r_tmp = RGB_8_TO_FLOAT(src[0]);                                       \
  g_tmp = RGB_8_TO_FLOAT(src[1]);                                       \
  b_tmp = RGB_8_TO_FLOAT(src[2]);                                       \
  a_tmp = RGB_8_TO_FLOAT(src[3]);                                       \
  RGBA_FLOAT_TO_RGB_FLOAT(r_tmp, g_tmp, b_tmp, a_tmp, dst[0], dst[1], dst[2])

#define INIT                                    \
  float r_tmp, g_tmp, b_tmp, a_tmp;             \
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

/* rgba_32_to_rgba_64_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_rgba_64_c
#define CONVERT \
  dst[0] = RGB_8_TO_16(src[0]); \
  dst[1] = RGB_8_TO_16(src[1]); \
  dst[2] = RGB_8_TO_16(src[2]); \
  dst[3] = RGB_8_TO_16(src[3]); \
  
#include "../csp_packed_packed.h"

/* rgba_32_to_rgba_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_rgba_float_c
#define CONVERT \
  dst[0] = RGB_8_TO_FLOAT(src[0]); \
  dst[1] = RGB_8_TO_FLOAT(src[1]); \
  dst[2] = RGB_8_TO_FLOAT(src[2]); \
  dst[3] = RGB_8_TO_FLOAT(src[3]);

#include "../csp_packed_packed.h"

/* Conversion from RGBA 64 to RGB */

/* rgba_64_to_rgb_15_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgb_15_c
#define CONVERT \
  RGBA_64_TO_RGB_24(src[0], src[1], src[2], src[3], r_tmp, g_tmp, b_tmp)\
  PACK_8_TO_RGB15(r_tmp, g_tmp, b_tmp, *dst)

#ifdef HQ
#define INIT \
  int32_t round_tmp;\
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_64
#else
#define INIT \
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_64
#endif
  
#include "../csp_packed_packed.h"

/* rgba_64_to_bgr_15_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_bgr_15_c
#define CONVERT \
  RGBA_64_TO_RGB_24(src[0], src[1], src[2], src[3], r_tmp, g_tmp, b_tmp) \
  PACK_8_TO_BGR15(r_tmp, g_tmp, b_tmp, *dst)

#ifdef HQ
#define INIT \
  int32_t round_tmp;\
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_64
#else
#define INIT \
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_64
#endif

#include "../csp_packed_packed.h"

/* rgba_64_to_rgb_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgb_16_c
#define CONVERT \
  RGBA_64_TO_RGB_24(src[0], src[1], src[2], src[3], r_tmp, g_tmp, b_tmp) \
  PACK_8_TO_RGB16(r_tmp, g_tmp, b_tmp, *dst)

#ifdef HQ
#define INIT \
  int32_t round_tmp;\
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_64
#else
#define INIT \
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_64
#endif


#include "../csp_packed_packed.h"

/* rgba_64_to_bgr_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_bgr_16_c
#define CONVERT \
  RGBA_64_TO_RGB_24(src[0], src[1], src[2], src[3], r_tmp, g_tmp, b_tmp) \
  PACK_8_TO_BGR16(r_tmp, g_tmp, b_tmp, *dst)

#ifdef HQ
#define INIT \
  int32_t round_tmp;\
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_64
#else
#define INIT \
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_64
#endif


#include "../csp_packed_packed.h"

#endif // !HQ

/* rgba_64_to_rgb_24_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgb_24_c
#define CONVERT \
  RGBA_64_TO_RGB_24(src[0], src[1], src[2], src[3], dst[0], dst[1], dst[2])

#ifdef HQ
#define INIT \
  int32_t round_tmp;\
  INIT_RGBA_64
#else
#define INIT \
  INIT_RGBA_64
#endif  
  
#include "../csp_packed_packed.h"

/* rgba_64_to_bgr_24_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_bgr_24_c
#define CONVERT \
  RGBA_64_TO_RGB_24(src[0], src[1], src[2], src[3], dst[2], dst[1], dst[0])

#ifdef HQ
#define INIT \
  int32_t round_tmp;\
  INIT_RGBA_64
#else
#define INIT \
  INIT_RGBA_64
#endif  

#include "../csp_packed_packed.h"

/* rgba_64_to_rgb_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgb_32_c
#define CONVERT \
  RGBA_64_TO_RGB_24(src[0], src[1], src[2], src[3], dst[0], dst[1], dst[2])

#ifdef HQ
#define INIT \
  int32_t round_tmp;\
  INIT_RGBA_64
#else
#define INIT \
  INIT_RGBA_64
#endif  


#include "../csp_packed_packed.h"

/* rgba_64_to_bgr_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_bgr_32_c
#define CONVERT \
  RGBA_64_TO_RGB_24(src[0], src[1], src[2], src[3], dst[2], dst[1], dst[0])

#ifdef HQ
#define INIT \
  int32_t round_tmp;\
  INIT_RGBA_64
#else
#define INIT \
  INIT_RGBA_64
#endif  


#include "../csp_packed_packed.h"

#ifndef HQ

/* rgba_64_to_rgb_48_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgb_48_c
#define CONVERT \
  RGBA_64_TO_RGB_48(src[0], src[1], src[2], src[3], dst[0], dst[1], dst[2])

#define INIT \
  INIT_RGBA_64

#include "../csp_packed_packed.h"

/* rgba_64_to_rgb_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgb_float_c
#define CONVERT                                                         \
  RGBA_64_TO_RGB_FLOAT(src[0], src[1], src[2], src[3], dst[0], dst[1], dst[2]);

#define INIT                    \
  INIT_RGBA_64

#include "../csp_packed_packed.h"

#endif // !HQ

/* rgba_64_to_rgba_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgba_32_c

#define CONVERT \
  RGB_16_TO_8(src[0], dst[0]); \
  RGB_16_TO_8(src[1], dst[1]); \
  RGB_16_TO_8(src[2], dst[2]); \
  RGB_16_TO_8(src[3], dst[3]); \

#ifdef HQ

#define INIT \
  int32_t round_tmp;
#endif  

  
#include "../csp_packed_packed.h"

#ifndef HQ

/* rgba_64_to_rgba_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgba_float_c
#define CONVERT \
  dst[0] = RGB_16_TO_FLOAT(src[0]); \
  dst[1] = RGB_16_TO_FLOAT(src[1]); \
  dst[2] = RGB_16_TO_FLOAT(src[2]); \
  dst[3] = RGB_16_TO_FLOAT(src[3]);

#include "../csp_packed_packed.h"

/* Conversion from RGBA 64 to RGB (ignoring alpha) */

/* rgba_64_to_rgb_15_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgb_15_ia_c
#define CONVERT \
  PACK_16_TO_RGB15(src[0], src[1], src[2], *dst)

#ifdef HQ
#define INIT int32_t round_tmp;
#endif
  
#include "../csp_packed_packed.h"

/* rgba_64_to_bgr_15_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_bgr_15_ia_c
#define CONVERT \
  PACK_16_TO_BGR15(src[0], src[1], src[2], *dst)
#ifdef HQ
#define INIT int32_t round_tmp;
#endif


#include "../csp_packed_packed.h"

/* rgba_64_to_rgb_16_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgb_16_ia_c
#define CONVERT \
  PACK_16_TO_RGB16(src[0], src[1], src[2], *dst)

#ifdef HQ
#define INIT int32_t round_tmp;
#endif

#include "../csp_packed_packed.h"

/* rgba_64_to_bgr_16_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_bgr_16_ia_c
#define CONVERT \
  PACK_16_TO_BGR16(src[0], src[1], src[2], *dst)
#ifdef HQ
#define INIT int32_t round_tmp;
#endif
  
#include "../csp_packed_packed.h"

/* rgba_64_to_rgb_24_ia_c */

#endif // !HQ

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgb_24_ia_c
#define CONVERT \
  COPY_48_TO_24

#ifdef HQ
#define INIT int32_t round_tmp;
#endif

#include "../csp_packed_packed.h"

/* rgba_64_to_bgr_24_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_bgr_24_ia_c
#define CONVERT \
  SWAP_48_TO_24
#ifdef HQ
#define INIT int32_t round_tmp;
#endif

#include "../csp_packed_packed.h"

/* rgba_64_to_rgb_32_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgb_32_ia_c
#define CONVERT \
  COPY_48_TO_24
#ifdef HQ
#define INIT int32_t round_tmp;
#endif

#include "../csp_packed_packed.h"

/* rgba_64_to_bgr_32_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_bgr_32_ia_c
#define CONVERT \
  SWAP_48_TO_24
#ifdef HQ
#define INIT int32_t round_tmp;
#endif

#include "../csp_packed_packed.h"

#ifndef HQ

/* rgba_64_to_rgb_48_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgb_48_ia_c
#define CONVERT \
  COPY_48_TO_48

#include "../csp_packed_packed.h"

/* rgba_64_to_rgb_float_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_rgb_float_ia_c
#define CONVERT                                                         \
  COPY_48_TO_FLOAT

#include "../csp_packed_packed.h"

/* Conversion from RGBA float to RGB */

#endif // !HQ

/* rgba_float_to_rgb_15_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgb_15_c
#define CONVERT \
  RGBA_FLOAT_TO_RGB_24(src[0], src[1], src[2], src[3], r_tmp, g_tmp, b_tmp)\
  PACK_8_TO_RGB15(r_tmp, g_tmp, b_tmp, *dst)

#define INIT \
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

/* rgba_float_to_bgr_15_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_bgr_15_c
#define CONVERT \
  RGBA_FLOAT_TO_RGB_24(src[0], src[1], src[2], src[3], r_tmp, g_tmp, b_tmp) \
  PACK_8_TO_BGR15(r_tmp, g_tmp, b_tmp, *dst)

#define INIT \
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

/* rgba_float_to_rgb_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgb_16_c
#define CONVERT \
  RGBA_FLOAT_TO_RGB_24(src[0], src[1], src[2], src[3], r_tmp, g_tmp, b_tmp) \
  PACK_8_TO_RGB16(r_tmp, g_tmp, b_tmp, *dst)

#define INIT \
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

/* rgba_float_to_bgr_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_bgr_16_c
#define CONVERT \
  RGBA_FLOAT_TO_RGB_24(src[0], src[1], src[2], src[3], r_tmp, g_tmp, b_tmp) \
  PACK_8_TO_BGR16(r_tmp, g_tmp, b_tmp, *dst)

#define INIT \
  uint8_t r_tmp;\
  uint8_t g_tmp;\
  uint8_t b_tmp;\
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

/* rgba_float_to_rgb_24_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgb_24_c
#define CONVERT \
  RGBA_FLOAT_TO_RGB_24(src[0], src[1], src[2], src[3], dst[0], dst[1], dst[2])

#define INIT \
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

/* rgba_float_to_bgr_24_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_bgr_24_c
#define CONVERT \
  RGBA_FLOAT_TO_RGB_24(src[0], src[1], src[2], src[3], dst[2], dst[1], dst[0])

#define INIT \
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

/* rgba_float_to_rgb_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgb_32_c
#define CONVERT \
  RGBA_FLOAT_TO_RGB_24(src[0], src[1], src[2], src[3], dst[0], dst[1], dst[2])

#define INIT \
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

/* rgba_float_to_bgr_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_bgr_32_c
#define CONVERT \
  RGBA_FLOAT_TO_RGB_24(src[0], src[1], src[2], src[3], dst[2], dst[1], dst[0])

#define INIT \
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

/* rgba_float_to_rgb_48_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgb_48_c
#define CONVERT \
  RGBA_FLOAT_TO_RGB_48(src[0], src[1], src[2], src[3], dst[0], dst[1], dst[2])

#define INIT \
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

#ifndef HQ

/* rgba_float_to_rgb_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgb_float_c
#define CONVERT \
  RGBA_FLOAT_TO_RGB_FLOAT(src[0], src[1], src[2], src[3], dst[0], dst[1], dst[2])

#define INIT \
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

/* rgba_float_to_rgba_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgba_32_c
#define CONVERT \
  RGB_FLOAT_TO_8(src[0], dst[0]);                    \
  RGB_FLOAT_TO_8(src[1], dst[1]);                    \
  RGB_FLOAT_TO_8(src[2], dst[2]);                    \
  RGB_FLOAT_TO_8(src[3], dst[3]);

#include "../csp_packed_packed.h"

/* rgba_float_to_rgba_64_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgba_64_c
#define CONVERT \
  RGB_FLOAT_TO_16(src[0], dst[0]);                    \
  RGB_FLOAT_TO_16(src[1], dst[1]);                    \
  RGB_FLOAT_TO_16(src[2], dst[2]);                    \
  RGB_FLOAT_TO_16(src[3], dst[3]);

#include "../csp_packed_packed.h"

/* Conversion from RGBA float to RGB (ignoring alpha) */

/* rgba_float_to_rgb_15_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgb_15_ia_c

#define INIT uint8_t r_tmp, g_tmp, b_tmp;

#define CONVERT                                 \
  RGB_FLOAT_TO_8(src[0], r_tmp);               \
  RGB_FLOAT_TO_8(src[1], g_tmp);               \
  RGB_FLOAT_TO_8(src[2], b_tmp);               \
  PACK_8_TO_RGB15(r_tmp, g_tmp, b_tmp, *dst)

#include "../csp_packed_packed.h"

/* rgba_float_to_bgr_15_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_bgr_15_ia_c

#define INIT uint8_t r_tmp, g_tmp, b_tmp;

#define CONVERT                                 \
  RGB_FLOAT_TO_8(src[0], r_tmp);               \
  RGB_FLOAT_TO_8(src[1], g_tmp);               \
  RGB_FLOAT_TO_8(src[2], b_tmp);               \
  PACK_8_TO_BGR15(r_tmp, g_tmp, b_tmp, *dst)

#include "../csp_packed_packed.h"

/* rgba_float_to_rgb_16_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgb_16_ia_c
#define INIT uint8_t r_tmp, g_tmp, b_tmp;
#define CONVERT                                 \
  RGB_FLOAT_TO_8(src[0], r_tmp);               \
  RGB_FLOAT_TO_8(src[1], g_tmp);               \
  RGB_FLOAT_TO_8(src[2], b_tmp);               \
  PACK_8_TO_RGB16(r_tmp, g_tmp, b_tmp, *dst)


#include "../csp_packed_packed.h"

/* rgba_float_to_bgr_16_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_bgr_16_ia_c
#define INIT uint8_t r_tmp, g_tmp, b_tmp;
#define CONVERT                                 \
  RGB_FLOAT_TO_8(src[0], r_tmp);               \
  RGB_FLOAT_TO_8(src[1], g_tmp);               \
  RGB_FLOAT_TO_8(src[2], b_tmp);               \
  PACK_8_TO_BGR16(r_tmp, g_tmp, b_tmp, *dst)

#include "../csp_packed_packed.h"

#endif // !HQ

/* rgba_float_to_rgb_24_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgb_24_ia_c
#define CONVERT \
  COPY_FLOAT_TO_24

#include "../csp_packed_packed.h"

/* rgba_float_to_bgr_24_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_bgr_24_ia_c
#define CONVERT \
  SWAP_FLOAT_TO_24

#include "../csp_packed_packed.h"

/* rgba_float_to_rgb_32_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgb_32_ia_c
#define CONVERT \
  COPY_FLOAT_TO_24

#include "../csp_packed_packed.h"

/* rgba_float_to_bgr_32_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_bgr_32_ia_c
#define CONVERT \
  SWAP_FLOAT_TO_24

#include "../csp_packed_packed.h"

/* rgba_float_to_rgb_48_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgb_48_ia_c
#define CONVERT \
  COPY_FLOAT_TO_48

#include "../csp_packed_packed.h"

#ifndef HQ


/* rgba_float_to_rgb_float_ia_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_rgb_float_ia_c
#define CONVERT \
  COPY_FLOAT_TO_FLOAT

#include "../csp_packed_packed.h"

/* Conversion from RGB formats to RGBA 32 */

/* rgb_15_to_rgba_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_rgba_32_c
#define CONVERT \
  dst[0] = RGB15_TO_R_8(*src);\
  dst[1] = RGB15_TO_G_8(*src);\
  dst[2] = RGB15_TO_B_8(*src);\
  dst[3] = 0xff;

#include "../csp_packed_packed.h"

/* bgr_15_to_rgba_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME bgr_15_to_rgba_32_c
#define CONVERT \
  dst[0] = BGR15_TO_R_8(*src);\
  dst[1] = BGR15_TO_G_8(*src);\
  dst[2] = BGR15_TO_B_8(*src);\
  dst[3] = 0xff;

#include "../csp_packed_packed.h"

/* rgb_16_to_rgba_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_rgba_32_c
#define CONVERT \
  dst[0] = RGB16_TO_R_8(*src);\
  dst[1] = RGB16_TO_G_8(*src);\
  dst[2] = RGB16_TO_B_8(*src);\
  dst[3] = 0xff;

#include "../csp_packed_packed.h"

/* bgr_16_to_rgba_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME bgr_16_to_rgba_32_c
#define CONVERT \
  dst[0] = BGR16_TO_R_8(*src);\
  dst[1] = BGR16_TO_G_8(*src);\
  dst[2] = BGR16_TO_B_8(*src);\
  dst[3] = 0xff;

#include "../csp_packed_packed.h"

/* rgb_24_to_rgba_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_rgba_32_c
#define CONVERT \
  COPY_24 \
  dst[3] = 0xff;

#include "../csp_packed_packed.h"

/* bgr_24_to_rgba_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME bgr_24_to_rgba_32_c
#define CONVERT \
  SWAP_24 \
  dst[3] = 0xff;

#include "../csp_packed_packed.h"

/* rgb_32_to_rgba_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_rgba_32_c
#define CONVERT \
  COPY_24 \
  dst[3] = 0xff;

#include "../csp_packed_packed.h"

/* bgr_32_to_rgba_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME bgr_32_to_rgba_32_c
#define CONVERT \
  SWAP_24 \
  dst[3] = 0xff;

#include "../csp_packed_packed.h"

#endif // !HQ

/* rgb_48_to_rgba_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_rgba_32_c
#define CONVERT \
  COPY_48_TO_24 \
  dst[3] = 0xff;
#ifdef HQ
#define INIT int32_t round_tmp;
#endif

#include "../csp_packed_packed.h"

/* rgb_float_to_rgba_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_rgba_32_c
#define CONVERT \
  COPY_FLOAT_TO_24 \
  dst[3] = 0xff;

#include "../csp_packed_packed.h"

#ifndef HQ

/* Conversion from RGB formats to RGBA 64 */

/* rgb_15_to_rgba_64_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_rgba_64_c
#define CONVERT \
  dst[0] = RGB15_TO_R_16(*src);\
  dst[1] = RGB15_TO_G_16(*src);\
  dst[2] = RGB15_TO_B_16(*src);\
  dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* bgr_15_to_rgba_64_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME bgr_15_to_rgba_64_c
#define CONVERT \
  dst[0] = BGR15_TO_R_16(*src);\
  dst[1] = BGR15_TO_G_16(*src);\
  dst[2] = BGR15_TO_B_16(*src);\
  dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* rgb_16_to_rgba_64_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_rgba_64_c
#define CONVERT \
  dst[0] = RGB16_TO_R_16(*src);\
  dst[1] = RGB16_TO_G_16(*src);\
  dst[2] = RGB16_TO_B_16(*src);\
  dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* bgr_16_to_rgba_64_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME bgr_16_to_rgba_64_c
#define CONVERT \
  dst[0] = BGR16_TO_R_16(*src);\
  dst[1] = BGR16_TO_G_16(*src);\
  dst[2] = BGR16_TO_B_16(*src);\
  dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* rgb_24_to_rgba_64_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_rgba_64_c
#define CONVERT \
  COPY_24_TO_48 \
  dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* bgr_24_to_rgba_64_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME bgr_24_to_rgba_64_c
#define CONVERT \
  SWAP_24_TO_48 \
  dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* rgb_32_to_rgba_64_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_rgba_64_c
#define CONVERT \
  COPY_24_TO_48 \
  dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* bgr_32_to_rgba_64_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME bgr_32_to_rgba_64_c
#define CONVERT \
  SWAP_24_TO_48 \
  dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* rgb_48_to_rgba_64_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_rgba_64_c
#define CONVERT \
  COPY_48_TO_48 \
  dst[3] = 0xffff;

#include "../csp_packed_packed.h"

#endif // !HQ

/* rgb_float_to_rgba_64_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_rgba_64_c
#define CONVERT \
  COPY_FLOAT_TO_48 \
  dst[3] = 0xffff;

#include "../csp_packed_packed.h"

#ifndef HQ

/* Conversion from RGB formats to RGBA Float */

/* rgb_15_to_rgba_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_rgba_float_c
#define CONVERT \
  dst[0] = RGB15_TO_R_FLOAT(*src);\
  dst[1] = RGB15_TO_G_FLOAT(*src);\
  dst[2] = RGB15_TO_B_FLOAT(*src);\
  dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* bgr_15_to_rgba_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME bgr_15_to_rgba_float_c
#define CONVERT \
  dst[0] = BGR15_TO_R_FLOAT(*src);\
  dst[1] = BGR15_TO_G_FLOAT(*src);\
  dst[2] = BGR15_TO_B_FLOAT(*src);\
  dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* rgb_16_to_rgba_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_rgba_float_c
#define CONVERT \
  dst[0] = RGB16_TO_R_FLOAT(*src);\
  dst[1] = RGB16_TO_G_FLOAT(*src);\
  dst[2] = RGB16_TO_B_FLOAT(*src);\
  dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* bgr_16_to_rgba_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME bgr_16_to_rgba_float_c
#define CONVERT \
  dst[0] = BGR16_TO_R_FLOAT(*src);\
  dst[1] = BGR16_TO_G_FLOAT(*src);\
  dst[2] = BGR16_TO_B_FLOAT(*src);\
  dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* rgb_24_to_rgba_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_rgba_float_c
#define CONVERT \
  COPY_24_TO_FLOAT \
  dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* bgr_24_to_rgba_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME bgr_24_to_rgba_float_c
#define CONVERT \
  SWAP_24_TO_FLOAT \
  dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* rgb_32_to_rgba_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_rgba_float_c
#define CONVERT \
  COPY_24_TO_FLOAT \
  dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* bgr_32_to_rgba_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME bgr_32_to_rgba_float_c
#define CONVERT \
  SWAP_24_TO_FLOAT \
  dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* rgb_48_to_rgba_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_rgba_float_c
#define CONVERT \
  COPY_48_TO_FLOAT \
  dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* rgb_float_to_rgba_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_rgba_float_c
#define CONVERT \
  COPY_FLOAT_TO_FLOAT \
  dst[3] = 1.0;

#include "../csp_packed_packed.h"

#endif // !HQ


#ifdef HQ
void gavl_init_rgb_rgb_funcs_hq(gavl_pixelformat_function_table_t * tab,
                                const gavl_video_options_t * opt)
#else
void gavl_init_rgb_rgb_funcs_c(gavl_pixelformat_function_table_t * tab,
                               const gavl_video_options_t * opt)
#endif
  {
#ifndef HQ
  tab->swap_rgb_24 = swap_rgb_24_c;
  tab->swap_rgb_32 = swap_rgb_32_c;
  tab->swap_rgb_16 = swap_rgb_16_c;
  tab->swap_rgb_15 = swap_rgb_15_c;

  tab->rgb_15_to_16    = rgb_15_to_16_c;
  tab->rgb_15_to_24    = rgb_15_to_24_c;
  tab->rgb_15_to_32    = rgb_15_to_32_c;
  tab->rgb_15_to_48    = rgb_15_to_48_c;
  tab->rgb_15_to_float = rgb_15_to_float_c;

  tab->rgb_16_to_15    = rgb_16_to_15_c;
  tab->rgb_16_to_24    = rgb_16_to_24_c;
  tab->rgb_16_to_32    = rgb_16_to_32_c;
  tab->rgb_16_to_48    = rgb_16_to_48_c;
  tab->rgb_16_to_float = rgb_16_to_float_c;
  
  tab->rgb_24_to_15    = rgb_24_to_15_c;
  tab->rgb_24_to_16    = rgb_24_to_16_c;
  tab->rgb_24_to_32    = rgb_24_to_32_c;
  tab->rgb_24_to_48    = rgb_24_to_48_c;
  tab->rgb_24_to_float = rgb_24_to_float_c;
  
  tab->rgb_32_to_15    = rgb_32_to_15_c;
  tab->rgb_32_to_16    = rgb_32_to_16_c;
  tab->rgb_32_to_24    = rgb_32_to_24_c;
  tab->rgb_32_to_48    = rgb_32_to_48_c;
  tab->rgb_32_to_float = rgb_32_to_float_c;
  
  tab->rgb_48_to_15    = rgb_48_to_15_c;
  tab->rgb_48_to_16    = rgb_48_to_16_c;
#endif // !HQ
  tab->rgb_48_to_24    = rgb_48_to_24_c;
  tab->rgb_48_to_32    = rgb_48_to_32_c;
#ifndef HQ
  tab->rgb_48_to_float = rgb_48_to_float_c;

#endif // !HQ
  tab->rgb_float_to_15    = rgb_float_to_15_c;
  tab->rgb_float_to_16    = rgb_float_to_16_c;
  tab->rgb_float_to_24    = rgb_float_to_24_c;
  tab->rgb_float_to_32    = rgb_float_to_32_c;
  tab->rgb_float_to_48    = rgb_float_to_48_c;

#ifndef HQ
  
  tab->rgb_15_to_16_swap    = rgb_15_to_16_swap_c;
  tab->rgb_15_to_24_swap    = rgb_15_to_24_swap_c;
  tab->rgb_15_to_32_swap    = rgb_15_to_32_swap_c;
  tab->rgb_15_to_48_swap    = rgb_15_to_48_swap_c;
  tab->rgb_15_to_float_swap = rgb_15_to_float_swap_c;

  tab->rgb_16_to_15_swap    = rgb_16_to_15_swap_c;
  tab->rgb_16_to_24_swap    = rgb_16_to_24_swap_c;
  tab->rgb_16_to_32_swap    = rgb_16_to_32_swap_c;
  tab->rgb_16_to_48_swap    = rgb_16_to_48_swap_c;
  tab->rgb_16_to_float_swap = rgb_16_to_float_swap_c;
  
  tab->rgb_24_to_15_swap    = rgb_24_to_15_swap_c;
  tab->rgb_24_to_16_swap    = rgb_24_to_16_swap_c;
  tab->rgb_24_to_32_swap    = rgb_24_to_32_swap_c;
  tab->rgb_24_to_48_swap    = rgb_24_to_48_swap_c;
  tab->rgb_24_to_float_swap = rgb_24_to_float_swap_c;
  
  tab->rgb_32_to_15_swap    = rgb_32_to_15_swap_c;
  tab->rgb_32_to_16_swap    = rgb_32_to_16_swap_c;
  tab->rgb_32_to_24_swap    = rgb_32_to_24_swap_c;
  tab->rgb_32_to_48_swap    = rgb_32_to_48_swap_c;
  tab->rgb_32_to_float_swap = rgb_32_to_float_swap_c;

  tab->rgb_48_to_15_swap    = rgb_48_to_15_swap_c;
  tab->rgb_48_to_16_swap    = rgb_48_to_16_swap_c;
#endif // !HQ
  tab->rgb_48_to_24_swap    = rgb_48_to_24_swap_c;
  tab->rgb_48_to_32_swap    = rgb_48_to_32_swap_c;

  tab->rgb_float_to_15_swap    = rgb_float_to_15_swap_c;
  tab->rgb_float_to_16_swap    = rgb_float_to_16_swap_c;
  tab->rgb_float_to_24_swap    = rgb_float_to_24_swap_c;
  tab->rgb_float_to_32_swap    = rgb_float_to_32_swap_c;
  
  /* Conversion from RGBA to RGB formats */

  if(opt->alpha_mode == GAVL_ALPHA_BLEND_COLOR)
    {
#ifndef HQ
    tab->rgba_32_to_rgb_15    = rgba_32_to_rgb_15_c;
    tab->rgba_32_to_bgr_15    = rgba_32_to_bgr_15_c;
    tab->rgba_32_to_rgb_16    = rgba_32_to_rgb_16_c;
    tab->rgba_32_to_bgr_16    = rgba_32_to_bgr_16_c;
    tab->rgba_32_to_rgb_24    = rgba_32_to_rgb_24_c;
    tab->rgba_32_to_bgr_24    = rgba_32_to_bgr_24_c;
    tab->rgba_32_to_rgb_32    = rgba_32_to_rgb_32_c;
    tab->rgba_32_to_bgr_32    = rgba_32_to_bgr_32_c;
    tab->rgba_32_to_rgb_48    = rgba_32_to_rgb_48_c;
    tab->rgba_32_to_rgb_float = rgba_32_to_rgb_float_c;

    tab->rgba_64_to_rgb_15 = rgba_64_to_rgb_15_c;
    tab->rgba_64_to_bgr_15 = rgba_64_to_bgr_15_c;
    tab->rgba_64_to_rgb_16 = rgba_64_to_rgb_16_c;
    tab->rgba_64_to_bgr_16 = rgba_64_to_bgr_16_c;
#endif // !HQ
    tab->rgba_64_to_rgb_24 = rgba_64_to_rgb_24_c;
    tab->rgba_64_to_bgr_24 = rgba_64_to_bgr_24_c;
    tab->rgba_64_to_rgb_32 = rgba_64_to_rgb_32_c;
    tab->rgba_64_to_bgr_32 = rgba_64_to_bgr_32_c;
#ifndef HQ
    tab->rgba_64_to_rgb_48 = rgba_64_to_rgb_48_c;
    tab->rgba_64_to_rgb_float = rgba_64_to_rgb_float_c;
#endif // !HQ

    tab->rgba_float_to_rgb_15    = rgba_float_to_rgb_15_c;
    tab->rgba_float_to_bgr_15    = rgba_float_to_bgr_15_c;
    tab->rgba_float_to_rgb_16    = rgba_float_to_rgb_16_c;
    tab->rgba_float_to_bgr_16    = rgba_float_to_bgr_16_c;
    tab->rgba_float_to_rgb_24    = rgba_float_to_rgb_24_c;
    tab->rgba_float_to_bgr_24    = rgba_float_to_bgr_24_c;
    tab->rgba_float_to_rgb_32    = rgba_float_to_rgb_32_c;
    tab->rgba_float_to_bgr_32    = rgba_float_to_bgr_32_c;
    tab->rgba_float_to_rgb_48    = rgba_float_to_rgb_48_c;
#ifndef HQ
    tab->rgba_float_to_rgb_float = rgba_float_to_rgb_float_c;
#endif // !HQ
    }
  else if(opt->alpha_mode == GAVL_ALPHA_IGNORE)
    {
#ifndef HQ
    tab->rgba_32_to_rgb_15    = rgb_32_to_15_c;
    tab->rgba_32_to_bgr_15    = rgb_32_to_15_swap_c;
    tab->rgba_32_to_rgb_16    = rgb_32_to_16_c;
    tab->rgba_32_to_bgr_16    = rgb_32_to_16_swap_c;
    tab->rgba_32_to_rgb_24    = rgb_32_to_24_c;
    tab->rgba_32_to_bgr_24    = rgb_32_to_24_swap_c;
    tab->rgba_32_to_bgr_32    = swap_rgb_32_c;
    tab->rgba_32_to_rgb_48    = rgb_32_to_48_c;
    tab->rgba_32_to_rgb_float = rgb_32_to_float_c;
    
    tab->rgba_64_to_rgb_15    = rgba_64_to_rgb_15_ia_c;
    tab->rgba_64_to_bgr_15    = rgba_64_to_bgr_15_ia_c;
    tab->rgba_64_to_rgb_16    = rgba_64_to_rgb_16_ia_c;
    tab->rgba_64_to_bgr_16    = rgba_64_to_bgr_16_ia_c;
#endif // !HQ
    tab->rgba_64_to_rgb_24    = rgba_64_to_rgb_24_ia_c;
    tab->rgba_64_to_bgr_24    = rgba_64_to_bgr_24_ia_c;
    tab->rgba_64_to_rgb_32    = rgba_64_to_rgb_32_ia_c;
    tab->rgba_64_to_bgr_32    = rgba_64_to_bgr_32_ia_c;
#ifndef HQ
    tab->rgba_64_to_rgb_48    = rgba_64_to_rgb_48_ia_c;
    tab->rgba_64_to_rgb_float = rgba_64_to_rgb_float_ia_c;

    tab->rgba_float_to_rgb_15    = rgba_float_to_rgb_15_ia_c;
    tab->rgba_float_to_bgr_15    = rgba_float_to_bgr_15_ia_c;
    tab->rgba_float_to_rgb_16    = rgba_float_to_rgb_16_ia_c;
    tab->rgba_float_to_bgr_16    = rgba_float_to_bgr_16_ia_c;
#endif // !HQ
    tab->rgba_float_to_rgb_24    = rgba_float_to_rgb_24_ia_c;
    tab->rgba_float_to_bgr_24    = rgba_float_to_bgr_24_ia_c;
    tab->rgba_float_to_rgb_32    = rgba_float_to_rgb_32_ia_c;
    tab->rgba_float_to_bgr_32    = rgba_float_to_bgr_32_ia_c;
    tab->rgba_float_to_rgb_48    = rgba_float_to_rgb_48_ia_c;
#ifndef HQ
    tab->rgba_float_to_rgb_float = rgba_float_to_rgb_float_ia_c;
#endif // !HQ
    }
  
#ifndef HQ
  tab->rgba_32_to_rgba_64    = rgba_32_to_rgba_64_c;
  tab->rgba_32_to_rgba_float = rgba_32_to_rgba_float_c;
#endif // !HQ
  
  tab->rgba_64_to_rgba_32    = rgba_64_to_rgba_32_c;
#ifndef HQ
  tab->rgba_64_to_rgba_float = rgba_64_to_rgba_float_c;
#endif // !HQ
  
#ifndef HQ
  tab->rgba_float_to_rgba_32   = rgba_float_to_rgba_32_c;
  tab->rgba_float_to_rgba_64   = rgba_float_to_rgba_64_c;
  
  /* Conversion from RGB formats to RGBA 32 */

  tab->rgb_15_to_rgba_32 = rgb_15_to_rgba_32_c;
  tab->bgr_15_to_rgba_32 = bgr_15_to_rgba_32_c;
  tab->rgb_16_to_rgba_32 = rgb_16_to_rgba_32_c;
  tab->bgr_16_to_rgba_32 = bgr_16_to_rgba_32_c;
  tab->rgb_24_to_rgba_32 = rgb_24_to_rgba_32_c;
  tab->bgr_24_to_rgba_32 = bgr_24_to_rgba_32_c;
  tab->rgb_32_to_rgba_32 = rgb_32_to_rgba_32_c;
  tab->bgr_32_to_rgba_32 = bgr_32_to_rgba_32_c;
#endif // !HQ
  tab->rgb_48_to_rgba_32 = rgb_48_to_rgba_32_c;
  tab->rgb_float_to_rgba_32 = rgb_float_to_rgba_32_c;
#ifndef HQ

  tab->rgb_15_to_rgba_64 = rgb_15_to_rgba_64_c;
  tab->bgr_15_to_rgba_64 = bgr_15_to_rgba_64_c;
  tab->rgb_16_to_rgba_64 = rgb_16_to_rgba_64_c;
  tab->bgr_16_to_rgba_64 = bgr_16_to_rgba_64_c;
  tab->rgb_24_to_rgba_64 = rgb_24_to_rgba_64_c;
  tab->bgr_24_to_rgba_64 = bgr_24_to_rgba_64_c;
  tab->rgb_32_to_rgba_64 = rgb_32_to_rgba_64_c;
  tab->bgr_32_to_rgba_64 = bgr_32_to_rgba_64_c;
  tab->rgb_48_to_rgba_64 = rgb_48_to_rgba_64_c;
#endif // !HQ
  tab->rgb_float_to_rgba_64 = rgb_float_to_rgba_64_c;
#ifndef HQ
  tab->rgb_15_to_rgba_float = rgb_15_to_rgba_float_c;
  tab->bgr_15_to_rgba_float = bgr_15_to_rgba_float_c;
  tab->rgb_16_to_rgba_float = rgb_16_to_rgba_float_c;
  tab->bgr_16_to_rgba_float = bgr_16_to_rgba_float_c;
  tab->rgb_24_to_rgba_float = rgb_24_to_rgba_float_c;
  tab->bgr_24_to_rgba_float = bgr_24_to_rgba_float_c;
  tab->rgb_32_to_rgba_float = rgb_32_to_rgba_float_c;
  tab->bgr_32_to_rgba_float = bgr_32_to_rgba_float_c;
  tab->rgb_48_to_rgba_float = rgb_48_to_rgba_float_c;
  tab->rgb_float_to_rgba_float = rgb_float_to_rgba_float_c;
#endif // !HQ
  
  }

#undef COPY_24
#undef SWAP_24 
