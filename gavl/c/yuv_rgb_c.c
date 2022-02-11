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


/*************************************************
  YUY2 ->
 *************************************************/

#ifndef HQ

/* yuy2_to_rgb_15_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  2
#define FUNC_NAME yuy2_to_rgb_15_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[3], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src[2], src[1], src[3], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[1]);

#define INIT \
  uint8_t r; \
  uint8_t g; \
  uint8_t b; \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* yuy2_to_bgr_15_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  2
#define FUNC_NAME yuy2_to_bgr_15_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[3], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src[2], src[1], src[3], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[1]);

#define INIT \
  uint8_t r; \
  uint8_t g; \
  uint8_t b; \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* yuy2_to_rgb_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  2
#define FUNC_NAME yuy2_to_rgb_16_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[3], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src[2], src[1], src[3], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[1]);

#define INIT \
  uint8_t r; \
  uint8_t g; \
  uint8_t b; \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* yuy2_to_bgr_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  2
#define FUNC_NAME yuy2_to_bgr_16_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[3], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src[2], src[1], src[3], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[1]);

#define INIT \
  uint8_t r; \
  uint8_t g; \
  uint8_t b; \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* yuy2_to_rgb_24_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 6
#define NUM_PIXELS  2
#define FUNC_NAME yuy2_to_rgb_24_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[3], dst[0], dst[1], dst[2]) \
  YUV_8_TO_RGB_24(src[2], src[1], src[3], dst[3], dst[4], dst[5])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* yuy2_to_rgb_48_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 6
#define NUM_PIXELS  2
#define FUNC_NAME yuy2_to_rgb_48_c
#define CONVERT  \
  YUV_8_TO_RGB_48(src[0], src[1], src[3], dst[0], dst[1], dst[2]) \
  YUV_8_TO_RGB_48(src[2], src[1], src[3], dst[3], dst[4], dst[5])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* yuy2_to_rgb_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 6
#define NUM_PIXELS  2
#define FUNC_NAME yuy2_to_rgb_float_c
#define CONVERT  \
  YUV_8_TO_RGB_FLOAT(src[0], src[1], src[3], dst[0], dst[1], dst[2]) \
  YUV_8_TO_RGB_FLOAT(src[2], src[1], src[3], dst[3], dst[4], dst[5])

#define INIT \
  float i_tmp;

#include "../csp_packed_packed.h"




/* yuy2_to_bgr_24_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 6
#define NUM_PIXELS  2
#define FUNC_NAME yuy2_to_bgr_24_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[3], dst[2], dst[1], dst[0]) \
  YUV_8_TO_RGB_24(src[2], src[1], src[3], dst[5], dst[4], dst[3])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* yuy2_to_rgb_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 8
#define NUM_PIXELS  2
#define FUNC_NAME yuy2_to_rgb_32_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[3], dst[0], dst[1], dst[2]) \
  YUV_8_TO_RGB_24(src[2], src[1], src[3], dst[4], dst[5], dst[6])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* yuy2_to_bgr_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 8
#define NUM_PIXELS  2
#define FUNC_NAME yuy2_to_bgr_32_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[3], dst[2], dst[1], dst[0]) \
  YUV_8_TO_RGB_24(src[2], src[1], src[3], dst[6], dst[5], dst[4])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* yuy2_to_rgba_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 8
#define NUM_PIXELS  2
#define FUNC_NAME yuy2_to_rgba_32_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[3], dst[0], dst[1], dst[2]) \
  dst[3] = 0xFF;\
  YUV_8_TO_RGB_24(src[2], src[1], src[3], dst[4], dst[5], dst[6]) \
  dst[7] = 0xFF;

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* yuy2_to_rgba_64_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 8
#define NUM_PIXELS  2
#define FUNC_NAME yuy2_to_rgba_64_c
#define CONVERT  \
  YUV_8_TO_RGB_48(src[0], src[1], src[3], dst[0], dst[1], dst[2]) \
  dst[3] = 0xFFFF;\
  YUV_8_TO_RGB_48(src[2], src[1], src[3], dst[4], dst[5], dst[6]) \
  dst[7] = 0xFFFF;

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* yuy2_to_rgba_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 8
#define NUM_PIXELS  2
#define FUNC_NAME yuy2_to_rgba_float_c
#define CONVERT  \
  YUV_8_TO_RGB_FLOAT(src[0], src[1], src[3], dst[0], dst[1], dst[2]) \
  dst[3] = 1.0;\
  YUV_8_TO_RGB_FLOAT(src[2], src[1], src[3], dst[4], dst[5], dst[6]) \
  dst[7] = 1.0;

#define INIT \
  float i_tmp;

#include "../csp_packed_packed.h"

/*************************************************
  UYVY ->
 *************************************************/

/* uyvy_to_rgb_15_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  2
#define FUNC_NAME uyvy_to_rgb_15_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[1], src[0], src[2], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src[3], src[0], src[2], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[1]);

#define INIT \
  uint8_t r; \
  uint8_t g; \
  uint8_t b; \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* uyvy_to_bgr_15_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  2
#define FUNC_NAME uyvy_to_bgr_15_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[1], src[0], src[2], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src[3], src[0], src[2], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[1]);

#define INIT \
  uint8_t r; \
  uint8_t g; \
  uint8_t b; \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* uyvy_to_rgb_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  2
#define FUNC_NAME uyvy_to_rgb_16_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[1], src[0], src[2], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src[3], src[0], src[2], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[1]);

#define INIT \
  uint8_t r; \
  uint8_t g; \
  uint8_t b; \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* uyvy_to_bgr_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  2
#define FUNC_NAME uyvy_to_bgr_16_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[1], src[0], src[2], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src[3], src[0], src[2], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[1]);

#define INIT \
  uint8_t r; \
  uint8_t g; \
  uint8_t b; \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* uyvy_to_rgb_24_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 6
#define NUM_PIXELS  2
#define FUNC_NAME uyvy_to_rgb_24_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[1], src[0], src[2], dst[0], dst[1], dst[2]) \
  YUV_8_TO_RGB_24(src[3], src[0], src[2], dst[3], dst[4], dst[5])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* uyvy_to_rgb_48_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 6
#define NUM_PIXELS  2
#define FUNC_NAME uyvy_to_rgb_48_c
#define CONVERT  \
  YUV_8_TO_RGB_48(src[1], src[0], src[2], dst[0], dst[1], dst[2]) \
  YUV_8_TO_RGB_48(src[3], src[0], src[2], dst[3], dst[4], dst[5])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* uyvy_to_rgb_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 6
#define NUM_PIXELS  2
#define FUNC_NAME uyvy_to_rgb_float_c
#define CONVERT  \
  YUV_8_TO_RGB_FLOAT(src[1], src[0], src[2], dst[0], dst[1], dst[2]) \
  YUV_8_TO_RGB_FLOAT(src[3], src[0], src[2], dst[3], dst[4], dst[5])

#define INIT                                    \
  float i_tmp;

#include "../csp_packed_packed.h"

/* uyvy_to_rgb_24_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 6
#define NUM_PIXELS  2
#define FUNC_NAME uyvy_to_bgr_24_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[1], src[0], src[2], dst[2], dst[1], dst[0]) \
  YUV_8_TO_RGB_24(src[3], src[0], src[2], dst[5], dst[4], dst[3])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* uyvy_to_rgb_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 8
#define NUM_PIXELS  2
#define FUNC_NAME uyvy_to_rgb_32_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[1], src[0], src[2], dst[0], dst[1], dst[2]) \
  YUV_8_TO_RGB_24(src[3], src[0], src[2], dst[4], dst[5], dst[6])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* uyvy_to_bgr_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 8
#define NUM_PIXELS  2
#define FUNC_NAME uyvy_to_bgr_32_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[1], src[0], src[2], dst[2], dst[1], dst[0]) \
  YUV_8_TO_RGB_24(src[3], src[0], src[2], dst[6], dst[5], dst[4])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* uyvy_to_rgba_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 8
#define NUM_PIXELS  2
#define FUNC_NAME uyvy_to_rgba_32_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[1], src[0], src[2], dst[0], dst[1], dst[2]) \
  dst[3] = 0xFF;\
  YUV_8_TO_RGB_24(src[3], src[0], src[2], dst[4], dst[5], dst[6]) \
  dst[7] = 0xFF;

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* uyvy_to_rgba_64_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 8
#define NUM_PIXELS  2
#define FUNC_NAME uyvy_to_rgba_64_c
#define CONVERT  \
  YUV_8_TO_RGB_48(src[1], src[0], src[2], dst[0], dst[1], dst[2]) \
  dst[3] = 0xFFFF;\
  YUV_8_TO_RGB_48(src[3], src[0], src[2], dst[4], dst[5], dst[6]) \
  dst[7] = 0xFFFF;

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

/* uyvy_to_rgba_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 8
#define NUM_PIXELS  2
#define FUNC_NAME uyvy_to_rgba_float_c
#define CONVERT  \
  YUV_8_TO_RGB_FLOAT(src[1], src[0], src[2], dst[0], dst[1], dst[2]) \
  dst[3] = 1.0;\
  YUV_8_TO_RGB_FLOAT(src[3], src[0], src[2], dst[4], dst[5], dst[6]) \
  dst[7] = 1.0;

#define INIT \
  float i_tmp;

#include "../csp_packed_packed.h"



/*************************************************
  YUV420P ->
 *************************************************/

/* yuv_420_p_to_rgb_15_c */

#define FUNC_NAME yuv_420_p_to_rgb_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_420_p_to_bgr_15_c */

#define FUNC_NAME yuv_420_p_to_bgr_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_420_p_to_rgb_16_c */

#define FUNC_NAME yuv_420_p_to_rgb_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_420_p_to_bgr_16_c */

#define FUNC_NAME yuv_420_p_to_bgr_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_420_p_to_rgb_24_c */

#define FUNC_NAME yuv_420_p_to_rgb_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_420_p_to_rgb_48_c */

#define FUNC_NAME yuv_420_p_to_rgb_48_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUV_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_8_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_420_p_to_rgb_float_c */

#define FUNC_NAME yuv_420_p_to_rgb_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUV_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_8_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"

/* yuv_420_p_to_bgr_24_c */

#define FUNC_NAME yuv_420_p_to_bgr_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[5], dst[4], dst[3])


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_420_p_to_rgb_32_c */

