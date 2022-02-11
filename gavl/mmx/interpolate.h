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

#ifdef INTERPOLATE_USE_16 /* Only used in functions, which are a bit too slow anyway */
static const mmx_t interpolate_rgb16_upper_mask =   { 0x000000000000f800LL };
static const mmx_t interpolate_rgb16_middle_mask =  { 0x00000000000007e0LL };
static const mmx_t interpolate_rgb16_lower_mask =   { 0x000000000000001fLL };

static const mmx_t interpolate_rgb15_upper_mask =   { 0x0000000000007C00LL };
static const mmx_t interpolate_rgb15_middle_mask =  { 0x00000000000003e0LL };
static const mmx_t interpolate_rgb15_lower_mask =   { 0x000000000000001fLL };
#endif

#define INTERPOLATE_INIT_TEMP \
  mmx_t tmp;\
  pxor_r2r(mm5,mm5);\
  tmp.q = 0;

/* Interpolate C = src1 * F + src2 * (1 - F) = F * (A - B) + B
 *  
 * before:
 * mm0: 00 A3 00 A2 00 A1 00 A0 (src 1)
 * mm1: 00 B3 00 B2 00 B1 00 B0 (src 2)
 * mm4: 00 F3 00 F2 00 F1 00 F0 0..7f
 * mm5: 00 00 00 00 00 00 00 00 (Must be cleared before)
 *
 * after:
 * mm7: 00 C3 00 C3 00 C1 00 C0
 * mm0: destroyed
 */

#define INTERPOLATE_1D \
  movq_r2r(mm0, mm7);    /* mm7: A */                        \
  psubsw_r2r(mm1, mm7);  /* mm7: A-B */                      \
  pmullw_r2r(mm4, mm7);  /* mm7: F * (A_B) */                \
  psraw_i2r(7, mm7);     /* fit into 8 bits  */              \
  paddsw_r2r(mm1, mm7);  /* mm7: F * (A_B)+B */

#define INTERPOLATE_LOAD_SRC_RGBA32 \
  movd_m2r(*src, mm0);\
  movq_r2r(mm0, mm4);\
  punpcklbw_r2r(mm5, mm0);\
  psrlq_i2r(25, mm4);\
  movq_r2r(mm4, mm6);\
  psllq_i2r(16, mm6);\
  por_r2r(mm6, mm4);\
  movq_r2r(mm4, mm6);\
  psllq_i2r(32, mm6);\
  por_r2r(mm6, mm4);

#define INTERPOLATE_1D_LOAD_SRC_1_15 \
  tmp.q = *src_1;                                                   \
  movq_m2r(tmp, mm7);                                                   \
  movq_r2r(mm7, mm0);/* Lower 5 bits */                                 \
  pand_m2r(interpolate_rgb15_lower_mask, mm0);                          \
  psllq_i2r(3, mm0);                                                    \
  movq_r2r(mm7, mm6);/* Middle 5 bits */                                \
  pand_m2r(interpolate_rgb15_middle_mask, mm6);                         \
  psllq_i2r(14, mm6);                                                   \
  por_r2r(mm6, mm0);                                                    \
  pand_m2r(interpolate_rgb15_upper_mask, mm7);/* Upper 5 bits */        \
  psllq_i2r(25, mm7);                                                   \
  por_r2r(mm7, mm0);

#define INTERPOLATE_1D_LOAD_SRC_2_15 \
  tmp.q = *src_2;                                       \
  movq_m2r(tmp, mm7);                                       \
  movq_r2r(mm7, mm1);                                       \
  pand_m2r(interpolate_rgb15_lower_mask, mm1);              \
  psllq_i2r(3, mm1);                                        \
  movq_r2r(mm7, mm6);                                       \
  pand_m2r(interpolate_rgb15_middle_mask, mm6);             \
  psllq_i2r(14, mm6);                                       \
  por_r2r(mm6, mm1);                                        \
  pand_m2r(interpolate_rgb15_upper_mask, mm7);              \
  psllq_i2r(25, mm7);                                       \
  por_r2r(mm7, mm1);


#define INTERPOLATE_1D_LOAD_SRC_1_16 \
  tmp.q = *src_1;                                                       \
  movq_m2r(tmp, mm7);                                                   \
  movq_r2r(mm7, mm0);/* Lower 5 bits */                                 \
  pand_m2r(interpolate_rgb16_lower_mask, mm0);                          \
  psllq_i2r(3, mm0);                                                    \
  movq_r2r(mm7, mm6);/* Middle 6 bits */                                \
  pand_m2r(interpolate_rgb16_middle_mask, mm6);                         \
  psllq_i2r(13, mm6);                                                   \
  por_r2r(mm6, mm0);                                                    \
  pand_m2r(interpolate_rgb16_upper_mask, mm7);/* Upper 5 bits */        \
  psllq_i2r(24, mm7);                                                   \
  por_r2r(mm7, mm0);

