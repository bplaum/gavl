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

#define RECLIP_32_TO_8(color) (uint8_t)((color & ~0xFF)?((-color) >> 31) : color) 
// #define RECLIP_32_TO_8(color) color>255?255:((color<0)?0:color)

#define RECLIP_32_TO_16(color) (uint16_t)((color & ~0xFFFF)?((-color) >> 31) : color) 

#define RECLIP_64_TO_8(color) (uint8_t)((color & ~0xFF)?((-color) >> 63) : color) 
#define RECLIP_64_TO_16(color) (uint16_t)((color & ~0xFFFF)?((-color) >> 63) : color) 

// #define RECLIP_16(color) (uint16_t)((color & ~0xFFFF)?0xFFFF:((color<0)?0:color))

#define RECLIP_FLOAT(color) ((color>1.0)?1.0:((color<0.0)?0.0:color))

#define RECLIP(color, min, max) ((color>max)?max:((color<min)?min:color))

/* Masks for BGR16 and RGB16 formats */

#define RGB16_LOWER_MASK  0x001f
#define RGB16_MIDDLE_MASK 0x07e0
#define RGB16_UPPER_MASK  0xf800

/* Extract 8 bit RGB values from 16 bit pixels */

#define RGB16_TO_R_8(pixel) gavl_rgb_5_to_8[(pixel & RGB16_UPPER_MASK)>>11]
#define RGB16_TO_G_8(pixel) gavl_rgb_6_to_8[(pixel & RGB16_MIDDLE_MASK)>>5]
#define RGB16_TO_B_8(pixel) gavl_rgb_5_to_8[(pixel & RGB16_LOWER_MASK)]

#define BGR16_TO_B_8(pixel) RGB16_TO_R_8(pixel)
#define BGR16_TO_G_8(pixel) RGB16_TO_G_8(pixel)
#define BGR16_TO_R_8(pixel) RGB16_TO_B_8(pixel)

/* Extract 16 bit RGB values from 16 bit pixels */

#define RGB16_TO_R_16(pixel) gavl_rgb_5_to_16[(pixel & RGB16_UPPER_MASK)>>11]
#define RGB16_TO_G_16(pixel) gavl_rgb_6_to_16[(pixel & RGB16_MIDDLE_MASK)>>5]
#define RGB16_TO_B_16(pixel) gavl_rgb_5_to_16[(pixel & RGB16_LOWER_MASK)]

#define BGR16_TO_B_16(pixel) RGB16_TO_R_16(pixel)
#define BGR16_TO_G_16(pixel) RGB16_TO_G_16(pixel)
#define BGR16_TO_R_16(pixel) RGB16_TO_B_16(pixel)

/* Extract float RGB values from 16 bit pixels */

#define RGB16_TO_R_FLOAT(pixel) gavl_rgb_5_to_float[(pixel & RGB16_UPPER_MASK)>>11]
#define RGB16_TO_G_FLOAT(pixel) gavl_rgb_6_to_float[(pixel & RGB16_MIDDLE_MASK)>>5]
#define RGB16_TO_B_FLOAT(pixel) gavl_rgb_5_to_float[(pixel & RGB16_LOWER_MASK)]

#define BGR16_TO_B_FLOAT(pixel) RGB16_TO_R_FLOAT(pixel)
#define BGR16_TO_G_FLOAT(pixel) RGB16_TO_G_FLOAT(pixel)
#define BGR16_TO_R_FLOAT(pixel) RGB16_TO_B_FLOAT(pixel)

/* Masks for BGR16 and RGB16 formats */

#define RGB15_LOWER_MASK  0x001f
#define RGB15_MIDDLE_MASK 0x03e0
#define RGB15_UPPER_MASK  0x7C00

/* Extract 8 bit RGB values from 16 bit pixels */

#define RGB15_TO_R_8(pixel) gavl_rgb_5_to_8[(pixel & RGB15_UPPER_MASK)>>10]
#define RGB15_TO_G_8(pixel) gavl_rgb_5_to_8[(pixel & RGB15_MIDDLE_MASK)>>5] 
#define RGB15_TO_B_8(pixel) gavl_rgb_5_to_8[(pixel & RGB15_LOWER_MASK)]

#define BGR15_TO_B_8(pixel) RGB15_TO_R_8(pixel)
#define BGR15_TO_G_8(pixel) RGB15_TO_G_8(pixel) 
#define BGR15_TO_R_8(pixel) RGB15_TO_B_8(pixel)

#define RGB15_TO_R_16(pixel) gavl_rgb_5_to_16[(pixel & RGB15_UPPER_MASK)>>10]
#define RGB15_TO_G_16(pixel) gavl_rgb_5_to_16[(pixel & RGB15_MIDDLE_MASK)>>5] 
#define RGB15_TO_B_16(pixel) gavl_rgb_5_to_16[(pixel & RGB15_LOWER_MASK)]

#define BGR15_TO_B_16(pixel) RGB15_TO_R_16(pixel)
#define BGR15_TO_G_16(pixel) RGB15_TO_G_16(pixel) 
#define BGR15_TO_R_16(pixel) RGB15_TO_B_16(pixel)

#define RGB15_TO_R_FLOAT(pixel) gavl_rgb_5_to_float[(pixel & RGB15_UPPER_MASK)>>10]
#define RGB15_TO_G_FLOAT(pixel) gavl_rgb_5_to_float[(pixel & RGB15_MIDDLE_MASK)>>5] 
#define RGB15_TO_B_FLOAT(pixel) gavl_rgb_5_to_float[(pixel & RGB15_LOWER_MASK)]

#define BGR15_TO_B_FLOAT(pixel) RGB15_TO_R_FLOAT(pixel)
#define BGR15_TO_G_FLOAT(pixel) RGB15_TO_G_FLOAT(pixel) 
#define BGR15_TO_R_FLOAT(pixel) RGB15_TO_B_FLOAT(pixel)

/* Conversion from 8 bit */

#define RGB_8_TO_16(val) (((uint32_t)(val))<<8)|(val)
#define RGB_8_TO_FLOAT(val) ((float)(val)/255.0)

/* Conversion from float */

#ifdef DO_ROUND
#define RGB_FLOAT_TO_8(val, dst)  dst=(uint8_t)((val)*255.0+0.5)
#define RGB_FLOAT_TO_16(val, dst) dst=(uint16_t)((val)*65535.0+0.5)
#else
#define RGB_FLOAT_TO_8(val, dst)  dst=(uint8_t)((val)*255.0)
#define RGB_FLOAT_TO_16(val, dst) dst=(uint16_t)((val)*65535.0)
#endif

/* Conversion from 16 bit */

#ifdef DO_ROUND
#define RGB_16_TO_8(val,dst) \
  round_tmp=(((int32_t)val+0x80)>>8);\
  dst=RECLIP_32_TO_8(round_tmp);
#else
#define RGB_16_TO_8(val,dst) dst=((val)>>8)
#endif

#define RGB_16_TO_FLOAT(val) ((float)(val)/65535.0)

/* Conversion from YUV float */

#ifdef DO_ROUND
#define Y_FLOAT_TO_8(val,dst)  dst=(int)(val * 219.0+0.5) + 16;
#define UV_FLOAT_TO_8(val,dst) dst=(int)(val * 224.0+0.5) + 128;
#else
#define Y_FLOAT_TO_8(val,dst)  dst=(int)(val * 219.0) + 16;
#define UV_FLOAT_TO_8(val,dst) dst=(int)(val * 224.0) + 128;
#endif

