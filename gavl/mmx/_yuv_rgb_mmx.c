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

/*
 *  Support for mmxext
 *  this macro procudes another set of
 *  functions in ../mmxext
 */



/* Constants for YUV -> RGB conversion */

static const mmx_t mmx_80w =     { 0x0080008000800080LL };


static const mmx_t mmx_U_green = { 0xf37df37df37df37dLL }; // U Green: -3203 (= -0.34414*255.0/224.0 * 8192)
static const mmx_t mmx_U_blue =  { 0x4093409340934093LL }; // U Blue:  16531 (=  1.77200*255.0/224.0 * 8192)
static const mmx_t mmx_V_red =   { 0x3312331233123312LL }; // V red:   13074 (=  1.40200*255.0/224.0 * 8192)
static const mmx_t mmx_V_green = { 0xe5fce5fce5fce5fcLL }; // V Green: -6660 (= -0.71414*255.0/224.0 * 8192)
static const mmx_t mmx_Y_coeff = { 0x253f253f253f253fLL }; // Y Coeff:  9535 (=          255.0/219.0 * 8192)

#if 1
static const mmx_t mmx_UJ_green = { 0xf4fdf4fdf4fdf4fdLL }; // U Green: -2819 (= -0.34414 * 8192)
static const mmx_t mmx_UJ_blue =  { 0x38b438b438b438b4LL }; // U Blue:  14516 (=  1.77200 * 8192)
static const mmx_t mmx_VJ_red =   { 0x2cdd2cdd2cdd2cddLL }; // V red:   11485 (=  1.40200 * 8192)
static const mmx_t mmx_VJ_green = { 0xe926e926e926e926LL }; // V Green: -5850 (= -0.71414 * 8192)
static const mmx_t mmx_YJ_coeff = { 0x2000200020002000LL }; // Y Coeff:  8192 (=            8192)
#endif

static const mmx_t mmx_10w =     { 0x1010101010101010LL };
static const mmx_t mmx_00ffw =   { 0x00ff00ff00ff00ffLL };
static const mmx_t mmx_ff00w =   { 0xff00ff00ff00ff00LL };

/* Macros for loading the YUV images into the MMX registers */

#define LOAD_YUV_PLANAR_2 movq_m2r (*src_y, mm6);\
                          movd_m2r (*src_u, mm0);\
                          movd_m2r (*src_v, mm1);\
                          pxor_r2r (mm4, mm4);

#define INIT_YUV_PLANAR_4 int32_t i_tmp;

#define LOAD_YUV_PLANAR_4 movq_m2r (*src_y, mm6);\
                          i_tmp = (((int32_t)src_u[1])) << 8 | src_u[0];\
                          movd_m2r (i_tmp, mm0); \
                          i_tmp = (((int32_t)src_v[1])) << 8 | src_v[0];\
                          movd_m2r (i_tmp, mm1);\
                          pxor_r2r (mm4, mm4);\
                          punpcklbw_r2r(mm4, mm0);\
                          punpcklbw_r2r(mm4, mm1);\
                          movq_r2r (mm0, mm2);\
                          psllw_i2r(8,mm2);\
                          por_r2r(mm2, mm0);\
                          movq_r2r (mm1, mm2);\
                          psllw_i2r(8,mm2);\
                          por_r2r(mm2, mm1);

#define LOAD_YUY2  movq_m2r(*src,mm0);/*          mm0: V2 Y3 U2 Y2 V0 Y1 U0 Y0 */\
                   movq_m2r(*(src+8),mm1);/*      mm1: V6 Y7 U6 Y6 V4 Y5 U4 Y4 */\
                   movq_r2r(mm0,mm2);/*           mm2: V2 Y3 U2 Y2 V0 Y1 U0 Y0 */\
                   pand_m2r(mmx_00ffw,mm2);/*     mm2: 00 Y3 00 Y2 00 Y1 00 Y0 */\
                   pxor_r2r(mm4, mm4);/*          Zero mm4 */\
                   packuswb_r2r(mm4,mm2);/*       mm2: 00 00 00 00 Y3 Y2 Y1 Y0 */\
                   movq_r2r(mm1,mm3);/*           mm3: V6 Y7 U6 Y6 V4 Y5 U4 Y4 */\
                   pand_m2r(mmx_00ffw,mm3);/*     mm3: 00 Y7 00 Y6 00 Y5 00 Y4 */\
                   pxor_r2r(mm6, mm6);/*          Zero mm6 */\
                   packuswb_r2r(mm3,mm6);/*       mm6: Y7 Y6 Y5 Y4 00 00 00 00 */\
                   por_r2r(mm2,mm6);/*            mm6: Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */\
                   psrlw_i2r(8,mm0);/*            mm0: 00 V2 00 U2 00 V0 00 U0 */\
                   psrlw_i2r(8,mm1);/*            mm1: 00 V6 00 U6 00 V4 00 U4 */\
                   packuswb_r2r(mm1,mm0);/*       mm0: V6 U6 V4 U4 V2 U2 V0 U0 */\
                   movq_r2r(mm0,mm1);/*           mm1: V6 U6 V4 U4 V2 U2 V0 U0 */\
                   pand_m2r(mmx_00ffw,mm0);/*     mm0: 00 U6 00 U4 00 U2 00 U0 */\
                   psrlw_i2r(8,mm1);/*            mm1: 00 V6 00 V4 00 V2 00 V0 */\
                   packuswb_r2r(mm4,mm0);/*       mm0: 00 00 00 00 U6 U4 U2 U0 */\
                   packuswb_r2r(mm4,mm1);/*       mm1: 00 00 00 00 V6 V4 V2 V0 */

#define LOAD_UYVY  movq_m2r(*src,mm0);/*          mm0: Y3 V2 Y2 U2 Y1 V0 Y0 U0 */\
                   movq_m2r(*(src+8),mm1);/*      mm1: Y7 V6 Y6 U6 Y5 V4 Y4 U4 */\
                   movq_r2r(mm0,mm2);/*           mm2: Y3 V2 Y2 U2 Y1 V0 Y0 U0 */\
                   pand_m2r(mmx_ff00w,mm2);/*     mm2: Y3 00 Y2 00 Y1 00 Y0 00 */\
                   psrlw_i2r(8,mm2);/*            mm2: 00 Y3 00 Y2 00 Y1 00 Y0 */\
                   pxor_r2r(mm4, mm4);/*          Zero mm4 */                    \
                   packuswb_r2r(mm4,mm2);/*       mm2: 00 00 00 00 Y3 Y2 Y1 Y0 */\
                   movq_r2r(mm1,mm3);/*           mm3: Y7 V6 Y6 U6 Y5 V4 Y4 U4 */\
                   pand_m2r(mmx_ff00w,mm3);/*     mm3: Y7 00 Y6 00 Y5 00 Y4 00 */\
                   psrlw_i2r(8,mm3);/*            mm3: 00 Y7 00 Y6 00 Y5 00 Y4 */\
                   pxor_r2r(mm6, mm6);/*          Zero mm6 */\
                   packuswb_r2r(mm3,mm6);/*       mm6: Y7 Y6 Y5 Y4 00 00 00 00 */\
                   por_r2r(mm2,mm6);/*            mm6: Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */\
                   pand_m2r(mmx_00ffw,mm0);/*     mm0: 00 V2 00 U2 00 V0 00 U0 */\
                   pand_m2r(mmx_00ffw,mm1);/*     mm1: 00 V6 00 U6 00 V4 00 U4 */\
                   packuswb_r2r(mm1,mm0);/*       mm0: V6 U6 V4 U4 V2 U2 V0 U0 */\
                   movq_r2r(mm0,mm1);/*           mm1: V6 U6 V4 U4 V2 U2 V0 U0 */\
                   pand_m2r(mmx_00ffw,mm0);/*     mm0: 00 U6 00 U4 00 U2 00 U0 */\
                   psrlw_i2r(8,mm1);/*            mm1: 00 V6 00 V4 00 V2 00 V0 */\
                   packuswb_r2r(mm4,mm0);/*       mm0: 00 00 00 00 U6 U4 U2 U0 */\
                   packuswb_r2r(mm4,mm1);/*       mm1: 00 00 00 00 V6 V4 V2 V0 */

/* This macro converts 8 Pixels at once (taken from mpeg2dec) */

/*
 * Before the conversion:
 *
 * mm0 = 00 00 00 00 u3 u2 u1 u0
 * mm1 = 00 00 00 00 v3 v2 v1 v0
 * mm6 = Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0
 * mm4 = 00 00 00 00 00 00 00 00 (must be zerod before!)
 * After the conversion:
 *
 * mm0 = B7 B6 B5 B4 B3 B2 B1 B0
 * mm1 = R7 R6 R5 R4 R3 R2 R1 R0
 * mm2 = G7 G6 G5 G4 G3 G2 G1 G0
 *
 */

