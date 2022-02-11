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

/* rgb_15_to_gray_8_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_gray_8_c
#define CONVERT    RGB_24_TO_YJ_8(RGB15_TO_R_8(src[0]), \
         RGB15_TO_G_8(src[0]), \
         RGB15_TO_B_8(src[0]), \
         dst[0])


#include "../csp_packed_packed.h"

/* bgr_15_to_gray_8_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME bgr_15_to_gray_8_c
#define CONVERT    RGB_24_TO_YJ_8(BGR15_TO_R_8(src[0]), \
         BGR15_TO_G_8(src[0]), \
         BGR15_TO_B_8(src[0]), \
         dst[0])

#include "../csp_packed_packed.h"

/* rgb_16_to_gray_8_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_gray_8_c
#define CONVERT    RGB_24_TO_YJ_8(RGB16_TO_R_8(src[0]), \
         RGB16_TO_G_8(src[0]), \
         RGB16_TO_B_8(src[0]), \
         dst[0])


#include "../csp_packed_packed.h"

/* bgr_16_to_gray_8_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME bgr_16_to_gray_8_c
#define CONVERT    RGB_24_TO_YJ_8(BGR16_TO_R_8(src[0]), \
         BGR16_TO_G_8(src[0]), \
         BGR16_TO_B_8(src[0]), \
         dst[0])

#include "../csp_packed_packed.h"

/* */

/* rgb_15_to_gray_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_gray_16_c
#define CONVERT    RGB_24_TO_YJ_16(RGB15_TO_R_8(src[0]), \
         RGB15_TO_G_8(src[0]), \
         RGB15_TO_B_8(src[0]), \
         dst[0])


#include "../csp_packed_packed.h"

/* bgr_15_to_gray_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME bgr_15_to_gray_16_c
#define CONVERT    RGB_24_TO_YJ_16(BGR15_TO_R_8(src[0]), \
         BGR15_TO_G_8(src[0]), \
         BGR15_TO_B_8(src[0]), \
         dst[0])

#include "../csp_packed_packed.h"

/* rgb_16_to_gray_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_gray_16_c
#define CONVERT    RGB_24_TO_YJ_16(RGB16_TO_R_8(src[0]), \
         RGB16_TO_G_8(src[0]), \
         RGB16_TO_B_8(src[0]), \
         dst[0])


#include "../csp_packed_packed.h"

/* bgr_16_to_gray_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME bgr_16_to_gray_16_c
#define CONVERT    RGB_24_TO_YJ_16(BGR16_TO_R_8(src[0]), \
         BGR16_TO_G_8(src[0]), \
         BGR16_TO_B_8(src[0]), \
         dst[0])

#include "../csp_packed_packed.h"

/* */

/* rgb_15_to_gray_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_gray_float_c
#define CONVERT    RGB_24_TO_Y_FLOAT(RGB15_TO_R_8(src[0]), \
         RGB15_TO_G_8(src[0]), \
         RGB15_TO_B_8(src[0]), \
         dst[0])


#include "../csp_packed_packed.h"

/* bgr_15_to_gray_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME bgr_15_to_gray_float_c
#define CONVERT    RGB_24_TO_Y_FLOAT(BGR15_TO_R_8(src[0]), \
         BGR15_TO_G_8(src[0]), \
         BGR15_TO_B_8(src[0]), \
         dst[0])

#include "../csp_packed_packed.h"

/* rgb_16_to_gray_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_gray_float_c
#define CONVERT    RGB_24_TO_Y_FLOAT(RGB16_TO_R_8(src[0]), \
         RGB16_TO_G_8(src[0]), \
         RGB16_TO_B_8(src[0]), \
         dst[0])


#include "../csp_packed_packed.h"

/* bgr_16_to_gray_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME bgr_16_to_gray_float_c
#define CONVERT    RGB_24_TO_Y_FLOAT(BGR16_TO_R_8(src[0]), \
         BGR16_TO_G_8(src[0]), \
         BGR16_TO_B_8(src[0]), \
         dst[0])

#include "../csp_packed_packed.h"

/* RGB 15/16 to graya */

