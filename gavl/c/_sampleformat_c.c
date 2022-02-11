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

static void RENAME(swap_sign_8)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->U_8 =
    ctx->input_frame->U_8 ^ 0x80;
  CONVERSION_FUNC_END
  }  

static void RENAME(swap_sign_16)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->U_16 =
    ctx->input_frame->U_16 ^ 0x8000;
  CONVERSION_FUNC_END
  }

static void RENAME(s_8_to_s_16)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->S_16 =
    ctx->input_frame->S_8 * 0x0101;
  CONVERSION_FUNC_END
  }

static void RENAME(u_8_to_s_16)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->S_16 =
    (ctx->input_frame->S_8 ^ 0x80)  * 0x0101;
  CONVERSION_FUNC_END
  }



static void RENAME(s_8_to_u_16)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->S_16 =
    ((ctx->input_frame->S_8)  * 0x0101) ^ 0x8000;
  CONVERSION_FUNC_END
  }

static void RENAME(u_8_to_u_16)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
    ctx->output_frame->U_16 =
    ctx->input_frame->U_8 |
     ((uint16_t)(ctx->input_frame->U_8) << 8);
  CONVERSION_FUNC_END
  }

/* 8 -> 32 bits */

static void RENAME(s_8_to_s_32)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
    ctx->output_frame->S_32 =
    ctx->input_frame->S_8  * 0x01010101;
  CONVERSION_FUNC_END
  }

static void RENAME(u_8_to_s_32)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
    ctx->output_frame->S_32 =
    (ctx->input_frame->S_8 ^ 0x80)  * 0x01010101;
  CONVERSION_FUNC_END
  }
  
/* 16 -> 8 bits */
  
static void RENAME(convert_16_to_8_swap)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
    ctx->output_frame->U_8 =
    (ctx->input_frame->U_16 ^ 0x8000) >> 8;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_16_to_8)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
    ctx->output_frame->U_8 =
    ctx->input_frame->U_16 >> 8;
  CONVERSION_FUNC_END
  }

/* 16 -> 32 bits */

static void RENAME(s_16_to_s_32)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
    ctx->output_frame->S_32 =
    ctx->input_frame->S_16 * 0x00010001;
  CONVERSION_FUNC_END
  }

static void RENAME(u_16_to_s_32)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
    ctx->output_frame->S_32 =
    (ctx->input_frame->S_16 ^ 0x8000 ) * 0x00010001;
  CONVERSION_FUNC_END
  }

/* 32 -> 8 bits */
  
static void RENAME(convert_32_to_8_swap)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
    ctx->output_frame->S_8 =
    (ctx->input_frame->S_32 >> 24) ^ 0x80;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_32_to_8)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
    ctx->output_frame->S_8 =
    ctx->input_frame->S_32 >> 24;
  CONVERSION_FUNC_END
  }

/* 32 -> 16 bits */
  
static void RENAME(convert_32_to_16_swap)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
    ctx->output_frame->S_16 =
    (ctx->input_frame->S_32 >> 16) ^ 0x8000;
  
  CONVERSION_FUNC_END
  }

static void RENAME(convert_32_to_16)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
    ctx->output_frame->S_16 =
    ctx->input_frame->S_32 >> 16;
  CONVERSION_FUNC_END
  }

/* Int to float */

static void RENAME(convert_s8_to_float)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->FLOAT =
    (float)(ctx->input_frame->S_8)/128.0;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_u8_to_float)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->FLOAT =
    (float)(ctx->input_frame->U_8)/128.0-1.0;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_s16_to_float)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->FLOAT =
    (float)(ctx->input_frame->S_16)/32768.0;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_u16_to_float)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->FLOAT =
    (float)(ctx->input_frame->U_16)/32768.0-1.0;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_s32_to_float)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->FLOAT =
    (float)(ctx->input_frame->S_32)/2147483648.0;
  CONVERSION_FUNC_END
  }

/* Int to double */