#define YUV_2_RGB punpcklbw_r2r (mm4, mm0);/*      mm0 = u3 u2 u1 u0 */\
                  punpcklbw_r2r (mm4, mm1);/*      mm1 = v3 v2 v1 v0 */\
                  psubsw_m2r (mmx_80w, mm0);/*     u -= 128 */\
                  psubsw_m2r (mmx_80w, mm1);/*     v -= 128 */\
                  psllw_i2r (3, mm0);/*            promote precision */\
                  psllw_i2r (3, mm1);/*            promote precision */\
                  movq_r2r (mm0, mm2);/*           mm2 = u3 u2 u1 u0 */\
                  movq_r2r (mm1, mm3);/*           mm3 = v3 v2 v1 v0 */\
                  pmulhw_m2r (mmx_U_green, mm2);/* mm2 = u * u_green */\
                  pmulhw_m2r (mmx_V_green, mm3);/* mm3 = v * v_green */\
                  pmulhw_m2r (mmx_U_blue, mm0);/*  mm0 = chroma_b */\
                  pmulhw_m2r (mmx_V_red, mm1);/*   mm1 = chroma_r */\
                  paddsw_r2r (mm3, mm2);/*         mm2 = chroma_g */\
                  psubusb_m2r (mmx_10w, mm6);/*    Y -= 16  */\
                  movq_r2r (mm6, mm7);/*           mm7 = Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */\
                  pand_m2r (mmx_00ffw, mm6);/*     mm6 =    Y6    Y4    Y2    Y0 */\
                  psrlw_i2r (8, mm7);/*            mm7 =    Y7    Y5    Y3    Y1 */\
                  psllw_i2r (3, mm6);/*            promote precision */\
                  psllw_i2r (3, mm7);/*            promote precision */\
                  pmulhw_m2r (mmx_Y_coeff, mm6);/* mm6 = luma_rgb even */\
                  pmulhw_m2r (mmx_Y_coeff, mm7);/* mm7 = luma_rgb odd */\
                  movq_r2r (mm0, mm3);/*           mm3 = chroma_b */\
                  movq_r2r (mm1, mm4);/*           mm4 = chroma_r */\
                  movq_r2r (mm2, mm5);/*           mm5 = chroma_g */\
                  paddsw_r2r (mm6, mm0);/*         mm0 = B6 B4 B2 B0 */\
                  paddsw_r2r (mm7, mm3);/*         mm3 = B7 B5 B3 B1 */\
                  paddsw_r2r (mm6, mm1);/*         mm1 = R6 R4 R2 R0 */\
                  paddsw_r2r (mm7, mm4);/*         mm4 = R7 R5 R3 R1 */\
                  paddsw_r2r (mm6, mm2);/*         mm2 = G6 G4 G2 G0 */\
                  paddsw_r2r (mm7, mm5);/*         mm5 = G7 G5 G3 G1 */\
                  packuswb_r2r (mm0, mm0);/*       saturate to 0-255 */\
                  packuswb_r2r (mm1, mm1);/*       saturate to 0-255 */\
                  packuswb_r2r (mm2, mm2);/*       saturate to 0-255 */\
                  packuswb_r2r (mm3, mm3);/*       saturate to 0-255 */\
                  packuswb_r2r (mm4, mm4);/*       saturate to 0-255 */\
                  packuswb_r2r (mm5, mm5);/*       saturate to 0-255 */\
                  punpcklbw_r2r (mm3, mm0);/*      mm0 = B7 B6 B5 B4 B3 B2 B1 B0 */\
                  punpcklbw_r2r (mm4, mm1);/*      mm1 = R7 R6 R5 R4 R3 R2 R1 R0 */\
                  punpcklbw_r2r (mm5, mm2);/*      mm2 = G7 G6 G5 G4 G3 G2 G1 G0 */

/* Same as above, but for JPEG quantization */

#define YUVJ_2_RGB punpcklbw_r2r (mm4, mm0);/*      mm0 = u3 u2 u1 u0 */\
                  punpcklbw_r2r (mm4, mm1);/*      mm1 = v3 v2 v1 v0 */\
                  psubsw_m2r (mmx_80w, mm0);/*     u -= 128 */\
                  psubsw_m2r (mmx_80w, mm1);/*     v -= 128 */\
                  psllw_i2r (3, mm0);/*            promote precision */\
                  psllw_i2r (3, mm1);/*            promote precision */\
                  movq_r2r (mm0, mm2);/*           mm2 = u3 u2 u1 u0 */\
                  movq_r2r (mm1, mm3);/*           mm3 = v3 v2 v1 v0 */\
                  pmulhw_m2r (mmx_UJ_green, mm2);/* mm2 = u * u_green */\
                  pmulhw_m2r (mmx_VJ_green, mm3);/* mm3 = v * v_green */\
                  pmulhw_m2r (mmx_UJ_blue, mm0);/*  mm0 = chroma_b */\
                  pmulhw_m2r (mmx_VJ_red, mm1);/*   mm1 = chroma_r */\
                  paddsw_r2r (mm3, mm2);/*         mm2 = chroma_g */\
                  /* psubusb_m2r (mmx_10w, mm6);      Y -= 16  */\
                  movq_r2r (mm6, mm7);/*           mm7 = Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 */\
                  pand_m2r (mmx_00ffw, mm6);/*     mm6 =    Y6    Y4    Y2    Y0 */\
                  psrlw_i2r (8, mm7);/*            mm7 =    Y7    Y5    Y3    Y1 */\
                  psllw_i2r (3, mm6);/*            promote precision */\
                  psllw_i2r (3, mm7);/*            promote precision */\
                  pmulhw_m2r (mmx_YJ_coeff, mm6);/* mm6 = luma_rgb even */\
                  pmulhw_m2r (mmx_YJ_coeff, mm7);/* mm7 = luma_rgb odd */\
                  movq_r2r (mm0, mm3);/*           mm3 = chroma_b */\
                  movq_r2r (mm1, mm4);/*           mm4 = chroma_r */\
                  movq_r2r (mm2, mm5);/*           mm5 = chroma_g */\
                  paddsw_r2r (mm6, mm0);/*         mm0 = B6 B4 B2 B0 */\
                  paddsw_r2r (mm7, mm3);/*         mm3 = B7 B5 B3 B1 */\
                  paddsw_r2r (mm6, mm1);/*         mm1 = R6 R4 R2 R0 */\
                  paddsw_r2r (mm7, mm4);/*         mm4 = R7 R5 R3 R1 */\
                  paddsw_r2r (mm6, mm2);/*         mm2 = G6 G4 G2 G0 */\
                  paddsw_r2r (mm7, mm5);/*         mm5 = G7 G5 G3 G1 */\
                  packuswb_r2r (mm0, mm0);/*       saturate to 0-255 */\
                  packuswb_r2r (mm1, mm1);/*       saturate to 0-255 */\
                  packuswb_r2r (mm2, mm2);/*       saturate to 0-255 */\
                  packuswb_r2r (mm3, mm3);/*       saturate to 0-255 */\
                  packuswb_r2r (mm4, mm4);/*       saturate to 0-255 */\
                  packuswb_r2r (mm5, mm5);/*       saturate to 0-255 */\
                  punpcklbw_r2r (mm3, mm0);/*      mm0 = B7 B6 B5 B4 B3 B2 B1 B0 */\
                  punpcklbw_r2r (mm4, mm1);/*      mm1 = R7 R6 R5 R4 R3 R2 R1 R0 */\
                  punpcklbw_r2r (mm5, mm2);/*      mm2 = G7 G6 G5 G4 G3 G2 G1 G0 */


/*******************************************
 *   Output Macros
 *******************************************/

/*
 *  mm0 = B7 B6 B5 B4 B3 B2 B1 B0
 *  mm1 = R7 R6 R5 R4 R3 R2 R1 R0
 *  mm2 = G7 G6 G5 G4 G3 G2 G1 G0
 */

#define OUTPUT_BGR_32 pxor_r2r (mm3, mm3);\
                      movq_r2r (mm0, mm6);\
                      punpcklbw_r2r (mm2, mm6);\
                      movq_r2r (mm1, mm7);\
                      punpcklbw_r2r (mm3, mm7);\
                      movq_r2r (mm0, mm4);\
                      punpcklwd_r2r (mm7, mm6);\
                      movq_r2r (mm1, mm5);\
                      MOVQ_R2M (mm6, *dst);\
                      movq_r2r (mm0, mm6);\
                      punpcklbw_r2r (mm2, mm6);\
                      punpckhwd_r2r (mm7, mm6);\
                      MOVQ_R2M (mm6, *(dst+8));\
                      punpckhbw_r2r (mm2, mm4);\
                      punpckhbw_r2r (mm3, mm5);\
                      punpcklwd_r2r (mm5, mm4);\
                      MOVQ_R2M (mm4, *(dst+16));\
                      movq_r2r (mm0, mm4);\
                      punpckhbw_r2r (mm2, mm4);\
                      punpckhwd_r2r (mm5, mm4);\
                      MOVQ_R2M (mm4, *(dst+24));

