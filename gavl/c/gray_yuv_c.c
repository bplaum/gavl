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

/* Gray -> YUV */

/* gray_8_to_yuy2_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_yuy2_c
#define CONVERT dst[0] = YJ_8_TO_Y_8(src[0]);\
                dst[1] = 0x80;

#include "../csp_packed_packed.h"

/* gray_8_to_uyvy_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_uyvy_c
#define CONVERT dst[1] = YJ_8_TO_Y_8(src[0]);\
                dst[0] = 0x80;

#include "../csp_packed_packed.h"

/* gray_8_to_yuva_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_yuva_32_c
#define CONVERT dst[0] = YJ_8_TO_Y_8(src[0]);\
                dst[1] = 0x80; \
                dst[2] = 0x80; \
                dst[3] = 0xFF;

#include "../csp_packed_packed.h"

/* gray_8_to_yuva_64_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_yuva_64_c
#define CONVERT dst[0] = YJ_8_TO_Y_16(src[0]);\
                dst[1] = 0x8000; \
                dst[2] = 0x8000; \
                dst[3] = 0xFFFF;

#include "../csp_packed_packed.h"

/* gray_8_to_yuva_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_yuva_float_c
#define CONVERT dst[0] = RGB_8_TO_FLOAT(src[0]);\
                dst[1] = 0.0; \
                dst[2] = 0.0; \
                dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* gray_8_to_yuv_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_yuv_float_c
#define CONVERT dst[0] = RGB_8_TO_FLOAT(src[0]);\
                dst[1] = 0.0; \
                dst[2] = 0.0;

#include "../csp_packed_packed.h"

/* gray_8_to_y_8_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_y_8_c
#define CONVERT dst[0] = YJ_8_TO_Y_8(src[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* gray_8_to_yj_8_c */

static void gray_8_to_yj_8_c(gavl_video_convert_context_t * ctx)
  {
  gavl_video_frame_copy(&ctx->input_format, ctx->output_frame, ctx->input_frame);
  gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
  }

/* gray_8_to_y_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_8_to_y_16_c
#define CONVERT dst[0] = YJ_8_TO_Y_16(src[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* */

/* gray_16_to_yuy2_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_yuy2_c
#define CONVERT YJ_16_TO_Y_8(src[0], dst[0]);\
                dst[1] = 0x80;

#include "../csp_packed_packed.h"

/* gray_16_to_uyvy_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_uyvy_c
#define CONVERT YJ_16_TO_Y_8(src[0], dst[1]);\
                dst[0] = 0x80;

#include "../csp_packed_packed.h"

/* gray_16_to_yuva_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_yuva_32_c
#define CONVERT YJ_16_TO_Y_8(src[0], dst[0]);\
                dst[1] = 0x80; \
                dst[2] = 0x80; \
                dst[3] = 0xFF;

#include "../csp_packed_packed.h"

/* gray_16_to_yuva_64_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_yuva_64_c
#define CONVERT YJ_16_TO_Y_16(src[0], dst[0]);\
                dst[1] = 0x8000; \
                dst[2] = 0x8000; \
                dst[3] = 0xFFFF;

#include "../csp_packed_packed.h"

/* gray_16_to_yuva_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_yuva_float_c
#define CONVERT dst[0] = RGB_16_TO_FLOAT(src[0]);\
                dst[1] = 0.0; \
                dst[2] = 0.0; \
                dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* gray_16_to_yuv_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_yuv_float_c
#define CONVERT dst[0] = RGB_16_TO_FLOAT(src[0]);\
                dst[1] = 0.0; \
                dst[2] = 0.0;

#include "../csp_packed_packed.h"

/* gray_16_to_y_8_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_y_8_c
#define CONVERT YJ_16_TO_Y_8(src[0], dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* gray_16_to_yj_8_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_yj_8_c
#define CONVERT RGB_16_TO_8(src[0], dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"


/* gray_16_to_y_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_16_to_y_16_c
#define CONVERT YJ_16_TO_Y_16(src[0], dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* */

