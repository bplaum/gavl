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

#include <config.h>
#include <gavl/gavl.h>
#include <gavl/metadata.h>
#include <gavl/metatags.h>
#include <gavl/utils.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/* Time <-> String */

void
gavl_metadata_date_to_string(int year,
                             int month,
                             int day, char * ret)
  {
  snprintf(ret, GAVL_METADATA_DATE_STRING_LEN,
           "%04d-%02d-%02d", year, month, day);
  }

void
gavl_metadata_date_time_to_string(int year,
                                  int month,
                                  int day,
                                  int hour,
                                  int minute,
                                  int second,
                                  char * ret)
  {
  snprintf(ret, GAVL_METADATA_DATE_TIME_STRING_LEN,
           "%04d-%02d-%02d %02d:%02d:%02d",
           year, month, day, hour, minute, second);
  }

void
gavl_dictionary_set_date(gavl_dictionary_t * m,
                         const char * key,
                         int year,
                         int month,
                         int day)
  {
  // YYYY-MM-DD
  char buf[GAVL_METADATA_DATE_STRING_LEN];
  gavl_metadata_date_to_string(year, month, day, buf);
  gavl_dictionary_set_string(m, key, buf);
  }

void
gavl_dictionary_set_date_time(gavl_dictionary_t * m,
                            const char * key,
                            int year,
                            int month,
                            int day,
                            int hour,
                            int minute,
                            int second)
  {
  // YYYY-MM-DD HH:MM:SS
  char buf[GAVL_METADATA_DATE_TIME_STRING_LEN];
  gavl_metadata_date_time_to_string(year,
                                    month,
                                    day,
                                    hour,
                                    minute,
                                    second,
                                    buf);
  gavl_dictionary_set_string(m, key, buf);
  }

int
gavl_dictionary_get_date(const gavl_dictionary_t * m,
                         const char * key,
                         int * year,
                         int * month,
                         int * day)
  {
  const char * val = gavl_dictionary_get_string(m, key);
  if(!val)
    return 0;

  if(sscanf(val, "%04d-%02d-%02d", year, month, day) < 3)
    return 0;
  return 1;
  }

GAVL_PUBLIC int
gavl_dictionary_get_year(const gavl_dictionary_t * m,
                         const char * key)
  {
  int year;
  int month;
  int day;
  
  if(gavl_dictionary_get_date(m, key, &year, &month, &day) &&
     (year != 9999))
    return year;
  else
    return 0;
  }

GAVL_PUBLIC int
gavl_dictionary_get_date_time(const gavl_dictionary_t * m,
                              const char * key,
                              int * year,
                              int * month,
                              int * day,
                              int * hour,
                              int * minute,
                              int * second)
  {
  const char * val = gavl_dictionary_get_string(m, key);
  if(!val)
    return 0;

  if(sscanf(val, "%04d-%02d-%02d %02d:%02d:%02d",
            year, month, day, hour, minute, second) < 6)
    return 0;
  return 1;
  }


int
gavl_metadata_equal(const gavl_dictionary_t * m1,
                    const gavl_dictionary_t * m2)
  {
  return !gavl_dictionary_compare(m1, m2);
  }

static const char * compression_fields[] =
  {
    GAVL_META_SOFTWARE,
    GAVL_META_MIMETYPE,
    GAVL_META_FORMAT,
    GAVL_META_BITRATE,
    GAVL_META_AUDIO_BITS,
    GAVL_META_VIDEO_BPP,
    NULL,
  };

const char * implicit_fields[] =
  {
    GAVL_META_URI,
    GAVL_META_AVG_BITRATE,
    GAVL_META_AVG_FRAMERATE,
    GAVL_META_STREAM_DURATION,
    GAVL_META_WIDTH,
    GAVL_META_HEIGHT,
    GAVL_META_FORMAT,
    GAVL_META_AUDIO_CODEC,
    GAVL_META_VIDEO_CODEC,
    GAVL_META_AUDIO_SAMPLERATE,
    GAVL_META_AUDIO_CHANNELS,
    GAVL_META_SRC,
    GAVL_META_AUDIO_BITRATE,
    GAVL_META_VIDEO_BITRATE,
    NULL,
  };


void
gavl_metadata_delete_compression_fields(gavl_dictionary_t * m)
  {
  gavl_dictionary_delete_fields(m, compression_fields);
  }

void
gavl_metadata_delete_implicit_fields(gavl_dictionary_t * m)
  {
  gavl_dictionary_delete_fields(m, implicit_fields);
  }

void
gavl_dictionary_set_string_endian(gavl_dictionary_t * m)
  {
  int val;
#ifdef WORDS_BIGENDIAN
  val = 1;
#else
  val = 0;
#endif
  gavl_dictionary_set_int(m, "BigEndian", val);
  }

