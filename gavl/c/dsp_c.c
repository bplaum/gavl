/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2024 Members of the Gmerlin project
 * http://github.com/bplaum
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
#include <gavl/gavldsp.h>
#include <dsp.h>
#include <bswap.h>
#include <math.h>

#include "colorspace_tables.h"
#include "colorspace_macros.h"

/* Sum of absolute differences */
static int sad_rgb15_c(const uint8_t * src_1, const uint8_t * src_2, 
                       int stride_1, int stride_2, 
                       int w, int h)
  {
  int ret = 0, i, j;
  uint16_t * s1, *s2;
  for(i = 0; i < h; i++)
    {
    s1 = (uint16_t *)src_1;
    s2 = (uint16_t *)src_2;

    for(j = 0; j < w; j++)
      {
      ret += 
        abs(RGB15_TO_R_8(*s1)-RGB15_TO_R_8(*s2)) +
        abs(RGB15_TO_G_8(*s1)-RGB15_TO_G_8(*s2)) +
        abs(RGB15_TO_B_8(*s1)-RGB15_TO_B_8(*s2));
      s1++;
      s2++;
      }
    src_1 += stride_1;
    src_2 += stride_2;
    }
  return ret;
  }

static int sad_rgb16_c(const uint8_t * src_1, const uint8_t * src_2, 
                   int stride_1, int stride_2, 
                   int w, int h)
  {
  int ret = 0, i, j;
  uint16_t * s1, *s2;
  for(i = 0; i < h; i++)
    {
    s1 = (uint16_t *)src_1;
    s2 = (uint16_t *)src_2;

    for(j = 0; j < w; j++)
      {
      ret += 
        abs(RGB16_TO_R_8(*s1)-RGB16_TO_R_8(*s2)) +
        abs(RGB16_TO_G_8(*s1)-RGB16_TO_G_8(*s2)) +
        abs(RGB16_TO_B_8(*s1)-RGB16_TO_B_8(*s2));
      s1++;
      s2++;
      }
    src_1 += stride_1;
    src_2 += stride_2;
    }
  return ret;
  }

static int sad_8_c(const uint8_t * src_1, const uint8_t * src_2, 
                   int stride_1, int stride_2, 
                   int w, int h)
  {
  int ret = 0, i, j;
  const uint8_t * s1, *s2;
  for(i = 0; i < h; i++)
    {
    s1 = src_1;
    s2 = src_2;

    for(j = 0; j < w; j++)
      {
      ret += abs((*s1)-(*s2));
      s1++;
      s2++;
      }
    src_1 += stride_1;
    src_2 += stride_2;
    }
  return ret;
  }

static int sad_16_c(const uint8_t * src_1, const uint8_t * src_2, 
                    int stride_1, int stride_2, 
                    int w, int h)
  {
  int ret = 0, i, j;
  const uint16_t * s1, *s2;
  for(i = 0; i < h; i++)
    {
    s1 = (const uint16_t*)src_1;
    s2 = (const uint16_t*)src_2;

    for(j = 0; j < w; j++)
      {
      ret += abs((*s1)-(*s2));
      s1++;
      s2++;
      }
    src_1 += stride_1;
    src_2 += stride_2;
    }
  return ret;
  }

static float sad_f_c(const uint8_t * src_1, const uint8_t * src_2, 
                     int stride_1, int stride_2, 
                     int w, int h)
  {
  float ret = 0.0;
  int i, j;
  const float * s1, *s2;
  for(i = 0; i < h; i++)
    {
    s1 = (const float*)src_1;
    s2 = (const float*)src_2;

    for(j = 0; j < w; j++)
      {
      ret += fabs((*s1)-(*s2));
      s1++;
      s2++;
      }
    src_1 += stride_1;
    src_2 += stride_2;
    }
  return ret;
  }

