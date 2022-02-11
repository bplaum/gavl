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

#define INTERPOLATE_USE_16
#include "interpolate.h"

#ifndef MMXEXT

static const mmx_t rgb_rgb_rgb32_upper_mask =       { 0x00ff000000ff0000LL };
static const mmx_t rgb_rgb_rgb32_middle_mask =      { 0x0000ff000000ff00LL };
static const mmx_t rgb_rgb_rgb32_lower_mask =       { 0x000000ff000000ffLL };

static const mmx_t rgb_rgb_rgb32_upper_lower_mask = { 0x00ff00ff00ff00ffLL };

static const mmx_t rgba32_alpha_mask =      { 0xFF000000FF000000LL };

static const mmx_t rgb_rgb_rgb16_upper_mask =   { 0xf800f800f800f800LL };
static const mmx_t rgb_rgb_rgb16_middle_mask =  { 0x07e007e007e007e0LL };
static const mmx_t rgb_rgb_rgb16_lower_mask =   { 0x001f001f001f001fLL };

static const mmx_t rgb_rgb_rgb15_upper_mask =   { 0x7C007C007C007C00LL };
static const mmx_t rgb_rgb_rgb15_middle_mask =  { 0x03e003e003e003e0LL };
static const mmx_t rgb_rgb_rgb15_lower_mask =   { 0x001f001f001f001fLL };

static const mmx_t rgb_rgb_rgb15_up_mask =      { 0x7fe07fe07fe07fe0LL };

static const mmx_t rgb_rgb_rgb16_up_mask =      { 0xffe0ffe0ffe0ffe0LL };

/*
 *   Macros for pixel conversion
 */

/*
 *   Load pixels for 32 bit formats
 */

#define LOAD_32 movq_m2r(*src,mm0);/*      mm0: 00 B1 G1 R1 00 B0 G0 R0 */\
                movq_m2r(*(src+8),mm1);/*  mm1: 00 B3 G3 R3 00 B2 G2 R2 */\
                movq_m2r(*(src+16),mm2);/* mm2: 00 B5 G5 R5 00 B4 G4 R4 */\
                movq_m2r(*(src+24),mm3);/* mm3: 00 B7 G7 R7 00 B6 G6 R6 */

/*
 *   Load pixels for 24 bit formats
 */

static const mmx_t rgb_rgb_rgb24_l = { 0x0000000000FFFFFFLL };
static const mmx_t rgb_rgb_rgb24_u = { 0x0000FFFFFF000000LL };


#define LOAD_24 movq_m2r(*src,mm0);\
                movd_m2r(*(src+8),mm1);\
                movq_r2r(mm0, mm4);\
                psrlq_i2r(48, mm4);\
                psllq_i2r(16 , mm1);\
                por_r2r(mm4, mm1);\
                movq_r2r(mm0, mm4);\
                pand_m2r(rgb_rgb_rgb24_l, mm0);\
                pand_m2r(rgb_rgb_rgb24_u, mm4);\
                psllq_i2r(8, mm4);\
                por_r2r(mm4, mm0);\
                movq_r2r(mm1, mm4);\
                pand_m2r(rgb_rgb_rgb24_l, mm1);\
                pand_m2r(rgb_rgb_rgb24_u, mm4);\
                psllq_i2r(8, mm4);\
                por_r2r(mm4, mm1);\
                movq_m2r(*(src+12),mm2);\
                movd_m2r(*(src+20),mm3);\
                movq_r2r(mm2, mm4);\
                psrlq_i2r(48, mm4);\
                psllq_i2r(16 , mm3);\
                por_r2r(mm4, mm3);\
                movq_r2r(mm2, mm4);\
                pand_m2r(rgb_rgb_rgb24_l, mm2);\
                pand_m2r(rgb_rgb_rgb24_u, mm4);\
                psllq_i2r(8, mm4);\
                por_r2r(mm4, mm2);\
                movq_r2r(mm3, mm4);\
                pand_m2r(rgb_rgb_rgb24_l, mm3);\
                pand_m2r(rgb_rgb_rgb24_u, mm4);\
                psllq_i2r(8, mm4);\
                por_r2r(mm4, mm3);



/*
 *   Load pixels for 15/16 bit formats
 */

#define LOAD_16 PREFETCH(src);\
                movq_m2r(*src,mm0);/*     mm0: P3 P3 P2 P2 P1 P1 P0 P0 */\
                movq_m2r(*(src+8),mm1);/* mm1: P7 P7 P6 P6 P5 P5 P4 P4  */

/*
 *   Write pixels for 32 bit formats
 */

#define WRITE_32 MOVQ_R2M(mm0,*dst);/*      mm0: 00 B1 G1 R1 00 B0 G0 R0 */\
                 MOVQ_R2M(mm1,*(dst+8));/*  mm1: 00 B3 G3 R3 00 B2 G2 R2 */\
                 MOVQ_R2M(mm2,*(dst+16));/* mm2: 00 B5 G5 R5 00 B4 G4 R4 */\
                 MOVQ_R2M(mm3,*(dst+24));/* mm3: 00 B7 G7 R7 00 B6 G6 R6 */

/*
 *  Write RGBA32, must call INIT_WRITE_RGBA32 before
 */

#define INIT_WRITE_RGBA_32 movq_m2r(rgba32_alpha_mask, mm7);

#define WRITE_RGBA_32 por_r2r(mm7, mm0);\
                      por_r2r(mm7, mm1);\
                      por_r2r(mm7, mm2);\
                      por_r2r(mm7, mm3);\
                      MOVQ_R2M(mm0,*dst);/*      mm0: 00 B1 G1 R1 00 B0 G0 R0 */\
                      MOVQ_R2M(mm1,*(dst+8));/*  mm1: 00 B3 G3 R3 00 B2 G2 R2 */\
                      MOVQ_R2M(mm2,*(dst+16));/* mm2: 00 B5 G5 R5 00 B4 G4 R4 */\
                      MOVQ_R2M(mm3,*(dst+24));/* mm3: 00 B7 G7 R7 00 B6 G6 R6 */

/*
 *   Write pixels for 24 bit formats (Start format is RGB32)
 */

static const mmx_t rgb_rgb_lower_dword   = { 0x00000000FFFFFFFFLL };
static const mmx_t rgb_rgb_upper_dword   = { 0xFFFFFFFF00000000LL };

static const mmx_t write_24_lower_mask   = { 0x0000000000FFFFFFLL };
static const mmx_t write_24_upper_mask   = { 0x00FFFFFF00000000LL };

#define WRITE_24 movq_r2r(mm0, mm4);\
                 pand_m2r(write_24_upper_mask, mm4);\
                 pand_m2r(write_24_lower_mask, mm0);\
                 psrlq_i2r(8, mm4);\
                 por_r2r(mm4, mm0);\
                 movq_r2r(mm1, mm4);\
                 pand_m2r(write_24_upper_mask, mm4);\
                 pand_m2r(write_24_lower_mask, mm1);\
                 psrlq_i2r(8, mm4);\
                 por_r2r(mm4, mm1);\
                 movq_r2r(mm2, mm4);\
                 pand_m2r(write_24_upper_mask, mm4);\
                 pand_m2r(write_24_lower_mask, mm2);\
                 psrlq_i2r(8, mm4);\
                 por_r2r(mm4, mm2);\
                 movq_r2r(mm3, mm4);\
                 pand_m2r(write_24_upper_mask, mm4);\
                 pand_m2r(write_24_lower_mask, mm3);\
                 psrlq_i2r(8, mm4);\
                 por_r2r(mm4, mm3);\
                 movq_r2r(mm1, mm5);\
                 psllq_i2r(48, mm5);\
                 por_r2r(mm5, mm0);\
                 psrlq_i2r(16, mm1);\
                 movq_r2r(mm3, mm5);\
                 psllq_i2r(48, mm5);\
                 por_r2r(mm5, mm2);\
                 psrlq_i2r(16, mm3);\
                 MOVQ_R2M(mm0,*dst);\
                 movd_r2m(mm1,*(dst+8));\
                 MOVQ_R2M(mm2,*(dst+12));\
                 movd_r2m(mm3,*(dst+20));


/*
 *   Write pixels for 15/16 bit formats
 */

#define WRITE_16 MOVQ_R2M(mm0,*dst);/*     mm0: P3 P3 P2 P2 P1 P1 P0 P0 */\
                 MOVQ_R2M(mm1,*(dst+8));/* mm1: P7 P7 P6 P6 P5 P5 P4 P4  */

#define INIT_SWAP_16 movq_m2r(rgb_rgb_rgb16_upper_mask, mm4);\
                     movq_m2r(rgb_rgb_rgb16_lower_mask, mm5);\
                     movq_m2r(rgb_rgb_rgb16_middle_mask, mm6);

