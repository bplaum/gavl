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
  uint8_t * _src_1, * _src_2;
#else
#define _src_1 src_1
#define _src_2 src_2
#endif

  TYPE * src_1, *src_2, *dst;
#ifdef INIT
  INIT
#endif
  
  _src_1 = ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride;
  _src_2 = _src_1 + ctx->src_stride;
  for(i = 0; i < ctx->dst_size; i++)
    {
    dst = (TYPE*)(dest_start);
#ifdef NO_UINT8  
    src_1 = (TYPE*)(_src_1);
    src_2 = (TYPE*)(_src_2);
#endif
    
    SCALE
    
    dest_start += ctx->offset->dst_advance;
    _src_1 += ctx->offset->src_advance;
    _src_2 += ctx->offset->src_advance;
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

#ifdef NO_UINT8
#undef NO_UINT8
#endif