/* rgb_15_to_graya_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_graya_16_c
#define CONVERT    RGB_24_TO_YJ_8(RGB15_TO_R_8(src[0]), \
         RGB15_TO_G_8(src[0]), \
         RGB15_TO_B_8(src[0]), \
         dst[0]) \
         dst[1] = 0Xff;


#include "../csp_packed_packed.h"

/* bgr_15_to_graya_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME bgr_15_to_graya_16_c
#define CONVERT    RGB_24_TO_YJ_8(BGR15_TO_R_8(src[0]), \
         BGR15_TO_G_8(src[0]), \
         BGR15_TO_B_8(src[0]), \
         dst[0]) \
         dst[1] = 0Xff;

#include "../csp_packed_packed.h"

/* rgb_16_to_graya_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_graya_16_c
#define CONVERT    RGB_24_TO_YJ_8(RGB16_TO_R_8(src[0]), \
         RGB16_TO_G_8(src[0]), \
         RGB16_TO_B_8(src[0]), \
         dst[0]) \
         dst[1] = 0Xff;


#include "../csp_packed_packed.h"

/* bgr_16_to_graya_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME bgr_16_to_graya_16_c
#define CONVERT    RGB_24_TO_YJ_8(BGR16_TO_R_8(src[0]), \
         BGR16_TO_G_8(src[0]), \
         BGR16_TO_B_8(src[0]), \
         dst[0]) \
         dst[1] = 0Xff;

#include "../csp_packed_packed.h"

/* */

/* rgb_15_to_graya_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_graya_32_c
#define CONVERT    RGB_24_TO_YJ_16(RGB15_TO_R_8(src[0]), \
         RGB15_TO_G_8(src[0]), \
         RGB15_TO_B_8(src[0]), \
         dst[0]) \
         dst[1] = 0Xffff;


#include "../csp_packed_packed.h"

/* bgr_15_to_graya_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME bgr_15_to_graya_32_c
#define CONVERT    RGB_24_TO_YJ_16(BGR15_TO_R_8(src[0]), \
         BGR15_TO_G_8(src[0]), \
         BGR15_TO_B_8(src[0]), \
         dst[0]) \
         dst[1] = 0Xffff;

#include "../csp_packed_packed.h"

/* rgb_16_to_graya_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_graya_32_c
#define CONVERT    RGB_24_TO_YJ_16(RGB16_TO_R_8(src[0]), \
         RGB16_TO_G_8(src[0]), \
         RGB16_TO_B_8(src[0]), \
         dst[0]) \
         dst[1] = 0Xffff;


#include "../csp_packed_packed.h"

/* bgr_16_to_graya_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME bgr_16_to_graya_32_c
#define CONVERT    RGB_24_TO_YJ_16(BGR16_TO_R_8(src[0]), \
         BGR16_TO_G_8(src[0]), \
         BGR16_TO_B_8(src[0]), \
         dst[0]) \
         dst[1] = 0Xffff;

#include "../csp_packed_packed.h"

/* */

/* rgb_15_to_graya_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_15_to_graya_float_c
#define CONVERT    RGB_24_TO_Y_FLOAT(RGB15_TO_R_8(src[0]), \
         RGB15_TO_G_8(src[0]), \
         RGB15_TO_B_8(src[0]), \
         dst[0]) \
         dst[1] = 1.0;


#include "../csp_packed_packed.h"

/* bgr_15_to_graya_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME bgr_15_to_graya_float_c
#define CONVERT    RGB_24_TO_Y_FLOAT(BGR15_TO_R_8(src[0]), \
         BGR15_TO_G_8(src[0]), \
         BGR15_TO_B_8(src[0]), \
         dst[0]) \
         dst[1] = 1.0;

#include "../csp_packed_packed.h"

/* rgb_16_to_graya_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_16_to_graya_float_c
#define CONVERT    RGB_24_TO_Y_FLOAT(RGB16_TO_R_8(src[0]), \
         RGB16_TO_G_8(src[0]), \
         RGB16_TO_B_8(src[0]), \
         dst[0]) \
         dst[1] = 1.0;


#include "../csp_packed_packed.h"

/* bgr_16_to_graya_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME bgr_16_to_graya_float_c
#define CONVERT    RGB_24_TO_Y_FLOAT(BGR16_TO_R_8(src[0]), \
         BGR16_TO_G_8(src[0]), \
         BGR16_TO_B_8(src[0]), \
         dst[0]) \
         dst[1] = 1.0;

#include "../csp_packed_packed.h"

/* RGB 24/32 -> Gray */

