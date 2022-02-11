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

/* gray_8_to_rgb_15_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_rgb_15_c
#define CONVERT PACK_8_TO_RGB15(src[0],src[0],src[0],dst[0])

#include "../csp_packed_packed.h"

/* gray_8_to_rgb_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_rgb_16_c
#define CONVERT PACK_8_TO_RGB16(src[0],src[0],src[0],dst[0])

#include "../csp_packed_packed.h"

/* gray_16_to_rgb_15_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_rgb_15_c
#define CONVERT PACK_16_TO_RGB15(src[0],src[0],src[0],dst[0])

#include "../csp_packed_packed.h"

/* gray_16_to_rgb_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_rgb_16_c
#define CONVERT PACK_16_TO_RGB16(src[0],src[0],src[0],dst[0])

#include "../csp_packed_packed.h"

/* gray_float_to_rgb_15_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_rgb_15_c
#define INIT int tmp;
#define CONVERT RGB_FLOAT_TO_8(src[0], tmp); \
   PACK_8_TO_RGB15(tmp,tmp,tmp,dst[0]);

#include "../csp_packed_packed.h"

/* gray_float_to_rgb_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_rgb_16_c
#define INIT int tmp;
#define CONVERT RGB_FLOAT_TO_8(src[0], tmp); \
   PACK_8_TO_RGB16(tmp,tmp,tmp,dst[0]);

#include "../csp_packed_packed.h"

/* gray_8_to_rgb_24_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_rgb_24_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0];

#include "../csp_packed_packed.h"

/* gray_8_to_rgb_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_rgb_32_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0];

#include "../csp_packed_packed.h"

/* gray_8_to_rgba_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_rgba_32_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0]; \
                 dst[3] = 0xFF;

#include "../csp_packed_packed.h"

/* gray_16_to_rgb_24_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_rgb_24_c
#define CONVERT  RGB_16_TO_8(src[0], dst[0]); \
                 RGB_16_TO_8(src[0], dst[1]); \
                 RGB_16_TO_8(src[0], dst[2]);

#include "../csp_packed_packed.h"

/* gray_16_to_rgb_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_rgb_32_c
#define CONVERT  RGB_16_TO_8(src[0], dst[0]); \
                 RGB_16_TO_8(src[0], dst[1]); \
                 RGB_16_TO_8(src[0], dst[2]);

#include "../csp_packed_packed.h"

/* gray_16_to_rgba_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_rgba_32_c
#define CONVERT  RGB_16_TO_8(src[0], dst[0]); \
                 RGB_16_TO_8(src[0], dst[1]); \
                 RGB_16_TO_8(src[0], dst[2]); \
                 dst[3] = 0xFF;

#include "../csp_packed_packed.h"

/* gray_float_to_rgb_24_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_rgb_24_c
#define CONVERT  RGB_FLOAT_TO_8(src[0], dst[0]); \
                 RGB_FLOAT_TO_8(src[0], dst[1]); \
                 RGB_FLOAT_TO_8(src[0], dst[2]);


#include "../csp_packed_packed.h"

/* gray_float_to_rgb_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_rgb_32_c
#define CONVERT  RGB_FLOAT_TO_8(src[0], dst[0]); \
                 RGB_FLOAT_TO_8(src[0], dst[1]); \
                 RGB_FLOAT_TO_8(src[0], dst[2]);

#include "../csp_packed_packed.h"

/* gray_float_to_rgba_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_rgba_32_c
#define CONVERT  RGB_FLOAT_TO_8(src[0], dst[0]); \
                 RGB_FLOAT_TO_8(src[0], dst[1]); \
                 RGB_FLOAT_TO_8(src[0], dst[2]); \
                 dst[3] = 0xff;

#include "../csp_packed_packed.h"

/* gray_8_to_rgb_48_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_rgb_48_c
#define CONVERT  dst[0] = RGB_8_TO_16(src[0]); \
                 dst[1] = RGB_8_TO_16(src[0]); \
                 dst[2] = RGB_8_TO_16(src[0]);

#include "../csp_packed_packed.h"

/* gray_8_to_rgba_64_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_rgba_64_c
#define CONVERT  dst[0] = RGB_8_TO_16(src[0]); \
                 dst[1] = RGB_8_TO_16(src[0]); \
                 dst[2] = RGB_8_TO_16(src[0]); \
                 dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* gray_16_to_rgb_48_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_rgb_48_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0];

#include "../csp_packed_packed.h"

/* gray_16_to_rgb_64_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_rgba_64_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0]; \
                 dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* gray_float_to_rgb_48_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_rgb_48_c
#define CONVERT  RGB_FLOAT_TO_16(src[0], dst[0]); \
                 RGB_FLOAT_TO_16(src[0], dst[1]); \
                 RGB_FLOAT_TO_16(src[0], dst[2]);

#include "../csp_packed_packed.h"

/* gray_float_to_rgba_64_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_rgba_64_c
#define CONVERT  RGB_FLOAT_TO_16(src[0], dst[0]); \
                 RGB_FLOAT_TO_16(src[0], dst[1]); \
                 RGB_FLOAT_TO_16(src[0], dst[2]); \
                 dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* */