#define OUTPUT_RGB_32 pxor_r2r (mm3, mm3);\
                      movq_r2r (mm1, mm6);\
                      punpcklbw_r2r (mm2, mm6);\
                      movq_r2r (mm0, mm7);\
                      punpcklbw_r2r (mm3, mm7);\
                      movq_r2r (mm1, mm4);\
                      punpcklwd_r2r (mm7, mm6);\
                      movq_r2r (mm0, mm5);\
                      MOVQ_R2M (mm6, *dst);\
                      movq_r2r (mm1, mm6);\
                      punpcklbw_r2r (mm2, mm6);\
                      punpckhwd_r2r (mm7, mm6);\
                      MOVQ_R2M (mm6, *(dst+8));\
                      punpckhbw_r2r (mm2, mm4);\
                      punpckhbw_r2r (mm3, mm5);\
                      punpcklwd_r2r (mm5, mm4);\
                      MOVQ_R2M (mm4, *(dst+16));\
                      movq_r2r (mm1, mm4);\
                      punpckhbw_r2r (mm2, mm4);\
                      punpckhwd_r2r (mm5, mm4);\
                      MOVQ_R2M (mm4, *(dst+24));



#ifdef MMXEXT

/*
 *  mm0 = B7 B6 B5 B4 B3 B2 B1 B0
 *  mm1 = R7 R6 R5 R4 R3 R2 R1 R0
 *  mm2 = G7 G6 G5 G4 G3 G2 G1 G0
 */

static const mmx_t M24A= { 0x00FF0000FF0000FFLL };
static const mmx_t M24B= { 0xFF0000FF0000FF00LL };
static const mmx_t M24C= { 0x0000FF0000FF0000LL };

#define OUTPUT_RGB_24 \
        movq_m2r(M24A, mm4); \
        movq_m2r(M24C, mm7); \
        pshufw_r2r(mm1, mm5, 0x50); /* B3 B2 B3 B2  B1 B0 B1 B0 */ \
        pshufw_r2r(mm2, mm3, 0x50); /* G3 G2 G3 G2  G1 G0 G1 G0 */ \
        pshufw_r2r(mm0, mm6,  0x00); /* R1 R0 R1 R0  R1 R0 R1 R0 */ \
        pand_r2r(mm4, mm5); /*    B2        B1       B0 */ \
        pand_r2r(mm4, mm3); /*    G2        G1       G0 */ \
        pand_r2r(mm7, mm6); /*       R1        R0       */ \
        psllq_i2r(8, mm3); /* G2        G1       G0    */ \
        por_r2r(mm5, mm6);\
        por_r2r(mm3, mm6);\
        MOVQ_R2M(mm6, *(dst));\
        psrlq_i2r(8, mm2); /* 00 G7 G6 G5  G4 G3 G2 G1 */\
        pshufw_r2r(mm1, mm5, 0xA5); /* B5 B4 B5 B4  B3 B2 B3 B2 */ \
        pshufw_r2r(mm2, mm3, 0x55); /* G4 G3 G4 G3  G4 G3 G4 G3 */ \
        pshufw_r2r(mm0, mm6, 0xA5); /* R5 R4 R5 R4  R3 R2 R3 R2 */ \
        pand_m2r(M24B, mm5); /* B5       B4        B3    */ \
        pand_r2r(mm7, mm3); /*       G4        G3       */ \
        pand_r2r(mm4, mm6); /*    R4        R3       R2 */ \
        por_r2r(mm5, mm3); /* B5    G4 B4     G3 B3    */\
        por_r2r(mm3, mm6       );\
        MOVQ_R2M(mm6, *(dst+8));\
        pshufw_r2r(mm1, mm5, 0xFF); /* B7 B6 B7 B6  B7 B6 B6 B7 */\
        pshufw_r2r(mm2, mm3, 0xFA); /* 00 G7 00 G7  G6 G5 G6 G5 */\
        pshufw_r2r(mm0, mm6, 0xFA); /* R7 R6 R7 R6  R5 R4 R5 R4 */\
        pand_r2r(mm7, mm5); /*       B7        B6       */\
        pand_r2r(mm4, mm3); /*    G7        G6       G5 */\
        pand_m2r(M24B, mm6); /* R7       R6        R5    */\
        por_r2r(mm5, mm3);\
        por_r2r(mm3, mm6);\
        MOVQ_R2M(mm6, *(dst+16));


#define OUTPUT_BGR_24 \
        movq_m2r(M24A, mm4); \
        movq_m2r(M24C, mm7); \
        pshufw_r2r(mm0, mm5, 0x50); /* B3 B2 B3 B2  B1 B0 B1 B0 */ \
        pshufw_r2r(mm2, mm3, 0x50); /* G3 G2 G3 G2  G1 G0 G1 G0 */ \
        pshufw_r2r(mm1, mm6,  0x00); /* R1 R0 R1 R0  R1 R0 R1 R0 */ \
        pand_r2r(mm4, mm5); /*    B2        B1       B0 */ \
        pand_r2r(mm4, mm3); /*    G2        G1       G0 */ \
        pand_r2r(mm7, mm6); /*       R1        R0       */ \
        psllq_i2r(8, mm3); /* G2        G1       G0    */ \
        por_r2r(mm5, mm6);\
        por_r2r(mm3, mm6);\
        MOVQ_R2M(mm6, *(dst));\
        psrlq_i2r(8, mm2); /* 00 G7 G6 G5  G4 G3 G2 G1 */\
        pshufw_r2r(mm0, mm5, 0xA5); /* B5 B4 B5 B4  B3 B2 B3 B2 */ \
        pshufw_r2r(mm2, mm3, 0x55); /* G4 G3 G4 G3  G4 G3 G4 G3 */ \
        pshufw_r2r(mm1, mm6, 0xA5); /* R5 R4 R5 R4  R3 R2 R3 R2 */ \
        pand_m2r(M24B, mm5); /* B5       B4        B3    */ \
        pand_r2r(mm7, mm3); /*       G4        G3       */ \
        pand_r2r(mm4, mm6); /*    R4        R3       R2 */ \
        por_r2r(mm5, mm3); /* B5    G4 B4     G3 B3    */\
        por_r2r(mm3, mm6       );\
        MOVQ_R2M(mm6, *(dst+8));\
        pshufw_r2r(mm0, mm5, 0xFF); /* B7 B6 B7 B6  B7 B6 B6 B7 */\
        pshufw_r2r(mm2, mm3, 0xFA); /* 00 G7 00 G7  G6 G5 G6 G5 */\
        pshufw_r2r(mm1, mm6, 0xFA); /* R7 R6 R7 R6  R5 R4 R5 R4 */\
        pand_r2r(mm7, mm5); /*       B7        B6       */\
        pand_r2r(mm4, mm3); /*    G7        G6       G5 */\
        pand_m2r(M24B, mm6); /* R7       R6        R5    */\
        por_r2r(mm5, mm3);\
        por_r2r(mm3, mm6);\
        MOVQ_R2M(mm6, *(dst+16));


#else

/*
 *  mm0 = B7 B6 B5 B4 B3 B2 B1 B0
 *  mm1 = R7 R6 R5 R4 R3 R2 R1 R0
 *  mm2 = G7 G6 G5 G4 G3 G2 G1 G0
 */

static const mmx_t yuv_rgb_lowest_word = { 0x000000000000FFFFLL };
static const mmx_t yuv_rgb_lowest_byte = { 0x00000000000000FFLL };

