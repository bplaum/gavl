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

/* yuy2_to_gray_8_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuy2_to_gray_8_c
#define CONVERT dst[0] = Y_8_TO_YJ_8(src[0]);

#include "../csp_packed_packed.h"

/* uyvy_to_gray_8_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME uyvy_to_gray_8_c
#define CONVERT dst[0] = Y_8_TO_YJ_8(src[1]);

#include "../csp_packed_packed.h"

/* yuy2_to_gray_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuy2_to_gray_16_c
#define CONVERT dst[0] = Y_8_TO_YJ_16(src[0]);

#include "../csp_packed_packed.h"

/* uyvy_to_gray_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME uyvy_to_gray_16_c
#define CONVERT dst[0] = Y_8_TO_YJ_16(src[1]);;

#include "../csp_packed_packed.h"

/* yuy2_to_gray_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuy2_to_gray_float_c
#define CONVERT dst[0] = Y_8_TO_FLOAT(src[0]);

#include "../csp_packed_packed.h"

/* uyvy_to_gray_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME uyvy_to_gray_float_c
#define CONVERT dst[0] = Y_8_TO_FLOAT(src[1]);

#include "../csp_packed_packed.h"

/* */

/* yuy2_to_graya_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuy2_to_graya_16_c
#define CONVERT dst[0] = Y_8_TO_YJ_8(src[0]); \
                dst[1] = 0xff;

#include "../csp_packed_packed.h"

/* uyvy_to_graya_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME uyvy_to_graya_16_c
#define CONVERT dst[0] = Y_8_TO_YJ_8(src[1]); \
                dst[1] = 0xff;


#include "../csp_packed_packed.h"

/* yuy2_to_graya_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuy2_to_graya_32_c
#define CONVERT dst[0] = Y_8_TO_YJ_16(src[0]); \
                dst[1] = 0xffff;


#include "../csp_packed_packed.h"

/* uyvy_to_graya_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME uyvy_to_graya_32_c
#define CONVERT dst[0] = Y_8_TO_YJ_16(src[1]); \
                dst[1] = 0xffff;


#include "../csp_packed_packed.h"

/* yuy2_to_graya_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuy2_to_graya_float_c
#define CONVERT dst[0] = Y_8_TO_FLOAT(src[0]); \
                dst[1] = 1.0;


#include "../csp_packed_packed.h"

/* uyvy_to_graya_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME uyvy_to_graya_float_c
#define CONVERT dst[0] = Y_8_TO_FLOAT(src[1]); \
                dst[1] = 1.0;

#include "../csp_packed_packed.h"

/* */

/* y_8_to_gray_8_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME y_8_to_gray_8_c
#define CONVERT dst[0] = Y_8_TO_YJ_8(src[0]);

#include "../csp_packed_packed.h"

/* y_8_to_gray_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME y_8_to_gray_16_c
#define CONVERT dst[0] = Y_8_TO_YJ_16(src[0]);

#include "../csp_packed_packed.h"

/* y_8_to_gray_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME y_8_to_gray_float_c
#define CONVERT dst[0] = Y_8_TO_FLOAT(src[0]);

#include "../csp_packed_packed.h"

/* yj_8_to_gray_8_c */

static void yj_8_to_gray_8_c(gavl_video_convert_context_t * ctx)
  {
  gavl_video_frame_copy(&ctx->output_format, ctx->output_frame, ctx->input_frame);
  }

/* yj_8_to_gray_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yj_8_to_gray_16_c
#define CONVERT dst[0] = RGB_8_TO_16(src[0]);

#include "../csp_packed_packed.h"

/* yj_8_to_gray_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yj_8_to_gray_float_c
#define CONVERT dst[0] = RGB_8_TO_FLOAT(src[0]);

#include "../csp_packed_packed.h"

/* */