#ifdef DO_ROUND
#define Y_FLOAT_TO_16(val,dst)  dst=(int)(val * 219.0 * (float)0x100+0.5) + 0x1000;
#define UV_FLOAT_TO_16(val,dst) dst=(int)(val * 224.0 * (float)0x100+0.5) + 0x8000;
#else
#define Y_FLOAT_TO_16(val,dst)  dst=(int)(val * 219.0 * (float)0x100) + 0x1000;
#define UV_FLOAT_TO_16(val,dst) dst=(int)(val * 224.0 * (float)0x100) + 0x8000;
#endif

#ifdef DO_ROUND
#define YJ_FLOAT_TO_8(val,dst)  dst=(int)(val * 255.0+0.5);
#define UVJ_FLOAT_TO_8(val,dst) dst=(int)(val * 255.0+0.5) + 128;
#else
#define YJ_FLOAT_TO_8(val,dst)  dst=(int)(val * 255.0);
#define UVJ_FLOAT_TO_8(val,dst) dst=(int)(val * 255.0) + 128;
#endif

/* Conversion from YUV 16 */

#ifdef DO_ROUND
#define Y_16_TO_Y_8(val,dst) dst=((val+0x80)>>8)
#define UV_16_TO_UV_8(val,dst) dst=((val+0x80)>>8)
#define YJ_16_TO_Y_8(val,dst)  dst=(((int)val * 219+0x8000)>>16) + 16;
#define YJ_16_TO_Y_16(val,dst)  dst=(((int)val * 219+0x80)>>8) + 0x1000;
#define UVJ_16_TO_UV_16(val,dst)  dst=(((int)val * 224+0x80)>>8) + 0x1000;

#else
#define Y_16_TO_Y_8(val,dst) dst=((val)>>8)
#define UV_16_TO_UV_8(val,dst) dst=((val)>>8)
#define YJ_16_TO_Y_8(val,dst)  dst=(((int)val * 219)>>16) + 16;
#define YJ_16_TO_Y_16(val,dst)  dst=(((int)val * 219)>>8) + 0x1000;
#define UVJ_16_TO_UV_16(val,dst)  dst=(((int)val * 224)>>8) + 0x1000;
#endif


#ifdef DO_ROUND
#define Y_16_TO_YJ_8(val,dst)   dst=((((RECLIP(val, 0x1000, 0xEB00)-0x1000)*255)/219+0x80)>>8)
#define UV_16_TO_UVJ_8(val,dst) dst=((((RECLIP(val, 0x1000, 0xF000)-0x1000)*255)/224+0x80)>>8)

#define Y_16_TO_YJ_16(val,dst)   dst=(((uint32_t)((RECLIP((uint32_t)val, 0x1000, 0xEB00)-0x1000)*65535)/219+0x80)>>8)
#define UV_16_TO_UVJ_16(val,dst) dst=(((uint32_t)((RECLIP((uint32_t)val, 0x1000, 0xF000)-0x1000)*65535)/224+0x80)>>8)

#else
#define Y_16_TO_YJ_8(val,dst)   dst=((((RECLIP(val, 0x1000, 0xEB00)-0x1000)*255)/219)>>8)
#define UV_16_TO_UVJ_8(val,dst) dst=((((RECLIP(val, 0x1000, 0xF000)-0x1000)*255)/224)>>8)

#define Y_16_TO_YJ_16(val,dst)   dst=((((uint32_t)(RECLIP(val, 0x1000, 0xEB00)-0x1000)*65535)/219)>>8)
#define UV_16_TO_UVJ_16(val,dst) dst=((((uint32_t)(RECLIP(val, 0x1000, 0xF000)-0x1000)*65535)/224)>>8)

#endif

#define Y_16_TO_Y_FLOAT(val) (float)(RECLIP(val, 0x1000, 0xEB00)-0x1000) / (219.0*256.0)
#define UV_16_TO_UV_FLOAT(val) (float)(RECLIP(val, 0x1000, 0xF000)-0x1000) / (256.0 * 224.0) - 0.5


/* Conversion from YUV 8 */

#define Y_8_TO_16(val) ((val)<<8)
#define Y_8_TO_FLOAT(val) gavl_y_8_to_y_float[val]

#define UV_8_TO_16(val) ((val)<<8)
#define UV_8_TO_FLOAT(val) gavl_uv_8_to_uv_float[val]

#define Y_8_TO_YJ_8(val)   gavl_y_8_to_yj_8[val]
#define UV_8_TO_UVJ_8(val) gavl_uv_8_to_uvj_8[val]

#define YJ_8_TO_Y_8(val)   gavl_yj_8_to_y_8[val]
#define UVJ_8_TO_UV_8(val) gavl_uvj_8_to_uv_8[val]

#define YJ_8_TO_Y_16(val)   gavl_yj_8_to_y_16[val]
#define UVJ_8_TO_UV_16(val) gavl_uvj_8_to_uv_16[val]

#define YJ_8_TO_Y_FLOAT(val)   RGB_8_TO_FLOAT(val)
#define UVJ_8_TO_UV_FLOAT(val) RGB_8_TO_FLOAT(val) - 0.5;

#define Y_8_TO_YJ_16(val)   gavl_y_8_to_yj_16[val]

/* RGB -> YUV conversion */

/* 24 -> 8 */

#define RGB_24_TO_Y_8(r,g,b,y) y=(gavl_r_to_y[r]+gavl_g_to_y[g]+gavl_b_to_y[b])>>16;

#define RGB_24_TO_YUV_8(r,g,b,y,u,v)                                    \
  RGB_24_TO_Y_8(r,g,b,y);                                               \
  u=(gavl_r_to_u[r]+gavl_g_to_u[g]+gavl_b_to_u[b])>>16;                                \
  v=(gavl_r_to_v[r]+gavl_g_to_v[g]+gavl_b_to_v[b])>>16;

#define RGB_24_TO_YJ_8(r,g,b,y) y=(gavl_r_to_yj[r]+gavl_g_to_yj[g]+gavl_b_to_yj[b])>>16;

#define RGB_24_TO_YUVJ_8(r,g,b,y,u,v) \
  RGB_24_TO_YJ_8(r,g,b,y);                                              \
  u=(gavl_r_to_uj[r]+gavl_g_to_uj[g]+gavl_b_to_uj[b])>>16;                             \
  v=(gavl_r_to_vj[r]+gavl_g_to_vj[g]+gavl_b_to_vj[b])>>16;

/* 24 -> float */

#define RGB_24_TO_Y_FLOAT(r,g,b,y) y=(gavl_r_to_y_float[r]+gavl_g_to_y_float[g]+gavl_b_to_y_float[b]);

#define RGB_24_TO_YUV_FLOAT(r,g,b,y,u,v)                                    \
  RGB_24_TO_Y_FLOAT(r,g,b,y);                                               \
  u=(gavl_r_to_u_float[r]+gavl_g_to_u_float[g]+gavl_b_to_u_float[b]);                                \
  v=(gavl_r_to_v_float[r]+gavl_g_to_v_float[g]+gavl_b_to_v_float[b]);


/* 24 -> 16 */

#define RGB_24_TO_Y_16(r,g,b,y) y=(gavl_r_to_y[r]+gavl_g_to_y[g]+gavl_b_to_y[b])>>8;

#define RGB_24_TO_YJ_16(r,g,b,y) y=(gavl_r_to_yj[r]+gavl_g_to_yj[g]+gavl_b_to_yj[b])>>8;

#define RGB_24_TO_YUV_16(r,g,b,y,u,v)                                    \
  RGB_24_TO_Y_16(r,g,b,y);                                               \
  u=(gavl_r_to_u[r]+gavl_g_to_u[g]+gavl_b_to_u[b])>>8;                                \
  v=(gavl_r_to_v[r]+gavl_g_to_v[g]+gavl_b_to_v[b])>>8;