#define OUTPUT_RGB_24 pxor_r2r(mm3, mm3);/*                 Zero -> mm3 */\
                      movq_r2r(mm2, mm4);/*                 mm4: G7 G6 G5 G4 G3 G2 G1 G0 */\
                      movq_r2r(mm1, mm5);/*                 mm5: R7 R6 R5 R4 R3 R2 R1 R0 */\
                      movq_r2r(mm1, mm5);/*                 mm5: R7 R6 R5 R4 R3 R2 R1 R0 */\
                      punpcklbw_r2r(mm3, mm5);/*            mm5: 00 R3 00 R2 00 R1 00 R0 */\
                      punpcklbw_r2r(mm4, mm3);/*            mm3: G3 00 G2 00 G1 00 G0 00 */\
                      por_r2r(mm5, mm3);/*                  mm3: G3 R3 G2 R2 G1 R1 G0 R0 */\
                      movq_r2r(mm0, mm5);/*                 mm5: B7 B6 B5 B4 B3 B2 B1 B0 */\
                      pand_m2r(yuv_rgb_lowest_byte, mm5);/* mm5: 00 00 00 00 00 00 00 B0 */\
                      movq_r2r(mm3, mm7);/*                 mm7: G3 R3 G2 R2 G1 R1 G0 R0 */\
                      pand_m2r(yuv_rgb_lowest_word, mm7);/* mm7: 00 00 00 00 00 00 G0 R0 */\
                      psllq_i2r(16, mm5);/*                 mm5: 00 00 00 00 00 B0 00 00 */\
                      por_r2r(mm5, mm7);/*                  mm7: 00 00 00 00 00 B0 G0 R0 */\
                      psrlq_i2r(16, mm3);/*                 mm3: 00 00 G3 R3 G2 R2 G1 R1 */\
                      psrlq_i2r(8, mm0);/*                  mm0: 00 B7 B6 B5 B4 B3 B2 B1 */\
                      movq_r2r(mm0, mm5);/*                 mm5: 00 B7 B6 B5 B4 B3 B2 B1 */\
                      pand_m2r(yuv_rgb_lowest_byte, mm5);/* mm5: 00 00 00 00 00 00 00 B1 */\
                      psllq_i2r(40, mm5);/*                 mm5: 00 00 B1 00 00 00 00 00 */\
                      por_r2r(mm5, mm7);/*                  mm7: 00 00 B1 00 00 B0 G0 R0 */\
                      movq_r2r(mm3, mm6);/*                 mm7: 00 00 G3 R3 G2 R2 G1 R1 */\
                      pand_m2r(yuv_rgb_lowest_word, mm6);/* mm6: 00 00 00 00 00 00 G1 R1 */\
                      psllq_i2r(24, mm6);/*                 mm6: 00 00 00 G1 R1 00 00 00 */\
                      por_r2r(mm6, mm7);/*                  mm7: 00 00 B1 G1 R1 B0 G0 R0 */\
                      psrlq_i2r(16, mm3);/*                 mm3: 00 00 00 00 G3 R3 G2 R2 */\
                      psrlq_i2r(8, mm0);/*                  mm0: 00 00 B7 B6 B5 B4 B3 B2 */\
                      movq_r2r(mm3, mm6);/*                 mm6: 00 00 00 00 G3 R3 G2 R2 */\
                      pand_m2r(yuv_rgb_lowest_word, mm6);/* mm6: 00 00 00 00 00 00 G2 R2 */\
                      psllq_i2r(48, mm6);/*                 mm6: G2 R2 00 00 00 00 00 00 */\
                      por_r2r(mm6, mm7);/*                  mm7: G2 R2 B1 G1 R1 B0 G0 R0 */\
                      MOVQ_R2M(mm7, *dst);\
                      movq_r2r(mm0, mm7);/*                 mm7: 00 00 B7 B6 B5 B4 B3 B2 */\
                      pand_m2r(yuv_rgb_lowest_byte, mm7);/* mm7: 00 00 00 00 00 00 00 B2 */\
                      psrlq_i2r(16, mm3);/*                 mm3: 00 00 00 00 00 00 G3 R3 */\
                      psllq_i2r(8, mm3);/*                  mm3: 00 00 00 00 00 G3 R3 00 */\
                      por_r2r(mm3, mm7);/*                  mm7: 00 00 00 00 00 G3 R3 B2 */\
                      psrlq_i2r(8, mm0);/*                  mm0: 00 00 00 B7 B6 B5 B4 B3 */\
                      movq_r2r(mm0, mm5);/*                 mm5: 00 00 00 B7 B6 B5 B4 B3 */\
                      pand_m2r(yuv_rgb_lowest_byte, mm5);/* mm5: 00 00 00 00 00 00 00 B3 */\
                      psllq_i2r(24, mm5);/*                 mm5: 00 00 00 00 B3 00 00 00 */\
                      por_r2r(mm5, mm7);/*                  mm7: 00 00 00 00 B3 G3 R3 B2 */\
                      psrlq_i2r(32, mm1);/*                 mm1: 00 00 00 00 R7 R6 R5 R4 */\
                      psrlq_i2r(32, mm2);/*                 mm2: 00 00 00 00 G7 G6 G5 G4 */\
                      psrlq_i2r(8, mm0);/*                  mm0: 00 00 00 00 B7 B6 B5 B4 */\
                      punpcklbw_r2r(mm0, mm2);/*            mm2: B7 G7 B6 G6 B5 G5 B4 G4 */\
                      movq_r2r(mm1, mm5);/*                 mm5: 00 00 00 00 R7 R6 R5 R4 */\
                      pand_m2r(yuv_rgb_lowest_byte, mm5);/* mm5: 00 00 00 00 00 00 00 R4 */\
                      psllq_i2r(32, mm5);/*                 mm5: 00 00 00 R4 00 00 00 00 */\
                      por_r2r(mm5, mm7);/*                  mm7: 00 00 00 R4 B3 G3 R3 B2 */\
                      movq_r2r(mm2, mm6);/*                 mm6: B7 G7 B6 G6 B5 G5 B4 G4 */\
                      pand_m2r(yuv_rgb_lowest_word, mm6);/* mm6: 00 00 00 00 00 00 B4 G4 */\
                      psllq_i2r(40, mm6);/*                 mm6: 00 B4 G4 00 00 00 00 00 */\
                      por_r2r(mm6, mm7);/*                  mm7: 00 B4 G4 R4 B3 G3 R3 B2 */\
                      psrlq_i2r(8, mm1);/*                  mm1: 00 00 00 00 00 R7 R6 R5 */\
                      psrlq_i2r(16, mm2);/*                 mm2: 00 00 B7 G7 B6 G6 B5 G5 */\
                      movq_r2r(mm1, mm5);/*                 mm5: 00 00 00 00 00 R7 R6 R5 */\
                      psllq_i2r(56, mm5);/*                 mm5: R5 00 00 00 00 00 00 00 */\
                      por_r2r(mm5, mm7);/*                  mm7: R5 B4 G4 R4 B3 G3 R3 B2 */\
                      MOVQ_R2M(mm7, *(dst+8));\
                      movq_r2r(mm2, mm7);/*                 mm7: 00 00 B7 G7 B6 G6 B5 G5 */\
                      pand_m2r(yuv_rgb_lowest_word, mm7);/* mm7: 00 00 00 00 00 00 B5 G5 */\
                      psrlq_i2r(8, mm1);/*                  mm1: 00 00 00 00 00 00 R7 R6 */\
                      psrlq_i2r(16, mm2);/*                 mm2: 00 00 00 00 B7 G7 B6 G6 */\
                      movq_r2r(mm1, mm5);/*                 mm5: 00 00 00 00 00 00 R7 R6 */\
                      pand_m2r(yuv_rgb_lowest_byte, mm5);/* mm5: 00 00 00 00 00 00 00 R6 */\
                      psllq_i2r(16, mm5);/*                 mm5: 00 00 00 00 00 R6 00 00 */\
                      por_r2r(mm5, mm7);/*                  mm7: 00 00 00 00 00 R6 B5 G5 */\
                      movq_r2r(mm2, mm6);/*                 mm6: 00 00 00 00 B7 G7 B6 G6 */\
                      pand_m2r(yuv_rgb_lowest_word, mm6);/* mm6: 00 00 00 00 00 00 B6 G6 */\
                      psllq_i2r(24, mm6);/*                 mm5: 00 00 00 B6 G6 00 00 00 */\
                      por_r2r(mm6, mm7);/*                  mm7: 00 00 00 B6 G6 R6 B5 G5 */\
                      psrlq_i2r(8, mm1);/*                  mm1: 00 00 00 00 00 00 00 R7 */\
                      psrlq_i2r(16, mm2);/*                 mm2: 00 00 00 00 00 00 B7 G7 */\
                      psllq_i2r(40, mm1);/*                 mm1: 00 00 R7 00 00 00 00 00 */\
                      psllq_i2r(48, mm2);/*                 mm2: B7 G7 00 00 00 00 00 00 */\
                      por_r2r(mm1, mm7);\
                      por_r2r(mm2, mm7);\
                      MOVQ_R2M(mm7, *(dst+16));\