/* rgb_24_to_gray_8_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_gray_8_c
#define CONVERT \
         RGB_24_TO_YJ_8(src[0], src[1], src[2], \
         dst[0])

#include "../csp_packed_packed.h"

/* bgr_24_to_gray_8_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME bgr_24_to_gray_8_c
#define CONVERT \
         RGB_24_TO_YJ_8(src[2], src[1], src[0], \
         dst[0])

#include "../csp_packed_packed.h"

/* rgb_32_to_gray_8_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_gray_8_c
#define CONVERT \
         RGB_24_TO_YJ_8(src[0], src[1], src[2], \
         dst[0])

#include "../csp_packed_packed.h"

/* bgr_32_to_gray_8_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME bgr_32_to_gray_8_c
#define CONVERT \
         RGB_24_TO_YJ_8(src[2], src[1], src[0], \
         dst[0])

#include "../csp_packed_packed.h"

/* */

/* rgb_24_to_gray_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_gray_16_c
#define CONVERT \
         RGB_24_TO_YJ_16(src[0], src[1], src[2], \
         dst[0])

#include "../csp_packed_packed.h"

/* bgr_24_to_gray_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME bgr_24_to_gray_16_c
#define CONVERT \
         RGB_24_TO_YJ_16(src[2], src[1], src[0], \
         dst[0])

#include "../csp_packed_packed.h"

/* rgb_32_to_gray_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_gray_16_c
#define CONVERT \
         RGB_24_TO_YJ_16(src[0], src[1], src[2], \
         dst[0])

#include "../csp_packed_packed.h"

/* bgr_32_to_gray_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME bgr_32_to_gray_16_c
#define CONVERT \
         RGB_24_TO_YJ_16(src[2], src[1], src[0], \
         dst[0])

#include "../csp_packed_packed.h"

/* */

/* rgb_24_to_gray_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_gray_float_c
#define CONVERT \
         RGB_24_TO_Y_FLOAT(src[0], src[1], src[2], \
         dst[0])

#include "../csp_packed_packed.h"

/* bgr_24_to_gray_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME bgr_24_to_gray_float_c
#define CONVERT \
         RGB_24_TO_Y_FLOAT(src[2], src[1], src[0], \
         dst[0])

#include "../csp_packed_packed.h"

/* rgb_32_to_gray_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_gray_float_c
#define CONVERT \
         RGB_24_TO_Y_FLOAT(src[0], src[1], src[2], \
         dst[0])

#include "../csp_packed_packed.h"

/* bgr_32_to_gray_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME bgr_32_to_gray_float_c
#define CONVERT \
         RGB_24_TO_Y_FLOAT(src[2], src[1], src[0], \
         dst[0])

#include "../csp_packed_packed.h"

/* RGB 24/32 -> Graya */

/* rgb_24_to_graya_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_graya_16_c
#define CONVERT \
         RGB_24_TO_YJ_8(src[0], src[1], src[2], \
         dst[0]) \
         dst[1] = 0xff;

#include "../csp_packed_packed.h"

/* bgr_24_to_graya_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME bgr_24_to_graya_16_c
#define CONVERT \
         RGB_24_TO_YJ_8(src[2], src[1], src[0], \
         dst[0]) \
         dst[1] = 0xff;

#include "../csp_packed_packed.h"

/* rgb_32_to_graya_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_graya_16_c
#define CONVERT \
         RGB_24_TO_YJ_8(src[0], src[1], src[2], \
         dst[0]) \
         dst[1] = 0xff;

#include "../csp_packed_packed.h"

/* bgr_32_to_graya_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME bgr_32_to_graya_16_c
#define CONVERT \
         RGB_24_TO_YJ_8(src[2], src[1], src[0], \
         dst[0]) \
         dst[1] = 0xff;

#include "../csp_packed_packed.h"

/* */