/* gray_float_to_yuy2_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_yuy2_c
#define CONVERT Y_FLOAT_TO_8(src[0], dst[0]);\
                dst[1] = 0x80;

#include "../csp_packed_packed.h"

/* gray_float_to_uyvy_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_uyvy_c
#define CONVERT Y_FLOAT_TO_8(src[0], dst[1]);\
                dst[0] = 0x80;

#include "../csp_packed_packed.h"

/* gray_float_to_yuva_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_yuva_32_c
#define CONVERT Y_FLOAT_TO_8(src[0], dst[0]);\
                dst[1] = 0x80; \
                dst[2] = 0x80; \
                dst[3] = 0xFF;

#include "../csp_packed_packed.h"

/* gray_float_to_yuva_64_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_yuva_64_c
#define CONVERT Y_FLOAT_TO_16(src[0], dst[0]);\
                dst[1] = 0x8000; \
                dst[2] = 0x8000; \
                dst[3] = 0xFFFF;

#include "../csp_packed_packed.h"

/* gray_float_to_yuva_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_yuva_float_c
#define CONVERT dst[0]= src[0];\
                dst[1] = 0.0; \
                dst[2] = 0.0; \
                dst[3] = 1.0;

#include "../csp_packed_packed.h"

/* gray_float_to_yuv_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  1
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_yuv_float_c
#define CONVERT dst[0]= src[0];\
                dst[1] = 0.0; \
                dst[2] = 0.0;

#include "../csp_packed_packed.h"

/* gray_float_to_y_8_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_y_8_c
#define CONVERT Y_FLOAT_TO_8(src[0], dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* gray_float_to_yj_8_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_yj_8_c
#define CONVERT RGB_FLOAT_TO_8(src[0], dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"


/* gray_float_to_y_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  1
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME gray_float_to_y_16_c
#define CONVERT Y_FLOAT_TO_16(src[0], dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* GRAYA -> YUV Ignoring alpha */

/* graya_16_to_yuy2_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_yuy2_ia_c
#define CONVERT dst[0] = YJ_8_TO_Y_8(src[0]);\
                dst[1] = 0x80;

#include "../csp_packed_packed.h"

/* graya_16_to_yuv_float_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_yuv_float_ia_c
#define CONVERT dst[0] = RGB_8_TO_FLOAT(src[0]);\
                dst[1] = 0.0; \
                dst[2] = 0.0;

#include "../csp_packed_packed.h"

/* graya_16_to_uyvy_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_uyvy_ia_c
#define CONVERT dst[1] = YJ_8_TO_Y_8(src[0]);\
                dst[0] = 0x80;

#include "../csp_packed_packed.h"

/* graya_16_to_yuva_32_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_yuva_32_c
#define CONVERT dst[0] = YJ_8_TO_Y_8(src[0]);\
                dst[1] = 0x80; \
                dst[2] = 0x80; \
                dst[3] = src[1];

#include "../csp_packed_packed.h"

/* graya_16_to_yuva_64_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_yuva_64_c
#define CONVERT dst[0] = YJ_8_TO_Y_16(src[0]);\
                dst[1] = 0x8000; \
                dst[2] = 0x8000; \
                dst[3] = RGB_8_TO_16(src[1]);

#include "../csp_packed_packed.h"

/* graya_16_to_yuva_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_yuva_float_c
#define CONVERT dst[0] = RGB_8_TO_FLOAT(src[0]);\
                dst[1] = 0.0; \
                dst[2] = 0.0; \
                dst[3] = RGB_8_TO_FLOAT(src[1]);

#include "../csp_packed_packed.h"


/* graya_16_to_y_8_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_y_8_ia_c
#define CONVERT dst[0] = YJ_8_TO_Y_8(src[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* graya_16_to_yj_8_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_yj_8_ia_c
#define CONVERT dst[0] = src[0];

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* graya_16_to_y_16_ia_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_y_16_ia_c
#define CONVERT dst[0] = YJ_8_TO_Y_16(src[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* */