#define OUTPUT_BGR_24 pxor_r2r(mm3, mm3);/*                 Zero -> mm3 */\
                      movq_r2r(mm2, mm4);/*                 mm4: G7 G6 G5 G4 G3 G2 G1 G0 */\
                      movq_r2r(mm0, mm5);/*                 mm5: R7 R6 R5 R4 R3 R2 R1 R0 */\
                      movq_r2r(mm0, mm5);/*                 mm5: R7 R6 R5 R4 R3 R2 R1 R0 */\
                      punpcklbw_r2r(mm3, mm5);/*            mm5: 00 R3 00 R2 00 R1 00 R0 */\
                      punpcklbw_r2r(mm4, mm3);/*            mm3: G3 00 G2 00 G1 00 G0 00 */\
                      por_r2r(mm5, mm3);/*                  mm3: G3 R3 G2 R2 G1 R1 G0 R0 */\
                      movq_r2r(mm1, mm5);/*                 mm5: B7 B6 B5 B4 B3 B2 B1 B0 */\
                      pand_m2r(yuv_rgb_lowest_byte, mm5);/* mm5: 00 00 00 00 00 00 00 B0 */\
                      movq_r2r(mm3, mm7);/*                 mm7: G3 R3 G2 R2 G1 R1 G0 R0 */\
                      pand_m2r(yuv_rgb_lowest_word, mm7);/* mm7: 00 00 00 00 00 00 G0 R0 */\
                      psllq_i2r(16, mm5);/*                 mm5: 00 00 00 00 00 B0 00 00 */\
                      por_r2r(mm5, mm7);/*                  mm7: 00 00 00 00 00 B0 G0 R0 */\
                      psrlq_i2r(16, mm3);/*                 mm3: 00 00 G3 R3 G2 R2 G1 R1 */\
                      psrlq_i2r(8, mm1);/*                  mm1: 00 B7 B6 B5 B4 B3 B2 B1 */\
                      movq_r2r(mm1, mm5);/*                 mm5: 00 B7 B6 B5 B4 B3 B2 B1 */\
                      pand_m2r(yuv_rgb_lowest_byte, mm5);/* mm5: 00 00 00 00 00 00 00 B1 */\
                      psllq_i2r(40, mm5);/*                 mm5: 00 00 B1 00 00 00 00 00 */\
                      por_r2r(mm5, mm7);/*                  mm7: 00 00 B1 00 00 B0 G0 R0 */\
                      movq_r2r(mm3, mm6);/*                 mm7: 00 00 G3 R3 G2 R2 G1 R1 */\
                      pand_m2r(yuv_rgb_lowest_word, mm6);/* mm6: 00 00 00 00 00 00 G1 R1 */\
                      psllq_i2r(24, mm6);/*                 mm6: 00 00 00 G1 R1 00 00 00 */\
                      por_r2r(mm6, mm7);/*                  mm7: 00 00 B1 G1 R1 B0 G0 R0 */\
                      psrlq_i2r(16, mm3);/*                 mm3: 00 00 00 00 G3 R3 G2 R2 */\
                      psrlq_i2r(8, mm1);/*                  mm1: 00 00 B7 B6 B5 B4 B3 B2 */\
                      movq_r2r(mm3, mm6);/*                 mm6: 00 00 00 00 G3 R3 G2 R2 */\
                      pand_m2r(yuv_rgb_lowest_word, mm6);/* mm6: 00 00 00 00 00 00 G2 R2 */\
                      psllq_i2r(48, mm6);/*                 mm6: G2 R2 00 00 00 00 00 00 */\
                      por_r2r(mm6, mm7);/*                  mm7: G2 R2 B1 G1 R1 B0 G0 R0 */\
                      MOVQ_R2M(mm7, *dst);\
                      movq_r2r(mm1, mm7);/*                 mm7: 00 00 B7 B6 B5 B4 B3 B2 */\
                      pand_m2r(yuv_rgb_lowest_byte, mm7);/* mm7: 00 00 00 00 00 00 00 B2 */\
                      psrlq_i2r(16, mm3);/*                 mm3: 00 00 00 00 00 00 G3 R3 */\
                      psllq_i2r(8, mm3);/*                  mm3: 00 00 00 00 00 G3 R3 00 */\
                      por_r2r(mm3, mm7);/*                  mm7: 00 00 00 00 00 G3 R3 B2 */\
                      psrlq_i2r(8, mm1);/*                  mm1: 00 00 00 B7 B6 B5 B4 B3 */\
                      movq_r2r(mm1, mm5);/*                 mm5: 00 00 00 B7 B6 B5 B4 B3 */\
                      pand_m2r(yuv_rgb_lowest_byte, mm5);/* mm5: 00 00 00 00 00 00 00 B3 */\
                      psllq_i2r(24, mm5);/*                 mm5: 00 00 00 00 B3 00 00 00 */\
                      por_r2r(mm5, mm7);/*                  mm7: 00 00 00 00 B3 G3 R3 B2 */\
                      psrlq_i2r(32, mm0);/*                 mm0: 00 00 00 00 R7 R6 R5 R4 */\
                      psrlq_i2r(32, mm2);/*                 mm2: 00 00 00 00 G7 G6 G5 G4 */\
                      psrlq_i2r(8, mm1);/*                  mm1: 00 00 00 00 B7 B6 B5 B4 */\
                      punpcklbw_r2r(mm1, mm2);/*            mm2: B7 G7 B6 G6 B5 G5 B4 G4 */\
                      movq_r2r(mm0, mm5);/*                 mm5: 00 00 00 00 R7 R6 R5 R4 */\
                      pand_m2r(yuv_rgb_lowest_byte, mm5);/* mm5: 00 00 00 00 00 00 00 R4 */\
                      psllq_i2r(32, mm5);/*                 mm5: 00 00 00 R4 00 00 00 00 */\
                      por_r2r(mm5, mm7);/*                  mm7: 00 00 00 R4 B3 G3 R3 B2 */\
                      movq_r2r(mm2, mm6);/*                 mm6: B7 G7 B6 G6 B5 G5 B4 G4 */\
                      pand_m2r(yuv_rgb_lowest_word, mm6);/* mm6: 00 00 00 00 00 00 B4 G4 */\
                      psllq_i2r(40, mm6);/*                 mm6: 00 B4 G4 00 00 00 00 00 */\
                      por_r2r(mm6, mm7);/*                  mm7: 00 B4 G4 R4 B3 G3 R3 B2 */\
                      psrlq_i2r(8, mm0);/*                  mm0: 00 00 00 00 00 R7 R6 R5 */\
                      psrlq_i2r(16, mm2);/*                 mm2: 00 00 B7 G7 B6 G6 B5 G5 */\
                      movq_r2r(mm0, mm5);/*                 mm5: 00 00 00 00 00 R7 R6 R5 */\
                      psllq_i2r(56, mm5);/*                 mm5: R5 00 00 00 00 00 00 00 */\
                      por_r2r(mm5, mm7);/*                  mm7: R5 B4 G4 R4 B3 G3 R3 B2 */\
                      MOVQ_R2M(mm7, *(dst+8));\
                      movq_r2r(mm2, mm7);/*                 mm7: 00 00 B7 G7 B6 G6 B5 G5 */\
                      pand_m2r(yuv_rgb_lowest_word, mm7);/* mm7: 00 00 00 00 00 00 B5 G5 */\
                      psrlq_i2r(8, mm0);/*                  mm0: 00 00 00 00 00 00 R7 R6 */\
                      psrlq_i2r(16, mm2);/*                 mm2: 00 00 00 00 B7 G7 B6 G6 */\
                      movq_r2r(mm0, mm5);/*                 mm5: 00 00 00 00 00 00 R7 R6 */\
                      pand_m2r(yuv_rgb_lowest_byte, mm5);/* mm5: 00 00 00 00 00 00 00 R6 */\
                      psllq_i2r(16, mm5);/*                 mm5: 00 00 00 00 00 R6 00 00 */\
                      por_r2r(mm5, mm7);/*                  mm7: 00 00 00 00 00 R6 B5 G5 */\
                      movq_r2r(mm2, mm6);/*                 mm6: 00 00 00 00 B7 G7 B6 G6 */\
                      pand_m2r(yuv_rgb_lowest_word, mm6);/* mm6: 00 00 00 00 00 00 B6 G6 */\
                      psllq_i2r(24, mm6);/*                 mm5: 00 00 00 B6 G6 00 00 00 */\
                      por_r2r(mm6, mm7);/*                  mm7: 00 00 00 B6 G6 R6 B5 G5 */\
                      psrlq_i2r(8, mm0);/*                  mm0: 00 00 00 00 00 00 00 R7 */\
                      psrlq_i2r(16, mm2);/*                 mm2: 00 00 00 00 00 00 B7 G7 */\
                      psllq_i2r(40, mm0);/*                 mm0: 00 00 R7 00 00 00 00 00 */\
                      psllq_i2r(48, mm2);/*                 mm2: B7 G7 00 00 00 00 00 00 */\
                      por_r2r(mm0, mm7);\
                      por_r2r(mm2, mm7);\
                      MOVQ_R2M(mm7, *(dst+16));\



#endif

static const mmx_t rgba32_alphamask = {0xff000000ff000000LL};

