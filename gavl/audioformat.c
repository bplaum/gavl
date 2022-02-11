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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gavl/gavl.h>
#include <stdlib.h>

#include <gavl/value.h>

static const struct
  {
  gavl_sample_format_t format;
  char * name;
  char * short_name;
  }
sample_format_names[] =
  {
    { GAVL_SAMPLE_NONE,   "Not specified", "unknown" }, // Must be first
    { GAVL_SAMPLE_U8,     "Unsigned 8 bit", "u8" },
    { GAVL_SAMPLE_S8,     "Signed 8 bit", "s8" },
    { GAVL_SAMPLE_U16,    "Unsigned 16 bit", "u16" },
    { GAVL_SAMPLE_S16,    "Signed 16 bit", "s16" },
    { GAVL_SAMPLE_S32,    "Signed 32 bit", "s32" },
    { GAVL_SAMPLE_FLOAT,  "Floating point", "float"},
    { GAVL_SAMPLE_DOUBLE, "Double precision", "double"},
  };

const char * gavl_sample_format_to_string(gavl_sample_format_t format)
  {
  int i;
  for(i = 0; i < sizeof(sample_format_names)/sizeof(sample_format_names[0]); i++)
    {
    if(format == sample_format_names[i].format)
      return sample_format_names[i].name;
    }
  return NULL;
  }

const char * gavl_sample_format_to_short_string(gavl_sample_format_t format)
  {
  int i;
  for(i = 0; i < sizeof(sample_format_names)/sizeof(sample_format_names[0]); i++)
    {
    if(format == sample_format_names[i].format)
      return sample_format_names[i].short_name;
    }
  return sample_format_names[0].short_name;
  }

gavl_sample_format_t gavl_string_to_sample_format(const char * str)
  {
  int i;
  for(i = 0; i < sizeof(sample_format_names)/sizeof(sample_format_names[0]); i++)
    {
    if(!strcmp(str, sample_format_names[i].name))
       return sample_format_names[i].format;
    }
  return GAVL_SAMPLE_NONE;
  }

gavl_sample_format_t gavl_short_string_to_sample_format(const char * str)
  {
  int i;
  for(i = 0; i < sizeof(sample_format_names)/sizeof(sample_format_names[0]); i++)
    {
    if(!strcmp(str, sample_format_names[i].short_name))
       return sample_format_names[i].format;
    }
  return GAVL_SAMPLE_NONE;
  }

int gavl_num_sample_formats()
  {
  return sizeof(sample_format_names)/sizeof(sample_format_names[0])-1;
  }

gavl_sample_format_t gavl_get_sample_format(int index)
  {
  if((index >= 0) &&
     (index < sizeof(sample_format_names)/sizeof(sample_format_names[0])))
    return sample_format_names[index].format;
  return GAVL_SAMPLE_NONE;
  }
  
static const struct
  {
  gavl_interleave_mode_t mode;
  char * name;
  char * short_name;
  }
interleave_mode_names[] =
  {
    { GAVL_INTERLEAVE_NONE, "Not interleaved", "none" },
    { GAVL_INTERLEAVE_2,    "Interleaved channel pairs", "pairs" },
    { GAVL_INTERLEAVE_ALL,  "All channels interleaved", "all" },
  };

const char * gavl_interleave_mode_to_string(gavl_interleave_mode_t mode)
  {
  int i;
  for(i = 0;
      i < sizeof(interleave_mode_names)/sizeof(interleave_mode_names[0]);
      i++)
    {
    if(mode == interleave_mode_names[i].mode)
      return interleave_mode_names[i].name;
    }
  return NULL;
  }

const char * gavl_interleave_mode_to_short_string(gavl_interleave_mode_t mode)
  {
  int i;
  for(i = 0;
      i < sizeof(interleave_mode_names)/sizeof(interleave_mode_names[0]);
      i++)
    {
    if(mode == interleave_mode_names[i].mode)
      return interleave_mode_names[i].short_name;
    }
  return interleave_mode_names[0].short_name;
  }

gavl_interleave_mode_t gavl_short_string_to_interleave_mode(const char * mode)
  {
  int i;
  for(i = 0;
      i < sizeof(interleave_mode_names)/sizeof(interleave_mode_names[0]);
      i++)
    {
    if(!strcmp(mode, interleave_mode_names[i].short_name))
      return interleave_mode_names[i].mode;
    }
  return interleave_mode_names[0].mode;
  }