/* gray_8_to_rgb_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_rgb_float_c
#define CONVERT  dst[0] = RGB_8_TO_FLOAT(src[0]); \
                 dst[1] = RGB_8_TO_FLOAT(src[0]); \
                 dst[2] = RGB_8_TO_FLOAT(src[0]);

#include "../csp_packed_packed.h"

/* gray_8_to_rgba_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_rgba_float_c
#define CONVERT  dst[0] = RGB_8_TO_FLOAT(src[0]); \
                 dst[1] = RGB_8_TO_FLOAT(src[0]); \
                 dst[2] = RGB_8_TO_FLOAT(src[0]); \
                 dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* gray_16_to_rgb_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_rgb_float_c
#define CONVERT  dst[0] = RGB_16_TO_FLOAT(src[0]); \
                 dst[1] = RGB_16_TO_FLOAT(src[0]); \
                 dst[2] = RGB_16_TO_FLOAT(src[0]);

#include "../csp_packed_packed.h"

/* gray_16_to_rgba_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_rgba_float_c
#define CONVERT  dst[0] = RGB_16_TO_FLOAT(src[0]); \
                 dst[1] = RGB_16_TO_FLOAT(src[0]); \
                 dst[2] = RGB_16_TO_FLOAT(src[0]); \
                 dst[3] = 0xffff;

#include "../csp_packed_packed.h"

/* gray_float_to_rgb_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_rgb_float_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0];

#include "../csp_packed_packed.h"

/* gray_float_to_rgba_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_rgba_float_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0]; \
                 dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* Graya -> RGBA */