#define OUTPUT_RGBA_32 pxor_r2r (mm3, mm3);\
                       movq_r2r (mm1, mm6);\
                       punpcklbw_r2r (mm2, mm6);\
                       movq_r2r (mm0, mm7);\
                       punpcklbw_r2r (mm3, mm7);\
                       movq_r2r (mm1, mm4);\
                       punpcklwd_r2r (mm7, mm6);\
                       movq_r2r (mm0, mm5);\
                       por_m2r (rgba32_alphamask, mm6);\
                       MOVQ_R2M (mm6, *dst);\
                       movq_r2r (mm1, mm6);\
                       punpcklbw_r2r (mm2, mm6);\
                       punpckhwd_r2r (mm7, mm6);\
                       por_m2r (rgba32_alphamask, mm6);\
                       MOVQ_R2M (mm6, *(dst+8));\
                       punpckhbw_r2r (mm2, mm4);\
                       punpckhbw_r2r (mm3, mm5);\
                       punpcklwd_r2r (mm5, mm4);\
                       por_m2r (rgba32_alphamask, mm4);\
                       MOVQ_R2M (mm4, *(dst+16));\
                       movq_r2r (mm1, mm4);\
                       punpckhbw_r2r (mm2, mm4);\
                       punpckhwd_r2r (mm5, mm4);\
                       por_m2r (rgba32_alphamask, mm4);\
                       MOVQ_R2M (mm4, *(dst+24));

static const mmx_t rgb16_bluemask = {0xf8f8f8f8f8f8f8f8LL};
static const mmx_t rgb16_greenmask = {0xfcfcfcfcfcfcfcfcLL};
static const mmx_t rgb16_redmask = {0xf8f8f8f8f8f8f8f8LL};

#define OUTPUT_RGB_16 pand_m2r (rgb16_bluemask, mm0);/*  mm0 = b7b6b5b4b3______ */\
                      pxor_r2r (mm4, mm4);/*             mm4 = 0                */\
                      pand_m2r (rgb16_greenmask, mm2);/* mm2 = g7g6g5g4g3g2____ */\
                      psrlq_i2r (3, mm0);/*              mm0 = ______b7b6b5b4b3 */\
                      movq_r2r (mm2, mm7);/*             mm7 = g7g6g5g4g3g2____ */\
                      movq_r2r (mm0, mm5);/*             mm5 = ______b7b6b5b4b3 */\
                      pand_m2r (rgb16_redmask, mm1);/*   mm1 = r7r6r5r4r3______ */\
                      punpcklbw_r2r (mm4, mm2);\
                      punpcklbw_r2r (mm1, mm0);\
                      psllq_i2r (3, mm2);\
                      punpckhbw_r2r (mm4, mm7);\
                      por_r2r (mm2, mm0);\
                      psllq_i2r (3, mm7);\
                      MOVQ_R2M (mm0, *dst);\
                      punpckhbw_r2r (mm1, mm5);\
                      por_r2r (mm7, mm5);\
                      MOVQ_R2M (mm5, *(dst+8));

#define OUTPUT_BGR_16 pand_m2r (rgb16_bluemask, mm1);/*  mm0 = b7b6b5b4b3______ */\
                      pxor_r2r (mm4, mm4);/*             mm4 = 0                */\
                      pand_m2r (rgb16_greenmask, mm2);/* mm2 = g7g6g5g4g3g2____ */\
                      psrlq_i2r (3, mm1);/*              mm0 = ______b7b6b5b4b3 */\
                      movq_r2r (mm2, mm7);/*             mm7 = g7g6g5g4g3g2____ */\
                      movq_r2r (mm1, mm5);/*             mm5 = ______b7b6b5b4b3 */\
                      pand_m2r (rgb16_redmask, mm0);/*   mm1 = r7r6r5r4r3______ */\
                      punpcklbw_r2r (mm4, mm2);\
                      punpcklbw_r2r (mm0, mm1);\
                      psllq_i2r (3, mm2);\
                      punpckhbw_r2r (mm4, mm7);\
                      por_r2r (mm2, mm1);\
                      psllq_i2r (3, mm7);\
                      MOVQ_R2M (mm1, *dst);\
                      punpckhbw_r2r (mm0, mm5);\
                      por_r2r (mm7, mm5);\
                      MOVQ_R2M (mm5, *(dst+8));\


static const mmx_t rgb15_bluemask = {0xf8f8f8f8f8f8f8f8LL};
static const mmx_t rgb15_greenmask = {0xf8f8f8f8f8f8f8f8LL};
static const mmx_t rgb15_redmask = {0xf8f8f8f8f8f8f8f8LL};

#define OUTPUT_RGB_15 pand_m2r (rgb15_bluemask, mm0);/*  mm0 = b7b6b5b4b3______ */\
                      pxor_r2r (mm4, mm4);/*             mm4 = 0                */\
                      pand_m2r (rgb15_greenmask, mm2);/* mm2 = g7g6g5g4g3g2____ */\
                      psrlq_i2r (3, mm0);/*              mm0 = ______b7b6b5b4b3 */\
                      movq_r2r (mm2, mm7);/*             mm7 = g7g6g5g4g3g2____ */\
                      movq_r2r (mm0, mm5);/*             mm5 = ______b7b6b5b4b3 */\
                      pand_m2r (rgb15_redmask, mm1);/*   mm1 = r7r6r5r4r3______ */\
                      punpcklbw_r2r (mm4, mm2);\
                      psrlq_i2r (1, mm1);\
                      punpcklbw_r2r (mm1, mm0);\
                      psllq_i2r (2, mm2);\
                      punpckhbw_r2r (mm4, mm7);\
                      por_r2r (mm2, mm0);\
                      psllq_i2r (2, mm7);\
                      MOVQ_R2M(mm0, *dst);\
                      punpckhbw_r2r (mm1, mm5);\
                      por_r2r (mm7, mm5);\
                      MOVQ_R2M(mm5, *(dst+8));

#define OUTPUT_BGR_15 pand_m2r (rgb15_bluemask, mm1);/*  mm0 = b7b6b5b4b3______ */\
                      pxor_r2r (mm4, mm4);/*             mm4 = 0                */\
                      pand_m2r (rgb15_greenmask, mm2);/* mm2 = g7g6g5g4g3g2____ */\
                      psrlq_i2r (3, mm1);/*              mm0 = ______b7b6b5b4b3 */\
                      movq_r2r (mm2, mm7);/*             mm7 = g7g6g5g4g3g2____ */\
                      movq_r2r (mm1, mm5);/*             mm5 = ______b7b6b5b4b3 */\
                      pand_m2r (rgb15_redmask, mm0);/*   mm1 = r7r6r5r4r3______ */\
                      punpcklbw_r2r (mm4, mm2);\
                      psrlq_i2r (1, mm0);\
                      punpcklbw_r2r (mm0, mm1);\
                      psllq_i2r (2, mm2);\
                      punpckhbw_r2r (mm4, mm7);\
                      por_r2r (mm2, mm1);\
                      psllq_i2r (2, mm7);\
                      MOVQ_R2M(mm1, *dst);\
                      punpckhbw_r2r (mm0, mm5);\
                      por_r2r (mm7, mm5);\
                      MOVQ_R2M(mm5, *(dst+8));


/***********************************************************
 * YUY2 ->
 ***********************************************************/

#ifndef MMXEXT

#define FUNC_NAME   yuy2_to_rgb_15_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_YUY2 \
  YUV_2_RGB \
  OUTPUT_RGB_15

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   yuy2_to_bgr_15_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_YUY2 \
  YUV_2_RGB \
  OUTPUT_BGR_15

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   yuy2_to_rgb_16_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_YUY2 \
  YUV_2_RGB \
  OUTPUT_RGB_16

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"


#define FUNC_NAME   yuy2_to_bgr_16_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_YUY2 \
  YUV_2_RGB \
  OUTPUT_BGR_16

#define CLEANUP emms();


#include "../csp_packed_packed.h"

#endif // !MMXEXT

#define FUNC_NAME   yuy2_to_rgb_24_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 24
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_YUY2 \
  YUV_2_RGB \
  OUTPUT_RGB_24

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   yuy2_to_bgr_24_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 24
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_YUY2 \
  YUV_2_RGB \
  OUTPUT_BGR_24

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#ifndef MMXEXT


#define FUNC_NAME   yuy2_to_rgb_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_YUY2 \
  YUV_2_RGB \
  OUTPUT_RGB_32

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   yuy2_to_bgr_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_YUY2 \
  YUV_2_RGB \
  OUTPUT_BGR_32

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   yuy2_to_rgba_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_YUY2 \
  YUV_2_RGB \
  OUTPUT_RGBA_32

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#endif // !MMXEXT


/***********************************************************
 * UYVY ->
 ***********************************************************/

#ifndef MMXEXT

#define FUNC_NAME   uyvy_to_rgb_15_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_UYVY \
  YUV_2_RGB \
  OUTPUT_RGB_15

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   uyvy_to_bgr_15_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_UYVY \
  YUV_2_RGB \
  OUTPUT_BGR_15

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   uyvy_to_rgb_16_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_UYVY \
  YUV_2_RGB \
  OUTPUT_RGB_16

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"


