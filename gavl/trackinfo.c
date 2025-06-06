/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2024 Members of the Gmerlin project
 * http://github.com/bplaum
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



#include <config.h>
#include <gavl/gavl.h>
#include <gavl/metadata.h>
#include <gavl/metatags.h>
#include <gavl/utils.h>
#include <gavl/trackinfo.h>

#include <countrycodes.h>


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

const struct
  {
  gavl_stream_type_t type;
  const char * name;
  }
stream_types[] =
  {

    { GAVL_STREAM_AUDIO, "Audio"     },
    { GAVL_STREAM_VIDEO, "Video"     },
    { GAVL_STREAM_TEXT, "Text"       },
    { GAVL_STREAM_OVERLAY, "Overlay" },
    { GAVL_STREAM_MSG, "Message"     },
    { GAVL_STREAM_NONE, "None"       },
    { /* End */  },
  }; 

const char * gavl_stream_type_name(gavl_stream_type_t type)
  {
  int idx = 0;
  while(stream_types[idx].name)
    {
    if(type == stream_types[idx].type)
      return stream_types[idx].name;
    idx++;
    }
  return NULL;
  }

int gavl_track_get_num_streams(const gavl_dictionary_t * d, gavl_stream_type_t type)
  {
  int type_from_dict = 0;
  int ret = 0, i;
  const gavl_array_t * arr;
  const gavl_dictionary_t * dict;

  if(!(arr = gavl_dictionary_get_array(d, GAVL_META_STREAMS)))
    return 0;

  for(i = 0; i < arr->num_entries; i++)
    {
    if((dict = gavl_value_get_dictionary_nc(&arr->entries[i])) &&
       gavl_dictionary_get_int(dict, GAVL_META_STREAM_TYPE, &type_from_dict) &&
       (type_from_dict == type))
      ret++;
    }
  return ret;
  }

void gavl_track_delete_implicit_fields(gavl_dictionary_t * track)
  {
  int i;
  
  gavl_dictionary_t * m;
  gavl_dictionary_t * s;
  gavl_array_t * arr;

  m = gavl_track_get_metadata_nc(track);
  gavl_metadata_delete_implicit_fields(m);
  
  if((arr = gavl_dictionary_get_array_nc(track, GAVL_META_STREAMS)))
    {
    for(i = 0; i < arr->num_entries; i++)
      {
      if((s = gavl_value_get_dictionary_nc(&arr->entries[i])))
        {
        gavl_dictionary_set(s, GAVL_META_STREAM_STATS, NULL);
        m = gavl_stream_get_metadata_nc(s);
        gavl_metadata_delete_implicit_fields(m);
        }
      }
    }
  }

int gavl_track_stream_idx_to_abs(const gavl_dictionary_t * d, gavl_stream_type_t type, int idx)
  {
  int ctr = 0;
  int i;
  int type_from_dict = 0;
  
  const gavl_array_t * arr;
  const gavl_dictionary_t * dict;
  
  if(!(arr = gavl_dictionary_get_array(d, GAVL_META_STREAMS)))
    return -1;
  
  for(i = 0; i < arr->num_entries; i++)
    {
    if((dict = gavl_value_get_dictionary_nc(&arr->entries[i])) &&
       gavl_dictionary_get_int(dict, GAVL_META_STREAM_TYPE, &type_from_dict) &&
       (type_from_dict == type))
      {
      if(ctr == idx)
        return i;
      ctr++;
      }
    }
  return -1;
  }

int gavl_track_stream_idx_to_rel(const gavl_dictionary_t * d, int idx)
  {
  int i;
  int type;
  int ret = 0;
  int type_from_dict = 0;
  
  const gavl_array_t * arr;
  const gavl_dictionary_t * dict;

  if(!(arr = gavl_dictionary_get_array(d, GAVL_META_STREAMS)) ||
     (idx < 0) || (idx >= arr->num_entries) ||
     (dict = gavl_value_get_dictionary_nc(&arr->entries[idx])) ||
     !gavl_dictionary_get_int(dict, GAVL_META_STREAM_TYPE, &type))
    return -1;
  
  for(i = 0; i < idx; i++)
    {
    if((dict = gavl_value_get_dictionary_nc(&arr->entries[idx])) &&
       gavl_dictionary_get_int(dict, GAVL_META_STREAM_TYPE, &type_from_dict) &&
       (type == type_from_dict))
      ret++;
    }
  return ret;
  }


/* */

gavl_dictionary_t *
gavl_track_get_stream_nc(gavl_dictionary_t * d, gavl_stream_type_t type, int i)
  {
  gavl_array_t * arr;
  gavl_value_t * val;
  gavl_dictionary_t * dict;

  int idx = 0;
  int pos = 0;
  int type_from_dict = 0;
    
  if(!(arr = gavl_dictionary_get_array_nc(d, GAVL_META_STREAMS)))
    return NULL;

  for(pos = 0; pos < arr->num_entries; pos++)
    {
    if((val = gavl_array_get_nc(arr, pos)) &&
       (dict = gavl_value_get_dictionary_nc(val)) &&
       gavl_dictionary_get_int(dict, GAVL_META_STREAM_TYPE, &type_from_dict) &&
       (type_from_dict == type))
      {
      if(i == idx)
        return dict;
      else
        idx++;
      }
    }
  
  return NULL; 
  }

const gavl_dictionary_t *
gavl_track_get_stream(const gavl_dictionary_t * d, gavl_stream_type_t type, int i)
  {
  const gavl_array_t * arr;
  const gavl_value_t * val;
  const gavl_dictionary_t * dict;

  int idx = 0;
  int pos = 0;
  int type_from_dict = 0;
    
  if(!(arr = gavl_dictionary_get_array(d, GAVL_META_STREAMS)))
    return NULL;

  for(pos = 0; pos < arr->num_entries; pos++)
    {
    if((val = gavl_array_get(arr, pos)) &&
       (dict = gavl_value_get_dictionary(val)) &&
       gavl_dictionary_get_int(dict, GAVL_META_STREAM_TYPE, &type_from_dict) &&
       (type_from_dict == type))
      {
      if(i == idx)
        return dict;
      else
        idx++;
      }
    }
  
  return NULL; 
  }

static const gavl_dictionary_t * get_stream_metadata(const gavl_dictionary_t * d, gavl_stream_type_t type, int i)
  {
  const gavl_dictionary_t * s;

  if(!(s = gavl_track_get_stream(d, type, i)))
    return NULL;
  
  return gavl_dictionary_get_dictionary(s, GAVL_META_METADATA);
  }

static gavl_dictionary_t * get_stream_metadata_nc(gavl_dictionary_t * d, gavl_stream_type_t type, int i)
  {
  gavl_dictionary_t * s;

  if(!(s = gavl_track_get_stream_nc(d, type, i)))
    return NULL;
  return gavl_dictionary_get_dictionary_nc(s, GAVL_META_METADATA);
  }

int
gavl_track_get_num_streams_all(const gavl_dictionary_t * d)
  {
  const gavl_array_t * arr;

  if(!(arr = gavl_dictionary_get_array(d, GAVL_META_STREAMS)))
    return 0;
  return arr->num_entries;
  }

const gavl_dictionary_t *
gavl_track_get_stream_all(const gavl_dictionary_t * d, int idx)
  {
  const gavl_array_t * arr;

  if(!(arr = gavl_dictionary_get_array(d, GAVL_META_STREAMS)) ||
     (idx < 0) ||
     (idx >= arr->num_entries))
    return NULL;

  return gavl_value_get_dictionary(&arr->entries[idx]);
  }

gavl_stream_type_t
gavl_stream_get_type(const gavl_dictionary_t * s)
  {
  int ret = GAVL_STREAM_NONE;
  gavl_dictionary_get_int(s, GAVL_META_STREAM_TYPE, &ret);
  return ret;
  }

int
gavl_stream_get_id(const gavl_dictionary_t * s, int * id)
  {
  return gavl_dictionary_get_int(s, GAVL_META_STREAM_ID, id);
  }

void
gavl_stream_set_id(gavl_dictionary_t * s, int id)
  {
  gavl_dictionary_set_int(s, GAVL_META_STREAM_ID, id);
  }

gavl_dictionary_t *
gavl_track_get_stream_all_nc(gavl_dictionary_t * d, int idx)
  {
  gavl_array_t * arr;

  if(!(arr = gavl_dictionary_get_array_nc(d, GAVL_META_STREAMS)) ||
     (idx < 0) ||
     (idx >= arr->num_entries))
    return NULL;
  
  return gavl_value_get_dictionary_nc(&arr->entries[idx]);
  }

gavl_dictionary_t *
gavl_track_find_stream_by_id_nc(gavl_dictionary_t * d, int id)
  {
  int i;
  int id_s;
  gavl_array_t * arr;

  if(!(arr = gavl_dictionary_get_array_nc(d, GAVL_META_STREAMS)))
    return NULL;

  for(i = 0; i < arr->num_entries; i++)
    {
    gavl_dictionary_t * s;
    
    if((s = gavl_value_get_dictionary_nc(&arr->entries[i])) &&
       gavl_stream_get_id(s, &id_s) &&
       (id == id_s))
      return s;
    }
  return NULL;
  }

const gavl_dictionary_t *
gavl_track_find_stream_by_id(const gavl_dictionary_t * d, int id)
  {
  int i;
  int id_s;
  const gavl_array_t * arr;

  if(!(arr = gavl_dictionary_get_array(d, GAVL_META_STREAMS)))
    return NULL;

  for(i = 0; i < arr->num_entries; i++)
    {
    const gavl_dictionary_t * s;
    
    if((s = gavl_value_get_dictionary(&arr->entries[i])) &&
       gavl_stream_get_id(s, &id_s) &&
       (id == id_s))
      return s;
    }
  return NULL;
  }

static gavl_dictionary_t *
append_stream_common(gavl_dictionary_t * d, gavl_stream_type_t type, const char * arr_name)
  {
  gavl_value_t val;
  gavl_array_t * arr;
  gavl_dictionary_t * s;
  gavl_dictionary_t * m;
  
  arr = gavl_dictionary_get_array_create(d, arr_name);
  
  gavl_value_init(&val);
  s = gavl_value_set_dictionary(&val);

  gavl_dictionary_set_int(s, GAVL_META_STREAM_TYPE, type);
  
  m = gavl_dictionary_get_dictionary_create(s, GAVL_META_METADATA);
  gavl_dictionary_set_int(m, GAVL_META_IDX, arr->num_entries);
  
  gavl_array_splice_val_nocopy(arr, arr->num_entries, 0, &val);
  return s;
  }

static gavl_dictionary_t *
append_stream(gavl_dictionary_t * d, gavl_stream_type_t type, const char * arr_name)
  {
  gavl_dictionary_t * s = append_stream_common(d, type, arr_name);
  gavl_stream_set_id(s, gavl_track_get_num_streams_all(d) + GAVL_META_STREAM_ID_MEDIA_START);
  return s;
  }

int gavl_track_delete_stream(gavl_dictionary_t * d, int idx)
  {
  gavl_array_t * arr;

  if(!(arr = gavl_dictionary_get_array_nc(d, GAVL_META_STREAMS)) ||
     (idx < 0) || (idx >= arr->num_entries))
    return 0;
  
  gavl_array_splice_val(arr, idx, 1, NULL);
  return 1;

  }

static int delete_stream(gavl_dictionary_t * d, gavl_stream_type_t type, int idx)
  {
  
  idx = gavl_track_stream_idx_to_abs(d, type, idx);
  if(idx < 0)
    return 0;

  return gavl_track_delete_stream(d, idx);
  }

static void init_stream(gavl_dictionary_t * dict)
  {
  gavl_dictionary_get_dictionary_create(dict, GAVL_META_METADATA);
  }



/* Initialize streams */

  
static void init_audio_stream(gavl_dictionary_t * dict)
  {
  gavl_value_t fmt_val;
  
  gavl_value_init(&fmt_val);
  gavl_value_set_audio_format(&fmt_val);
  gavl_dictionary_set_nocopy(dict, GAVL_META_STREAM_FORMAT, &fmt_val);
  }

static void init_video_stream(gavl_dictionary_t * dict)
  {
  gavl_value_t fmt_val;

  gavl_value_init(&fmt_val);
  gavl_value_set_video_format(&fmt_val);
  gavl_dictionary_set_nocopy(dict, GAVL_META_STREAM_FORMAT, &fmt_val);

  }

static void init_text_stream(gavl_dictionary_t * dict)
  {
#if 0 // Text streams should have no video format
  gavl_value_t fmt_val;

  gavl_value_init(&fmt_val);
  gavl_value_set_video_format(&fmt_val);
  gavl_dictionary_set_nocopy(dict, GAVL_META_STREAM_FORMAT, &fmt_val);
#endif
  }

static void init_overlay_stream(gavl_dictionary_t * dict)
  {
  gavl_value_t fmt_val;

  gavl_value_init(&fmt_val);
  gavl_value_set_video_format(&fmt_val);
  gavl_dictionary_set_nocopy(dict, GAVL_META_STREAM_FORMAT, &fmt_val);

  }

