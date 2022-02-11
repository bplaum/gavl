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

/* Planar -> Packed conversion */

/*
 *  Needs the following macros:
 *  IN_TYPE:        Type of the input pointers
 *  OUT_TYPE:       Type of the output pointers
 *  IN_ADVANCE_Y:   Input advance
 *  IN_ADVANCE_UV:  Input advance
 *  OUT_ADVANCE:    Output advance
 *  FUNC_NAME:      Name of the function
 *  NUM_PIXELS:     The number of pixels the conversion processes at once
 *  CONVERT:        Makes the appropriate conversion
 *                  from <src> to <dst> for luma and chroma
 *  CHROMA_SUB:     Vertical chroma subsampling factor
 *  INIT:        Variable declarations and initialization (Optional)
 *  CLEANUP:     Stuff at the end (Optional)
 */

static void (FUNC_NAME)(gavl_video_convert_context_t * ctx)
  {
#ifndef SCANLINE
  int i, imax;
  uint8_t * dst_save;
  uint8_t * src_save_y;
  uint8_t * src_save_u;
  uint8_t * src_save_v;
#endif
  int j, jmax;
  OUT_TYPE  * dst;
  IN_TYPE * src_y;
  IN_TYPE * src_u;
  IN_TYPE * src_v;

#ifdef INIT
  INIT
#endif

#ifndef SCANLINE
  dst_save = ctx->output_frame->planes[0];
  src_save_y = ctx->input_frame->planes[0];
  src_save_u = ctx->input_frame->planes[1];
  src_save_v = ctx->input_frame->planes[2];
#else
  dst   = (OUT_TYPE*)ctx->output_frame->planes[0];
  src_y = (IN_TYPE*)ctx->input_frame->planes[0];
  src_u = (IN_TYPE*)ctx->input_frame->planes[1];
  src_v = (IN_TYPE*)ctx->input_frame->planes[2];
#endif
  
  jmax = ctx->input_format.image_width  / NUM_PIXELS;
#ifndef SCANLINE
  imax = ctx->input_format.image_height / CHROMA_SUB;
  for(i = 0; i < imax; i++)
    {
    dst =   (OUT_TYPE*)dst_save;
    src_y = (IN_TYPE*)src_save_y;
    src_u = (IN_TYPE*)src_save_u;
    src_v = (IN_TYPE*)src_save_v;
#endif /* !SCANLINE */
    
GAVL_LOOP_HEAD(j, jmax)
      CONVERT
      dst += OUT_ADVANCE;
      src_y += IN_ADVANCE_Y;
      src_u += IN_ADVANCE_UV;
      src_v += IN_ADVANCE_UV;
GAVL_LOOP_TAIL
  
#ifndef SCANLINE
    src_save_y += ctx->input_frame->strides[0];
    dst_save += ctx->output_frame->strides[0];
    
#if CHROMA_SUB > 1
    dst =   (OUT_TYPE*)dst_save;
    src_y = (IN_TYPE*)src_save_y;
    src_u = (IN_TYPE*)src_save_u;
    src_v = (IN_TYPE*)src_save_v;
    
GAVL_LOOP_HEAD(j, jmax)
      CONVERT
      dst += OUT_ADVANCE;
      src_y += IN_ADVANCE_Y;
      src_u += IN_ADVANCE_UV;
      src_v += IN_ADVANCE_UV;
GAVL_LOOP_TAIL

    src_save_y += ctx->input_frame->strides[0];
    dst_save += ctx->output_frame->strides[0];
#endif

#if CHROMA_SUB > 2
    dst =   (OUT_TYPE*)dst_save;
    src_y = (IN_TYPE*)src_save_y;
    src_u = (IN_TYPE*)src_save_u;
    src_v = (IN_TYPE*)src_save_v;
    
GAVL_LOOP_HEAD(j, jmax)
      CONVERT
      dst += OUT_ADVANCE;
      src_y += IN_ADVANCE_Y;
      src_u += IN_ADVANCE_UV;
      src_v += IN_ADVANCE_UV;
GAVL_LOOP_TAIL
    src_save_y += ctx->input_frame->strides[0];
    dst_save += ctx->output_frame->strides[0];
#endif

#if CHROMA_SUB > 3
    dst =   (OUT_TYPE*)dst_save;
    src_y = (IN_TYPE*)src_save_y;
    src_u = (IN_TYPE*)src_save_u;
    src_v = (IN_TYPE*)src_save_v;
    
GAVL_LOOP_HEAD(j, jmax)
      CONVERT
      dst += OUT_ADVANCE;
      src_y += IN_ADVANCE_Y;
      src_u += IN_ADVANCE_UV;
      src_v += IN_ADVANCE_UV;
GAVL_LOOP_TAIL

    src_save_y += ctx->input_frame->strides[0];
    dst_save += ctx->output_frame->strides[0];
#endif
    src_save_u += ctx->input_frame->strides[1];
    src_save_v += ctx->input_frame->strides[2];
    }
#endif /* !SCANLINE */

#ifdef CLEANUP
  CLEANUP
#endif
  
  }

#undef FUNC_NAME      // Name of the function
#undef IN_TYPE        // Type of the input pointers
#undef OUT_TYPE       // Type of the output pointers
#undef IN_ADVANCE_Y   // Input advance
#undef IN_ADVANCE_UV  // Input advance
#undef OUT_ADVANCE    // Output advance
#undef NUM_PIXELS     // The number of pixels the conversion processes at once
#undef CONVERT        // Makes the appropriate conversion
                      // from <src> to <dst> for luma and chroma
#undef CHROMA_SUB     // Vertical chroma subsampling factor

#ifdef INIT
#undef INIT
#endif

#ifdef CLEANUP
#undef CLEANUP
#endif