/* y_8_to_graya_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME y_8_to_graya_16_c
#define CONVERT dst[0] = Y_8_TO_YJ_8(src[0]); \
  dst[1] = 0xff;

#include "../csp_packed_packed.h"

/* y_8_to_graya_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME y_8_to_graya_32_c
#define CONVERT dst[0] = Y_8_TO_YJ_16(src[0]); \
  dst[1] = 0xffff;

#include "../csp_packed_packed.h"

/* y_8_to_graya_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME y_8_to_graya_float_c
#define CONVERT dst[0] = Y_8_TO_FLOAT(src[0]); \
  dst[1] = 1.0;


#include "../csp_packed_packed.h"

/* yj_8_to_graya_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yj_8_to_graya_16_c
#define CONVERT dst[0] = src[0]; \
  dst[1] = 0xff;

#include "../csp_packed_packed.h"


/* yj_8_to_graya_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yj_8_to_graya_32_c
#define CONVERT dst[0] = RGB_8_TO_16(src[0]); \
  dst[1] = 0xffff;

#include "../csp_packed_packed.h"

/* yj_8_to_graya_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yj_8_to_graya_float_c
#define CONVERT dst[0] = RGB_8_TO_FLOAT(src[0]); \
  dst[1] = 1.0;


#include "../csp_packed_packed.h"

/* */

/* y_16_to_gray_8_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME y_16_to_gray_8_c
#define CONVERT Y_16_TO_YJ_8(src[0], dst[0]);

#include "../csp_packed_packed.h"

/* y_16_to_gray_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME y_16_to_gray_16_c
#define CONVERT Y_16_TO_YJ_16(src[0], dst[0]);

#include "../csp_packed_packed.h"

/* y_16_to_gray_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME y_16_to_gray_float_c
#define CONVERT dst[0] = Y_16_TO_Y_FLOAT(src[0]);

#include "../csp_packed_packed.h"

/* y_16_to_graya_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME y_16_to_graya_16_c
#define CONVERT Y_16_TO_YJ_8(src[0], dst[0]);\
  dst[1] = 0xff;

#include "../csp_packed_packed.h"

/* y_16_to_graya_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME y_16_to_graya_32_c
#define CONVERT Y_16_TO_YJ_16(src[0], dst[0]);\
  dst[1] = 0xffff;

#include "../csp_packed_packed.h"

/* y_16_to_graya_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME y_16_to_graya_float_c
#define CONVERT dst[0] = Y_16_TO_Y_FLOAT(src[0]);\
  dst[1] = 1.0;

#include "../csp_packed_packed.h"

/* yuva_32_to_graya_16 */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_graya_16_c
#define CONVERT dst[0] = Y_8_TO_YJ_8(src[0]); \
                dst[1] = src[3];

#include "../csp_packed_packed.h"

/* yuva_32_to_graya_32 */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_graya_32_c
#define CONVERT dst[0] = Y_8_TO_YJ_16(src[0]); \
                dst[1] = RGB_8_TO_16(src[3]);

#include "../csp_packed_packed.h"

/* yuva_32_to_graya_float  */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_graya_float_c
#define CONVERT dst[0] = Y_8_TO_FLOAT(src[0]); \
                dst[1] = RGB_8_TO_FLOAT(src[3]);

#include "../csp_packed_packed.h"


/* yuva_32_to_gray_8_ia */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_gray_8_ia_c
#define CONVERT dst[0] = Y_8_TO_YJ_8(src[0]);

#include "../csp_packed_packed.h"

/* yuva_32_to_gray_16_ia */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_gray_16_ia_c
#define CONVERT dst[0] = Y_8_TO_YJ_16(src[0]);

#include "../csp_packed_packed.h"

/* yuva_32_to_gray_float_ia  */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_gray_float_ia_c
#define CONVERT dst[0] = Y_8_TO_FLOAT(src[0]);

#include "../csp_packed_packed.h"

/* YUVA -> GRAY */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_gray_8_c
#define INIT INIT_GRAYA_16
#define CONVERT GRAYA_16_TO_GRAY_8(Y_8_TO_YJ_8(src[0]), src[3], dst[0])

