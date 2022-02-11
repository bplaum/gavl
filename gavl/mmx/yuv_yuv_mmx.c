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

#include "mmx_macros.h"

static const mmx_t mmx_00ffw =   { 0x00ff00ff00ff00ffLL };
static const mmx_t mmx_ff00w =   { 0xff00ff00ff00ff00LL };

#ifndef MMXEXT



/* Pixel conversion Macros */

/*
 *   Convert a single scanline from YUY2 to separate planes
 *   preserving the horizonal subsampling.
 *   This macro does practically the same as the LOAD_YUY2
 *   macro in _yuv_rgb_mmx.c
 */

#define YUY2_TO_YUV_PLANAR movq_m2r(*src,mm0);\
                           movq_m2r(*(src+8),mm1);\
                           movq_r2r(mm0,mm2);/*       mm2: V2 Y3 U2 Y2 V0 Y1 U0 Y0 */\
                           pand_m2r(mmx_00ffw,mm2);/* mm2: 00 Y3 00 Y2 00 Y1 00 Y0 */\
                           pxor_r2r(mm4, mm4);/*      Zero mm4 */\
                           packuswb_r2r(mm4,mm2);/*   mm2: 00 00 00 00 Y3 Y2 Y1 Y0 */\
                           movq_r2r(mm1,mm3);/*       mm3: V6 Y7 U6 Y6 V4 Y5 U4 Y4 */\
                           pand_m2r(mmx_00ffw,mm3);/* mm3: 00 Y7 00 Y6 00 Y5 00 Y4 */\
                           pxor_r2r(mm6, mm6);/*      Zero mm6 */\
                           packuswb_r2r(mm3,mm6);/*   mm6: Y7 Y6 Y5 Y4 00 00 00 00 */\
                           por_r2r(mm2,mm6);/*        mm6: Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */\
                           psrlw_i2r(8,mm0);/*        mm0: 00 V2 00 U2 00 V0 00 U0 */\
                           psrlw_i2r(8,mm1);/*        mm1: 00 V6 00 U6 00 V4 00 U4 */\
                           packuswb_r2r(mm1,mm0);/*   mm0: V6 U6 V4 U4 V2 U2 V0 U0 */\
                           movq_r2r(mm0,mm1);/*       mm1: V6 U6 V4 U4 V2 U2 V0 U0 */\
                           pand_m2r(mmx_00ffw,mm0);/* mm0: 00 U6 00 U4 00 U2 00 U0 */\
                           psrlw_i2r(8,mm1);/*        mm1: 00 V6 00 V4 00 V2 00 V0 */\
                           packuswb_r2r(mm4,mm0);/*   mm0: 00 00 00 00 U6 U4 U2 U0 */\
                           packuswb_r2r(mm4,mm1);/*   mm1: 00 00 00 00 V6 V4 V2 V0 */\
                           MOVQ_R2M(mm6, *dst_y);\
                           movd_r2m(mm0, *dst_u);\
                           movd_r2m(mm1, *dst_v);

#define UYVY_TO_YUV_PLANAR movq_m2r(*src,mm0);/*      mm0: Y3 V2 Y2 U2 Y1 V0 Y0 U0 */\
                           movq_m2r(*(src+8),mm1);/*  mm1: Y7 V6 Y6 U6 Y5 V4 Y4 U4 */\
                           movq_r2r(mm0,mm2);/*       mm2: Y3 V2 Y2 U2 Y1 V0 Y0 U0 */\
                           pand_m2r(mmx_ff00w,mm2);/* mm2: Y3 00 Y2 00 Y1 00 Y0 00 */\
                           psrlw_i2r(8,mm2);/*        mm0: 00 Y3 00 Y2 00 Y1 00 Y0 */\
                           pxor_r2r(mm4, mm4);/*      Zero mm4 */                    \
                           packuswb_r2r(mm4,mm2);/*   mm2: 00 00 00 00 Y3 Y2 Y1 Y0 */\
                           movq_r2r(mm1,mm3);/*       mm3: Y7 V6 Y6 U6 Y5 V4 Y4 U4 */\
                           pand_m2r(mmx_ff00w,mm3);/* mm3: Y7 00 Y6 00 Y5 00 Y4 00 */\
                           psrlw_i2r(8,mm3);/*        mm3: 00 Y3 00 Y2 00 Y1 00 Y0 */\
                           pxor_r2r(mm6, mm6);/*      Zero mm6 */\
                           packuswb_r2r(mm3,mm6);/*   mm6: Y7 Y6 Y5 Y4 00 00 00 00 */\
                           por_r2r(mm2,mm6);/*        mm6: Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */\
                           pand_m2r(mmx_00ffw,mm0);/* mm0: 00 V2 00 U2 00 V0 00 U0 */\
                           pand_m2r(mmx_00ffw,mm1);/* mm1: 00 V6 00 U6 00 V4 00 U4 */\
                           packuswb_r2r(mm1,mm0);/*   mm0: V6 U6 V4 U4 V2 U2 V0 U0 */\
                           movq_r2r(mm0,mm1);/*       mm1: V6 U6 V4 U4 V2 U2 V0 U0 */\
                           pand_m2r(mmx_00ffw,mm0);/* mm0: 00 U6 00 U4 00 U2 00 U0 */\
                           psrlw_i2r(8,mm1);/*        mm1: 00 V6 00 V4 00 V2 00 V0 */\
                           packuswb_r2r(mm4,mm0);/*   mm0: 00 00 00 00 U6 U4 U2 U0 */\
                           packuswb_r2r(mm4,mm1);/*   mm1: 00 00 00 00 V6 V4 V2 V0 */\
                           MOVQ_R2M(mm6, *dst_y);\
                           movd_r2m(mm0, *dst_u);\
                           movd_r2m(mm1, *dst_v);