/* graya_16_to_rgba_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgba_32_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0]; \
                 dst[3] = src[1];

#include "../csp_packed_packed.h"

/* graya_16_to_rgba_64_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgba_64_c
#define CONVERT  dst[0] = RGB_8_TO_16(src[0]); \
                 dst[1] = RGB_8_TO_16(src[0]); \
                 dst[2] = RGB_8_TO_16(src[0]); \
                 dst[3] = RGB_8_TO_16(src[1]);

#include "../csp_packed_packed.h"

/* graya_16_to_rgba_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgba_float_c
#define CONVERT  dst[0] = RGB_8_TO_FLOAT(src[0]); \
                 dst[1] = RGB_8_TO_FLOAT(src[0]); \
                 dst[2] = RGB_8_TO_FLOAT(src[0]); \
                 dst[3] = RGB_8_TO_FLOAT(src[1]);

#include "../csp_packed_packed.h"

/* graya_32_to_rgba_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgba_32_c
#define CONVERT   RGB_16_TO_8(src[0], dst[0]); \
                  RGB_16_TO_8(src[0], dst[1]); \
                  RGB_16_TO_8(src[0], dst[2]); \
                  RGB_16_TO_8(src[1], dst[3]);

#include "../csp_packed_packed.h"

/* graya_32_to_rgba_64_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgba_64_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0]; \
                 dst[3] = src[1];

#include "../csp_packed_packed.h"

/* graya_32_to_rgba_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgba_float_c
#define CONVERT  dst[0] = RGB_16_TO_FLOAT(src[0]); \
                 dst[1] = RGB_16_TO_FLOAT(src[0]); \
                 dst[2] = RGB_16_TO_FLOAT(src[0]); \
                 dst[3] = RGB_16_TO_FLOAT(src[1]);

#include "../csp_packed_packed.h"


/* graya_float_to_rgba_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgba_32_c
#define CONVERT   RGB_FLOAT_TO_8(src[0], dst[0]); \
                  RGB_FLOAT_TO_8(src[0], dst[1]); \
                  RGB_FLOAT_TO_8(src[0], dst[2]); \
                  RGB_FLOAT_TO_8(src[1], dst[3]);

#include "../csp_packed_packed.h"

/* graya_float_to_rgba_64_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgba_64_c
#define CONVERT  RGB_FLOAT_TO_16(src[0], dst[0]); \
                 RGB_FLOAT_TO_16(src[0], dst[1]); \
                 RGB_FLOAT_TO_16(src[0], dst[2]); \
                 RGB_FLOAT_TO_16(src[1], dst[3]);

#include "../csp_packed_packed.h"

/* graya_float_to_rgba_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgba_float_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0]; \
                 dst[3] = src[1];

#include "../csp_packed_packed.h"

/* graya -> RGB ignoring alpha */

/* graya_16_to_rgb_15_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgb_15_ia_c
#define CONVERT PACK_8_TO_RGB15(src[0],src[0],src[0],dst[0])

#include "../csp_packed_packed.h"

/* graya_16_to_rgb_16_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgb_16_ia_c
#define CONVERT PACK_8_TO_RGB16(src[0],src[0],src[0],dst[0])

#include "../csp_packed_packed.h"

/* graya_32_to_rgb_15_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgb_15_ia_c
#define CONVERT PACK_16_TO_RGB15(src[0],src[0],src[0],dst[0])

#include "../csp_packed_packed.h"

/* graya_32_to_rgb_16_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgb_16_ia_c
#define CONVERT PACK_16_TO_RGB16(src[0],src[0],src[0],dst[0])

#include "../csp_packed_packed.h"

/* graya_float_to_rgb_15_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgb_15_ia_c
#define INIT int tmp;
#define CONVERT RGB_FLOAT_TO_8(src[0], tmp); \
   PACK_8_TO_RGB15(tmp,tmp,tmp,dst[0]);

#include "../csp_packed_packed.h"

/* graya_float_to_rgb_16_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgb_16_ia_c
#define INIT int tmp;
#define CONVERT RGB_FLOAT_TO_8(src[0], tmp); \
   PACK_8_TO_RGB16(tmp,tmp,tmp,dst[0]);

#include "../csp_packed_packed.h"

/* graya_16_to_rgb_24_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgb_24_ia_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0];

#include "../csp_packed_packed.h"

/* graya_16_to_rgb_32_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgb_32_ia_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0];

#include "../csp_packed_packed.h"

/* graya_32_to_rgb_24_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgb_24_ia_c
#define CONVERT  RGB_16_TO_8(src[0], dst[0]); \
                 RGB_16_TO_8(src[0], dst[1]); \
                 RGB_16_TO_8(src[0], dst[2]);

#include "../csp_packed_packed.h"

/* graya_32_to_rgb_32_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgb_32_ia_c
#define CONVERT  RGB_16_TO_8(src[0], dst[0]); \
                 RGB_16_TO_8(src[0], dst[1]); \
                 RGB_16_TO_8(src[0], dst[2]);

#include "../csp_packed_packed.h"

/* graya_float_to_rgb_24_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgb_24_ia_c
#define CONVERT  RGB_FLOAT_TO_8(src[0], dst[0]); \
                 RGB_FLOAT_TO_8(src[0], dst[1]); \
                 RGB_FLOAT_TO_8(src[0], dst[2]);


#include "../csp_packed_packed.h"

/* graya_float_to_rgb_32_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgb_32_ia_c
#define CONVERT  RGB_FLOAT_TO_8(src[0], dst[0]); \
                 RGB_FLOAT_TO_8(src[0], dst[1]); \
                 RGB_FLOAT_TO_8(src[0], dst[2]);

#include "../csp_packed_packed.h"

/* graya_16_to_rgb_48_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgb_48_ia_c
#define CONVERT  dst[0] = RGB_8_TO_16(src[0]); \
                 dst[1] = RGB_8_TO_16(src[0]); \
                 dst[2] = RGB_8_TO_16(src[0]);

#include "../csp_packed_packed.h"


/* graya_32_to_rgb_48_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgb_48_ia_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0];

#include "../csp_packed_packed.h"

/* graya_float_to_rgb_48_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgb_48_ia_c
#define CONVERT  RGB_FLOAT_TO_16(src[0], dst[0]); \
                 RGB_FLOAT_TO_16(src[0], dst[1]); \
                 RGB_FLOAT_TO_16(src[0], dst[2]);

#include "../csp_packed_packed.h"

/* */