/* rgb_24_to_graya_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_graya_32_c
#define CONVERT \
         RGB_24_TO_YJ_16(src[0], src[1], src[2], \
         dst[0]) \
         dst[1] = 0xffff;

#include "../csp_packed_packed.h"

/* bgr_24_to_graya_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME bgr_24_to_graya_32_c
#define CONVERT \
         RGB_24_TO_YJ_16(src[2], src[1], src[0], \
         dst[0]) \
         dst[1] = 0xffff;

#include "../csp_packed_packed.h"

/* rgb_32_to_graya_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_graya_32_c
#define CONVERT \
         RGB_24_TO_YJ_16(src[0], src[1], src[2], \
         dst[0]) \
         dst[1] = 0xffff;

#include "../csp_packed_packed.h"

/* bgr_32_to_graya_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME bgr_32_to_graya_32_c
#define CONVERT \
         RGB_24_TO_YJ_16(src[2], src[1], src[0], \
         dst[0]) \
         dst[1] = 0xffff;

#include "../csp_packed_packed.h"

/* */

/* rgb_24_to_graya_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_24_to_graya_float_c
#define CONVERT \
         RGB_24_TO_Y_FLOAT(src[0], src[1], src[2], \
         dst[0]) \
         dst[1] = 1.0;

#include "../csp_packed_packed.h"

/* bgr_24_to_graya_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME bgr_24_to_graya_float_c
#define CONVERT \
         RGB_24_TO_Y_FLOAT(src[2], src[1], src[0], \
         dst[0]) \
         dst[1] = 1.0;

#include "../csp_packed_packed.h"

/* rgb_32_to_graya_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_32_to_graya_float_c
#define CONVERT \
         RGB_24_TO_Y_FLOAT(src[0], src[1], src[2], \
         dst[0]) \
         dst[1] = 1.0;

#include "../csp_packed_packed.h"

/* bgr_32_to_graya_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME bgr_32_to_graya_float_c
#define CONVERT \
         RGB_24_TO_Y_FLOAT(src[2], src[1], src[0], \
         dst[0]) \
         dst[1] = 1.0;

#include "../csp_packed_packed.h"

/* RGB 48 -> Gray */

/* rgb_48_to_gray_8_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_gray_8_c
#define CONVERT \
         RGB_48_TO_YJ_8(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"

/* rgb_48_to_graya_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_graya_16_c
#define CONVERT \
         RGB_48_TO_YJ_8(src[0], src[1], src[2], dst[0]) \
         dst[1] = 0xff;

#include "../csp_packed_packed.h"

/* rgb_48_to_gray_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_gray_16_c
#define CONVERT \
         RGB_48_TO_YJ_16(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"

/* rgb_48_to_graya_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_graya_32_c
#define CONVERT \
         RGB_48_TO_YJ_16(src[0], src[1], src[2], dst[0]) \
         dst[1] = 0xffff;

#include "../csp_packed_packed.h"

/* rgb_48_to_gray_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_gray_float_c
#define CONVERT \
         RGB_48_TO_Y_FLOAT(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"

/* rgb_48_to_graya_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_48_to_graya_float_c
#define CONVERT \
         RGB_48_TO_Y_FLOAT(src[0], src[1], src[2], dst[0]) \
         dst[1] = 1.0;

#include "../csp_packed_packed.h"

/* RGB float -> Gray */