#define SWAP_16 movq_r2r(mm0, mm2);\
                movq_r2r(mm0, mm3);\
                pand_r2r(mm4, mm2);\
                pand_r2r(mm5, mm3);\
                pand_r2r(mm6, mm0);\
                psrlq_i2r(11,mm2);\
                psllq_i2r(11,mm3);\
                por_r2r(mm2,mm0);\
                por_r2r(mm3,mm0);\
                movq_r2r(mm1, mm2);\
                movq_r2r(mm1, mm3);\
                pand_r2r(mm4, mm2);\
                pand_r2r(mm5, mm3);\
                pand_r2r(mm6, mm1);\
                psrlq_i2r(11,mm2);\
                psllq_i2r(11,mm3);\
                por_r2r(mm2,mm1);\
                por_r2r(mm3,mm1);

#define INIT_SWAP_16_TO_15 movq_m2r(rgb_rgb_rgb16_upper_mask, mm4);\
                           movq_m2r(rgb_rgb_rgb16_lower_mask, mm5);\
                           movq_m2r(rgb_rgb_rgb16_middle_mask, mm6);\
                           movq_m2r(rgb_rgb_rgb15_middle_mask, mm7);

#define SWAP_16_TO_15 movq_r2r(mm0, mm2);\
                      movq_r2r(mm0, mm3);\
                      pand_r2r(mm4, mm2);\
                      pand_r2r(mm5, mm3);\
                      pand_r2r(mm6, mm0);\
                      psrlq_i2r(11,mm2);\
                      psllq_i2r(10,mm3);\
                      psrlq_i2r(1,mm0);\
                      pand_r2r(mm7, mm0);\
                      por_r2r(mm2,mm0);\
                      por_r2r(mm3,mm0);\
                      movq_r2r(mm1, mm2);\
                      movq_r2r(mm1, mm3);\
                      pand_r2r(mm4, mm2);\
                      pand_r2r(mm5, mm3);\
                      pand_r2r(mm6, mm1);\
                      psrlq_i2r(11,mm2);\
                      psllq_i2r(10,mm3);\
                      psrlq_i2r(1,mm1);\
                      pand_r2r(mm7, mm1);\
                      por_r2r(mm2,mm1);\
                      por_r2r(mm3,mm1);

#define INIT_SWAP_15 movq_m2r(rgb_rgb_rgb15_upper_mask, mm4);\
                     movq_m2r(rgb_rgb_rgb15_lower_mask, mm5);\
                     movq_m2r(rgb_rgb_rgb15_middle_mask, mm6);

#define SWAP_15 movq_r2r(mm0, mm2);\
                movq_r2r(mm0, mm3);\
                pand_r2r(mm4, mm2);\
                pand_r2r(mm5, mm3);\
                pand_r2r(mm6, mm0);\
                psrlq_i2r(10,mm2);\
                psllq_i2r(10,mm3);\
                por_r2r(mm2,mm0);\
                por_r2r(mm3,mm0);\
                movq_r2r(mm1, mm2);\
                movq_r2r(mm1, mm3);\
                pand_r2r(mm4, mm2);\
                pand_r2r(mm5, mm3);\
                pand_r2r(mm6, mm1);\
                psrlq_i2r(10,mm2);\
                psllq_i2r(10,mm3);\
                por_r2r(mm2,mm1);\
                por_r2r(mm3,mm1);

#define INIT_SWAP_15_TO_16 movq_m2r(rgb_rgb_rgb15_upper_mask, mm4);\
                           movq_m2r(rgb_rgb_rgb15_lower_mask, mm5);\
                           movq_m2r(rgb_rgb_rgb15_middle_mask, mm6);

#define SWAP_15_TO_16 movq_r2r(mm0, mm2);\
                      movq_r2r(mm0, mm3);\
                      pand_r2r(mm4, mm2);\
                      pand_r2r(mm5, mm3);\
                      pand_r2r(mm6, mm0);\
                      psrlq_i2r(10,mm2);\
                      psllq_i2r(11,mm3);\
                      psllq_i2r(1,mm0);\
                      por_r2r(mm2,mm0);\
                      por_r2r(mm3,mm0);\
                      movq_r2r(mm1, mm2);\
                      movq_r2r(mm1, mm3);\
                      pand_r2r(mm4, mm2);\
                      pand_r2r(mm5, mm3);\
                      pand_r2r(mm6, mm1);\
                      psrlq_i2r(10,mm2);\
                      psllq_i2r(11,mm3);\
                      psllq_i2r(1,mm1);\
                      por_r2r(mm2,mm1);\
                      por_r2r(mm3,mm1);

#define SWAP_32 movq_r2r(mm0, mm4);\
                movq_r2r(mm0, mm5);\
                pand_m2r(rgb_rgb_rgb32_middle_mask, mm0);\
                pand_m2r(rgb_rgb_rgb32_lower_mask, mm4);\
                pand_m2r(rgb_rgb_rgb32_upper_mask, mm5);\
                psllq_i2r(16, mm4);\
                psrlq_i2r(16, mm5);\
                por_r2r(mm4, mm0);\
                por_r2r(mm5, mm0);\
                movq_r2r(mm1, mm4);\
                movq_r2r(mm1, mm5);\
                pand_m2r(rgb_rgb_rgb32_middle_mask, mm1);\
                pand_m2r(rgb_rgb_rgb32_lower_mask, mm4);\
                pand_m2r(rgb_rgb_rgb32_upper_mask, mm5);\
                psllq_i2r(16, mm4);\
                psrlq_i2r(16, mm5);\
                por_r2r(mm4, mm1);\
                por_r2r(mm5, mm1);\
                movq_r2r(mm2, mm4);\
                movq_r2r(mm2, mm5);\
                pand_m2r(rgb_rgb_rgb32_middle_mask, mm2);\
                pand_m2r(rgb_rgb_rgb32_lower_mask, mm4);\
                pand_m2r(rgb_rgb_rgb32_upper_mask, mm5);\
                psllq_i2r(16, mm4);\
                psrlq_i2r(16, mm5);\
                por_r2r(mm4, mm2);\
                por_r2r(mm5, mm2);\
                movq_r2r(mm3, mm4);\
                movq_r2r(mm3, mm5);\
                pand_m2r(rgb_rgb_rgb32_middle_mask, mm3);\
                pand_m2r(rgb_rgb_rgb32_lower_mask, mm4);\
                pand_m2r(rgb_rgb_rgb32_upper_mask, mm5);\
                psllq_i2r(16, mm4);\
                psrlq_i2r(16, mm5);\
                por_r2r(mm4, mm3);\
                por_r2r(mm5, mm3);

static const mmx_t rgb_rgb_swap_24_mask_11 = { 0x0000FF0000FF0000LL };
static const mmx_t rgb_rgb_swap_24_mask_12 = { 0x00000000FF0000FFLL };
static const mmx_t rgb_rgb_swap_24_mask_13 = { 0xFFFF00FF0000FF00LL };

static const mmx_t rgb_rgb_swap_24_mask_21 = { 0xFF00FFFFFFFFFFFFLL };
static const mmx_t rgb_rgb_swap_24_mask_22 = { 0x00000000000000FFLL };
static const mmx_t rgb_rgb_swap_24_mask_23 = { 0x00000000FFFFFF00LL };

static const mmx_t rgb_rgb_swap_24_mask_31 = { 0x00000000FF000000LL };
static const mmx_t rgb_rgb_swap_24_mask_32 = { 0x000000000000FF00LL };
static const mmx_t rgb_rgb_swap_24_mask_33 = { 0x0000000000FF00FFLL };