static void RENAME(convert_s8_to_double)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->DOUBLE =
    (double)(ctx->input_frame->S_8)/128.0;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_u8_to_double)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->DOUBLE =
    (double)(ctx->input_frame->U_8)/128.0-1.0;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_s16_to_double)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->DOUBLE =
    (double)(ctx->input_frame->S_16)/32768.0;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_u16_to_double)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->DOUBLE =
    (double)(ctx->input_frame->U_16)/32768.0-1.0;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_s32_to_double)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->DOUBLE =
    (double)(ctx->input_frame->S_32)/2147483648.0;
  CONVERSION_FUNC_END
  }

/* Float to int */

static void RENAME(convert_float_to_s8)(gavl_audio_convert_context_t * ctx)
  {
  long tmp;
  CONVERSION_FUNC_START
  tmp = lrintf(ctx->input_frame->FLOAT * 128.0);
  CLAMP(tmp, -128, 127);
  ctx->output_frame->S_8 = tmp;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_float_to_u8)(gavl_audio_convert_context_t * ctx)
  {
  long tmp;
  CONVERSION_FUNC_START
  tmp = lrintf((ctx->input_frame->FLOAT+1.0) * 128.0);
  CLAMP(tmp, 0, 255);
  ctx->output_frame->U_8 = tmp;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_float_to_s16)(gavl_audio_convert_context_t * ctx)
  {
  long tmp;
  CONVERSION_FUNC_START
  tmp = lrintf((ctx->input_frame->FLOAT) * 32768.0);
  CLAMP(tmp, -32768, 32767);
  ctx->output_frame->S_16 = tmp;

  CONVERSION_FUNC_END
  }

static void RENAME(convert_float_to_u16)(gavl_audio_convert_context_t * ctx)
  {
  long tmp;
  CONVERSION_FUNC_START
  tmp = lrintf((ctx->input_frame->FLOAT+1.0) * 32768.0);
  CLAMP(tmp, 0, 65535);
  ctx->output_frame->U_16 = tmp;

  CONVERSION_FUNC_END
  }

static void RENAME(convert_float_to_s32)(gavl_audio_convert_context_t * ctx)
  {
  int64_t tmp;
  CONVERSION_FUNC_START
  tmp = llrintf((ctx->input_frame->FLOAT) * 2147483648.0);
  CLAMP(tmp, -2147483648LL, 2147483647LL);
  ctx->output_frame->S_32 = tmp;
  CONVERSION_FUNC_END
  }

/* Double to int */

static void RENAME(convert_double_to_s8)(gavl_audio_convert_context_t * ctx)
  {
  long tmp;
  CONVERSION_FUNC_START
  tmp = lrint(ctx->input_frame->DOUBLE * 128.0);
  CLAMP(tmp, -128, 127);
  ctx->output_frame->S_8 = tmp;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_double_to_u8)(gavl_audio_convert_context_t * ctx)
  {
  long tmp;
  CONVERSION_FUNC_START
  tmp = lrint((ctx->input_frame->DOUBLE+1.0) * 128.0);
  CLAMP(tmp, 0, 255);
  ctx->output_frame->U_8 = tmp;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_double_to_s16)(gavl_audio_convert_context_t * ctx)
  {
  long tmp;
  CONVERSION_FUNC_START
  tmp = lrint((ctx->input_frame->DOUBLE) * 32768.0);
  CLAMP(tmp, -32768, 32767);
  ctx->output_frame->S_16 = tmp;

  CONVERSION_FUNC_END
  }

static void RENAME(convert_double_to_u16)(gavl_audio_convert_context_t * ctx)
  {
  long tmp;
  CONVERSION_FUNC_START
  tmp = lrint((ctx->input_frame->DOUBLE+1.0) * 32768.0);
  CLAMP(tmp, 0, 65535);
  ctx->output_frame->U_16 = tmp;

  CONVERSION_FUNC_END
  }