void gavl_init_audio_stream(gavl_dictionary_t * dict)
  {
  gavl_dictionary_set_int(dict, GAVL_META_STREAM_TYPE, GAVL_STREAM_AUDIO);
  init_stream(dict);
  init_audio_stream(dict);
  }

void gavl_init_video_stream(gavl_dictionary_t * dict)
  {
  gavl_dictionary_set_int(dict, GAVL_META_STREAM_TYPE, GAVL_STREAM_VIDEO);
  init_stream(dict);
  init_video_stream(dict);
  }

void gavl_init_text_stream(gavl_dictionary_t * dict)
  {
  gavl_dictionary_set_int(dict, GAVL_META_STREAM_TYPE, GAVL_STREAM_TEXT);
  init_stream(dict);
  init_text_stream(dict);
  }

void gavl_init_overlay_stream(gavl_dictionary_t * dict)
  {
  gavl_dictionary_set_int(dict, GAVL_META_STREAM_TYPE, GAVL_STREAM_OVERLAY);
  init_stream(dict);
  init_overlay_stream(dict);
  }

static void init_msg_stream(gavl_dictionary_t * dict)
  {
  gavl_dictionary_t * m;
  init_stream(dict);

  m = gavl_dictionary_get_dictionary_nc(dict, GAVL_META_METADATA);
  
  gavl_dictionary_set_int(m, GAVL_META_STREAM_SAMPLE_TIMESCALE, GAVL_TIME_SCALE);
  gavl_dictionary_set_int(m, GAVL_META_STREAM_PACKET_TIMESCALE, GAVL_TIME_SCALE);
  }

static gavl_dictionary_t * track_append_stream(gavl_dictionary_t * d, gavl_stream_type_t type,
                                               const char * arr_name)
  {
  gavl_dictionary_t * s = append_stream(d, type, arr_name);

  init_stream(s);
  
  switch(type)
    {
    case GAVL_STREAM_AUDIO:
      init_audio_stream(s);
      break;
    case GAVL_STREAM_VIDEO:
      init_video_stream(s);
      break;
    case GAVL_STREAM_TEXT:
      init_text_stream(s);
      break;
    case GAVL_STREAM_OVERLAY:
      init_overlay_stream(s);
      break;
    case GAVL_STREAM_MSG:
    case GAVL_STREAM_NONE:
      break;
    }
  return s;
  }

gavl_dictionary_t * gavl_track_append_stream(gavl_dictionary_t * d, gavl_stream_type_t type)
  {
  return track_append_stream(d, type, GAVL_META_STREAMS);
  }
/* Audio */

gavl_dictionary_t * gavl_track_get_audio_stream_nc(gavl_dictionary_t * d, int i)
  {
  return gavl_track_get_stream_nc(d, GAVL_STREAM_AUDIO, i);
  }

const gavl_dictionary_t * gavl_track_get_audio_stream(const gavl_dictionary_t * d, int i)
  {
  return gavl_track_get_stream(d, GAVL_STREAM_AUDIO, i);
  }

int gavl_track_get_num_audio_streams(const gavl_dictionary_t * d)
  {
  return gavl_track_get_num_streams(d, GAVL_STREAM_AUDIO);
  }

gavl_dictionary_t * gavl_track_append_audio_stream(gavl_dictionary_t * d)
  {
  return gavl_track_append_stream(d, GAVL_STREAM_AUDIO);
  }

const gavl_dictionary_t * gavl_track_get_audio_metadata(const gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata(d, GAVL_STREAM_AUDIO, stream);
  }

gavl_dictionary_t * gavl_track_get_audio_metadata_nc(gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata_nc(d, GAVL_STREAM_AUDIO, stream);
  }

const gavl_audio_format_t * gavl_track_get_audio_format(const gavl_dictionary_t * d, int stream)
  {
  const gavl_dictionary_t * s;
  const gavl_value_t * val;
    
  if(!(s = gavl_track_get_stream(d, GAVL_STREAM_AUDIO, stream)) ||
     !(val = gavl_dictionary_get(s, GAVL_META_STREAM_FORMAT)))
    return NULL;
  
  return gavl_value_get_audio_format(val);
  }

gavl_audio_format_t * gavl_track_get_audio_format_nc(gavl_dictionary_t * d, int stream)
  {
  gavl_dictionary_t * s;
  gavl_value_t * val;
  
  if(!(s = gavl_track_get_stream_nc(d, GAVL_STREAM_AUDIO, stream)) ||
     !(val = gavl_dictionary_get_nc(s, GAVL_META_STREAM_FORMAT)))
    return NULL;
  
  return gavl_value_get_audio_format_nc(val);
  }

int gavl_track_delete_audio_stream(gavl_dictionary_t * d, int stream)
  {
  return delete_stream(d, GAVL_STREAM_AUDIO, stream);
  }

/* Video */

gavl_dictionary_t * gavl_track_get_video_stream_nc(gavl_dictionary_t * d, int i)
  {
  return gavl_track_get_stream_nc(d, GAVL_STREAM_VIDEO, i);
  }

const gavl_dictionary_t * gavl_track_get_video_stream(const gavl_dictionary_t * d, int i)
  {
  return gavl_track_get_stream(d, GAVL_STREAM_VIDEO, i);
  }
  
int gavl_track_get_num_video_streams(const gavl_dictionary_t * d)
  {
  return gavl_track_get_num_streams(d, GAVL_STREAM_VIDEO);
  }
  
gavl_dictionary_t * gavl_track_append_video_stream(gavl_dictionary_t * d)
  {
  return gavl_track_append_stream(d, GAVL_STREAM_VIDEO);
  }

const gavl_dictionary_t * gavl_track_get_video_metadata(const gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata(d, GAVL_STREAM_VIDEO, stream);
  }

gavl_dictionary_t * gavl_track_get_video_metadata_nc(gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata_nc(d, GAVL_STREAM_VIDEO, stream);
  }

const gavl_video_format_t * gavl_track_get_video_format(const gavl_dictionary_t * d, int stream)
  {
  const gavl_dictionary_t * s;
  const gavl_value_t * val;
    
  if(!(s = gavl_track_get_stream(d, GAVL_STREAM_VIDEO, stream)) ||
     !(val = gavl_dictionary_get(s, GAVL_META_STREAM_FORMAT)))
    return NULL;
  
  return gavl_value_get_video_format(val);
  }

gavl_video_format_t * gavl_track_get_video_format_nc(gavl_dictionary_t * d, int stream)
  {
  gavl_dictionary_t * s;
  gavl_value_t * val;
  
  if(!(s = gavl_track_get_stream_nc(d, GAVL_STREAM_VIDEO, stream)) ||
     !(val = gavl_dictionary_get_nc(s, GAVL_META_STREAM_FORMAT)))
    return NULL;
  
  return gavl_value_get_video_format_nc(val);
  }

int gavl_track_delete_video_stream(gavl_dictionary_t * d, int stream)
  {
  return delete_stream(d, GAVL_STREAM_VIDEO, stream);
  }

/* Text */


gavl_dictionary_t * gavl_track_get_text_stream_nc(gavl_dictionary_t * d, int i)
  {
  return gavl_track_get_stream_nc(d, GAVL_STREAM_TEXT, i);
  }

const gavl_dictionary_t * gavl_track_get_text_stream(const gavl_dictionary_t * d, int i)
  {
  return gavl_track_get_stream(d, GAVL_STREAM_TEXT, i);
  }

int gavl_track_get_num_text_streams(const gavl_dictionary_t * d)
  {
  return gavl_track_get_num_streams(d, GAVL_STREAM_TEXT);
  }
  
gavl_dictionary_t * gavl_track_append_text_stream(gavl_dictionary_t * d)
  {
  return gavl_track_append_stream(d, GAVL_STREAM_TEXT);
  }

const gavl_dictionary_t * gavl_track_get_text_metadata(const gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata(d, GAVL_STREAM_TEXT, stream);
  }

gavl_dictionary_t * gavl_track_get_text_metadata_nc(gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata_nc(d, GAVL_STREAM_TEXT, stream);
  }

int gavl_track_delete_text_stream(gavl_dictionary_t * d, int stream)
  {
  return delete_stream(d, GAVL_STREAM_TEXT, stream);
  }

/* Overlay */

gavl_dictionary_t * gavl_track_get_overlay_stream_nc(gavl_dictionary_t * d, int i)
  {
  return gavl_track_get_stream_nc(d, GAVL_STREAM_OVERLAY, i);
  }

const gavl_dictionary_t * gavl_track_get_overlay_stream(const gavl_dictionary_t * d, int i)
  {
  return gavl_track_get_stream(d, GAVL_STREAM_OVERLAY, i);
  }

int gavl_track_get_num_overlay_streams(const gavl_dictionary_t * d)
  {
  return gavl_track_get_num_streams(d, GAVL_STREAM_OVERLAY);
  }
  
gavl_dictionary_t * gavl_track_append_overlay_stream(gavl_dictionary_t * d)
  {
  return gavl_track_append_stream(d, GAVL_STREAM_OVERLAY);
  }
     
const gavl_dictionary_t * gavl_track_get_overlay_metadata(const gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata(d, GAVL_STREAM_OVERLAY, stream);
  }

gavl_dictionary_t * gavl_track_get_overlay_metadata_nc(gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata_nc(d, GAVL_STREAM_OVERLAY, stream);
  }

const gavl_video_format_t * gavl_track_get_overlay_format(const gavl_dictionary_t * d, int stream)
  {
  const gavl_dictionary_t * s;
  const gavl_value_t * val;
    
  if(!(s = gavl_track_get_stream(d, GAVL_STREAM_OVERLAY, stream)) ||
     !(val = gavl_dictionary_get(s, GAVL_META_STREAM_FORMAT)))
    return NULL;
  
  return gavl_value_get_video_format(val);
  }

gavl_video_format_t * gavl_track_get_overlay_format_nc(gavl_dictionary_t * d, int stream)
  {
  gavl_dictionary_t * s;
  gavl_value_t * val;
  
  if(!(s = gavl_track_get_stream_nc(d, GAVL_STREAM_OVERLAY, stream)) ||
     !(val = gavl_dictionary_get_nc(s, GAVL_META_STREAM_FORMAT)))
    return NULL;
  
  return gavl_value_get_video_format_nc(val);
  }

int gavl_track_delete_overlay_stream(gavl_dictionary_t * d, int stream)
  {
  return delete_stream(d, GAVL_STREAM_OVERLAY, stream);
  }

/* Message */


gavl_dictionary_t * gavl_track_get_msg_stream_nc(gavl_dictionary_t * d, int i)
  {
  return gavl_track_get_stream_nc(d, GAVL_STREAM_MSG, i);
  }

const gavl_dictionary_t * gavl_track_get_msg_stream(const gavl_dictionary_t * d, int i)
  {
  return gavl_track_get_stream(d, GAVL_STREAM_MSG, i);
  }

int gavl_track_get_num_msg_streams(const gavl_dictionary_t * d)
  {
  return gavl_track_get_num_streams(d, GAVL_STREAM_MSG);
  }

gavl_dictionary_t * gavl_track_append_msg_stream(gavl_dictionary_t * d, int id)
  {
  gavl_dictionary_t * s;

  if((s = gavl_track_find_stream_by_id_nc(d, id)))
    return s;
  
  s = append_stream_common(d, GAVL_STREAM_MSG, GAVL_META_STREAMS);
  init_msg_stream(s);
  gavl_stream_set_id(s, id);
  return s;
  }

const gavl_dictionary_t * gavl_track_get_msg_metadata(const gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata(d, GAVL_STREAM_MSG, stream);
  }

gavl_dictionary_t * gavl_track_get_msg_metadata_nc(gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata_nc(d, GAVL_STREAM_MSG, stream);
  }

int gavl_track_delete_msg_stream(gavl_dictionary_t * d, int stream)
  {
  return delete_stream(d, GAVL_STREAM_MSG, stream);
  }

/* Track */

static void track_init(gavl_dictionary_t * track)
  {
  gavl_dictionary_get_dictionary_create(track, GAVL_META_METADATA);
  }

gavl_array_t * gavl_get_tracks_nc(gavl_dictionary_t * dict)
  {
  return gavl_dictionary_get_array_create(dict, GAVL_META_TRACKS);
  }

const gavl_array_t * gavl_get_tracks(const gavl_dictionary_t * dict)
  {
  return gavl_dictionary_get_array(dict, GAVL_META_TRACKS);
  }

static gavl_dictionary_t * append_track(gavl_dictionary_t * dict, const char * arr_name)
  {
  gavl_value_t val;
  gavl_dictionary_t * ret;
  gavl_array_t * arr = gavl_dictionary_get_array_create(dict, arr_name);
  
  gavl_value_init(&val);
  ret = gavl_value_set_dictionary(&val);
  track_init(ret);
  gavl_array_push_nocopy(arr, &val);
  return arr->entries[arr->num_entries-1].v.dictionary;
  }


