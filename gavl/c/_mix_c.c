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

static void RENAME(mix_1_to_1)(gavl_mix_output_channel_t * channel,
                               const gavl_audio_frame_t * input_frame,
                               gavl_audio_frame_t * output_frame)
  {
  int i;
  TMP_TYPE tmp;

  SAMPLE_TYPE factor1 = FACTOR(0);
  
  i = input_frame->valid_samples;
  
  while(i--)
    {
    tmp = (TMP_TYPE)SRC(0,i) * (TMP_TYPE)factor1;
    ADJUST_TMP(tmp);
    SETDST(i, tmp);
    }
  }


static void RENAME(mix_2_to_1)(gavl_mix_output_channel_t * channel,
                               const gavl_audio_frame_t * input_frame,
                               gavl_audio_frame_t * output_frame)
  {
  int i;
  TMP_TYPE tmp;

  SAMPLE_TYPE factor1 = FACTOR(0);
  SAMPLE_TYPE factor2 = FACTOR(1);
  
  i = input_frame->valid_samples;
  
  while(i--)
    {
    tmp =
      (TMP_TYPE)SRC(0,i) * (TMP_TYPE)factor1 +
      (TMP_TYPE)SRC(1,i) * (TMP_TYPE)factor2;
    ADJUST_TMP(tmp);
    SETDST(i, tmp);
    }
  }

static void RENAME(mix_3_to_1)(gavl_mix_output_channel_t * channel,
                               const gavl_audio_frame_t * input_frame,
                               gavl_audio_frame_t * output_frame)
  {
  int i;
  TMP_TYPE tmp;
    
  SAMPLE_TYPE factor1 = FACTOR(0);
  SAMPLE_TYPE factor2 = FACTOR(1);
  SAMPLE_TYPE factor3 = FACTOR(2);
  
  i = input_frame->valid_samples;
  
  while(i--)
    {
    tmp =
      (TMP_TYPE)SRC(0,i) * (TMP_TYPE)factor1 +
      (TMP_TYPE)SRC(1,i) * (TMP_TYPE)factor2 +
      (TMP_TYPE)SRC(2,i) * (TMP_TYPE)factor3;
    ADJUST_TMP(tmp);
    SETDST(i, tmp);
    }
  }

static void RENAME(mix_4_to_1)(gavl_mix_output_channel_t * channel,
                               const gavl_audio_frame_t * input_frame,
                               gavl_audio_frame_t * output_frame)
  {
  int i;
  TMP_TYPE tmp;

  SAMPLE_TYPE factor1 = FACTOR(0);
  SAMPLE_TYPE factor2 = FACTOR(1);
  SAMPLE_TYPE factor3 = FACTOR(2);
  SAMPLE_TYPE factor4 = FACTOR(3);
  
  i = input_frame->valid_samples;
  
  while(i--)
    {
    tmp =
      (TMP_TYPE)SRC(0,i) * (TMP_TYPE)factor1 +
      (TMP_TYPE)SRC(1,i) * (TMP_TYPE)factor2 +
      (TMP_TYPE)SRC(2,i) * (TMP_TYPE)factor3 +
      (TMP_TYPE)SRC(3,i) * (TMP_TYPE)factor4;
    ADJUST_TMP(tmp);
    SETDST(i, tmp);
    }
  }


static void RENAME(mix_5_to_1)(gavl_mix_output_channel_t * channel,
                               const gavl_audio_frame_t * input_frame,
                               gavl_audio_frame_t * output_frame)
  {
  int i;
  TMP_TYPE tmp;

  SAMPLE_TYPE factor1 = FACTOR(0);
  SAMPLE_TYPE factor2 = FACTOR(1);
  SAMPLE_TYPE factor3 = FACTOR(2);
  SAMPLE_TYPE factor4 = FACTOR(3);
  SAMPLE_TYPE factor5 = FACTOR(4);
  
  i = input_frame->valid_samples;
  
  while(i--)
    {
    tmp =
      (TMP_TYPE)SRC(0,i) * (TMP_TYPE)factor1 +
      (TMP_TYPE)SRC(1,i) * (TMP_TYPE)factor2 +
      (TMP_TYPE)SRC(1,i) * (TMP_TYPE)factor3 +
      (TMP_TYPE)SRC(3,i) * (TMP_TYPE)factor4 +
      (TMP_TYPE)SRC(4,i) * (TMP_TYPE)factor5;
    ADJUST_TMP(tmp);
    SETDST(i, tmp);
    }
  }

static void RENAME(mix_6_to_1)(gavl_mix_output_channel_t * channel,
                               const gavl_audio_frame_t * input_frame,
                               gavl_audio_frame_t * output_frame)
  {
  int i;
  TMP_TYPE tmp;

  SAMPLE_TYPE factor1 = FACTOR(0);
  SAMPLE_TYPE factor2 = FACTOR(1);
  SAMPLE_TYPE factor3 = FACTOR(2);
  SAMPLE_TYPE factor4 = FACTOR(3);
  SAMPLE_TYPE factor5 = FACTOR(4);
  SAMPLE_TYPE factor6 = FACTOR(5);

  i = input_frame->valid_samples;
  
  while(i--)
    {
    tmp =
      (TMP_TYPE)SRC(0,i) * (TMP_TYPE)factor1 +
      (TMP_TYPE)SRC(1,i) * (TMP_TYPE)factor2 +
      (TMP_TYPE)SRC(2,i) * (TMP_TYPE)factor3 +
      (TMP_TYPE)SRC(3,i) * (TMP_TYPE)factor4 +
      (TMP_TYPE)SRC(4,i) * (TMP_TYPE)factor5 +
      (TMP_TYPE)SRC(5,i) * (TMP_TYPE)factor6;
    ADJUST_TMP(tmp);

    SETDST(i, tmp);
    
    }
  }

static void RENAME(mix_all_to_1)(gavl_mix_output_channel_t * channel,
                                 const gavl_audio_frame_t * input_frame,
                                 gavl_audio_frame_t * output_frame)
  {
  int i, j;
  TMP_TYPE tmp;
    
  i = input_frame->valid_samples;
  
  while(i--)
    {
    tmp = 0;
    j = channel->num_inputs;
    
    while(j--)
      tmp += (TMP_TYPE)SRC(j,i) * (TMP_TYPE)FACTOR(j);
    
    ADJUST_TMP(tmp);

    SETDST(i, tmp);
    
    }
  }

