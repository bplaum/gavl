/*****************************************************************
 * Gavl - a general purpose audio/video processing library
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
#include <stdlib.h>

#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/value.h>

static const struct
  {
  gavl_type_t type;
  const char * name;
  }
types[] =
  {
    { GAVL_TYPE_INT,         "i"  },
    { GAVL_TYPE_LONG,        "l"  },
    { GAVL_TYPE_FLOAT,       "f"  },
    { GAVL_TYPE_STRING,      "s"  },
    { GAVL_TYPE_AUDIOFORMAT, "af" },
    { GAVL_TYPE_VIDEOFORMAT, "vf" },
    { GAVL_TYPE_COLOR_RGB,   "rgb" },
    { GAVL_TYPE_COLOR_RGBA,  "rgba" },
    { GAVL_TYPE_POSITION,    "pos" },
    { GAVL_TYPE_DICTIONARY,  "d" },
    { GAVL_TYPE_ARRAY,       "a" },
    { GAVL_TYPE_BINARY,      "b" },
    { /* End */ },
  };
  
const char * gavl_type_to_string(gavl_type_t type)
  {
  int i = 0;
  while(types[i].name)
    {
    if(type == types[i].type)
      return types[i].name;
    i++;
    }
  return NULL;
  }

gavl_type_t gavl_type_from_string(const char * str)
  {
  int i = 0;
  while(types[i].name)
    {
    if(!strcmp(str, types[i].name))
      return types[i].type;
    i++;
    }
  return GAVL_TYPE_UNDEFINED;
  }



/* Value functions */

#define CMP_NUM(a, b) (a == b) ? 0 : ((a < b) ? -1 : 1)

#define CMP_NUM_ARR(a, b, num) \
  for(i = 0; i < num; i++) \
    { \
    if((res = CMP_NUM(a[i], b[i]))) \
      return res; \
    } \
  return res

int gavl_value_compare(const gavl_value_t * v1, const gavl_value_t * v2)
  {
  int i, res;

  if(!v1 && v2)
    return 1;

  if(v1 && !v2)
    return 1;

  if(!v1 && !v2)
    return 0;
  
  if(v1->type != v2->type)
    return 1;

  switch(v1->type)
    {
    case GAVL_TYPE_UNDEFINED:
      return 0;
      break;
    case GAVL_TYPE_INT:
      return CMP_NUM(v1->v.i, v2->v.i);
      break;
    case GAVL_TYPE_LONG:
      return CMP_NUM(v1->v.l, v2->v.l);
      break;
    case GAVL_TYPE_FLOAT:
      return CMP_NUM(v1->v.d, v2->v.d);
      break;
    case GAVL_TYPE_STRING:
      if(!v1->v.str && !v2->v.str)
        return 0;
      else if(v1->v.str && v2->v.str)
        return strcmp(v1->v.str, v2->v.str);
      else
        return 1;
      break;
    case GAVL_TYPE_AUDIOFORMAT:
      return memcmp(&v1->v.audioformat, &v2->v.audioformat, sizeof(v1->v.audioformat));
      break;
    case GAVL_TYPE_VIDEOFORMAT:
      return memcmp(&v1->v.videoformat, &v2->v.videoformat, sizeof(v1->v.videoformat));
      break;
    case GAVL_TYPE_COLOR_RGB:
      CMP_NUM_ARR(v1->v.color, v2->v.color, 3);
      break;
    case GAVL_TYPE_COLOR_RGBA:
      CMP_NUM_ARR(v1->v.color, v2->v.color, 4);
      break;
    case GAVL_TYPE_POSITION:
      CMP_NUM_ARR(v1->v.position, v2->v.position, 2);
      break;
    case GAVL_TYPE_DICTIONARY:
      return gavl_dictionary_compare(v1->v.dictionary, v2->v.dictionary);
      break;
    case GAVL_TYPE_ARRAY:
      return gavl_array_compare(v1->v.array, v2->v.array);
      break;
    case GAVL_TYPE_BINARY:
      {
      int num;
      int result;

      if(!v1->v.buffer->len && !v2->v.buffer->len)
        return 0;

      if(!v1->v.buffer->len || !v2->v.buffer->len)
        {
        return CMP_NUM(v1->v.buffer->len, v2->v.buffer->len);
        }
        
      if(v1->v.buffer->len > v2->v.buffer->len)
        num = v2->v.buffer->len;
      else
        num = v1->v.buffer->len;
      
      result = memcmp(v1->v.buffer->buf, v2->v.buffer->buf, num);
      
      if(!result)
        return CMP_NUM(v1->v.buffer->len, v2->v.buffer->len);
      else
        return result;
      }
      break;
    }
  return 0;
  }