/* Averaging */
static void average_rgb15_c(const uint8_t * src_1, const uint8_t * src_2, 
                        uint8_t * dst, int num)
  {
  int i;
  const uint16_t * s1, *s2;
  uint16_t *d;

  s1 = (const uint16_t*)src_1;
  s2 = (const uint16_t*)src_2;
  d = (uint16_t*)dst;

  for(i = 0; i < num; i++)
    {
    *d = 
      (((*s1 & RGB15_LOWER_MASK) + 
        (*s2 & RGB15_LOWER_MASK)) >> 1) & RGB15_LOWER_MASK;
    *d |= 
      (((*s1 & RGB15_MIDDLE_MASK) + 
        (*s2 & RGB15_MIDDLE_MASK)) >> 1) & RGB15_MIDDLE_MASK;
    *d |= 
      (((*s1 & RGB15_UPPER_MASK) + 
        (*s2 & RGB15_UPPER_MASK)) >> 1) & RGB15_UPPER_MASK;
    s1++;
    s2++;
    d++;
    }
  
  }

static void average_rgb16_c(const uint8_t * src_1, const uint8_t * src_2, 
                        uint8_t * dst, int num)
  {
  int i;
  const uint16_t * s1, *s2;
  uint16_t *d;

  s1 = (const uint16_t*)src_1;
  s2 = (const uint16_t*)src_2;
  d = (uint16_t*)dst;

  for(i = 0; i < num; i++)
    {
    *d = 
      (((*s1 & RGB16_LOWER_MASK) + 
        (*s2 & RGB16_LOWER_MASK)) >> 1) & RGB16_LOWER_MASK;
    *d |= 
      (((*s1 & RGB16_MIDDLE_MASK) + 
        (*s2 & RGB16_MIDDLE_MASK)) >> 1) & RGB16_MIDDLE_MASK;
    *d |= 
      (((*s1 & RGB16_UPPER_MASK) + 
        (*s2 & RGB16_UPPER_MASK)) >> 1) & RGB16_UPPER_MASK;
    s1++;
    s2++;
    d++;
    }
  
  }

static void average_8_c(const uint8_t * src_1, const uint8_t * src_2, 
                    uint8_t * dst, int num)
  {
  int i;
  for(i = 0; i < num; i++)
    {
    *dst = (*src_1 + *src_2 + 1) >> 1;
    src_1++;
    src_2++;
    dst++;
    }
  }

static void average_16_c(const uint8_t * src_1, const uint8_t * src_2, 
                     uint8_t * dst, int num)
  {
  int i;
  const uint16_t * s1, *s2;
  uint16_t *d;

  s1 = (const uint16_t*)src_1;
  s2 = (const uint16_t*)src_2;
  d = (uint16_t*)dst;

  for(i = 0; i < num; i++)
    {
    *d = (*s1 + *s2 + 1) >> 1;
    s1++;
    s2++;
    d++;
    }

  }

static void average_f_c(const uint8_t * src_1, const uint8_t * src_2, 
                        uint8_t * dst, int num)
  {
  int i;
  const float * s1, *s2;
  float *d;

  s1 = (const float*)src_1;
  s2 = (const float*)src_2;
  d = (float*)dst;

  for(i = 0; i < num; i++)
    {
    *d = (*s1 + *s2) * 0.5;
    s1++;
    s2++;
    d++;
    }
  }


/* */

/* Interpolating */