/* graya_32_to_yuy2_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_yuy2_ia_c
#define CONVERT YJ_16_TO_Y_8(src[0], dst[0]);\
                dst[1] = 0x80;

#include "../csp_packed_packed.h"

/* graya_32_to_yuv_float_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_yuv_float_ia_c
#define CONVERT dst[0] = RGB_16_TO_FLOAT(src[0]);\
                dst[1] = 0.0;\
                dst[2] = 0.0;

#include "../csp_packed_packed.h"

/* graya_32_to_uyvy_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_uyvy_ia_c
#define CONVERT YJ_16_TO_Y_8(src[0], dst[1]);\
                dst[0] = 0x80;

#include "../csp_packed_packed.h"

/* graya_32_to_yuva_32_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_yuva_32_c
#define CONVERT YJ_16_TO_Y_8(src[0], dst[0]);\
                dst[1] = 0x80; \
                dst[2] = 0x80; \
                RGB_16_TO_8(src[1], dst[3]);

#include "../csp_packed_packed.h"

/* graya_32_to_yuva_64_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_yuva_64_c
#define CONVERT YJ_16_TO_Y_16(src[0], dst[0]);\
                dst[1] = 0x8000; \
                dst[2] = 0x8000; \
                dst[3] = src[1];

#include "../csp_packed_packed.h"

/* graya_32_to_yuva_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_yuva_float_c
#define CONVERT dst[0] = RGB_16_TO_FLOAT(src[0]);\
                dst[1] = 0.0; \
                dst[2] = 0.0; \
                dst[3] = RGB_16_TO_FLOAT(src[1]);

#include "../csp_packed_packed.h"

/* graya_32_to_y_8_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_y_8_ia_c
#define CONVERT YJ_16_TO_Y_8(src[0], dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* graya_32_to_yj_8_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_yj_8_ia_c
#define CONVERT RGB_16_TO_8(src[0], dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"


/* graya_32_to_y_16_ia_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_y_16_ia_c
#define CONVERT YJ_16_TO_Y_16(src[0], dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* */

/* graya_float_to_yuy2_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_yuy2_ia_c
#define CONVERT Y_FLOAT_TO_8(src[0], dst[0]);\
                dst[1] = 0x80;

#include "../csp_packed_packed.h"

/* graya_float_to_yuv_float_ia_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_yuv_float_ia_c
#define CONVERT dst[0] = src[0];\
                dst[1] = 0.0;\
                dst[2] = 0.0;

#include "../csp_packed_packed.h"

/* graya_float_to_uyvy_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_uyvy_ia_c
#define CONVERT Y_FLOAT_TO_8(src[0], dst[1]);\
                dst[0] = 0x80;

#include "../csp_packed_packed.h"

/* graya_float_to_yuva_32_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_yuva_32_c
#define CONVERT Y_FLOAT_TO_8(src[0], dst[0]);\
                dst[1] = 0x80; \
                dst[2] = 0x80; \
                RGB_FLOAT_TO_8(src[1], dst[3]);

#include "../csp_packed_packed.h"

/* graya_float_to_yuva_64_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_yuva_64_c
#define CONVERT Y_FLOAT_TO_16(src[0], dst[0]);\
                dst[1] = 0x8000; \
                dst[2] = 0x8000; \
                RGB_FLOAT_TO_16(src[1], dst[3]);

#include "../csp_packed_packed.h"

/* graya_float_to_yuva_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_yuva_float_c
#define CONVERT dst[0] = src[0];\
                dst[1] = 0.0; \
                dst[2] = 0.0; \
                dst[3] = src[1];

#include "../csp_packed_packed.h"

/* graya_float_to_y_8_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_y_8_ia_c
#define CONVERT Y_FLOAT_TO_8(src[0], dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* graya_float_to_yj_8_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_yj_8_ia_c
#define CONVERT RGB_FLOAT_TO_8(src[0], dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"


/* graya_float_to_y_16_ia_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_y_16_ia_c
#define CONVERT Y_FLOAT_TO_16(src[0], dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* GRAYA -> YUV */