void gavl_value_copy(gavl_value_t * dst, const gavl_value_t * src)
  {
  gavl_value_reset(dst);
  if(!src || !src->type)
    {
    gavl_value_init(dst);
    return;
    }

  gavl_value_set_type(dst, src->type);
  
  switch(src->type)
    {
    case GAVL_TYPE_UNDEFINED:
      break;
    case GAVL_TYPE_INT:
      dst->v.i = src->v.i;
      break;
    case GAVL_TYPE_LONG:
      dst->v.l = src->v.l;
      break;
    case GAVL_TYPE_FLOAT:
      dst->v.d = src->v.d;
      break;
    case GAVL_TYPE_STRING:
      dst->v.str = gavl_strdup(src->v.str);
      break;
    case GAVL_TYPE_AUDIOFORMAT:
      {
      const gavl_audio_format_t * fmt_src;
      gavl_audio_format_t * fmt_dst;
      fmt_src = gavl_value_get_audio_format(src);
      fmt_dst = gavl_value_get_audio_format_nc(dst);
      gavl_audio_format_copy(fmt_dst, fmt_src);
      }
      break;
    case GAVL_TYPE_VIDEOFORMAT:
      {
      const gavl_video_format_t * fmt_src;
      gavl_video_format_t * fmt_dst;
      fmt_src = gavl_value_get_video_format(src);
      fmt_dst = gavl_value_get_video_format_nc(dst);
      gavl_video_format_copy(fmt_dst, fmt_src);
      }
      break;
    case GAVL_TYPE_BINARY:
      {
      const gavl_buffer_t * buf_src;
      gavl_buffer_t * buf_dst;
      buf_src = gavl_value_get_binary(src);
      buf_dst = gavl_value_get_binary_nc(dst);
      gavl_buffer_copy(buf_dst, buf_src);
      }
      break;
    case GAVL_TYPE_COLOR_RGB:
      memcpy(dst->v.color, src->v.color, 3 * sizeof(dst->v.color[0]));
      break;
    case GAVL_TYPE_COLOR_RGBA:
      memcpy(dst->v.color, src->v.color, 4 * sizeof(dst->v.color[0]));
      break;
    case GAVL_TYPE_POSITION:
      memcpy(dst->v.position, src->v.position, 2 * sizeof(dst->v.position[0]));
      break;
    case GAVL_TYPE_DICTIONARY:
      if(src->v.dictionary)
        gavl_dictionary_copy(dst->v.dictionary, src->v.dictionary);
      break;
    case GAVL_TYPE_ARRAY:
      if(src->v.array)
        gavl_array_copy(dst->v.array, src->v.array);
      break;
    }
  }

void gavl_value_free(gavl_value_t * v)
  {
  switch(v->type)
    {
    case GAVL_TYPE_UNDEFINED:
    case GAVL_TYPE_INT:
    case GAVL_TYPE_LONG:
    case GAVL_TYPE_FLOAT:
    case GAVL_TYPE_COLOR_RGB:
    case GAVL_TYPE_COLOR_RGBA:
    case GAVL_TYPE_POSITION:
      break;
    case GAVL_TYPE_STRING:
      if(v->v.str)
        free(v->v.str);
      break;
    case GAVL_TYPE_DICTIONARY:
      if(v->v.dictionary)
        gavl_dictionary_destroy(v->v.dictionary);
      break;
    case GAVL_TYPE_ARRAY:
      if(v->v.array)
        gavl_array_destroy(v->v.array);
      break;
    case GAVL_TYPE_AUDIOFORMAT:
      if(v->v.audioformat)
        free(v->v.audioformat);
      break;
    case GAVL_TYPE_VIDEOFORMAT:
      if(v->v.videoformat)
        free(v->v.videoformat);
      break;
    case GAVL_TYPE_BINARY:
      if(v->v.buffer)
        {
        gavl_buffer_free(v->v.buffer);
        free(v->v.buffer);
        }
      break;
    }
  }

