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

/* SSE Optimized scaling */

static void (FUNC_NAME)(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i, imax;
  uint8_t * src;
  uint8_t * src_start;
  uint8_t * dst;

#if NUM_TAPS <= 0
  int j;
#endif
#ifdef INIT_GLOBAL
  INIT_GLOBAL;
#endif

  dst = dest_start;

  src_start =
    ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride;
  
  /* While source is not aligned... */
  imax = (((long)(src_start)) % 16)/4;
  //  imax = ctx->dst_size * BYTES * WIDTH_MUL;

  for(i = 0; i < imax; i++)
    {
#ifdef INIT_C
    INIT_C;
#endif
    src = src_start;
    
    /* Accum */
#if NUM_TAPS <= 0
    for(j = 0; j < ctx->table_v.factors_per_pixel; j++)
      {
      ACCUM_C(j);
      src += ctx->src_stride;
      }
    
#else// NUM_TAPS > 0

    ACCUM_C(0);
    src += ctx->src_stride;
    
#if NUM_TAPS > 1
    ACCUM_C(1);
    src += ctx->src_stride;
#endif

#if NUM_TAPS > 2
    ACCUM_C(2);
    src += ctx->src_stride;
#endif

#if NUM_TAPS > 3
    ACCUM_C(3);
    src += ctx->src_stride;
#endif
    
#endif // NUM_TAPS > 0

    OUTPUT_C;
    dst+=4;
    src_start+=4;
    }

  /* SSE routines scale 8 numbers (= 32 bytes) at once */
  imax = (ctx->dst_size * 4 * WIDTH_MUL - (dst - dest_start)) / /* Bytes left */
          (32); /* Bytes processed at once */
          
  for(i = 0; i < imax; i++)
    {
    INIT;

    src = src_start;
    
    /* Accum */
#if NUM_TAPS <= 0
    for(j = 0; j < ctx->table_v.factors_per_pixel; j++)
      {
      ACCUM(j);
      src += ctx->src_stride;
      }
#else

    ACCUM(0);

    src += ctx->src_stride;
    
#if NUM_TAPS > 1
    ACCUM(1);
    src += ctx->src_stride;
#endif

#if NUM_TAPS > 2
    ACCUM(2);
    src += ctx->src_stride;
#endif

#if NUM_TAPS > 3

    ACCUM(3);
    src += ctx->src_stride;
#endif
    
#endif
    
    OUTPUT;
    dst += 32;
    src_start += 32;
    
    }

  imax = (ctx->dst_size * 4 * WIDTH_MUL - (dst - dest_start)) / 4;
          
  //  imax = (ctx->dst_size * WIDTH_MUL);
  
  if(!imax)
    return;
  
  for(i = 0; i < imax; i++)
    {
    src = src_start;

#ifdef INIT_C
    INIT_C;
#endif
    
    /* Accum */
#if NUM_TAPS <= 0
    for(j = 0; j < ctx->table_v.factors_per_pixel; j++)
      {
      ACCUM_C(j);
      src += ctx->src_stride;
      }
    
#else

    ACCUM_C(0);
      
    src += ctx->src_stride;
    
#if NUM_TAPS > 1
    ACCUM_C(1);
    src += ctx->src_stride;
#endif

#if NUM_TAPS > 2
    ACCUM_C(2);
    src += ctx->src_stride;
#endif

#if NUM_TAPS > 3
    ACCUM_C(3);
    src += ctx->src_stride;
#endif
    
#endif

    OUTPUT_C;
    dst+=4;
    src_start+=4;
    }
  }

#undef FUNC_NAME
#undef NUM_TAPS
#undef WIDTH_MUL
#undef ACCUM
#undef ACCUM_C
#undef OUTPUT
#undef OUTPUT_C

#ifdef INIT_GLOBAL
#undef INIT_GLOBAL
#endif

#ifdef INIT_C
#undef INIT_C
#endif

#undef INIT