#define FUNC_NAME yuv_420_p_to_rgb_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_420_p_to_bgr_32_c */

#define FUNC_NAME yuv_420_p_to_bgr_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[6], dst[5], dst[4])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_420_p_to_rgba_32_c */

#define FUNC_NAME yuv_420_p_to_rgba_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xff;\
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_420_p_to_rgba_64_c */

#define FUNC_NAME yuv_420_p_to_rgba_64_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUV_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xffff;\
  YUV_8_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xffff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"


/* yuv_420_p_to_rgba_float_c */

#define FUNC_NAME yuv_420_p_to_rgba_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUV_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 1.0;\
  YUV_8_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 1.0;

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"

/*************************************************
  YUV410P ->
 *************************************************/

/* yuv_410_p_to_rgb_15_c */

#define FUNC_NAME yuv_410_p_to_rgb_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    4
#define CHROMA_SUB    4
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[1]);\
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[2]);\
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[3]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_410_p_to_bgr_15_c */

#define FUNC_NAME yuv_410_p_to_bgr_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    4
#define CHROMA_SUB    4
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[1]);\
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[2]);\
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[3]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_410_p_to_rgb_16_c */

#define FUNC_NAME yuv_410_p_to_rgb_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    4
#define CHROMA_SUB    4
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[1]);\
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[2]);\
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[3]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_410_p_to_bgr_16_c */

#define FUNC_NAME yuv_410_p_to_bgr_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    4
#define CHROMA_SUB    4
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[1]); \
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[2]); \
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[3]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_410_p_to_rgb_24_c */

#define FUNC_NAME yuv_410_p_to_rgb_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   12
#define NUM_PIXELS    4
#define CHROMA_SUB    4
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])\
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], dst[6], dst[7], dst[8])\
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], dst[9], dst[10], dst[11])


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"


/* yuv_410_p_to_rgb_48_c */

#define FUNC_NAME yuv_410_p_to_rgb_48_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   12
#define NUM_PIXELS    4
#define CHROMA_SUB    4
#define CONVERT \
  YUV_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_8_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])\
  YUV_8_TO_RGB_48(src_y[2], src_u[0], src_v[0], dst[6], dst[7], dst[8])\
  YUV_8_TO_RGB_48(src_y[3], src_u[0], src_v[0], dst[9], dst[10], dst[11])


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_410_p_to_rgb_float_c */

#define FUNC_NAME yuv_410_p_to_rgb_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   12
#define NUM_PIXELS    4
#define CHROMA_SUB    4
#define CONVERT \
  YUV_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_8_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])\
  YUV_8_TO_RGB_FLOAT(src_y[2], src_u[0], src_v[0], dst[6], dst[7], dst[8])\
  YUV_8_TO_RGB_FLOAT(src_y[3], src_u[0], src_v[0], dst[9], dst[10], dst[11])

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"



/* yuv_410_p_to_bgr_24_c */

#define FUNC_NAME yuv_410_p_to_bgr_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   12
#define NUM_PIXELS    4
#define CHROMA_SUB    4
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[5], dst[4], dst[3]) \
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], dst[8], dst[7], dst[6]) \
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], dst[11], dst[10], dst[9])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_410_p_to_rgb_32_c */

#define FUNC_NAME yuv_410_p_to_rgb_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   16
#define NUM_PIXELS    4
#define CHROMA_SUB    4
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], dst[8], dst[9], dst[10]) \
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], dst[12], dst[13], dst[14])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_410_p_to_bgr_32_c */

#define FUNC_NAME yuv_410_p_to_bgr_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   16
#define NUM_PIXELS    4
#define CHROMA_SUB    4
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[6], dst[5], dst[4]) \
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], dst[10], dst[9], dst[8]) \
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], dst[14], dst[13], dst[12])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_410_p_to_rgba_32_c */

#define FUNC_NAME yuv_410_p_to_rgba_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   16
#define NUM_PIXELS    4
#define CHROMA_SUB    4
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xff;\
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xff; \
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], dst[8], dst[9], dst[10]) \
  dst[11] = 0xff; \
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], dst[12], dst[13], dst[14]) \
  dst[15] = 0xff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_410_p_to_rgba_64_c */

#define FUNC_NAME yuv_410_p_to_rgba_64_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   16
#define NUM_PIXELS    4
#define CHROMA_SUB    4
#define CONVERT \
  YUV_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xffff;\
  YUV_8_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xffff; \
  YUV_8_TO_RGB_48(src_y[2], src_u[0], src_v[0], dst[8], dst[9], dst[10]) \
  dst[11] = 0xffff; \
  YUV_8_TO_RGB_48(src_y[3], src_u[0], src_v[0], dst[12], dst[13], dst[14]) \
  dst[15] = 0xffff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"


/* yuv_410_p_to_rgba_float_c */

#define FUNC_NAME yuv_410_p_to_rgba_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   16
#define NUM_PIXELS    4
#define CHROMA_SUB    4
#define CONVERT \
  YUV_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 1.0;\
  YUV_8_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 1.0; \
  YUV_8_TO_RGB_FLOAT(src_y[2], src_u[0], src_v[0], dst[8], dst[9], dst[10]) \
  dst[11] = 1.0; \
  YUV_8_TO_RGB_FLOAT(src_y[3], src_u[0], src_v[0], dst[12], dst[13], dst[14]) \
  dst[15] = 1.0;

#define INIT  float i_tmp;

#include "../csp_planar_packed.h"



/*************************************************
  YUV422P ->
 *************************************************/

/* yuv_422_p_to_rgb_15_c */

#define FUNC_NAME yuv_422_p_to_rgb_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_422_p_to_bgr_15_c */

#define FUNC_NAME yuv_422_p_to_bgr_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_422_p_to_rgb_16_c */

#define FUNC_NAME yuv_422_p_to_rgb_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_422_p_to_bgr_16_c */

#define FUNC_NAME yuv_422_p_to_bgr_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_422_p_to_rgb_24_c */

#define FUNC_NAME yuv_422_p_to_rgb_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_422_p_to_rgb_48_c */

#define FUNC_NAME yuv_422_p_to_rgb_48_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_8_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_422_p_to_rgb_float_c */

#define FUNC_NAME yuv_422_p_to_rgb_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_8_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"


/* yuv_422_p_to_bgr_24_c */

#define FUNC_NAME yuv_422_p_to_bgr_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[5], dst[4], dst[3])


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_422_p_to_rgb_32_c */

#define FUNC_NAME yuv_422_p_to_rgb_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_422_p_to_bgr_32_c */

#define FUNC_NAME yuv_422_p_to_bgr_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[6], dst[5], dst[4])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_422_p_to_rgba_32_c */

#define FUNC_NAME yuv_422_p_to_rgba_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xff;\
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_422_p_to_rgba_64_c */

#define FUNC_NAME yuv_422_p_to_rgba_64_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xffff;\
  YUV_8_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xffff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_422_p_to_rgba_float_c */

#define FUNC_NAME yuv_422_p_to_rgba_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 1.0;                                                       \
  YUV_8_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 1.0;

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"

#endif // HQ

/*************************************************
  YUV422P (16 bit) ->
 *************************************************/

/* yuv_422_p_16_to_rgb_15_c */

#define FUNC_NAME yuv_422_p_16_to_rgb_15_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[0]); \
  YUV_16_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[1]);

#define INIT   int64_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_422_p_16_to_bgr_15_c */

#define FUNC_NAME yuv_422_p_16_to_bgr_15_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[0]); \
  YUV_16_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[1]);

#define INIT   int64_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_422_p_16_to_rgb_16_c */

#define FUNC_NAME yuv_422_p_16_to_rgb_16_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[0]); \
  YUV_16_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[1]);

#define INIT   int64_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_422_p_16_to_bgr_16_c */

#define FUNC_NAME yuv_422_p_16_to_bgr_16_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[0]); \
  YUV_16_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[1]);

#define INIT   int64_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_422_p_16_to_rgb_24_c */

#define FUNC_NAME yuv_422_p_16_to_rgb_24_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_16_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])


#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

#ifndef HQ

/* yuv_422_p_16_to_rgb_48_c */

#define FUNC_NAME yuv_422_p_16_to_rgb_48_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_16_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])

#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_422_p_16_to_rgb_float_c */

#define FUNC_NAME yuv_422_p_16_to_rgb_float_c
#define IN_TYPE uint16_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_16_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"

#endif // !HQ
/* yuv_422_p_16_to_bgr_24_c */

#define FUNC_NAME yuv_422_p_16_to_bgr_24_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUV_16_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[5], dst[4], dst[3])


#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_422_p_16_to_rgb_32_c */

#define FUNC_NAME yuv_422_p_16_to_rgb_32_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  YUV_16_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6])

#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_422_p_16_to_bgr_32_c */

#define FUNC_NAME yuv_422_p_16_to_bgr_32_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUV_16_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[6], dst[5], dst[4])

#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_422_p_16_to_rgba_32_c */

#define FUNC_NAME yuv_422_p_16_to_rgba_32_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xff;\
  YUV_16_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xff;

#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

#ifndef HQ

/* yuv_422_p_16_to_rgba_64_c */

#define FUNC_NAME yuv_422_p_16_to_rgba_64_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xffff;\
  YUV_16_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xffff;

#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_422_p_16_to_rgba_float_c */

#define FUNC_NAME yuv_422_p_16_to_rgba_float_c
#define IN_TYPE uint16_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 1.0;                                                       \
  YUV_16_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 1.0;

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"

/*************************************************
  YUV411P ->
 *************************************************/

/* yuv_411_p_to_rgb_15_c */

#define FUNC_NAME yuv_411_p_to_rgb_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    4
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[1]);\
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[2]); \
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[3]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_411_p_to_bgr_15_c */

#define FUNC_NAME yuv_411_p_to_bgr_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    4
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[1]);\
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[2]); \
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[3]);
  

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_411_p_to_rgb_16_c */

#define FUNC_NAME yuv_411_p_to_rgb_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    4
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[1]);\
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[2]); \
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[3]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_411_p_to_bgr_16_c */

#define FUNC_NAME yuv_411_p_to_bgr_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    4
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[0]); \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[1]);\
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[2]); \
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[3]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_411_p_to_rgb_24_c */