/* graya_16_to_rgb_float_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgb_float_ia_c
#define CONVERT  dst[0] = RGB_8_TO_FLOAT(src[0]); \
                 dst[1] = RGB_8_TO_FLOAT(src[0]); \
                 dst[2] = RGB_8_TO_FLOAT(src[0]);

#include "../csp_packed_packed.h"

/* graya_32_to_rgb_float_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgb_float_ia_c
#define CONVERT  dst[0] = RGB_16_TO_FLOAT(src[0]); \
                 dst[1] = RGB_16_TO_FLOAT(src[0]); \
                 dst[2] = RGB_16_TO_FLOAT(src[0]);

#include "../csp_packed_packed.h"

/* graya_float_to_rgb_float_ia_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgb_float_ia_c
#define CONVERT  dst[0] = src[0]; \
                 dst[1] = src[0]; \
                 dst[2] = src[0];

#include "../csp_packed_packed.h"

/* GRAYA -> RGB */

/* graya_16_to_rgb_15_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgb_15_c
#define INIT int tmp; \
  INIT_GRAYA_16

#define CONVERT \
  GRAYA_16_TO_GRAY_8(src[0], src[1], tmp); \
  PACK_8_TO_RGB15(tmp,tmp,tmp,dst[0])
  

#include "../csp_packed_packed.h"

/* graya_16_to_rgb_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgb_16_c
#define INIT int tmp; \
  INIT_GRAYA_16

#define CONVERT \
  GRAYA_16_TO_GRAY_8(src[0], src[1], tmp); \
  PACK_8_TO_RGB16(tmp,tmp,tmp,dst[0])
  
#include "../csp_packed_packed.h"

/* graya_16_to_rgb_24_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgb_24_c
#define INIT int tmp; \
  INIT_GRAYA_16

#define CONVERT \
  GRAYA_16_TO_GRAY_8(src[0], src[1], tmp); \
  dst[0] = tmp; \
  dst[1] = tmp; \
  dst[2] = tmp;
  
#include "../csp_packed_packed.h"

/* graya_16_to_rgb_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgb_32_c
#define INIT int tmp; \
  INIT_GRAYA_16

#define CONVERT \
  GRAYA_16_TO_GRAY_8(src[0], src[1], tmp); \
  dst[0] = tmp; \
  dst[1] = tmp; \
  dst[2] = tmp;
  
#include "../csp_packed_packed.h"

/* graya_16_to_rgb_48_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgb_48_c
#define INIT int tmp; \
  INIT_GRAYA_16

#define CONVERT \
  GRAYA_16_TO_GRAY_8(src[0], src[1], tmp); \
  tmp = RGB_8_TO_16(tmp); \
  dst[0] = tmp; \
  dst[1] = tmp; \
  dst[2] = tmp;
  
#include "../csp_packed_packed.h"

/* graya_16_to_rgb_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_rgb_float_c
#define INIT int tmp; float tmp_f; \
  INIT_GRAYA_16

#define CONVERT \
  GRAYA_16_TO_GRAY_8(src[0], src[1], tmp); \
  tmp_f = RGB_8_TO_FLOAT(tmp); \
  dst[0] = tmp_f; \
  dst[1] = tmp_f; \
  dst[2] = tmp_f;
  
#include "../csp_packed_packed.h"

/* */