gavl_dictionary_t * gavl_append_track(gavl_dictionary_t * dict, const gavl_dictionary_t * t)
  {
  gavl_dictionary_t * new_track = append_track(dict, GAVL_META_TRACKS);
  
  if(t)
    gavl_dictionary_copy(new_track, t);
  
  gavl_track_update_children(dict);
  return new_track;
  }

gavl_dictionary_t * gavl_prepend_track(gavl_dictionary_t * dict, const gavl_dictionary_t * t)
  {
  gavl_dictionary_t * new_track;
  gavl_value_t val;
  gavl_array_t * arr = gavl_get_tracks_nc(dict);
  
  gavl_value_init(&val);
  new_track = gavl_value_set_dictionary(&val);

  if(t)
    gavl_dictionary_copy(new_track, t);
  else
    track_init(new_track);
  
  gavl_array_unshift_nocopy(arr, &val);
  //  gavl_array_push_nocopy(arr, &val);
  
  gavl_track_update_children(dict);
  return arr->entries[0].v.dictionary;
  }

static const gavl_dictionary_t * get_track(const gavl_dictionary_t * dict, const char * array_name, int idx)
  {
  const gavl_value_t * val;
  const gavl_array_t * arr;

  if(!(arr = gavl_dictionary_get_array(dict, array_name)) ||
     !(val = gavl_array_get(arr, idx)))
    return NULL;
  else
    return gavl_value_get_dictionary(val);
  }

static gavl_dictionary_t * get_track_nc(gavl_dictionary_t * dict, const char * array_name, int idx)
  {
  gavl_value_t * val;
  gavl_array_t * arr;

  if(!(arr = gavl_dictionary_get_array_nc(dict, array_name)) ||
     !(val = gavl_array_get_nc(arr, idx)))
    return NULL;
  else
    return gavl_value_get_dictionary_nc(val);
  }

const gavl_dictionary_t * gavl_get_track(const gavl_dictionary_t * dict, int idx)
  {
  if((idx < 0) && !gavl_dictionary_get_int(dict, GAVL_META_CURIDX, &idx))
    idx = 0;

  return get_track(dict, GAVL_META_TRACKS, idx);
  }

gavl_dictionary_t * gavl_get_track_nc(gavl_dictionary_t * dict, int idx)
  {
  gavl_value_t * val;
  gavl_array_t * tracks;

  if((idx < 0) && !gavl_dictionary_get_int(dict, GAVL_META_CURIDX, &idx))
    idx = 0;

  if(!(tracks = gavl_get_tracks_nc(dict)) ||
     !(val = gavl_array_get_nc(tracks, idx)))
    return NULL;
  return gavl_value_get_dictionary_nc(val);
  }

void gavl_set_current_track(gavl_dictionary_t * dict, int idx)
  {
  gavl_dictionary_set_int(dict, GAVL_META_CURIDX, idx);
  }

int gavl_get_current_track(gavl_dictionary_t * dict)
  {
  int ret = 0;
  gavl_dictionary_get_int(dict, GAVL_META_CURIDX, &ret);
  return ret;
  }

static int get_num_tracks(const gavl_dictionary_t * dict, const char * arr_name)
  {
  const gavl_array_t * tracks;
  if(!(tracks = gavl_dictionary_get_array(dict, arr_name)))
    return 0;
  return tracks->num_entries;
  }

int gavl_get_num_tracks(const gavl_dictionary_t * dict)
  {
  return get_num_tracks(dict, GAVL_META_TRACKS);
  }

int gavl_get_num_tracks_loaded(const gavl_dictionary_t * dict,
                               int * total)
  {
  int ret;
  int num_children = 0;

  const gavl_array_t * tracks;
  const gavl_dictionary_t * m;

  if(!(tracks = gavl_get_tracks(dict)))
    ret = 0;
  else
    ret = tracks->num_entries;
  
  if(!(m = gavl_dictionary_get_dictionary(dict, GAVL_META_METADATA)) ||
     !gavl_dictionary_get_int(m, GAVL_META_NUM_CHILDREN, &num_children))
    *total = 0;
  else
    *total = num_children;
  
  return ret;
  }

void gavl_delete_track(gavl_dictionary_t * dict, int idx)
  {
  gavl_track_splice_children(dict, idx, 1, NULL);
  }

void gavl_track_splice_children(gavl_dictionary_t * dict, int idx, int del,
                                const gavl_value_t * val)
  {
  gavl_array_t * tracks;
  if(!(tracks = gavl_get_tracks_nc(dict)))
    return;
  
  if(!val || (val->type == GAVL_TYPE_DICTIONARY) || (val->type == GAVL_TYPE_UNDEFINED))
    {
    gavl_array_splice_val(tracks, idx, del, val);
    }
  else if(val->type == GAVL_TYPE_ARRAY)
    {
    gavl_array_splice_array(tracks, idx, del, val->v.array);
    }
  gavl_track_update_children(dict);
  }

void gavl_track_splice_children_nocopy(gavl_dictionary_t * dict, int idx, int del,
                                       gavl_value_t * val)
  {
  gavl_array_t * tracks;
  if(!(tracks = gavl_get_tracks_nc(dict)))
    return;
  
  if(!val || (val->type == GAVL_TYPE_DICTIONARY) || (val->type == GAVL_TYPE_UNDEFINED))
    {
    gavl_array_splice_val_nocopy(tracks, idx, del, val);
    }
  else if(val->type == GAVL_TYPE_ARRAY)
    {
    gavl_array_splice_array_nocopy(tracks, idx, del, val->v.array);
    }
  gavl_track_update_children(dict);
  gavl_value_reset(val);
  }


gavl_audio_format_t * gavl_stream_get_audio_format_nc(gavl_dictionary_t * dict)
  {
  gavl_value_t * val;
  if((val = gavl_dictionary_get_nc(dict, GAVL_META_STREAM_FORMAT)))
    return gavl_value_get_audio_format_nc(val);
  else
    return NULL;
  }

gavl_video_format_t * gavl_stream_get_video_format_nc(gavl_dictionary_t * dict)
  {
  gavl_value_t * val;
  if((val = gavl_dictionary_get_nc(dict, GAVL_META_STREAM_FORMAT)))
    return gavl_value_get_video_format_nc(val);
  else
    return NULL;
  }

const gavl_audio_format_t * gavl_stream_get_audio_format(const gavl_dictionary_t * dict)
  {
  const gavl_value_t * val;
  if((val = gavl_dictionary_get(dict, GAVL_META_STREAM_FORMAT)))
    return gavl_value_get_audio_format(val);
  else
    return NULL;
  }

const gavl_video_format_t * gavl_stream_get_video_format(const gavl_dictionary_t * dict)
  {
  const gavl_value_t * val;
  if((val = gavl_dictionary_get(dict, GAVL_META_STREAM_FORMAT)))
    return gavl_value_get_video_format(val);
  else
    return NULL;
  }

gavl_dictionary_t * gavl_stream_get_metadata_nc(gavl_dictionary_t * dict)
  {
  gavl_value_t * val;
  if((val = gavl_dictionary_get_nc(dict, GAVL_META_METADATA)))
    return gavl_value_get_dictionary_nc(val);
  else
    return NULL;
  }


const gavl_dictionary_t * gavl_stream_get_metadata(const gavl_dictionary_t * dict)
  {
  const gavl_value_t * val;
  if((val = gavl_dictionary_get(dict, GAVL_META_METADATA)))
    return gavl_value_get_dictionary(val);
  else
    return NULL;
  }

gavl_dictionary_t * gavl_track_get_metadata_nc(gavl_dictionary_t * dict)
  {
  gavl_value_t * val;
  if((val = gavl_dictionary_get_nc(dict, GAVL_META_METADATA)))
    return gavl_value_get_dictionary_nc(val);
  else
    return NULL;
  }

const gavl_dictionary_t * gavl_track_get_metadata(const gavl_dictionary_t * dict)
  {
  const gavl_value_t * val;
  if((val = gavl_dictionary_get(dict, GAVL_META_METADATA)))
    return gavl_value_get_dictionary(val);
  else
    return NULL;
  }


static void get_stream_duration(void * priv, 
                                int idx,
                                const gavl_value_t * v)
  {
  gavl_time_t * t;
  gavl_time_t test_duration;
  const gavl_dictionary_t * s;
  const gavl_dictionary_t * m;
  int timescale;
  int64_t stream_duration;
  
  t = priv;
  
  if(!(s = gavl_value_get_dictionary(v)))
    return;
  
  if(!(m = gavl_dictionary_get_dictionary(s, GAVL_META_METADATA)))
    return;
    
  if(!(gavl_dictionary_get_int(m, GAVL_META_STREAM_SAMPLE_TIMESCALE, &timescale)))
    {
    //    fprintf(stderr, "Got no sample timescale\n");
    //    gavl_dictionary_dump(s, 2);
    return;
    }
  if((timescale <= 0))
    return;
  
  if(!(gavl_dictionary_get_long(m, GAVL_META_STREAM_DURATION, &stream_duration)))
    return;
  
  test_duration = gavl_time_unscale(timescale, stream_duration);
  if(test_duration > *t)
    *t = test_duration;
  }

void gavl_track_compute_duration(gavl_dictionary_t * dict)
  {
  gavl_array_t * arr;  
  gavl_time_t dur = 0;
  gavl_dictionary_t * m = gavl_track_get_metadata_nc(dict);
  
  if(gavl_dictionary_get_long(m, GAVL_META_APPROX_DURATION, &dur) && (dur > 0))
    return;
  
  if((arr = gavl_dictionary_get_array_nc(dict, GAVL_META_STREAMS)))
    gavl_array_foreach(arr, get_stream_duration, &dur);
  
  if(dur > 0)
    gavl_track_set_duration(dict, dur);
  
  }

/* Detect various patterns in filenames */

static int is_skip_char(char c)
  {
  return isspace(c) || (c == '_') || (c == '-');
  }

static const char * detect_date(const char * filename, gavl_dictionary_t * metadata)
  {
  int year;
  int month;
  int day;
  
  const char * pos = strrchr(filename, '(');
  if(!pos)
    return NULL;

  year = 9999;
  month = 99;
  day = 99;
  
  if(sscanf(pos, "(%d-%d-%d)", &year, &month, &day) < 3)
    {
    month = 99;
    day = 99;

    if(sscanf(pos, "(%d)", &year) < 1)
      {
      year = 9999;
      return NULL;
      }
    }
  pos--;
  while(is_skip_char(*pos) && (pos > filename))
    pos--;
  pos++;

  gavl_dictionary_set_date(metadata, GAVL_META_DATE, year, month, day);
  
  return pos;
  }

const char * gavl_detect_episode_tag(const char * filename, const char * end, 
                                     int * season_p, int * idx_p)
  {
  const char * pos;
  int season, idx;
  if(!end)
    end = filename + strlen(filename);

  pos = filename;
  
  while(pos < end)
    {
    if((sscanf(pos, "S%dE%d", &season, &idx) == 2) ||
       (sscanf(pos, "s%de%d", &season, &idx) == 2))
      {
      if(season_p)
        *season_p = season;
      if(idx_p)
        *idx_p = idx;
      return pos;
      }
    pos++;
    }
  return NULL;
  }

/* "Show - S1E2 - Episode (1990)" */

static int detect_episode(const char * filename, gavl_dictionary_t * dict)
  {
  int season = 0;
  int idx = 0;
  
  const char * tag;
  const char * pos;
  const char * end = detect_date(filename, dict);
  if(!end)
    end = filename + strlen(filename);

  tag = gavl_detect_episode_tag(filename, end, &season, &idx);
  if(!tag)
    return 0;

  pos = tag;
  while(!is_skip_char(*pos) && (pos < end))
    pos++;
  if(pos == end)
    return 0;
  while(is_skip_char(*pos) && (pos < end))
    pos++;

  if(pos == end)
    return 0;

  gavl_dictionary_set_string_nocopy(dict, GAVL_META_TITLE, gavl_strndup(pos, end));
  
  pos = tag;
  pos--;

  while(is_skip_char(*pos) && (pos > filename))
    pos--;

  if(pos == filename)
    {
    gavl_dictionary_set(dict, GAVL_META_TITLE, NULL);
    return 0;
    }
  pos++;
  
  gavl_dictionary_set_string_nocopy(dict, GAVL_META_SHOW, gavl_strndup(filename, pos));
  gavl_dictionary_set_int(dict, GAVL_META_SEASON, season);
  gavl_dictionary_set_int(dict, GAVL_META_EPISODENUMBER, idx);
  return 1;  
  }




/* "Movie title (1990)" */

static int detect_movie_singlefile(const char * filename, gavl_dictionary_t * dict)
  {
  const char * end;

  if(!(end = detect_date(filename, dict)))
    end = filename + strlen(filename);
  
  gavl_dictionary_set_string_nocopy(dict, GAVL_META_TITLE, gavl_strndup(filename, end));
  return 1;
  }