#define SWAP_24 movq_m2r(*src, mm0);\
                movd_m2r(*(src+8), mm1);\
                movq_r2r(mm0, mm2);\
                movq_r2r(mm0, mm3);\
                pand_m2r(rgb_rgb_swap_24_mask_13, mm0);\
                pand_m2r(rgb_rgb_swap_24_mask_12, mm2);\
                pand_m2r(rgb_rgb_swap_24_mask_11, mm3);\
                psrlq_i2r(16, mm3);\
                psllq_i2r(16, mm2);\
                por_r2r(mm2, mm0);\
                por_r2r(mm3, mm0);\
                movq_r2r(mm0, mm2);\
                movq_r2r(mm1, mm3);\
                pand_m2r(rgb_rgb_swap_24_mask_21, mm0);\
                pand_m2r(rgb_rgb_swap_24_mask_22, mm3);\
                psllq_i2r(48, mm3);\
                por_r2r(mm3, mm0);\
                pand_m2r(rgb_rgb_swap_24_mask_23, mm1);\
                psrlq_i2r(48, mm2);\
                pand_m2r(rgb_rgb_swap_24_mask_22, mm2);\
                por_r2r(mm2, mm1);\
                movq_r2r(mm1, mm2);\
                movq_r2r(mm1, mm3);\
                pand_m2r(rgb_rgb_swap_24_mask_31, mm2);\
                pand_m2r(rgb_rgb_swap_24_mask_32, mm3);\
                pand_m2r(rgb_rgb_swap_24_mask_33, mm1);\
                psrlq_i2r(16, mm2);\
                psllq_i2r(16, mm3);\
                por_r2r(mm3, mm1);\
                por_r2r(mm2, mm1);\
                MOVQ_R2M(mm0, *dst);\
                movd_r2m(mm1, *(dst+8));\
                movq_m2r(*(src+12), mm0);\
                movd_m2r(*(src+20), mm1);\
                movq_r2r(mm0, mm2);\
                movq_r2r(mm0, mm3);\
                pand_m2r(rgb_rgb_swap_24_mask_13, mm0);\
                pand_m2r(rgb_rgb_swap_24_mask_12, mm2);\
                pand_m2r(rgb_rgb_swap_24_mask_11, mm3);\
                psrlq_i2r(16, mm3);\
                psllq_i2r(16, mm2);\
                por_r2r(mm2, mm0);\
                por_r2r(mm3, mm0);\
                movq_r2r(mm0, mm2);\
                movq_r2r(mm1, mm3);\
                pand_m2r(rgb_rgb_swap_24_mask_21, mm0);\
                pand_m2r(rgb_rgb_swap_24_mask_22, mm3);\
                psllq_i2r(48, mm3);\
                por_r2r(mm3, mm0);\
                pand_m2r(rgb_rgb_swap_24_mask_23, mm1);\
                psrlq_i2r(48, mm2);\
                pand_m2r(rgb_rgb_swap_24_mask_22, mm2);\
                por_r2r(mm2, mm1);\
                movq_r2r(mm1, mm2);\
                movq_r2r(mm1, mm3);\
                pand_m2r(rgb_rgb_swap_24_mask_31, mm2);\
                pand_m2r(rgb_rgb_swap_24_mask_32, mm3);\
                pand_m2r(rgb_rgb_swap_24_mask_33, mm1);\
                psrlq_i2r(16, mm2);\
                psllq_i2r(16, mm3);\
                por_r2r(mm3, mm1);\
                por_r2r(mm2, mm1);\
                MOVQ_R2M(mm0, *(dst+12));\
                movd_r2m(mm1, *(dst+20));




/* Pack 15 bits to 16 bits */

#define INIT_RGB_15_TO_16 movq_m2r(rgb_rgb_rgb15_up_mask, mm3);\
                          movq_m2r(rgb_rgb_rgb15_lower_mask, mm4);                     \

#define RGB_15_TO_16 movq_r2r(mm0, mm2);\
                      pand_r2r(mm3, mm2);\
                      psllq_i2r(1, mm2);\
                      pand_r2r(mm4, mm0);\
                      por_r2r(mm2, mm0);\
                      movq_r2r(mm1, mm2);\
                      pand_r2r(mm3, mm2);\
                      psllq_i2r(1, mm2);\
                      pand_r2r(mm4, mm1);\
                      por_r2r(mm2, mm1);

#define INIT_RGB_16_TO_15 movq_m2r(rgb_rgb_rgb16_up_mask, mm3);\
                           movq_m2r(rgb_rgb_rgb16_lower_mask, mm4);

#define RGB_16_TO_15 movq_r2r(mm0, mm2);\
                     psrlq_i2r(1, mm2);\
                     pand_r2r(mm3, mm2);\
                     pand_r2r(mm4, mm0);\
                     por_r2r(mm2, mm0);\
                     movq_r2r(mm1, mm2);\
                     psrlq_i2r(1, mm2);\
                     pand_r2r(mm3, mm2);\
                     pand_r2r(mm4, mm1);\
                     por_r2r(mm2, mm1);

#define RGB_15_TO_32 pxor_r2r(mm3, mm3);/* Zero mm3 */\
                     movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb15_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm6);\
                     punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm7);\
                     punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                     psrlq_i2r(2, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_rgb_rgb15_upper_mask, mm0);\
                     psllq_i2r(1, mm0);\
                     movq_r2r(mm0, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm6);\
                     punpckhbw_r2r(mm3, mm0);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm0, mm7);\
                     MOVQ_R2M(mm6,*dst);/*      mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                     MOVQ_R2M(mm7,*(dst+8));/*  mm7: 00 B3 G3 R3 00 B2 G2 R2 */\
                     movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb15_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm6);\
                     punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm7);\
                     punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                     psrlq_i2r(2, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_rgb_rgb15_upper_mask, mm1);\
                     psllq_i2r(1, mm1);\
                     movq_r2r(mm1, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm6);\
                     punpckhbw_r2r(mm3, mm1);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm1, mm7);\
                     MOVQ_R2M(mm6,*(dst+16));/* mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                     MOVQ_R2M(mm7,*(dst+24));/* mm7: 00 B3 G3 R3 00 B2 G2 R2 */

#define RGB_15_TO_24 pxor_r2r(mm3, mm3);/* Zero mm3 */\
                     movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb15_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm6);\
                     punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm7);\
                     punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                     psrlq_i2r(2, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_rgb_rgb15_upper_mask, mm0);\
                     psllq_i2r(1, mm0);\
                     movq_r2r(mm0, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm6);\
                     punpckhbw_r2r(mm3, mm0);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm0, mm7);\
                     /* 32 -> 24 */\
                     movq_r2r(mm6, mm5);\
                     pand_m2r(rgb_rgb_lower_dword, mm5);\
                     pand_m2r(rgb_rgb_upper_dword, mm6);\
                     psrlq_i2r(8, mm6);\
                     por_r2r(mm5, mm6);\
                     movq_r2r(mm7, mm5);\
                     pand_m2r(rgb_rgb_lower_dword, mm5);\
                     pand_m2r(rgb_rgb_upper_dword, mm7);\
                     psrlq_i2r(8, mm7);\
                     por_r2r(mm5, mm7);\
                     movq_r2r(mm7, mm5);\
                     psllq_i2r(48, mm5);\
                     por_r2r(mm5, mm6);\
                     psrlq_i2r(16, mm7);\
                     MOVQ_R2M(mm6,*dst);\
                     movd_r2m(mm7,*(dst+8));\
                     /* Next 4 pixels */\
                     movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb15_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm6);\
                     punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm7);\
                     punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                     psrlq_i2r(2, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_rgb_rgb15_upper_mask, mm1);\
                     psllq_i2r(1, mm1);\
                     movq_r2r(mm1, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm6);\
                     punpckhbw_r2r(mm3, mm1);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm1, mm7);\
                     /* 32 -> 24 */\
                     movq_r2r(mm6, mm5);\
                     pand_m2r(rgb_rgb_lower_dword, mm5);\
                     pand_m2r(rgb_rgb_upper_dword, mm6);\
                     psrlq_i2r(8, mm6);\
                     por_r2r(mm5, mm6);\
                     movq_r2r(mm7, mm5);\
                     pand_m2r(rgb_rgb_lower_dword, mm5);\
                     pand_m2r(rgb_rgb_upper_dword, mm7);\
                     psrlq_i2r(8, mm7);\
                     por_r2r(mm5, mm7);\
                     movq_r2r(mm7, mm5);\
                     psllq_i2r(48, mm5);\
                     por_r2r(mm5, mm6);\
                     psrlq_i2r(16, mm7);\
                     MOVQ_R2M(mm6,*(dst+12));\
                     movd_r2m(mm7,*(dst+20));

#define RGB_15_TO_32_RGBA pxor_r2r(mm3, mm3);/* Zero mm3 */\
                     movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb15_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm6);\
                     punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm7);\
                     punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                     psrlq_i2r(2, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_rgb_rgb15_upper_mask, mm0);\
                     psllq_i2r(1, mm0);\
                     movq_r2r(mm0, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm6);\
                     punpckhbw_r2r(mm3, mm0);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm0, mm7);\
                     por_m2r(rgba32_alpha_mask, mm6);\
                     por_m2r(rgba32_alpha_mask, mm7);\
                     MOVQ_R2M(mm6,*dst);/*      mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                     MOVQ_R2M(mm7,*(dst+8));/*  mm7: 00 B3 G3 R3 00 B2 G2 R2 */\
                     movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb15_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm6);\
                     punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm7);\
                     punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                     psrlq_i2r(2, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_rgb_rgb15_upper_mask, mm1);\
                     psllq_i2r(1, mm1);\
                     movq_r2r(mm1, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm6);\
                     punpckhbw_r2r(mm3, mm1);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm1, mm7);\
                     por_m2r(rgba32_alpha_mask, mm6);\
                     por_m2r(rgba32_alpha_mask, mm7);\
                     MOVQ_R2M(mm6,*(dst+16));/*  mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                     MOVQ_R2M(mm7,*(dst+24));/* mm7: 00 B3 G3 R3 00 B2 G2 R2 */