static const struct
  {
  gavl_channel_id_t id;
  char * name;
  char * short_name;
  }
channel_id_names[] =
  {
    { GAVL_CHID_NONE,                "Unknown channel", "unknown" },
    { GAVL_CHID_FRONT_CENTER,        "Front C", "fc" },
    { GAVL_CHID_FRONT_LEFT,          "Front L", "fl"  },
    { GAVL_CHID_FRONT_RIGHT,         "Front R", "fr"  },
    { GAVL_CHID_FRONT_CENTER_LEFT,   "Front CL", "fcl"  },
    { GAVL_CHID_FRONT_CENTER_RIGHT,  "Front CR", "fcr"  },
    { GAVL_CHID_REAR_CENTER,         "Rear C", "rc"  },
    { GAVL_CHID_REAR_LEFT,           "Rear L", "rl"  },
    { GAVL_CHID_REAR_RIGHT,          "Rear R", "rr"  },
    { GAVL_CHID_SIDE_LEFT,           "Side L", "sl"  },
    { GAVL_CHID_SIDE_RIGHT,          "Side R", "sr"  },
    { GAVL_CHID_LFE,                 "LFE", "lfe"  },
    { GAVL_CHID_AUX,                 "AUX", "aux"  },
  };

const char * gavl_channel_id_to_string(gavl_channel_id_t id)
  {
  int i;
  for(i = 0;
      i < sizeof(channel_id_names)/sizeof(channel_id_names[0]);
      i++)
    {
    //    fprintf(stderr, "ID: %d\n", id);
    if(id == channel_id_names[i].id)
      return channel_id_names[i].name;
    }
  return NULL;
  }

const char * gavl_channel_id_to_short_string(gavl_channel_id_t id)
  {
  int i;
  for(i = 0;
      i < sizeof(channel_id_names)/sizeof(channel_id_names[0]);
      i++)
    {
    //    fprintf(stderr, "ID: %d\n", id);
    if(id == channel_id_names[i].id)
      return channel_id_names[i].short_name;
    }
  return channel_id_names[0].short_name;
  }

gavl_channel_id_t gavl_short_string_to_channel_id(const char * id)
  {
  int i;
  for(i = 0;
      i < sizeof(channel_id_names)/sizeof(channel_id_names[0]);
      i++)
    {
    //    fprintf(stderr, "ID: %d\n", id);
    if(!strcmp(id, channel_id_names[i].short_name))
      return channel_id_names[i].id;
    }
  return channel_id_names[0].id;
  }

static void do_indent(int num)
  {
  int i;
  for(i = 0; i < num; i++)
    fprintf(stderr, " ");
  }

void gavl_audio_format_dump(const gavl_audio_format_t * format)
  {
  gavl_audio_format_dumpi(format, 2);
  }

void gavl_audio_format_dumpi(const gavl_audio_format_t * f, int indent)
  {
  int i;
  do_indent(indent);
  fprintf(stderr, "Channels:          %d\n", f->num_channels);

  do_indent(indent);
  fprintf(stderr, "Channel order:     ");
  for(i = 0; i < f->num_channels; i++)
    {
    fprintf(stderr, "%s", gavl_channel_id_to_string(f->channel_locations[i]));
    if(i < f->num_channels - 1)
      fprintf(stderr, ", ");
    }
  fprintf(stderr, "\n");

  do_indent(indent);
  fprintf(stderr, "Samplerate:        %d\n", f->samplerate);

  do_indent(indent);
  fprintf(stderr, "Samples per frame: %d\n", f->samples_per_frame);

  do_indent(indent);
  fprintf(stderr, "Interleave Mode:   %s\n",
          gavl_interleave_mode_to_string(f->interleave_mode));

  do_indent(indent);
  fprintf(stderr, "Sample format:     %s\n",
          gavl_sample_format_to_string(f->sample_format));
  
  if(gavl_front_channels(f) == 3)
    {
    do_indent(indent);
    if(f->center_level > 0.0)
      fprintf(stderr, "Center level:      %0.1f dB\n", 20 * log10(f->center_level));
    else
      fprintf(stderr, "Center level:      Zero\n");
    }
  if(gavl_rear_channels(f))
    {
    do_indent(indent);
    if(f->rear_level > 0.0)
      fprintf(stderr, "Rear level:        %0.1f dB\n", 20 * log10(f->rear_level));
    else
      fprintf(stderr, "Rear level:        Zero\n");
    }
  }