/* 48 -> 8 */


#define r_16_to_y ((int64_t)((0.29900*219.0/255.0)*0x10000))
#define g_16_to_y ((int64_t)((0.58700*219.0/255.0)*0x10000))
#define b_16_to_y ((int64_t)((0.11400*219.0/255.0)*0x10000))

#define r_16_to_u ((int64_t)(-(0.16874*224.0/255.0)*0x10000))
#define g_16_to_u ((int64_t)(-(0.33126*224.0/255.0)*0x10000))
#define b_16_to_u ((int64_t)( (0.50000*224.0/255.0)*0x10000))

#define r_16_to_v ((int64_t)( (0.50000*224.0/255.0)*0x10000))
#define g_16_to_v ((int64_t)(-(0.41869*224.0/255.0)*0x10000))
#define b_16_to_v ((int64_t)(-(0.08131*224.0/255.0)*0x10000))

#define r_16_to_yj ((int64_t)((0.29900)*0x10000))
#define g_16_to_yj ((int64_t)((0.58700)*0x10000))
#define b_16_to_yj ((int64_t)((0.11400)*0x10000))

#define r_16_to_uj ((int64_t)(-(0.16874)*0x10000))
#define g_16_to_uj ((int64_t)(-(0.33126)*0x10000))
#define b_16_to_uj ((int64_t)( (0.50000)*0x10000))

#define r_16_to_vj ((int64_t)( (0.50000)*0x10000))
#define g_16_to_vj ((int64_t)(-(0.41869)*0x10000))
#define b_16_to_vj ((int64_t)(-(0.08131)*0x10000))

#define r_16_to_yf (0.29900/65535.0)
#define g_16_to_yf (0.58700/65535.0)
#define b_16_to_yf (0.11400/65535.0)

#define r_16_to_uf (-0.16874/65535.0)
#define g_16_to_uf (-0.33126/65535.0)
#define b_16_to_uf (0.50000/65535.0)

#define r_16_to_vf (0.50000/65535.0))
#define g_16_to_vf (-0.41869/65535.0)
#define b_16_to_vf (-0.08131/65535.0)


#ifdef DO_ROUND

#define RGB_48_TO_Y_8(r,g,b,y) \
  y=(r_16_to_y * r + g_16_to_y * g + b_16_to_y * b + 0x10800000LL)>>24;
  
#define RGB_48_TO_YUV_8(r,g,b,y,u,v)                                    \
  RGB_48_TO_Y_8(r,g,b,y);                                               \
  u=(r_16_to_u * r + g_16_to_u * g + b_16_to_u * b + 0x80800000LL)>>24;  \
  v=(r_16_to_v * r + g_16_to_v * g + b_16_to_v * b + 0x80800000LL)>>24;

#define RGB_48_TO_YJ_8(r,g,b,y) \
  round_tmp=(r_16_to_yj * r + g_16_to_yj * g + b_16_to_yj * b)>>24;\
  y=RECLIP_32_TO_8(round_tmp);

#define RGB_48_TO_YJ_16(r,g,b,y) \
  round_tmp=(r_16_to_yj * r + g_16_to_yj * g + b_16_to_yj * b)>>16;\
  y=RECLIP_32_TO_16(round_tmp);

#define RGB_48_TO_YUVJ_8(r,g,b,y,u,v)                                    \
  RGB_48_TO_YJ_8(r,g,b,y);                                               \
  round_tmp=(r_16_to_uj * r + g_16_to_uj * g + b_16_to_uj * b + 0x80800000LL)>>24;  \
  u=RECLIP_32_TO_8(round_tmp); \
  round_tmp=(r_16_to_vj * r + g_16_to_vj * g + b_16_to_vj * b + 0x80800000LL)>>24; \
  v=RECLIP_32_TO_8(round_tmp);
  
#else // !DO_ROUND

#define RGB_48_TO_Y_8(r,g,b,y) \
  y=(r_16_to_y * r + g_16_to_y * g + b_16_to_y * b + 0x10000000LL)>>24;


#define RGB_48_TO_YUV_8(r,g,b,y,u,v)                                    \
  RGB_48_TO_Y_8(r,g,b,y);                                               \
  u=(r_16_to_u * r + g_16_to_u * g + b_16_to_u * b + 0x80000000LL)>>24;  \
  v=(r_16_to_v * r + g_16_to_v * g + b_16_to_v * b + 0x80000000LL)>>24;

#define RGB_48_TO_YJ_8(r,g,b,y) \
  y=(r_16_to_yj * r + g_16_to_yj * g + b_16_to_yj * b)>>24;

#define RGB_48_TO_YJ_16(r,g,b,y) \
  y=(r_16_to_yj * r + g_16_to_yj * g + b_16_to_yj * b)>>16;


#define RGB_48_TO_YUVJ_8(r,g,b,y,u,v)                                    \
  RGB_48_TO_YJ_8(r,g,b,y);                                               \
  u=(r_16_to_uj * r + g_16_to_uj * g + b_16_to_uj * b + 0x80000000LL)>>24;  \
  v=(r_16_to_vj * r + g_16_to_vj * g + b_16_to_vj * b + 0x80000000LL)>>24;

#endif // !DO_ROUND

/* 48 -> 16 */

#define RGB_48_TO_Y_16(r,g,b,y) \
  y=(r_16_to_y * r + g_16_to_y * g + b_16_to_y * b + 16 * 0x1000000LL)>>16;

#define RGB_48_TO_YUV_16(r,g,b,y,u,v)                                    \
  RGB_48_TO_Y_16(r,g,b,y);                                               \
  u=(r_16_to_u * r + g_16_to_u * g + b_16_to_u * b + 0x80000000LL)>>16;  \
  v=(r_16_to_v * r + g_16_to_v * g + b_16_to_v * b + 0x80000000LL)>>16;

/* 48 -> float */

#define RGB_48_TO_Y_FLOAT(r,g,b,y) \
  y=r_16_to_yf * (float)(r) + g_16_to_yf * (float)(g) + b_16_to_yf * (float)(b);
  