static void interpolate_rgb15_c(const uint8_t * src_1, const uint8_t * src_2, 
                            uint8_t * dst, int num, float fac)
  {
  int i;
  const uint16_t * s1, *s2;
  uint16_t *d;
  int fac_i = (int)(fac * 0x10000 + 0.5);
  int anti_fac = 0x10000 - fac_i;
  
  s1 = (const uint16_t*)src_1;
  s2 = (const uint16_t*)src_2;
  d = (uint16_t*)dst;

  for(i = 0; i < num; i++)
    {
    *d = 
      (((*s1 & RGB15_LOWER_MASK)*fac_i + 
        (*s2 & RGB15_LOWER_MASK)*anti_fac) >> 16) & RGB15_LOWER_MASK;
    *d |= 
      (((*s1 & RGB15_MIDDLE_MASK)*fac_i + 
        (*s2 & RGB15_MIDDLE_MASK)*anti_fac) >> 16) & RGB15_MIDDLE_MASK;
    *d |= 
      (((*s1 & RGB15_UPPER_MASK)*fac_i + 
        (*s2 & RGB15_UPPER_MASK)*anti_fac) >> 16) & RGB15_UPPER_MASK;
    s1++;
    s2++;
    d++;
    }
  
  }

static void interpolate_rgb16_c(const uint8_t * src_1, const uint8_t * src_2, 
                        uint8_t * dst, int num, float fac)
  {
  int i;
  const uint16_t * s1, *s2;
  uint16_t *d;
  int fac_i = (int)(fac * 0x10000 + 0.5);
  int anti_fac = 0x10000 - fac_i;

  s1 = (const uint16_t*)src_1;
  s2 = (const uint16_t*)src_2;
  d = (uint16_t*)dst;

  for(i = 0; i < num; i++)
    {
    *d = 
      (((*s1 & RGB16_LOWER_MASK)*fac_i + 
        (*s2 & RGB16_LOWER_MASK)*anti_fac) >> 16) & RGB16_LOWER_MASK;
    *d |= 
      (((*s1 & RGB16_MIDDLE_MASK)*fac_i + 
        (*s2 & RGB16_MIDDLE_MASK)*anti_fac) >> 16) & RGB16_MIDDLE_MASK;
    *d |= 
      (((*s1 & RGB16_UPPER_MASK)*fac_i + 
        (*s2 & RGB16_UPPER_MASK)*anti_fac) >> 16) & RGB16_UPPER_MASK;
    s1++;
    s2++;
    d++;
    }
  
  }

static void interpolate_8_c(const uint8_t * src_1, const uint8_t * src_2, 
                    uint8_t * dst, int num, float fac)
  {
  int i;
  int fac_i = (int)(fac * 0x10000 + 0.5);
  int anti_fac = 0x10000 - fac_i;
  for(i = 0; i < num; i++)
    {
    *dst = (*src_1 * fac_i + *src_2 * anti_fac) >> 16;
    src_1++;
    src_2++;
    dst++;
    }
  }

static void interpolate_16_c(const uint8_t * src_1, const uint8_t * src_2, 
                             uint8_t * dst, int num, float fac)
  {
  int i;
  const uint16_t * s1, *s2;
  uint16_t *d;
  int fac_i = (int)(fac * 0x8000 + 0.5);
  int anti_fac = 0x8000 - fac_i;

  s1 = (const uint16_t*)src_1;
  s2 = (const uint16_t*)src_2;
  d = (uint16_t*)dst;

  for(i = 0; i < num; i++)
    {
    *d = (*s1 * fac_i + *s2 * anti_fac) >> 15;
    s1++;
    s2++;
    d++;
    }

  }

static void interpolate_f_c(const uint8_t * src_1, const uint8_t * src_2, 
                            uint8_t * dst, int num, float fac)
  {
  int i;
  const float * s1, *s2;
  float *d;
  float anti_fac = 1.0 - fac;
  s1 = (const float*)src_1;
  s2 = (const float*)src_2;
  d = (float*)dst;

  for(i = 0; i < num; i++)
    {
    *d = *s1 * fac + *s2 * anti_fac;
    s1++;
    s2++;
    d++;
    }
  }

static void bswap_16_c(void * data, int len)
  {
  int i;
  uint16_t * ptr = (uint16_t*)data;

  for(i = 0; i < len; i++)
    {
    *ptr = bswap_16(*ptr);
    ptr++;
    }
  
  }