void gavl_set_channel_setup(gavl_audio_format_t * dst)
  {
  int i;
  if(dst->channel_locations[0] == GAVL_CHID_NONE)
    {
    switch(dst->num_channels)
      {
      case 1:
        dst->channel_locations[0] = GAVL_CHID_FRONT_CENTER;
        break;
      case 2: /* 2 Front channels (Stereo or Dual channels) */
        dst->channel_locations[0] = GAVL_CHID_FRONT_LEFT;
        dst->channel_locations[1] = GAVL_CHID_FRONT_RIGHT;
        break;
      case 3:
        dst->channel_locations[0] = GAVL_CHID_FRONT_LEFT;
        dst->channel_locations[1] = GAVL_CHID_FRONT_RIGHT;
        dst->channel_locations[2] = GAVL_CHID_FRONT_CENTER;
        break;
      case 4:
        dst->channel_locations[0] = GAVL_CHID_FRONT_LEFT;
        dst->channel_locations[1] = GAVL_CHID_FRONT_RIGHT;
        dst->channel_locations[2] = GAVL_CHID_REAR_LEFT;
        dst->channel_locations[3] = GAVL_CHID_REAR_RIGHT;
        break;
      case 5:
        dst->channel_locations[0] = GAVL_CHID_FRONT_LEFT;
        dst->channel_locations[1] = GAVL_CHID_FRONT_RIGHT;
        dst->channel_locations[2] = GAVL_CHID_REAR_LEFT;
        dst->channel_locations[3] = GAVL_CHID_REAR_RIGHT;
        dst->channel_locations[4] = GAVL_CHID_FRONT_CENTER;
        break;
      case 6:
        dst->channel_locations[0] = GAVL_CHID_FRONT_LEFT;
        dst->channel_locations[1] = GAVL_CHID_FRONT_RIGHT;
        dst->channel_locations[2] = GAVL_CHID_REAR_LEFT;
        dst->channel_locations[3] = GAVL_CHID_REAR_RIGHT;
        dst->channel_locations[4] = GAVL_CHID_FRONT_CENTER;
        dst->channel_locations[5] = GAVL_CHID_LFE;
        break;
      default:
        for(i = 0; i < dst->num_channels; i++)
          dst->channel_locations[i] = GAVL_CHID_AUX;
        break;
      }
    }
  }

void gavl_audio_format_copy(gavl_audio_format_t * dst,
                            const gavl_audio_format_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  }

int gavl_channel_index(const gavl_audio_format_t * f, gavl_channel_id_t id)
  {
  int i;
  for(i = 0; i < f->num_channels; i++)
    {
    if(f->channel_locations[i] == id)
      return i;
    }
  //  fprintf(stderr, "Channel %s not present!!! Format was\n",
  //          gavl_channel_id_to_string(id));
  //  gavl_audio_format_dump(f);
  return -1;
  }

int gavl_front_channels(const gavl_audio_format_t * f)
  {
  int i;
  int result = 0;
  for(i = 0; i < f->num_channels; i++)
    {
    switch(f->channel_locations[i])
      {
      case GAVL_CHID_FRONT_CENTER:
      case GAVL_CHID_FRONT_LEFT:
      case GAVL_CHID_FRONT_RIGHT:
      case GAVL_CHID_FRONT_CENTER_LEFT:
      case GAVL_CHID_FRONT_CENTER_RIGHT:
        result++;
        break;
      case GAVL_CHID_NONE:
      case GAVL_CHID_REAR_LEFT:
      case GAVL_CHID_REAR_RIGHT:
      case GAVL_CHID_REAR_CENTER:
      case GAVL_CHID_SIDE_LEFT:
      case GAVL_CHID_SIDE_RIGHT:
      case GAVL_CHID_LFE:
      case GAVL_CHID_AUX:
        break;
      }
    }
  return result;
  }