/* graya_32_to_rgb_15_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgb_15_c
#define INIT int tmp; \
  INIT_GRAYA_32

#define CONVERT \
  GRAYA_32_TO_GRAY_16(src[0], src[1], tmp); \
  PACK_16_TO_RGB15(tmp,tmp,tmp,dst[0])
  

#include "../csp_packed_packed.h"

/* graya_32_to_rgb_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgb_16_c
#define INIT int tmp; \
  INIT_GRAYA_32

#define CONVERT \
  GRAYA_32_TO_GRAY_16(src[0], src[1], tmp); \
  PACK_16_TO_RGB16(tmp,tmp,tmp,dst[0])
  
#include "../csp_packed_packed.h"

/* graya_32_to_rgb_24_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgb_24_c
#define INIT int tmp; \
  INIT_GRAYA_32

#define CONVERT \
  GRAYA_32_TO_GRAY_16(src[0], src[1], tmp); \
  RGB_16_TO_8(tmp, dst[0]); \
  RGB_16_TO_8(tmp, dst[1]); \
  RGB_16_TO_8(tmp, dst[2]);
  
#include "../csp_packed_packed.h"

/* graya_32_to_rgb_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgb_32_c
#define INIT int tmp; \
  INIT_GRAYA_32

#define CONVERT \
  GRAYA_32_TO_GRAY_16(src[0], src[1], tmp); \
  RGB_16_TO_8(tmp, dst[0]); \
  RGB_16_TO_8(tmp, dst[1]); \
  RGB_16_TO_8(tmp, dst[2]);
  
#include "../csp_packed_packed.h"

/* graya_32_to_rgb_48_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgb_48_c
#define INIT int tmp; \
  INIT_GRAYA_32

#define CONVERT \
  GRAYA_32_TO_GRAY_16(src[0], src[1], tmp); \
  dst[0] = tmp; \
  dst[1] = tmp; \
  dst[2] = tmp;
  
#include "../csp_packed_packed.h"

/* graya_32_to_rgb_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_rgb_float_c
#define INIT int tmp_i; float tmp_f;\
  INIT_GRAYA_32

#define CONVERT \
  GRAYA_32_TO_GRAY_16(src[0], src[1], tmp_i); \
  tmp_f = RGB_16_TO_FLOAT(tmp_i); \
  dst[0] = tmp_f; \
  dst[1] = tmp_f; \
  dst[2] = tmp_f;
  
#include "../csp_packed_packed.h"

/* */