static void bswap_32_c(void * data, int len)
  {
  int i;
  uint32_t * ptr = (uint32_t*)data;
  for(i = 0; i < len; i++)
    {
    *ptr = bswap_32(*ptr);
    ptr++;
    }
  }

static void bswap_64_c(void * data, int len)
  {
  int i;
  uint64_t * ptr = (uint64_t*)data;
  for(i = 0; i < len; i++)
    {
    *ptr = bswap_64(*ptr);
    ptr++;
    }
  }

#define GENERIC_CLIP(val, min, max) (val>max?max:(val<min?min:val))

/* Add functions */

static void add_u8_c(const void * _src1, const void * _src2,
                     void * _dst, int num)
  {
  int32_t tmp;
  int i = num+1;
  const uint8_t * src1 = _src1;
  const uint8_t * src2 = _src2;
  uint8_t * dst  = _dst;
  
  while(--i)
    {
    tmp = *(src1++) + *(src2++);
    *(dst++) = RECLIP_32_TO_8(tmp);
    }
  }

static void add_s8_c(const void * _src1, const void * _src2,
                     void * _dst, int num)
  {
  int tmp;
  int i = num+1;
  const int8_t * src1 = _src1;
  const int8_t * src2 = _src2;
  int8_t * dst  = _dst;

  while(--i)
    {
    tmp = *(src1++) + *(src2++);
    *(dst++) = GENERIC_CLIP(tmp,-128,127);
    }
  }

static void add_u8_s_c(const void * _src1, const void * _src2,
                       void * _dst, int num)
  {
  int tmp;
  int i = num+1;
  const uint8_t * src1 = _src1;
  const uint8_t * src2 = _src2;
  uint8_t * dst  = _dst;

  while(--i)
    {
    tmp = *(src1++) + *(src2++) - 256;
    *(dst++) = GENERIC_CLIP(tmp,-128,127) + 128;
    }
  }


static void add_u16_c(const void * _src1, const void * _src2,
                      void * _dst, int num)
  {
  int32_t tmp;
  int i = num+1;
  const uint16_t * src1 = _src1;
  const uint16_t * src2 = _src2;
  uint16_t * dst  = _dst;

  while(--i)
    {
    tmp = *(src1++) + *(src2++);
    *(dst++) = RECLIP_32_TO_16(tmp);
    }

  }

static void add_s16_c(const void * _src1, const void * _src2,
                      void * _dst, int num)
  {
  int tmp;
  int i = num+1;
  const int16_t * src1 = _src1;
  const int16_t * src2 = _src2;
  int16_t * dst  = _dst;

  while(--i)
    {
    tmp = *(src1++) + *(src2++);
    *(dst++) = GENERIC_CLIP(tmp,-32768,32767);
    }
  }

static void add_u16_s_c(const void * _src1, const void * _src2,
                        void * _dst, int num)
  {
  int tmp;
  int i = num+1;
  const uint16_t * src1 = _src1;
  const uint16_t * src2 = _src2;
  uint16_t * dst  = _dst;

  while(--i)
    {
    tmp = *(src1++) + *(src2++) - 65536;
    *(dst++) = GENERIC_CLIP(tmp,-32768,32767) + 32768;
    }
  }

static void add_s32_c(const void * _src1, const void * _src2,
                      void * _dst, int num)
  {
  int64_t tmp;
  int i = num+1;
  const int32_t * src1 = _src1;
  const int32_t * src2 = _src2;
  int32_t * dst  = _dst;

  while(--i)
    {
    tmp = *(src1++) + *(src2++);
    *(dst++) = GENERIC_CLIP(tmp,-2147483648LL,2147483647LL);
    }
  }

static void add_float_c(const void * _src1, const void * _src2,
                        void * _dst, int num)
  {
  int i = num+1;
  const float * src1 = _src1;
  const float * src2 = _src2;
  float * dst  = _dst;

  while(--i)
    {
    *(dst++) = *(src1++) + *(src2++);
    }
  }