#define FUNC_NAME yuv_411_p_to_rgb_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   12
#define NUM_PIXELS    4
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])\
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], dst[6], dst[7], dst[8])\
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], dst[9], dst[10], dst[11])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_411_p_to_rgb_48_c */

#define FUNC_NAME yuv_411_p_to_rgb_48_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   12
#define NUM_PIXELS    4
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_8_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])\
  YUV_8_TO_RGB_48(src_y[2], src_u[0], src_v[0], dst[6], dst[7], dst[8])\
  YUV_8_TO_RGB_48(src_y[3], src_u[0], src_v[0], dst[9], dst[10], dst[11])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_411_p_to_rgb_float_c */

#define FUNC_NAME yuv_411_p_to_rgb_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   12
#define NUM_PIXELS    4
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUV_8_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])\
  YUV_8_TO_RGB_FLOAT(src_y[2], src_u[0], src_v[0], dst[6], dst[7], dst[8])\
  YUV_8_TO_RGB_FLOAT(src_y[3], src_u[0], src_v[0], dst[9], dst[10], dst[11])

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"


/* yuv_411_p_to_bgr_24_c */

#define FUNC_NAME yuv_411_p_to_bgr_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   12
#define NUM_PIXELS    4
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[5], dst[4], dst[3]) \
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], dst[8], dst[7], dst[6]) \
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], dst[11], dst[10], dst[9])



#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_411_p_to_rgb_32_c */

#define FUNC_NAME yuv_411_p_to_rgb_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   16
#define NUM_PIXELS    4
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], dst[8], dst[9], dst[10]) \
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], dst[12], dst[13], dst[14])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_411_p_to_bgr_32_c */

#define FUNC_NAME yuv_411_p_to_bgr_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   16
#define NUM_PIXELS    4
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[6], dst[5], dst[4]) \
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], dst[10], dst[9], dst[8]) \
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], dst[14], dst[13], dst[12])


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_411_p_to_rgba_32_c */

#define FUNC_NAME yuv_411_p_to_rgba_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   16
#define NUM_PIXELS    4
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xff;\
  YUV_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xff;\
  YUV_8_TO_RGB_24(src_y[2], src_u[0], src_v[0], dst[8], dst[9], dst[10]) \
  dst[11] = 0xff;\
  YUV_8_TO_RGB_24(src_y[3], src_u[0], src_v[0], dst[12], dst[13], dst[14]) \
  dst[15] = 0xff;


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_411_p_to_rgba_64_c */

#define FUNC_NAME yuv_411_p_to_rgba_64_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   16
#define NUM_PIXELS    4
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xffff;\
  YUV_8_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xffff;\
  YUV_8_TO_RGB_48(src_y[2], src_u[0], src_v[0], dst[8], dst[9], dst[10]) \
  dst[11] = 0xffff;\
  YUV_8_TO_RGB_48(src_y[3], src_u[0], src_v[0], dst[12], dst[13], dst[14]) \
  dst[15] = 0xffff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_411_p_to_rgba_float_c */

#define FUNC_NAME yuv_411_p_to_rgba_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  4
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   16
#define NUM_PIXELS    4
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 1.0;                                                    \
  YUV_8_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 1.0;\
  YUV_8_TO_RGB_FLOAT(src_y[2], src_u[0], src_v[0], dst[8], dst[9], dst[10]) \
  dst[11] = 1.0;\
  YUV_8_TO_RGB_FLOAT(src_y[3], src_u[0], src_v[0], dst[12], dst[13], dst[14]) \
  dst[15] = 1.0;

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"


/*************************************************
  YUV444P ->
 *************************************************/

/* yuv_444_p_to_rgb_15_c */

#define FUNC_NAME yuv_444_p_to_rgb_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   1
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[0]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_444_p_to_bgr_15_c */

#define FUNC_NAME yuv_444_p_to_bgr_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   1
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[0]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_444_p_to_rgb_16_c */

#define FUNC_NAME yuv_444_p_to_rgb_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   1
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[0]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_444_p_to_bgr_16_c */

#define FUNC_NAME yuv_444_p_to_bgr_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   1
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[0]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_444_p_to_rgb_24_c */

#define FUNC_NAME yuv_444_p_to_rgb_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   3
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_444_p_to_rgb_48_c */

#define FUNC_NAME yuv_444_p_to_rgb_48_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   3
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_444_p_to_rgb_float_c */

#define FUNC_NAME yuv_444_p_to_rgb_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   3
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"


/* yuv_444_p_to_bgr_24_c */

#define FUNC_NAME yuv_444_p_to_bgr_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   3
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0])


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_444_p_to_rgb_32_c */

#define FUNC_NAME yuv_444_p_to_rgb_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_444_p_to_bgr_32_c */

#define FUNC_NAME yuv_444_p_to_bgr_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_444_p_to_rgba_32_c */

#define FUNC_NAME yuv_444_p_to_rgba_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_444_p_to_rgba_64_c */

#define FUNC_NAME yuv_444_p_to_rgba_64_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xffff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_444_p_to_rgba_float_c */

#define FUNC_NAME yuv_444_p_to_rgba_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 1.0;

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"

#endif // !HQ

/*************************************************
  YUV444P (16 bit) ->
 *************************************************/

/* yuv_444_p_16_to_rgb_15_c */

#define FUNC_NAME yuv_444_p_16_to_rgb_15_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   1
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[0]);

#define INIT   int64_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_444_p_16_to_bgr_15_c */

#define FUNC_NAME yuv_444_p_16_to_bgr_15_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   1
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[0]);

#define INIT   int64_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_444_p_16_to_rgb_16_c */

#define FUNC_NAME yuv_444_p_16_to_rgb_16_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   1
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[0]);

#define INIT   int64_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_444_p_16_to_bgr_16_c */

#define FUNC_NAME yuv_444_p_16_to_bgr_16_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   1
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[0]);

#define INIT   int64_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuv_444_p_16_to_rgb_24_c */

#define FUNC_NAME yuv_444_p_16_to_rgb_24_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   3
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])

#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

#ifndef HQ

/* yuv_444_p_16_to_rgb_48_c */

#define FUNC_NAME yuv_444_p_16_to_rgb_48_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   3
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])

#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_444_p_16_to_rgb_float_c */

#define FUNC_NAME yuv_444_p_16_to_rgb_float_c
#define IN_TYPE uint16_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   3
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"

#endif // HQ

/* yuv_444_p_16_to_bgr_24_c */

#define FUNC_NAME yuv_444_p_16_to_bgr_24_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   3
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0])


#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_444_p_16_to_rgb_32_c */

#define FUNC_NAME yuv_444_p_16_to_rgb_32_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])

#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_444_p_16_to_bgr_32_c */

#define FUNC_NAME yuv_444_p_16_to_bgr_32_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0])

#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_444_p_16_to_rgba_32_c */

#define FUNC_NAME yuv_444_p_16_to_rgba_32_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xff;

#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

#ifndef HQ

/* yuv_444_p_16_to_rgba_64_c */

#define FUNC_NAME yuv_444_p_16_to_rgba_64_c
#define IN_TYPE uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xffff;

#define INIT   int64_t i_tmp;

#include "../csp_planar_packed.h"

/* yuv_444_p_16_to_rgba_float_c */

#define FUNC_NAME yuv_444_p_16_to_rgba_float_c
#define IN_TYPE uint16_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUV_16_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 1.0;

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"


/* JPEG */

/*************************************************
  YUVJ 420 P ->
 *************************************************/

/* yuvj_420_p_to_rgb_15_c */

#define FUNC_NAME yuvj_420_p_to_rgb_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[0]); \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuvj_420_p_to_bgr_15_c */

#define FUNC_NAME yuvj_420_p_to_bgr_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[0]); \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuvj_420_p_to_rgb_16_c */

#define FUNC_NAME yuvj_420_p_to_rgb_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[0]); \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuvj_420_p_to_bgr_16_c */

#define FUNC_NAME yuvj_420_p_to_bgr_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[0]); \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuvj_420_p_to_rgb_24_c */

#define FUNC_NAME yuvj_420_p_to_rgb_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_420_p_to_rgb_48_c */

#define FUNC_NAME yuvj_420_p_to_rgb_48_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUVJ_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUVJ_8_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"


/* yuvj_420_p_to_rgb_float_c */

#define FUNC_NAME yuvj_420_p_to_rgb_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUVJ_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUVJ_8_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"


/* yuvj_420_p_to_bgr_24_c */

#define FUNC_NAME yuvj_420_p_to_bgr_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[5], dst[4], dst[3])


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_420_p_to_rgb_32_c */

#define FUNC_NAME yuvj_420_p_to_rgb_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_420_p_to_bgr_32_c */

#define FUNC_NAME yuvj_420_p_to_bgr_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[6], dst[5], dst[4])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_420_p_to_rgba_32_c */

#define FUNC_NAME yuvj_420_p_to_rgba_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xff;\
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_420_p_to_rgba_64_c */

#define FUNC_NAME yuvj_420_p_to_rgba_64_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUVJ_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xffff;\
  YUVJ_8_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xffff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_420_p_to_rgba_float_c */

#define FUNC_NAME yuvj_420_p_to_rgba_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    2
#define CONVERT \
  YUVJ_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 1.0;\
  YUVJ_8_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 1.0;

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"


/*************************************************
  YUVJ 422 P ->
 *************************************************/

/* yuvj_422_p_to_rgb_15_c */

#define FUNC_NAME yuvj_422_p_to_rgb_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[0]); \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuvj_422_p_to_bgr_15_c */

#define FUNC_NAME yuvj_422_p_to_bgr_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[0]); \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuvj_422_p_to_rgb_16_c */

#define FUNC_NAME yuvj_422_p_to_rgb_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[0]); \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuvj_422_p_to_bgr_16_c */

#define FUNC_NAME yuvj_422_p_to_bgr_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   2
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[0]); \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[1]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuvj_422_p_to_rgb_24_c */

#define FUNC_NAME yuvj_422_p_to_rgb_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_422_p_to_rgb_48_c */

#define FUNC_NAME yuvj_422_p_to_rgb_48_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUVJ_8_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_422_p_to_rgb_float_c */

#define FUNC_NAME yuvj_422_p_to_rgb_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])\
  YUVJ_8_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[3], dst[4], dst[5])

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_422_p_to_bgr_24_c */