#define RGB_48_TO_YUV_FLOAT(r,g,b,y,u,v)                                    \
  RGB_48_TO_Y_FLOAT(r,g,b,y);                                               \
  u=r_16_to_uf * (float)(r) + g_16_to_uf * (float)(g) + b_16_to_uf * (float)(b);  \
  v=(r_16_to_vf * (float)(r) + g_16_to_vf * (float)(g) + b_16_to_vf * (float)(b);


/* RGB Float -> YUV */

#define r_float_to_y  0.29900
#define g_float_to_y  0.58700
#define b_float_to_y  0.11400

#define r_float_to_u  (-0.16874)
#define g_float_to_u  (-0.33126)
#define b_float_to_u   0.50000

#define r_float_to_v   0.50000
#define g_float_to_v  (-0.41869)
#define b_float_to_v  (-0.08131)

#define INIT_RGB_FLOAT_TO_YUV \
  float y_tmp, u_tmp, v_tmp;

#define INIT_RGB_FLOAT_TO_Y \
  float y_tmp;



/* Float -> 8 */

#define RGB_FLOAT_TO_Y_8(r, g, b, y)                              \
  y_tmp = r_float_to_y * r + g_float_to_y * g + b_float_to_y * b; \
  Y_FLOAT_TO_8(y_tmp, y);

#define RGB_FLOAT_TO_YUV_8(r, g, b, y, u, v)                      \
  RGB_FLOAT_TO_Y_8(r, g, b, y)                                    \
  u_tmp = r_float_to_u * r + g_float_to_u * g + b_float_to_u * b; \
  v_tmp = r_float_to_v * r + g_float_to_v * g + b_float_to_v * b; \
  UV_FLOAT_TO_8(u_tmp, u);                                        \
  UV_FLOAT_TO_8(v_tmp, v);

#define RGB_FLOAT_TO_YJ_8(r, g, b, y)                              \
  y_tmp = r_float_to_y * r + g_float_to_y * g + b_float_to_y * b; \
  YJ_FLOAT_TO_8(y_tmp, y);

#define RGB_FLOAT_TO_YUVJ_8(r, g, b, y, u, v)                      \
  RGB_FLOAT_TO_YJ_8(r, g, b, y)                                    \
  u_tmp = r_float_to_u * r + g_float_to_u * g + b_float_to_u * b; \
  v_tmp = r_float_to_v * r + g_float_to_v * g + b_float_to_v * b; \
  UVJ_FLOAT_TO_8(u_tmp, u);                                        \
  UVJ_FLOAT_TO_8(v_tmp, v);

/* Float -> 16 */

#define RGB_FLOAT_TO_Y_16(r, g, b, y)                              \
  y_tmp = r_float_to_y * r + g_float_to_y * g + b_float_to_y * b; \
  Y_FLOAT_TO_16(y_tmp, y);

#define RGB_FLOAT_TO_YUV_16(r, g, b, y, u, v)                      \
  RGB_FLOAT_TO_Y_16(r, g, b, y)                                    \
  u_tmp = r_float_to_u * r + g_float_to_u * g + b_float_to_u * b; \
  v_tmp = r_float_to_v * r + g_float_to_v * g + b_float_to_v * b; \
  UV_FLOAT_TO_16(u_tmp, u);                                        \
  UV_FLOAT_TO_16(v_tmp, v);

#define RGB_FLOAT_TO_YJ_16(r, g, b, y)                              \
  y_tmp = r_float_to_y * r + g_float_to_y * g + b_float_to_y * b; \
  Y_FLOAT_TO_16(y_tmp, y);

/* Float -> float */

#define RGB_FLOAT_TO_Y_FLOAT(r, g, b, y)                              \
  y = r_float_to_y * r + g_float_to_y * g + b_float_to_y * b; \

#define RGB_FLOAT_TO_YUV_FLOAT(r, g, b, y, u, v)                      \
  RGB_FLOAT_TO_Y_FLOAT(r, g, b, y)                                    \
  u = r_float_to_u * r + g_float_to_u * g + b_float_to_u * b; \
  v = r_float_to_v * r + g_float_to_v * g + b_float_to_v * b;


/* YUV (8bit) -> */


#define YUV_8_TO_RGB_24(y,u,v,r,g,b) i_tmp=(gavl_y_to_rgb[y]+gavl_v_to_r[v])>>16;\
                               r=RECLIP_32_TO_8(i_tmp);\
                               i_tmp=(gavl_y_to_rgb[y]+gavl_u_to_g[u]+gavl_v_to_g[v])>>16;\
                               g=RECLIP_32_TO_8(i_tmp);\
                               i_tmp=(gavl_y_to_rgb[y]+gavl_u_to_b[u])>>16;\
                               b=RECLIP_32_TO_8(i_tmp);

#define YUVJ_8_TO_RGB_24(y,u,v,r,g,b) i_tmp=(gavl_yj_to_rgb[y]+gavl_vj_to_r[v])>>16;\
                                r=RECLIP_32_TO_8(i_tmp);\
                                i_tmp=(gavl_yj_to_rgb[y]+gavl_uj_to_g[u]+gavl_vj_to_g[v])>>16;\
                                g=RECLIP_32_TO_8(i_tmp);\
                                i_tmp=(gavl_yj_to_rgb[y]+gavl_uj_to_b[u])>>16;\
                                b=RECLIP_32_TO_8(i_tmp);

#define YUV_8_TO_RGB_48(y,u,v,r,g,b) i_tmp=(gavl_y_to_rgb[y]+gavl_v_to_r[v])>>8;\
                               r=RECLIP_32_TO_16(i_tmp);\
                               i_tmp=(gavl_y_to_rgb[y]+gavl_u_to_g[u]+gavl_v_to_g[v])>>8;\
                               g=RECLIP_32_TO_16(i_tmp);\
                               i_tmp=(gavl_y_to_rgb[y]+gavl_u_to_b[u])>>8;\
                               b=RECLIP_32_TO_16(i_tmp);

#define YUVJ_8_TO_RGB_48(y,u,v,r,g,b) i_tmp=(gavl_yj_to_rgb[y]+gavl_vj_to_r[v])>>8;\
                                r=RECLIP_32_TO_16(i_tmp);\
                                i_tmp=(gavl_yj_to_rgb[y]+gavl_uj_to_g[u]+gavl_vj_to_g[v])>>8;\
                                g=RECLIP_32_TO_16(i_tmp);\
                                i_tmp=(gavl_yj_to_rgb[y]+gavl_uj_to_b[u])>>8;\
                                b=RECLIP_32_TO_16(i_tmp);

#define YUV_8_TO_RGB_FLOAT(y,u,v,r,g,b)                                 \
  i_tmp=(gavl_y_to_rgb_float[y]+gavl_v_to_r_float[v]);                         \
  r=RECLIP_FLOAT(i_tmp);                                                   \
  i_tmp=(gavl_y_to_rgb_float[y]+gavl_u_to_g_float[u]+gavl_v_to_g_float[v]);         \
  g=RECLIP_FLOAT(i_tmp);                                                   \
  i_tmp=(gavl_y_to_rgb_float[y]+gavl_u_to_b_float[u]);                         \
  b=RECLIP_FLOAT(i_tmp);

#define YUVJ_8_TO_RGB_FLOAT(y,u,v,r,g,b)                                \
  i_tmp=(gavl_yj_to_rgb_float[y]+gavl_vj_to_r_float[v]);                       \
  r=RECLIP_FLOAT(i_tmp);                                                   \
  i_tmp=(gavl_yj_to_rgb_float[y]+gavl_uj_to_g_float[u]+gavl_vj_to_g_float[v]);      \
  g=RECLIP_FLOAT(i_tmp);                                                   \
  i_tmp=(gavl_yj_to_rgb_float[y]+gavl_uj_to_b_float[u]);                       \
  b=RECLIP_FLOAT(i_tmp);

/* YUV (16 bit) -> 8 */

#define y_16_to_rgb  ((int64_t)(255.0/219.0 * 0x10000))
#define v_16_to_r    ((int64_t)( 1.40200*255.0/224.0 * 0x10000))
#define u_16_to_g    ((int64_t)(-0.34414*255.0/224.0 * 0x10000))
#define v_16_to_g    ((int64_t)(-0.71414*255.0/224.0 * 0x10000))
#define u_16_to_b    ((int64_t)( 1.77200*255.0/224.0 * 0x10000))

#ifdef DO_ROUND

#define YUV_16_TO_RGB_24(y,u,v,r,g,b) \
  i_tmp=(y_16_to_rgb * (y-0x1000) + v_16_to_r * (v-0x8000)+0x8000)>>24;        \
  r = RECLIP_64_TO_8(i_tmp);\
  i_tmp=(y_16_to_rgb * (y-0x1000) + u_16_to_g * (u-0x8000)+ v_16_to_g * (v-0x8000)+0x8000)>>24; \
  g = RECLIP_64_TO_8(i_tmp);\
  i_tmp=(y_16_to_rgb * (y-0x1000) + u_16_to_b * (u-0x8000)+0x8000)>>24; \
  b = RECLIP_64_TO_8(i_tmp);

#else

#define YUV_16_TO_RGB_24(y,u,v,r,g,b) \
  i_tmp=(y_16_to_rgb * (y-0x1000) + v_16_to_r * (v-0x8000))>>24;        \
  r = RECLIP_64_TO_8(i_tmp);\
  i_tmp=(y_16_to_rgb * (y-0x1000) + u_16_to_g * (u-0x8000)+ v_16_to_g * (v-0x8000))>>24; \
  g = RECLIP_64_TO_8(i_tmp);\
  i_tmp=(y_16_to_rgb * (y-0x1000) + u_16_to_b * (u-0x8000))>>24; \
  b = RECLIP_64_TO_8(i_tmp);

#endif

/* YUV (16 bit) -> 16 */

#define YUV_16_TO_RGB_48(y,u,v,r,g,b) \
  i_tmp=(y_16_to_rgb * (y-0x1000) + v_16_to_r * (v-0x8000))>>16;        \
  r = RECLIP_64_TO_16(i_tmp);\
  i_tmp=(y_16_to_rgb * (y-0x1000) + u_16_to_g * (u-0x8000)+ v_16_to_g * (v-0x8000))>>16; \
  g = RECLIP_64_TO_16(i_tmp);\
  i_tmp=(y_16_to_rgb * (y-0x1000) + u_16_to_b * (u-0x8000))>>16; \
  b = RECLIP_64_TO_16(i_tmp);

/* YUV (16 bit) -> float */

#define y_16_to_rgb_float (255.0/219.0/65535.0)
#define v_16_to_r_float   (1.40200*255.0/224.0/65535.0)
#define u_16_to_g_float   (-0.34414*255.0/224.0/65535.0)
#define v_16_to_g_float   (-0.71414*255.0/224.0/65535.0)
#define u_16_to_b_float   (1.77200*255.0/224.0/65535.0)

#define YUV_16_TO_RGB_FLOAT(y,u,v,r,g,b) \
  i_tmp=(y_16_to_rgb_float * (y-0x1000) + v_16_to_r_float * (v-0x8000));        \
  r = RECLIP_FLOAT(i_tmp);\
  i_tmp=(y_16_to_rgb_float * (y-0x1000) + u_16_to_g_float * (u-0x8000)+ v_16_to_g_float * (v-0x8000)); \
  g = RECLIP_FLOAT(i_tmp);\
  i_tmp=(y_16_to_rgb_float * (y-0x1000) + u_16_to_b_float * (u-0x8000)); \
  b = RECLIP_FLOAT(i_tmp);

/* YUV (float) -> 8 */

#define v_float_to_r_float   (1.40200)
#define u_float_to_g_float   (-0.34414)
#define v_float_to_g_float   (-0.71414)
#define u_float_to_b_float   (1.77200)

#define YUV_FLOAT_TO_RGB_24(y,u,v,r,g,b) \
  i_tmp=(y + v_float_to_r_float * v);        \
  i_tmp = RECLIP_FLOAT(i_tmp);\
  RGB_FLOAT_TO_8(i_tmp, r);\
  i_tmp=(y + u_float_to_g_float * u + v_float_to_g_float * v); \
  i_tmp = RECLIP_FLOAT(i_tmp);\
  RGB_FLOAT_TO_8(i_tmp, g);\
  i_tmp=(y + u_float_to_b_float * u); \
  i_tmp = RECLIP_FLOAT(i_tmp);\
  RGB_FLOAT_TO_8(i_tmp, b);

#define YUV_FLOAT_TO_RGB_48(y,u,v,r,g,b) \
  i_tmp=(y + v_float_to_r_float * v);        \
  i_tmp = RECLIP_FLOAT(i_tmp);\
  RGB_FLOAT_TO_16(i_tmp, r);\
  i_tmp=(y + u_float_to_g_float * u + v_float_to_g_float * v); \
  i_tmp = RECLIP_FLOAT(i_tmp);\
  RGB_FLOAT_TO_16(i_tmp, g);\
  i_tmp=(y + u_float_to_b_float * u); \
  i_tmp = RECLIP_FLOAT(i_tmp);\
  RGB_FLOAT_TO_16(i_tmp, b);

#define YUV_FLOAT_TO_RGB_FLOAT(y,u,v,r,g,b) \
  i_tmp=(y + v_float_to_r_float * v);        \
  r = RECLIP_FLOAT(i_tmp);\
  i_tmp=(y + u_float_to_g_float * u + v_float_to_g_float * v); \
  g = RECLIP_FLOAT(i_tmp);\
  i_tmp=(y + u_float_to_b_float * u); \
  b = RECLIP_FLOAT(i_tmp);

/* Combine r, g and b values to a 16 bit rgb pixel (taken from avifile) */

#define PACK_8_TO_RGB16(r,g,b,pixel) pixel=((((((r<<5)&0xff00)|g)<<6)&0xfff00)|b)>>3;
#define PACK_8_TO_BGR16(r,g,b,pixel) pixel=((((((b<<5)&0xff00)|g)<<6)&0xfff00)|r)>>3;

#define PACK_8_TO_RGB15(r,g,b,pixel) pixel=((((((r<<5)&0xff00)|g)<<5)&0xfff00)|b)>>3;
#define PACK_8_TO_BGR15(r,g,b,pixel) pixel=((((((b<<5)&0xff00)|g)<<5)&0xfff00)|r)>>3;

#define PACK_16_TO_RGB16(r,g,b,pixel) pixel=((((((r>>3)&0xff00)|(g>>8))<<6)&0xfff00)|(b>>8))>>3;
#define PACK_16_TO_BGR16(r,g,b,pixel) pixel=((((((b>>3)&0xff00)|(g>>8))<<6)&0xfff00)|(r>>8))>>3;

#define PACK_16_TO_RGB15(r,g,b,pixel) pixel=((((((r>>3)&0xff00)|(g>>8))<<5)&0xfff00)|(b>>8))>>3;
#define PACK_16_TO_BGR15(r,g,b,pixel) pixel=((((((b>>3)&0xff00)|(g>>8))<<5)&0xfff00)|(r>>8))>>3;


/* RGBA -> RGB */

#define INIT_RGBA_32 uint32_t anti_alpha;\
  uint32_t background_r=ctx->options->background_16[0] >> 8;    \
  uint32_t background_g=ctx->options->background_16[1] >> 8;    \
  uint32_t background_b=ctx->options->background_16[2] >> 8;

#define INIT_RGBA_64  uint32_t anti_alpha;\
  uint32_t background_r=ctx->options->background_16[0];  \
  uint32_t background_g=ctx->options->background_16[1];  \
  uint32_t background_b=ctx->options->background_16[2];

#define INIT_RGBA_FLOAT  float anti_alpha;                              \
  float background_r=ctx->options->background_float[0];                 \
  float background_g=ctx->options->background_float[1];                 \
  float background_b=ctx->options->background_float[2];

#define INIT_RGBA_32_GRAY \
  int background;                              \
  int src_gray; \
  RGB_24_TO_YJ_8(ctx->options->background_16[0] >> 8,\
                ctx->options->background_16[1] >> 8,\
                ctx->options->background_16[2] >> 8,\
                background);

#define INIT_RGBA_64_GRAY \
  uint32_t background;                              \
  uint32_t src_gray; \
  RGB_48_TO_YJ_16(ctx->options->background_16[0],\
                  ctx->options->background_16[1],\
                  ctx->options->background_16[2],\
                  background);

#define INIT_RGBA_FLOAT_GRAY \
  float background;                              \
  float src_gray; \
  RGB_FLOAT_TO_Y_FLOAT(ctx->options->background_float[0],\
                       ctx->options->background_float[1],\
                       ctx->options->background_float[2],\
                       background);

#define INIT_GRAYA_16 \
  int background;                              \
  RGB_24_TO_YJ_8(ctx->options->background_16[0] >> 8,\
                ctx->options->background_16[1] >> 8,\
                ctx->options->background_16[2] >> 8,\
                background);

#define INIT_GRAYA_32 \
  uint32_t background;                              \
  RGB_48_TO_YJ_16(ctx->options->background_16[0],\
                 ctx->options->background_16[1],\
                 ctx->options->background_16[2],\
                 background);

#define INIT_GRAYA_FLOAT \
  float background;                              \
  RGB_FLOAT_TO_Y_FLOAT(ctx->options->background_float[0],\
                    ctx->options->background_float[1],\
                    ctx->options->background_float[2],\
                    background);



#define RGBA_32_TO_RGB_24(src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b) \
anti_alpha = 0xFF - src_a;\
dst_r=((src_a*src_r)+(anti_alpha*background_r))>>8;\
dst_g=((src_a*src_g)+(anti_alpha*background_g))>>8;\
dst_b=((src_a*src_b)+(anti_alpha*background_b))>>8;

#define RGBA_32_TO_RGB_48(src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b) \
anti_alpha = 0xFF - src_a;\
dst_r=((src_a*src_r)+(anti_alpha*background_r));\
dst_g=((src_a*src_g)+(anti_alpha*background_g));\
dst_b=((src_a*src_b)+(anti_alpha*background_b));

#define RGBA_32_TO_GRAY_8(src_r, src_g, src_b, src_a, dst) \
  RGB_24_TO_YJ_8(src_r, src_g, src_b, src_gray) \
  dst=((src_a*src_gray)+((0xFF-src_a) *background))>>8;

#define RGBA_64_TO_GRAY_16(src_r, src_g, src_b, src_a, dst) \
  RGB_48_TO_YJ_16(src_r, src_g, src_b, src_gray) \
  dst=((src_a*src_gray)+((0xFFFF-src_a) *background))>>16;

#define RGBA_FLOAT_TO_GRAY_FLOAT(src_r, src_g, src_b, src_a, dst) \
  RGB_FLOAT_TO_Y_FLOAT(src_r, src_g, src_b, src_gray) \
  dst=((src_a*src_gray)+((1.0-src_a)*background));


#ifdef DO_ROUND

#define RGBA_64_TO_RGB_24(src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b) \
  anti_alpha = 0xFFFF - src_a;                                          \
  round_tmp=((src_a*src_r)+(anti_alpha*background_r))>>8; \
  round_tmp=(round_tmp+0x8000)>>16;\
  dst_r=RECLIP_32_TO_8(round_tmp);\
  round_tmp=((src_a*src_g)+(anti_alpha*background_g))>>8; \
  round_tmp=(round_tmp+0x8000)>>16;\
  dst_g=RECLIP_32_TO_8(round_tmp);\
  round_tmp=((src_a*src_b)+(anti_alpha*background_b))>>8;\
  round_tmp=(round_tmp+0x8000)>>16;\
  dst_b=RECLIP_32_TO_8(round_tmp);\

#else
#define RGBA_64_TO_RGB_24(src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b) \
  anti_alpha = 0xFFFF - src_a;                                          \
  dst_r=((src_a*src_r)+(anti_alpha*background_r))>>24; \
  dst_g=((src_a*src_g)+(anti_alpha*background_g))>>24; \
  dst_b=((src_a*src_b)+(anti_alpha*background_b))>>24;

#endif // DO_ROUND

#define RGBA_64_TO_RGB_48(src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b) \
  anti_alpha = 0xFFFF - src_a;                                          \
  dst_r=((src_a*src_r)+(anti_alpha*background_r))>>16; \
  dst_g=((src_a*src_g)+(anti_alpha*background_g))>>16; \
  dst_b=((src_a*src_b)+(anti_alpha*background_b))>>16;


#define GRAYA_16_TO_GRAY_8(src, src_a, dst) \
  dst = (src * src_a + (0xff - src_a) * background)>>8;

#define GRAYA_32_TO_GRAY_16(src, src_a, dst) \
  dst = ((uint32_t)src * src_a + (uint32_t)(0xffff - src_a) * background) >> 16;

#define GRAYA_FLOAT_TO_GRAY_FLOAT(src, src_a, dst) \
  dst = (src * src_a + (1.0 - src_a) * background);


#define RGBA_64_TO_RGB_FLOAT(src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b) \
  anti_alpha = 0xFFFF - src_a;                                          \
  dst_r=RGB_16_TO_FLOAT(((src_a*src_r)+(anti_alpha*background_r))>>16); \
  dst_g=RGB_16_TO_FLOAT(((src_a*src_g)+(anti_alpha*background_g))>>16); \
  dst_b=RGB_16_TO_FLOAT(((src_a*src_b)+(anti_alpha*background_b))>>16);


#define RGBA_FLOAT_TO_RGB_24(src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b) \
  anti_alpha = 1.0 - src_a;                                             \
  RGB_FLOAT_TO_8(src_a*src_r+anti_alpha*background_r, dst_r);            \
  RGB_FLOAT_TO_8(src_a*src_g+anti_alpha*background_g, dst_g);            \
  RGB_FLOAT_TO_8(src_a*src_b+anti_alpha*background_b, dst_b);

#define RGBA_FLOAT_TO_RGB_48(src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b) \
  anti_alpha = 1.0 - src_a;                                             \
  RGB_FLOAT_TO_16(src_a*src_r+anti_alpha*background_r, dst_r);            \
  RGB_FLOAT_TO_16(src_a*src_g+anti_alpha*background_g, dst_g);            \
  RGB_FLOAT_TO_16(src_a*src_b+anti_alpha*background_b, dst_b);

#define RGBA_FLOAT_TO_RGB_FLOAT(src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b) \
  anti_alpha = 1.0 - src_a;                                             \
  dst_r=src_a*src_r+anti_alpha*background_r;            \
  dst_g=src_a*src_g+anti_alpha*background_g;            \
  dst_b=src_a*src_b+anti_alpha*background_b;

/* YUVA 32 -> YUV */

#define INIT_YUVA_32 int anti_alpha;\
  int background_y;    \
  int background_u;    \
  int background_v;    \
  RGB_48_TO_YUV_8(ctx->options->background_16[0], \
                  ctx->options->background_16[1], \
                  ctx->options->background_16[2],\
                  background_y, background_u, background_v);

#define INIT_YUVA_64 uint32_t anti_alpha;\
  uint32_t background_y;    \
  uint32_t background_u;    \
  uint32_t background_v;    \
  RGB_48_TO_YUV_16(ctx->options->background_16[0], \
                  ctx->options->background_16[1], \
                  ctx->options->background_16[2],\
                  background_y, background_u, background_v);

#define INIT_YUVA_FLOAT_FLOAT float anti_alpha;\
  float background_y, background_u, background_v; \
  RGB_FLOAT_TO_YUV_FLOAT(ctx->options->background_float[0], \
                  ctx->options->background_float[1], \
                  ctx->options->background_float[2],\
                  background_y, background_u, background_v);

#define INIT_YUVA_FLOAT float tmp; \
  INIT_YUVA_FLOAT_FLOAT

#define YUVA_32_TO_YUV_8(src_y, src_u, src_v, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFF - src_a;                                            \
  dst_y=((src_a*src_y)+(anti_alpha*background_y))>>8;                   \
  dst_u=((src_a*src_u)+(anti_alpha*background_u))>>8;                   \
  dst_v=((src_a*src_v)+(anti_alpha*background_v))>>8;

#define YUVA_32_TO_YUV_FLOAT(src_y, src_u, src_v, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFF - src_a;                                            \
  dst_y=Y_16_TO_Y_FLOAT(((src_a*src_y)+(anti_alpha*background_y)));                   \
  dst_u=UV_16_TO_UV_FLOAT(((src_a*src_u)+(anti_alpha*background_u)));                   \
  dst_v=UV_16_TO_UV_FLOAT(((src_a*src_v)+(anti_alpha*background_v)));


#define YUVA_64_TO_YUV_8(src_y, src_u, src_v, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFFFF - src_a;                                            \
  dst_y=((src_a*src_y)+(anti_alpha*background_y))>>24;                   \
  dst_u=((src_a*src_u)+(anti_alpha*background_u))>>24;                   \
  dst_v=((src_a*src_v)+(anti_alpha*background_v))>>24;

#define YUVA_64_TO_YUV_FLOAT(src_y, src_u, src_v, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFFFF - src_a;                                            \
  dst_y=Y_16_TO_Y_FLOAT(((src_a*src_y)+(anti_alpha*background_y))>>16);                   \
  dst_u=UV_16_TO_UV_FLOAT(((src_a*src_u)+(anti_alpha*background_u))>>16);                   \
  dst_v=UV_16_TO_UV_FLOAT(((src_a*src_v)+(anti_alpha*background_v))>>16);

#define YUVA_FLOAT_TO_YUV_8(src_y, src_u, src_v, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 1.0 - src_a;                                            \
  tmp=((src_a*src_y)+(anti_alpha*background_y));                   \
  Y_FLOAT_TO_8(tmp, dst_y);\
  tmp=((src_a*src_u)+(anti_alpha*background_u));                   \
  UV_FLOAT_TO_8(tmp, dst_u);\
  tmp=((src_a*src_v)+(anti_alpha*background_v));\
  UV_FLOAT_TO_8(tmp, dst_v);

#define YUVA_FLOAT_TO_YUV_FLOAT(src_y, src_u, src_v, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 1.0 - src_a;                                            \
  dst_y=((src_a*src_y)+(anti_alpha*background_y));\
  dst_u=((src_a*src_u)+(anti_alpha*background_u));\
  dst_v=((src_a*src_v)+(anti_alpha*background_v));

#define YUVA_FLOAT_TO_YUVJ_8(src_y, src_u, src_v, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 1.0 - src_a;                                            \
  tmp=((src_a*src_y)+(anti_alpha*background_y));                   \
  YJ_FLOAT_TO_8(tmp, dst_y);\
  tmp=((src_a*src_u)+(anti_alpha*background_u));                   \
  UVJ_FLOAT_TO_8(tmp, dst_u);\
  tmp=((src_a*src_v)+(anti_alpha*background_v));\
  UVJ_FLOAT_TO_8(tmp, dst_v);

#define YUVA_FLOAT_TO_YUV_16(src_y, src_u, src_v, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 1.0 - src_a;                                            \
  tmp=((src_a*src_y)+(anti_alpha*background_y));                   \
  Y_FLOAT_TO_16(tmp, dst_y);\
  tmp=((src_a*src_u)+(anti_alpha*background_u));                   \
  UV_FLOAT_TO_16(tmp, dst_u);\
  tmp=((src_a*src_v)+(anti_alpha*background_v));\
  UV_FLOAT_TO_16(tmp, dst_v);


#define YUVA_32_TO_Y_8(src_y, src_a, dst_y)                \
  anti_alpha = 0xFF - src_a;                                            \
  dst_y=((src_a*src_y)+(anti_alpha*background_y))>>8;

#define YUVA_32_TO_Y_FLOAT(src_y, src_a, dst_y)                \
  anti_alpha = 0xFF - src_a;                                            \
  dst_y=Y_8_TO_FLOAT(((src_a*src_y)+(anti_alpha*background_y))>>8);

#define YUVA_FLOAT_TO_Y_16(src_y, src_a, dst_y) \
  anti_alpha = 1.0 - src_a;                                            \
  tmp=((src_a*src_y)+(anti_alpha*background_y));                   \
  Y_FLOAT_TO_16(tmp, dst_y);

#define YUVA_FLOAT_TO_Y_8(src_y, src_a, dst_y) \
  anti_alpha = 1.0 - src_a;                                            \
  tmp=((src_a*src_y)+(anti_alpha*background_y));                   \
  Y_FLOAT_TO_8(tmp, dst_y);

#define YUVA_64_TO_Y_8(src_y, src_a, dst_y)                \
  anti_alpha = 0xFFFF - src_a;                                            \
  dst_y=((src_a*src_y)+(anti_alpha*background_y))>>24;

#define YUVA_64_TO_Y_FLOAT(src_y, src_a, dst_y)                \
  anti_alpha = 0xFFFF - src_a;                                            \
  dst_y=Y_16_TO_Y_FLOAT(((src_a*src_y)+(anti_alpha*background_y))>>24);

#define YUVA_32_TO_YUVJ_8(src_y, src_u, src_v, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFF - src_a;                                            \
  Y_16_TO_YJ_8(((src_a*src_y)+(anti_alpha*background_y)), dst_y);        \
  UV_16_TO_UVJ_8(((src_a*src_u)+(anti_alpha*background_u)), dst_u);      \
  UV_16_TO_UVJ_8(((src_a*src_v)+(anti_alpha*background_v)), dst_v);

#define YUVA_64_TO_YUVJ_8(src_y, src_u, src_v, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFFFF - src_a;                                            \
  Y_16_TO_YJ_8(((src_a*src_y)+(anti_alpha*background_y))>>16, dst_y);        \
  UV_16_TO_UVJ_8(((src_a*src_u)+(anti_alpha*background_u))>>16, dst_u);      \
  UV_16_TO_UVJ_8(((src_a*src_v)+(anti_alpha*background_v))>>16, dst_v);

#define YUVA_FLOAT_TO_YUVJ_8(src_y, src_u, src_v, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 1.0 - src_a;                                            \
  tmp=((src_a*src_y)+(anti_alpha*background_y));                   \
  YJ_FLOAT_TO_8(tmp, dst_y);\
  tmp=((src_a*src_u)+(anti_alpha*background_u));                   \
  UVJ_FLOAT_TO_8(tmp, dst_u);\
  tmp=((src_a*src_v)+(anti_alpha*background_v));\
  UVJ_FLOAT_TO_8(tmp, dst_v);

#define YUVA_32_TO_YJ_8(src_y, src_a, dst_y)                \
  anti_alpha = 0xFF - src_a;                                            \
  Y_16_TO_YJ_8(((src_a*src_y)+(anti_alpha*background_y)), dst_y);

#define YUVA_64_TO_YJ_8(src_y, src_a, dst_y)                \
  anti_alpha = 0xFFFF - src_a;                                            \
  Y_16_TO_YJ_8(((src_a*src_y)+(anti_alpha*background_y))>>16, dst_y);

#define YUVA_FLOAT_TO_YJ_8(src_y, src_a, dst_y) \
  anti_alpha = 1.0 - src_a;                                            \
  tmp=((src_a*src_y)+(anti_alpha*background_y));                   \
  YJ_FLOAT_TO_8(tmp, dst_y);

#define YUVA_32_TO_YUV_16(src_y, src_u, src_v, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFF - src_a;                                            \
  dst_y=((src_a*src_y)+(anti_alpha*background_y));                   \
  dst_u=((src_a*src_u)+(anti_alpha*background_u));                   \
  dst_v=((src_a*src_v)+(anti_alpha*background_v));

#define YUVA_64_TO_YUV_16(src_y, src_u, src_v, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFFFF - src_a;                                            \
  dst_y=((src_a*src_y)+(anti_alpha*background_y))>>16;                   \
  dst_u=((src_a*src_u)+(anti_alpha*background_u))>>16;                   \
  dst_v=((src_a*src_v)+(anti_alpha*background_v))>>16;
  
#define YUVA_32_TO_Y_16(src_y, src_a, dst_y)               \
  anti_alpha = 0xFF - src_a;                                            \
  dst_y=((src_a*src_y)+(anti_alpha*background_y));

#define YUVA_64_TO_Y_16(src_y, src_a, dst_y)               \
  anti_alpha = 0xFFFF - src_a;                                            \
  dst_y=((src_a*src_y)+(anti_alpha*background_y))>>16;


/********************************************************************
 * RGBA -> YUV
 ********************************************************************/

/* 8 Bit */

#define RGBA_32_TO_YUV_8(src_r, src_g, src_b, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFF - src_a;                                            \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));                   \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));                   \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));                   \
  RGB_48_TO_YUV_8(r_tmp, g_tmp, b_tmp, dst_y, dst_u, dst_v);

#define RGBA_32_TO_Y_8(src_r, src_g, src_b, src_a, dst_y) \
  anti_alpha = 0xFF - src_a;                              \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));     \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));     \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));     \
  RGB_48_TO_Y_8(r_tmp, g_tmp, b_tmp, dst_y);

