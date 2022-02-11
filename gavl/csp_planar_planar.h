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

/* Planar -> Planar conversion */

/*
 *  Needs the following macros:
 *  IN_TYPE:        Type of the input pointers
 *  OUT_TYPE:       Type of the output pointers
 *  IN_ADVANCE_Y:   Input advance
 *  IN_ADVANCE_UV:  Input advance
 *  OUT_ADVANCE_Y:  Output advance
 *  OUT_ADVANCE_UV: Output advance
 *  FUNC_NAME:      Name of the function
 *  NUM_PIXELS:     The number of pixels the conversion processes at once
 *  CONVERT_YUV:    Makes the appropriate conversion
 *                  from <src> to <dst> for luma and chroma in output
 *  CONVERT_Y:      Makes the appropriate conversion
 *                  from <src> to <dst> for luma only in output
 *  CHROMA_SUB_IN:  Vertical chroma subsampling factor for input format
 *  CHROMA_SUB_OUT: Vertical chroma subsampling factor for output format
 *  INIT:        Variable declarations and initialization (Optional)
 *  CLEANUP:     Stuff at the end (Optional)
 */

static void (FUNC_NAME)(gavl_video_convert_context_t * ctx)
  {
  int i, imax, in_row_counter = 0;
  uint8_t * dst_save_y;
  uint8_t * dst_save_u;
  uint8_t * dst_save_v;
  
  uint8_t * src_save_y;
  uint8_t * src_save_u;
  uint8_t * src_save_v;

  int j, jmax;
  OUT_TYPE  * dst_y;
  OUT_TYPE  * dst_u;
  OUT_TYPE  * dst_v;
 
  IN_TYPE * src_y;
  IN_TYPE * src_u;
  IN_TYPE * src_v;

#ifdef INIT
  INIT
#endif

  dst_save_y = ctx->output_frame->planes[0];
  dst_save_u = ctx->output_frame->planes[1];
  dst_save_v = ctx->output_frame->planes[2];

  src_save_y = ctx->input_frame->planes[0];
  src_save_u = ctx->input_frame->planes[1];
  src_save_v = ctx->input_frame->planes[2];

  dst_y = (OUT_TYPE*)ctx->output_frame->planes[0];
  dst_u = (OUT_TYPE*)ctx->output_frame->planes[1];
  dst_v = (OUT_TYPE*)ctx->output_frame->planes[2];
 
  src_y = (IN_TYPE*)ctx->input_frame->planes[0];
  src_u = (IN_TYPE*)ctx->input_frame->planes[1];
  src_v = (IN_TYPE*)ctx->input_frame->planes[2];
  
  jmax = ctx->input_format.image_width  / NUM_PIXELS;

  imax = ctx->input_format.image_height / CHROMA_SUB_OUT;
  for(i = 0; i < imax; i++)
    {
    dst_y = (OUT_TYPE*)dst_save_y;
    dst_u = (OUT_TYPE*)dst_save_u;
    dst_v = (OUT_TYPE*)dst_save_v;
    src_y = (IN_TYPE*)src_save_y;
    src_u = (IN_TYPE*)src_save_u;
    src_v = (IN_TYPE*)src_save_v;
    
GAVL_LOOP_HEAD(j, jmax)
      CONVERT_YUV
      dst_y += OUT_ADVANCE_Y;
      dst_u += OUT_ADVANCE_UV;
      dst_v += OUT_ADVANCE_UV;
      
      src_y += IN_ADVANCE_Y;
      src_u += IN_ADVANCE_UV;
      src_v += IN_ADVANCE_UV;
GAVL_LOOP_TAIL
    
    src_save_y += ctx->input_frame->strides[0];
    dst_save_y += ctx->output_frame->strides[0];

    in_row_counter++;
    if(in_row_counter == CHROMA_SUB_IN)
      {
      in_row_counter = 0;
      src_save_u += ctx->input_frame->strides[1];
      src_save_v += ctx->input_frame->strides[2];
      }
    
#if CHROMA_SUB_OUT > 1
#ifdef CONVERT_Y
    dst_y = (OUT_TYPE*)dst_save_y;
    dst_u = (OUT_TYPE*)dst_save_u;
    dst_v = (OUT_TYPE*)dst_save_v;
    src_y = (IN_TYPE*)src_save_y;
    src_u = (IN_TYPE*)src_save_u;
    src_v = (IN_TYPE*)src_save_v;
    
GAVL_LOOP_HEAD(j, jmax)
      CONVERT_Y
      dst_y += OUT_ADVANCE_Y;
      dst_u += OUT_ADVANCE_UV;
      dst_v += OUT_ADVANCE_UV;
      
      src_y += IN_ADVANCE_Y;
      src_u += IN_ADVANCE_UV;
      src_v += IN_ADVANCE_UV;
GAVL_LOOP_TAIL

#endif    
    src_save_y += ctx->input_frame->strides[0];
    dst_save_y += ctx->output_frame->strides[0];

    in_row_counter++;
    if(in_row_counter == CHROMA_SUB_IN)
      {
      in_row_counter = 0;
      src_save_u += ctx->input_frame->strides[1];
      src_save_v += ctx->input_frame->strides[2];
      }

#endif

#if CHROMA_SUB_OUT > 2
#ifdef CONVERT_Y
    dst_y = (OUT_TYPE*)dst_save_y;
    dst_u = (OUT_TYPE*)dst_save_u;
    dst_v = (OUT_TYPE*)dst_save_v;
    src_y = (IN_TYPE*)src_save_y;
    src_u = (IN_TYPE*)src_save_u;
    src_v = (IN_TYPE*)src_save_v;
    
GAVL_LOOP_HEAD(j, jmax)
      CONVERT_Y
      dst_y += OUT_ADVANCE_Y;
      dst_u += OUT_ADVANCE_UV;
      dst_v += OUT_ADVANCE_UV;
      
      src_y += IN_ADVANCE_Y;
      src_u += IN_ADVANCE_UV;
      src_v += IN_ADVANCE_UV;
GAVL_LOOP_TAIL

#endif
    src_save_y += ctx->input_frame->strides[0];
    dst_save_y += ctx->output_frame->strides[0];

    in_row_counter++;
    if(in_row_counter == CHROMA_SUB_IN)
      {
      in_row_counter = 0;
      src_save_u += ctx->input_frame->strides[1];
      src_save_v += ctx->input_frame->strides[2];
      }

#endif

#if CHROMA_SUB_OUT > 3
#ifdef CONVERT_Y
    dst_y = (OUT_TYPE*)dst_save_y;
    dst_u = (OUT_TYPE*)dst_save_u;
    dst_v = (OUT_TYPE*)dst_save_v;
    src_y = (IN_TYPE*)src_save_y;
    src_u = (IN_TYPE*)src_save_u;
    src_v = (IN_TYPE*)src_save_v;
    
GAVL_LOOP_HEAD(j, jmax)
      CONVERT_Y
      dst_y += OUT_ADVANCE_Y;
      dst_u += OUT_ADVANCE_UV;
      dst_v += OUT_ADVANCE_UV;
      
      src_y += IN_ADVANCE_Y;
      src_u += IN_ADVANCE_UV;
      src_v += IN_ADVANCE_UV;
GAVL_LOOP_TAIL
  
#endif
    src_save_y += ctx->input_frame->strides[0];
    dst_save_y += ctx->output_frame->strides[0];

    in_row_counter++;
    if(in_row_counter == CHROMA_SUB_IN)
      {
      in_row_counter = 0;
      src_save_u += ctx->input_frame->strides[1];
      src_save_v += ctx->input_frame->strides[2];
      }

#endif
    dst_save_u += ctx->output_frame->strides[1];
    dst_save_v += ctx->output_frame->strides[2];
    }

#ifdef CLEANUP
  CLEANUP
#endif
  
  }

#undef FUNC_NAME      // Name of the function
#undef IN_TYPE        // Type of the input pointers
#undef OUT_TYPE       // Type of the output pointers
#undef IN_ADVANCE_Y   // Input advance
#undef IN_ADVANCE_UV  // Input advance
#undef OUT_ADVANCE_Y  // Output advance
#undef OUT_ADVANCE_UV // Output advance
#undef NUM_PIXELS     // The number of pixels the conversion processes at once
#undef CONVERT_YUV    // Makes the appropriate conversion
                      // from <src> to <dst> for luma and chroma
#ifdef CONVERT_Y
#undef CONVERT_Y
#endif

#undef CHROMA_SUB_IN     // Vertical chroma subsampling factor
#undef CHROMA_SUB_OUT     // Vertical chroma subsampling factor
#ifdef INIT
#undef INIT
#endif

#ifdef CLEANUP
#undef CLEANUP
#endif
