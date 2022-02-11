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

#ifndef GAVL_VALUE_H_INCLUDED
#define GAVL_VALUE_H_INCLUDED

#include <gavl/utils.h>

typedef enum
  {
    GAVL_TYPE_UNDEFINED   = 0,
    GAVL_TYPE_INT         = 1,
    GAVL_TYPE_LONG        = 2,
    GAVL_TYPE_FLOAT       = 3,
    GAVL_TYPE_STRING      = 4,
    GAVL_TYPE_AUDIOFORMAT = 5,
    GAVL_TYPE_VIDEOFORMAT = 6,
    GAVL_TYPE_COLOR_RGB   = 7,
    GAVL_TYPE_COLOR_RGBA  = 8,
    GAVL_TYPE_POSITION    = 9,
    GAVL_TYPE_DICTIONARY  = 10,
    GAVL_TYPE_ARRAY       = 11,
    GAVL_TYPE_BINARY      = 12, // Binary buffer
  } gavl_type_t;

GAVL_PUBLIC
const char * gavl_type_to_string(gavl_type_t type);

GAVL_PUBLIC
gavl_type_t gavl_type_from_string(const char * str);

typedef struct gavl_value_s gavl_value_t;
typedef struct gavl_dict_entry_s gavl_dict_entry_t;
typedef struct gavl_array_s gavl_array_t;

/* Dictionary */

typedef struct
  {
  int entries_alloc;
  int num_entries;
  gavl_dict_entry_t * entries;
  } gavl_dictionary_t;

typedef void (*gavl_dictionary_foreach_func)(void * priv, const char * name,
                                             const gavl_value_t * val);

/* Dictionary functions */

GAVL_PUBLIC
void gavl_dictionary_init(gavl_dictionary_t * d);

GAVL_PUBLIC
void gavl_dictionary_reset(gavl_dictionary_t * d);

GAVL_PUBLIC
const char * gavl_dictionary_get_string(const gavl_dictionary_t * d,
                                        const char * name);

GAVL_PUBLIC
char * gavl_dictionary_get_string_nc(gavl_dictionary_t * d,
                                     const char * name);


GAVL_PUBLIC
const char * gavl_dictionary_get_string_i(const gavl_dictionary_t * d,
                                          const char * name);

/* *_set* functions return 1 is the dictionary was really changed */

GAVL_PUBLIC
int gavl_dictionary_set_string(gavl_dictionary_t * d,
                                const char * name, const char * val);
GAVL_PUBLIC
int gavl_dictionary_set_string_nocopy(gavl_dictionary_t * d,
                                       const char * name, char * val);

GAVL_PUBLIC
int gavl_dictionary_set(gavl_dictionary_t * d,
                         const char * name, const gavl_value_t * val);

GAVL_PUBLIC
int gavl_dictionary_set_i(gavl_dictionary_t * d,
                          const char * name, const gavl_value_t * val);

GAVL_PUBLIC
int gavl_dictionary_set_nocopy(gavl_dictionary_t * d,
                               const char * name, gavl_value_t * val);

GAVL_PUBLIC
int gavl_dictionary_set_nocopy_i(gavl_dictionary_t * d, const char * name,
                                 gavl_value_t * val);

GAVL_PUBLIC
void gavl_dictionary_append(gavl_dictionary_t * d, const char * name,
                            const gavl_value_t * val);

GAVL_PUBLIC
void gavl_dictionary_append_i(gavl_dictionary_t * d, const char * name,
                              const gavl_value_t * val);

GAVL_PUBLIC
void gavl_dictionary_append_nocopy(gavl_dictionary_t * d, const char * name,
                                   gavl_value_t * val);

GAVL_PUBLIC
void gavl_dictionary_append_nocopy_i(gavl_dictionary_t * d, const char * name,
                                     gavl_value_t * val);

GAVL_PUBLIC
int gavl_dictionary_set_int(gavl_dictionary_t * d,
                            const char * name, int val);

GAVL_PUBLIC
int gavl_dictionary_set_long(gavl_dictionary_t * d,
                             const char * name, int64_t val);

GAVL_PUBLIC
int gavl_dictionary_set_float(gavl_dictionary_t * d,
                              const char * name, double val);

GAVL_PUBLIC
int gavl_dictionary_set_dictionary(gavl_dictionary_t * d,
                                   const char * name, const gavl_dictionary_t * dict);

GAVL_PUBLIC
int gavl_dictionary_set_array(gavl_dictionary_t * d,
                              const char * name, const gavl_array_t * arr);