#define FUNC_NAME yuvj_422_p_to_bgr_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   6
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[5], dst[4], dst[3])


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_422_p_to_rgb_32_c */

#define FUNC_NAME yuvj_422_p_to_rgb_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_422_p_to_bgr_32_c */

#define FUNC_NAME yuvj_422_p_to_bgr_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0]) \
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[6], dst[5], dst[4])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_422_p_to_rgba_32_c */

#define FUNC_NAME yuvj_422_p_to_rgba_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xff;\
  YUVJ_8_TO_RGB_24(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_422_p_to_rgba_64_c */

#define FUNC_NAME yuvj_422_p_to_rgba_64_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xffff;\
  YUVJ_8_TO_RGB_48(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 0xffff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_422_p_to_rgba_float_c */

#define FUNC_NAME yuvj_422_p_to_rgba_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  2
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   8
#define NUM_PIXELS    2
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 1.0;\
  YUVJ_8_TO_RGB_FLOAT(src_y[1], src_u[0], src_v[0], dst[4], dst[5], dst[6]) \
  dst[7] = 1.0;

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"


/*************************************************
  YUVJ 444 P ->
 *************************************************/

/* yuvj_444_p_to_rgb_15_c */

#define FUNC_NAME yuvj_444_p_to_rgb_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   1
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB15(r, g, b, dst[0]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuvj_444_p_to_bgr_15_c */

#define FUNC_NAME yuvj_444_p_to_bgr_15_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   1
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR15(r, g, b, dst[0]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuvj_444_p_to_rgb_16_c */

#define FUNC_NAME yuvj_444_p_to_rgb_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   1
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_RGB16(r, g, b, dst[0]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuvj_444_p_to_bgr_16_c */

#define FUNC_NAME yuvj_444_p_to_bgr_16_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   1
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], r, g, b) \
  PACK_8_TO_BGR16(r, g, b, dst[0]);

#define INIT   int32_t i_tmp; \
  uint8_t r; \
  uint8_t g; \
  uint8_t b;

#include "../csp_planar_packed.h"

/* yuvj_444_p_to_rgb_24_c */

#define FUNC_NAME yuvj_444_p_to_rgb_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   3
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_444_p_to_rgb_48_c */

#define FUNC_NAME yuvj_444_p_to_rgb_48_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   3
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_444_p_to_rgb_float_c */

#define FUNC_NAME yuvj_444_p_to_rgb_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   3
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_444_p_to_bgr_24_c */

#define FUNC_NAME yuvj_444_p_to_bgr_24_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   3
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0])


#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_444_p_to_rgb_32_c */

#define FUNC_NAME yuvj_444_p_to_rgb_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_444_p_to_bgr_32_c */

#define FUNC_NAME yuvj_444_p_to_bgr_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[2], dst[1], dst[0])

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_444_p_to_rgba_32_c */

#define FUNC_NAME yuvj_444_p_to_rgba_32_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_24(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_444_p_to_rgba_64_c */

#define FUNC_NAME yuvj_444_p_to_rgba_64_c
#define IN_TYPE uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_48(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 0xffff;

#define INIT   int32_t i_tmp;

#include "../csp_planar_packed.h"

/* yuvj_444_p_to_rgba_float_c */

#define FUNC_NAME yuvj_444_p_to_rgba_float_c
#define IN_TYPE uint8_t
#define OUT_TYPE float
#define IN_ADVANCE_Y  1
#define IN_ADVANCE_UV 1
#define OUT_ADVANCE   4
#define NUM_PIXELS    1
#define CHROMA_SUB    1
#define CONVERT \
  YUVJ_8_TO_RGB_FLOAT(src_y[0], src_u[0], src_v[0], dst[0], dst[1], dst[2]) \
  dst[3] = 1.0;

#define INIT   float i_tmp;

#include "../csp_planar_packed.h"

/*************************************************
  YUVA -> RGBA
 *************************************************/

/* yuva_32_to_rgba_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgba_32_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], dst[0], dst[1], dst[2]) \
    dst[3] = src[3];
#define INIT int32_t i_tmp;

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_rgba_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgba_32_c
#define CONVERT  \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], dst[0], dst[1], dst[2]) \
  RGB_16_TO_8(src[3], dst[3]);

#ifdef HQ
#define INIT int32_t round_tmp; int64_t i_tmp;
#else
#define INIT int64_t i_tmp;
#endif


#include "../csp_packed_packed.h"

/* yuva_float_to_rgba_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgba_32_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], dst[0], dst[1], dst[2]) \
  RGB_FLOAT_TO_8(src[3], dst[3]);

#define INIT float i_tmp;

#include "../csp_packed_packed.h"

/* yuv_float_to_rgba_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_rgba_32_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], dst[0], dst[1], dst[2]) \
  dst[3] = 0xff;

#define INIT float i_tmp;

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_rgba_64_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgba_64_c
#define CONVERT  \
  YUV_8_TO_RGB_48(src[0], src[1], src[2], dst[0], dst[1], dst[2]) \
    dst[3] = RGB_8_TO_16(src[3]);
#define INIT int32_t i_tmp;

#include "../csp_packed_packed.h"

/* yuva_64_to_rgba_64_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgba_64_c
#define CONVERT  \
  YUV_16_TO_RGB_48(src[0], src[1], src[2], dst[0], dst[1], dst[2]) \
  dst[3] = src[3];
#define INIT int64_t i_tmp;

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_float_to_rgba_64_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgba_64_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_48(src[0], src[1], src[2], dst[0], dst[1], dst[2]) \
  RGB_FLOAT_TO_16(src[3], dst[3]);
#define INIT float i_tmp;

#include "../csp_packed_packed.h"

/* yuv_float_to_rgba_64_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_rgba_64_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_48(src[0], src[1], src[2], dst[0], dst[1], dst[2]) \
  dst[3] = 0xFFFF;

#define INIT float i_tmp;

#include "../csp_packed_packed.h"


#ifndef HQ

/* yuva_32_to_rgba_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgba_float_c
#define CONVERT  \
  YUV_8_TO_RGB_FLOAT(src[0], src[1], src[2], dst[0], dst[1], dst[2]) \
    dst[3] = RGB_8_TO_FLOAT(src[3]);
#define INIT float i_tmp;

#include "../csp_packed_packed.h"

/* yuva_64_to_rgba_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgba_float_c
#define CONVERT  \
  YUV_16_TO_RGB_FLOAT(src[0], src[1], src[2], dst[0], dst[1], dst[2]) \
    dst[3] = RGB_16_TO_FLOAT(src[3]);
#define INIT float i_tmp;

#include "../csp_packed_packed.h"

/* yuva_float_to_rgba_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgba_float_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_FLOAT(src[0], src[1], src[2], dst[0], dst[1], dst[2]) \
    dst[3] = src[3];
#define INIT float i_tmp;

#include "../csp_packed_packed.h"

/* yuv_float_to_rgba_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_rgba_float_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_FLOAT(src[0], src[1], src[2], dst[0], dst[1], dst[2]) \
    dst[3] = 1.0;

#define INIT float i_tmp;

#include "../csp_packed_packed.h"

/*************************************************
  YUVA -> RGB
 *************************************************/

/* yuva_32_to_rgb_15_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgb_15_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  RGBA_32_TO_RGB_24(r_tmp1, g_tmp1, b_tmp1, src[3], r_tmp2, g_tmp2, b_tmp2)\
  PACK_8_TO_RGB15(r_tmp2, g_tmp2, b_tmp2, *dst);

#define INIT \
  int32_t i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#endif // !HQ

/* Yuva_64_to_rgb_15_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgb_15_c
#define CONVERT  \
  RGB_16_TO_8(src[3], a_tmp); \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1) \
  RGBA_32_TO_RGB_24(r_tmp1, g_tmp1, b_tmp1, a_tmp, r_tmp2, g_tmp2, b_tmp2)\
  PACK_8_TO_RGB15(r_tmp2, g_tmp2, b_tmp2, *dst);

#ifdef HQ
#define INIT \
  int64_t i_tmp; \
  uint32_t a_tmp; \
  int round_tmp;\
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32
#else
#define INIT \
  int64_t i_tmp; \
  uint32_t a_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32
#endif

#include "../csp_packed_packed.h"

/* yuva_float_to_rgb_15_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgb_15_c
#define CONVERT  \
  RGB_FLOAT_TO_8(src[3], a_tmp);\
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  RGBA_32_TO_RGB_24(r_tmp1, g_tmp1, b_tmp1, a_tmp, r_tmp2, g_tmp2, b_tmp2)\
  PACK_8_TO_RGB15(r_tmp2, g_tmp2, b_tmp2, *dst);

#define INIT \
  uint32_t a_tmp;\
  float i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_bgr_15_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_bgr_15_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  RGBA_32_TO_RGB_24(r_tmp1, g_tmp1, b_tmp1, src[3], r_tmp2, g_tmp2, b_tmp2)\
  PACK_8_TO_BGR15(r_tmp2, g_tmp2, b_tmp2, *dst);

#define INIT \
  int32_t i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_bgr_15_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_bgr_15_c
#define CONVERT  \
  RGB_16_TO_8(src[3], a_tmp); \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1) \
  RGBA_32_TO_RGB_24(r_tmp1, g_tmp1, b_tmp1, a_tmp, r_tmp2, g_tmp2, b_tmp2)\
  PACK_8_TO_BGR15(r_tmp2, g_tmp2, b_tmp2, *dst);

#ifdef HQ
#define INIT \
  int64_t i_tmp; \
  uint8_t a_tmp; \
  int round_tmp;\
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32
#else
#define INIT \
  int64_t i_tmp; \
  uint32_t a_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32
#endif

#include "../csp_packed_packed.h"

/* yuva_float_to_bgr_15_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_bgr_15_c
#define CONVERT  \
  RGB_FLOAT_TO_8(src[3], a_tmp);\
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  RGBA_32_TO_RGB_24(r_tmp1, g_tmp1, b_tmp1, a_tmp, r_tmp2, g_tmp2, b_tmp2)\
  PACK_8_TO_BGR15(r_tmp2, g_tmp2, b_tmp2, *dst);

#define INIT \
  int a_tmp;\
  float i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_rgb_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgb_16_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  RGBA_32_TO_RGB_24(r_tmp1, g_tmp1, b_tmp1, src[3], r_tmp2, g_tmp2, b_tmp2)\
  PACK_8_TO_RGB16(r_tmp2, g_tmp2, b_tmp2, *dst);

#define INIT \
  int32_t i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_rgb_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgb_16_c
#define CONVERT  \
  RGB_16_TO_8(src[3], a_tmp); \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1) \
  RGBA_32_TO_RGB_24(r_tmp1, g_tmp1, b_tmp1, a_tmp, r_tmp2, g_tmp2, b_tmp2)\
  PACK_8_TO_RGB16(r_tmp2, g_tmp2, b_tmp2, *dst);

#ifdef HQ
#define INIT \
  int64_t i_tmp; \
  uint8_t a_tmp; \
  int round_tmp;\
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32
#else
#define INIT \
  int64_t i_tmp; \
  uint32_t a_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32
#endif

#include "../csp_packed_packed.h"

/* yuva_float_to_rgb_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgb_16_c
#define CONVERT  \
  RGB_FLOAT_TO_8(src[3], a_tmp);\
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  RGBA_32_TO_RGB_24(r_tmp1, g_tmp1, b_tmp1, a_tmp, r_tmp2, g_tmp2, b_tmp2)\
  PACK_8_TO_RGB16(r_tmp2, g_tmp2, b_tmp2, *dst);

#define INIT \
  int a_tmp;\
  float i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_bgr_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_bgr_16_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  RGBA_32_TO_RGB_24(r_tmp1, g_tmp1, b_tmp1, src[3], r_tmp2, g_tmp2, b_tmp2)\
  PACK_8_TO_BGR16(r_tmp2, g_tmp2, b_tmp2, *dst);

#define INIT \
  int32_t i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_bgr_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_bgr_16_c
#define CONVERT  \
  RGB_16_TO_8(src[3], a_tmp); \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1) \
  RGBA_32_TO_RGB_24(r_tmp1, g_tmp1, b_tmp1, a_tmp, r_tmp2, g_tmp2, b_tmp2)\
  PACK_8_TO_BGR16(r_tmp2, g_tmp2, b_tmp2, *dst);

#ifdef HQ
#define INIT \
  int64_t i_tmp; \
  uint8_t a_tmp; \
  int round_tmp;\
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32
#else
#define INIT \
  int64_t i_tmp; \
  uint32_t a_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32
#endif

#include "../csp_packed_packed.h"

/* yuva_float_to_bgr_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_bgr_16_c
#define CONVERT  \
  RGB_FLOAT_TO_8(src[3], a_tmp);\
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  RGBA_32_TO_RGB_24(r_tmp1, g_tmp1, b_tmp1, a_tmp, r_tmp2, g_tmp2, b_tmp2)\
  PACK_8_TO_BGR16(r_tmp2, g_tmp2, b_tmp2, *dst);

#define INIT \
  int a_tmp;\
  float i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \
  int r_tmp2; \
  int g_tmp2; \
  int b_tmp2; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_rgb_24_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgb_24_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGBA_32_TO_RGB_24(r_tmp, g_tmp, b_tmp, src[3], dst[0], dst[1], dst[2]) \

#define INIT \
  int32_t i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_rgb_24_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgb_24_c
#define CONVERT  \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGB_16_TO_8(src[3], a_tmp); \
  RGBA_32_TO_RGB_24(r_tmp, g_tmp, b_tmp, a_tmp, dst[0], dst[1], dst[2]) \

#ifndef HQ
#define INIT \
  int64_t i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  int a_tmp; \
  INIT_RGBA_32
#else
#define INIT \
  int64_t i_tmp; \
  int round_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  int a_tmp; \
  INIT_RGBA_32
#endif

#include "../csp_packed_packed.h"

/* yuva_float_to_rgb_24_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgb_24_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGB_FLOAT_TO_8(src[3], a_tmp); \
  RGBA_32_TO_RGB_24(r_tmp, g_tmp, b_tmp, a_tmp, dst[0], dst[1], dst[2]) \

#define INIT \
  float i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  int a_tmp; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"


#ifndef HQ

/* yuva_32_to_bgr_24_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_bgr_24_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGBA_32_TO_RGB_24(r_tmp, g_tmp, b_tmp, src[3], dst[2], dst[1], dst[0]) \

#define INIT \
  int32_t i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_bgr_24_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_bgr_24_c
#define CONVERT  \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGB_16_TO_8(src[3], a_tmp); \
  RGBA_32_TO_RGB_24(r_tmp, g_tmp, b_tmp, a_tmp, dst[2], dst[1], dst[0]) \

#ifndef HQ
#define INIT \
  int64_t i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  int a_tmp; \
  INIT_RGBA_32
#else
#define INIT \
  int64_t i_tmp; \
  int round_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  int a_tmp; \
  INIT_RGBA_32
#endif


#include "../csp_packed_packed.h"

/* yuva_float_to_bgr_24_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_bgr_24_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGB_FLOAT_TO_8(src[3], a_tmp); \
  RGBA_32_TO_RGB_24(r_tmp, g_tmp, b_tmp, a_tmp, dst[2], dst[1], dst[0]) \

#define INIT \
  float i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  int a_tmp; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#ifndef HQ


/* yuva_32_to_rgb_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgb_32_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGBA_32_TO_RGB_24(r_tmp, g_tmp, b_tmp, src[3], dst[0], dst[1], dst[2]) \

#define INIT \
  int32_t i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_rgb_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgb_32_c
#define CONVERT  \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGB_16_TO_8(src[3], a_tmp); \
  RGBA_32_TO_RGB_24(r_tmp, g_tmp, b_tmp, a_tmp, dst[0], dst[1], dst[2]) \

#ifndef HQ
#define INIT \
  int64_t i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  int a_tmp; \
  INIT_RGBA_32
#else
#define INIT \
  int64_t i_tmp; \
  int round_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  int a_tmp; \
  INIT_RGBA_32
#endif

#include "../csp_packed_packed.h"

/* yuva_float_to_rgb_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgb_32_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGB_FLOAT_TO_8(src[3], a_tmp); \
  RGBA_32_TO_RGB_24(r_tmp, g_tmp, b_tmp, a_tmp, dst[0], dst[1], dst[2]) \

#define INIT \
  float i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  int a_tmp; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_bgr_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_bgr_32_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGBA_32_TO_RGB_24(r_tmp, g_tmp, b_tmp, src[3], dst[2], dst[1], dst[0]) \

#define INIT \
  int32_t i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_bgr_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_bgr_32_c
#define CONVERT  \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGB_16_TO_8(src[3], a_tmp); \
  RGBA_32_TO_RGB_24(r_tmp, g_tmp, b_tmp, a_tmp, dst[2], dst[1], dst[0]) \

#ifndef HQ
#define INIT \
  int64_t i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  int a_tmp; \
  INIT_RGBA_32
#else
#define INIT \
  int64_t i_tmp; \
  int round_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  int a_tmp; \
  INIT_RGBA_32
#endif

#include "../csp_packed_packed.h"

/* yuva_float_to_bgr_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_bgr_32_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGB_FLOAT_TO_8(src[3], a_tmp); \
  RGBA_32_TO_RGB_24(r_tmp, g_tmp, b_tmp, a_tmp, dst[2], dst[1], dst[0]) \

#define INIT \
  float i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  int a_tmp; \
  INIT_RGBA_32

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_rgb_48_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgb_48_c
#define CONVERT  \
  a_tmp = RGB_8_TO_16(src[3]);\
  YUV_8_TO_RGB_48(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
    RGBA_64_TO_RGB_48(r_tmp, g_tmp, b_tmp, a_tmp, dst[0], dst[1], dst[2]) \
    
#define INIT \
  int32_t i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  int a_tmp; \
  INIT_RGBA_64

#include "../csp_packed_packed.h"

/* yuva_64_to_rgb_48_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgb_48_c
#define CONVERT  \
  YUV_16_TO_RGB_48(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGBA_64_TO_RGB_48(r_tmp, g_tmp, b_tmp, src[3], dst[0], dst[1], dst[2]) \
    
#define INIT \
  int64_t i_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  INIT_RGBA_64

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_float_to_rgb_48_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgb_48_c
#define CONVERT  \
  RGB_FLOAT_TO_16(src[3], a_tmp); \
  YUV_FLOAT_TO_RGB_48(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGBA_64_TO_RGB_48(r_tmp, g_tmp, b_tmp, a_tmp, dst[0], dst[1], dst[2])

#define INIT \
  float i_tmp; \
  int a_tmp; \
  int r_tmp; \
  int g_tmp; \
  int b_tmp; \
  INIT_RGBA_64

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_rgb_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgb_float_c
#define CONVERT  \
  a_tmp = RGB_8_TO_FLOAT(src[3]);\
  YUV_8_TO_RGB_FLOAT(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
    RGBA_FLOAT_TO_RGB_FLOAT(r_tmp, g_tmp, b_tmp, a_tmp, dst[0], dst[1], dst[2]) \
    
#define INIT \
  float i_tmp; \
  float r_tmp; \
  float g_tmp; \
  float b_tmp; \
  float a_tmp; \
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

/* yuva_64_to_rgb_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgb_float_c
#define CONVERT  \
  a_tmp = RGB_16_TO_FLOAT(src[3]);\
  YUV_16_TO_RGB_FLOAT(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
    RGBA_FLOAT_TO_RGB_FLOAT(r_tmp, g_tmp, b_tmp, a_tmp, dst[0], dst[1], dst[2]) \
    
#define INIT \
  float i_tmp; \
  float r_tmp; \
  float g_tmp; \
  float b_tmp; \
  float a_tmp; \
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

/* yuva_float_to_rgb_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgb_float_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_FLOAT(src[0], src[1], src[2], r_tmp, g_tmp, b_tmp)          \
  RGBA_FLOAT_TO_RGB_FLOAT(r_tmp, g_tmp, b_tmp, src[3], dst[0], dst[1], dst[2]) \
    
#define INIT \
  float i_tmp; \
  float r_tmp; \
  float g_tmp; \
  float b_tmp; \
  INIT_RGBA_FLOAT

#include "../csp_packed_packed.h"

/*************************************************
  YUVA -> RGB (No alpha)
 *************************************************/

/* yuva_32_to_rgb_15_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgb_15_ia_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_RGB15(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  int32_t i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_rgb_15_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgb_15_ia_c
#define CONVERT  \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_RGB15(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  int64_t i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"

/* yuva_float_to_rgb_15_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgb_15_ia_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_RGB15(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  float i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"

/* yuv_float_to_rgb_15_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_rgb_15_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1) \
  PACK_8_TO_RGB15(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  float i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_bgr_15_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_bgr_15_ia_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_BGR15(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  int32_t i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"


#endif // !HQ

/* yuva_64_to_bgr_15_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_bgr_15_ia_c
#define CONVERT  \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_BGR15(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  int64_t i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"

/* yuva_float_to_bgr_15_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_bgr_15_ia_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_BGR15(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  float i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"

/* yuv_float_to_bgr_15_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_bgr_15_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_BGR15(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  float i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"

#ifndef HQ


/* yuva_32_to_rgb_16_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgb_16_ia_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_RGB16(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  int32_t i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_rgb_16_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgb_16_ia_c
#define CONVERT  \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_RGB16(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  int64_t i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"

/* yuva_float_to_rgb_16_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgb_16_ia_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_RGB16(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  float i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"

/* yuv_float_to_rgb_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_rgb_16_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_RGB16(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  float i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"


#ifndef HQ


/* yuva_32_to_bgr_16_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_bgr_16_ia_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_BGR16(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  int32_t i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"


#endif // !HQ

/* yuva_64_to_bgr_16_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_bgr_16_ia_c
#define CONVERT  \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_BGR16(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  int64_t i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"

/* yuva_float_to_bgr_16_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_bgr_16_ia_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_BGR16(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  float i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"

/* yuv_float_to_bgr_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_bgr_16_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], r_tmp1, g_tmp1, b_tmp1)        \
  PACK_8_TO_BGR16(r_tmp1, g_tmp1, b_tmp1, *dst);

#define INIT \
  float i_tmp; \
  int r_tmp1; \
  int g_tmp1; \
  int b_tmp1; \

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_rgb_24_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgb_24_ia_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], dst[0], dst[1], dst[2])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_rgb_24_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgb_24_ia_c
#define CONVERT  \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], dst[0], dst[1], dst[2])

#define INIT \
  int64_t i_tmp;

#include "../csp_packed_packed.h"

/* yuva_float_to_rgb_24_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgb_24_ia_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], dst[0], dst[1], dst[2])

#define INIT \
  float i_tmp;

#include "../csp_packed_packed.h"

/* yuv_float_to_rgb_24_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_rgb_24_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], dst[0], dst[1], dst[2])

#define INIT \
  float i_tmp;

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_bgr_24_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_bgr_24_ia_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], dst[2], dst[1], dst[0])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_bgr_24_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_bgr_24_ia_c
#define CONVERT  \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], dst[2], dst[1], dst[0])

#define INIT \
  int64_t i_tmp;

#include "../csp_packed_packed.h"

/* yuva_float_to_bgr_24_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_bgr_24_ia_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], dst[2], dst[1], dst[0])

#define INIT \
  float i_tmp;

#include "../csp_packed_packed.h"

/* yuv_float_to_bgr_24_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_bgr_24_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], dst[2], dst[1], dst[0])

#define INIT \
  float i_tmp;

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_rgb_32_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgb_32_ia_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], dst[0], dst[1], dst[2])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_rgb_32_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgb_32_ia_c
#define CONVERT  \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], dst[0], dst[1], dst[2])

#define INIT \
  int64_t i_tmp;

#include "../csp_packed_packed.h"

/* yuva_float_to_rgb_32_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgb_32_ia_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], dst[0], dst[1], dst[2])

#define INIT \
  float i_tmp;

#include "../csp_packed_packed.h"

/* yuv_float_to_rgb_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_rgb_32_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], dst[0], dst[1], dst[2])

#define INIT \
  float i_tmp;

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_bgr_32_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_bgr_32_ia_c
#define CONVERT  \
  YUV_8_TO_RGB_24(src[0], src[1], src[2], dst[2], dst[1], dst[0])

#define INIT \
  int32_t i_tmp;

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_64_to_bgr_32_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_bgr_32_ia_c
#define CONVERT  \
  YUV_16_TO_RGB_24(src[0], src[1], src[2], dst[2], dst[1], dst[0])

#define INIT \
  int64_t i_tmp;

#include "../csp_packed_packed.h"

/* yuva_float_to_bgr_32_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_bgr_32_ia_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], dst[2], dst[1], dst[0])

#define INIT \
  float i_tmp;

#include "../csp_packed_packed.h"

/* yuv_float_to_bgr_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_bgr_32_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_24(src[0], src[1], src[2], dst[2], dst[1], dst[0])

#define INIT \
  float i_tmp;

#include "../csp_packed_packed.h"

#ifndef HQ

/* yuva_32_to_rgb_48_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgb_48_ia_c
#define CONVERT  \
  YUV_8_TO_RGB_48(src[0], src[1], src[2], dst[0], dst[1], dst[2])          \

#define INIT   \
  int32_t i_tmp; \

#include "../csp_packed_packed.h"

/* yuva_64_to_rgb_48_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgb_48_ia_c
#define CONVERT  \
  YUV_16_TO_RGB_48(src[0], src[1], src[2], dst[0], dst[1], dst[2])          \

#define INIT   \
  int64_t i_tmp; \

#include "../csp_packed_packed.h"

#endif // !HQ

/* yuva_float_to_rgb_48_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgb_48_ia_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_48(src[0], src[1], src[2], dst[0], dst[1], dst[2])          \

#define INIT   \
  float i_tmp; \

#include "../csp_packed_packed.h"

/* yuv_float_to_rgb_48_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_rgb_48_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_48(src[0], src[1], src[2], dst[0], dst[1], dst[2])          \

#define INIT   \
  float i_tmp; \

#include "../csp_packed_packed.h"


#ifndef HQ


/* yuva_32_to_rgb_float_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_rgb_float_ia_c
#define CONVERT  \
  YUV_8_TO_RGB_FLOAT(src[0], src[1], src[2], dst[0], dst[1], dst[2])          \
    
#define INIT \
  float i_tmp; \

#include "../csp_packed_packed.h"

/* yuva_64_to_rgb_float_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_rgb_float_ia_c
#define CONVERT  \
  YUV_16_TO_RGB_FLOAT(src[0], src[1], src[2], dst[0], dst[1], dst[2])          \
    
#define INIT \
  float i_tmp; \

#include "../csp_packed_packed.h"

/* yuva_float_to_rgb_float_ia_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_rgb_float_ia_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_FLOAT(src[0], src[1], src[2], dst[0], dst[1], dst[2])          \
    
#define INIT \
  float i_tmp; \

#include "../csp_packed_packed.h"

/* yuv_float_to_rgb_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_rgb_float_c
#define CONVERT  \
  YUV_FLOAT_TO_RGB_FLOAT(src[0], src[1], src[2], dst[0], dst[1], dst[2])          \
    
#define INIT \
  float i_tmp; \

#include "../csp_packed_packed.h"

#endif // !HQ


#ifdef HQ
void gavl_init_yuv_rgb_funcs_hq(gavl_pixelformat_function_table_t * tab, const gavl_video_options_t * opt)
#else
void gavl_init_yuv_rgb_funcs_c(gavl_pixelformat_function_table_t * tab, const gavl_video_options_t * opt)
#endif
  {
  if(opt->alpha_mode == GAVL_ALPHA_BLEND_COLOR)
    {
#ifndef HQ
    tab->yuva_32_to_rgb_15 = yuva_32_to_rgb_15_c;
    tab->yuva_32_to_bgr_15 = yuva_32_to_bgr_15_c;
    tab->yuva_32_to_rgb_16 = yuva_32_to_rgb_16_c;
    tab->yuva_32_to_bgr_16 = yuva_32_to_bgr_16_c;
    tab->yuva_32_to_rgb_24 = yuva_32_to_rgb_24_c;
    tab->yuva_32_to_bgr_24 = yuva_32_to_bgr_24_c;
    tab->yuva_32_to_rgb_32 = yuva_32_to_rgb_32_c;
    tab->yuva_32_to_bgr_32 = yuva_32_to_bgr_32_c;
    tab->yuva_32_to_rgb_48 = yuva_32_to_rgb_48_c;
    tab->yuva_32_to_rgb_float = yuva_32_to_rgb_float_c;
    tab->yuva_64_to_rgb_float = yuva_64_to_rgb_float_c;
    tab->yuva_64_to_rgb_48 = yuva_64_to_rgb_48_c;
    tab->yuva_float_to_rgb_float = yuva_float_to_rgb_float_c;
    
#endif
    tab->yuva_64_to_rgb_15 = yuva_64_to_rgb_15_c;
    tab->yuva_64_to_bgr_15 = yuva_64_to_bgr_15_c;
    tab->yuva_64_to_rgb_16 = yuva_64_to_rgb_16_c;
    tab->yuva_64_to_bgr_16 = yuva_64_to_bgr_16_c;
    tab->yuva_64_to_rgb_24 = yuva_64_to_rgb_24_c;
    tab->yuva_64_to_bgr_24 = yuva_64_to_bgr_24_c;
    tab->yuva_64_to_rgb_32 = yuva_64_to_rgb_32_c;
    tab->yuva_64_to_bgr_32 = yuva_64_to_bgr_32_c;
    tab->yuva_float_to_rgb_15 = yuva_float_to_rgb_15_c;
    tab->yuva_float_to_bgr_15 = yuva_float_to_bgr_15_c;
    tab->yuva_float_to_rgb_16 = yuva_float_to_rgb_16_c;
    tab->yuva_float_to_bgr_16 = yuva_float_to_bgr_16_c;
    tab->yuva_float_to_rgb_24 = yuva_float_to_rgb_24_c;
    tab->yuva_float_to_bgr_24 = yuva_float_to_bgr_24_c;
    tab->yuva_float_to_rgb_32 = yuva_float_to_rgb_32_c;
    tab->yuva_float_to_bgr_32 = yuva_float_to_bgr_32_c;
    tab->yuva_float_to_rgb_48 = yuva_float_to_rgb_48_c;
    }
  else if(opt->alpha_mode == GAVL_ALPHA_IGNORE)
    {
#ifndef HQ
    tab->yuva_32_to_rgb_15 = yuva_32_to_rgb_15_ia_c;
    tab->yuva_32_to_bgr_15 = yuva_32_to_bgr_15_ia_c;
    tab->yuva_32_to_rgb_16 = yuva_32_to_rgb_16_ia_c;
    tab->yuva_32_to_bgr_16 = yuva_32_to_bgr_16_ia_c;
    tab->yuva_32_to_rgb_24 = yuva_32_to_rgb_24_ia_c;
    tab->yuva_32_to_bgr_24 = yuva_32_to_bgr_24_ia_c;
    tab->yuva_32_to_rgb_32 = yuva_32_to_rgb_32_ia_c;
    tab->yuva_32_to_bgr_32 = yuva_32_to_bgr_32_ia_c;
    tab->yuva_32_to_rgb_48 = yuva_32_to_rgb_48_ia_c;
    tab->yuva_32_to_rgb_float = yuva_32_to_rgb_float_ia_c;
    tab->yuva_float_to_rgb_float = yuva_float_to_rgb_float_ia_c;
    tab->yuva_64_to_rgb_48 = yuva_64_to_rgb_48_ia_c;
    tab->yuva_64_to_rgb_float = yuva_64_to_rgb_float_ia_c;

#endif
    tab->yuva_64_to_rgb_15 = yuva_64_to_rgb_15_ia_c;
    tab->yuva_64_to_bgr_15 = yuva_64_to_bgr_15_ia_c;
    tab->yuva_64_to_rgb_16 = yuva_64_to_rgb_16_ia_c;
    tab->yuva_64_to_bgr_16 = yuva_64_to_bgr_16_ia_c;
    tab->yuva_64_to_rgb_24 = yuva_64_to_rgb_24_ia_c;
    tab->yuva_64_to_bgr_24 = yuva_64_to_bgr_24_ia_c;
    tab->yuva_64_to_rgb_32 = yuva_64_to_rgb_32_ia_c;
    tab->yuva_64_to_bgr_32 = yuva_64_to_bgr_32_ia_c;
    tab->yuva_float_to_rgb_15 = yuva_float_to_rgb_15_ia_c;
    tab->yuva_float_to_bgr_15 = yuva_float_to_bgr_15_ia_c;
    tab->yuva_float_to_rgb_16 = yuva_float_to_rgb_16_ia_c;
    tab->yuva_float_to_bgr_16 = yuva_float_to_bgr_16_ia_c;
    tab->yuva_float_to_rgb_24 = yuva_float_to_rgb_24_ia_c;
    tab->yuva_float_to_bgr_24 = yuva_float_to_bgr_24_ia_c;
    tab->yuva_float_to_rgb_32 = yuva_float_to_rgb_32_ia_c;
    tab->yuva_float_to_bgr_32 = yuva_float_to_bgr_32_ia_c;
    tab->yuva_float_to_rgb_48 = yuva_float_to_rgb_48_ia_c;

    }

  tab->yuva_64_to_rgba_32 = yuva_64_to_rgba_32_c;
  tab->yuva_float_to_rgba_32 = yuva_float_to_rgba_32_c;
  tab->yuva_float_to_rgba_64 = yuva_float_to_rgba_64_c;
 
  
#ifndef HQ
  tab->yuva_64_to_rgba_float = yuva_64_to_rgba_float_c;
  tab->yuva_float_to_rgba_float = yuva_float_to_rgba_float_c;
  tab->yuva_64_to_rgba_64 = yuva_64_to_rgba_64_c;
  
  tab->yuva_32_to_rgba_32 = yuva_32_to_rgba_32_c;
  tab->yuva_32_to_rgba_64 = yuva_32_to_rgba_64_c;
  tab->yuva_32_to_rgba_float = yuva_32_to_rgba_float_c;
  
  tab->yuy2_to_rgb_15 = yuy2_to_rgb_15_c;
  tab->yuy2_to_bgr_15 = yuy2_to_bgr_15_c;
  tab->yuy2_to_rgb_16 = yuy2_to_rgb_16_c;
  tab->yuy2_to_bgr_16 = yuy2_to_bgr_16_c;
  tab->yuy2_to_rgb_24 = yuy2_to_rgb_24_c;
  tab->yuy2_to_bgr_24 = yuy2_to_bgr_24_c;
  tab->yuy2_to_rgb_32 = yuy2_to_rgb_32_c;
  tab->yuy2_to_bgr_32 = yuy2_to_bgr_32_c;
  tab->yuy2_to_rgb_48 = yuy2_to_rgb_48_c;
  tab->yuy2_to_rgb_float = yuy2_to_rgb_float_c;

  tab->yuy2_to_rgba_32 = yuy2_to_rgba_32_c;
  tab->yuy2_to_rgba_64 = yuy2_to_rgba_64_c;
  tab->yuy2_to_rgba_float = yuy2_to_rgba_float_c;

  tab->uyvy_to_rgb_15 = uyvy_to_rgb_15_c;
  tab->uyvy_to_bgr_15 = uyvy_to_bgr_15_c;
  tab->uyvy_to_rgb_16 = uyvy_to_rgb_16_c;
  tab->uyvy_to_bgr_16 = uyvy_to_bgr_16_c;
  tab->uyvy_to_rgb_24 = uyvy_to_rgb_24_c;
  tab->uyvy_to_bgr_24 = uyvy_to_bgr_24_c;
  tab->uyvy_to_rgb_32 = uyvy_to_rgb_32_c;
  tab->uyvy_to_bgr_32 = uyvy_to_bgr_32_c;
  tab->uyvy_to_rgb_48 = uyvy_to_rgb_48_c;
  tab->uyvy_to_rgb_float = uyvy_to_rgb_float_c;
  
  tab->uyvy_to_rgba_32 = uyvy_to_rgba_32_c;
  tab->uyvy_to_rgba_64 = uyvy_to_rgba_64_c;
  tab->uyvy_to_rgba_float = uyvy_to_rgba_float_c;
  
  tab->yuv_420_p_to_rgb_15 = yuv_420_p_to_rgb_15_c;
  tab->yuv_420_p_to_bgr_15 = yuv_420_p_to_bgr_15_c;
  tab->yuv_420_p_to_rgb_16 = yuv_420_p_to_rgb_16_c;
  tab->yuv_420_p_to_bgr_16 = yuv_420_p_to_bgr_16_c;
  tab->yuv_420_p_to_rgb_24 = yuv_420_p_to_rgb_24_c;
  tab->yuv_420_p_to_bgr_24 = yuv_420_p_to_bgr_24_c;
  tab->yuv_420_p_to_rgb_32 = yuv_420_p_to_rgb_32_c;
  tab->yuv_420_p_to_bgr_32 = yuv_420_p_to_bgr_32_c;
  tab->yuv_420_p_to_rgb_48 = yuv_420_p_to_rgb_48_c;
  tab->yuv_420_p_to_rgb_float = yuv_420_p_to_rgb_float_c;
  
  tab->yuv_420_p_to_rgba_32 = yuv_420_p_to_rgba_32_c;
  tab->yuv_420_p_to_rgba_64 = yuv_420_p_to_rgba_64_c;
  tab->yuv_420_p_to_rgba_float = yuv_420_p_to_rgba_float_c;

  tab->yuv_410_p_to_rgb_15 = yuv_410_p_to_rgb_15_c;
  tab->yuv_410_p_to_bgr_15 = yuv_410_p_to_bgr_15_c;
  tab->yuv_410_p_to_rgb_16 = yuv_410_p_to_rgb_16_c;
  tab->yuv_410_p_to_bgr_16 = yuv_410_p_to_bgr_16_c;
  tab->yuv_410_p_to_rgb_24 = yuv_410_p_to_rgb_24_c;
  tab->yuv_410_p_to_bgr_24 = yuv_410_p_to_bgr_24_c;
  tab->yuv_410_p_to_rgb_32 = yuv_410_p_to_rgb_32_c;
  tab->yuv_410_p_to_bgr_32 = yuv_410_p_to_bgr_32_c;
  tab->yuv_410_p_to_rgb_48 = yuv_410_p_to_rgb_48_c;
  tab->yuv_410_p_to_rgb_float = yuv_410_p_to_rgb_float_c;

  tab->yuv_410_p_to_rgba_32 = yuv_410_p_to_rgba_32_c;
  tab->yuv_410_p_to_rgba_64 = yuv_410_p_to_rgba_64_c;
  tab->yuv_410_p_to_rgba_float = yuv_410_p_to_rgba_float_c;
 
  tab->yuv_422_p_to_rgb_15 = yuv_422_p_to_rgb_15_c;
  tab->yuv_422_p_to_bgr_15 = yuv_422_p_to_bgr_15_c;
  tab->yuv_422_p_to_rgb_16 = yuv_422_p_to_rgb_16_c;
  tab->yuv_422_p_to_bgr_16 = yuv_422_p_to_bgr_16_c;
  tab->yuv_422_p_to_rgb_24 = yuv_422_p_to_rgb_24_c;
  tab->yuv_422_p_to_bgr_24 = yuv_422_p_to_bgr_24_c;
  tab->yuv_422_p_to_rgb_32 = yuv_422_p_to_rgb_32_c;
  tab->yuv_422_p_to_bgr_32 = yuv_422_p_to_bgr_32_c;
  tab->yuv_422_p_to_rgb_48 = yuv_422_p_to_rgb_48_c;
  tab->yuv_422_p_to_rgb_float = yuv_422_p_to_rgb_float_c;
  tab->yuv_422_p_to_rgba_32 = yuv_422_p_to_rgba_32_c;
  tab->yuv_422_p_to_rgba_64 = yuv_422_p_to_rgba_64_c;
  tab->yuv_422_p_to_rgba_float = yuv_422_p_to_rgba_float_c;
#endif
  
  tab->yuv_422_p_16_to_rgb_15 = yuv_422_p_16_to_rgb_15_c;
  tab->yuv_422_p_16_to_bgr_15 = yuv_422_p_16_to_bgr_15_c;
  tab->yuv_422_p_16_to_rgb_16 = yuv_422_p_16_to_rgb_16_c;
  tab->yuv_422_p_16_to_bgr_16 = yuv_422_p_16_to_bgr_16_c;
  tab->yuv_422_p_16_to_rgb_24 = yuv_422_p_16_to_rgb_24_c;
  tab->yuv_422_p_16_to_bgr_24 = yuv_422_p_16_to_bgr_24_c;
  tab->yuv_422_p_16_to_rgb_32 = yuv_422_p_16_to_rgb_32_c;
  tab->yuv_422_p_16_to_bgr_32 = yuv_422_p_16_to_bgr_32_c;
  tab->yuv_422_p_16_to_rgba_32 = yuv_422_p_16_to_rgba_32_c;
#ifndef HQ
  tab->yuv_422_p_16_to_rgb_48 = yuv_422_p_16_to_rgb_48_c;
  tab->yuv_422_p_16_to_rgb_float = yuv_422_p_16_to_rgb_float_c;
  tab->yuv_422_p_16_to_rgba_64 = yuv_422_p_16_to_rgba_64_c;
  tab->yuv_422_p_16_to_rgba_float = yuv_422_p_16_to_rgba_float_c;

  tab->yuv_411_p_to_rgb_15 = yuv_411_p_to_rgb_15_c;
  tab->yuv_411_p_to_bgr_15 = yuv_411_p_to_bgr_15_c;
  tab->yuv_411_p_to_rgb_16 = yuv_411_p_to_rgb_16_c;
  tab->yuv_411_p_to_bgr_16 = yuv_411_p_to_bgr_16_c;
  tab->yuv_411_p_to_rgb_24 = yuv_411_p_to_rgb_24_c;
  tab->yuv_411_p_to_bgr_24 = yuv_411_p_to_bgr_24_c;
  tab->yuv_411_p_to_rgb_32 = yuv_411_p_to_rgb_32_c;
  tab->yuv_411_p_to_bgr_32 = yuv_411_p_to_bgr_32_c;
  tab->yuv_411_p_to_rgb_48 = yuv_411_p_to_rgb_48_c;
  tab->yuv_411_p_to_rgb_float = yuv_411_p_to_rgb_float_c;

  tab->yuv_411_p_to_rgba_32 = yuv_411_p_to_rgba_32_c;
  tab->yuv_411_p_to_rgba_64 = yuv_411_p_to_rgba_64_c;
  tab->yuv_411_p_to_rgba_float = yuv_411_p_to_rgba_float_c;
  
  tab->yuv_444_p_to_rgb_15 = yuv_444_p_to_rgb_15_c;
  tab->yuv_444_p_to_bgr_15 = yuv_444_p_to_bgr_15_c;
  tab->yuv_444_p_to_rgb_16 = yuv_444_p_to_rgb_16_c;
  tab->yuv_444_p_to_bgr_16 = yuv_444_p_to_bgr_16_c;
  tab->yuv_444_p_to_rgb_24 = yuv_444_p_to_rgb_24_c;
  tab->yuv_444_p_to_bgr_24 = yuv_444_p_to_bgr_24_c;
  tab->yuv_444_p_to_rgb_32 = yuv_444_p_to_rgb_32_c;
  tab->yuv_444_p_to_bgr_32 = yuv_444_p_to_bgr_32_c;
  tab->yuv_444_p_to_rgb_48 = yuv_444_p_to_rgb_48_c;
  tab->yuv_444_p_to_rgb_float = yuv_444_p_to_rgb_float_c;
  tab->yuv_444_p_to_rgba_32 = yuv_444_p_to_rgba_32_c;
  tab->yuv_444_p_to_rgba_64 = yuv_444_p_to_rgba_64_c;
  tab->yuv_444_p_to_rgba_float = yuv_444_p_to_rgba_float_c;
#endif

  tab->yuv_444_p_16_to_rgb_15 = yuv_444_p_16_to_rgb_15_c;
  tab->yuv_444_p_16_to_bgr_15 = yuv_444_p_16_to_bgr_15_c;
  tab->yuv_444_p_16_to_rgb_16 = yuv_444_p_16_to_rgb_16_c;
  tab->yuv_444_p_16_to_bgr_16 = yuv_444_p_16_to_bgr_16_c;
  tab->yuv_444_p_16_to_rgb_24 = yuv_444_p_16_to_rgb_24_c;
  tab->yuv_444_p_16_to_bgr_24 = yuv_444_p_16_to_bgr_24_c;
  tab->yuv_444_p_16_to_rgb_32 = yuv_444_p_16_to_rgb_32_c;
  tab->yuv_444_p_16_to_bgr_32 = yuv_444_p_16_to_bgr_32_c;
  tab->yuv_444_p_16_to_rgba_32 = yuv_444_p_16_to_rgba_32_c;
#ifndef HQ

  tab->yuv_444_p_16_to_rgb_48 = yuv_444_p_16_to_rgb_48_c;
  tab->yuv_444_p_16_to_rgb_float = yuv_444_p_16_to_rgb_float_c;
  tab->yuv_444_p_16_to_rgba_64 = yuv_444_p_16_to_rgba_64_c;
  tab->yuv_444_p_16_to_rgba_float = yuv_444_p_16_to_rgba_float_c;

  
  tab->yuvj_420_p_to_rgb_15 = yuvj_420_p_to_rgb_15_c;
  tab->yuvj_420_p_to_bgr_15 = yuvj_420_p_to_bgr_15_c;
  tab->yuvj_420_p_to_rgb_16 = yuvj_420_p_to_rgb_16_c;
  tab->yuvj_420_p_to_bgr_16 = yuvj_420_p_to_bgr_16_c;
  tab->yuvj_420_p_to_rgb_24 = yuvj_420_p_to_rgb_24_c;
  tab->yuvj_420_p_to_bgr_24 = yuvj_420_p_to_bgr_24_c;
  tab->yuvj_420_p_to_rgb_32 = yuvj_420_p_to_rgb_32_c;
  tab->yuvj_420_p_to_bgr_32 = yuvj_420_p_to_bgr_32_c;
  tab->yuvj_420_p_to_rgb_48 = yuvj_420_p_to_rgb_48_c;
  tab->yuvj_420_p_to_rgb_float = yuvj_420_p_to_rgb_float_c;

  tab->yuvj_420_p_to_rgba_32 = yuvj_420_p_to_rgba_32_c;
  tab->yuvj_420_p_to_rgba_64 = yuvj_420_p_to_rgba_64_c;
  tab->yuvj_420_p_to_rgba_float = yuvj_420_p_to_rgba_float_c;

  tab->yuvj_422_p_to_rgb_15 = yuvj_422_p_to_rgb_15_c;
  tab->yuvj_422_p_to_bgr_15 = yuvj_422_p_to_bgr_15_c;
  tab->yuvj_422_p_to_rgb_16 = yuvj_422_p_to_rgb_16_c;
  tab->yuvj_422_p_to_bgr_16 = yuvj_422_p_to_bgr_16_c;
  tab->yuvj_422_p_to_rgb_24 = yuvj_422_p_to_rgb_24_c;
  tab->yuvj_422_p_to_bgr_24 = yuvj_422_p_to_bgr_24_c;
  tab->yuvj_422_p_to_rgb_32 = yuvj_422_p_to_rgb_32_c;
  tab->yuvj_422_p_to_bgr_32 = yuvj_422_p_to_bgr_32_c;
  tab->yuvj_422_p_to_rgb_48 = yuvj_422_p_to_rgb_48_c;
  tab->yuvj_422_p_to_rgb_float = yuvj_422_p_to_rgb_float_c;

  tab->yuvj_422_p_to_rgba_32 = yuvj_422_p_to_rgba_32_c;
  tab->yuvj_422_p_to_rgba_64 = yuvj_422_p_to_rgba_64_c;
  tab->yuvj_422_p_to_rgba_float = yuvj_422_p_to_rgba_float_c;

  tab->yuvj_444_p_to_rgb_15 = yuvj_444_p_to_rgb_15_c;
  tab->yuvj_444_p_to_bgr_15 = yuvj_444_p_to_bgr_15_c;
  tab->yuvj_444_p_to_rgb_16 = yuvj_444_p_to_rgb_16_c;
  tab->yuvj_444_p_to_bgr_16 = yuvj_444_p_to_bgr_16_c;
  tab->yuvj_444_p_to_rgb_24 = yuvj_444_p_to_rgb_24_c;
  tab->yuvj_444_p_to_bgr_24 = yuvj_444_p_to_bgr_24_c;
  tab->yuvj_444_p_to_rgb_32 = yuvj_444_p_to_rgb_32_c;
  tab->yuvj_444_p_to_bgr_32 = yuvj_444_p_to_bgr_32_c;
  tab->yuvj_444_p_to_rgb_48 = yuvj_444_p_to_rgb_48_c;
  tab->yuvj_444_p_to_rgb_float = yuvj_444_p_to_rgb_float_c;

  tab->yuvj_444_p_to_rgba_32 = yuvj_444_p_to_rgba_32_c;
  tab->yuvj_444_p_to_rgba_64 = yuvj_444_p_to_rgba_64_c;
  tab->yuvj_444_p_to_rgba_float = yuvj_444_p_to_rgba_float_c;
#endif

  tab->yuv_float_to_rgb_15 = yuv_float_to_rgb_15_c;
  tab->yuv_float_to_bgr_15 = yuv_float_to_bgr_15_c;
  tab->yuv_float_to_rgb_16 = yuv_float_to_rgb_16_c;
 tab->yuv_float_to_bgr_16 = yuv_float_to_bgr_16_c;
  tab->yuv_float_to_rgb_24 = yuv_float_to_rgb_24_c;
  tab->yuv_float_to_bgr_24 = yuv_float_to_bgr_24_c;
  tab->yuv_float_to_rgb_32 = yuv_float_to_rgb_32_c;
  tab->yuv_float_to_bgr_32 = yuv_float_to_bgr_32_c;
  tab->yuv_float_to_rgba_32 = yuv_float_to_rgba_32_c;
  tab->yuv_float_to_rgb_48 = yuv_float_to_rgb_48_c;
  tab->yuv_float_to_rgba_64 = yuv_float_to_rgba_64_c;
#ifndef HQ
  tab->yuv_float_to_rgb_float = yuv_float_to_rgb_float_c;
  tab->yuv_float_to_rgba_float = yuv_float_to_rgba_float_c;
#endif
 
  }