/* rgb_float_to_gray_8_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_gray_8_c
#define INIT INIT_RGB_FLOAT_TO_Y
#define CONVERT \
         RGB_FLOAT_TO_YJ_8(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"

/* rgb_float_to_graya_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_graya_16_c
#define INIT INIT_RGB_FLOAT_TO_Y
#define CONVERT \
         RGB_FLOAT_TO_YJ_8(src[0], src[1], src[2], dst[0]) \
         dst[1] = 0xff;

#include "../csp_packed_packed.h"

/* rgb_float_to_gray_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_gray_16_c
#define INIT INIT_RGB_FLOAT_TO_Y
#define CONVERT \
         RGB_FLOAT_TO_YJ_16(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"

/* rgb_float_to_graya_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_graya_32_c
#define INIT INIT_RGB_FLOAT_TO_Y
#define CONVERT \
         RGB_FLOAT_TO_YJ_16(src[0], src[1], src[2], dst[0]) \
         dst[1] = 0xffff;

#include "../csp_packed_packed.h"

/* rgb_float_to_gray_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_gray_float_c
#define CONVERT \
         RGB_FLOAT_TO_Y_FLOAT(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"

/* rgb_float_to_graya_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgb_float_to_graya_float_c
#define CONVERT \
         RGB_FLOAT_TO_Y_FLOAT(src[0], src[1], src[2], dst[0]) \
         dst[1] = 1.0;

#include "../csp_packed_packed.h"

/* RGBA -> Graya */

/* rgba_32_to_graya_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_graya_16_c
#define CONVERT \
         RGB_24_TO_YJ_8(src[0], src[1], src[2], \
         dst[0]) \
         dst[1] = src[3];

#include "../csp_packed_packed.h"

/* rgba_32_to_graya_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_graya_32_c
#define CONVERT \
         RGB_24_TO_YJ_16(src[0], src[1], src[2], \
         dst[0]) \
         dst[1] = RGB_8_TO_16(src[3]);

#include "../csp_packed_packed.h"

/* rgba_32_to_graya_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_graya_float_c
#define CONVERT \
         RGB_24_TO_Y_FLOAT(src[0], src[1], src[2], \
         dst[0]) \
         dst[1] = RGB_8_TO_FLOAT(src[3]);

#include "../csp_packed_packed.h"


/* rgba_64_to_graya_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_graya_16_c
#define CONVERT \
         RGB_48_TO_YJ_8(src[0], src[1], src[2], \
         dst[0]) \
         RGB_16_TO_8(src[3], dst[1]);

#include "../csp_packed_packed.h"

/* rgba_64_to_graya_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_graya_32_c
#define CONVERT \
         RGB_48_TO_YJ_16(src[0], src[1], src[2], \
         dst[0]) \
         dst[1] = src[3];

#include "../csp_packed_packed.h"

/* rgba_64_to_graya_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_graya_float_c
#define CONVERT \
         RGB_48_TO_Y_FLOAT(src[0], src[1], src[2], \
         dst[0]) \
         dst[1] = RGB_16_TO_FLOAT(src[3]);

#include "../csp_packed_packed.h"

/* rgba_float_to_graya_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_graya_16_c
#define INIT INIT_RGB_FLOAT_TO_Y
#define CONVERT \
         RGB_FLOAT_TO_YJ_8(src[0], src[1], src[2], \
         dst[0]) \
         RGB_FLOAT_TO_8(src[3], dst[1]);

#include "../csp_packed_packed.h"

/* rgba_float_to_graya_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_graya_32_c
#define INIT INIT_RGB_FLOAT_TO_Y
#define CONVERT \
         RGB_FLOAT_TO_YJ_16(src[0], src[1], src[2], \
         dst[0]) \
         RGB_FLOAT_TO_16(src[3], dst[1]);

#include "../csp_packed_packed.h"

/* rgba_float_to_graya_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_graya_float_c
#define CONVERT \
         RGB_FLOAT_TO_Y_FLOAT(src[0], src[1], src[2], \
         dst[0]) \
         dst[1] = src[3];

#include "../csp_packed_packed.h"

/* RGBA -> GRAY */