int
gavl_metadata_do_swap_endian(const gavl_dictionary_t * m)
  {
  int val;

  if(!m)
    return 0;
  
  if(!gavl_dictionary_get_int(m, "BigEndian", &val))
    val = 0;
#ifdef WORDS_BIGENDIAN
  if(!val)
    return 1;
#else
  if(val)
    return 1;
#endif
  else
    return 0;
  }

/* Array functions */

void
gavl_metadata_append_nocpy(gavl_dictionary_t * m,
                           const char * key,
                           char * val)
  {
  gavl_value_t v;
  gavl_value_init(&v);
  gavl_value_set_string_nocopy(&v, val);
  gavl_dictionary_append_nocopy(m, key, &v);
  }

void
gavl_metadata_append(gavl_dictionary_t * m,
                     const char * key,
                     const char * val)
  {
  gavl_metadata_append_nocpy(m, key, gavl_strdup(val));
  }

static char * metadata_get_arr_internal(const gavl_dictionary_t * m,
                                        const char * key,
                                        int i, int ign)
  {
  const gavl_value_t * val;

  if(ign)
    val = gavl_dictionary_get_i(m, key);
  else
    val = gavl_dictionary_get(m, key);

  if(!val)
    return NULL;
  
  if(val->type == GAVL_TYPE_ARRAY)
    {
    if(i < 0)
      return NULL;
    if(i >= val->v.array->num_entries)
      return NULL;
    if(val->v.array->entries[i].type != GAVL_TYPE_STRING)
      return NULL;
    return val->v.array->entries[i].v.str;
    }
  else if((val->type == GAVL_TYPE_STRING) && !i)
    return val->v.str;
  else
    return NULL;
  
  }

const char * 
gavl_dictionary_get_arr(const gavl_dictionary_t * m,
                      const char * key,
                      int i)
  {
  return metadata_get_arr_internal(m, key, i, 0);
  }

const char * 
gavl_dictionary_get_arr_i(const gavl_dictionary_t * m,
                        const char * key,
                        int i)
  {
  return metadata_get_arr_internal(m, key, i, 1);
  }



int gavl_dictionary_get_arr_len(const gavl_dictionary_t * m,
                              const char * key)
  {
  const gavl_value_t * val = gavl_dictionary_get(m, key);
  
  if(!val)
    return 0;
  else if(val->type == GAVL_TYPE_STRING)
    return 1;
  else if(val->type == GAVL_TYPE_ARRAY)
    return val->v.array->num_entries;
  else
    return 0;
  }
  
char * 
gavl_metadata_join_arr(const gavl_dictionary_t * m,
                       const char * key, const char * glue)
  {
  return gavl_value_join_arr(gavl_dictionary_get(m, key), glue);
  }

//

void gavl_metadata_add_image_embedded(gavl_dictionary_t * m,
                                      const char * key,
                                      int w, int h,
                                      const char * mimetype,
                                      int64_t offset,
                                      int64_t size)
  {
  gavl_value_t child;
  gavl_value_t val;

  gavl_dictionary_t * dict;

  gavl_value_init(&child);

  dict = gavl_value_set_dictionary(&child);

  gavl_value_init(&val);
  
  gavl_value_set_long(&val, offset);
  gavl_dictionary_set_nocopy(dict, GAVL_META_COVER_OFFSET, &val);

  gavl_value_set_long(&val, size);
  gavl_dictionary_set_nocopy(dict, GAVL_META_COVER_SIZE, &val);

  if(mimetype)
    {
    gavl_value_set_string(&val, mimetype);
    gavl_dictionary_set_nocopy(dict, GAVL_META_MIMETYPE, &val);
    }
  if(w > 0)
    {
    gavl_value_set_int(&val, w);
    gavl_dictionary_set_nocopy(dict, GAVL_META_WIDTH, &val);
    }
  if(h > 0)
    {
    gavl_value_set_int(&val, h);
    gavl_dictionary_set_nocopy(dict, GAVL_META_HEIGHT, &val);
    }
  gavl_dictionary_append_nocopy(m, key, &child);
  }
                         

// 

