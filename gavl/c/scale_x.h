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

static void (FUNC_NAME)(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dst_start)
  {
  int i;
  uint8_t * src;

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
  
  src = ctx->src + scanline * ctx->src_stride;
  for(i = 0; i < ctx->dst_size; i++)
    {
    dst = (TYPE*)(dst_start);
    src_1 = (TYPE*)(src + ctx->offset->src_advance * ctx->table_h.pixels[i].index);
#if NUM_TAPS > 1
    src_2 = (TYPE*)((uint8_t*)(src_1) + ctx->offset->src_advance);
#endif
#if NUM_TAPS > 2
    src_3 = (TYPE*)((uint8_t*)(src_2) + ctx->offset->src_advance);
#endif
#if NUM_TAPS > 3
    src_4 = (TYPE*)((uint8_t*)(src_3) + ctx->offset->src_advance);
#endif
    
    SCALE
    
    dst_start += ctx->offset->dst_advance;
    }
  }

#ifdef INIT
#undef INIT
#endif

#undef FUNC_NAME
#undef TYPE
#undef SCALE

#undef NUM_TAPS