#define RGB_16_TO_32 pxor_r2r(mm3, mm3);/* Zero mm3 */\
                     movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb16_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm6);\
                     punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm7);\
                     punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                     psrlq_i2r(3, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_rgb_rgb16_upper_mask, mm0);\
                     movq_r2r(mm0, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm6);\
                     punpckhbw_r2r(mm3, mm0);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm0, mm7);\
                     MOVQ_R2M(mm6,*dst);/*      mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                     MOVQ_R2M(mm7,*(dst+8));/*  mm7: 00 B3 G3 R3 00 B2 G2 R2 */\
                     movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb16_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm6);\
                     punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm7);\
                     punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                     psrlq_i2r(3, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_rgb_rgb16_upper_mask, mm1);\
                     movq_r2r(mm1, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm6);\
                     punpckhbw_r2r(mm3, mm1);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm1, mm7);\
                     MOVQ_R2M(mm6,*(dst+16));/*  mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                     MOVQ_R2M(mm7,*(dst+24));/* mm7: 00 B3 G3 R3 00 B2 G2 R2 */


#define RGB_16_TO_24 pxor_r2r(mm3, mm3);/* Zero mm3 */\
                     movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb16_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm6);\
                     punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm7);\
                     punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                     psrlq_i2r(3, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_rgb_rgb16_upper_mask, mm0);\
                     movq_r2r(mm0, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm6);\
                     punpckhbw_r2r(mm3, mm0);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm0, mm7);\
                     /* 32 -> 24 */\
                     movq_r2r(mm6, mm5);\
                     pand_m2r(rgb_rgb_lower_dword, mm5);\
                     pand_m2r(rgb_rgb_upper_dword, mm6);\
                     psrlq_i2r(8, mm6);\
                     por_r2r(mm5, mm6);\
                     movq_r2r(mm7, mm5);\
                     pand_m2r(rgb_rgb_lower_dword, mm5);\
                     pand_m2r(rgb_rgb_upper_dword, mm7);\
                     psrlq_i2r(8, mm7);\
                     por_r2r(mm5, mm7);\
                     movq_r2r(mm7, mm5);\
                     psllq_i2r(48, mm5);\
                     por_r2r(mm5, mm6);\
                     psrlq_i2r(16, mm7);\
                     MOVQ_R2M(mm6,*dst);\
                     movd_r2m(mm7,*(dst+8));\
                     movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb16_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm6);\
                     punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm7);\
                     punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                     psrlq_i2r(3, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_rgb_rgb16_upper_mask, mm1);\
                     movq_r2r(mm1, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm6);\
                     punpckhbw_r2r(mm3, mm1);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm1, mm7);\
                     /* 32 -> 24 */\
                     movq_r2r(mm6, mm5);\
                     pand_m2r(rgb_rgb_lower_dword, mm5);\
                     pand_m2r(rgb_rgb_upper_dword, mm6);\
                     psrlq_i2r(8, mm6);\
                     por_r2r(mm5, mm6);\
                     movq_r2r(mm7, mm5);\
                     pand_m2r(rgb_rgb_lower_dword, mm5);\
                     pand_m2r(rgb_rgb_upper_dword, mm7);\
                     psrlq_i2r(8, mm7);\
                     por_r2r(mm5, mm7);\
                     movq_r2r(mm7, mm5);\
                     psllq_i2r(48, mm5);\
                     por_r2r(mm5, mm6);\
                     psrlq_i2r(16, mm7);\
                     MOVQ_R2M(mm6,*(dst+12));\
                     movd_r2m(mm7,*(dst+20));


#define RGB_16_TO_32_RGBA pxor_r2r(mm3, mm3);/* Zero mm3 */\
                     movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb16_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm6);\
                     punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm7);\
                     punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                     psrlq_i2r(3, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_rgb_rgb16_upper_mask, mm0);\
                     movq_r2r(mm0, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm6);\
                     punpckhbw_r2r(mm3, mm0);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm0, mm7);\
                     por_m2r(rgba32_alpha_mask, mm6);\
                     por_m2r(rgba32_alpha_mask, mm7);\
                     MOVQ_R2M(mm6,*dst);/*      mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                     MOVQ_R2M(mm7,*(dst+8));/*  mm7: 00 B3 G3 R3 00 B2 G2 R2 */\
                     movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb16_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm6);\
                     punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm7);\
                     punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                     psrlq_i2r(3, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_rgb_rgb16_upper_mask, mm1);\
                     movq_r2r(mm1, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm6);\
                     punpckhbw_r2r(mm3, mm1);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm1, mm7);\
                     por_m2r(rgba32_alpha_mask, mm6);\
                     por_m2r(rgba32_alpha_mask, mm7);\
                     MOVQ_R2M(mm6,*(dst+16));/*  mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                     MOVQ_R2M(mm7,*(dst+24));/* mm7: 00 B3 G3 R3 00 B2 G2 R2 */


#define RGB_15_TO_32_SWAP pxor_r2r(mm3, mm3);/* Zero mm3 */\
                          movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb15_upper_mask, mm2);\
                          psrlq_i2r(7, mm2);\
                          movq_r2r(mm2, mm6);\
                          punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                          movq_r2r(mm2, mm7);\
                          punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                          movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                          psrlq_i2r(2, mm2);\
                          punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                          por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                          por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                          pand_m2r(rgb_rgb_rgb15_lower_mask, mm0);\
                          psllq_i2r(11, mm0);\
                          movq_r2r(mm0, mm2);\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm2, mm6);\
                          punpckhbw_r2r(mm3, mm0);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm0, mm7);\
                          MOVQ_R2M(mm6,*dst);/*      mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                          MOVQ_R2M(mm7,*(dst+8));/*  mm7: 00 B3 G3 R3 00 B2 G2 R2 */\
                          movq_r2r(mm1, mm2);\
                          pand_m2r(rgb_rgb_rgb15_upper_mask, mm2);\
                          psrlq_i2r(7, mm2);\
                          movq_r2r(mm2, mm6);\
                          punpcklbw_r2r(mm3, mm6);\
                          movq_r2r(mm2, mm7);\
                          punpckhbw_r2r(mm3, mm7);\
                          movq_r2r(mm1, mm2);\
                          pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                          psrlq_i2r(2, mm2);\
                          punpcklbw_r2r(mm2, mm3);\
                          por_r2r(mm3, mm6);\
                          pxor_r2r(mm3, mm3);\
                          punpckhbw_r2r(mm2, mm3);\
                          por_r2r(mm3, mm7);\
                          pand_m2r(rgb_rgb_rgb15_lower_mask, mm1);\
                          psllq_i2r(11, mm1);\
                          movq_r2r(mm1, mm2);\
                          pxor_r2r(mm3, mm3);\
                          punpcklbw_r2r(mm3, mm2);\
                          por_r2r(mm2, mm6);\
                          punpckhbw_r2r(mm3, mm1);\
                          por_r2r(mm1, mm7);\
                          MOVQ_R2M(mm6,*(dst+16));\
                          MOVQ_R2M(mm7,*(dst+24));


#define RGB_15_TO_24_SWAP pxor_r2r(mm3, mm3);/* Zero mm3 */\
                          movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb15_upper_mask, mm2);\
                          psrlq_i2r(7, mm2);\
                          movq_r2r(mm2, mm6);\
                          punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                          movq_r2r(mm2, mm7);\
                          punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                          movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                          psrlq_i2r(2, mm2);\
                          punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                          por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                          por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                          pand_m2r(rgb_rgb_rgb15_lower_mask, mm0);\
                          psllq_i2r(11, mm0);\
                          movq_r2r(mm0, mm2);\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm2, mm6);\
                          punpckhbw_r2r(mm3, mm0);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm0, mm7);\
                          /* 32 -> 24 */\
                          movq_r2r(mm6, mm5);\
                          pand_m2r(rgb_rgb_lower_dword, mm5);\
                          pand_m2r(rgb_rgb_upper_dword, mm6);\
                          psrlq_i2r(8, mm6);\
                          por_r2r(mm5, mm6);\
                          movq_r2r(mm7, mm5);\
                          pand_m2r(rgb_rgb_lower_dword, mm5);\
                          pand_m2r(rgb_rgb_upper_dword, mm7);\
                          psrlq_i2r(8, mm7);\
                          por_r2r(mm5, mm7);\
                          movq_r2r(mm7, mm5);\
                          psllq_i2r(48, mm5);\
                          por_r2r(mm5, mm6);\
                          psrlq_i2r(16, mm7);\
                          MOVQ_R2M(mm6,*dst);\
                          movd_r2m(mm7,*(dst+8));\
                          /* Next 4 pixels */\
                          movq_r2r(mm1, mm2);\
                          pand_m2r(rgb_rgb_rgb15_upper_mask, mm2);\
                          psrlq_i2r(7, mm2);\
                          movq_r2r(mm2, mm6);\
                          punpcklbw_r2r(mm3, mm6);\
                          movq_r2r(mm2, mm7);\
                          punpckhbw_r2r(mm3, mm7);\
                          movq_r2r(mm1, mm2);\
                          pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                          psrlq_i2r(2, mm2);\
                          punpcklbw_r2r(mm2, mm3);\
                          por_r2r(mm3, mm6);\
                          pxor_r2r(mm3, mm3);\
                          punpckhbw_r2r(mm2, mm3);\
                          por_r2r(mm3, mm7);\
                          pand_m2r(rgb_rgb_rgb15_lower_mask, mm1);\
                          psllq_i2r(11, mm1);\
                          movq_r2r(mm1, mm2);\
                          pxor_r2r(mm3, mm3);\
                          punpcklbw_r2r(mm3, mm2);\
                          por_r2r(mm2, mm6);\
                          punpckhbw_r2r(mm3, mm1);\
                          por_r2r(mm1, mm7);\
                          movq_r2r(mm6, mm5);\
                          pand_m2r(rgb_rgb_lower_dword, mm5);\
                          pand_m2r(rgb_rgb_upper_dword, mm6);\
                          psrlq_i2r(8, mm6);\
                          por_r2r(mm5, mm6);\
                          movq_r2r(mm7, mm5);\
                          pand_m2r(rgb_rgb_lower_dword, mm5);\
                          pand_m2r(rgb_rgb_upper_dword, mm7);\
                          psrlq_i2r(8, mm7);\
                          por_r2r(mm5, mm7);\
                          movq_r2r(mm7, mm5);\
                          psllq_i2r(48, mm5);\
                          por_r2r(mm5, mm6);\
                          psrlq_i2r(16, mm7);\
                          MOVQ_R2M(mm6,*(dst+12));\
                          movd_r2m(mm7,*(dst+20));