GAVL_PUBLIC
int gavl_dictionary_set_dictionary_nocopy(gavl_dictionary_t * d,
                                          const char * name, gavl_dictionary_t * dict);

GAVL_PUBLIC
const gavl_value_t *
gavl_dictionary_get_item(const gavl_dictionary_t * d, const char * name, int item);

GAVL_PUBLIC
int gavl_dictionary_get_num_items(const gavl_dictionary_t * d, const char * name);

GAVL_PUBLIC
gavl_value_t * gavl_dictionary_get_item_nc(gavl_dictionary_t * d, const char * name, int item);

GAVL_PUBLIC
void gavl_dictionary_delete_item(gavl_dictionary_t * d, const char * name, int item);

GAVL_PUBLIC
const gavl_value_t *
gavl_dictionary_get_item_i(const gavl_dictionary_t * d, const char * name, int item);

GAVL_PUBLIC
void gavl_dictionary_foreach(const gavl_dictionary_t * d, gavl_dictionary_foreach_func, void * priv);

GAVL_PUBLIC
int gavl_dictionary_find(const gavl_dictionary_t * m, const char * name, int ign);

GAVL_PUBLIC
const gavl_value_t * gavl_dictionary_get(const gavl_dictionary_t * d, const char * name);

GAVL_PUBLIC
gavl_value_t * gavl_dictionary_get_nc(gavl_dictionary_t * d, const char * name);

GAVL_PUBLIC
const gavl_value_t * gavl_dictionary_get_i(const gavl_dictionary_t * d, const char * name);

GAVL_PUBLIC
void gavl_dictionary_free(gavl_dictionary_t * d);

GAVL_PUBLIC
void gavl_dictionary_copy(gavl_dictionary_t * dst, const gavl_dictionary_t * src); 

GAVL_PUBLIC
void gavl_dictionary_move(gavl_dictionary_t * dst, gavl_dictionary_t * src);

GAVL_PUBLIC
void gavl_dictionary_dump(const gavl_dictionary_t * m, int indent);

/* src1 has priority */
/* From src2 we take only the tags, which aren't available */

GAVL_PUBLIC
void gavl_dictionary_merge(gavl_dictionary_t * dst,
                           const gavl_dictionary_t * src1,
                           const gavl_dictionary_t * src2);

GAVL_PUBLIC
void gavl_dictionary_merge2(gavl_dictionary_t * dst,
                            const gavl_dictionary_t * src);

GAVL_PUBLIC
void gavl_dictionary_update_fields(gavl_dictionary_t * dst,
                                   const gavl_dictionary_t * src);

GAVL_PUBLIC int
gavl_dictionary_compare(const gavl_dictionary_t * m1,
                        const gavl_dictionary_t * m2);

GAVL_PUBLIC
gavl_dictionary_t *
gavl_dictionary_get_dictionary_create(gavl_dictionary_t *, const char * name);

GAVL_PUBLIC
gavl_array_t *
gavl_dictionary_get_array_create(gavl_dictionary_t * d, const char * name);


GAVL_PUBLIC void
gavl_dictionary_delete_fields(gavl_dictionary_t * m, const char * fields[]);

GAVL_PUBLIC
int gavl_dictionary_get_int(const gavl_dictionary_t * d, const char * name, int * ret);

GAVL_PUBLIC
int gavl_dictionary_get_int_i(const gavl_dictionary_t * d, const char * name, int * ret);


GAVL_PUBLIC
int gavl_dictionary_get_long(const gavl_dictionary_t * d, const char * name, int64_t * ret);

GAVL_PUBLIC
int gavl_dictionary_get_float(const gavl_dictionary_t * d, const char * name, double * ret);

GAVL_PUBLIC
const gavl_array_t * gavl_dictionary_get_array(const gavl_dictionary_t * d, const char * name);

GAVL_PUBLIC
gavl_array_t * gavl_dictionary_get_array_nc(gavl_dictionary_t * d, const char * name);

GAVL_PUBLIC
const gavl_dictionary_t *  gavl_dictionary_get_dictionary(const gavl_dictionary_t * d, const char * name);

GAVL_PUBLIC
gavl_dictionary_t *  gavl_dictionary_get_dictionary_nc(gavl_dictionary_t * d, const char * name);

GAVL_PUBLIC
gavl_buffer_t * gavl_dictionary_get_binary_nc(gavl_dictionary_t * d, const char * name);

GAVL_PUBLIC
const gavl_buffer_t * gavl_dictionary_get_binary(const gavl_dictionary_t * d, const char * name);

GAVL_PUBLIC
void gavl_dictionary_set_binary(const gavl_dictionary_t * d, const char * name, const uint8_t * buf, int len);