#define INTERPOLATE_1D_LOAD_SRC_2_16 \
  tmp.q = *src_1;                                                       \
  movq_m2r(tmp, mm7);                                                   \
  movq_r2r(mm7, mm1);/* Lower 5 bits */                                 \
  pand_m2r(interpolate_rgb16_lower_mask, mm1);                          \
  psllq_i2r(3, mm1);                                                    \
  movq_r2r(mm7, mm6);/* Middle 6 bits */                                \
  pand_m2r(interpolate_rgb16_middle_mask, mm6);                         \
  psllq_i2r(13, mm6);                                                   \
  por_r2r(mm6, mm1);                                                    \
  pand_m2r(interpolate_rgb16_upper_mask, mm7);/* Upper 5 bits */        \
  psllq_i2r(24, mm7);                                                   \
  por_r2r(mm7, mm1);


#define INTERPOLATE_1D_LOAD_SRC_1_24            \
  tmp.uw[0] = src_1[0];                           \
  tmp.uw[1] = src_1[1];                           \
  tmp.uw[2] = src_1[2];                           \
  movq_m2r(tmp, mm0);

#define INTERPOLATE_1D_LOAD_SRC_2_24            \
  tmp.uw[0] = src_2[0];                           \
  tmp.uw[1] = src_2[1];                           \
  tmp.uw[2] = src_2[2];                           \
  movq_m2r(tmp, mm1);

#define INTERPOLATE_1D_LOAD_SRC_1_32 \
  movd_m2r(*src_1, mm0);         \
  punpcklbw_r2r(mm5, mm0);

#define INTERPOLATE_1D_LOAD_SRC_2_32 \
  movd_m2r(*src_2, mm1);         \
  punpcklbw_r2r(mm5, mm1);

#define INTERPOLATE_WRITE_RGBA32 \
  packuswb_r2r(mm5, mm7);/* Pack result */\
  movd_r2m(mm7, *dst);

#define INTERPOLATE_WRITE_RGB32 \
  packuswb_r2r(mm5, mm7);/* Pack result */\
  movd_r2m(mm7, *dst); \
  dst[3] = 0xff;

#define INTERPOLATE_WRITE_RGB24 \
  MOVQ_R2M(mm7, tmp); \
  dst[0] = tmp.ub[0]; \
  dst[1] = tmp.ub[2]; \
  dst[2] = tmp.ub[4];

#define INTERPOLATE_WRITE_BGR24 \
  MOVQ_R2M(mm7, tmp); \
  dst[0] = tmp.ub[4]; \
  dst[1] = tmp.ub[2]; \
  dst[2] = tmp.ub[0];

#define INTERPOLATE_WRITE_15 \
  psrlw_i2r(3, mm7);\
  movq_r2r(mm7,mm6);\
  pand_m2r(interpolate_rgb15_lower_mask, mm6);\
  movq_r2r(mm6,mm0);\
  movq_r2r(mm7,mm6);\
  psrlq_i2r(11, mm6);\
  pand_m2r(interpolate_rgb15_middle_mask, mm6);\
  por_r2r(mm6,mm0);\
  psrlq_i2r(22, mm7);\
  pand_m2r(interpolate_rgb15_upper_mask, mm7);\
  por_r2r(mm7,mm0);\
  MOVQ_R2M(mm0, tmp);\
  *dst = tmp.uw[0];

#define INTERPOLATE_WRITE_15_SWAP \
  psrlw_i2r(3, mm7);\
  movq_r2r(mm7,mm6);\
  pand_m2r(interpolate_rgb15_lower_mask, mm6);\
  movq_r2r(mm6,mm0);\
  psllw_i2r(10, mm0);\
  movq_r2r(mm7,mm6);\
  psrlq_i2r(11, mm6);\
  pand_m2r(interpolate_rgb15_middle_mask, mm6);\
  por_r2r(mm6,mm0);\
  psrlq_i2r(32, mm7);\
  por_r2r(mm7,mm0);\
  MOVQ_R2M(mm0, tmp);\
  *dst = tmp.uw[0];

#define INTERPOLATE_WRITE_16 \
  psrlw_i2r(2, mm7);\
  movq_r2r(mm7,mm6);\
  psrlw_i2r(1, mm6);\
  pand_m2r(interpolate_rgb16_lower_mask, mm6);\
  movq_r2r(mm6,mm0);\
  movq_r2r(mm7,mm6);                              \
  psrlq_i2r(11, mm6);\
  pand_m2r(interpolate_rgb16_middle_mask, mm6);\
  por_r2r(mm6,mm0);\
  psrlq_i2r(22, mm7);\
  pand_m2r(interpolate_rgb16_upper_mask, mm7);\
  por_r2r(mm7,mm0);\
  MOVQ_R2M(mm0, tmp);\
  *dst = tmp.uw[0];

#define INTERPOLATE_WRITE_16_SWAP \
  psrlw_i2r(2, mm7);\
  movq_r2r(mm7,mm6);\
  psrlw_i2r(1, mm6);\
  pand_m2r(interpolate_rgb16_lower_mask, mm6);\
  movq_r2r(mm6,mm0);\
  psllw_i2r(11, mm0);\
  movq_r2r(mm7,mm6);\
  psrlq_i2r(11, mm6);\
  pand_m2r(interpolate_rgb16_middle_mask, mm6);\
  por_r2r(mm6,mm0);\
  psrlq_i2r(33, mm7);\
  pand_m2r(interpolate_rgb16_lower_mask, mm7);\
  por_r2r(mm7,mm0);\
  MOVQ_R2M(mm0, tmp);\
  *dst = tmp.uw[0];
  