/*
 *  Convert a single scanline from YUY2 to a luminance plane
 *  preserving the horizontal subsampling
 */

#define YUY2_TO_Y_PLANAR movq_m2r(*src,mm0);\
                         movq_m2r(*(src+8),mm1);\
                         movq_r2r(mm0,mm2);/*           mm2: V2 Y3 U2 Y2 V0 Y1 U0 Y0 */\
                         pand_m2r(mmx_00ffw,mm2);/*     mm2: 00 Y3 00 Y2 00 Y1 00 Y0 */\
                         pxor_r2r(mm4, mm4);/*          Zero mm4 */\
                         packuswb_r2r(mm4,mm2);/*       mm2: 00 00 00 00 Y3 Y2 Y1 Y0 */\
                         movq_r2r(mm1,mm3);/*           mm3: V6 Y7 U6 Y6 V4 Y5 U4 Y4 */\
                         pand_m2r(mmx_00ffw,mm3);/*     mm3: 00 Y7 00 Y6 00 Y5 00 Y4 */\
                         pxor_r2r(mm6, mm6);/*          Zero mm6 */\
                         packuswb_r2r(mm3,mm6);/*       mm6: Y7 Y6 Y5 Y4 00 00 00 00 */\
                         por_r2r(mm2,mm6);/*            mm6: Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */\
                         MOVQ_R2M(mm6, *dst_y);

#define UYVY_TO_Y_PLANAR movq_m2r(*src,mm0);/*          mm0: Y3 V2 Y2 U2 Y1 V0 Y0 U0 */\
                         movq_m2r(*(src+8),mm1);/*      mm1: Y7 V6 Y6 U6 Y5 V4 Y4 U4 */\
                         movq_r2r(mm0,mm2);/*           mm2: Y3 V2 Y2 U2 Y1 V0 Y0 U0 */\
                         pand_m2r(mmx_ff00w,mm2);/*     mm2: Y3 00 Y2 00 Y1 00 Y0 00 */\
                         psrlw_i2r(8,mm2);/*            mm2: 00 Y3 00 Y2 00 Y1 00 Y0 */\
                         pxor_r2r(mm4, mm4);/*          Zero mm4 */\
                         packuswb_r2r(mm4,mm2);/*       mm2: 00 00 00 00 Y3 Y2 Y1 Y0 */\
                         movq_r2r(mm1,mm3);/*           mm3: Y7 V6 Y6 U6 Y5 V4 Y4 U4 */\
                         pand_m2r(mmx_ff00w,mm3);/*     mm3: Y7 00 Y6 00 Y5 00 Y4 00 */\
                         psrlw_i2r(8,mm3);/*            mm3: 00 Y7 00 Y6 00 Y5 00 Y4 */\
                         pxor_r2r(mm6, mm6);/*          Zero mm6 */\
                         packuswb_r2r(mm3,mm6);/*       mm6: Y7 Y6 Y5 Y4 00 00 00 00 */\
                         por_r2r(mm2,mm6);/*            mm6: Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */\
                         MOVQ_R2M(mm6, *dst_y);

