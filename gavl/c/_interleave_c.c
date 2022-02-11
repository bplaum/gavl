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

static void
RENAME(interleave_none_to_all)(gavl_audio_convert_context_t * ctx)
  {
  int i, j;
  SAMPLE_TYPE * dst = (SAMPLE_TYPE*)(ctx->output_frame->samples.s_8);
  
  for(i = 0; i < ctx->input_frame->valid_samples; i++)
    {
    for(j = 0; j < ctx->input_format.num_channels; j++)
      {
      *(dst++) = SRC(j,i);
      }
    }
  }

static void
RENAME(interleave_none_to_all_stereo)(gavl_audio_convert_context_t * ctx)
  {
  int i;

  SAMPLE_TYPE * src1;
  SAMPLE_TYPE * src2;
  SAMPLE_TYPE * dst;

  src1 = &(SRC(0,0));
  src2 = &(SRC(1,0));
  dst  = &(DST(0,0));
  for(i = 0; i < ctx->input_frame->valid_samples; i++)
    {
    *(dst++) = *(src1++);
    *(dst++) = *(src2++);
    }
  }

static void
RENAME(interleave_all_to_none)(gavl_audio_convert_context_t * ctx)
  {
  int i, j;
  SAMPLE_TYPE * src = (SAMPLE_TYPE*)(ctx->input_frame->samples.s_8);

  for(i = 0; i < ctx->input_frame->valid_samples; i++)
    for(j = 0; j < ctx->input_format.num_channels; j++)
      DST(j, i) = *(src++);
  }

static void
RENAME(interleave_all_to_none_stereo)(gavl_audio_convert_context_t * ctx)
  {
  int i;
  
  SAMPLE_TYPE * src = (SAMPLE_TYPE*)(ctx->input_frame->samples.s_8);
  SAMPLE_TYPE * dst1 = (SAMPLE_TYPE*)(ctx->output_frame->channels.s_8[0]);
  SAMPLE_TYPE * dst2 = (SAMPLE_TYPE*)(ctx->output_frame->channels.s_8[1]);
  
  for(i = 0; i < ctx->input_frame->valid_samples; i++)
    {
    *(dst1++) = *(src++);
    *(dst2++) = *(src++);
    }
  }

static void RENAME(interleave_2_to_all)(gavl_audio_convert_context_t * ctx)
  {
  int i, j;
  int jmax;
  
  SAMPLE_TYPE * dst;

  dst = &(DST(0,0));
    
  jmax = ctx->input_format.num_channels/2;
  
  for(i = 0; i < ctx->input_frame->valid_samples; i++)
    {
    for(j = 0; j < jmax; j++)
      {
      *(dst++) = SRC(j*2, i*2);
      *(dst++) = SRC(j*2, i*2+1);
      }
    if(ctx->input_format.num_channels % 2)
      *(dst++) = SRC(ctx->input_format.num_channels-1, i);
    }
  }

static void RENAME(interleave_2_to_none)(gavl_audio_convert_context_t * ctx)
  {
  int i, j;
  int jmax;

  jmax = ctx->input_format.num_channels/2;

  for(i = 0; i < ctx->input_frame->valid_samples; i++)
    {
    for(j = 0; j < jmax; j++)
      {
      DST(2*j,i)   = SRC(j*2, i*2);
      DST(2*j+1,i) = SRC(j*2, i*2+1);
      }
    }
  if(ctx->input_format.num_channels % 2)
    memcpy(&(DST(ctx->input_format.num_channels-1, 0)),
           &(SRC(ctx->input_format.num_channels-1, 0)),
           ctx->input_frame->valid_samples * sizeof(SAMPLE_TYPE));
  } 

static void RENAME(interleave_all_to_2)(gavl_audio_convert_context_t * ctx)
  {
  int i, j;
  int jmax;

  jmax = ctx->input_format.num_channels/2;

  for(i = 0; i < ctx->input_frame->valid_samples; i++)
    {
    for(j = 0; j < jmax; j++)
      {
      DST(j*2, i*2)   = SRC(2*j,i);
      DST(j*2, i*2+1) = SRC(2*j+1,i);
      }
    }
  if(ctx->input_format.num_channels % 2)
    memcpy(&(DST(ctx->input_format.num_channels-1, 0)),
           &(SRC(ctx->input_format.num_channels-1, 0)),
           ctx->input_frame->valid_samples * sizeof(SAMPLE_TYPE));
  
  }

static void RENAME(interleave_none_to_2)(gavl_audio_convert_context_t * ctx)
  {
  int i, j;
  int jmax;

  jmax = ctx->input_format.num_channels/2;

  for(i = 0; i < ctx->input_frame->valid_samples; i++)
    {
    for(j = 0; j < jmax; j++)
      {
      DST(j*2, i*2)   = SRC(2*j  ,i);
      DST(j*2, i*2+1) = SRC(2*j+1,i);
      }
    if(ctx->input_format.num_channels % 2)
      {
      DST(ctx->input_format.num_channels-1, i) = 
        SRC(ctx->input_format.num_channels-1, i);
      }
    }
  
  }