/* rgba_32_to_gray_8_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_gray_8_c
#define INIT INIT_RGBA_32_GRAY
#define CONVERT \
  RGBA_32_TO_GRAY_8(src[0], src[1], src[2], src[3], dst[0])

#include "../csp_packed_packed.h"

/* rgba_32_to_gray_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_gray_16_c
#define INIT int dst_gray; INIT_RGBA_32_GRAY
#define CONVERT \
  RGBA_32_TO_GRAY_8(src[0], src[1], src[2], src[3], dst_gray) \
  dst[0] = RGB_8_TO_16(dst_gray);

#include "../csp_packed_packed.h"

/* rgba_32_to_gray_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_gray_float_c
#define INIT int dst_gray; INIT_RGBA_32_GRAY
#define CONVERT \
  RGBA_32_TO_GRAY_8(src[0], src[1], src[2], src[3], dst_gray) \
  dst[0] = RGB_8_TO_FLOAT(dst_gray);

#include "../csp_packed_packed.h"

/* rgba_64_to_gray_8_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_gray_8_c
#define INIT int dst_gray; INIT_RGBA_64_GRAY
#define CONVERT \
  RGBA_64_TO_GRAY_16(src[0], src[1], src[2], src[3], dst_gray) \
  RGB_16_TO_8(dst_gray, dst[0]);

#include "../csp_packed_packed.h"

/* rgba_64_to_gray_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_gray_16_c
#define INIT INIT_RGBA_64_GRAY
#define CONVERT \
  RGBA_64_TO_GRAY_16(src[0], src[1], src[2], src[3], dst[0])

#include "../csp_packed_packed.h"

/* rgba_64_to_gray_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_gray_float_c
#define INIT int dst_gray; INIT_RGBA_64_GRAY
#define CONVERT \
  RGBA_64_TO_GRAY_16(src[0], src[1], src[2], src[3], dst_gray) \
  dst[0] = RGB_16_TO_FLOAT(dst_gray);

#include "../csp_packed_packed.h"

/* rgba_float_to_gray_8_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_gray_8_c
#define INIT float dst_gray; INIT_RGBA_FLOAT_GRAY
#define CONVERT \
  RGBA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], src[2], src[3], dst_gray) \
  RGB_FLOAT_TO_8(dst_gray, dst[0]);

#include "../csp_packed_packed.h"

/* rgba_float_to_gray_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_gray_16_c
#define INIT float dst_gray; INIT_RGBA_FLOAT_GRAY
#define CONVERT \
  RGBA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], src[2], src[3], dst_gray) \
  RGB_FLOAT_TO_16(dst_gray, dst[0]);

#include "../csp_packed_packed.h"

/* rgba_float_to_gray_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_gray_float_c
#define INIT INIT_RGBA_FLOAT_GRAY
#define CONVERT \
  RGBA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], src[2], src[3], dst[0])

#include "../csp_packed_packed.h"

/* RGBA -> Gray (ignoring alpha) */