/* graya_16_to_yuy2_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_yuy2_c
#define INIT int tmp; \
  INIT_GRAYA_16

#define CONVERT \
  GRAYA_16_TO_GRAY_8(src[0], src[1], tmp); \
  dst[0] = YJ_8_TO_Y_8(tmp); \
  dst[1] = 0x80;

#include "../csp_packed_packed.h"

/* graya_16_to_yuv_float_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_yuv_float_c
#define INIT int tmp; \
  INIT_GRAYA_16

#define CONVERT \
  GRAYA_16_TO_GRAY_8(src[0], src[1], tmp); \
  dst[0] = RGB_8_TO_FLOAT(tmp); \
  dst[1] = 0.0; \
  dst[2] = 0.0;

#include "../csp_packed_packed.h"

/* graya_16_to_uyvy_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_uyvy_c
#define INIT int tmp; \
  INIT_GRAYA_16

#define CONVERT \
  GRAYA_16_TO_GRAY_8(src[0], src[1], tmp); \
  dst[1] = YJ_8_TO_Y_8(tmp); \
  dst[0] = 0x80;

#include "../csp_packed_packed.h"

/* graya_16_to_y_8_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_y_8_c
#define INIT int tmp; \
  INIT_GRAYA_16

#define CONVERT \
  GRAYA_16_TO_GRAY_8(src[0], src[1], tmp); \
  dst[0] = YJ_8_TO_Y_8(tmp);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* graya_16_to_yj_8_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_yj_8_c
#define INIT \
  INIT_GRAYA_16

#define CONVERT \
  GRAYA_16_TO_GRAY_8(src[0], src[1], dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* graya_16_to_y_16_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_16_to_y_16_c
#define INIT int tmp; \
  INIT_GRAYA_16

#define CONVERT \
  GRAYA_16_TO_GRAY_8(src[0], src[1], tmp); \
  dst[0] = YJ_8_TO_Y_16(tmp);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* */

/* graya_32_to_yuy2_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_yuy2_c
#define INIT int tmp; \
  INIT_GRAYA_32

#define CONVERT \
  GRAYA_32_TO_GRAY_16(src[0], src[1], tmp); \
  YJ_16_TO_Y_8(tmp, dst[0]); \
  dst[1] = 0x80;

#include "../csp_packed_packed.h"

/* graya_32_to_yuv_float_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_yuv_float_c
#define INIT int tmp; \
  INIT_GRAYA_32

#define CONVERT \
  GRAYA_32_TO_GRAY_16(src[0], src[1], tmp); \
  dst[0] = RGB_16_TO_FLOAT(tmp); \
  dst[1] = 0.0;\
  dst[2] = 0.0;

#include "../csp_packed_packed.h"

/* graya_32_to_uyvy_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_uyvy_c
#define INIT int tmp; \
  INIT_GRAYA_32

#define CONVERT \
  GRAYA_32_TO_GRAY_16(src[0], src[1], tmp); \
  YJ_16_TO_Y_8(tmp, dst[1]); \
  dst[0] = 0x80;

#include "../csp_packed_packed.h"

/* graya_32_to_y_8_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_y_8_c
#define INIT int tmp; \
  INIT_GRAYA_32

#define CONVERT \
  GRAYA_32_TO_GRAY_16(src[0], src[1], tmp); \
  YJ_16_TO_Y_8(tmp, dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* graya_32_to_yj_8_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_yj_8_c
#define INIT int tmp; \
  INIT_GRAYA_32

#define CONVERT \
  GRAYA_32_TO_GRAY_16(src[0], src[1], tmp); \
  RGB_16_TO_8(tmp, dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* graya_32_to_y_16_c */

#define IN_TYPE  uint16_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_32_to_y_16_c
#define INIT int tmp; \
  INIT_GRAYA_32

#define CONVERT \
  GRAYA_32_TO_GRAY_16(src[0], src[1], tmp); \
  YJ_16_TO_Y_16(tmp, dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"



/* */

/* graya_float_to_yuy2_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_yuy2_c
#define INIT float tmp; \
  INIT_GRAYA_FLOAT

#define CONVERT \
  GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], tmp); \
  Y_FLOAT_TO_8(tmp, dst[0]); \
  dst[1] = 0x80;

#include "../csp_packed_packed.h"

/* graya_float_to_yuv_float_c */

#define IN_TYPE  float
#define OUT_TYPE float
#define IN_ADVANCE  2
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_yuv_float_c
#define INIT \
  INIT_GRAYA_FLOAT

#define CONVERT \
  GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], dst[0]); \
  dst[1] = 0.0;\
  dst[2] = 0.0;

