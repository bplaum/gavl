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
  int i;

#ifdef NO_UINT8
  uint8_t * _src_1
#if NUM_TAPS > 1
    , * _src_2
#endif
#if NUM_TAPS > 2
    , * _src_3
#endif
#if NUM_TAPS > 3
    , * _src_4
#endif
    ;

#else
#define _src_1 src_1
#define _src_2 src_2
#define _src_3 src_3
#define _src_4 src_4
#endif

  TYPE * src_1,
#if NUM_TAPS > 1
  * src_2,
#endif
#if NUM_TAPS > 2
  * src_3,
#endif
#if NUM_TAPS > 3
  * src_4,
#endif
  *dst;

#ifdef INIT
  INIT
#endif
  
  _src_1 = ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride;
#if NUM_TAPS > 1
  _src_2 = _src_1 + ctx->src_stride;
#endif
#if NUM_TAPS > 2
  _src_3 = _src_2 + ctx->src_stride;
#endif
#if NUM_TAPS > 3
  _src_4 = _src_3 + ctx->src_stride;
#endif
  
  for(i = 0; i < ctx->dst_size; i++)
    {
    dst = (TYPE*)(dest_start);
#ifdef NO_UINT8  
    src_1 = (TYPE*)(_src_1);
#if NUM_TAPS > 1
    src_2 = (TYPE*)(_src_2);
#endif
#if NUM_TAPS > 2
    src_3 = (TYPE*)(_src_3);
#endif
#if NUM_TAPS > 3
    src_4 = (TYPE*)(_src_4);
#endif
    
#endif
    
    SCALE
    
    dest_start += ctx->offset->dst_advance;
    _src_1 += ctx->offset->src_advance;
#if NUM_TAPS > 1
    _src_2 += ctx->offset->src_advance;
#endif
#if NUM_TAPS > 2
    _src_3 += ctx->offset->src_advance;
#endif
#if NUM_TAPS > 3
    _src_4 += ctx->offset->src_advance;
#endif
    }
  }

#ifdef INIT
#undef INIT
#endif

#undef FUNC_NAME
#undef TYPE
#undef SCALE

#ifdef _src_1
#undef _src_1
#endif

#ifdef _src_2
#undef _src_2
#endif

#ifdef _src_3
#undef _src_3
#endif

#ifdef _src_4
#undef _src_4
#endif

#ifdef NO_UINT8
#undef NO_UINT8
#endif