void gavl_value_init(gavl_value_t * v)
  {
  memset(v, 0, sizeof(*v));
  }

void gavl_value_reset(gavl_value_t * v)
  {
  gavl_value_free(v);
  gavl_value_init(v);
  }

void gavl_value_set_int(gavl_value_t * v, int val)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_INT;
  v->v.i = val;
  }

void gavl_value_set_float(gavl_value_t * v, double val)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_FLOAT;
  v->v.d = val;
  }

void gavl_value_set_long(gavl_value_t * v, int64_t val)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_LONG;
  v->v.l = val;
  }

void gavl_value_set_string(gavl_value_t * v, const char * str)
  {
  gavl_value_reset(v);
  gavl_value_set_string_nocopy(v, gavl_strdup(str));
  }



void gavl_value_set_string_nocopy(gavl_value_t * v, char * str)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_STRING;
  v->v.str = str;
  if(v->v.str)
    gavl_strtrim(v->v.str);
  }

gavl_buffer_t * gavl_value_set_binary(gavl_value_t * v)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_BINARY;
  v->v.buffer = calloc(1, sizeof(*v->v.buffer));
  return v->v.buffer;
  }

const gavl_buffer_t * gavl_value_get_binary(const gavl_value_t * v)
  {
  if(v->type != GAVL_TYPE_BINARY)
    return NULL;
  return v->v.buffer;
  }
  
gavl_buffer_t * gavl_value_get_binary_nc(gavl_value_t * v)
  {
  if(v->type != GAVL_TYPE_BINARY)
    return NULL;
  return v->v.buffer;
  }


gavl_audio_format_t * gavl_value_set_audio_format(gavl_value_t * v)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_AUDIOFORMAT;
  v->v.audioformat = calloc(1, sizeof(*v->v.audioformat));
  return v->v.audioformat;
  }

gavl_video_format_t * gavl_value_set_video_format(gavl_value_t * v)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_VIDEOFORMAT;
  v->v.videoformat = calloc(1, sizeof(*v->v.videoformat));
  return v->v.videoformat;
  }

gavl_dictionary_t * gavl_value_set_dictionary(gavl_value_t * v)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_DICTIONARY;
  v->v.dictionary = calloc(1, sizeof(*v->v.dictionary));
  return v->v.dictionary;
  }

void gavl_value_set_dictionary_nocopy(gavl_value_t * v, gavl_dictionary_t * val)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_DICTIONARY;
  v->v.dictionary = val;
  }

void gavl_value_set_array_nocopy(gavl_value_t * v, gavl_array_t * val)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_ARRAY;
  v->v.array = val;
  }

gavl_array_t * gavl_value_set_array(gavl_value_t * v)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_ARRAY;
  v->v.array = calloc(1, sizeof(*v->v.array));
  return v->v.array;
  }

double * gavl_value_set_position(gavl_value_t * v)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_POSITION;
  return v->v.position;
  }

double * gavl_value_set_color_rgb(gavl_value_t * v)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_COLOR_RGB;
  return v->v.color;
  }

double * gavl_value_set_color_rgba(gavl_value_t * v)
  {
  gavl_value_reset(v);
  v->type = GAVL_TYPE_COLOR_RGBA;
  return v->v.color;
  }

void gavl_value_move(gavl_value_t * dst, gavl_value_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  gavl_value_init(src);
  }

void gavl_value_append(gavl_value_t * v, const gavl_value_t * child)
  {
  if(v->type == GAVL_TYPE_UNDEFINED)
    {
    /* First element: Set as value */
    gavl_value_copy(v, child);
    }
  else if(v->type != GAVL_TYPE_ARRAY)
    {
    /* First element already there: Convert to array */
    gavl_array_t * arr;
    gavl_value_t save;

    gavl_value_move(&save, v);
    arr = gavl_value_set_array(v);
    
    /* First element already there: Convert to array */
    gavl_array_push_nocopy(arr, &save);
    gavl_array_push(arr, child);
    }
  else
    {
    /* Append to array */
    gavl_array_push(v->v.array, child);
    }
  }