/* graya_float_to_rgb_15_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgb_15_c
#define INIT int tmp_i; float tmp_f;\
  INIT_GRAYA_FLOAT

#define CONVERT \
  GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], tmp_f); \
  RGB_FLOAT_TO_16(tmp_f, tmp_i); \
  PACK_16_TO_RGB15(tmp_i,tmp_i,tmp_i,dst[0])
  

#include "../csp_packed_packed.h"

/* graya_float_to_rgb_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgb_16_c
#define INIT int tmp_i; float tmp_f; \
  INIT_GRAYA_FLOAT

#define CONVERT \
  GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], tmp_f); \
  RGB_FLOAT_TO_16(tmp_f, tmp_i); \
  PACK_16_TO_RGB16(tmp_i,tmp_i,tmp_i,dst[0])
  
#include "../csp_packed_packed.h"

/* graya_float_to_rgb_24_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgb_24_c
#define INIT int tmp_i; float tmp_f; \
  INIT_GRAYA_FLOAT

#define CONVERT \
  GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], tmp_f); \
  RGB_FLOAT_TO_8(tmp_f, tmp_i); \
  dst[0] = tmp_i; \
  dst[1] = tmp_i; \
  dst[2] = tmp_i;
  
#include "../csp_packed_packed.h"

/* graya_float_to_rgb_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgb_32_c
#define INIT int tmp_i; float tmp_f; \
  INIT_GRAYA_FLOAT

#define CONVERT \
  GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], tmp_f); \
  RGB_FLOAT_TO_8(tmp_f, tmp_i); \
  dst[0] = tmp_i; \
  dst[1] = tmp_i; \
  dst[2] = tmp_i;
  
#include "../csp_packed_packed.h"

/* graya_float_to_rgb_48_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgb_48_c
#define INIT int tmp_i; float tmp_f; \
  INIT_GRAYA_FLOAT

#define CONVERT \
  GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], tmp_f); \
  RGB_FLOAT_TO_16(tmp_f, tmp_i); \
  dst[0] = tmp_i; \
  dst[1] = tmp_i; \
  dst[2] = tmp_i;
  
#include "../csp_packed_packed.h"

/* graya_float_to_rgb_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_rgb_float_c
#define INIT float tmp_f;\
  INIT_GRAYA_FLOAT

#define CONVERT \
  GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], tmp_f); \
  dst[0] = tmp_f; \
  dst[1] = tmp_f; \
  dst[2] = tmp_f;
  
#include "../csp_packed_packed.h"






void gavl_init_gray_rgb_funcs_c(gavl_pixelformat_function_table_t * tab, const gavl_video_options_t * opt)
  {
  tab->gray_8_to_rgb_15= gray_8_to_rgb_15_c;
  tab->gray_8_to_rgb_16= gray_8_to_rgb_16_c;
  tab->gray_8_to_rgb_24= gray_8_to_rgb_24_c;
  tab->gray_8_to_rgb_32= gray_8_to_rgb_32_c;
  tab->gray_8_to_rgba_32= gray_8_to_rgba_32_c;
  tab->gray_8_to_rgb_48= gray_8_to_rgb_48_c;
  tab->gray_8_to_rgba_64= gray_8_to_rgba_64_c;
  tab->gray_8_to_rgb_float= gray_8_to_rgb_float_c;
  tab->gray_8_to_rgba_float= gray_8_to_rgba_float_c;
  tab->gray_16_to_rgb_15= gray_16_to_rgb_15_c;
  tab->gray_16_to_rgb_16= gray_16_to_rgb_16_c;
  tab->gray_16_to_rgb_24= gray_16_to_rgb_24_c;
  tab->gray_16_to_rgb_32= gray_16_to_rgb_32_c;
  tab->gray_16_to_rgba_32= gray_16_to_rgba_32_c;
  tab->gray_16_to_rgb_48= gray_16_to_rgb_48_c;
  tab->gray_16_to_rgba_64= gray_16_to_rgba_64_c;
  tab->gray_16_to_rgb_float= gray_16_to_rgb_float_c;
  tab->gray_16_to_rgba_float= gray_16_to_rgba_float_c;
  tab->gray_float_to_rgb_15= gray_float_to_rgb_15_c;
  tab->gray_float_to_rgb_16= gray_float_to_rgb_16_c;
  tab->gray_float_to_rgb_24= gray_float_to_rgb_24_c;
  tab->gray_float_to_rgb_32= gray_float_to_rgb_32_c;
  tab->gray_float_to_rgba_32= gray_float_to_rgba_32_c;
  tab->gray_float_to_rgb_48= gray_float_to_rgb_48_c;
  tab->gray_float_to_rgba_64= gray_float_to_rgba_64_c;
  tab->gray_float_to_rgb_float= gray_float_to_rgb_float_c;
  tab->gray_float_to_rgba_float= gray_float_to_rgba_float_c;

  tab->graya_16_to_rgba_32= graya_16_to_rgba_32_c;
  tab->graya_16_to_rgba_64= graya_16_to_rgba_64_c;
  tab->graya_16_to_rgba_float= graya_16_to_rgba_float_c;
  tab->graya_32_to_rgba_32= graya_32_to_rgba_32_c;
  tab->graya_32_to_rgba_64= graya_32_to_rgba_64_c;
  tab->graya_32_to_rgba_float= graya_32_to_rgba_float_c;
  tab->graya_float_to_rgba_32= graya_float_to_rgba_32_c;
  tab->graya_float_to_rgba_64= graya_float_to_rgba_64_c;
  tab->graya_float_to_rgba_float= graya_float_to_rgba_float_c;
  
  if(opt->alpha_mode == GAVL_ALPHA_BLEND_COLOR)
    {
    tab->graya_16_to_rgb_15= graya_16_to_rgb_15_c;
    tab->graya_16_to_rgb_16= graya_16_to_rgb_16_c;
    tab->graya_16_to_rgb_24= graya_16_to_rgb_24_c;
    tab->graya_16_to_rgb_32= graya_16_to_rgb_32_c;
    tab->graya_16_to_rgb_48= graya_16_to_rgb_48_c;
    tab->graya_16_to_rgb_float= graya_16_to_rgb_float_c;
    tab->graya_32_to_rgb_15= graya_32_to_rgb_15_c;
    tab->graya_32_to_rgb_16= graya_32_to_rgb_16_c;
    tab->graya_32_to_rgb_24= graya_32_to_rgb_24_c;
    tab->graya_32_to_rgb_32= graya_32_to_rgb_32_c;
    tab->graya_32_to_rgb_48= graya_32_to_rgb_48_c;
    tab->graya_32_to_rgb_float= graya_32_to_rgb_float_c;
    tab->graya_float_to_rgb_15= graya_float_to_rgb_15_c;
    tab->graya_float_to_rgb_16= graya_float_to_rgb_16_c;
    tab->graya_float_to_rgb_24= graya_float_to_rgb_24_c;
    tab->graya_float_to_rgb_32= graya_float_to_rgb_32_c;
    tab->graya_float_to_rgb_48= graya_float_to_rgb_48_c;
    tab->graya_float_to_rgb_float= graya_float_to_rgb_float_c;
    }
  else
    {
    tab->graya_16_to_rgb_15= graya_16_to_rgb_15_ia_c;
    tab->graya_16_to_rgb_16= graya_16_to_rgb_16_ia_c;
    tab->graya_16_to_rgb_24= graya_16_to_rgb_24_ia_c;
    tab->graya_16_to_rgb_32= graya_16_to_rgb_32_ia_c;
    tab->graya_16_to_rgb_48= graya_16_to_rgb_48_ia_c;
    tab->graya_16_to_rgb_float= graya_16_to_rgb_float_ia_c;
    tab->graya_32_to_rgb_15= graya_32_to_rgb_15_ia_c;
    tab->graya_32_to_rgb_16= graya_32_to_rgb_16_ia_c;
    tab->graya_32_to_rgb_24= graya_32_to_rgb_24_ia_c;
    tab->graya_32_to_rgb_32= graya_32_to_rgb_32_ia_c;
    tab->graya_32_to_rgb_48= graya_32_to_rgb_48_ia_c;
    tab->graya_32_to_rgb_float= graya_32_to_rgb_float_ia_c;
    tab->graya_float_to_rgb_15= graya_float_to_rgb_15_ia_c;
    tab->graya_float_to_rgb_16= graya_float_to_rgb_16_ia_c;
    tab->graya_float_to_rgb_24= graya_float_to_rgb_24_ia_c;
    tab->graya_float_to_rgb_32= graya_float_to_rgb_32_ia_c;
    tab->graya_float_to_rgb_48= graya_float_to_rgb_48_ia_c;
    tab->graya_float_to_rgb_float= graya_float_to_rgb_float_ia_c;
    }
  
  }