/* "Movie title (1990) CD1" */

static char * detect_multipart_tag(char * filename, int * part)
  {
  char * pos = filename + strlen(filename) - 3;

  while(pos > filename)
    {
    if(!strncasecmp(pos, "CD", 2) && isdigit(pos[2]))
      {
      if(part)
        *part = atoi(pos+2);
      return pos;
      }
    else if(!strncasecmp(pos, "part", 4) && isdigit(pos[4]))
      {
      if(part)
        *part = atoi(pos+4);
      return pos;
      }
    pos--;
    }
  return NULL;
  }

static int detect_movie_multifile(char * basename, gavl_dictionary_t * dict)
  {
  int idx = 0;
  char * pos;
  const char * end = basename + strlen(basename);
  
  pos = detect_multipart_tag(basename, &idx);
  
  if(!pos)
    return 0;
  
  end = detect_date(basename, dict);
  if(!end)
    {
    end = pos;
    end--;
 
    while(is_skip_char(*end) && (end > basename))
      end--;
    end++;
    }
  
  gavl_dictionary_set_string_nocopy(dict, GAVL_META_TITLE, gavl_strndup(basename, end));
  
  gavl_dictionary_set_string(dict, GAVL_META_LABEL, basename);
  gavl_dictionary_set_int(dict, GAVL_META_IDX, idx);

  pos--;
  while(is_skip_char(*pos) && (pos > basename))
    pos--;
  pos++;
  *pos = '\0';
  return 1;
  }

static void finalize_stream(void * data, int idx, const gavl_value_t * val)
  {
  gavl_stream_stats_t stats;
  gavl_compression_info_t ci;
  
  gavl_stream_type_t type;
  gavl_dictionary_t * m;
  int have_stats = 0;
  int have_ci = 0;
  gavl_dictionary_t * s = gavl_track_get_stream_all_nc(data, idx);
  
  if(!s)
    return;

  gavl_compression_info_init(&ci);
 
  if(gavl_stream_get_stats(s, &stats))
    have_stats = 1;

  if(gavl_stream_get_compression_info(s, &ci))
    have_ci = 1;

  gavl_stream_set_sample_timescale(s);

  
  m = gavl_stream_get_metadata_nc(s);
  
  type = gavl_stream_get_type(s);

  switch(type)
    {
    case GAVL_STREAM_AUDIO:
      {
      const gavl_audio_format_t * fmt = gavl_stream_get_audio_format(s);
      
      if(have_stats)
        gavl_stream_stats_apply_audio(&stats, fmt, (have_ci ? &ci : NULL), m);
      }
      break;
    case GAVL_STREAM_VIDEO:
      {
      gavl_video_format_t * fmt = gavl_stream_get_video_format_nc(s);
      if(have_stats)
        gavl_stream_stats_apply_video(&stats, fmt, (have_ci ? &ci : NULL), m);
      }
      break;
    case GAVL_STREAM_TEXT:

      if(have_stats)
        gavl_stream_stats_apply_subtitle(&stats, m);

      break;
    case GAVL_STREAM_OVERLAY:
      if(have_stats)
        gavl_stream_stats_apply_subtitle(&stats, m);
      break;
    case GAVL_STREAM_MSG:
      break;
    case GAVL_STREAM_NONE:
      break;
    }

  gavl_compression_info_free(&ci);
  
  }

static void finalize_audio(gavl_dictionary_t * dict)
  {
  const char * var;
  gavl_dictionary_t * m;
  const gavl_dictionary_t * sm;
  const gavl_audio_format_t * fmt;
  int val_i;
  
  if(!(m = gavl_track_get_metadata_nc(dict)) ||
     !(fmt = gavl_track_get_audio_format(dict, 0)) ||
     !(sm = gavl_track_get_audio_metadata(dict, 0)))
    return;
  
  if(fmt->num_channels)
    gavl_dictionary_set_int(m, GAVL_META_AUDIO_CHANNELS, fmt->num_channels);
  if(fmt->samplerate)
    {
    gavl_dictionary_set_int(m, GAVL_META_AUDIO_SAMPLERATE, fmt->samplerate);
    }
  if(gavl_dictionary_get_int(sm, GAVL_META_BITRATE, &val_i))
    gavl_dictionary_set_int(m, GAVL_META_AUDIO_BITRATE, val_i);
  
  if((var = gavl_dictionary_get_string(sm, GAVL_META_FORMAT)))
    gavl_dictionary_set_string(m, GAVL_META_AUDIO_CODEC, var);
  
  }

static void finalize_video(gavl_dictionary_t * dict)
  {
  const char * var;
  gavl_dictionary_t * m;
  const gavl_dictionary_t * sm;
  const gavl_video_format_t * fmt;
  
  if(!(m = gavl_track_get_metadata_nc(dict)) ||
     !(fmt = gavl_track_get_video_format(dict, 0)) ||
     !(sm = gavl_track_get_video_metadata(dict, 0)))
    return;

  if(fmt->image_width && fmt->image_height)
    {
    gavl_dictionary_set_int(m, GAVL_META_WIDTH, fmt->image_width);
    gavl_dictionary_set_int(m, GAVL_META_HEIGHT, fmt->image_height);
    }

  
  if((var = gavl_dictionary_get_string(sm, GAVL_META_FORMAT)))
    gavl_dictionary_set_string(m, GAVL_META_VIDEO_CODEC, var);
  }

static void add_country_flag(gavl_dictionary_t * m,
                             const char * code,
                             int width)
  {
  //  gavl_dictionary_t * u;
  
  //  char * uri = gavl_sprintf("https://www.countryflags.io/%s/flat/%d.png", code, size);

  char * uri = gavl_sprintf("https://flagcdn.com/w%d/%c%c.png", width, tolower(code[0]), tolower(code[1]));
  
  gavl_metadata_add_src(m, GAVL_META_ICON_URL, "image/png", uri);

  //  gavl_dictionary_set_int(u, GAVL_META_WIDTH, size);
  //  gavl_dictionary_set_int(u, GAVL_META_HEIGHT, size);
  
  free(uri);
  }

void gavl_track_finalize(gavl_dictionary_t * track)
  {
  gavl_time_t duration = GAVL_TIME_UNDEFINED;
  
  const char * media_class = NULL;
  const char * countrycode = NULL;
  gavl_dictionary_t * m;
  //  gavl_dictionary_t * ;

  const char * var;
  int num_audio_streams;
  int num_video_streams;
  int num_subtitle_streams;
  char * basename = NULL;
  
  const char * location = NULL;
  const char * pos1;
  const char * pos2;

  gavl_array_t * arr;
  
  if((arr = gavl_dictionary_get_array_nc(track, GAVL_META_VARIANTS)))
    gavl_sort_tracks_by_quality(arr);
  
  if((arr = gavl_dictionary_get_array_nc(track, GAVL_META_STREAMS)))
    {
    gavl_array_foreach(arr, finalize_stream, track);
    }
  
  if(!(m = gavl_track_get_metadata_nc(track)))
    return;
  
  //  fprintf(stderr, "gavl_track_finalize %s\n",
  //          gavl_dictionary_get_string(m, GAVL_META_CLASS));

  num_audio_streams = gavl_track_get_num_audio_streams(track);
  num_video_streams = gavl_track_get_num_video_streams(track);
  num_subtitle_streams =
    gavl_track_get_num_text_streams(track) +
    gavl_track_get_num_overlay_streams(track);
  
  gavl_metadata_get_src(m, GAVL_META_SRC, 0,
                          NULL, &location);
  
  if(location && (pos1 = strrchr(location, '/')) &&
     (pos2 = strrchr(location, '.')) && (pos2 > pos1))
    {
    basename = gavl_strndup(pos1+1, pos2);
    }
  
  if(location && !gavl_dictionary_get(m, GAVL_META_HASH))
    {
    char hash[GAVL_MD5_LENGTH];
    gavl_md5_buffer_str(location, strlen(location), hash);
    gavl_dictionary_set_string(m, GAVL_META_HASH, hash);
    }
  
  media_class = gavl_dictionary_get_string(m, GAVL_META_CLASS);

  if(!media_class)
    {
    /* Figure out the media type */
    if((num_audio_streams == 1) && !num_video_streams)
      {
      /* Audio file */
      media_class = GAVL_META_CLASS_AUDIO_FILE;
      }
    else if(!num_audio_streams && !num_video_streams && num_subtitle_streams)
      {
      /* Subtitle file */
      media_class = GAVL_META_CLASS_SUBTITLE_FILE;
      }
    else if(num_video_streams >= 1)
      {
      const gavl_video_format_t * fmt;

      if(!num_audio_streams &&
         (num_video_streams == 1) &&
         (fmt = gavl_track_get_video_format(track, 0)) && 
         (fmt->framerate_mode == GAVL_FRAMERATE_STILL))
        {
        /* Photo */
        media_class = GAVL_META_CLASS_IMAGE;
        }
      else
        {
        /* Video */
        media_class = GAVL_META_CLASS_VIDEO_FILE;
        }
      }
    }
  
  gavl_track_compute_duration(track);
  
  if(media_class)
    {
    if(!strcmp(media_class, GAVL_META_CLASS_AUDIO_FILE))
      {
      finalize_audio(track);
      
      /* Check for audio broadcast */
      if(gavl_dictionary_get(m, GAVL_META_STATION))
        media_class = GAVL_META_CLASS_AUDIO_BROADCAST;

      /* Check for song */
      else if(gavl_dictionary_get(m, GAVL_META_ARTIST) &&
              gavl_dictionary_get(m, GAVL_META_TITLE) &&
              gavl_dictionary_get(m, GAVL_META_ALBUM))
        {
        media_class = GAVL_META_CLASS_SONG;
        }
      }
    else if(!strcmp(media_class, GAVL_META_CLASS_IMAGE))
      {
      
      }
    else if(!strcmp(media_class, GAVL_META_CLASS_VIDEO_FILE))
      {
      finalize_video(track);
      if(num_audio_streams > 0)
        finalize_audio(track);
      
      if((num_video_streams == 1))
        {
        if(gavl_dictionary_get_long(m, GAVL_META_APPROX_DURATION, &duration) && (duration > 0))
          {
          /* Check for episode */

          if(basename && detect_episode(basename, m))
            media_class = GAVL_META_CLASS_TV_EPISODE;
          /* Check for movie */
          else if(basename && detect_movie_multifile(basename, m))
            media_class = GAVL_META_CLASS_MOVIE_PART;
          else if(basename && detect_movie_singlefile(basename, m))
            media_class = GAVL_META_CLASS_MOVIE;
          }
        else if(gavl_dictionary_get(m, GAVL_META_STATION))
          media_class = GAVL_META_CLASS_VIDEO_BROADCAST;
        }
      }
    }

  if(media_class &&
     (!(var = gavl_dictionary_get_string(m, GAVL_META_CLASS)) ||
      strcmp(var, media_class)))
    gavl_dictionary_set_string(m, GAVL_META_CLASS, media_class);
  
  if(basename)
    free(basename);

  media_class = gavl_dictionary_get_string(m, GAVL_META_CLASS);
  
  /* Add country flag */
  
  if(media_class &&
     !strcmp(media_class, GAVL_META_CLASS_CONTAINER_COUNTRY) &&
     !gavl_dictionary_get(m, GAVL_META_ICON_URL))
    {
    if(!(countrycode = gavl_dictionary_get_string(m, GAVL_META_COUNTRY_CODE_2)))
      {
      const char * label = gavl_dictionary_get_string(m, GAVL_META_LABEL);
      if(label)
        countrycode = gavl_get_country_code_2_from_label(label);
      }

    if(countrycode)
      {
      add_country_flag(m, countrycode, 160);
      add_country_flag(m, countrycode, 20);
      add_country_flag(m, countrycode, 40);
      add_country_flag(m, countrycode, 320);
      add_country_flag(m, countrycode, 640);
      }
    
    }

  /* Simplify one-variant streams and one-part movies */

  if(gavl_track_get_num_variants(track) == 1)
    {
    const gavl_dictionary_t * vm = gavl_track_get_metadata(gavl_track_get_variant(track, 0));
    gavl_dictionary_set(m, GAVL_META_SRC, gavl_dictionary_get(vm, GAVL_META_SRC));
    gavl_dictionary_set(track, GAVL_META_VARIANTS, NULL);
    }
  else if(gavl_track_get_num_parts(track) == 1)
    {
    const gavl_dictionary_t * pm = gavl_track_get_metadata(gavl_track_get_part(track, 0));
    gavl_dictionary_set(m, GAVL_META_SRC, gavl_dictionary_get(pm, GAVL_META_SRC));
    gavl_dictionary_set(track, GAVL_META_PARTS, NULL);
    }

  /* Set default label */

  gavl_track_set_label(track);
  
  }