void gavl_value_append_nocopy(gavl_value_t * v, gavl_value_t * child)
  {
  if(v->type == GAVL_TYPE_UNDEFINED)
    {
    /* First element: Set as value */
    gavl_value_move(v, child);
    return;
    }
  else if(v->type != GAVL_TYPE_ARRAY)
    {
    /* First element already there: Convert to array */
    gavl_array_t * arr;
    gavl_value_t save;
    gavl_value_move(&save, v);
    arr = gavl_value_set_array(v);
    
    gavl_array_push_nocopy(arr, &save);
    gavl_array_push_nocopy(arr, child);
    }
  else
    {
    /* Append to array */
    gavl_array_push_nocopy(v->v.array, child);
    }
  }

int gavl_value_get_num_items(const gavl_value_t * v)
  {
  if(v->type == GAVL_TYPE_UNDEFINED)
    return 0;
  
  if(v->type == GAVL_TYPE_ARRAY)
    return v->v.array->num_entries; 
  
  return 1;
  }

const gavl_value_t * gavl_value_get_item(const gavl_value_t * v, int item)
  {
  if(v->type == GAVL_TYPE_UNDEFINED)
    return NULL;
  
  if(v->type == GAVL_TYPE_ARRAY)
    return gavl_array_get(v->v.array, item);
  else if(!item)
    return v;
  else
    return NULL;
  }

void gavl_value_delete_item(gavl_value_t * v, int item)
  {
  if(v->type == GAVL_TYPE_UNDEFINED)
    return;

  if(v->type == GAVL_TYPE_ARRAY)
    return gavl_array_splice_val(v->v.array, item, 1, NULL);
  else if(!item)
    {
    gavl_value_free(v);
    gavl_value_init(v);
    }
  }

gavl_value_t * gavl_value_get_item_nc(gavl_value_t * v, int item)
  {
  if(v->type == GAVL_TYPE_UNDEFINED)
    return NULL;
  
  if(v->type == GAVL_TYPE_ARRAY)
    return gavl_array_get_nc(v->v.array, item);
  else if(!item)
    return v;
  else
    return NULL;
  }


/* Get value */

int gavl_value_get_int(const gavl_value_t * v, int * val)
  {
  switch(v->type)
    {
    case GAVL_TYPE_INT:
      *val = v->v.i;
      break;
    case GAVL_TYPE_LONG:
      *val = (int)(v->v.l);
      break;
    case GAVL_TYPE_FLOAT:
      *val = (int)(v->v.d);
      break;
    case GAVL_TYPE_STRING:
      {
      char * rest;
      const char * end = v->v.str + strlen(v->v.str);
      *val = strtol(v->v.str, &rest, 10);
      if(rest != end)
        return 0;
      }
      break;
    default:
      return 0;
      break;
    }
  return 1;
  }

int gavl_value_get_float(const gavl_value_t * v, double * val)
  {
  switch(v->type)
    {
    case GAVL_TYPE_INT:
      *val = v->v.i;
      break;
    case GAVL_TYPE_LONG:
      *val = v->v.l;
      break;
    case GAVL_TYPE_FLOAT:
      *val = v->v.d;
      break;
    case GAVL_TYPE_STRING:
      {
      char * rest;
      const char * end = v->v.str + strlen(v->v.str);
      *val = strtod(v->v.str, &rest);
      if(rest != end)
        return 0;
      }
    default:
      return 0;
      break;
    }
  return 1;
  }

int gavl_value_get_long(const gavl_value_t * v, int64_t * val)
  {
  switch(v->type)
    {
    case GAVL_TYPE_INT:
      *val = v->v.i;
      break;
    case GAVL_TYPE_LONG:
      *val = v->v.l;
      break;
    case GAVL_TYPE_FLOAT:
      *val = v->v.d;
      break;
    case GAVL_TYPE_STRING:
      {
      char * rest;
      const char * end = v->v.str + strlen(v->v.str);
      *val = strtoll(v->v.str, &rest, 10);
      if(rest != end)
        {
        //        fprintf(stderr, "Couldn't transform value: %s", v->v.str);
        return 0;
        }
      }
      break;
    default:
      return 0;
      break;
    }
  return 1;
  }

