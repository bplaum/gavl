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

static void (FUNC_NAME)(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i, imax;
  int64_t tmp;
  
#if NUM_TAPS <= 0
  int j;
#endif
  uint8_t * src, *src_start;
  uint8_t * dst;

  //  if(!ctx->scanline)
  //    fprintf(stderr, "scale_y_mmx %d %d\n", ctx->src_stride);
  
  imax = (ctx->dst_size * WIDTH_MUL) / 8;
  //  imax = 0;
  
  src_start =
    ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride;

  dst = dest_start;

#if BITS == 8
    INIT_8_GLOBAL;
#else
    INIT_16_GLOBAL;
#endif
  
  for(i = 0; i < imax; i++)
    {
#if BITS == 8
    INIT_8;
#else
    INIT_16;
#endif

    src = src_start;
    
    /* Accum */
#if NUM_TAPS <= 0
    for(j = 0; j < ctx->table_v.factors_per_pixel; j++)
      {
#if BITS == 8
      ACCUM_8(j);
#else
      ACCUM_16(j);
#endif
      src += ctx->src_stride;
      }
#else

#if BITS == 8
      ACCUM_8(0);
#else
      ACCUM_16(0);
#endif

    src += ctx->src_stride;
    
#if NUM_TAPS > 1

#if BITS == 8
      ACCUM_8(1);
#else
      ACCUM_16(1);
#endif
    src += ctx->src_stride;
#endif

#if NUM_TAPS > 2

#if BITS == 8
    ACCUM_8(2);
#else
    ACCUM_16(2);
#endif

    src += ctx->src_stride;
#endif

#if NUM_TAPS > 3

#if BITS == 8
    ACCUM_8(3);
#else
    ACCUM_16(3);
#endif

    src += ctx->src_stride;
#endif
    
#endif

#if BITS == 8
    OUTPUT_8;
#else
    OUTPUT_16;
#endif
    dst += 8;
    src_start += 8;
    
    }

  ctx->need_emms = 1;
  
  imax = (ctx->dst_size * WIDTH_MUL) % 8;
  //  imax = (ctx->dst_size * WIDTH_MUL);
  
  if(!imax)
    return;
  
  for(i = 0; i < imax; i++)
    {
    src = src_start;
    tmp = 0;
    
    /* Accum */
#if NUM_TAPS <= 0
    for(j = 0; j < ctx->table_v.factors_per_pixel; j++)
      {
#if BITS == 8
      ACCUM_C_8(j);
#else
      ACCUM_C_16(j);
#endif
      src += ctx->src_stride;
      }
    
#else

#if BITS == 8
      ACCUM_C_8(0);
#else
      ACCUM_C_16(0);
#endif

    src += ctx->src_stride;
    
#if NUM_TAPS > 1
#if BITS == 8
    ACCUM_C_8(1);
#else
    ACCUM_C_16(1);
#endif

    src += ctx->src_stride;
#endif

#if NUM_TAPS > 2
#if BITS == 8
    ACCUM_C_8(2);
#else
    ACCUM_C_16(2);
#endif
    src += ctx->src_stride;
#endif

#if NUM_TAPS > 3
#if BITS == 8
    ACCUM_C_8(3);
#else
    ACCUM_C_16(3);
#endif
    src += ctx->src_stride;
#endif
    
#endif

#if BITS == 8
    OUTPUT_C_8;
#else
    OUTPUT_C_16;
#endif
    dst++;
    src_start++;
    }
  }

#undef FUNC_NAME
#undef NUM_TAPS
#undef BITS

#undef WIDTH_MUL