#define RGBA_32_TO_YUVJ_8(src_r, src_g, src_b, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFF - src_a;                                            \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));   \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));   \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));           \
  RGB_48_TO_YUVJ_8(r_tmp, g_tmp, b_tmp, dst_y, dst_u, dst_v);

#define RGBA_32_TO_YJ_8(src_r, src_g, src_b, src_a, dst_y) \
  anti_alpha = 0xFF - src_a;                               \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));      \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));      \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));      \
  RGB_48_TO_YJ_8(r_tmp, g_tmp, b_tmp, dst_y);

/* RGBA 32 to YUV 16 */

#define RGBA_32_TO_YUV_16(src_r, src_g, src_b, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFF - src_a;                                            \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));                   \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));                   \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));                   \
  RGB_48_TO_YUV_16(r_tmp, g_tmp, b_tmp, dst_y, dst_u, dst_v);

#define RGBA_32_TO_Y_16(src_r, src_g, src_b, src_a, dst_y) \
  anti_alpha = 0xFF - src_a;                              \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));     \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));     \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));     \
  RGB_48_TO_Y_16(r_tmp, g_tmp, b_tmp, dst_y);

#define RGBA_32_TO_YUV_FLOAT(src_r, src_g, src_b, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFF - src_a;                                            \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));                   \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));                   \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));                   \
  RGB_48_TO_YUV_FLOAT(r_tmp, g_tmp, b_tmp, dst_y, dst_u, dst_v);