#define PLANAR_TO_YUY2   movq_m2r(*src_y, mm0);/*   mm0: Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */\
                         movd_m2r(*src_u, mm1);/*   mm1: 00 00 00 00 U6 U4 U2 U0 */\
                         movd_m2r(*src_v, mm2);/*   mm2: 00 00 00 00 V6 V4 V2 V0 */\
                         pxor_r2r(mm3, mm3);/*      Zero mm3                     */\
                         movq_r2r(mm0, mm7);/*      mm7: Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */\
                         punpcklbw_r2r(mm3, mm0);/* mm0: 00 Y3 00 Y2 00 Y1 00 Y0 */\
                         punpckhbw_r2r(mm3, mm7);/* mm7: 00 Y7 00 Y6 00 Y5 00 Y4 */\
                         pxor_r2r(mm4, mm4);     /* Zero mm4                     */\
                         punpcklbw_r2r(mm1, mm4);/* mm4: U6 00 U4 00 U2 00 U0 00 */\
                         pxor_r2r(mm5, mm5);     /* Zero mm5                     */\
                         punpcklbw_r2r(mm2, mm5);/* mm5: V6 00 V4 00 V2 00 V0 00 */\
                         movq_r2r(mm4, mm6);/*      mm6: U6 00 U4 00 U2 00 U0 00 */\
                         punpcklwd_r2r(mm3, mm6);/* mm6: 00 00 U2 00 00 00 U0 00 */\
                         por_r2r(mm6, mm0);      /* mm0: 00 Y3 U2 Y2 00 Y1 U0 Y0 */\
                         punpcklwd_r2r(mm3, mm4);/* mm4: 00 00 U6 00 00 00 U4 00 */\
                         por_r2r(mm4, mm7);      /* mm7: 00 Y7 U6 Y6 00 Y5 U4 Y4 */\
                         pxor_r2r(mm6, mm6);     /* Zero mm6                     */\
                         punpcklwd_r2r(mm5, mm6);/* mm6: V2 00 00 00 V0 00 00 00 */\
                         por_r2r(mm6, mm0);      /* mm0: V2 Y3 U2 Y2 V0 Y1 U0 Y0 */\
                         punpckhwd_r2r(mm5, mm3);/* mm3: V6 00 00 00 V4 00 00 00 */\
                         por_r2r(mm3, mm7);      /* mm7: V6 Y7 U6 Y6 V4 Y5 U4 Y4 */\
                         MOVQ_R2M(mm0, *dst);\
                         MOVQ_R2M(mm7, *(dst+8));

#define PLANAR_TO_UYVY   movq_m2r(*src_y, mm0);/*   mm0: Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */\
                         movd_m2r(*src_u, mm1);/*   mm1: 00 00 00 00 U6 U4 U2 U0 */\
                         movd_m2r(*src_v, mm2);/*   mm2: 00 00 00 00 V6 V4 V2 V0 */\
                         pxor_r2r(mm3, mm3);/*      Zero mm3                     */\
                         movq_r2r(mm0, mm7);/*      mm7: Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */\
                         punpcklbw_r2r(mm3, mm0);/* mm0: 00 Y3 00 Y2 00 Y1 00 Y0 */\
                         punpckhbw_r2r(mm3, mm7);/* mm7: 00 Y7 00 Y6 00 Y5 00 Y4 */\
                         psllw_i2r(8,mm0);/*        mm0: Y3 00 Y2 00 Y1 00 Y0 00 */\
                         psllw_i2r(8,mm7);/*        mm7: Y7 00 Y6 00 Y5 00 Y4 00 */\
                         pxor_r2r(mm4, mm4);     /* Zero mm4                     */\
                         punpcklbw_r2r(mm1, mm4);/* mm4: U6 00 U4 00 U2 00 U0 00 */\
                         pxor_r2r(mm5, mm5);     /* Zero mm5                     */\
                         punpcklbw_r2r(mm2, mm5);/* mm5: V6 00 V4 00 V2 00 V0 00 */\
                         movq_r2r(mm4, mm6);/*      mm6: U6 00 U4 00 U2 00 U0 00 */\
                         punpcklwd_r2r(mm3, mm6);/* mm6: 00 00 U2 00 00 00 U0 00 */\
                         psrlw_i2r(8,mm6);/*        mm6: 00 00 00 U2 00 00 00 U0 */\
                         por_r2r(mm6, mm0);      /* mm0: Y3 00 Y2 U2 Y1 00 Y0 U0 */\
                         punpcklwd_r2r(mm3, mm4);/* mm4: 00 00 U6 00 00 00 U4 00 */\
                         psrlw_i2r(8,mm4);/*        mm4: 00 00 00 U6 00 00 00 U4 */\
                         por_r2r(mm4, mm7);      /* mm7: Y7 00 Y6 U6 Y5 00 Y4 U4 */\
                         pxor_r2r(mm6, mm6);     /* Zero mm6                     */\
                         punpcklwd_r2r(mm5, mm6);/* mm6: V2 00 00 00 V0 00 00 00 */\
                         psrlw_i2r(8,mm6);/*        mm6: 00 V2 00 00 00 V0 00 00 */\
                         por_r2r(mm6, mm0);      /* mm0: Y3 V2 Y2 U2 Y1 V0 Y0 U0 */\
                         punpckhwd_r2r(mm5, mm3);/* mm3: V6 00 00 00 V4 00 00 00 */\
                         psrlw_i2r(8,mm3);/*        mm3: 00 V6 00 00 00 V4 00 00 */\
                         por_r2r(mm3, mm7);      /* mm7: Y7 V6 Y6 U6 Y5 V4 Y4 U4 */\
                         MOVQ_R2M(mm0, *dst);\
                         MOVQ_R2M(mm7, *(dst+8));