void gavl_track_set_label(gavl_dictionary_t * dict)
  {
  const char * klass;
  const char * var;
  gavl_dictionary_t * m;
  
  if(!(m = gavl_track_get_metadata_nc(dict)))
    return;

  if(gavl_dictionary_get_string(m, GAVL_META_LABEL))
    return;

  klass = gavl_dictionary_get_string(m, GAVL_META_CLASS);
  
  if(klass)
    {
    if(!strcmp(klass, GAVL_META_CLASS_SONG))
      {
      const char * title;
      char * artists;

      if((title = gavl_dictionary_get_string(m, GAVL_META_TITLE)) &&
         (artists = gavl_metadata_join_arr(m, GAVL_META_ARTIST, ", ")))
        {
        gavl_dictionary_set_string_nocopy(m, GAVL_META_LABEL, gavl_sprintf("%s - %s",
                                                                           artists, title));
        free(artists);
        }
      
      }
    else if(!strcmp(klass, GAVL_META_CLASS_MOVIE))
      {
      const char * title;
      int year;

      if((year = gavl_dictionary_get_year(m, GAVL_META_DATE)) &&
         (title = gavl_dictionary_get_string(m, GAVL_META_TITLE)))
        gavl_dictionary_set_string_nocopy(m, GAVL_META_LABEL, gavl_sprintf("%s (%d)", title, year));
      }
    }

  if(gavl_dictionary_get_string(m, GAVL_META_LABEL))
    return;

  if((var = gavl_dictionary_get_string(m, GAVL_META_TITLE)) ||
     (var = gavl_dictionary_get_string(m, GAVL_META_STATION)))
    {
    gavl_dictionary_set_string(m, GAVL_META_LABEL, var);
    return;
    }

  if((gavl_metadata_get_src(m, GAVL_META_SRC, 0, NULL, &var)))
    {
    const char * pos1;
    const char * pos2;
    
    if(strstr(var, "://"))
      {
      gavl_dictionary_set_string(m, GAVL_META_LABEL, var);
      return;
      }

    if(!strstr(var, "track=") && (pos1 = strrchr(var, '/')) && (pos2 = strrchr(var, '.')) &&
       (pos2 > pos1))
      {
      if(gavl_dictionary_set_string_nocopy(m, GAVL_META_LABEL, gavl_strndup(pos1 + 1, pos2)))
        return;
      }
    }
  
  if(gavl_dictionary_set_string(m, GAVL_META_LABEL, "Unnamed track"))
    return;
  
  }
  
gavl_time_t gavl_track_get_duration(const gavl_dictionary_t * dict)
  {
  gavl_time_t dur;
  const gavl_dictionary_t * m;

  if(!(m = gavl_track_get_metadata(dict)))
    return GAVL_TIME_UNDEFINED;
  
  if(gavl_dictionary_get_long(m, GAVL_META_APPROX_DURATION, &dur) && (dur > 0))
    return dur;
  else
    return GAVL_TIME_UNDEFINED;
  }

void gavl_track_set_duration(gavl_dictionary_t * dict, gavl_time_t dur)
  {
  gavl_dictionary_t * m = gavl_track_get_metadata_nc(dict);
  gavl_dictionary_set_long(m, GAVL_META_APPROX_DURATION, dur);
  }

void gavl_track_set_num_audio_streams(gavl_dictionary_t * dict, int num)
  {
  int i;

  /* Delete previous streams */
  while(gavl_track_delete_audio_stream(dict, 0))
    ;
  
  for(i = 0; i < num; i++)
    gavl_track_append_audio_stream(dict);
  }

void gavl_track_set_num_video_streams(gavl_dictionary_t * dict, int num)
  {
  int i;

  /* Delete previous streams */
  while(gavl_track_delete_video_stream(dict, 0))
    ;
  
  for(i = 0; i < num; i++)
    gavl_track_append_video_stream(dict);
  
  }

void gavl_track_set_num_text_streams(gavl_dictionary_t * dict, int num)
  {
  int i;

  /* Delete previous streams */
  while(gavl_track_delete_text_stream(dict, 0))
    ;

  for(i = 0; i < num; i++)
    gavl_track_append_text_stream(dict);
  
  }

void gavl_track_set_num_overlay_streams(gavl_dictionary_t * dict, int num)
  {
  int i;

  /* Delete previous streams */
  while(gavl_track_delete_overlay_stream(dict, 0))
    ;

  for(i = 0; i < num; i++)
    gavl_track_append_overlay_stream(dict);
  
  }

int gavl_track_can_seek(const gavl_dictionary_t * track)
  {
  int val = 0;
  const gavl_dictionary_t * m;

  if(!(m = gavl_track_get_metadata(track)) ||
     !gavl_dictionary_get_int(m, GAVL_META_CAN_SEEK, &val) ||
     !val)
    return 0;

  return 1;
  }


int gavl_track_can_pause(const gavl_dictionary_t * track)
  {
  int val = 0;
  const gavl_dictionary_t * m;

  if(!(m = gavl_track_get_metadata(track)) ||
     !gavl_dictionary_get_int(m, GAVL_META_CAN_PAUSE, &val) ||
     !val)
    return 0;

  return 1;
  
  }

int gavl_track_is_async(const gavl_dictionary_t * track)
  {
  int val = 0;
  const gavl_dictionary_t * m;

  if(!(m = gavl_track_get_metadata(track)) ||
     !gavl_dictionary_get_int(m, GAVL_META_ASYNC, &val) ||
     !val)
    return 0;

  return 1;
  
  }

int gavl_track_set_async(gavl_dictionary_t * track, int async)
  {
  gavl_dictionary_t * m;
  
  if(!(m = gavl_track_get_metadata_nc(track)))
    return 0;

  if(async)
    gavl_dictionary_set_int(m, GAVL_META_ASYNC, async);
  else
    gavl_dictionary_set(m, GAVL_META_ASYNC, NULL);
  
  return 1;
  }


#define META_GUI "gui"

void gavl_track_set_gui_state(gavl_dictionary_t * track, const char * state, int val)
  {
  int old_val = 0;
  gavl_dictionary_t * gui = gavl_dictionary_get_dictionary_create(track, META_GUI);

  gavl_dictionary_get_int(track, state, &old_val);
  
  if((val < 0) || (val > 1))
    val = !old_val;
  
  gavl_dictionary_set_int(gui, state, val);
  }

void gavl_tracks_set_gui_state(gavl_array_t * tracks, const char * state, int val, int start, int end)
  {
  int i;
  gavl_dictionary_t * track;
  
  if(end < 0)
    end = tracks->num_entries;

  for(i = start; i < end; i++)
    {
    if((track = gavl_value_get_dictionary_nc(&tracks->entries[i])))
      gavl_track_set_gui_state(track, state, val);
    }
  }


int gavl_track_get_gui_state(const gavl_dictionary_t * track, const char * state)
  {
  int ret = 0;
  const gavl_dictionary_t * gui = gavl_dictionary_get_dictionary(track, META_GUI);

  if(!gui || !gavl_dictionary_get_int(gui, state, &ret))
    return 0;
  
  return ret;
  }

void gavl_track_clear_gui_state(gavl_dictionary_t * track)
  {
  gavl_dictionary_set(track, META_GUI, NULL);
  }

void gavl_track_copy_gui_state(gavl_dictionary_t * dst, const gavl_dictionary_t * src)
  {
  gavl_dictionary_t * dst_gui;
  gavl_dictionary_t tmp;
  
  const gavl_dictionary_t * src_gui = gavl_dictionary_get_dictionary(src, META_GUI);

  if(!src_gui)
    return;
  
  dst_gui = gavl_dictionary_get_dictionary_create(dst, META_GUI);

  gavl_dictionary_init(&tmp);
  
  gavl_dictionary_merge(&tmp, src_gui, dst_gui);
  gavl_dictionary_reset(dst_gui);
  gavl_dictionary_move(dst_gui, &tmp);
  }

void gavl_track_merge(gavl_dictionary_t * dst, const gavl_dictionary_t * src)
  {
  int num_streams;
  int i;
  const gavl_audio_format_t * afmt_src;
  gavl_audio_format_t * afmt_dst;

  const gavl_video_format_t * vfmt_src;
  gavl_video_format_t * vfmt_dst;
  
  const gavl_dictionary_t * src_m;
  gavl_dictionary_t * dst_m;
  
  const gavl_dictionary_t * src_s;
  gavl_dictionary_t * dst_s;

  gavl_compression_info_t ci;
  
  src_m = gavl_track_get_metadata(src);
  dst_m = gavl_track_get_metadata_nc(dst);

  gavl_dictionary_merge2(dst_m, src_m);

  num_streams = gavl_track_get_num_streams_all(src);

  for(i = 0; i < num_streams; i++)
    {
    if(!(src_s = gavl_track_get_stream_all(src, i)) ||
       !(dst_s = gavl_track_get_stream_all_nc(dst, i)))
      break;

    src_m = gavl_stream_get_metadata(src_s);
    dst_m = gavl_stream_get_metadata_nc(dst_s);
    gavl_dictionary_merge2(dst_m, src_m);

    if((afmt_src = gavl_stream_get_audio_format(src_s)) &&
       (afmt_dst = gavl_stream_get_audio_format_nc(dst_s)))
      gavl_audio_format_copy(afmt_dst, afmt_src);
    
    if((vfmt_src = gavl_stream_get_video_format(src_s)) &&
       (vfmt_dst = gavl_stream_get_video_format_nc(dst_s)))
      gavl_video_format_copy(vfmt_dst, vfmt_src);

    gavl_compression_info_init(&ci);
    if(gavl_stream_get_compression_info(src, &ci))
      {
      gavl_stream_set_compression_info(dst, &ci);
      gavl_compression_info_free(&ci);
      }
    }
  
  }

void gavl_track_update_children(gavl_dictionary_t * dict)
  {
  int i;
  const char * klass;
  gavl_dictionary_t * m;
  gavl_dictionary_t * tm;
  gavl_dictionary_t * track;
  gavl_array_t * arr;
  int num_items = 0;
  int num_containers = 0;

  gavl_time_t duration = 0;
  gavl_time_t track_duration = GAVL_TIME_UNDEFINED;
  
  arr = gavl_get_tracks_nc(dict);
  
  m = gavl_dictionary_get_dictionary_create(dict, GAVL_META_METADATA);

  //  fprintf(stderr, "gavl_track_update_children %p\n", dict);
  //  gavl_dictionary_dump(dict, 2);
  
  for(i = 0; i < arr->num_entries; i++)
    {
    //    if(!())
    //      fprintf(stderr, "Blupp %d %p %d %p\n", i, arr, arr->num_entries, arr->entries);

    if((track = gavl_get_track_nc(dict, i)) &&
       (tm = gavl_dictionary_get_dictionary_create(track, GAVL_META_METADATA)))
      {
      gavl_dictionary_set_int(tm, GAVL_META_IDX, i);
      gavl_dictionary_set_int(tm, GAVL_META_TOTAL, arr->num_entries);
      
      if((klass = gavl_dictionary_get_string(tm, GAVL_META_CLASS)))
        {
        if(gavl_string_starts_with(klass, "item"))
          num_items++;
        else if(gavl_string_starts_with(klass, "container"))
          num_containers++;
        }

      if(duration != GAVL_TIME_UNDEFINED)
        {
        if(gavl_dictionary_get_long(tm, GAVL_META_APPROX_DURATION, &track_duration) &&
           (track_duration > 0))
          duration += track_duration;
        else
          duration = GAVL_TIME_UNDEFINED;
        }
      
      }
    }

  gavl_dictionary_set_long(m, GAVL_META_APPROX_DURATION, duration); 
  
  if(num_items + num_containers == arr->num_entries)
    gavl_track_set_num_children(dict, num_containers, num_items);
  else // Fallback
    gavl_track_set_num_children(dict, arr->num_entries, arr->num_entries);
  
  }

const char * gavl_track_get_id(const gavl_dictionary_t * dict)
  {
  const gavl_dictionary_t * m;

  if((m = gavl_track_get_metadata(dict)))
    return gavl_dictionary_get_string(m, GAVL_META_ID);
  else
    return NULL;
  }

const char * gavl_track_get_media_class(const gavl_dictionary_t * dict)
  {
  const gavl_dictionary_t * m;

  if((m = gavl_track_get_metadata(dict)))
    return gavl_dictionary_get_string(m, GAVL_META_CLASS);
  else
    return NULL;
  }

void gavl_track_set_lock(gavl_dictionary_t * track,
                         int lock)
  {
  gavl_dictionary_t * m;
  m = gavl_track_get_metadata_nc(track);

  //  if(lock)
  gavl_dictionary_set_int(m, GAVL_META_LOCKED, lock);
  //  else
  //    gavl_dictionary_set(m, GAVL_META_LOCKED, NULL);
  }

int gavl_track_is_locked(const gavl_dictionary_t * track)
  {
  int locked = 0;
  const gavl_dictionary_t * m;
  
  if((m = gavl_track_get_metadata(track)) &&
     (gavl_dictionary_get_int(m, GAVL_META_LOCKED, &locked)))
    return locked;
  else
    return 0;
  }
 