/* rgba_32_to_gray_8_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_gray_8_ia_c
#define CONVERT \
  RGB_24_TO_YJ_8(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"

/* rgba_32_to_gray_16_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_gray_16_ia_c
#define CONVERT \
  RGB_24_TO_YJ_16(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"

/* rgba_32_to_gray_float_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_gray_float_ia_c
#define CONVERT \
  RGB_24_TO_Y_FLOAT(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"

/* rgba_64_to_gray_8_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_gray_8_ia_c
#define CONVERT \
  RGB_48_TO_YJ_8(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"

/* rgba_64_to_gray_16_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_gray_16_ia_c
#define CONVERT \
  RGB_48_TO_YJ_16(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"

/* rgba_64_to_gray_float_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_64_to_gray_float_ia_c
#define CONVERT \
  RGB_48_TO_Y_FLOAT(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"


/* rgba_float_to_gray_8_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_gray_8_ia_c
#define INIT INIT_RGB_FLOAT_TO_Y
#define CONVERT \
  RGB_FLOAT_TO_YJ_8(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"

/* rgba_float_to_gray_16_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_gray_16_ia_c
#define INIT INIT_RGB_FLOAT_TO_Y
#define CONVERT \
  RGB_FLOAT_TO_YJ_16(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"

/* rgba_float_to_gray_float_ia_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_float_to_gray_float_ia_c
#define CONVERT \
  RGB_FLOAT_TO_Y_FLOAT(src[0], src[1], src[2], dst[0])

#include "../csp_packed_packed.h"




void gavl_init_rgb_gray_funcs_c(gavl_pixelformat_function_table_t * tab,
                                const gavl_video_options_t * opt)
  {
  if(opt->alpha_mode == GAVL_ALPHA_BLEND_COLOR)
    {
    tab->rgba_32_to_gray_8 = rgba_32_to_gray_8_c;
    tab->rgba_64_to_gray_8 = rgba_64_to_gray_8_c;
    tab->rgba_float_to_gray_8 = rgba_float_to_gray_8_c;

    tab->rgba_32_to_gray_16 = rgba_32_to_gray_16_c;
    tab->rgba_64_to_gray_16 = rgba_64_to_gray_16_c;
    tab->rgba_float_to_gray_16 = rgba_float_to_gray_16_c;

    tab->rgba_32_to_gray_float = rgba_32_to_gray_float_c;
    tab->rgba_64_to_gray_float = rgba_64_to_gray_float_c;
    tab->rgba_float_to_gray_float = rgba_float_to_gray_float_c;
    }    
  else
    {
    tab->rgba_32_to_gray_8 = rgba_32_to_gray_8_ia_c;
    tab->rgba_64_to_gray_8 = rgba_64_to_gray_8_ia_c;
    tab->rgba_float_to_gray_8 = rgba_float_to_gray_8_ia_c;

    tab->rgba_32_to_gray_16 = rgba_32_to_gray_16_ia_c;
    tab->rgba_64_to_gray_16 = rgba_64_to_gray_16_ia_c;
    tab->rgba_float_to_gray_16 = rgba_float_to_gray_16_ia_c;

    tab->rgba_32_to_gray_float = rgba_32_to_gray_float_ia_c;
    tab->rgba_64_to_gray_float = rgba_64_to_gray_float_ia_c;
    tab->rgba_float_to_gray_float = rgba_float_to_gray_float_ia_c;
    }

  
  tab->rgb_15_to_gray_8 = rgb_15_to_gray_8_c;
  tab->bgr_15_to_gray_8 = bgr_15_to_gray_8_c;
  tab->rgb_16_to_gray_8 = rgb_16_to_gray_8_c;
  tab->bgr_16_to_gray_8 = bgr_16_to_gray_8_c;
  tab->rgb_24_to_gray_8 = rgb_24_to_gray_8_c;
  tab->bgr_24_to_gray_8 = bgr_24_to_gray_8_c;
  tab->rgb_32_to_gray_8 = rgb_32_to_gray_8_c;
  tab->bgr_32_to_gray_8 = bgr_32_to_gray_8_c;
  tab->rgb_48_to_gray_8 = rgb_48_to_gray_8_c;
  tab->rgb_float_to_gray_8 = rgb_float_to_gray_8_c;
  tab->rgb_15_to_gray_16 = rgb_15_to_gray_16_c;
  tab->bgr_15_to_gray_16 = bgr_15_to_gray_16_c;
  tab->rgb_16_to_gray_16 = rgb_16_to_gray_16_c;
  tab->bgr_16_to_gray_16 = bgr_16_to_gray_16_c;
  tab->rgb_24_to_gray_16 = rgb_24_to_gray_16_c;
  tab->bgr_24_to_gray_16 = bgr_24_to_gray_16_c;
  tab->rgb_32_to_gray_16 = rgb_32_to_gray_16_c;
  tab->bgr_32_to_gray_16 = bgr_32_to_gray_16_c;
  tab->rgb_48_to_gray_16 = rgb_48_to_gray_16_c;
  tab->rgb_float_to_gray_16 = rgb_float_to_gray_16_c;
  tab->rgb_15_to_gray_float = rgb_15_to_gray_float_c;
  tab->bgr_15_to_gray_float = bgr_15_to_gray_float_c;
  tab->rgb_16_to_gray_float = rgb_16_to_gray_float_c;
  tab->bgr_16_to_gray_float = bgr_16_to_gray_float_c;
  tab->rgb_24_to_gray_float = rgb_24_to_gray_float_c;
  tab->bgr_24_to_gray_float = bgr_24_to_gray_float_c;
  tab->rgb_32_to_gray_float = rgb_32_to_gray_float_c;
  tab->bgr_32_to_gray_float = bgr_32_to_gray_float_c;
  tab->rgb_48_to_gray_float = rgb_48_to_gray_float_c;
  tab->rgb_float_to_gray_float = rgb_float_to_gray_float_c;
  tab->rgb_15_to_graya_16 = rgb_15_to_graya_16_c;
  tab->bgr_15_to_graya_16 = bgr_15_to_graya_16_c;
  tab->rgb_16_to_graya_16 = rgb_16_to_graya_16_c;
  tab->bgr_16_to_graya_16 = bgr_16_to_graya_16_c;
  tab->rgb_24_to_graya_16 = rgb_24_to_graya_16_c;
  tab->bgr_24_to_graya_16 = bgr_24_to_graya_16_c;
  tab->rgb_32_to_graya_16 = rgb_32_to_graya_16_c;
  tab->bgr_32_to_graya_16 = bgr_32_to_graya_16_c;
  tab->rgba_32_to_graya_16 = rgba_32_to_graya_16_c;
  tab->rgb_48_to_graya_16 = rgb_48_to_graya_16_c;
  tab->rgba_64_to_graya_16 = rgba_64_to_graya_16_c;
  tab->rgb_float_to_graya_16 = rgb_float_to_graya_16_c;
  tab->rgba_float_to_graya_16 = rgba_float_to_graya_16_c;
  tab->rgb_15_to_graya_32 = rgb_15_to_graya_32_c;
  tab->bgr_15_to_graya_32 = bgr_15_to_graya_32_c;
  tab->rgb_16_to_graya_32 = rgb_16_to_graya_32_c;
  tab->bgr_16_to_graya_32 = bgr_16_to_graya_32_c;
  tab->rgb_24_to_graya_32 = rgb_24_to_graya_32_c;
  tab->bgr_24_to_graya_32 = bgr_24_to_graya_32_c;
  tab->rgb_32_to_graya_32 = rgb_32_to_graya_32_c;
  tab->bgr_32_to_graya_32 = bgr_32_to_graya_32_c;
  tab->rgba_32_to_graya_32 = rgba_32_to_graya_32_c;
  tab->rgb_48_to_graya_32 = rgb_48_to_graya_32_c;
  tab->rgba_64_to_graya_32 = rgba_64_to_graya_32_c;
  tab->rgb_float_to_graya_32 = rgb_float_to_graya_32_c;
  tab->rgba_float_to_graya_32 = rgba_float_to_graya_32_c;
  tab->rgb_15_to_graya_float = rgb_15_to_graya_float_c;
  tab->bgr_15_to_graya_float = bgr_15_to_graya_float_c;
  tab->rgb_16_to_graya_float = rgb_16_to_graya_float_c;
  tab->bgr_16_to_graya_float = bgr_16_to_graya_float_c;
  tab->rgb_24_to_graya_float = rgb_24_to_graya_float_c;
  tab->bgr_24_to_graya_float = bgr_24_to_graya_float_c;
  tab->rgb_32_to_graya_float = rgb_32_to_graya_float_c;
  tab->bgr_32_to_graya_float = bgr_32_to_graya_float_c;
  tab->rgba_32_to_graya_float = rgba_32_to_graya_float_c;
  tab->rgb_48_to_graya_float = rgb_48_to_graya_float_c;
  tab->rgba_64_to_graya_float = rgba_64_to_graya_float_c;
  tab->rgb_float_to_graya_float = rgb_float_to_graya_float_c;
  tab->rgba_float_to_graya_float = rgba_float_to_graya_float_c;
  
  }