#include "../csp_packed_packed.h"

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_gray_16_c
#define INIT int tmp; INIT_GRAYA_16
#define CONVERT GRAYA_16_TO_GRAY_8(Y_8_TO_YJ_8(src[0]), src[3], tmp) \
                dst[0] = RGB_8_TO_16(tmp);

#include "../csp_packed_packed.h"

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_32_to_gray_float_c
#define INIT int tmp; INIT_GRAYA_16
#define CONVERT GRAYA_16_TO_GRAY_8(Y_8_TO_YJ_8(src[0]), src[3], tmp) \
                dst[0] = RGB_8_TO_FLOAT(tmp);

#include "../csp_packed_packed.h"

/* */

/* yuva_64_to_graya_16 */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_graya_16_c
#define CONVERT Y_16_TO_YJ_8(src[0], dst[0]); \
                RGB_16_TO_8(src[3], dst[1]);

#include "../csp_packed_packed.h"

/* yuva_64_to_graya_32 */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_graya_32_c
#define CONVERT Y_16_TO_YJ_16(src[0], dst[0]); \
                dst[1] = src[3];

#include "../csp_packed_packed.h"

/* yuva_64_to_graya_float  */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_graya_float_c
#define CONVERT dst[0] = Y_16_TO_Y_FLOAT(src[0]); \
                dst[1] = RGB_16_TO_FLOAT(src[3]);

#include "../csp_packed_packed.h"


/* yuva_64_to_gray_8_ia */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_gray_8_ia_c
#define CONVERT Y_16_TO_YJ_8(src[0], dst[0]);

#include "../csp_packed_packed.h"

/* yuva_64_to_gray_16_ia */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_gray_16_ia_c
#define CONVERT Y_16_TO_YJ_16(src[0], dst[0]);

#include "../csp_packed_packed.h"

/* yuva_64_to_gray_float_ia  */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_gray_float_ia_c
#define CONVERT dst[0] = Y_16_TO_Y_FLOAT(src[0]);

#include "../csp_packed_packed.h"

/* YUVA -> GRAY */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_gray_8_c
#define INIT uint32_t tmp, tmp1; INIT_GRAYA_32
#define CONVERT Y_16_TO_YJ_16(src[0], tmp1); \
                GRAYA_32_TO_GRAY_16(tmp1, src[3], tmp)\
                RGB_16_TO_8(tmp, dst[0]);

#include "../csp_packed_packed.h"

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_gray_16_c
#define INIT int tmp; INIT_GRAYA_32
#define CONVERT \
  Y_16_TO_YJ_16(src[0], tmp); \
  GRAYA_32_TO_GRAY_16(tmp, src[3], dst[0])

#include "../csp_packed_packed.h"

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_64_to_gray_float_c
#define INIT int tmp, tmp_gray; INIT_GRAYA_32
#define CONVERT Y_16_TO_YJ_16(src[0], tmp_gray);\
                GRAYA_32_TO_GRAY_16(tmp_gray, src[3], tmp) \
                dst[0] = RGB_16_TO_FLOAT(tmp);

#include "../csp_packed_packed.h"

/* */

/* yuva_float_to_graya_16 */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_graya_16_c
#define CONVERT YJ_FLOAT_TO_8(src[0], dst[0]); \
                RGB_FLOAT_TO_8(src[3], dst[1]);

#include "../csp_packed_packed.h"

/* yuva_float_to_graya_32 */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_graya_32_c
#define CONVERT RGB_FLOAT_TO_16(src[0], dst[0]); \
                RGB_FLOAT_TO_16(src[3], dst[1]);

#include "../csp_packed_packed.h"

/* yuva_float_to_graya_float  */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_graya_float_c
#define CONVERT dst[0] = src[0]; \
                dst[1] = src[3];

#include "../csp_packed_packed.h"

