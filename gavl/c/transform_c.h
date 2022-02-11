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

static void (FUNC_NAME)(gavl_transform_context_t * ctx, gavl_transform_pixel_t * pixels, uint8_t * dest_start)
  {
  int i;

  TYPE * src_0,
#if NUM_TAPS > 1
  * src_1,
#endif
#if NUM_TAPS > 2
  * src_2,
#endif
#if NUM_TAPS > 3
  * src_3,
#endif
  *dst;

  gavl_transform_pixel_t * pixel;
  
#ifdef INIT
  INIT
#endif

  pixel = pixels;    

  i = ctx->dst_width+1;
  
  while(--i)
    {
    if(!pixel->outside)
      {
      dst = (TYPE*)(dest_start);
      src_0 = (TYPE*)(ctx->src + ctx->advance * pixel->index_x +
                      ctx->src_stride * pixel->index_y);
#if NUM_TAPS > 1
      src_1 = (TYPE*)((uint8_t*)(src_0) + ctx->src_stride);
#endif
#if NUM_TAPS > 2
      src_2 = (TYPE*)((uint8_t*)(src_1) + ctx->src_stride);
#endif
#if NUM_TAPS > 3
      src_3 = (TYPE*)((uint8_t*)(src_2) + ctx->src_stride);
#endif
      TRANSFORM
      }
    dest_start += ctx->advance;
    pixel++;
    }
#ifdef FINISH
  FINISH
#endif
  }

#ifdef INIT
#undef INIT
#endif

#ifdef FINISH
#undef FINISH
#endif

#undef FUNC_NAME
#undef TYPE
#undef TRANSFORM

// #undef NUM_TAPS