void gavl_track_set_id_nocopy(gavl_dictionary_t * dict, char * id)
  {
  gavl_dictionary_t * m = gavl_dictionary_get_dictionary_create(dict, GAVL_META_METADATA);
  gavl_dictionary_set_string_nocopy(m, GAVL_META_ID, id);
  }

void gavl_track_set_id(gavl_dictionary_t * dict, const char * id)
  {
  gavl_track_set_id_nocopy(dict, gavl_strdup(id));
  }

int gavl_get_track_idx_by_id(const gavl_dictionary_t * dict, const char * id)
  {
  int i;
  const gavl_dictionary_t * track;
  const gavl_dictionary_t * m;
  const char * track_id;

  int num = gavl_get_num_tracks(dict);
  
  for(i = 0; i < num; i++)
    {
    if((track = gavl_get_track(dict, i)) &&
       (m = gavl_track_get_metadata(track)) &&
       (track_id = gavl_dictionary_get_string(m, GAVL_META_ID)) &&
       !strcmp(track_id, id))
      return i;
    }
  return -1;
  }

const gavl_dictionary_t * gavl_get_track_by_string(const gavl_dictionary_t * dict, const char * tag,
                                                   const char * val)
  {
  int i;
  const gavl_dictionary_t * track;
  const gavl_dictionary_t * m;
  const char * track_val;

  int num = gavl_get_num_tracks(dict);
  
  for(i = 0; i < num; i++)
    {
    if((track = gavl_get_track(dict, i)) &&
       (m = gavl_track_get_metadata(track)) &&
       (track_val = gavl_dictionary_get_string(m, tag)) &&
       !strcmp(track_val, val))
      return track;
    }
  return NULL;
  
  }

const gavl_dictionary_t * gavl_get_track_by_id(const gavl_dictionary_t * dict, const char * id)
  {
  return gavl_get_track_by_string(dict, GAVL_META_ID, id);
  }

const gavl_dictionary_t * gavl_get_track_by_string_arr(const gavl_array_t * arr,
                                                       const char * tag, const char * val)
  {
  int i;
  const gavl_dictionary_t * track;
  const gavl_dictionary_t * m;
  const char * track_val;
  
  for(i = 0; i < arr->num_entries; i++)
    {
    if((track = gavl_value_get_dictionary(&arr->entries[i])) &&
       (m = gavl_track_get_metadata(track)) &&
       (track_val = gavl_dictionary_get_string(m, tag)) &&
       !strcmp(track_val, val))
      return track;
    }
  return NULL;
  }

const gavl_dictionary_t * gavl_get_track_by_id_arr(const gavl_array_t * arr, const char * id)
  {
  return gavl_get_track_by_string_arr(arr, GAVL_META_ID, id);
  }

gavl_dictionary_t * gavl_get_track_by_id_arr_nc(gavl_array_t * arr, const char * id)
  {
  int i;
  gavl_dictionary_t * track;
  const gavl_dictionary_t * m;
  const char * track_id;
  
  for(i = 0; i < arr->num_entries; i++)
    {
    if((track = gavl_value_get_dictionary_nc(&arr->entries[i])) &&
       (m = gavl_track_get_metadata(track)) &&
       (track_id = gavl_dictionary_get_string(m, GAVL_META_ID)) &&
       !strcmp(track_id, id))
      return track;
    }
  return NULL;
  }

int gavl_get_track_idx_by_id_arr(const gavl_array_t * arr, const char * id)
  {
  int i;
  const gavl_dictionary_t * track;
  const gavl_dictionary_t * m;
  const char * track_id;

  for(i = 0; i < arr->num_entries; i++)
    {
    if((track = gavl_value_get_dictionary_nc(&arr->entries[i])) &&
       (m = gavl_track_get_metadata(track)) &&
       (track_id = gavl_dictionary_get_string(m, GAVL_META_ID)) &&
       !strcmp(track_id, id))
      return i;
    }
  return -1;
  }


gavl_dictionary_t * gavl_get_track_by_id_nc(gavl_dictionary_t * dict, const char * id)
  {
  int i;
  gavl_dictionary_t * track;
  const gavl_dictionary_t * m;
  const char * track_id;
  int num = gavl_get_num_tracks(dict);
  
  for(i = 0; i < num; i++)
    {
    if((track = gavl_get_track_nc(dict, i)) &&
       (m = gavl_track_get_metadata(track)) &&
       (track_id = gavl_dictionary_get_string(m, GAVL_META_ID)) &&
       !strcmp(track_id, id))
      return track;
    }
  return NULL;
  }

static int compare_metadata_string(const void * p1, const void * p2, void * data)
  {
  const char * s1;
  const char * s2;
  
  const gavl_dictionary_t * dict1;
  const gavl_dictionary_t * dict2;

  if(!(dict1 = gavl_value_get_dictionary(p1)) ||
     !(dict2 = gavl_value_get_dictionary(p2)) ||
     !(dict1 = gavl_track_get_metadata(dict1)) ||
     !(dict2 = gavl_track_get_metadata(dict2)) ||
     !(s1 = gavl_dictionary_get_string(dict1, data)) ||
     !(s2 = gavl_dictionary_get_string(dict2, data)))
    return 0;
  
  return strcoll(s1, s2);
  }
  
void gavl_sort_tracks_by_label(gavl_array_t * arr)
  {
  gavl_array_sort(arr, compare_metadata_string, GAVL_META_LABEL);
  }



static int compare_metadata_int_reverse(const void * p1,
                                        const void * p2, void * data)
  {
  int i1 = 0;
  int i2 = 0;
  
  const gavl_dictionary_t * dict1;
  const gavl_dictionary_t * dict2;

  if(!(dict1 = gavl_value_get_dictionary(p1)) ||
     !(dict2 = gavl_value_get_dictionary(p2)) ||
     !(dict1 = gavl_track_get_metadata(dict1)) ||
     !(dict2 = gavl_track_get_metadata(dict2)) ||
     !gavl_dictionary_get_int(dict1, data, &i1) ||
     !gavl_dictionary_get_int(dict2, data, &i2))
    return 0;
  
  return (i2 - i1);
  }

void gavl_sort_tracks_by_bitrate(gavl_array_t * arr)
  {
  gavl_array_sort(arr, compare_metadata_int_reverse, GAVL_META_BITRATE);
  }

static int compare_quality(const void * p1,
                           const void * p2, void * data)
  {
  int i1 = 0;
  int i2 = 0;
  
  const gavl_dictionary_t * dict1;
  const gavl_dictionary_t * dict2;

  if(!(dict1 = gavl_value_get_dictionary(p1)) ||
     !(dict2 = gavl_value_get_dictionary(p2)) ||
     !(dict1 = gavl_track_get_metadata(dict1)) ||
     !(dict2 = gavl_track_get_metadata(dict2)))
    return 0;

  if(gavl_dictionary_get_int(dict1, GAVL_META_BITRATE, &i1) &&
     gavl_dictionary_get_int(dict2, GAVL_META_BITRATE, &i2) &&
     (i1 != i2))
    return (i2 - i1);

  if(gavl_dictionary_get_int(dict1, GAVL_META_WIDTH, &i1) &&
     gavl_dictionary_get_int(dict2, GAVL_META_WIDTH, &i2) &&
     (i1 != i2))
    return (i2 - i1);

  if(gavl_dictionary_get_int(dict1, GAVL_META_AUDIO_SAMPLERATE, &i1) &&
     gavl_dictionary_get_int(dict2, GAVL_META_AUDIO_SAMPLERATE, &i2) &&
     (i1 != i2))
    return (i2 - i1);

  if(gavl_dictionary_get_int(dict1, GAVL_META_AUDIO_CHANNELS, &i1) &&
     gavl_dictionary_get_int(dict2, GAVL_META_AUDIO_CHANNELS, &i2) &&
     (i1 != i2))
    return (i2 - i1);
  
  return 0;
  }


void gavl_sort_tracks_by_quality(gavl_array_t * arr)
  {
  gavl_array_sort(arr, compare_quality, NULL);
  }

/* Compression info */

#if 0

  uint32_t flags; //!< ORed combination of GAVL_COMPRESSION_* flags
  gavl_codec_id_t id; //!< Codec ID
  
  uint8_t * global_header; //!< Global header
  uint32_t global_header_len;  //!< Length of global header
  
  int32_t bitrate;             //!< Needed by some codecs, negative values mean VBR
  int palette_size;            //!< Size of the embedded palette for image codecs
  int pre_skip;                //!< Samples to skip at the start

  uint32_t video_buffer_size;   //!< VBV buffer size for video (in BYTES)
  
  uint32_t max_packet_size;     //!< Maximum packet size or 0 if unknown

  uint32_t max_ref_frames;      //!< Maximum reference frames (if > 2)
#endif

#define COMPRESSION_INFO_KEY                 GAVL_META_STREAM_COMPRESSION_INFO
#define COMPRESSION_INFO_KEY_FLAGS           "flg"
#define COMPRESSION_INFO_KEY_ID              "id"
#define COMPRESSION_INFO_KEY_BITRATE         "br"
#define COMPRESSION_INFO_KEY_PALETTE_SIZE    "ps"
#define COMPRESSION_INFO_KEY_TAG             "tag"
#define COMPRESSION_INFO_KEY_BLOCK_ALIGN     "block_align"
#define COMPRESSION_INFO_KEY_HEAD            "head"

#define GET_INT(key, member) gavl_dictionary_get_int(cmp, key, &ret->member)

#define SET_INT(key, member) if(info->member) gavl_dictionary_set_int(cmp, key, info->member)

int gavl_stream_get_compression_info(const gavl_dictionary_t * s,
                                     gavl_compression_info_t * ret)
  {
  int val_i = GAVL_CODEC_ID_NONE;
  const gavl_buffer_t * buf;
  const gavl_dictionary_t * cmp;
  
  if(!(cmp = gavl_dictionary_get_dictionary(s, COMPRESSION_INFO_KEY)))
    return 0;
  
  if(!ret)
    return 1;

  if(gavl_dictionary_get_int(cmp, COMPRESSION_INFO_KEY_ID, &val_i))
    ret->id = val_i;
  
  if(gavl_dictionary_get_int(cmp, COMPRESSION_INFO_KEY_TAG, &val_i))
    ret->codec_tag = val_i;
  
  GET_INT(COMPRESSION_INFO_KEY_FLAGS,   flags);
  GET_INT(COMPRESSION_INFO_KEY_BITRATE, bitrate);
  GET_INT(COMPRESSION_INFO_KEY_PALETTE_SIZE, palette_size);
  GET_INT(COMPRESSION_INFO_KEY_BLOCK_ALIGN, block_align);
  
  if((buf = gavl_dictionary_get_binary(cmp, COMPRESSION_INFO_KEY_HEAD)) &&
     (buf->len > 0))
    {
    gavl_buffer_reset(&ret->codec_header);
    gavl_buffer_append_pad(&ret->codec_header, buf, GAVL_PACKET_PADDING);
    }
  return 1;
  }
  
int gavl_track_get_audio_compression_info(const gavl_dictionary_t * t, int idx,
                                          gavl_compression_info_t * ret)
  {
  if(!(t = gavl_track_get_audio_stream(t, idx)))
    return 0;
  
  return gavl_stream_get_compression_info(t, ret);
  }

int gavl_track_get_video_compression_info(const gavl_dictionary_t * t, int idx,
                                          gavl_compression_info_t * ret)
  {
  if(!(t = gavl_track_get_video_stream(t, idx)))
    return 0;

  return gavl_stream_get_compression_info(t, ret);
  }

int gavl_track_get_overlay_compression_info(const gavl_dictionary_t * t, int idx,
                                            gavl_compression_info_t * ret)
  {
  if(!(t = gavl_track_get_overlay_stream(t, idx)))
    return 0;
  
  return gavl_stream_get_compression_info(t, ret);
  }

void gavl_stream_set_compression_info(gavl_dictionary_t * s, const gavl_compression_info_t * info)
  {
  gavl_dictionary_t * cmp;

#if 0  
  if(info->codec_header.len && !info->id)
    {
    fprintf(stderr, "gavl_stream_set_compression_info: %p\n", s);
    gavl_compression_info_dump(info);
    }
#endif
  
  cmp = gavl_dictionary_get_dictionary_create(s, COMPRESSION_INFO_KEY);
  gavl_dictionary_reset(cmp);
  
  SET_INT(COMPRESSION_INFO_KEY_FLAGS,   flags);
  SET_INT(COMPRESSION_INFO_KEY_ID,      id);
  SET_INT(COMPRESSION_INFO_KEY_TAG,     codec_tag);
  SET_INT(COMPRESSION_INFO_KEY_BITRATE, bitrate);
  SET_INT(COMPRESSION_INFO_KEY_PALETTE_SIZE, palette_size);
  SET_INT(COMPRESSION_INFO_KEY_BLOCK_ALIGN, block_align);
  
  if(info->codec_header.len > 0)
    {
    gavl_value_t val;
    gavl_buffer_t * buf;

    gavl_value_init(&val);
    buf = gavl_value_set_binary(&val);
    gavl_buffer_copy(buf, &info->codec_header);
    gavl_dictionary_set_nocopy(cmp, COMPRESSION_INFO_KEY_HEAD, &val);
    }
  }