#define RGB_15_TO_32_SWAP_RGBA pxor_r2r(mm3, mm3);/* Zero mm3 */\
                          movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb15_upper_mask, mm2);\
                          psrlq_i2r(7, mm2);\
                          movq_r2r(mm2, mm6);\
                          punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                          movq_r2r(mm2, mm7);\
                          punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                          movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                          psrlq_i2r(2, mm2);\
                          punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                          por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                          por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                          pand_m2r(rgb_rgb_rgb15_lower_mask, mm0);\
                          psllq_i2r(11, mm0);\
                          movq_r2r(mm0, mm2);\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm2, mm6);\
                          punpckhbw_r2r(mm3, mm0);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm0, mm7);\
                          por_m2r(rgba32_alpha_mask, mm6);\
                          por_m2r(rgba32_alpha_mask, mm7);\
                          MOVQ_R2M(mm6,*dst);/*      mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                          MOVQ_R2M(mm7,*(dst+8));/*  mm7: 00 B3 G3 R3 00 B2 G2 R2 */\
                          movq_r2r(mm1, mm2);\
                          pand_m2r(rgb_rgb_rgb15_upper_mask, mm2);\
                          psrlq_i2r(7, mm2);\
                          movq_r2r(mm2, mm6);\
                          punpcklbw_r2r(mm3, mm6);\
                          movq_r2r(mm2, mm7);\
                          punpckhbw_r2r(mm3, mm7);\
                          movq_r2r(mm1, mm2);\
                          pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                          psrlq_i2r(2, mm2);\
                          punpcklbw_r2r(mm2, mm3);\
                          por_r2r(mm3, mm6);\
                          pxor_r2r(mm3, mm3);\
                          punpckhbw_r2r(mm2, mm3);\
                          por_r2r(mm3, mm7);\
                          pand_m2r(rgb_rgb_rgb15_lower_mask, mm1);\
                          psllq_i2r(11, mm1);\
                          movq_r2r(mm1, mm2);\
                          pxor_r2r(mm3, mm3);\
                          punpcklbw_r2r(mm3, mm2);\
                          por_r2r(mm2, mm6);\
                          punpckhbw_r2r(mm3, mm1);\
                          por_r2r(mm1, mm7);\
                          por_m2r(rgba32_alpha_mask, mm6);\
                          por_m2r(rgba32_alpha_mask, mm7);\
                          MOVQ_R2M(mm6,*(dst+16));\
                          MOVQ_R2M(mm7,*(dst+24));

#define RGB_15_TO_32_SWAP_RGBA pxor_r2r(mm3, mm3);/* Zero mm3 */\
                          movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb15_upper_mask, mm2);\
                          psrlq_i2r(7, mm2);\
                          movq_r2r(mm2, mm6);\
                          punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                          movq_r2r(mm2, mm7);\
                          punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                          movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                          psrlq_i2r(2, mm2);\
                          punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                          por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                          por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                          pand_m2r(rgb_rgb_rgb15_lower_mask, mm0);\
                          psllq_i2r(11, mm0);\
                          movq_r2r(mm0, mm2);\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm2, mm6);\
                          punpckhbw_r2r(mm3, mm0);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm0, mm7);\
                          por_m2r(rgba32_alpha_mask, mm6);\
                          por_m2r(rgba32_alpha_mask, mm7);\
                          MOVQ_R2M(mm6,*dst);/*      mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                          MOVQ_R2M(mm7,*(dst+8));/*  mm7: 00 B3 G3 R3 00 B2 G2 R2 */\
                          movq_r2r(mm1, mm2);\
                          pand_m2r(rgb_rgb_rgb15_upper_mask, mm2);\
                          psrlq_i2r(7, mm2);\
                          movq_r2r(mm2, mm6);\
                          punpcklbw_r2r(mm3, mm6);\
                          movq_r2r(mm2, mm7);\
                          punpckhbw_r2r(mm3, mm7);\
                          movq_r2r(mm1, mm2);\
                          pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                          psrlq_i2r(2, mm2);\
                          punpcklbw_r2r(mm2, mm3);\
                          por_r2r(mm3, mm6);\
                          pxor_r2r(mm3, mm3);\
                          punpckhbw_r2r(mm2, mm3);\
                          por_r2r(mm3, mm7);\
                          pand_m2r(rgb_rgb_rgb15_lower_mask, mm1);\
                          psllq_i2r(11, mm1);\
                          movq_r2r(mm1, mm2);\
                          pxor_r2r(mm3, mm3);\
                          punpcklbw_r2r(mm3, mm2);\
                          por_r2r(mm2, mm6);\
                          punpckhbw_r2r(mm3, mm1);\
                          por_r2r(mm1, mm7);\
                          por_m2r(rgba32_alpha_mask, mm6);\
                          por_m2r(rgba32_alpha_mask, mm7);\
                          MOVQ_R2M(mm6,*(dst+16));\
                          MOVQ_R2M(mm7,*(dst+24));



#define RGB_16_TO_32_SWAP pxor_r2r(mm3, mm3);/* Zero mm3 */\
                          movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb16_upper_mask, mm2);\
                          psrlq_i2r(8, mm2);\
                          movq_r2r(mm2, mm6);\
                          punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                          movq_r2r(mm2, mm7);\
                          punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                          movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                          psrlq_i2r(3, mm2);\
                          punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                          por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                          por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                          pand_m2r(rgb_rgb_rgb16_lower_mask, mm0);\
                          psllq_i2r(11, mm0);\
                          movq_r2r(mm0, mm2);\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm2, mm6);\
                          punpckhbw_r2r(mm3, mm0);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm0, mm7);\
                          MOVQ_R2M(mm6,*dst);/*      mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                          MOVQ_R2M(mm7,*(dst+8));/*  mm7: 00 B3 G3 R3 00 B2 G2 R2 */\
                          movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb16_upper_mask, mm2);\
                          psrlq_i2r(8, mm2);\
                          movq_r2r(mm2, mm6);\
                          punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                          movq_r2r(mm2, mm7);\
                          punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                          movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                          psrlq_i2r(3, mm2);\
                          punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                          por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                          por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                          pand_m2r(rgb_rgb_rgb16_lower_mask, mm1);\
                          psllq_i2r(11, mm1);\
                          movq_r2r(mm1, mm2);\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm2, mm6);\
                          punpckhbw_r2r(mm3, mm1);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm1, mm7);\
                          MOVQ_R2M(mm6,*(dst+16));/* mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                          MOVQ_R2M(mm7,*(dst+24));/* mm7: 00 B3 G3 R3 00 B2 G2 R2 */

