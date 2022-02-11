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

/* Packed -> Packed conversion */

/*
 *  Needs the following macros:
 *  IN_TYPE:     Type of the input pointer
 *  OUT_TYPE:    Type of the output pointer
 *  IN_ADVANCE:  Input advance
 *  OUT_ADVANCE: Output advance
 *  FUNC_NAME:   Name of the function
 *  NUM_PIXELS:  The number of pixels the conversion processes at once
 *  CONVERT:     This macro takes no arguments and makes the appropriate conversion
 *               from <src> to <dst>
 *  INIT:        Variable declarations and initialization (Optional)
 *  CLEANUP:     Stuff at the end (Optional)
 */

static void (FUNC_NAME)(gavl_video_convert_context_t * ctx)
  {
  int i;
  int j, jmax;
  IN_TYPE  * src;
  OUT_TYPE * dst;
  uint8_t * src_save;
  uint8_t * dst_save;
#ifdef INIT
  INIT
#endif
  src_save = ctx->input_frame->planes[0];
  dst_save = ctx->output_frame->planes[0];
  jmax = ctx->input_format.image_width / NUM_PIXELS;
  for(i = 0; i < ctx->input_format.image_height; i++)
    {
    src = (IN_TYPE*)src_save;
    dst = (OUT_TYPE*)dst_save;

    j = jmax + 1;
    while(--j)
      {
      CONVERT
      src += IN_ADVANCE;
      dst += OUT_ADVANCE;
      }
    src_save += ctx->input_frame->strides[0];
    dst_save += ctx->output_frame->strides[0];
    }

#ifdef CLEANUP
  CLEANUP
#endif

  }

#undef IN_TYPE 
#undef OUT_TYPE
#undef IN_ADVANCE
#undef OUT_ADVANCE
#undef NUM_PIXELS
#undef FUNC_NAME
#undef CONVERT

#ifdef INIT
#undef INIT
#endif

#ifdef CLEANUP
#undef CLEANUP
#endif