int gavl_stream_get_sample_timescale(const gavl_dictionary_t * s)
  {
  gavl_stream_type_t t;
  
  t = gavl_stream_get_type(s);

  switch(t)
    {
    case GAVL_STREAM_AUDIO:
      {
      const gavl_audio_format_t * afmt;

      if((afmt = gavl_dictionary_get_audio_format(s, GAVL_META_STREAM_FORMAT)))
        return afmt->samplerate;
      }
      break;
    case GAVL_STREAM_TEXT:
    case GAVL_STREAM_MSG:
      {
      int scale = 0;
      const gavl_dictionary_t * m = gavl_stream_get_metadata(s);
      if(gavl_dictionary_get_int(m, GAVL_META_STREAM_SAMPLE_TIMESCALE, &scale))
        return scale;
      }
      break;
    case GAVL_STREAM_OVERLAY:
    case GAVL_STREAM_VIDEO:
      {
      const gavl_video_format_t * vfmt;

      if((vfmt = gavl_dictionary_get_video_format(s, GAVL_META_STREAM_FORMAT)))
        return vfmt->timescale;
      }
      break;
    case GAVL_STREAM_NONE:
      return 0;
    }
  return 0;
  }

/* 
void gavl_sort_tracks_by_metadata_long(gavl_dictionary_t * dict,
                                       const char * str)
  {
  
  }

*/

int
gavl_stream_get_pts_range(const gavl_dictionary_t * s, int64_t * start, int64_t * end)
  {
  gavl_stream_stats_t stats;
  if(!gavl_stream_get_stats(s, &stats))
    return 0;

  if((stats.pts_start == GAVL_TIME_UNDEFINED) ||
     (stats.pts_end == GAVL_TIME_UNDEFINED))
    return 0;
  
  *start = stats.pts_start;
  *end = stats.pts_end;
  
  return 1;
  }

int gavl_track_get_num_children(const gavl_dictionary_t * track)
  {
  int ret = 0;
  const gavl_dictionary_t * m;
  if(!(m = gavl_track_get_metadata(track)) ||
     !gavl_dictionary_get_int(m, GAVL_META_NUM_CHILDREN, &ret))
    return 0;
  return ret;
  }


int gavl_track_get_num_item_children(const gavl_dictionary_t * track)
  {
  int ret = 0;
  const gavl_dictionary_t * m;
  if(!(m = gavl_track_get_metadata(track)) ||
     !gavl_dictionary_get_int(m, GAVL_META_NUM_ITEM_CHILDREN, &ret))
    return 0;
  return ret;
  }

int gavl_track_get_num_container_children(const gavl_dictionary_t * track)
  {
  int ret = 0;
  const gavl_dictionary_t * m;
  if(!(m = gavl_track_get_metadata(track)) ||
     !gavl_dictionary_get_int(m, GAVL_META_NUM_CONTAINER_CHILDREN, &ret))
    return 0;
  return ret;
  }

void gavl_track_set_num_children(gavl_dictionary_t * track,
                                 int num_container_children,
                                 int num_item_children)
  {
  gavl_dictionary_t * m;

  if(!(m = gavl_track_get_metadata_nc(track)))
    return;

  if(num_container_children)
    gavl_dictionary_set_int(m, GAVL_META_NUM_CONTAINER_CHILDREN, num_container_children);
  else
    gavl_dictionary_set(m, GAVL_META_NUM_CONTAINER_CHILDREN, NULL);

  if(num_item_children)
    gavl_dictionary_set_int(m, GAVL_META_NUM_ITEM_CHILDREN,      num_item_children);
  else
    gavl_dictionary_set(m, GAVL_META_NUM_ITEM_CHILDREN, NULL);

  if(num_container_children + num_item_children)
    gavl_dictionary_set_int(m, GAVL_META_NUM_CHILDREN,
                            num_container_children + num_item_children);
  else
    gavl_dictionary_set(m, GAVL_META_NUM_CHILDREN, NULL);
  
  }

const char * gavl_get_country_label(const char * iso)
  {
  int i = 0;
  
  if(strlen(iso) == 2)
    {
    while(gavl_countries[i].label)
      {
      if(!strcmp(iso, gavl_countries[i].code_alpha_2))
        return gavl_countries[i].label;
      i++;
      }
    }
  else if(strlen(iso) == 3)
    {
    while(gavl_countries[i].label)
      {
      if(!strcmp(iso, gavl_countries[i].code_alpha_3))
        return gavl_countries[i].label;
      i++;
      }
    }

  return iso;
  }

const char * gavl_get_country_code_2_from_label(const char * label)
  {
  int i = 0;
  
  while(gavl_countries[i].label)
    {
    if(!strcmp(label, gavl_countries[i].label))
      {
      return gavl_countries[i].code_alpha_2;
      }
    i++;
    }
  
  return NULL;
  }

const char * gavl_get_country_code_3_from_label(const char * label)
  {
  int i = 0;
  
  while(gavl_countries[i].label)
    {
    if(!strcmp(label, gavl_countries[i].label))
      {
      return gavl_countries[i].code_alpha_3;
      }
    i++;
    }
  
  return NULL;
  
  }


void gavl_track_set_countries(gavl_dictionary_t * track)
  {
  int i, num;
  gavl_dictionary_t * m;

  const gavl_value_t * src;
  
  const char * src_str = NULL;
  const char * dst_str;

  gavl_value_t dest;
  gavl_value_t dest_item;
  
  if(!(m = gavl_track_get_metadata_nc(track)))
    return;

  if((num = gavl_dictionary_get_num_items(m, GAVL_META_COUNTRY_CODE_2)))
    {
    if(!gavl_dictionary_get(m, GAVL_META_COUNTRY))
      {
      gavl_value_init(&dest);
      
      for(i = 0; i < num; i++)
        {
        gavl_value_init(&dest_item);
        
        if((src = gavl_dictionary_get_item(m, GAVL_META_COUNTRY_CODE_2, i)) &&
           (src_str = gavl_value_get_string(src)) &&
           (dst_str = gavl_get_country_label(src_str)))
          {
          gavl_value_set_string(&dest_item, dst_str);
          }
        else
          gavl_value_set_string(&dest_item, src_str);

        gavl_value_append_nocopy(&dest, &dest_item);
        }
      
      gavl_dictionary_set_nocopy(m, GAVL_META_COUNTRY, &dest);
      }
    }
#if 0
  else if((num = gavl_dictionary_get_num_items(m, GAVL_META_COUNTRY_CODE_3)))
    {
    int do_code_2 = 0;
    int do_label = 0;

    gavl_value_t label;
    gavl_value_t label_item;
    
    if(!gavl_dictionary_get(m, GAVL_META_COUNTRY_CODE_2))
      do_code_2 = 1;
    
    if(!gavl_dictionary_get(m, GAVL_META_COUNTRY))
      do_label = 1;

    if(do_code_2 || do_label)
      {
      
      gavl_value_init(&dest);
      gavl_value_init(&label);
      
      for(i = 0; i < num; i++)
        {
        gavl_value_init(&dest_item);
        gavl_value_init(&label_item);
        
        if((src = gavl_dictionary_get_item(m, GAVL_META_COUNTRY_CODE_3, i)) &&
           (src_str = gavl_value_get_string(src)) &&
           (dst_str = gavl_get_country_label(src_str)))
          {
          gavl_value_set_string(&label_item, dst_str);
          }
        else if(src_str)
          gavl_value_set_string(&label_item, src_str);
        else
          gavl_value_set_string(&label_item, "Unknown");

        if(do_code_2)
          {
          dst_str = gavl_get_country_code_2_from_label(gavl_value_get_string(&label_item));
          
          if(!dst_str)
            dst_str = "--";
          
          gavl_value_set_string(&dest_item, dst_str);
          }
        
        gavl_value_append_nocopy(&label, &label_item);
        }
      
      if(do_label)
        gavl_dictionary_set_nocopy(m, GAVL_META_COUNTRY, &label);
      else
        gavl_value_free(&label);
      
      if(do_code_2)
        gavl_dictionary_set_nocopy(m, GAVL_META_COUNTRY_CODE_2, &dest);
      else
        gavl_value_free(&dest);
      
      }

    }
#endif
  else if((num = gavl_dictionary_get_num_items(m, GAVL_META_COUNTRY)))
    {
    /* Set alpha 2 code */

    if(!gavl_dictionary_get(m, GAVL_META_COUNTRY_CODE_2))
      {
      gavl_value_init(&dest);
      
      for(i = 0; i < num; i++)
        {
        gavl_value_init(&dest_item);
        
        if((src = gavl_dictionary_get_item(m, GAVL_META_COUNTRY, i)) &&
           (src_str = gavl_value_get_string(src)) &&
           (dst_str = gavl_get_country_code_2_from_label(src_str)))
          {
          gavl_value_set_string(&dest_item, dst_str);
          }
        else
          gavl_value_set_string(&dest_item, src_str);

        gavl_value_append_nocopy(&dest, &dest_item);
        }
      
      gavl_dictionary_set_nocopy(m, GAVL_META_COUNTRY_CODE_2, &dest);
      }
    }
  
  }

int
gavl_stream_is_enabled(const gavl_dictionary_t * s)
  {
  int val = 0;
  const gavl_dictionary_t * m;

  if((m = gavl_stream_get_metadata(s)) &&
     gavl_dictionary_get_int(m, GAVL_META_STREAM_ENABLED, &val) &&
     val)
    return 1;
  else
    return 0;
  }

void
gavl_stream_set_enabled(gavl_dictionary_t * s, int enabled)
  {
  gavl_dictionary_t * m;

  m = gavl_dictionary_get_dictionary_create(s, GAVL_META_METADATA);
  gavl_dictionary_set_int(m, GAVL_META_STREAM_ENABLED, enabled);
  }

#if 0
void
gavl_stream_set_start_pts(gavl_dictionary_t * s, int64_t pts, int scale)
  {
  gavl_dictionary_t * m = gavl_dictionary_get_dictionary_nc(s, GAVL_META_METADATA);
  gavl_dictionary_set_long(m, GAVL_META_STREAM_FIRST_PTS, pts);
  gavl_dictionary_set_int(m, GAVL_META_STREAM_FIRST_PTS_SCALE, scale);
  }

void
gavl_stream_get_start_pts(const gavl_dictionary_t * s, int64_t * pts, int * scale)
  {
  const gavl_dictionary_t * m = gavl_dictionary_get_dictionary(s, GAVL_META_METADATA);

  *pts = 0;
  gavl_dictionary_get_long(m, GAVL_META_STREAM_FIRST_PTS, pts);
  
  *scale = GAVL_TIME_SCALE;
  gavl_dictionary_get_int(m, GAVL_META_STREAM_FIRST_PTS_SCALE, scale);
  }
#endif

gavl_time_t gavl_stream_get_start_time(const gavl_dictionary_t * s)
  {
  int timescale = 0;
  gavl_stream_stats_t stats;
  const gavl_dictionary_t * m = NULL;

  //  fprintf(stderr, "Get start time:\n");
  //  gavl_dictionary_dump(s->s, 2);
  
  gavl_stream_stats_init(&stats);
  if((gavl_stream_get_stats(s, &stats)) &&
     (m = gavl_stream_get_metadata(s)) &&
     gavl_dictionary_get_int(m, GAVL_META_STREAM_SAMPLE_TIMESCALE, &timescale))
    return gavl_time_unscale(timescale, stats.pts_start);
  else
    return 0;
  }

gavl_time_t gavl_track_get_start_time(const gavl_dictionary_t * dict)
  {
  const gavl_dictionary_t * d;
  if((d = gavl_track_get_audio_stream(dict, 0)) ||
     (d = gavl_track_get_video_stream(dict, 0)))
    return gavl_stream_get_start_time(d);
  else
    return 0;
  }



int gavl_stream_get_compression_tag(const gavl_dictionary_t * s)
  {
  int ret = 0;
  const gavl_dictionary_t * cmp;
  if(!(cmp = gavl_dictionary_get_dictionary(s, COMPRESSION_INFO_KEY)) ||
     !gavl_dictionary_get_int(cmp, COMPRESSION_INFO_KEY_TAG, &ret))
    return 0;
  else
    return ret;
  }

void gavl_stream_set_compression_tag(gavl_dictionary_t * s, int tag)
  {
  gavl_dictionary_t * cmp;
  cmp = gavl_dictionary_get_dictionary_create(s, COMPRESSION_INFO_KEY);
  gavl_dictionary_set_int(cmp, COMPRESSION_INFO_KEY_TAG, tag);
  }