GAVL_PUBLIC
void gavl_dictionary_set_audio_format(gavl_dictionary_t * d, const char * name,
                                      const gavl_audio_format_t * fmt);

GAVL_PUBLIC
void gavl_dictionary_set_video_format(gavl_dictionary_t * d, const char * name,
                                      const gavl_video_format_t * fmt);

GAVL_PUBLIC
const gavl_audio_format_t * gavl_dictionary_get_audio_format(const gavl_dictionary_t * d, const char * name);

GAVL_PUBLIC
const gavl_video_format_t * gavl_dictionary_get_video_format(const gavl_dictionary_t * d, const char * name);

GAVL_PUBLIC
gavl_audio_format_t * gavl_dictionary_get_audio_format_nc(gavl_dictionary_t * d, const char * name);

GAVL_PUBLIC
gavl_video_format_t * gavl_dictionary_get_video_format_nc(gavl_dictionary_t * d, const char * name);


/* For compatibility with old heap API */

GAVL_PUBLIC
gavl_dictionary_t *  gavl_dictionary_create();

GAVL_PUBLIC
gavl_dictionary_t * gavl_dictionary_clone(const gavl_dictionary_t * src);

GAVL_PUBLIC
void gavl_dictionary_destroy(gavl_dictionary_t *);



/*
 *  Can be used within a foreach_func to know, if this is the last entry
 */

GAVL_PUBLIC
int gavl_dictionary_is_last(const gavl_dictionary_t * d, const char * name);


/* Convert formats to and from dictionaries */

GAVL_PUBLIC
void gavl_audio_format_to_dictionary(const gavl_audio_format_t * fmt, gavl_dictionary_t * dict);

GAVL_PUBLIC
int gavl_audio_format_from_dictionary(gavl_audio_format_t * fmt, const gavl_dictionary_t * dict);

GAVL_PUBLIC
void gavl_video_format_to_dictionary(const gavl_video_format_t * fmt, gavl_dictionary_t * dict);

GAVL_PUBLIC
int gavl_video_format_from_dictionary(gavl_video_format_t * fmt, const gavl_dictionary_t * dict);

/* Array */

struct gavl_array_s
  {
  int entries_alloc;
  int num_entries;
  struct gavl_value_s * entries;
  };

typedef void (*gavl_array_foreach_func)(void * priv, int idx,
                                        const gavl_value_t * val);


GAVL_PUBLIC
void gavl_array_init(gavl_array_t * d);

GAVL_PUBLIC
void gavl_array_reset(gavl_array_t * d);

GAVL_PUBLIC
void gavl_array_push(gavl_array_t * d, const gavl_value_t * val);

GAVL_PUBLIC
void gavl_array_push_nocopy(gavl_array_t * d, gavl_value_t * val);

GAVL_PUBLIC
void gavl_array_unshift(gavl_array_t * d, const gavl_value_t * val);

GAVL_PUBLIC
void gavl_array_unshift_nocopy(gavl_array_t * d, gavl_value_t * val);

GAVL_PUBLIC
int gavl_array_pop(gavl_array_t * d, gavl_value_t * val);

GAVL_PUBLIC
int gavl_array_shift(gavl_array_t * d, gavl_value_t * val);

GAVL_PUBLIC
const gavl_value_t * gavl_array_get(const gavl_array_t * d, int idx);

GAVL_PUBLIC
gavl_value_t * gavl_array_get_nc(gavl_array_t * d, int idx);

GAVL_PUBLIC
void gavl_array_free(gavl_array_t * d);

GAVL_PUBLIC
void gavl_array_copy(gavl_array_t * dst, const gavl_array_t * src); 

GAVL_PUBLIC
void gavl_array_copy_sub(gavl_array_t * dst, const gavl_array_t * src, int start, int num);

GAVL_PUBLIC
void gavl_array_move(gavl_array_t * dst, gavl_array_t * src); 

GAVL_PUBLIC
void gavl_array_dump(const gavl_array_t * a, int indent);

GAVL_PUBLIC
void gavl_array_foreach(const gavl_array_t * a,
                        gavl_array_foreach_func func, void * priv);

GAVL_PUBLIC
int gavl_array_compare(const gavl_array_t * m1,
                       const gavl_array_t * m2);

GAVL_PUBLIC
void gavl_array_foreach(const gavl_array_t * a,
                        gavl_array_foreach_func func, void * priv);

GAVL_PUBLIC
int gavl_array_move_entry(gavl_array_t * m1,
                          int src_pos, int dst_pos);