const char * gavl_value_get_string(const gavl_value_t * v)
  {
  if(v->type == GAVL_TYPE_STRING)
    return v->v.str;
  return NULL;
  }

char * gavl_value_get_string_nc(gavl_value_t * v)
  {
  if(v->type == GAVL_TYPE_STRING)
    return v->v.str;
  return NULL;
  }

char * gavl_value_to_string(const gavl_value_t * v)
  {
  switch(v->type)
    {
    case GAVL_TYPE_INT:
      return gavl_sprintf("%d", v->v.i);
      break;
    case GAVL_TYPE_LONG:
      return gavl_sprintf("%"PRId64, v->v.l);
      break;
    case GAVL_TYPE_FLOAT:
      return gavl_sprintf("%.16e", v->v.d);
      break;
    case GAVL_TYPE_STRING:
      return gavl_strdup(v->v.str);
      break;
    default:
      return NULL;
      break;
    }
  }

void gavl_value_from_string(gavl_value_t * v, const char * str)
  {
  switch(v->type)
    {
    case GAVL_TYPE_UNDEFINED:
      {
      char * rest;
      const char * end;
      end = str + strlen(str);

      v->v.l = strtoll(str, &rest, 10);
      if(rest == end)
        {
        v->type = GAVL_TYPE_LONG;
        return;
        }

      v->v.d = strtod(str, &rest);
      if(rest == end)
        {
        v->type = GAVL_TYPE_FLOAT;
        return;
        }
      // Default
      v->v.str = gavl_strdup(str);
      v->type = GAVL_TYPE_STRING;
      }
      break;
    case GAVL_TYPE_INT:
      v->v.i = strtol(str, NULL, 10);
      break;
    case GAVL_TYPE_LONG:
      v->v.l = strtoll(str, NULL, 10);
      break;
    case GAVL_TYPE_FLOAT:
      v->v.d = strtod(str, NULL);
      break;
    case GAVL_TYPE_STRING:
      v->v.str = gavl_strdup(str);
      break;
    default:
      break;
    }
  
  }

const gavl_audio_format_t * gavl_value_get_audio_format(const gavl_value_t * v)
  {
  if(v->type != GAVL_TYPE_AUDIOFORMAT)
    return NULL;
  return v->v.audioformat;
  }

gavl_audio_format_t * gavl_value_get_audio_format_nc(gavl_value_t * v)
  {
  if(v->type != GAVL_TYPE_AUDIOFORMAT)
    return NULL;
  return v->v.audioformat;
  }

const gavl_video_format_t * gavl_value_get_video_format(const gavl_value_t * v)
  {
  if(v->type != GAVL_TYPE_VIDEOFORMAT)
    return NULL;
  return v->v.videoformat;
  }

gavl_video_format_t * gavl_value_get_video_format_nc(gavl_value_t * v)
  {
  if(v->type != GAVL_TYPE_VIDEOFORMAT)
    return NULL;
  return v->v.videoformat;
  }

const gavl_dictionary_t * gavl_value_get_dictionary(const gavl_value_t * v)
  {
  if(v->type != GAVL_TYPE_DICTIONARY)
    return NULL;
  return v->v.dictionary;
  }

gavl_dictionary_t * gavl_value_get_dictionary_nc(gavl_value_t * v)
  {
  if(v->type != GAVL_TYPE_DICTIONARY)
    return NULL;
  return v->v.dictionary;
  }

const gavl_array_t * gavl_value_get_array(const gavl_value_t * v)
  {
  if(v->type != GAVL_TYPE_ARRAY)
    return NULL;
  return v->v.array;
  }

gavl_array_t * gavl_value_get_array_nc(gavl_value_t * v)
  {
  if(v->type != GAVL_TYPE_ARRAY)
    return NULL;
  return v->v.array;
  }

const double * gavl_value_get_position(const gavl_value_t * v)
  {
  if(v->type != GAVL_TYPE_POSITION)
    return NULL;
  return v->v.position;
  }

const double * gavl_value_get_color_rgb(const gavl_value_t * v)
  {
  if(v->type != GAVL_TYPE_COLOR_RGB)
    return NULL;
  return v->v.color;
  }

