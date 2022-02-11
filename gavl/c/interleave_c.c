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
#include <string.h>
#include <stdio.h>

#include <gavl.h>
#include <audio.h>
#include <interleave.h>


#define SAMPLE_TYPE uint8_t
#define RENAME(a) a ## _8
#define SRC(c,s) (ctx->input_frame->channels.u_8[c][s])
#define DST(c,s) (ctx->output_frame->channels.u_8[c][s])
#include "_interleave_c.c"

#undef SAMPLE_TYPE
#undef RENAME
#undef SRC
#undef DST

#define SAMPLE_TYPE uint16_t
#define RENAME(a) a ## _16
#define SRC(c,s) (ctx->input_frame->channels.u_16[c][s])
#define DST(c,s) (ctx->output_frame->channels.u_16[c][s])
#include "_interleave_c.c"

#undef SAMPLE_TYPE
#undef RENAME
#undef SRC
#undef DST

#define SAMPLE_TYPE uint32_t
#define RENAME(a) a ## _32
#define SRC(c,s) (ctx->input_frame->channels.u_32[c][s])
#define DST(c,s) (ctx->output_frame->channels.u_32[c][s])
#include "_interleave_c.c"

#undef SAMPLE_TYPE
#undef RENAME
#undef SRC
#undef DST

#define SAMPLE_TYPE double
#define RENAME(a) a ## _64
#define SRC(c,s) (ctx->input_frame->channels.d[c][s])
#define DST(c,s) (ctx->output_frame->channels.d[c][s])
#include "_interleave_c.c"

#undef SAMPLE_TYPE
#undef RENAME
#undef SRC
#undef DST



void gavl_init_interleave_funcs_c(gavl_interleave_table_t * t)
  {
  /* 8 bit versions */
  
  t->interleave_none_to_all_8        = interleave_none_to_all_8;
  t->interleave_none_to_all_stereo_8 = interleave_none_to_all_stereo_8;

  t->interleave_all_to_none_8        = interleave_all_to_none_8;
  t->interleave_all_to_none_stereo_8 = interleave_all_to_none_stereo_8;

  t->interleave_2_to_all_8           = interleave_2_to_all_8;
  t->interleave_2_to_none_8          = interleave_2_to_none_8;

  t->interleave_all_to_2_8           = interleave_all_to_2_8;
  t->interleave_none_to_2_8          = interleave_none_to_2_8;

  /* 16 bit versions */
  
  t->interleave_none_to_all_16        = interleave_none_to_all_16;
  t->interleave_none_to_all_stereo_16 = interleave_none_to_all_stereo_16;

  t->interleave_all_to_none_16        = interleave_all_to_none_16;
  t->interleave_all_to_none_stereo_16 = interleave_all_to_none_stereo_16;

  t->interleave_2_to_all_16           = interleave_2_to_all_16;
  t->interleave_2_to_none_16          = interleave_2_to_none_16;

  t->interleave_all_to_2_16           = interleave_all_to_2_16;
  t->interleave_none_to_2_16          = interleave_none_to_2_16;

  /* 32 bit versions */
  
  t->interleave_none_to_all_32        = interleave_none_to_all_32;
  t->interleave_none_to_all_stereo_32 = interleave_none_to_all_stereo_32;

  t->interleave_all_to_none_32        = interleave_all_to_none_32;
  t->interleave_all_to_none_stereo_32 = interleave_all_to_none_stereo_32;

  t->interleave_2_to_all_32           = interleave_2_to_all_32;
  t->interleave_2_to_none_32          = interleave_2_to_none_32;

  t->interleave_all_to_2_32           = interleave_all_to_2_32;
  t->interleave_none_to_2_32          = interleave_none_to_2_32;

  /* 64 bit versions */
  
  t->interleave_none_to_all_64        = interleave_none_to_all_64;
  t->interleave_none_to_all_stereo_64 = interleave_none_to_all_stereo_64;

  t->interleave_all_to_none_64        = interleave_all_to_none_64;
  t->interleave_all_to_none_stereo_64 = interleave_all_to_none_stereo_64;

  t->interleave_2_to_all_64           = interleave_2_to_all_64;
  t->interleave_2_to_none_64          = interleave_2_to_none_64;

  t->interleave_all_to_2_64           = interleave_all_to_2_64;
  t->interleave_none_to_2_64          = interleave_none_to_2_64;

  
  }
