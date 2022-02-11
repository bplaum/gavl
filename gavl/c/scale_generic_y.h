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
  int i, j;
  uint8_t * _src;

  TYPE *dst, *src;
  
#ifdef INIT
  INIT
#endif
    
  for(i = 0; i < ctx->dst_size; i++)
    {
    dst = (TYPE*)(dest_start);

    SCALE_INIT

    _src = ctx->src + ctx->src_stride * ctx->table_v.pixels[scanline].index +
      i * ctx->offset->src_advance;
    for(j = 0; j < ctx->table_v.factors_per_pixel; j++)
      {
      src = (TYPE*)_src;

      SCALE_ACCUM
      _src += ctx->src_stride;
      }
    
    SCALE_FINISH
    
    dest_start += ctx->offset->dst_advance;
    }
  }

#ifdef INIT
#undef INIT
#endif

#undef FUNC_NAME
#undef TYPE

#undef SCALE_INIT
#undef SCALE_ACCUM
#undef SCALE_FINISH