void gavl_stream_set_sample_timescale(gavl_dictionary_t * s)
  {
  gavl_stream_type_t type;
  gavl_dictionary_t * m;
  int scale = 0;
  m = gavl_stream_get_metadata_nc(s);
  type = gavl_stream_get_type(s);

  //  fprintf(stderr, "gavl_stream_set_sample_timescale %s\n", gavl_stream_type_name(type));
  //  gavl_dictionary_dump(s, 2);
  
  if(!(m = gavl_stream_get_metadata_nc(s)) ||
     (gavl_dictionary_get_int(m, GAVL_META_STREAM_SAMPLE_TIMESCALE, &scale) && scale))
    return;

  switch(type)
    {
    case GAVL_STREAM_AUDIO:
      {
      const gavl_audio_format_t * fmt;
      
      if((fmt = gavl_stream_get_audio_format(s)) &&
         fmt->samplerate)
        gavl_dictionary_set_int(m, GAVL_META_STREAM_SAMPLE_TIMESCALE, fmt->samplerate);
      }
      break;
    case GAVL_STREAM_VIDEO:
      {
      const gavl_video_format_t * fmt;
      
      if((fmt = gavl_stream_get_video_format(s)) &&
         fmt->timescale)
        gavl_dictionary_set_int(m, GAVL_META_STREAM_SAMPLE_TIMESCALE, fmt->timescale);
      }
    default:
      break;
    }
  
  
  }

void gavl_stream_set_default_packet_timescale(gavl_dictionary_t * s)
  {
  gavl_stream_type_t type;
  gavl_dictionary_t * m;
  int scale = 0;
  m = gavl_stream_get_metadata_nc(s);
  type = gavl_stream_get_type(s);
  
  if(!(m = gavl_stream_get_metadata_nc(s)) ||
     (gavl_dictionary_get_int(m, GAVL_META_STREAM_PACKET_TIMESCALE, &scale) && scale))
    return;
  
  switch(type)
    {
    case GAVL_STREAM_AUDIO:
      {
      const gavl_audio_format_t * fmt;
      
      if((fmt = gavl_stream_get_audio_format(s)) &&
         fmt->samplerate)
        gavl_dictionary_set_int(m, GAVL_META_STREAM_PACKET_TIMESCALE, fmt->samplerate);
      }
      break;
    case GAVL_STREAM_VIDEO:
      {
      const gavl_video_format_t * fmt;
      
      if((fmt = gavl_stream_get_video_format(s)) &&
         fmt->timescale)
        gavl_dictionary_set_int(m, GAVL_META_STREAM_PACKET_TIMESCALE, fmt->timescale);
      }
    default:
      break;
    }
  }

void gavl_stream_set_audio_bits(gavl_dictionary_t * s, int bits)
  {
  gavl_dictionary_set_int(s, GAVL_META_AUDIO_BITS, bits);
  }

int gavl_stream_get_audio_bits(const gavl_dictionary_t * s)
  {
  int ret = 0;
  gavl_dictionary_get_int(s, GAVL_META_AUDIO_BITS, &ret);
  return ret;
  }

int gavl_stream_is_continuous(const gavl_dictionary_t * s)
  {
  gavl_stream_type_t type;

  type = gavl_stream_get_type(s);
  switch(type)
    {
    case GAVL_STREAM_AUDIO:
      return 1;
      break;
    case GAVL_STREAM_VIDEO:
      {
      const gavl_video_format_t * fmt = gavl_stream_get_video_format(s);
      if(fmt && (fmt->framerate_mode == GAVL_FRAMERATE_STILL))
        return 0;
      else
        return 1;
      }
      break;
    case GAVL_STREAM_TEXT:
    case GAVL_STREAM_OVERLAY:
    case GAVL_STREAM_MSG:
    case GAVL_STREAM_NONE:
      return 0;
      break;
    }
  
  return 0;
  }

static int has_external_stream(gavl_dictionary_t * track,
                               const char * location)
  {
  int i;
  const char * loc = NULL;
  const gavl_dictionary_t * dict;
  const gavl_array_t * arr = gavl_dictionary_get_array(track, GAVL_META_STREAMS_EXT);

  if(!arr)
    return 0;

  for(i = 0; i < arr->num_entries; i++)
    {
    if((dict = gavl_value_get_dictionary(&arr->entries[i])) &&
       (gavl_track_get_src(dict, GAVL_META_SRC, 0, NULL, &loc)) &&
       !strcmp(loc, location))
      return 1;
    }
  return 0;
  }

gavl_dictionary_t *
gavl_track_append_external_stream(gavl_dictionary_t * track,
                                  gavl_stream_type_t type,
                                  const char * mimetype,
                                  const char * location)
  {
  gavl_dictionary_t * m;
  gavl_dictionary_t * ret;

  if(has_external_stream(track, location))
    return NULL;
  
  ret = track_append_stream(track, type, GAVL_META_STREAMS_EXT);
    
  m = gavl_stream_get_metadata_nc(ret);
  
  gavl_metadata_add_src(m, GAVL_META_SRC, mimetype, location);
  return ret;
  }


int
gavl_track_get_num_external_streams(const gavl_dictionary_t * d)
  {
  const gavl_array_t * arr;
  
  if(!(arr = gavl_dictionary_get_array(d, GAVL_META_STREAMS_EXT)))
    return 0;
  
  return arr->num_entries;
  }

const gavl_dictionary_t *
gavl_track_get_external_stream(const gavl_dictionary_t * d, int i)
  {
  const gavl_array_t * arr;
  const gavl_value_t * val;
  const gavl_dictionary_t * dict;
  
  if(!(arr = gavl_dictionary_get_array(d, GAVL_META_STREAMS_EXT)))
    return NULL;

  if((val = gavl_array_get(arr, i)) &&
     (dict = gavl_value_get_dictionary(val)))
    return dict;
  else
    return NULL; 
  }

gavl_dictionary_t *
gavl_track_get_external_stream_nc(gavl_dictionary_t * d, int i)
  {
  gavl_array_t * arr;
  gavl_value_t * val;
  gavl_dictionary_t * dict;
  
  if(!(arr = gavl_dictionary_get_array_nc(d, GAVL_META_STREAMS_EXT)))
    return NULL;

  if((val = gavl_array_get_nc(arr, i)) &&
     (dict = gavl_value_get_dictionary_nc(val)))
    return dict;
  else
    return NULL; 
  }

// Multivariant support
int gavl_track_get_num_variants(const gavl_dictionary_t * dict)
  {
  return get_num_tracks(dict, GAVL_META_VARIANTS);
  }

const gavl_dictionary_t * gavl_track_get_variant(const gavl_dictionary_t * dict, int idx)
  {
  return get_track(dict, GAVL_META_VARIANTS, idx);
  }

gavl_dictionary_t * gavl_track_get_variant_nc(gavl_dictionary_t * dict, int idx)
  {
  return get_track_nc(dict, GAVL_META_VARIANTS, idx);
  }

gavl_dictionary_t * gavl_track_append_variant(gavl_dictionary_t * dict, const char * mimetype, const char * location)
  {
  gavl_dictionary_t * new_track = append_track(dict, GAVL_META_VARIANTS);
  gavl_track_add_src(new_track, GAVL_META_SRC, mimetype, location);
  return new_track;
  }


// Multipart support
int gavl_track_get_num_parts(const gavl_dictionary_t * dict)
  {
  return get_num_tracks(dict, GAVL_META_PARTS);
  }

gavl_dictionary_t * gavl_track_append_part(gavl_dictionary_t * dict, const char * mimetype, const char * location)
  {
  gavl_dictionary_t * new_track = append_track(dict, GAVL_META_PARTS);
  gavl_track_add_src(new_track, GAVL_META_SRC, mimetype, location);
  return new_track;
  }

const gavl_dictionary_t * gavl_track_get_part(const gavl_dictionary_t * dict, int idx)
  {
  return get_track(dict, GAVL_META_PARTS, idx);
  
  }

gavl_dictionary_t * gavl_track_get_part_nc(gavl_dictionary_t * dict, int idx)
  {
  return get_track_nc(dict, GAVL_META_PARTS, idx);
  
  }

/* SRC */

GAVL_PUBLIC
gavl_dictionary_t *
gavl_track_add_src(gavl_dictionary_t * dict, const char * key,
                   const char * mimetype, const char * location)
  {
  if((dict = gavl_track_get_metadata_nc(dict)))
    return gavl_metadata_add_src(dict, key, mimetype, location);
  else
    return NULL;
  }

GAVL_PUBLIC
const gavl_dictionary_t *
gavl_track_get_src(const gavl_dictionary_t * dict, const char * key, int idx,
                   const char ** mimetype, const char ** location)
  {
  if((dict = gavl_track_get_metadata(dict)))
    return gavl_metadata_get_src(dict, key, idx, mimetype, location);
  else
    return NULL;
  }

gavl_dictionary_t *
gavl_track_get_src_nc(gavl_dictionary_t * dict, const char * key, int idx)
  {
  if((dict = gavl_track_get_metadata_nc(dict)))
    return gavl_metadata_get_src_nc(dict, key, idx);
  else
    return NULL;
  }
  
int gavl_track_has_src(const gavl_dictionary_t * dict, const char * key,
                       const char * location)
  {
  if((dict = gavl_track_get_metadata(dict)))
    return gavl_metadata_has_src(dict, key, location);
  else
    return 0;
  }

void gavl_track_from_location(gavl_dictionary_t * ret, const char * location)
  {
  gavl_dictionary_t * m;
  track_init(ret);
  m = gavl_track_get_metadata_nc(ret);
  gavl_dictionary_set_string(m, GAVL_META_CLASS, GAVL_META_CLASS_LOCATION);
  gavl_metadata_add_src(m, GAVL_META_SRC, NULL, location);
  }

#if 0
void gavl_track_set_clock_time_map(gavl_dictionary_t * track,
                                   int64_t pts, int pts_scale, gavl_time_t clock_time)
  {
  gavl_dictionary_t * dict;

  dict = gavl_dictionary_get_dictionary_create(track, GAVL_META_METADATA);
  dict = gavl_dictionary_get_dictionary_create(dict, GAVL_META_CLOCK_TIME_MAP);

  gavl_dictionary_set_long(dict, GAVL_META_CLOCK_TIME_PTS, pts);
  gavl_dictionary_set_int(dict, GAVL_META_CLOCK_TIME_PTS_SCALE, pts_scale);
  gavl_dictionary_set_long(dict, GAVL_META_CLOCK_TIME, clock_time);
  
  }


int gavl_track_get_clock_time_map(const gavl_dictionary_t * track,
                                  int64_t * pts, int * pts_scale, gavl_time_t * clock_time)
  {
  const gavl_dictionary_t * dict;

  if(!(dict = gavl_track_get_metadata(track)) ||
     !(dict = gavl_dictionary_get_dictionary(dict, GAVL_META_CLOCK_TIME_MAP)))
    return 0;

  if(gavl_dictionary_get_long(dict, GAVL_META_CLOCK_TIME_PTS, pts) &&
     gavl_dictionary_get_int(dict, GAVL_META_CLOCK_TIME_PTS_SCALE, pts_scale) &&
     gavl_dictionary_get_long(dict, GAVL_META_CLOCK_TIME, clock_time))
    return 1;
  else
    return 0;
  }
#endif

gavl_time_t gavl_track_get_pts_to_clock_time(const gavl_dictionary_t * dict)
  {
  gavl_time_t ret = GAVL_TIME_UNDEFINED;

  if(!(dict = gavl_dictionary_get_dictionary(dict, GAVL_META_METADATA)) ||
     !gavl_dictionary_get_long(dict, GAVL_META_TIME_PTS_TO_CLOCK, &ret))
    return GAVL_TIME_UNDEFINED;
  
  return ret;
  }

gavl_time_t gavl_track_get_pts_to_start_time(const gavl_dictionary_t * dict)
  {
  gavl_time_t ret = GAVL_TIME_UNDEFINED;

  if(!(dict = gavl_dictionary_get_dictionary(dict, GAVL_META_METADATA)) ||
     !gavl_dictionary_get_long(dict, GAVL_META_TIME_PTS_TO_START, &ret))
    return GAVL_TIME_UNDEFINED;
  
  return ret;
  
  }

void gavl_track_set_pts_to_clock_time(gavl_dictionary_t * dict, gavl_time_t offset)
  {
  dict = gavl_dictionary_get_dictionary_create(dict, GAVL_META_METADATA);
  gavl_dictionary_set_long(dict, GAVL_META_TIME_PTS_TO_CLOCK, offset);
  }

void gavl_track_set_pts_to_start_time(gavl_dictionary_t * dict, gavl_time_t offset)
  {
  dict = gavl_dictionary_get_dictionary_create(dict, GAVL_META_METADATA);
  gavl_dictionary_set_long(dict, GAVL_META_TIME_PTS_TO_START, offset);
  }


int gavl_stream_is_sbr(const gavl_dictionary_t * s)
  {
  gavl_compression_info_t ci;
  int ret = 0;

  gavl_compression_info_init(&ci);
  if(gavl_stream_get_compression_info(s, &ci))
    ret = !!(ci.flags & GAVL_COMPRESSION_SBR);

  gavl_compression_info_free(&ci);
  return ret;
  }