#define RGB_16_TO_24_SWAP pxor_r2r(mm3, mm3);/* Zero mm3 */\
                          movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb16_upper_mask, mm2);\
                          psrlq_i2r(8, mm2);\
                          movq_r2r(mm2, mm6);\
                          punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                          movq_r2r(mm2, mm7);\
                          punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                          movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                          psrlq_i2r(3, mm2);\
                          punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                          por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                          por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                          pand_m2r(rgb_rgb_rgb16_lower_mask, mm0);\
                          psllq_i2r(11, mm0);\
                          movq_r2r(mm0, mm2);\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm2, mm6);\
                          punpckhbw_r2r(mm3, mm0);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm0, mm7);\
                          /* 32 -> 24 */\
                          movq_r2r(mm6, mm5);\
                          pand_m2r(rgb_rgb_lower_dword, mm5);\
                          pand_m2r(rgb_rgb_upper_dword, mm6);\
                          psrlq_i2r(8, mm6);\
                          por_r2r(mm5, mm6);\
                          movq_r2r(mm7, mm5);\
                          pand_m2r(rgb_rgb_lower_dword, mm5);\
                          pand_m2r(rgb_rgb_upper_dword, mm7);\
                          psrlq_i2r(8, mm7);\
                          por_r2r(mm5, mm7);\
                          movq_r2r(mm7, mm5);\
                          psllq_i2r(48, mm5);\
                          por_r2r(mm5, mm6);\
                          psrlq_i2r(16, mm7);\
                          MOVQ_R2M(mm6,*dst);\
                          movd_r2m(mm7,*(dst+8));\
                          /* Next 4 pixels */\
                          movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb16_upper_mask, mm2);\
                          psrlq_i2r(8, mm2);\
                          movq_r2r(mm2, mm6);\
                          punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                          movq_r2r(mm2, mm7);\
                          punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                          movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                          pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                          psrlq_i2r(3, mm2);\
                          punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                          por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                          por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                          pand_m2r(rgb_rgb_rgb16_lower_mask, mm1);\
                          psllq_i2r(11, mm1);\
                          movq_r2r(mm1, mm2);\
                          pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                          punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm2, mm6);\
                          punpckhbw_r2r(mm3, mm1);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                          por_r2r(mm1, mm7);\
                          /* 32 -> 24 */\
                          movq_r2r(mm6, mm5);\
                          pand_m2r(rgb_rgb_lower_dword, mm5);\
                          pand_m2r(rgb_rgb_upper_dword, mm6);\
                          psrlq_i2r(8, mm6);\
                          por_r2r(mm5, mm6);\
                          movq_r2r(mm7, mm5);\
                          pand_m2r(rgb_rgb_lower_dword, mm5);\
                          pand_m2r(rgb_rgb_upper_dword, mm7);\
                          psrlq_i2r(8, mm7);\
                          por_r2r(mm5, mm7);\
                          movq_r2r(mm7, mm5);\
                          psllq_i2r(48, mm5);\
                          por_r2r(mm5, mm6);\
                          psrlq_i2r(16, mm7);\
                          MOVQ_R2M(mm6,*(dst+12));\
                          movd_r2m(mm7,*(dst+20));



#define RGB_16_TO_32_SWAP_RGBA pxor_r2r(mm3, mm3);/* Zero mm3 */\
                               movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                               pand_m2r(rgb_rgb_rgb16_upper_mask, mm2);\
                               psrlq_i2r(8, mm2);\
                               movq_r2r(mm2, mm6);\
                               punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                               movq_r2r(mm2, mm7);\
                               punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                               movq_r2r(mm0, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                               pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                               psrlq_i2r(3, mm2);\
                               punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                               por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                               pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                               punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                               por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                               pand_m2r(rgb_rgb_rgb16_lower_mask, mm0);\
                               psllq_i2r(11, mm0);\
                               movq_r2r(mm0, mm2);\
                               pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                               punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                               por_r2r(mm2, mm6);\
                               punpckhbw_r2r(mm3, mm0);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                               por_r2r(mm0, mm7);\
                               por_m2r(rgba32_alpha_mask, mm6);\
                               por_m2r(rgba32_alpha_mask, mm7);\
                               MOVQ_R2M(mm6,*dst);/*      mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                               MOVQ_R2M(mm7,*(dst+8));/*  mm7: 00 B3 G3 R3 00 B2 G2 R2 */\
                               movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                               pand_m2r(rgb_rgb_rgb16_upper_mask, mm2);\
                               psrlq_i2r(8, mm2);\
                               movq_r2r(mm2, mm6);\
                               punpcklbw_r2r(mm3, mm6);/* mm6: 00 00 00 R1 00 00 00 R0 */\
                               movq_r2r(mm2, mm7);\
                               punpckhbw_r2r(mm3, mm7);/* mm7: 00 00 00 R3 00 00 00 R2 */\
                               movq_r2r(mm1, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                               pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                               psrlq_i2r(3, mm2);\
                               punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                               por_r2r(mm3, mm6);/*       mm6: 00 00 G1 R1 00 00 G0 R0 */\
                               pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                               punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                               por_r2r(mm3, mm7);/*       mm7: 00 00 G3 R3 00 00 G2 R2 */\
                               pand_m2r(rgb_rgb_rgb16_lower_mask, mm1);\
                               psllq_i2r(11, mm1);\
                               movq_r2r(mm1, mm2);\
                               pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                               punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                               por_r2r(mm2, mm6);\
                               punpckhbw_r2r(mm3, mm1);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                               por_r2r(mm1, mm7);\
                               por_m2r(rgba32_alpha_mask, mm6);\
                               por_m2r(rgba32_alpha_mask, mm7);\
                               MOVQ_R2M(mm6,*(dst+16));/* mm6: 00 B1 G1 R1 00 B0 G0 R0 */\
                               MOVQ_R2M(mm7,*(dst+24));/* mm7: 00 B3 G3 R3 00 B2 G2 R2 */

#define RGB_32_TO_16 movq_r2r(mm0, mm4);\
                     movq_r2r(mm1, mm5);\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm4);/*  mm4: 00 B1 00 R1 00 B0 00 R0 */\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm5);/*  mm5: 00 B3 00 R3 00 B3 00 R2 */\
                     packuswb_r2r(mm5, mm4);/*                 mm4: B3 R3 B2 R2 B1 R1 B0 R0 */\
                     movq_r2r(mm4, mm6);\
                     pand_m2r(rgb_rgb_rgb16_upper_mask, mm6);\
                     psrlq_i2r(3, mm4);\
                     pand_m2r(rgb_rgb_rgb16_lower_mask, mm4);\
                     por_r2r(mm4, mm6);\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm0);/*       mm0: 00 00 G1 00 00 00 G0 00  */\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm1);/*       mm1: 00 00 G3 00 00 00 G2 00   */\
                     psrlq_i2r(1, mm0);\
                     psrlq_i2r(1, mm1);\
                     packssdw_r2r(mm1, mm0);/*                 mm0: G3 00 G2 00 G1 00 G0 00 */\
                     psrlq_i2r(4, mm0);\
                     pand_m2r(rgb_rgb_rgb16_middle_mask, mm0);\
                     por_r2r(mm0, mm6);\
                     MOVQ_R2M(mm6, *dst);\
                     movq_r2r(mm2, mm4);\
                     movq_r2r(mm3, mm5);\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm4);\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm5);\
                     packuswb_r2r(mm5, mm4);\
                     movq_r2r(mm4, mm6);\
                     pand_m2r(rgb_rgb_rgb16_upper_mask, mm6);\
                     psrlq_i2r(3, mm4);\
                     pand_m2r(rgb_rgb_rgb16_lower_mask, mm4);\
                     por_r2r(mm4, mm6);\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm2);\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm3);\
                     psrlq_i2r(1, mm2);\
                     psrlq_i2r(1, mm3);\
                     packssdw_r2r(mm3, mm2);\
                     psrlq_i2r(4, mm2);\
                     pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                     por_r2r(mm2, mm6);\
                     MOVQ_R2M(mm6, *(dst+8));

#define RGB_32_TO_16_SWAP movq_r2r(mm0, mm4);\
                     movq_r2r(mm1, mm5);\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm4);/*  mm4: 00 B1 00 R1 00 B0 00 R0 */\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm5);/*  mm5: 00 B3 00 R3 00 B3 00 R2 */\
                     packuswb_r2r(mm5, mm4);/*                 mm4: B3 R3 B2 R2 B1 R1 B0 R0 */\
                     movq_r2r(mm4, mm6);\
                     psrlw_i2r(11, mm6);\
                     psllq_i2r(8, mm4);\
                     pand_m2r(rgb_rgb_rgb16_upper_mask, mm4);\
                     por_r2r(mm4, mm6);\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm0);/*       mm0: 00 00 G1 00 00 00 G0 00  */\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm1);/*       mm1: 00 00 G3 00 00 00 G2 00   */\
                     psrlq_i2r(1, mm0);\
                     psrlq_i2r(1, mm1);\
                     packssdw_r2r(mm1, mm0);/*                 mm0: G3 00 G2 00 G1 00 G0 00 */\
                     psrlq_i2r(4, mm0);\
                     pand_m2r(rgb_rgb_rgb16_middle_mask, mm0);\
                     por_r2r(mm0, mm6);\
                     MOVQ_R2M(mm6, *dst);\
                     movq_r2r(mm2, mm4);\
                     movq_r2r(mm3, mm5);\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm4);\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm5);\
                     packuswb_r2r(mm5, mm4);\
                     movq_r2r(mm4, mm6);\
                     psrlw_i2r(11, mm6);\
                     psllq_i2r(8, mm4);\
                     pand_m2r(rgb_rgb_rgb16_upper_mask, mm4);\
                     por_r2r(mm4, mm6);\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm2);\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm3);\
                     psrlq_i2r(1, mm2);\
                     psrlq_i2r(1, mm3);\
                     packssdw_r2r(mm3, mm2);\
                     psrlq_i2r(4, mm2);\
                     pand_m2r(rgb_rgb_rgb16_middle_mask, mm2);\
                     por_r2r(mm2, mm6);\
                     MOVQ_R2M(mm6, *(dst+8));