gavl_dictionary_t *  gavl_metadata_add_image_uri(gavl_dictionary_t * m,
                                                 const char * key,
                                                 int w, int h,
                                                 const char * mimetype,
                                                 const char * uri)
  {
  gavl_value_t child;
  gavl_value_t val;

  gavl_dictionary_t * dict;
  
  gavl_value_init(&child);

  dict = gavl_value_set_dictionary(&child);
  
  gavl_value_init(&val);
  gavl_value_set_string(&val, uri);
  gavl_dictionary_set_nocopy(dict, GAVL_META_URI, &val);

  if(mimetype)
    {
    gavl_value_set_string(&val, mimetype);
    gavl_dictionary_set_nocopy(dict, GAVL_META_MIMETYPE, &val);
    }
  if(w > 0)
    {
    gavl_value_set_int(&val, w);
    gavl_dictionary_set_nocopy(dict, GAVL_META_WIDTH, &val);
    }
  if(h > 0)
    {
    gavl_value_set_int(&val, h);
    gavl_dictionary_set_nocopy(dict, GAVL_META_HEIGHT, &val);
    }
  gavl_dictionary_append_nocopy(m, key, &child);
  return dict;
  }


/* Generic routine */

static const gavl_dictionary_t * get_image(const gavl_dictionary_t * m,
                                    const char * key,
                                    int idx)
  {
  const gavl_dictionary_t * dict = NULL;
  const gavl_value_t * val;

  if(!(val = gavl_dictionary_get(m, key)))
    return NULL;
  
  if((val->type == GAVL_TYPE_DICTIONARY) && !idx)
    dict = val->v.dictionary;
  else if(val->type == GAVL_TYPE_ARRAY)
    {
    if((val = gavl_array_get(val->v.array, idx)) &&
       (val->type == GAVL_TYPE_DICTIONARY))
      dict = val->v.dictionary;
    }

  return dict;
  }
                                    

const char * gavl_dictionary_get_string_image_uri(const gavl_dictionary_t * m,
                                                  const char * key,
                                                  int i,
                                                  int * wp, int * hp,
                                                  const char ** mimetype)
  {
  const gavl_dictionary_t * dict = NULL;
  const gavl_value_t * val;
  const char * ret;
  
  if(mimetype)
    *mimetype = NULL;
  
  if(!(dict = get_image(m, key, i)))
    return NULL;
  
  if(!(val = gavl_dictionary_get(dict, GAVL_META_URI)))
    return NULL;

  ret = gavl_value_get_string(val);

  /* mimetype, width, height */
  if(mimetype && (val = gavl_dictionary_get(dict, GAVL_META_MIMETYPE)))
    *mimetype = gavl_strdup(gavl_value_get_string(val));

  if(wp  && (val = gavl_dictionary_get(dict, GAVL_META_WIDTH)))
    gavl_value_get_int(val, wp);
  if(hp  && (val = gavl_dictionary_get(dict, GAVL_META_HEIGHT)))
    gavl_value_get_int(val, hp);

  return ret;
  }

#if 0
const char *
gavl_metadata_get_image_embedded(gavl_dictionary_t * m,
                                 const char * key,
                                 int idx,
                                 int * w, int * h,
                                 const char ** mimetype,
                                 int64_t * offset,
                                 int64_t * size)
  {
  const gavl_dictionary_t * dict = NULL;
  const gavl_value_t * val;

  if(!(dict = get_image(m, key, idx)))
    return NULL;
  
  if(mimetype)
    *mimetype = gavl_dictionary_get_string(dict, GAVL_META_MIMETYPE);
  
  gavl_dictionary_get_int(dict, GAVL_META_WIDTH, w);
  gavl_dictionary_get_int(dict, GAVL_META_HEIGHT, h);
  gavl_dictionary_get_int(dict, GAVL_META_COVER_OFFSET, offset);
  gavl_dictionary_get_int(dict, GAVL_META_COVER_SIZE, size);
  }
#endif

const gavl_dictionary_t *
gavl_dictionary_get_image_max_proto(const gavl_dictionary_t * m,
                                    const char * key,
                                    int w, int h,
                                    const char * mimetype, const char * protocol)
  {
  int i = 0;
  const gavl_dictionary_t * dict;
  int w_max = 0, i_max = -1;
  int val_w = -1;
  int val_h = -1;
  const char * val_string;
  const char * uri;
  
  while((dict = get_image(m, key, i)))
    {
    if(!gavl_dictionary_get_int(dict, GAVL_META_WIDTH, &val_w))
      val_w = -1;
    
    if(!gavl_dictionary_get_int(dict, GAVL_META_HEIGHT, &val_h))
      val_h = -1;

#if 0
    fprintf(stderr, "Testing image:\n");
    gavl_dictionary_dump(dict, 2);
    fprintf(stderr, "\n");
#endif
    
    /* Too wide */
    if((w > 0) && (val_w > 0) && (val_w > w))
      {
      i++;
      continue;
      }

    /* Too high */
    if((h > 0) && (val_h > 0) && (val_h > h))
      {
      i++;
      continue;
      }
    
    /* mimetype */
    if(mimetype && (val_string = gavl_dictionary_get_string(dict, GAVL_META_MIMETYPE)) && strcmp(val_string, mimetype))
      {
      i++;
      continue;
      }

    uri = gavl_dictionary_get_string(dict, GAVL_META_URI);
    
    if(protocol && 
       (!gavl_string_starts_with(uri, protocol) ||
        !gavl_string_starts_with(uri + strlen(protocol), "://")))
      {
      i++;
      continue;
      }

    /* Check if the file is accessible */
    if(*uri == '/')
      {
      if(access(uri, R_OK))
        {
        i++;
        continue;
        }
      }
    
    if((i_max < 0) || ((val_w > 0) && (w_max < val_w)))
      {
      i_max = i;
      w_max = val_w;
      }

    //    fprintf(stderr, "i_max: %d w_max: %d\n", i_max, w_max);
    
    i++;
    }

  if(i_max < 0)
    i_max = 0;
  
  return get_image(m, key, i_max);
  }