GAVL_PUBLIC
void gavl_array_splice_val(gavl_array_t * arr,
                           int idx, int del,
                           const gavl_value_t * add);

GAVL_PUBLIC
void gavl_array_splice_array(gavl_array_t * arr,
                             int idx, int del,
                             const gavl_array_t * add);

GAVL_PUBLIC
void gavl_array_splice_val_nocopy(gavl_array_t * arr,
                                 int idx, int del,
                                 gavl_value_t * add);

GAVL_PUBLIC
void gavl_array_splice_array_nocopy(gavl_array_t * arr,
                                   int idx, int del,
                                   gavl_array_t * add);

// uses qsort(3)
GAVL_PUBLIC
void gavl_array_sort(gavl_array_t * arr, int (*compare)(const void *, const void *));

GAVL_PUBLIC
void gavl_dictionary_append_string_array_nocopy(gavl_dictionary_t * d,
                                                const char * key, char * val);

GAVL_PUBLIC
void gavl_dictionary_append_string_array(gavl_dictionary_t * d,
                                         const char * key, const char * val);

GAVL_PUBLIC
const char * gavl_dictionary_get_string_array(const gavl_dictionary_t * d,
                                              const char * key, int idx);

GAVL_PUBLIC
int gavl_dictionary_has_string_array(const gavl_dictionary_t * d,
                                     const char * key, const char * val);



/* For compatibility with old heap API */

GAVL_PUBLIC
gavl_array_t *  gavl_array_create();

GAVL_PUBLIC
void gavl_array_destroy(gavl_array_t *);


/* Value */

struct gavl_value_s
  {
  gavl_type_t type;
  
  union
    {
    int    i;
    int64_t l;
    double d;
    char * str;
    double color[4];
    double position[2];
    gavl_dictionary_t * dictionary;
    gavl_array_t * array;
    gavl_audio_format_t * audioformat;
    gavl_video_format_t * videoformat;
    gavl_buffer_t * buffer;
    } v;
  };

struct gavl_dict_entry_s
  {
  char * name;
  gavl_value_t v;
  };

/* Value functions */

GAVL_PUBLIC
int gavl_value_compare(const gavl_value_t * v1, const gavl_value_t * v2);

GAVL_PUBLIC
void gavl_value_copy(gavl_value_t * dst, const gavl_value_t * src);

GAVL_PUBLIC
void gavl_value_move(gavl_value_t * dst, gavl_value_t * src);

GAVL_PUBLIC
void gavl_value_dump(const gavl_value_t * v, int indent);

GAVL_PUBLIC
void gavl_value_free(gavl_value_t * v);

GAVL_PUBLIC
void gavl_value_init(gavl_value_t * v);

GAVL_PUBLIC
void gavl_value_reset(gavl_value_t * v);

GAVL_PUBLIC
void gavl_value_append_nocopy(gavl_value_t * v, gavl_value_t * child);

GAVL_PUBLIC
void gavl_value_append(gavl_value_t * v, const gavl_value_t * child);

GAVL_PUBLIC
int gavl_value_get_num_items(const gavl_value_t * v);

GAVL_PUBLIC
const gavl_value_t * gavl_value_get_item(const gavl_value_t * v, int item);

GAVL_PUBLIC
void gavl_value_delete_item(gavl_value_t * v, int item);

GAVL_PUBLIC
gavl_value_t * gavl_value_get_item_nc(gavl_value_t * v, int item);

GAVL_PUBLIC
char * gavl_value_join_arr(const gavl_value_t * val, const char * glue);

GAVL_PUBLIC
void gavl_value_set_type(gavl_value_t * v, gavl_type_t  t);


GAVL_PUBLIC
void gavl_value_set_int(gavl_value_t * v, int val);

GAVL_PUBLIC
void gavl_value_set_float(gavl_value_t * v, double val);
GAVL_PUBLIC
void gavl_value_set_long(gavl_value_t * v, int64_t val);
GAVL_PUBLIC
void gavl_value_set_string(gavl_value_t * v, const char * str);
GAVL_PUBLIC
void gavl_value_set_string_nocopy(gavl_value_t * v, char * str);

GAVL_PUBLIC
gavl_audio_format_t * gavl_value_set_audio_format(gavl_value_t * v);
GAVL_PUBLIC
gavl_video_format_t * gavl_value_set_video_format(gavl_value_t * v);
GAVL_PUBLIC
gavl_dictionary_t * gavl_value_set_dictionary(gavl_value_t * v);

GAVL_PUBLIC
gavl_buffer_t * gavl_value_set_binary(gavl_value_t * v);

GAVL_PUBLIC
const gavl_buffer_t * gavl_value_get_binary(const gavl_value_t * v);