/* 16 Bit */

#define RGBA_64_TO_YUV_8(src_r, src_g, src_b, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFFFF - src_a;                                            \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r))>>16;                  \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g))>>16;                  \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b))>>16;                  \
  RGB_48_TO_YUV_8(r_tmp, g_tmp, b_tmp, dst_y, dst_u, dst_v);

#define RGBA_64_TO_Y_8(src_r, src_g, src_b, src_a, dst_y) \
  anti_alpha = 0xFFFF - src_a;                              \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r))>>16;     \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g))>>16;     \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b))>>16;     \
  RGB_48_TO_Y_8(r_tmp, g_tmp, b_tmp, dst_y);

#define RGBA_64_TO_YUVJ_8(src_r, src_g, src_b, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFFFF - src_a;                                            \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r))>>16;   \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g))>>16;   \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b))>>16;           \
  RGB_48_TO_YUVJ_8(r_tmp, g_tmp, b_tmp, dst_y, dst_u, dst_v);

#define RGBA_64_TO_YJ_8(src_r, src_g, src_b, src_a, dst_y) \
  anti_alpha = 0xFFFF - src_a;                               \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r))>>16;      \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g))>>16;      \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b))>>16;      \
  RGB_48_TO_YJ_8(r_tmp, g_tmp, b_tmp, dst_y);