#include "../csp_packed_packed.h"

/* graya_float_to_uyvy_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 2
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_uyvy_c
#define INIT float tmp; \
  INIT_GRAYA_FLOAT

#define CONVERT \
  GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], tmp); \
  Y_FLOAT_TO_8(tmp, dst[1]); \
  dst[0] = 0x80;

#include "../csp_packed_packed.h"

/* graya_float_to_y_8_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_y_8_c
#define INIT float tmp; \
  INIT_GRAYA_FLOAT

#define CONVERT \
  GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], tmp); \
  Y_FLOAT_TO_8(tmp, dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* graya_float_to_yj_8_c */

#define IN_TYPE  float
#define OUT_TYPE uint8_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_yj_8_c
#define INIT float tmp; \
  INIT_GRAYA_FLOAT

#define CONVERT \
  GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], tmp);\
  YJ_FLOAT_TO_8(tmp, dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"

/* graya_float_to_y_16_c */

#define IN_TYPE  float
#define OUT_TYPE uint16_t
#define IN_ADVANCE  2
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME graya_float_to_y_16_c
#define INIT float tmp; \
  INIT_GRAYA_FLOAT

#define CONVERT \
  GRAYA_FLOAT_TO_GRAY_FLOAT(src[0], src[1], tmp); \
  Y_FLOAT_TO_16(tmp, dst[0]);

#define CLEANUP gavl_video_frame_clear_mask(ctx->output_frame, &ctx->output_format, CLEAR_MASK_PLANE_1 | CLEAR_MASK_PLANE_2);
#include "../csp_packed_packed.h"




void gavl_init_gray_yuv_funcs_c(gavl_pixelformat_function_table_t * tab,
                                const gavl_video_options_t * opt)
  {
  if(opt->alpha_mode == GAVL_ALPHA_BLEND_COLOR)
    {
    tab->graya_16_to_yuy2 = graya_16_to_yuy2_c;
    tab->graya_16_to_uyvy = graya_16_to_uyvy_c;
    tab->graya_16_to_y_8 = graya_16_to_y_8_c;
    tab->graya_16_to_yj_8 = graya_16_to_yj_8_c;
    tab->graya_16_to_y_16 = graya_16_to_y_16_c;
    tab->graya_32_to_yuy2 = graya_32_to_yuy2_c;
    tab->graya_32_to_uyvy = graya_32_to_uyvy_c;
    tab->graya_32_to_y_8 = graya_32_to_y_8_c;
    tab->graya_32_to_yj_8 = graya_32_to_yj_8_c;
    tab->graya_32_to_y_16 = graya_32_to_y_16_c;
    tab->graya_float_to_yuy2 = graya_float_to_yuy2_c;
    tab->graya_float_to_uyvy = graya_float_to_uyvy_c;
    tab->graya_float_to_y_8 = graya_float_to_y_8_c;
    tab->graya_float_to_yj_8 = graya_float_to_yj_8_c;
    tab->graya_float_to_y_16 = graya_float_to_y_16_c;
    tab->graya_16_to_yuv_float = graya_16_to_yuv_float_c;
    tab->graya_32_to_yuv_float = graya_32_to_yuv_float_c;
    tab->graya_float_to_yuv_float = graya_float_to_yuv_float_c;
    }
  else
    {
    tab->graya_16_to_yuy2 = graya_16_to_yuy2_ia_c;
    tab->graya_16_to_uyvy = graya_16_to_uyvy_ia_c;
    tab->graya_16_to_y_8 = graya_16_to_y_8_ia_c;
    tab->graya_16_to_yj_8 = graya_16_to_yj_8_ia_c;
    tab->graya_16_to_y_16 = graya_16_to_y_16_ia_c;
    tab->graya_32_to_yuy2 = graya_32_to_yuy2_ia_c;
    tab->graya_32_to_uyvy = graya_32_to_uyvy_ia_c;
    tab->graya_32_to_y_8 = graya_32_to_y_8_ia_c;
    tab->graya_32_to_yj_8 = graya_32_to_yj_8_ia_c;
    tab->graya_32_to_y_16 = graya_32_to_y_16_ia_c;
    tab->graya_float_to_yuy2 = graya_float_to_yuy2_ia_c;
    tab->graya_float_to_uyvy = graya_float_to_uyvy_ia_c;
    tab->graya_float_to_y_8 = graya_float_to_y_8_ia_c;
    tab->graya_float_to_yj_8 = graya_float_to_yj_8_ia_c;
    tab->graya_float_to_y_16 = graya_float_to_y_16_ia_c;
    tab->graya_16_to_yuv_float = graya_16_to_yuv_float_ia_c;
    tab->graya_32_to_yuv_float = graya_32_to_yuv_float_ia_c;
    tab->graya_float_to_yuv_float = graya_float_to_yuv_float_ia_c;
    }
  
  tab->gray_8_to_yuy2 = gray_8_to_yuy2_c;
  tab->gray_8_to_yuva_32 = gray_8_to_yuva_32_c;
  tab->gray_8_to_uyvy = gray_8_to_uyvy_c;
  tab->gray_8_to_y_8 = gray_8_to_y_8_c;
  tab->gray_8_to_yj_8 = gray_8_to_yj_8_c;
  tab->gray_8_to_y_16 = gray_8_to_y_16_c;
  tab->gray_8_to_yuva_64 = gray_8_to_yuva_64_c;
  tab->gray_8_to_yuva_float = gray_8_to_yuva_float_c;
  tab->gray_8_to_yuv_float = gray_8_to_yuv_float_c;

  tab->gray_16_to_yuy2 = gray_16_to_yuy2_c;
  tab->gray_16_to_yuva_32 = gray_16_to_yuva_32_c;
  tab->gray_16_to_uyvy = gray_16_to_uyvy_c;
  tab->gray_16_to_y_8 = gray_16_to_y_8_c;
  tab->gray_16_to_yj_8 = gray_16_to_yj_8_c;
  tab->gray_16_to_y_16 = gray_16_to_y_16_c;
  tab->gray_16_to_yuva_64 = gray_16_to_yuva_64_c;
  tab->gray_16_to_yuva_float = gray_16_to_yuva_float_c;
  tab->gray_16_to_yuv_float = gray_16_to_yuv_float_c;

  tab->gray_float_to_yuy2 = gray_float_to_yuy2_c;
  tab->gray_float_to_yuva_32 = gray_float_to_yuva_32_c;
  tab->gray_float_to_uyvy = gray_float_to_uyvy_c;
  tab->gray_float_to_y_8 = gray_float_to_y_8_c;
  tab->gray_float_to_yj_8 = gray_float_to_yj_8_c;
  tab->gray_float_to_y_16 = gray_float_to_y_16_c;
  tab->gray_float_to_yuva_64 = gray_float_to_yuva_64_c;
  tab->gray_float_to_yuva_float = gray_float_to_yuva_float_c;
  tab->gray_float_to_yuv_float = gray_float_to_yuv_float_c;

  tab->graya_16_to_yuva_32 = graya_16_to_yuva_32_c;
  tab->graya_16_to_yuva_64 = graya_16_to_yuva_64_c;
  tab->graya_16_to_yuva_float = graya_16_to_yuva_float_c;

  tab->graya_32_to_yuva_32 = graya_32_to_yuva_32_c;
  tab->graya_32_to_yuva_64 = graya_32_to_yuva_64_c;
  tab->graya_32_to_yuva_float = graya_32_to_yuva_float_c;

  tab->graya_float_to_yuva_32 = graya_float_to_yuva_32_c;
  tab->graya_float_to_yuva_64 = graya_float_to_yuva_64_c;
  tab->graya_float_to_yuva_float = graya_float_to_yuva_float_c;
  
  
  }