#define RGB_32_TO_15 movq_r2r(mm0, mm4);\
                     movq_r2r(mm1, mm5);\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm4);/*  mm4: 00 B1 00 R1 00 B0 00 R0 */\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm5);/*  mm5: 00 B3 00 R3 00 B3 00 R2 */\
                     packuswb_r2r(mm5, mm4);/*                 mm4: B3 R3 B2 R2 B1 R1 B0 R0 */\
                     movq_r2r(mm4, mm6);\
                     psrlq_i2r(1, mm6);\
                     pand_m2r(rgb_rgb_rgb15_upper_mask, mm6);\
                     psrlq_i2r(3, mm4);\
                     pand_m2r(rgb_rgb_rgb15_lower_mask, mm4);\
                     por_r2r(mm4, mm6);\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm0);/*       mm0: 00 00 G1 00 00 00 G0 00  */\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm1);/*       mm1: 00 00 G3 00 00 00 G2 00   */\
                     psrlq_i2r(1, mm0);\
                     psrlq_i2r(1, mm1);\
                     packssdw_r2r(mm1, mm0);/*                 mm0: G3 00 G2 00 G1 00 G0 00 */\
                     psrlq_i2r(5, mm0);\
                     pand_m2r(rgb_rgb_rgb15_middle_mask, mm0);\
                     por_r2r(mm0, mm6);\
                     MOVQ_R2M(mm6, *dst);\
                     movq_r2r(mm2, mm4);\
                     movq_r2r(mm3, mm5);\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm4);\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm5);\
                     packuswb_r2r(mm5, mm4);\
                     movq_r2r(mm4, mm6);\
                     psrlq_i2r(1, mm6);\
                     pand_m2r(rgb_rgb_rgb15_upper_mask, mm6);\
                     psrlq_i2r(3, mm4);\
                     pand_m2r(rgb_rgb_rgb15_lower_mask, mm4);\
                     por_r2r(mm4, mm6);\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm2);\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm3);\
                     psrlq_i2r(1, mm2);\
                     psrlq_i2r(1, mm3);\
                     packssdw_r2r(mm3, mm2);\
                     psrlq_i2r(5, mm2);\
                     pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                     por_r2r(mm2, mm6);\
                     MOVQ_R2M(mm6, *(dst+8));

#define RGB_32_TO_15_SWAP movq_r2r(mm0, mm4);\
                     movq_r2r(mm1, mm5);\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm4);/*  mm4: 00 B1 00 R1 00 B0 00 R0 */\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm5);/*  mm5: 00 B3 00 R3 00 B3 00 R2 */\
                     packuswb_r2r(mm5, mm4);/*                 mm4: B3 R3 B2 R2 B1 R1 B0 R0 */\
                     movq_r2r(mm4, mm6);\
                     psrlw_i2r(11, mm6);\
                     psllw_i2r(7, mm4);\
                     pand_m2r(rgb_rgb_rgb15_upper_mask, mm4);\
                     por_r2r(mm4, mm6);\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm0);/*       mm0: 00 00 G1 00 00 00 G0 00  */\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm1);/*       mm1: 00 00 G3 00 00 00 G2 00   */\
                     psrlq_i2r(1, mm0);\
                     psrlq_i2r(1, mm1);\
                     packssdw_r2r(mm1, mm0);/*                 mm0: G3 00 G2 00 G1 00 G0 00 */\
                     psrlq_i2r(5, mm0);\
                     pand_m2r(rgb_rgb_rgb15_middle_mask, mm0);\
                     por_r2r(mm0, mm6);\
                     MOVQ_R2M(mm6, *dst);\
                     movq_r2r(mm2, mm4);\
                     movq_r2r(mm3, mm5);\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm4);\
                     pand_m2r(rgb_rgb_rgb32_upper_lower_mask, mm5);\
                     packuswb_r2r(mm5, mm4);\
                     movq_r2r(mm4, mm6);\
                     psrlw_i2r(11, mm6);\
                     psllw_i2r(7, mm4);\
                     pand_m2r(rgb_rgb_rgb15_upper_mask, mm4);\
                     por_r2r(mm4, mm6);\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm2);\
                     pand_m2r(rgb_rgb_rgb32_middle_mask, mm3);\
                     psrlq_i2r(1, mm2);\
                     psrlq_i2r(1, mm3);\
                     packssdw_r2r(mm3, mm2);\
                     psrlq_i2r(5, mm2);\
                     pand_m2r(rgb_rgb_rgb15_middle_mask, mm2);\
                     por_r2r(mm2, mm6);\
                     MOVQ_R2M(mm6, *(dst+8));


#define FUNC_NAME   swap_rgb_24_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  24
#define OUT_ADVANCE 24
#define NUM_PIXELS  8
#define CONVERT     SWAP_24
#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   swap_rgb_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  32
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_32 \
    SWAP_32 \
    WRITE_32
#define CLEANUP emms();

#include "../csp_packed_packed.h"

#define FUNC_NAME   swap_rgb_16_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    SWAP_16 \
    WRITE_16
#define INIT INIT_SWAP_16


#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   swap_rgb_15_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    SWAP_15 \
    WRITE_16
#define INIT INIT_SWAP_15
#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_15_to_16_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    RGB_15_TO_16 \
    WRITE_16

#define INIT        INIT_RGB_15_TO_16
#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_15_to_24_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 24
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    RGB_15_TO_24_SWAP

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_15_to_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    RGB_15_TO_32_SWAP

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_16_to_15_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define INIT   INIT_RGB_16_TO_15
#define CONVERT     \
    LOAD_16 \
    RGB_16_TO_15 \
    WRITE_16
#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_16_to_24_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 24
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    RGB_16_TO_24_SWAP

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_16_to_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    RGB_16_TO_32_SWAP

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_24_to_15_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  24
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_24 \
    RGB_32_TO_15_SWAP

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_24_to_16_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  24
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_24 \
    RGB_32_TO_16_SWAP

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_24_to_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  24
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_24 \
    WRITE_32

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_32_to_15_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  32
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_32 \
    RGB_32_TO_15_SWAP

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_32_to_16_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  32
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_32 \
    RGB_32_TO_16_SWAP

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_32_to_24_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  32
#define OUT_ADVANCE 24
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_32 \
    WRITE_24

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_15_to_16_swap_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    SWAP_15_TO_16 \
    WRITE_16

#define INIT INIT_SWAP_15_TO_16

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_15_to_24_swap_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 24
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    RGB_15_TO_24

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_15_to_32_swap_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    RGB_15_TO_32

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_16_to_15_swap_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    SWAP_16_TO_15 \
    WRITE_16

#define INIT INIT_SWAP_16_TO_15

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_16_to_24_swap_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 24
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    RGB_16_TO_24

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_16_to_32_swap_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    RGB_16_TO_32

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_24_to_15_swap_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  24
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_24 \
    RGB_32_TO_15

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_24_to_16_swap_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  24
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_24 \
    RGB_32_TO_16

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_24_to_32_swap_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  24
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_24 \
    SWAP_32 \
    WRITE_32

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_32_to_15_swap_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  32
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_32 \
    RGB_32_TO_15

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_32_to_16_swap_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  32
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_32 \
    RGB_32_TO_16

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_32_to_24_swap_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  32
#define OUT_ADVANCE 24
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_32 \
    SWAP_32 \
    WRITE_24

#define CLEANUP emms();
#include "../csp_packed_packed.h"

/* Conversion from RGB formats to RGBA */

#define FUNC_NAME   rgb_15_to_rgba_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    RGB_15_TO_32_SWAP_RGBA

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   bgr_15_to_rgba_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    RGB_15_TO_32_RGBA

#define CLEANUP emms();
#include "../csp_packed_packed.h"


#define FUNC_NAME   rgb_16_to_rgba_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    RGB_16_TO_32_SWAP_RGBA

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   bgr_16_to_rgba_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_16 \
    RGB_16_TO_32_RGBA

#define CLEANUP emms();
#include "../csp_packed_packed.h"
    
#define FUNC_NAME   rgb_24_to_rgba_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  24
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_24 \
    WRITE_RGBA_32

#define INIT INIT_WRITE_RGBA_32

#define CLEANUP emms();
#include "../csp_packed_packed.h"
    
    
#define FUNC_NAME   bgr_24_to_rgba_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  24
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_24 \
    SWAP_32 \
    WRITE_RGBA_32

#define INIT INIT_WRITE_RGBA_32

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_32_to_rgba_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  32
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_32 \
    WRITE_RGBA_32

#define INIT INIT_WRITE_RGBA_32

#define CLEANUP emms();
#include "../csp_packed_packed.h"

#define FUNC_NAME   bgr_32_to_rgba_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  32
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
    LOAD_32 \
    SWAP_32 \
    WRITE_RGBA_32

#define INIT INIT_WRITE_RGBA_32

#define CLEANUP emms();
#include "../csp_packed_packed.h"

/* Conversion from RGBA to RGB formats */