static void RENAME(convert_double_to_s32)(gavl_audio_convert_context_t * ctx)
  {
  int64_t tmp;
  CONVERSION_FUNC_START
  tmp = llrint((ctx->input_frame->DOUBLE) * 2147483648.0);
  CLAMP(tmp, -2147483648LL, 2147483647LL);
  ctx->output_frame->S_32 = tmp;
  CONVERSION_FUNC_END
  }

/* float <-> double */

static void RENAME(convert_double_to_float)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->FLOAT = ctx->input_frame->DOUBLE;
  CONVERSION_FUNC_END
  }

static void RENAME(convert_float_to_double)(gavl_audio_convert_context_t * ctx)
  {
  CONVERSION_FUNC_START
  ctx->output_frame->DOUBLE = ctx->input_frame->FLOAT;
  CONVERSION_FUNC_END
  }


static void RENAME(gavl_init_sampleformat_funcs_c)(gavl_sampleformat_table_t * t)
  {
  t->swap_sign_8 = RENAME(swap_sign_8);
  t->swap_sign_16 = RENAME(swap_sign_16);
  
  /* 8 -> 16 bits */

  t->s_8_to_s_16 = RENAME(s_8_to_s_16);
  t->u_8_to_s_16 = RENAME(u_8_to_s_16);

  t->s_8_to_u_16 = RENAME(s_8_to_u_16);
  t->u_8_to_u_16 = RENAME(u_8_to_u_16);

  /* 8 -> 32 bits */

  t->s_8_to_s_32 = RENAME(s_8_to_s_32);
  t->u_8_to_s_32 = RENAME(u_8_to_s_32);
  
  /* 16 -> 8 bits */
  
  t->convert_16_to_8_swap = RENAME(convert_16_to_8_swap);
  t->convert_16_to_8 = RENAME(convert_16_to_8);

  /* 16 -> 32 bits */

  t->s_16_to_s_32 = RENAME(s_16_to_s_32);
  t->u_16_to_s_32 = RENAME(u_16_to_s_32);
  
  /* 32 -> 8 bits */
  
  t->convert_32_to_8_swap = RENAME(convert_32_to_8_swap);
  t->convert_32_to_8 =      RENAME(convert_32_to_8);

  /* 32 -> 16 bits */
  
  t->convert_32_to_16_swap = RENAME(convert_32_to_16_swap);
  t->convert_32_to_16 =      RENAME(convert_32_to_16);
  
  /* Int to Float  */

  t->convert_s8_to_float = RENAME(convert_s8_to_float);
  t->convert_u8_to_float = RENAME(convert_u8_to_float);

  t->convert_s16_to_float = RENAME(convert_s16_to_float);
  t->convert_u16_to_float = RENAME(convert_u16_to_float);

  t->convert_s32_to_float = RENAME(convert_s32_to_float);

  /* Float to int */
  
  t->convert_float_to_s8 = RENAME(convert_float_to_s8);
  t->convert_float_to_u8 = RENAME(convert_float_to_u8);

  t->convert_float_to_s16 = RENAME(convert_float_to_s16);
  t->convert_float_to_u16 = RENAME(convert_float_to_u16);

  t->convert_float_to_s32 = RENAME(convert_float_to_s32);

  /* Int to Double  */

  t->convert_s8_to_double = RENAME(convert_s8_to_double);
  t->convert_u8_to_double = RENAME(convert_u8_to_double);

  t->convert_s16_to_double = RENAME(convert_s16_to_double);
  t->convert_u16_to_double = RENAME(convert_u16_to_double);

  t->convert_s32_to_double = RENAME(convert_s32_to_double);

  /* Double to int */
  
  t->convert_double_to_s8 = RENAME(convert_double_to_s8);
  t->convert_double_to_u8 = RENAME(convert_double_to_u8);

  t->convert_double_to_s16 = RENAME(convert_double_to_s16);
  t->convert_double_to_u16 = RENAME(convert_double_to_u16);

  t->convert_double_to_s32 = RENAME(convert_double_to_s32);

  t->convert_float_to_double = RENAME(convert_float_to_double);
  t->convert_double_to_float = RENAME(convert_double_to_float);
  
  }