#define RGBA_64_TO_YUV_16(src_r, src_g, src_b, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFFFF - src_a;                                            \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r))>>16;                  \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g))>>16;                  \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b))>>16;                  \
  RGB_48_TO_YUV_16(r_tmp, g_tmp, b_tmp, dst_y, dst_u, dst_v);

#define RGBA_64_TO_Y_16(src_r, src_g, src_b, src_a, dst_y) \
  anti_alpha = 0xFFFF - src_a;                              \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r))>>16;     \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g))>>16;     \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b))>>16;     \
  RGB_48_TO_Y_16(r_tmp, g_tmp, b_tmp, dst_y);

#define RGBA_64_TO_YUV_FLOAT(src_r, src_g, src_b, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 0xFFFF - src_a;                                            \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r))>>16;                  \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g))>>16;                  \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b))>>16;                  \
  RGB_48_TO_YUV_FLOAT(r_tmp, g_tmp, b_tmp, dst_y, dst_u, dst_v);


/* Float */

#define RGBA_FLOAT_TO_YUV_8(src_r, src_g, src_b, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 1.0 - src_a;                                            \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));                  \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));                  \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));                  \
  RGB_FLOAT_TO_YUV_8(r_tmp, g_tmp, b_tmp, dst_y, dst_u, dst_v);