/* yuva_float_to_gray_8_ia */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_gray_8_ia_c
#define CONVERT YJ_FLOAT_TO_8(src[0], dst[0]);

#include "../csp_packed_packed.h"

/* yuva_float_to_gray_16_ia */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_gray_16_ia_c
#define CONVERT RGB_FLOAT_TO_16(src[0], dst[0]);

#include "../csp_packed_packed.h"

/* yuva_float_to_gray_float_ia  */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_gray_float_ia_c
#define CONVERT dst[0] = src[0];

#include "../csp_packed_packed.h"

/* YUVA -> GRAY */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_gray_8_c
#define INIT float tmp; INIT_GRAYA_FLOAT
#define CONVERT GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[3], tmp)\
            RGB_FLOAT_TO_8(tmp, dst[0]);

#include "../csp_packed_packed.h"

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_gray_16_c
#define INIT float tmp; INIT_GRAYA_FLOAT
#define CONVERT \
  GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[3], tmp) \
  RGB_FLOAT_TO_16(tmp, dst[0]); \
  
#include "../csp_packed_packed.h"

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuva_float_to_gray_float_c
#define INIT INIT_GRAYA_FLOAT
#define CONVERT GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[3], dst[0])

#include "../csp_packed_packed.h"

/* YUV float -> */

/* yuv_float_to_graya_16 */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_graya_16_c
#define CONVERT YJ_FLOAT_TO_8(src[0], dst[0]); \
                dst[1] = 0xff;

#include "../csp_packed_packed.h"

/* yuv_float_to_graya_32 */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_graya_32_c
#define CONVERT RGB_FLOAT_TO_16(src[0], dst[0]); \
                dst[1] = 0xffff;

#include "../csp_packed_packed.h"

/* yuv_float_to_graya_float  */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_graya_float_c
#define CONVERT dst[0] = src[0]; \
                dst[1] = 1.0;

#include "../csp_packed_packed.h"

/* yuv_float_to_gray_8 */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_gray_8_c
#define CONVERT YJ_FLOAT_TO_8(src[0], dst[0]);

#include "../csp_packed_packed.h"

/* yuv_float_to_gray_16 */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_gray_16_c
#define CONVERT RGB_FLOAT_TO_16(src[0], dst[0]);

#include "../csp_packed_packed.h"

/* yuv_float_to_gray_float  */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  3
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME yuv_float_to_gray_float_c
#define CONVERT dst[0] = src[0];

#include "../csp_packed_packed.h"