#define FUNC_NAME   uyvy_to_bgr_16_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 16
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_UYVY \
  YUV_2_RGB \
  OUTPUT_BGR_16

#define CLEANUP emms();


// #define INIT

#include "../csp_packed_packed.h"

#endif // !MMXEXT

#define FUNC_NAME   uyvy_to_rgb_24_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 24
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_UYVY \
  YUV_2_RGB \
  OUTPUT_RGB_24

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   uyvy_to_bgr_24_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 24
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_UYVY \
  YUV_2_RGB \
  OUTPUT_BGR_24

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#ifndef MMXEXT

#define FUNC_NAME   uyvy_to_rgb_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_UYVY \
  YUV_2_RGB \
  OUTPUT_RGB_32

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   uyvy_to_bgr_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_UYVY \
  YUV_2_RGB \
  OUTPUT_BGR_32

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   uyvy_to_rgba_32_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 32
#define NUM_PIXELS  8
#define CONVERT     \
  LOAD_UYVY \
  YUV_2_RGB \
  OUTPUT_RGBA_32

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#endif // !MMXEXT


/***************************************************
 * YUV 420 P ->
 ***************************************************/

#ifndef MMXEXT

#define FUNC_NAME     yuv_420_p_to_rgb_15_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_RGB_15

#define CHROMA_SUB 2

// #define INIT
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_420_p_to_bgr_15_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_BGR_15

#define CHROMA_SUB 2

// #define INIT
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_420_p_to_rgb_16_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_RGB_16

#define CHROMA_SUB 2

// #define INIT
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_420_p_to_bgr_16_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_BGR_16

#define CHROMA_SUB 2

// #define INIT
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#endif // !MMXEXT

#define FUNC_NAME     yuv_420_p_to_rgb_24_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   24
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_RGB_24

#define CHROMA_SUB 2

// #define INIT
#define CLEANUP emms();

#include "../csp_planar_packed.h"


#define FUNC_NAME     yuv_420_p_to_bgr_24_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   24
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_BGR_24

#define CHROMA_SUB 2

// #define INIT
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#ifndef MMXEXT

#define FUNC_NAME     yuv_420_p_to_rgb_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_RGB_32

#define CHROMA_SUB 2

// #define INIT
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_420_p_to_bgr_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_BGR_32

#define CHROMA_SUB 2

// #define INIT
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_420_p_to_rgba_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_RGBA_32

#define CHROMA_SUB 2

// #define INIT
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#endif // !MMXEXT

/***************************************************
 * YUV 410 P ->
 ***************************************************/

#ifndef MMXEXT

#define FUNC_NAME     yuv_410_p_to_rgb_15_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_RGB_15

#define CHROMA_SUB 4

#define INIT INIT_YUV_PLANAR_4
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_410_p_to_bgr_15_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_BGR_15

#define CHROMA_SUB 4

#define INIT INIT_YUV_PLANAR_4
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_410_p_to_rgb_16_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_RGB_16

#define CHROMA_SUB 4

#define INIT INIT_YUV_PLANAR_4
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_410_p_to_bgr_16_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_BGR_16

#define CHROMA_SUB 4

#define INIT INIT_YUV_PLANAR_4
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#endif // !MMXEXT

#define FUNC_NAME     yuv_410_p_to_rgb_24_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   24
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_RGB_24

#define CHROMA_SUB 4

#define INIT INIT_YUV_PLANAR_4
#define CLEANUP emms();

#include "../csp_planar_packed.h"


#define FUNC_NAME     yuv_410_p_to_bgr_24_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   24
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_BGR_24

#define CHROMA_SUB 4

#define INIT INIT_YUV_PLANAR_4
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#ifndef MMXEXT

#define FUNC_NAME     yuv_410_p_to_rgb_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_RGB_32

#define CHROMA_SUB 4

#define INIT INIT_YUV_PLANAR_4
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_410_p_to_bgr_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_BGR_32

#define CHROMA_SUB 4

#define INIT INIT_YUV_PLANAR_4
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_410_p_to_rgba_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_RGBA_32

#define CHROMA_SUB 4

#define INIT INIT_YUV_PLANAR_4
#define CLEANUP emms();

#include "../csp_planar_packed.h"

#endif // !MMXEXT

/********************************************************
 * YUV 422 ->
 ********************************************************/

#ifndef MMXEXT

#define FUNC_NAME     yuv_422_p_to_rgb_15_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_RGB_15

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_422_p_to_bgr_15_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_BGR_15

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_422_p_to_rgb_16_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_RGB_16

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_422_p_to_bgr_16_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_BGR_16

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#endif // MMXEXT

#define FUNC_NAME     yuv_422_p_to_rgb_24_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   24
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_RGB_24

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"



#define FUNC_NAME     yuv_422_p_to_bgr_24_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   24
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_BGR_24

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#ifndef MMXEXT


#define FUNC_NAME     yuv_422_p_to_rgb_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_RGB_32

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_422_p_to_bgr_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_BGR_32

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_422_p_to_rgba_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUV_2_RGB \
  OUTPUT_RGBA_32

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#endif // !MMXEXT

/********************************************************
 * YUV 411 ->
 ********************************************************/

#ifndef MMXEXT

#define FUNC_NAME     yuv_411_p_to_rgb_15_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_RGB_15

#define CHROMA_SUB 1
#define INIT INIT_YUV_PLANAR_4

#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_411_p_to_bgr_15_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_BGR_15

#define CHROMA_SUB 1
#define INIT INIT_YUV_PLANAR_4


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_411_p_to_rgb_16_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_RGB_16

#define CHROMA_SUB 1
#define INIT INIT_YUV_PLANAR_4


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_411_p_to_bgr_16_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_BGR_16

#define CHROMA_SUB 1
#define INIT INIT_YUV_PLANAR_4


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#endif // MMXEXT

#define FUNC_NAME     yuv_411_p_to_rgb_24_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   24
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_RGB_24

#define CHROMA_SUB 1
#define INIT INIT_YUV_PLANAR_4


#define CLEANUP emms();

#include "../csp_planar_packed.h"


#define FUNC_NAME     yuv_411_p_to_bgr_24_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   24
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_BGR_24

#define CHROMA_SUB 1
#define INIT INIT_YUV_PLANAR_4


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#ifndef MMXEXT

#define FUNC_NAME     yuv_411_p_to_rgb_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_RGB_32

#define CHROMA_SUB 1
#define INIT INIT_YUV_PLANAR_4


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_411_p_to_bgr_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_BGR_32

#define CHROMA_SUB 1
#define INIT INIT_YUV_PLANAR_4


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuv_411_p_to_rgba_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 2
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_4 \
  YUV_2_RGB \
  OUTPUT_RGBA_32

#define CHROMA_SUB 1
#define INIT INIT_YUV_PLANAR_4


#define CLEANUP emms();

#include "../csp_planar_packed.h"


#endif // !MMXEXT

/* JPEG */

/***************************************************
 * YUVJ 420 P ->
 ***************************************************/

#ifndef MMXEXT

#define FUNC_NAME     yuvj_420_p_to_rgb_15_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_RGB_15

#define CHROMA_SUB 2


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuvj_420_p_to_bgr_15_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_BGR_15

#define CHROMA_SUB 2


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuvj_420_p_to_rgb_16_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_RGB_16

#define CHROMA_SUB 2


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuvj_420_p_to_bgr_16_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_BGR_16

#define CHROMA_SUB 2


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#endif // !MMXEXT

#define FUNC_NAME     yuvj_420_p_to_rgb_24_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   24
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_RGB_24

#define CHROMA_SUB 2


#define CLEANUP emms();

#include "../csp_planar_packed.h"


#define FUNC_NAME     yuvj_420_p_to_bgr_24_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   24
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_BGR_24

#define CHROMA_SUB 2


#define CLEANUP emms();

#ifndef MMXEXT

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuvj_420_p_to_rgb_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_RGB_32

#define CHROMA_SUB 2


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuvj_420_p_to_bgr_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_BGR_32

#define CHROMA_SUB 2


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuvj_420_p_to_rgba_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_RGBA_32

#define CHROMA_SUB 2


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#endif // !MMXEXT


/********************************************************
 * YUVJ 422 ->
 ********************************************************/

#ifndef MMXEXT

#define FUNC_NAME     yuvj_422_p_to_rgb_15_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_RGB_15

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuvj_422_p_to_bgr_15_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_BGR_15

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuvj_422_p_to_rgb_16_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_RGB_16

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuvj_422_p_to_bgr_16_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   16
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_BGR_16

#define CHROMA_SUB 1


#define CLEANUP emms();

#endif // !MMXEXT


#include "../csp_planar_packed.h"

#define FUNC_NAME     yuvj_422_p_to_rgb_24_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   24
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_RGB_24

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"