#define RGBA_FLOAT_TO_Y_8(src_r, src_g, src_b, src_a, dst_y) \
  anti_alpha = 1.0 - src_a;                              \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));     \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));     \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));     \
  RGB_FLOAT_TO_Y_8(r_tmp, g_tmp, b_tmp, dst_y);

#define RGBA_FLOAT_TO_YUVJ_8(src_r, src_g, src_b, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 1.0 - src_a;                                            \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));   \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));   \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));           \
  RGB_FLOAT_TO_YUVJ_8(r_tmp, g_tmp, b_tmp, dst_y, dst_u, dst_v);

#define RGBA_FLOAT_TO_YJ_8(src_r, src_g, src_b, src_a, dst_y) \
  anti_alpha = 1.0 - src_a;                               \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));      \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));      \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));      \
  RGB_FLOAT_TO_YJ_8(r_tmp, g_tmp, b_tmp, dst_y);

#define RGBA_FLOAT_TO_YUV_16(src_r, src_g, src_b, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 1.0 - src_a;                                            \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));                  \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));                  \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));                  \
  RGB_FLOAT_TO_YUV_16(r_tmp, g_tmp, b_tmp, dst_y, dst_u, dst_v);

#define RGBA_FLOAT_TO_Y_16(src_r, src_g, src_b, src_a, dst_y) \
  anti_alpha = 1.0 - src_a;                              \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));     \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));     \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));     \
  RGB_FLOAT_TO_Y_16(r_tmp, g_tmp, b_tmp, dst_y);

#define RGBA_FLOAT_TO_YUV_FLOAT(src_r, src_g, src_b, src_a, dst_y, dst_u, dst_v) \
  anti_alpha = 1.0 - src_a;                                            \
  r_tmp=((src_a*src_r)+(anti_alpha*background_r));                  \
  g_tmp=((src_a*src_g)+(anti_alpha*background_g));                  \
  b_tmp=((src_a*src_b)+(anti_alpha*background_b));                  \
  RGB_FLOAT_TO_YUV_FLOAT(r_tmp, g_tmp, b_tmp, dst_y, dst_u, dst_v);