int gavl_rear_channels(const gavl_audio_format_t * f)
  {
  int i;
  int result = 0;
  for(i = 0; i < f->num_channels; i++)
    {
    switch(f->channel_locations[i])
      {
      case GAVL_CHID_REAR_LEFT:
      case GAVL_CHID_REAR_RIGHT:
      case GAVL_CHID_REAR_CENTER:
        result++;
        break;
      case GAVL_CHID_FRONT_CENTER:
      case GAVL_CHID_FRONT_LEFT:
      case GAVL_CHID_FRONT_RIGHT:
      case GAVL_CHID_FRONT_CENTER_LEFT:
      case GAVL_CHID_FRONT_CENTER_RIGHT:
      case GAVL_CHID_NONE:
      case GAVL_CHID_SIDE_LEFT:
      case GAVL_CHID_SIDE_RIGHT:
      case GAVL_CHID_LFE:
      case GAVL_CHID_AUX:
        break;
      }
    }
  return result;
  }

int gavl_side_channels(const gavl_audio_format_t * f)
  {
  int i;
  int result = 0;
  for(i = 0; i < f->num_channels; i++)
    {
    switch(f->channel_locations[i])
      {
      case GAVL_CHID_SIDE_LEFT:
      case GAVL_CHID_SIDE_RIGHT:
        result++;
        break;
      case GAVL_CHID_REAR_LEFT:
      case GAVL_CHID_REAR_RIGHT:
      case GAVL_CHID_REAR_CENTER:
      case GAVL_CHID_FRONT_CENTER:
      case GAVL_CHID_FRONT_LEFT:
      case GAVL_CHID_FRONT_RIGHT:
      case GAVL_CHID_FRONT_CENTER_LEFT:
      case GAVL_CHID_FRONT_CENTER_RIGHT:
      case GAVL_CHID_NONE:
      case GAVL_CHID_LFE:
      case GAVL_CHID_AUX:
        break;
      }
    }
  return result;
  }

int gavl_lfe_channels(const gavl_audio_format_t * f)
  {
  int i;
  int result = 0;
  for(i = 0; i < f->num_channels; i++)
    {
    switch(f->channel_locations[i])
      {
      case GAVL_CHID_LFE:
        result++;
        break;
      case GAVL_CHID_SIDE_LEFT:
      case GAVL_CHID_SIDE_RIGHT:
      case GAVL_CHID_REAR_LEFT:
      case GAVL_CHID_REAR_RIGHT:
      case GAVL_CHID_REAR_CENTER:
      case GAVL_CHID_FRONT_CENTER:
      case GAVL_CHID_FRONT_LEFT:
      case GAVL_CHID_FRONT_RIGHT:
      case GAVL_CHID_FRONT_CENTER_LEFT:
      case GAVL_CHID_FRONT_CENTER_RIGHT:
      case GAVL_CHID_NONE:
      case GAVL_CHID_AUX:
        break;
      }
    }
  return result;
  }

int gavl_aux_channels(const gavl_audio_format_t * f)
  {
  int i;
  int result = 0;
  for(i = 0; i < f->num_channels; i++)
    {
    switch(f->channel_locations[i])
      {
      case GAVL_CHID_AUX:
        result++;
        break;
      case GAVL_CHID_SIDE_LEFT:
      case GAVL_CHID_SIDE_RIGHT:
      case GAVL_CHID_REAR_LEFT:
      case GAVL_CHID_REAR_RIGHT:
      case GAVL_CHID_REAR_CENTER:
      case GAVL_CHID_FRONT_CENTER:
      case GAVL_CHID_FRONT_LEFT:
      case GAVL_CHID_FRONT_RIGHT:
      case GAVL_CHID_FRONT_CENTER_LEFT:
      case GAVL_CHID_FRONT_CENTER_RIGHT:
      case GAVL_CHID_NONE:
      case GAVL_CHID_LFE:
        break;
      }
    }
  return result;
  }



int gavl_bytes_per_sample(gavl_sample_format_t format)
  {
  switch(format)
    {
    case     GAVL_SAMPLE_U8:
    case     GAVL_SAMPLE_S8:
      return 1;
      break;
    case     GAVL_SAMPLE_U16:
    case     GAVL_SAMPLE_S16:
      return 2;
      break;
    case     GAVL_SAMPLE_S32:
      return 4;
      break;
    case     GAVL_SAMPLE_FLOAT:
      return sizeof(float);
      break;
    case     GAVL_SAMPLE_DOUBLE:
      return sizeof(double);
      break;
    case     GAVL_SAMPLE_NONE:
      return 0;
    }
  return 0;
  }