/* YUY2 -> Planar */

#define FUNC_NAME      yuy2_to_yuv_420_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  8
#define OUT_ADVANCE_UV 4
#define NUM_PIXELS     8
#define CONVERT_YUV    \
  YUY2_TO_YUV_PLANAR

#define CONVERT_Y      \
  YUY2_TO_Y_PLANAR

#define CHROMA_SUB     2
#define CLEANUP        emms();

#include "../csp_packed_planar.h"

#define FUNC_NAME      yuy2_to_yuv_422_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  8
#define OUT_ADVANCE_UV 4
#define NUM_PIXELS     8
#define CONVERT_YUV    \
  YUY2_TO_YUV_PLANAR

#define CHROMA_SUB     1
#define CLEANUP        emms();

#include "../csp_packed_planar.h"

/* UYVY -> Planar */

#define FUNC_NAME      uyvy_to_yuv_420_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  8
#define OUT_ADVANCE_UV 4
#define NUM_PIXELS     8
#define CONVERT_YUV    \
  UYVY_TO_YUV_PLANAR

#define CONVERT_Y      \
  UYVY_TO_Y_PLANAR

#define CHROMA_SUB     2
#define CLEANUP        emms();

#include "../csp_packed_planar.h"

#define FUNC_NAME      uyvy_to_yuv_422_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  8
#define OUT_ADVANCE_UV 4
#define NUM_PIXELS     8
#define CONVERT_YUV    \
  UYVY_TO_YUV_PLANAR

#define CHROMA_SUB     1
#define CLEANUP        emms();

#include "../csp_packed_planar.h"

/* Planar -> YUY2 */

#define FUNC_NAME     yuv_422_p_to_yuy2_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
    PLANAR_TO_YUY2

#define CHROMA_SUB 1

// #define INIT
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_420_p_to_yuy2_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
    PLANAR_TO_YUY2

#define CHROMA_SUB 2

// #define INIT
#define CLEANUP emms();

#include "../csp_planar_packed.h"

/* Planar -> UYVY */

#define FUNC_NAME     yuv_422_p_to_uyvy_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
    PLANAR_TO_UYVY

#define CHROMA_SUB 1

// #define INIT
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_420_p_to_uyvy_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
    PLANAR_TO_UYVY

#define CHROMA_SUB 2

// #define INIT
#define CLEANUP emms();

#include "../csp_planar_packed.h"

  

void
gavl_init_yuv_yuv_funcs_mmx(gavl_pixelformat_function_table_t * tab, int width,
                            const gavl_video_options_t * opt)

  {
  if(width % 8)
    return;
  
  /* These are as good as the C-Functions. Higher quality will invoke a scaler,
     so this function won't get called anyway */
  
  tab->yuy2_to_yuv_420_p      = yuy2_to_yuv_420_p_mmx;
  tab->yuy2_to_yuv_422_p      = yuy2_to_yuv_422_p_mmx;

  tab->uyvy_to_yuv_420_p      = uyvy_to_yuv_420_p_mmx;
  tab->uyvy_to_yuv_422_p      = uyvy_to_yuv_422_p_mmx;
  
  tab->yuv_420_p_to_yuy2      = yuv_420_p_to_yuy2_mmx;
  tab->yuv_420_p_to_uyvy      = yuv_420_p_to_uyvy_mmx;

  
  tab->yuv_422_p_to_yuy2      = yuv_422_p_to_yuy2_mmx;
  tab->yuv_422_p_to_uyvy      = yuv_422_p_to_uyvy_mmx;

  }

#else // MMXEXT

void
gavl_init_yuv_yuv_funcs_mmxext(gavl_pixelformat_function_table_t * tab,
                               int width, const gavl_video_options_t * opt)
  {

  }
#endif