const double * gavl_value_get_color_rgba(const gavl_value_t * v)
  {
  if(v->type != GAVL_TYPE_COLOR_RGBA)
    return NULL;
  return v->v.color;
  }

void gavl_value_dump(const gavl_value_t * v, int indent)
  {
  if(!v)
    {
    gavl_diprintf(0, "NULL");
    return;
    }
  switch(v->type)
    {
    case GAVL_TYPE_UNDEFINED:
      break;
    case GAVL_TYPE_INT:
      gavl_diprintf(indent, "%d", v->v.i);
      break;
    case GAVL_TYPE_LONG:
      gavl_diprintf(indent, "%"PRId64, v->v.l);
      break;
    case GAVL_TYPE_FLOAT:
      gavl_diprintf(indent, "%f", v->v.d);
      break;
    case GAVL_TYPE_AUDIOFORMAT:
      gavl_diprintf(0, "\n");
      gavl_audio_format_dumpi(v->v.audioformat, indent);
      break;
    case GAVL_TYPE_VIDEOFORMAT:
      gavl_diprintf(0, "\n");
      gavl_video_format_dumpi(v->v.videoformat, indent);
      break;
    case GAVL_TYPE_COLOR_RGB:
      gavl_diprintf(indent, "[ %f, %f, %f ]",
                    v->v.color[0], v->v.color[1],
                    v->v.color[2]);
      break;
    case GAVL_TYPE_COLOR_RGBA:
      gavl_diprintf(indent, "[ %f, %f, %f, %f ]",
                    v->v.color[0], v->v.color[1],
                    v->v.color[2], v->v.color[3]);
      break;
    case GAVL_TYPE_POSITION:
      gavl_diprintf(indent, "[ %f, %f ]",
                    v->v.position[0], v->v.position[1]);
      break;
    case GAVL_TYPE_STRING:
      if(!v->v.str)
        gavl_diprintf(indent, "NULL");
      else
        gavl_diprintf(indent, "\"%s\"", v->v.str);
      break;
    case GAVL_TYPE_DICTIONARY:
      gavl_dprintf("\n");
      gavl_dictionary_dump(v->v.dictionary, indent);
      
      break;
    case GAVL_TYPE_ARRAY:
      gavl_dprintf("\n");
      gavl_array_dump(v->v.array, indent);
      break;
    case GAVL_TYPE_BINARY:
      gavl_diprintf(indent, "%d bytes\n", v->v.buffer->len);
      if(v->v.buffer->len)
        gavl_hexdumpi(v->v.buffer->buf, v->v.buffer->len, 16, indent+2);
      break;
      
    }

  }

GAVL_PUBLIC
void gavl_value_set_type(gavl_value_t * v, gavl_type_t  t)
  {
  switch(t)
    {
    case GAVL_TYPE_UNDEFINED:
    case GAVL_TYPE_INT:
    case GAVL_TYPE_LONG:
    case GAVL_TYPE_FLOAT:
    case GAVL_TYPE_COLOR_RGB:
    case GAVL_TYPE_COLOR_RGBA:
    case GAVL_TYPE_POSITION:
    case GAVL_TYPE_STRING:
      v->type = t;
      break;
    case GAVL_TYPE_AUDIOFORMAT:
      gavl_value_set_audio_format(v);
      break;
    case GAVL_TYPE_VIDEOFORMAT:
      gavl_value_set_video_format(v);
      break;
    case GAVL_TYPE_DICTIONARY:
      gavl_value_set_dictionary(v);
      break;
    case GAVL_TYPE_ARRAY:
      gavl_value_set_array(v);
      break;
    case GAVL_TYPE_BINARY:
      gavl_value_set_binary(v);
      break;
    }
  
  
  }

char * gavl_value_join_arr(const gavl_value_t * val, const char * glue)
  {
  if(!val)
    return NULL;
  if(val->type == GAVL_TYPE_STRING)
    return gavl_strdup(val->v.str);
  else if(val->type == GAVL_TYPE_ARRAY)
    {
    const gavl_array_t * arr = val->v.array;
    return gavl_string_array_join(arr, glue);
    }
  else
    return NULL;

  }