const gavl_dictionary_t *
gavl_dictionary_get_image_max(const gavl_dictionary_t * m,
                              const char * key,
                              int w, int h,
                              const char * mimetype)
  {
  return gavl_dictionary_get_image_max_proto(m, key, w, h, mimetype, NULL);
  }

const char *
gavl_dictionary_get_string_image_max(const gavl_dictionary_t * m,
                                     const char * key,
                                     int w, int h,
                                     const char * mimetype)
  {
  const gavl_dictionary_t * dict;
  if((dict = gavl_dictionary_get_image_max(m, key, w, h, mimetype)))
    return gavl_dictionary_get_string(dict, GAVL_META_URI);
  else
    return NULL;
  }

gavl_dictionary_t *
gavl_metadata_add_src(gavl_dictionary_t * m, const char * key,
                      const char * mimetype, const char * location)
  {
  int idx;
  gavl_value_t val;
  gavl_value_t child_val;
  gavl_dictionary_t * ret;
  gavl_value_t * valp;

  const char * loc;

  idx = 0;

  if(location)
    {
    /* Check if the location is already there */
    while((valp = gavl_dictionary_get_item_nc(m, key, idx)))
      {
      if((ret = gavl_value_get_dictionary_nc(valp)) &&
         (loc = gavl_dictionary_get_string(ret, GAVL_META_URI)) &&
         !strcmp(loc, location))
        return NULL;
      else
        idx++;
      }
    }
  
  ret = NULL;
  idx = 0;
  
  gavl_value_init(&val);
  gavl_value_init(&child_val);

  ret = gavl_value_set_dictionary(&val);
  
  if(mimetype)
    gavl_dictionary_set_string(ret, GAVL_META_MIMETYPE, mimetype);
  if(location)
    gavl_dictionary_set_string(ret, GAVL_META_URI, location);
  
  gavl_dictionary_append_nocopy(m, key, &val);
  idx = gavl_dictionary_find(m, key, 0);
  valp = &m->entries[idx].v;
  
  if(valp->type == GAVL_TYPE_ARRAY)
    return valp->v.array->entries[valp->v.array->num_entries-1].v.dictionary;
  else if(valp->type == GAVL_TYPE_DICTIONARY)
    return valp->v.dictionary;
  else
    return NULL;
  }

const gavl_dictionary_t *
gavl_dictionary_get_src(const gavl_dictionary_t * m, const char * key, int idx,
                        const char ** mimetype, const char ** location)
  {
  const gavl_value_t * val;
  const gavl_dictionary_t * dict;
  
  if(!(val = gavl_dictionary_get_item(m, key, idx)) ||
     (val->type != GAVL_TYPE_DICTIONARY))
    return NULL;

  dict = val->v.dictionary;
  
  if(location)
    *location = gavl_dictionary_get_string(dict, GAVL_META_URI);
  
  if(mimetype)
    *mimetype = gavl_dictionary_get_string(dict, GAVL_META_MIMETYPE);
  return dict;
  }

gavl_dictionary_t *
gavl_dictionary_get_src_nc(gavl_dictionary_t * m, const char * key, int idx)
  {
  gavl_value_t * val;
  gavl_dictionary_t * dict;
  
  if(!(val = gavl_dictionary_get_item_nc(m, key, idx)) ||
     (val->type != GAVL_TYPE_DICTIONARY))
    return NULL;
  
  dict = val->v.dictionary;
  return dict;
  }

int gavl_metadata_has_src(const gavl_dictionary_t * m, const char * key,
                          const char * location)
  {
  int i = 0;
  const char * loc;

  while(gavl_dictionary_get_src(m, key, i, NULL, &loc))
    {
    if(!strcmp(loc, location))
      return 1;
    i++;
    }
  return 0;
  }