GAVL_PUBLIC
gavl_buffer_t * gavl_value_get_binary_nc(gavl_value_t * v);

GAVL_PUBLIC
void gavl_value_set_dictionary_nocopy(gavl_value_t * v, gavl_dictionary_t * val);

GAVL_PUBLIC
gavl_array_t * gavl_value_set_array(gavl_value_t * v);

GAVL_PUBLIC
void gavl_value_set_array_nocopy(gavl_value_t * v, gavl_array_t * val);

GAVL_PUBLIC
double * gavl_value_set_position(gavl_value_t * v);
GAVL_PUBLIC
double * gavl_value_set_color_rgb(gavl_value_t * v);
GAVL_PUBLIC
double * gavl_value_set_color_rgba(gavl_value_t * v);

/* Get value */

GAVL_PUBLIC
int gavl_value_get_int(const gavl_value_t * v, int * val);
GAVL_PUBLIC
int gavl_value_get_float(const gavl_value_t * v, double * val);
GAVL_PUBLIC
int gavl_value_get_long(const gavl_value_t * v, int64_t * val);

GAVL_PUBLIC
const char * gavl_value_get_string(const gavl_value_t * v);

GAVL_PUBLIC
char * gavl_value_get_string_nc(gavl_value_t * v);

GAVL_PUBLIC
char * gavl_value_to_string(const gavl_value_t * v);

GAVL_PUBLIC
void gavl_value_from_string(gavl_value_t * v, const char * str);



GAVL_PUBLIC
const gavl_audio_format_t * gavl_value_get_audio_format(const gavl_value_t * v);

GAVL_PUBLIC
gavl_audio_format_t * gavl_value_get_audio_format_nc(gavl_value_t * v);


GAVL_PUBLIC
const gavl_video_format_t * gavl_value_get_video_format(const gavl_value_t * v);

GAVL_PUBLIC
gavl_video_format_t * gavl_value_get_video_format_nc(gavl_value_t * v);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_value_get_dictionary(const gavl_value_t * v);

GAVL_PUBLIC
gavl_dictionary_t * gavl_value_get_dictionary_nc(gavl_value_t * v);

GAVL_PUBLIC
const gavl_array_t * gavl_value_get_array(const gavl_value_t * v);

GAVL_PUBLIC
gavl_array_t * gavl_value_get_array_nc(gavl_value_t * v);


GAVL_PUBLIC
const double * gavl_value_get_position(const gavl_value_t * v);
GAVL_PUBLIC
const double * gavl_value_get_color_rgb(const gavl_value_t * v);
GAVL_PUBLIC
const double * gavl_value_get_color_rgba(const gavl_value_t * v);

/* String array (convenience wrapper) */

GAVL_PUBLIC
void gavl_string_array_add(gavl_array_t * arr, const char * str);

GAVL_PUBLIC
void gavl_string_array_insert_at(gavl_array_t * arr, int idx, const char * str);

GAVL_PUBLIC
void gavl_string_array_add_nocopy(gavl_array_t * arr, char * str);

GAVL_PUBLIC
void gavl_string_array_delete(gavl_array_t * arr, const char * str);

GAVL_PUBLIC
int gavl_string_array_indexof(const gavl_array_t * arr, const char * str);

GAVL_PUBLIC
const char * gavl_string_array_get(const gavl_array_t * arr, int idx);
 
GAVL_PUBLIC
char * gavl_string_array_join(const gavl_array_t * arr, const char * glue);

/* For initializing values statically */
#define GAVL_VALUE_INIT_INT(val)               { .type = GAVL_TYPE_INT, .v.i = val }
#define GAVL_VALUE_INIT_LONG(val)              { .type = GAVL_TYPE_LONG, .v.l = val }
#define GAVL_VALUE_INIT_FLOAT(val)             { .type = GAVL_TYPE_FLOAT, .v.d = val }
#define GAVL_VALUE_INIT_STRING(val)            { .type = GAVL_TYPE_STRING, .v.str = val }
#define GAVL_VALUE_INIT_COLOR_RGB(r, g, b)     { .type = GAVL_TYPE_COLOR_RGB,  .v.color = {r, g, b, 1.0 } }
#define GAVL_VALUE_INIT_COLOR_RGBA(r, g, b, a) { .type = GAVL_TYPE_COLOR_RGBA, .v.color = {r, g, b, a } }
#define GAVL_VALUE_INIT_POSITION(x, y) { .type = GAVL_TYPE_POSITION, .v.position = {x, y} }

#endif // GAVL_VALUE_H_INCLUDED