void gavl_init_yuv_gray_funcs_c(gavl_pixelformat_function_table_t * tab,
                                const gavl_video_options_t * opt)
  {
  if(opt->alpha_mode == GAVL_ALPHA_BLEND_COLOR)
    {
    tab->yuva_32_to_gray_8 = yuva_32_to_gray_8_c;
    tab->yuva_32_to_gray_16 = yuva_32_to_gray_16_c;
    tab->yuva_32_to_gray_float = yuva_32_to_gray_float_c;
    tab->yuva_64_to_gray_8 = yuva_64_to_gray_8_c;
    tab->yuva_float_to_gray_8 = yuva_float_to_gray_8_c;
    tab->yuva_64_to_gray_16 = yuva_64_to_gray_16_c;
    tab->yuva_float_to_gray_16 = yuva_float_to_gray_16_c;
    tab->yuva_64_to_gray_float = yuva_64_to_gray_float_c;
    tab->yuva_float_to_gray_float = yuva_float_to_gray_float_c;

    }
  else
    {
    tab->yuva_32_to_gray_8 = yuva_32_to_gray_8_ia_c;
    tab->yuva_32_to_gray_16 = yuva_32_to_gray_16_ia_c;
    tab->yuva_32_to_gray_float = yuva_32_to_gray_float_ia_c;

    tab->yuva_64_to_gray_8 = yuva_64_to_gray_8_ia_c;
    tab->yuva_float_to_gray_8 = yuva_float_to_gray_8_ia_c;
    tab->yuva_64_to_gray_16 = yuva_64_to_gray_16_ia_c;
    tab->yuva_float_to_gray_16 = yuva_float_to_gray_16_ia_c;
    tab->yuva_64_to_gray_float = yuva_64_to_gray_float_ia_c;
    tab->yuva_float_to_gray_float = yuva_float_to_gray_float_ia_c;

    }

  tab->yuv_float_to_gray_8 = yuv_float_to_gray_8_c;
  tab->yuv_float_to_gray_16 = yuv_float_to_gray_16_c;
  tab->yuv_float_to_gray_float = yuv_float_to_gray_float_c;
  tab->yuv_float_to_graya_16 = yuv_float_to_graya_16_c;
  tab->yuv_float_to_graya_32 = yuv_float_to_graya_32_c;
  tab->yuv_float_to_graya_float = yuv_float_to_graya_float_c;
  
  tab->yuva_64_to_graya_16 = yuva_64_to_graya_16_c;
  tab->yuva_float_to_graya_16 = yuva_float_to_graya_16_c;
  tab->yuva_64_to_graya_32 = yuva_64_to_graya_32_c;
  tab->yuva_float_to_graya_32 = yuva_float_to_graya_32_c;
  tab->yuva_64_to_graya_float = yuva_64_to_graya_float_c;
  tab->yuva_float_to_graya_float = yuva_float_to_graya_float_c;
  
  tab->yuy2_to_gray_8 = yuy2_to_gray_8_c;
  tab->uyvy_to_gray_8 = uyvy_to_gray_8_c;
  tab->y_8_to_gray_8 = y_8_to_gray_8_c;
  tab->yj_8_to_gray_8 = yj_8_to_gray_8_c;
  tab->y_16_to_gray_8 = y_16_to_gray_8_c;
  tab->yuy2_to_gray_16 = yuy2_to_gray_16_c;
  tab->uyvy_to_gray_16 = uyvy_to_gray_16_c;
  tab->y_8_to_gray_16 = y_8_to_gray_16_c;
  tab->yj_8_to_gray_16 = yj_8_to_gray_16_c;
  tab->y_16_to_gray_16 = y_16_to_gray_16_c;
  tab->yuy2_to_gray_float = yuy2_to_gray_float_c;
  tab->uyvy_to_gray_float = uyvy_to_gray_float_c;
  tab->y_8_to_gray_float = y_8_to_gray_float_c;
  tab->yj_8_to_gray_float = yj_8_to_gray_float_c;
  tab->y_16_to_gray_float = y_16_to_gray_float_c;
  tab->yuy2_to_graya_16 = yuy2_to_graya_16_c;
  tab->uyvy_to_graya_16 = uyvy_to_graya_16_c;
  tab->y_8_to_graya_16 = y_8_to_graya_16_c;
  tab->yj_8_to_graya_16 = yj_8_to_graya_16_c;
  tab->y_16_to_graya_16 = y_16_to_graya_16_c;
  tab->yuy2_to_graya_32 = yuy2_to_graya_32_c;
  tab->uyvy_to_graya_32 = uyvy_to_graya_32_c;
  tab->y_8_to_graya_32 = y_8_to_graya_32_c;
  tab->yj_8_to_graya_32 = yj_8_to_graya_32_c;
  tab->y_16_to_graya_32 = y_16_to_graya_32_c;
  tab->yuy2_to_graya_float = yuy2_to_graya_float_c;
  tab->uyvy_to_graya_float = uyvy_to_graya_float_c;
  tab->y_8_to_graya_float = y_8_to_graya_float_c;
  tab->yj_8_to_graya_float = yj_8_to_graya_float_c;
  tab->y_16_to_graya_float = y_16_to_graya_float_c;
  
  tab->yuva_32_to_graya_16 = yuva_32_to_graya_16_c;
  tab->yuva_32_to_graya_float = yuva_32_to_graya_float_c;
  tab->yuva_32_to_graya_32 = yuva_32_to_graya_32_c;
  }