#define INIT_RGBA_32 INTERPOLATE_INIT_TEMP;\
  tmp.ub[0] = ctx->options->background_16[0]>>8; \
  tmp.ub[2] = ctx->options->background_16[1]>>8; \
  tmp.ub[4] = ctx->options->background_16[2]>>8; \
  tmp.ub[6] = 0xff;\
  movq_m2r(tmp, mm1);\

/* rgba_32_to_rgb_15_c */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_rgb_15_mmx
#define CONVERT \
  INTERPOLATE_LOAD_SRC_RGBA32 \
  INTERPOLATE_1D \
  INTERPOLATE_WRITE_15_SWAP
  


#define INIT \
  INIT_RGBA_32

#define CLEANUP emms();
#include "../csp_packed_packed.h"


/* rgba_32_to_bgr_15_mmx */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_bgr_15_mmx
#define CONVERT \
  INTERPOLATE_LOAD_SRC_RGBA32 \
  INTERPOLATE_1D \
  INTERPOLATE_WRITE_15

#define INIT \
  INIT_RGBA_32

#define CLEANUP emms();
#include "../csp_packed_packed.h"


/* rgba_32_to_rgb_16_mmx */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_rgb_16_mmx
#define CONVERT \
  INTERPOLATE_LOAD_SRC_RGBA32 \
  INTERPOLATE_1D \
  INTERPOLATE_WRITE_16_SWAP

#define INIT \
  INIT_RGBA_32

#define CLEANUP emms();
#include  "../csp_packed_packed.h"

/* rgba_32_to_bgr_16_mmx */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint16_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 1
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_bgr_16_mmx
#define CONVERT \
  INTERPOLATE_LOAD_SRC_RGBA32 \
  INTERPOLATE_1D              \
  INTERPOLATE_WRITE_16


#define INIT \
  INIT_RGBA_32

#define CLEANUP emms();
#include  "../csp_packed_packed.h"


/* rgba_32_to_rgb_24_mmx */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_rgb_24_mmx
#define CONVERT \
  INTERPOLATE_LOAD_SRC_RGBA32 \
  INTERPOLATE_1D \
  INTERPOLATE_WRITE_RGB24

#define INIT \
  INIT_RGBA_32


#define CLEANUP emms();
#include  "../csp_packed_packed.h"

/* rgba_32_to_bgr_24_mmx */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 3
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_bgr_24_mmx
#define CONVERT \
  INTERPOLATE_LOAD_SRC_RGBA32 \
  INTERPOLATE_1D \
  INTERPOLATE_WRITE_BGR24

#define INIT \
  INIT_RGBA_32

#define CLEANUP emms();
#include  "../csp_packed_packed.h"

/* rgba_32_to_rgb_32_mmx */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_rgb_32_mmx
#define CONVERT \
  INTERPOLATE_LOAD_SRC_RGBA32 \
  INTERPOLATE_1D \
  INTERPOLATE_WRITE_RGB32

#define INIT \
  INIT_RGBA_32

#define CLEANUP emms();
#include  "../csp_packed_packed.h"

/* rgba_32_to_bgr_32_mmx */

#define IN_TYPE  uint8_t
#define OUT_TYPE uint8_t
#define IN_ADVANCE  4
#define OUT_ADVANCE 4
#define NUM_PIXELS  1
#define FUNC_NAME rgba_32_to_bgr_32_mmx
#define CONVERT \
  INTERPOLATE_LOAD_SRC_RGBA32 \
  INTERPOLATE_1D \
  INTERPOLATE_WRITE_BGR24
  

#define INIT \
  INIT_RGBA_32

#define CLEANUP emms();
#include  "../csp_packed_packed.h"


void
gavl_init_rgb_rgb_funcs_mmx(gavl_pixelformat_function_table_t * tab,
                            int width, const gavl_video_options_t * opt)
  {
  if(width % 8)
    return;

  /* Lossless conversions */
  
  tab->swap_rgb_24 = swap_rgb_24_mmx;
  tab->swap_rgb_32 = swap_rgb_32_mmx;
  tab->swap_rgb_16 = swap_rgb_16_mmx;
  tab->swap_rgb_15 = swap_rgb_15_mmx;

  /* Conversion from RGB formats to RGBA (lossless) */

  tab->rgb_24_to_rgba_32 = rgb_24_to_rgba_32_mmx;
  tab->bgr_24_to_rgba_32 = bgr_24_to_rgba_32_mmx;
  tab->rgb_32_to_rgba_32 = rgb_32_to_rgba_32_mmx;
  tab->bgr_32_to_rgba_32 = bgr_32_to_rgba_32_mmx;

  /* RGBA -> */

  if(opt->alpha_mode == GAVL_ALPHA_IGNORE)
    {
    tab->rgba_32_to_rgb_24    = rgb_32_to_24_mmx;
    tab->rgba_32_to_bgr_24    = rgb_32_to_24_swap_mmx;
    tab->rgba_32_to_bgr_32    = swap_rgb_32_mmx;
    }
  
  /* Conversions from fewer to more bits are not that good */

  if(opt->quality < 3)
    {
    tab->rgb_15_to_16 = rgb_15_to_16_mmx;
    tab->rgb_15_to_24 = rgb_15_to_24_mmx;
    tab->rgb_15_to_32 = rgb_15_to_32_mmx;

    tab->rgb_16_to_24 = rgb_16_to_24_mmx;
    tab->rgb_16_to_32 = rgb_16_to_32_mmx;

    tab->rgb_15_to_16_swap = rgb_15_to_16_swap_mmx;
    tab->rgb_15_to_24_swap = rgb_15_to_24_swap_mmx;
    tab->rgb_15_to_32_swap = rgb_15_to_32_swap_mmx;

    tab->rgb_16_to_24_swap = rgb_16_to_24_swap_mmx;
    tab->rgb_16_to_32_swap = rgb_16_to_32_swap_mmx;

    tab->rgb_15_to_rgba_32 = rgb_15_to_rgba_32_mmx;
    tab->bgr_15_to_rgba_32 = bgr_15_to_rgba_32_mmx;
    tab->rgb_16_to_rgba_32 = rgb_16_to_rgba_32_mmx;
    tab->bgr_16_to_rgba_32 = bgr_16_to_rgba_32_mmx;
    
    if(opt->alpha_mode == GAVL_ALPHA_BLEND_COLOR)
      {
      tab->rgba_32_to_rgb_15 = rgba_32_to_rgb_15_mmx;
      tab->rgba_32_to_bgr_15 = rgba_32_to_bgr_15_mmx;
      tab->rgba_32_to_rgb_16 = rgba_32_to_rgb_16_mmx;
      tab->rgba_32_to_bgr_16 = rgba_32_to_bgr_16_mmx;
      tab->rgba_32_to_rgb_24 = rgba_32_to_rgb_24_mmx;
      tab->rgba_32_to_bgr_24 = rgba_32_to_bgr_24_mmx;
      tab->rgba_32_to_rgb_32 = rgba_32_to_rgb_32_mmx;
      tab->rgba_32_to_bgr_32 = rgba_32_to_bgr_32_mmx;  
      }
    
    }

  /* Conversions from more to fewer bits are the same as the C versions.
     High quality versions however, might be better */

  if(opt->quality < 4)
    {
    tab->rgb_16_to_15 = rgb_16_to_15_mmx;
  
    tab->rgb_24_to_15 = rgb_24_to_15_mmx;
    tab->rgb_24_to_16 = rgb_24_to_16_mmx;
    tab->rgb_24_to_32 = rgb_24_to_32_mmx;
    
    tab->rgb_32_to_15 = rgb_32_to_15_mmx;
    tab->rgb_32_to_16 = rgb_32_to_16_mmx;
    tab->rgb_32_to_24 = rgb_32_to_24_mmx;
    
    tab->rgb_16_to_15_swap = rgb_16_to_15_swap_mmx;
    
    tab->rgb_24_to_15_swap = rgb_24_to_15_swap_mmx;
    tab->rgb_24_to_16_swap = rgb_24_to_16_swap_mmx;
    tab->rgb_24_to_32_swap = rgb_24_to_32_swap_mmx;
    
    tab->rgb_32_to_15_swap = rgb_32_to_15_swap_mmx;
    tab->rgb_32_to_16_swap = rgb_32_to_16_swap_mmx;
    tab->rgb_32_to_24_swap = rgb_32_to_24_swap_mmx;

    /* RGBA -> */
    
    if(opt->alpha_mode == GAVL_ALPHA_IGNORE)
      {
      tab->rgba_32_to_rgb_15    = rgb_32_to_15_mmx;
      tab->rgba_32_to_bgr_15    = rgb_32_to_15_swap_mmx;
      tab->rgba_32_to_rgb_16    = rgb_32_to_16_mmx;
      tab->rgba_32_to_bgr_16    = rgb_32_to_16_swap_mmx;
      }
    
    }
  

  }
#else


void
gavl_init_rgb_rgb_funcs_mmxext(gavl_pixelformat_function_table_t * tab,
                               int width, const gavl_video_options_t * opt)
  {
  }

#endif // !MMXEXT