static void add_double_c(const void * _src1, const void * _src2,
                         void * _dst, int num)
  {
  int i = num+1;
  const double * src1 = _src1;
  const double * src2 = _src2;
  double * dst  = _dst;
  
  while(--i)
    {
    *(dst++) = *(src1++) + *(src2++);
    }
  }

/* Subtract functions */

static void sub_u8_c(const void * _src1, const void * _src2,
                     void * _dst, int num)
  {
  int32_t tmp;
  int i = num+1;
  const uint8_t * src1 = _src1;
  const uint8_t * src2 = _src2;
  uint8_t * dst  = _dst;
  
  while(--i)
    {
    tmp = *(src1++) - *(src2++);
    *(dst++) = RECLIP_32_TO_8(tmp);
    }
  }

static void sub_s8_c(const void * _src1, const void * _src2,
                     void * _dst, int num)
  {
  int tmp;
  int i = num+1;
  const int8_t * src1 = _src1;
  const int8_t * src2 = _src2;
  int8_t * dst  = _dst;

  while(--i)
    {
    tmp = *(src1++) - *(src2++);
    *(dst++) = GENERIC_CLIP(tmp,-128,127);
    }
  }

static void sub_u8_s_c(const void * _src1, const void * _src2,
                       void * _dst, int num)
  {
  int tmp;
  int i = num+1;
  const uint8_t * src1 = _src1;
  const uint8_t * src2 = _src2;
  uint8_t * dst  = _dst;

  while(--i)
    {
    tmp = *(src1++) - *(src2++);
    *(dst++) = GENERIC_CLIP(tmp,-128,127) + 128;
    }
  }


static void sub_u16_c(const void * _src1, const void * _src2,
                      void * _dst, int num)
  {
  int32_t tmp;
  int i = num+1;
  const uint16_t * src1 = _src1;
  const uint16_t * src2 = _src2;
  uint16_t * dst  = _dst;

  while(--i)
    {
    tmp = *(src1++) - *(src2++);
    *(dst++) = RECLIP_32_TO_16(tmp);
    }

  }

static void sub_s16_c(const void * _src1, const void * _src2,
                      void * _dst, int num)
  {
  int tmp;
  int i = num+1;
  const int16_t * src1 = _src1;
  const int16_t * src2 = _src2;
  int16_t * dst  = _dst;

  while(--i)
    {
    tmp = *(src1++) - *(src2++);
    *(dst++) = GENERIC_CLIP(tmp,-32768,32767);
    }
  }

static void sub_u16_s_c(const void * _src1, const void * _src2,
                        void * _dst, int num)
  {
  int tmp;
  int i = num+1;
  const uint16_t * src1 = _src1;
  const uint16_t * src2 = _src2;
  uint16_t * dst  = _dst;

  while(--i)
    {
    tmp = *(src1++) + *(src2++);
    *(dst++) = GENERIC_CLIP(tmp,-32768,32767) + 32768;
    }
  }

static void sub_s32_c(const void * _src1, const void * _src2,
                      void * _dst, int num)
  {
  int64_t tmp;
  int i = num+1;
  const int32_t * src1 = _src1;
  const int32_t * src2 = _src2;
  int32_t * dst  = _dst;

  while(--i)
    {
    tmp = *(src1++) - *(src2++);
    *(dst++) = GENERIC_CLIP(tmp,-2147483648LL,2147483647LL);
    }
  }

static void sub_float_c(const void * _src1, const void * _src2,
                        void * _dst, int num)
  {
  int i = num+1;
  const float * src1 = _src1;
  const float * src2 = _src2;
  float * dst  = _dst;

  while(--i)
    {
    *(dst++) = *(src1++) - *(src2++);
    }
  }

static void sub_double_c(const void * _src1, const void * _src2,
                         void * _dst, int num)
  {
  int i = num+1;
  const double * src1 = _src1;
  const double * src2 = _src2;
  double * dst  = _dst;
  
  while(--i)
    {
    *(dst++) = *(src1++) - *(src2++);
    }
  }