#define FUNC_NAME     yuvj_422_p_to_bgr_24_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   24
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_BGR_24

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#ifndef MMXEXT

#define FUNC_NAME     yuvj_422_p_to_rgb_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_RGB_32

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuvj_422_p_to_bgr_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_BGR_32

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#define FUNC_NAME     yuvj_422_p_to_rgba_32_mmx
#define IN_TYPE       uint8_t
#define OUT_TYPE      uint8_t
#define IN_ADVANCE_Y  8
#define IN_ADVANCE_UV 4
#define OUT_ADVANCE   32
#define NUM_PIXELS    8
#define CONVERT       \
  LOAD_YUV_PLANAR_2 \
  YUVJ_2_RGB \
  OUTPUT_RGBA_32

#define CHROMA_SUB 1


#define CLEANUP emms();

#include "../csp_planar_packed.h"

#endif // !MMXEXT

#ifdef MMXEXT

void gavl_init_yuv_rgb_funcs_mmxext(gavl_pixelformat_function_table_t * tab,
                                    int width, const gavl_video_options_t * opt)
     
#else /* !MMXEXT */

void gavl_init_yuv_rgb_funcs_mmx(gavl_pixelformat_function_table_t * tab,
                                 int width, const gavl_video_options_t * opt)

#endif /* MMXEXT */
     
  {
  if(width % 8)
    return;

  if(opt->quality && (opt->quality >= 3))
    return;
  
  tab->yuy2_to_rgb_24 = yuy2_to_rgb_24_mmx;
  tab->yuy2_to_bgr_24 = yuy2_to_bgr_24_mmx;
  tab->uyvy_to_rgb_24 = uyvy_to_rgb_24_mmx;
  tab->uyvy_to_bgr_24 = uyvy_to_bgr_24_mmx;
  tab->yuv_420_p_to_rgb_24 = yuv_420_p_to_rgb_24_mmx;
  tab->yuv_420_p_to_bgr_24 = yuv_420_p_to_bgr_24_mmx;
  tab->yuv_410_p_to_rgb_24 = yuv_410_p_to_rgb_24_mmx;
  tab->yuv_410_p_to_bgr_24 = yuv_410_p_to_bgr_24_mmx;
  tab->yuv_422_p_to_rgb_24 = yuv_422_p_to_rgb_24_mmx;
  tab->yuv_422_p_to_bgr_24 = yuv_422_p_to_bgr_24_mmx;
  tab->yuv_411_p_to_rgb_24 = yuv_411_p_to_rgb_24_mmx;
  tab->yuv_411_p_to_bgr_24 = yuv_411_p_to_bgr_24_mmx;
  tab->yuvj_420_p_to_rgb_24 = yuvj_420_p_to_rgb_24_mmx;
  tab->yuvj_420_p_to_bgr_24 = yuvj_420_p_to_bgr_24_mmx;
  tab->yuvj_422_p_to_rgb_24 = yuvj_422_p_to_rgb_24_mmx;
  tab->yuvj_422_p_to_bgr_24 = yuvj_422_p_to_bgr_24_mmx;

#ifndef MMXEXT
  
  tab->yuy2_to_rgb_15 = yuy2_to_rgb_15_mmx;
  tab->yuy2_to_bgr_15 = yuy2_to_bgr_15_mmx;
  tab->yuy2_to_rgb_16 = yuy2_to_rgb_16_mmx;
  tab->yuy2_to_bgr_16 = yuy2_to_bgr_16_mmx;
  tab->yuy2_to_rgb_32 = yuy2_to_rgb_32_mmx;
  tab->yuy2_to_bgr_32 = yuy2_to_bgr_32_mmx;
  tab->yuy2_to_rgba_32 = yuy2_to_rgba_32_mmx;

  tab->uyvy_to_rgb_15 = uyvy_to_rgb_15_mmx;
  tab->uyvy_to_bgr_15 = uyvy_to_bgr_15_mmx;
  tab->uyvy_to_rgb_16 = uyvy_to_rgb_16_mmx;
  tab->uyvy_to_bgr_16 = uyvy_to_bgr_16_mmx;
  tab->uyvy_to_rgb_32 = uyvy_to_rgb_32_mmx;
  tab->uyvy_to_bgr_32 = uyvy_to_bgr_32_mmx;
  tab->uyvy_to_rgba_32 = uyvy_to_rgba_32_mmx;
  
  tab->yuv_420_p_to_rgb_15 = yuv_420_p_to_rgb_15_mmx;
  tab->yuv_420_p_to_bgr_15 = yuv_420_p_to_bgr_15_mmx;
  tab->yuv_420_p_to_rgb_16 = yuv_420_p_to_rgb_16_mmx;
  tab->yuv_420_p_to_bgr_16 = yuv_420_p_to_bgr_16_mmx;
  tab->yuv_420_p_to_rgb_32 = yuv_420_p_to_rgb_32_mmx;
  tab->yuv_420_p_to_bgr_32 = yuv_420_p_to_bgr_32_mmx;
  tab->yuv_420_p_to_rgba_32 = yuv_420_p_to_rgba_32_mmx;

  tab->yuv_410_p_to_rgb_15 = yuv_410_p_to_rgb_15_mmx;
  tab->yuv_410_p_to_bgr_15 = yuv_410_p_to_bgr_15_mmx;
  tab->yuv_410_p_to_rgb_16 = yuv_410_p_to_rgb_16_mmx;
  tab->yuv_410_p_to_bgr_16 = yuv_410_p_to_bgr_16_mmx;
  tab->yuv_410_p_to_rgb_32 = yuv_410_p_to_rgb_32_mmx;
  tab->yuv_410_p_to_bgr_32 = yuv_410_p_to_bgr_32_mmx;
  tab->yuv_410_p_to_rgba_32 = yuv_410_p_to_rgba_32_mmx;

  
  tab->yuv_422_p_to_rgb_15 = yuv_422_p_to_rgb_15_mmx;
  tab->yuv_422_p_to_bgr_15 = yuv_422_p_to_bgr_15_mmx;
  tab->yuv_422_p_to_rgb_16 = yuv_422_p_to_rgb_16_mmx;
  tab->yuv_422_p_to_bgr_16 = yuv_422_p_to_bgr_16_mmx;
  tab->yuv_422_p_to_rgb_32 = yuv_422_p_to_rgb_32_mmx;
  tab->yuv_422_p_to_bgr_32 = yuv_422_p_to_bgr_32_mmx;
  tab->yuv_422_p_to_rgba_32 = yuv_422_p_to_rgba_32_mmx;

  tab->yuv_411_p_to_rgb_15 = yuv_411_p_to_rgb_15_mmx;
  tab->yuv_411_p_to_bgr_15 = yuv_411_p_to_bgr_15_mmx;
  tab->yuv_411_p_to_rgb_16 = yuv_411_p_to_rgb_16_mmx;
  tab->yuv_411_p_to_bgr_16 = yuv_411_p_to_bgr_16_mmx;
  tab->yuv_411_p_to_rgb_32 = yuv_411_p_to_rgb_32_mmx;
  tab->yuv_411_p_to_bgr_32 = yuv_411_p_to_bgr_32_mmx;
  tab->yuv_411_p_to_rgba_32 = yuv_411_p_to_rgba_32_mmx;

  
  tab->yuvj_420_p_to_rgb_15 = yuvj_420_p_to_rgb_15_mmx;
  tab->yuvj_420_p_to_bgr_15 = yuvj_420_p_to_bgr_15_mmx;
  tab->yuvj_420_p_to_rgb_16 = yuvj_420_p_to_rgb_16_mmx;
  tab->yuvj_420_p_to_bgr_16 = yuvj_420_p_to_bgr_16_mmx;
  tab->yuvj_420_p_to_rgb_32 = yuvj_420_p_to_rgb_32_mmx;
  tab->yuvj_420_p_to_bgr_32 = yuvj_420_p_to_bgr_32_mmx;
  tab->yuvj_420_p_to_rgba_32 = yuvj_420_p_to_rgba_32_mmx;

  tab->yuvj_422_p_to_rgb_15 = yuvj_422_p_to_rgb_15_mmx;
  tab->yuvj_422_p_to_bgr_15 = yuvj_422_p_to_bgr_15_mmx;
  tab->yuvj_422_p_to_rgb_16 = yuvj_422_p_to_rgb_16_mmx;
  tab->yuvj_422_p_to_bgr_16 = yuvj_422_p_to_bgr_16_mmx;
  tab->yuvj_422_p_to_rgb_32 = yuvj_422_p_to_rgb_32_mmx;
  tab->yuvj_422_p_to_bgr_32 = yuvj_422_p_to_bgr_32_mmx;
  tab->yuvj_422_p_to_rgba_32 = yuvj_422_p_to_rgba_32_mmx;

#endif
  }