int gavl_audio_formats_equal(const gavl_audio_format_t * format_1,
                              const gavl_audio_format_t * format_2)
  {
  return !memcmp(format_1, format_2, sizeof(*format_1));
  }

int gavl_nearest_samplerate(int in_rate, const int * supported)
  {
  int index = 0, min_index = 0, min_diff = 0, diff;
  
  while(supported[index] >= 0)
    {
    if(in_rate == supported[index])
      return supported[index];

    diff = abs(in_rate - supported[index]);
    
    if(!index || diff < min_diff)
      {
      min_index = index;
      min_diff = diff;
      }
    index++;
    }
  return supported[min_index];
  }

/*   Conversion from <-> to Dictionary. This is for file formats
 *   (json or xml).
 */

#define PUT_INT(member) \
  gavl_value_set_int(&val, fmt->member); \
  gavl_dictionary_set_nocopy(dict, #member, &val)

#define PUT_FLOAT(member) \
  gavl_value_set_float(&val, fmt->member); \
  gavl_dictionary_set_nocopy(dict, #member, &val)

#define PUT_STRING(member, str) \
  gavl_value_set_string(&val, str); \
  gavl_dictionary_set_nocopy(dict, #member, &val)

void gavl_audio_format_to_dictionary(const gavl_audio_format_t * fmt, gavl_dictionary_t * dict)
  {
  int i;
  gavl_array_t * arr;
  gavl_value_t val;
  gavl_value_t child;
  gavl_value_init(&val);                 \
  
  PUT_INT(samples_per_frame);
  PUT_INT(samplerate);
  PUT_INT(num_channels);

  PUT_STRING(sample_format, gavl_sample_format_to_short_string(fmt->sample_format));
  PUT_STRING(interleave_mode, gavl_interleave_mode_to_short_string(fmt->interleave_mode));

  PUT_FLOAT(center_level);
  PUT_FLOAT(rear_level);
  
  arr = gavl_value_set_array(&val);
  
  for(i = 0; i < fmt->num_channels; i++)
    {
    gavl_value_init(&child);
    gavl_value_set_string(&child, gavl_channel_id_to_short_string(fmt->channel_locations[i]));
    gavl_array_push_nocopy(arr, &child);
    }
  gavl_dictionary_set_nocopy(dict, "channel_locations", &val);
  }

#define GET_INT(member) \
  if(!(val = gavl_dictionary_get(dict, #member)) || \
     !gavl_value_get_int(val, &val_i)) \
    goto fail; \
  fmt->member = val_i;

#define GET_FLOAT(member)                             \
  if(!(val = gavl_dictionary_get(dict, #member)) || \
     !gavl_value_get_float(val, &val_f)) \
    goto fail; \
  fmt->member = val_i;
     
  
int gavl_audio_format_from_dictionary(gavl_audio_format_t * fmt, const gavl_dictionary_t * dict)
  {
  int i;
  const char * str;
  const gavl_value_t * val;
  int val_i;
  double val_f;
  const gavl_array_t * arr;  
  int ret = 0;
  
  GET_INT(samples_per_frame);
  GET_INT(samplerate);
  GET_INT(num_channels);
  GET_FLOAT(center_level);
  GET_FLOAT(rear_level);

  if(!(str = gavl_dictionary_get_string(dict, "sample_format")))
    goto fail;
  fmt->sample_format = gavl_short_string_to_sample_format(str);

  if(!(str = gavl_dictionary_get_string(dict, "interleave_mode")))
    goto fail;
  fmt->interleave_mode = gavl_short_string_to_interleave_mode(str);

  if(!(val = gavl_dictionary_get(dict, "channel_locations")) ||
     !(arr = gavl_value_get_array(val)))
    goto fail;

  if(arr->num_entries != fmt->num_channels)
    goto fail;
    
  for(i = 0; i < fmt->num_channels; i++)
    {
    if(!(str = gavl_value_get_string(&arr->entries[i])))
      goto fail;
    fmt->channel_locations[i] = gavl_short_string_to_channel_id(str);
    }
  
  ret = 1;
  fail:
 
  return ret;
  }