static void shift_up_16_c(void * _ptr, int num, int bits)
  {
  int i = num+1;
  uint16_t * ptr  = _ptr;

  while(--i)
    {
    *ptr <<= bits;
    ptr++;
    }
  }

static void shift_down_16_c(void * _ptr, int num, int bits)
  {
  int i = num+1;
  uint16_t * ptr  = _ptr;

  while(--i)
    {
    *ptr >>= bits;
    ptr++;
    }
  
  }

static void shuffle_8_4_c(void * ptr, int num, uint8_t * mask)
  {
  union
    {
    uint8_t u8[4];
    uint32_t u32;
    } buf;

  union
    {
    uint8_t * u8;
    uint32_t * u32;
    } buf_ptr;
  
  num++;

  mask[0] &= (0x80 | 0x03);
  mask[1] &= (0x80 | 0x03);
  mask[2] &= (0x80 | 0x03);
  mask[3] &= (0x80 | 0x03);
  
  buf_ptr.u8 = ptr;
  
  while(--num)
    {
    buf.u8[0] = mask[0] & 0x80 ? 0 : buf_ptr.u8[mask[0]];
    buf.u8[1] = mask[1] & 0x80 ? 0 : buf_ptr.u8[mask[1]];
    buf.u8[2] = mask[2] & 0x80 ? 0 : buf_ptr.u8[mask[2]];
    buf.u8[3] = mask[3] & 0x80 ? 0 : buf_ptr.u8[mask[3]];
    
    *(buf_ptr.u32) = buf.u32;
    buf_ptr.u32++;
    }
  }

void gavl_dsp_init_c(gavl_dsp_funcs_t * funcs, 
                     int quality)
  {
  funcs->sad_rgb15 = sad_rgb15_c;
  funcs->sad_rgb16 = sad_rgb16_c;
  funcs->sad_8     = sad_8_c;
  funcs->sad_16    = sad_16_c;
  funcs->sad_f     = sad_f_c;

  funcs->average_rgb15 = average_rgb15_c;
  funcs->average_rgb16 = average_rgb16_c;
  funcs->average_8     = average_8_c;
  funcs->average_16    = average_16_c;
  funcs->average_f     = average_f_c;

  funcs->interpolate_rgb15 = interpolate_rgb15_c;
  funcs->interpolate_rgb16 = interpolate_rgb16_c;
  funcs->interpolate_8     = interpolate_8_c;
  funcs->interpolate_16    = interpolate_16_c;
  funcs->interpolate_f     = interpolate_f_c;
  
  funcs->bswap_16          = bswap_16_c;
  funcs->bswap_32          = bswap_32_c;
  funcs->bswap_64          = bswap_64_c;

  funcs->add_u8            = add_u8_c;
  funcs->add_s8            = add_s8_c;
  funcs->add_u8_s          = add_u8_s_c;
  funcs->add_u16           = add_u16_c;
  funcs->add_s16           = add_s16_c;
  funcs->add_u16_s         = add_u16_s_c;
  funcs->add_s32           = add_s32_c;
  funcs->add_float         = add_float_c;
  funcs->add_double        = add_double_c;

  funcs->sub_u8            = sub_u8_c;
  funcs->sub_s8            = sub_s8_c;
  funcs->sub_u8_s          = sub_u8_s_c;
  funcs->sub_u16           = sub_u16_c;
  funcs->sub_s16           = sub_s16_c;
  funcs->sub_u16_s         = sub_u16_s_c;
  funcs->sub_s32           = sub_s32_c;
  funcs->sub_float         = sub_float_c;
  funcs->sub_double        = sub_double_c;

  funcs->shift_up_16       = shift_up_16_c;
  funcs->shift_down_16     = shift_down_16_c;

  funcs->shuffle_8_4       = shuffle_8_4_c;
  
  }
