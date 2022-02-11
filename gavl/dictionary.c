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
#include <stdlib.h>

#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/value.h>

#include <gavl/metatags.h>

// #define WATCH_MEMBER GAVL_META_NUM_CHILDREN

/* Dictionary */

void gavl_dictionary_init(gavl_dictionary_t * d)
  {
  memset(d, 0, sizeof(*d));
  }

void gavl_dictionary_reset(gavl_dictionary_t * d)
  {
  gavl_dictionary_free(d);
  gavl_dictionary_init(d);
  }


int gavl_dictionary_find(const gavl_dictionary_t * m, const char * name, int ign)
  {
  int i;

  if(ign)
    {
    for(i = 0; i < m->num_entries; i++)
      {
      if(!strcasecmp(m->entries[i].name, name))
        return i;
      }
    }
  else
    {
    for(i = 0; i < m->num_entries; i++)
      {
      if(!strcmp(m->entries[i].name, name))
        return i;
      }
    }
  return -1;
  }

static gavl_dict_entry_t * dict_append(gavl_dictionary_t * m, const char * name)
  {
  gavl_dict_entry_t * ret;
  if(m->num_entries == m->entries_alloc)
    {
    m->entries_alloc += 128;
    m->entries = realloc(m->entries, m->entries_alloc * sizeof(*m->entries));
    memset(m->entries + m->num_entries, 0, (m->entries_alloc - m->num_entries) * sizeof(*m->entries));
    }
  ret = m->entries + m->num_entries;
  ret->name = gavl_strdup(name);
  m->num_entries++;
  return ret;
  }

static void dict_free_entry(gavl_dict_entry_t * e)
  {
  if(e->name)
    free(e->name);
  gavl_value_free(&e->v);
  }

static int
dict_set(gavl_dictionary_t * d, const char * name, const gavl_value_t * val,
         int ign,
         int cpy)
  {
  int idx;
  gavl_dict_entry_t * e;

#ifdef WATCH_MEMBER
  if(!strcmp(name, WATCH_MEMBER))
    {
    if(!val)
      gavl_dprintf("Dictionary member %s cleared\n", WATCH_MEMBER);
    else
      {
      gavl_dprintf("Member %s of dictionary %s set, new value:\n", WATCH_MEMBER, gavl_dictionary_get_string(d, GAVL_META_LABEL));
      gavl_value_dump(val, 2);
      gavl_dprintf("\n");
      }
    }

#endif  

  /* Value is NULL: Delete entry */
  if(!val)
    {
    if((idx = gavl_dictionary_find(d, name, ign)) >= 0)
      {
      dict_free_entry(d->entries + idx);
      if(idx < d->num_entries - 1)
        {
        memmove(d->entries + idx,
                d->entries + idx + 1,
                (d->num_entries - 1 - idx) * sizeof(*d->entries));
        memset(d->entries + (d->num_entries - 1), 0, sizeof(*d->entries));
        }
      else
        memset(d->entries + idx, 0, sizeof(*d->entries));
      
      d->num_entries--;
      }
    return 1;
    }
  
  if((idx = gavl_dictionary_find(d, name, ign)) >= 0)
    {
    e = d->entries + idx;
    
    /* Check for equality */
    if(!gavl_value_compare(&e->v, val))
      return 0; // Noop
    
    /* Replace */
    gavl_value_free(&e->v);
    gavl_value_init(&e->v);
    }
  else
    e = dict_append(d, name);
  
  if(cpy)
    gavl_value_copy(&e->v, val);
  else
    memcpy(&e->v, val, sizeof(*val));

  return 1;
  }

int gavl_dictionary_set_string(gavl_dictionary_t * d,
                                const char * name, const char * val)
  {
  if(!val)
    {
    return gavl_dictionary_set(d, name, NULL);
    }
  else
    {
    gavl_value_t v;
    gavl_value_init(&v);
    gavl_value_set_string(&v, val);
    return gavl_dictionary_set_nocopy(d, name, &v);
    }
  }

int gavl_dictionary_set_int(gavl_dictionary_t * d,
                            const char * name, int val)
  {
  gavl_value_t v;
  gavl_value_init(&v);
  gavl_value_set_int(&v, val);
  return gavl_dictionary_set_nocopy(d, name, &v);
  }

int gavl_dictionary_set_long(gavl_dictionary_t * d,
                             const char * name, int64_t val)
  {
  gavl_value_t v;
  gavl_value_init(&v);
  gavl_value_set_long(&v, val);
  return gavl_dictionary_set_nocopy(d, name, &v);
  }

int gavl_dictionary_set_float(gavl_dictionary_t * d,
                              const char * name, double val)
  {
  gavl_value_t v;
  gavl_value_init(&v);
  gavl_value_set_float(&v, val);
  return gavl_dictionary_set_nocopy(d, name, &v);
  }

int gavl_dictionary_set_string_nocopy(gavl_dictionary_t * d,
                                       const char * name, char * val)
  {
  gavl_value_t v;
  gavl_value_init(&v);
  gavl_value_set_string_nocopy(&v, val);
  return gavl_dictionary_set_nocopy(d, name, &v);
  }

int gavl_dictionary_set_dictionary_nocopy(gavl_dictionary_t * d,
                                          const char * name, gavl_dictionary_t * dict)
  {
  gavl_value_t v;
  gavl_value_init(&v);
  gavl_value_set_dictionary_nocopy(&v, dict);
  return gavl_dictionary_set_nocopy(d, name, &v);
  }


int gavl_dictionary_set_dictionary(gavl_dictionary_t * d,
                                   const char * name, const gavl_dictionary_t * dict)
  {
  gavl_dictionary_t * dst;
  gavl_value_t v;
  gavl_value_init(&v);
  
  dst = gavl_value_set_dictionary(&v);
  gavl_dictionary_copy(dst, dict);
  return gavl_dictionary_set_nocopy(d, name, &v);
  }

int gavl_dictionary_set_array(gavl_dictionary_t * d,
                              const char * name, const gavl_array_t * arr)
  {
  gavl_array_t * dst;
  gavl_value_t v;
  gavl_value_init(&v);
  
  dst = gavl_value_set_array(&v);
  gavl_array_copy(dst, arr);
  return gavl_dictionary_set_nocopy(d, name, &v);
  }

void gavl_dictionary_set_binary(const gavl_dictionary_t * d, const char * name, const uint8_t * buf, int len)
  {
  gavl_buffer_t * b;
  gavl_value_t v;
  gavl_value_init(&v);
  
  b = gavl_value_set_binary(&v);

  gavl_buffer_alloc(b, len);
  memcpy(b->buf, buf, len);
  b->len = len;
  }


int gavl_dictionary_set(gavl_dictionary_t * d, const char * name,
                         const gavl_value_t * val)
  {
  return dict_set(d, name, val, 0, 1);
  }

int gavl_dictionary_set_i(gavl_dictionary_t * d, const char * name,
                           const gavl_value_t * val)
  {
  return dict_set(d, name, val, 1, 1);
  }

int gavl_dictionary_set_nocopy(gavl_dictionary_t * d, const char * name,
                               gavl_value_t * val)
  {
  int ret = dict_set(d, name, val, 0, 0);
  if(!ret)
    gavl_value_free(val);
  gavl_value_init(val);
  return ret;
  }

int gavl_dictionary_set_nocopy_i(gavl_dictionary_t * d, const char * name,
                                  gavl_value_t * val)
  {
  int ret = dict_set(d, name, val, 1, 0);
  if(!ret)
    gavl_value_free(val);
  gavl_value_init(val);
  return ret;
  }

const gavl_value_t * gavl_dictionary_get(const gavl_dictionary_t * d, const char * name)
  {
  int idx;
  if((idx = gavl_dictionary_find(d, name, 0)) >= 0)
    return &d->entries[idx].v;
  else
    return NULL;
  }

gavl_value_t * gavl_dictionary_get_nc(gavl_dictionary_t * d, const char * name)
  {
  int idx;
  if((idx = gavl_dictionary_find(d, name, 0)) >= 0)
    return &d->entries[idx].v;
  else
    return NULL;
  }

int gavl_dictionary_get_int(const gavl_dictionary_t * d, const char * name, int * ret)
  {
  const gavl_value_t * val;
  if(!(val = gavl_dictionary_get(d, name)))
    return 0;
  return gavl_value_get_int(val, ret);
  }

int gavl_dictionary_get_int_i(const gavl_dictionary_t * d, const char * name, int * ret)
  {
  const gavl_value_t * val;
  if(!(val = gavl_dictionary_get_i(d, name)))
    return 0;
  return gavl_value_get_int(val, ret);
  }

int gavl_dictionary_get_long(const gavl_dictionary_t * d, const char * name, int64_t * ret)
  {
  const gavl_value_t * val;
  if(!(val = gavl_dictionary_get(d, name)))
    return 0;
  return gavl_value_get_long(val, ret);
  }

int gavl_dictionary_get_float(const gavl_dictionary_t * d, const char * name, double * ret)
  {
  const gavl_value_t * val;
  if(!(val = gavl_dictionary_get(d, name)))
    return 0;
  return gavl_value_get_float(val, ret);
  }

const gavl_array_t * gavl_dictionary_get_array(const gavl_dictionary_t * d, const char * name)
  {
  const gavl_value_t * val;
  if(!(val = gavl_dictionary_get(d, name)))
    return NULL;
  return gavl_value_get_array(val);
  }

gavl_array_t * gavl_dictionary_get_array_nc(gavl_dictionary_t * d, const char * name)
  {
  gavl_value_t * val;
  if(!(val = gavl_dictionary_get_nc(d, name)))
    return NULL;
  return gavl_value_get_array_nc(val);
  }

const gavl_dictionary_t *  gavl_dictionary_get_dictionary(const gavl_dictionary_t * d, const char * name)
  {
  const gavl_value_t * val;
  if(!(val = gavl_dictionary_get(d, name)))
    return NULL;
  return gavl_value_get_dictionary(val);
  }

gavl_dictionary_t *  gavl_dictionary_get_dictionary_nc(gavl_dictionary_t * d, const char * name)
  {
  gavl_value_t * val;
  if(!(val = gavl_dictionary_get_nc(d, name)))
    return NULL;
  return gavl_value_get_dictionary_nc(val);
  }

gavl_buffer_t * gavl_dictionary_get_binary_nc(gavl_dictionary_t * d, const char * name)
  {
  gavl_value_t * val;
  if(!(val = gavl_dictionary_get_nc(d, name)))
    return NULL;
  return gavl_value_get_binary_nc(val);
  }

const gavl_buffer_t * gavl_dictionary_get_binary(const gavl_dictionary_t * d, const char * name)
  {
  const gavl_value_t * val;
  if(!(val = gavl_dictionary_get(d, name)))
    return NULL;
  return gavl_value_get_binary(val);
  }




const gavl_value_t * gavl_dictionary_get_i(const gavl_dictionary_t * d, const char * name)
  {
  int idx;
  if((idx = gavl_dictionary_find(d, name, 1)) >= 0)
    return &d->entries[idx].v;
  else
    return NULL;
  }

const gavl_value_t * gavl_dictionary_get_item(const gavl_dictionary_t * d, const char * name, int item)
  {
  int idx;
  if((idx = gavl_dictionary_find(d, name, 0)) < 0)
    return NULL;
  return gavl_value_get_item(&d->entries[idx].v, item);
  }

int gavl_dictionary_get_num_items(const gavl_dictionary_t * d, const char * name)
  {
  int idx;
  if((idx = gavl_dictionary_find(d, name, 0)) < 0)
    return 0;
  return gavl_value_get_num_items(&d->entries[idx].v);
  }
  
GAVL_PUBLIC
void gavl_dictionary_delete_item(gavl_dictionary_t * d, const char * name, int item)
  {
  int idx;
  if((idx = gavl_dictionary_find(d, name, 0)) < 0)
    return;
  return gavl_value_delete_item(&d->entries[idx].v, item);
  }


gavl_value_t * gavl_dictionary_get_item_nc(gavl_dictionary_t * d, const char * name, int item)
  {
  int idx;
  if((idx = gavl_dictionary_find(d, name, 0)) < 0)
    return NULL;
  return gavl_value_get_item_nc(&d->entries[idx].v, item);
  }


const gavl_value_t * gavl_dictionary_get_item_i(const gavl_dictionary_t * d, const char * name, int item)
  {
  int idx;
  if((idx = gavl_dictionary_find(d, name, 1)) < 0)
    return NULL;
  return gavl_value_get_item(&d->entries[idx].v, item);
  }

const char * gavl_dictionary_get_string(const gavl_dictionary_t * d,
                                        const char * name)
  {
  const gavl_value_t * v;
  if(!(v = gavl_dictionary_get(d, name)) ||
     (v->type != GAVL_TYPE_STRING))
    return NULL;
  return v->v.str;
  }

char * gavl_dictionary_get_string_nc(gavl_dictionary_t * d,
                                     const char * name)
  {
  gavl_value_t * v;
  if(!(v = gavl_dictionary_get_nc(d, name)) ||
     (v->type != GAVL_TYPE_STRING))
    return NULL;
  return v->v.str;
  }

const char * gavl_dictionary_get_string_i(const gavl_dictionary_t * d,
                                          const char * name)
  {
  const gavl_value_t * v;
    if(!(v = gavl_dictionary_get_i(d, name)) ||
     (v->type != GAVL_TYPE_STRING))
    return NULL;
  return v->v.str;
  }

void gavl_dictionary_append_string_array_nocopy(gavl_dictionary_t * d,
                                                const char * key, char * str)
  {
  gavl_value_t * val;
  val = gavl_dictionary_get_nc(d, key);
  
  if(!val)
    gavl_dictionary_set_string_nocopy(d, key, str);
  else
    {
    gavl_value_t str_val;
    gavl_value_init(&str_val);
    gavl_value_set_string_nocopy(&str_val, str);
    gavl_value_append_nocopy(val, &str_val);
    }
  
  }

void gavl_dictionary_append_string_array(gavl_dictionary_t * d,
                                         const char * key, const char * val)
  {
  gavl_dictionary_append_string_array_nocopy(d, key, gavl_strdup(val));
  }

const char * gavl_dictionary_get_string_array(const gavl_dictionary_t * d,
                                              const char * key, int idx)
  {
  const gavl_value_t * val;
  const gavl_value_t * el;
  
  if((val = gavl_dictionary_get(d, key)) &&
     (el = gavl_value_get_item(val, idx)))
    return gavl_value_get_string(el);
  else
    return NULL;
  }

int gavl_dictionary_has_string_array(const gavl_dictionary_t * d,
                                     const char * key, const char * val)
  {
  const char * str;
  int idx;

  idx = 0;
  while((str = gavl_dictionary_get_string_array(d, key, idx)))
    {
    if(!strcmp(str, val))
      return 1;
    idx++;
    }
  return 0;
  }

void gavl_dictionary_free(gavl_dictionary_t * d)
  {
  int i;
  if(d->entries)
    {
    for(i = 0; i < d->num_entries; i++)
      dict_free_entry(d->entries + i);
    free(d->entries);
    }
  }

void gavl_dictionary_foreach(const gavl_dictionary_t * d, gavl_dictionary_foreach_func func, void * priv)
  {
  gavl_dict_entry_t * e;
  int i = 0;

  while(i < d->num_entries)
    {
    e = d->entries + i;
    func(priv, e->name, &e->v);
    i++;
    }
  }

static void dict_copy_func(void * priv, const char * name, const gavl_value_t * val)
  {
  gavl_dictionary_t * dst = priv;
  gavl_dictionary_set(dst, name, val);
  }

void gavl_dictionary_copy(gavl_dictionary_t * dst, const gavl_dictionary_t * src)
  {
  gavl_dictionary_foreach(src, dict_copy_func, dst);
  }

gavl_dictionary_t * gavl_dictionary_clone(const gavl_dictionary_t * src)
  {
  gavl_dictionary_t * ret;
  
  if(!src)
    return NULL;
  
  ret = gavl_dictionary_create();
  gavl_dictionary_copy(ret, src);
  return ret;
  }

void gavl_dictionary_move(gavl_dictionary_t * dst, gavl_dictionary_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  gavl_dictionary_init(src);
  }
  
void gavl_dictionary_dump(const gavl_dictionary_t * m, int indent)
  {
  int len, i, j;
  int max_key_len = 0;

  gavl_diprintf(indent, "{\n");

  
  for(i = 0; i < m->num_entries; i++)
    {
    len = strlen(m->entries[i].name);
    if(len > max_key_len)
      max_key_len = len;
    }
  
  for(i = 0; i < m->num_entries; i++)
    {
    len = strlen(m->entries[i].name);
    
    for(j = 0; j < indent; j++)
      fprintf(stderr, " ");

    fprintf(stderr, "%s: ", m->entries[i].name);
    
    for(j = 0; j < max_key_len - len; j++)
      fprintf(stderr, " ");
    gavl_value_dump(&m->entries[i].v, indent + 2);

    fprintf(stderr, "\n");
    }
  gavl_diprintf(indent, "}\n");
  }

static void merge_func_r(void * priv, const char * name, const gavl_value_t * val)
  {
  gavl_dictionary_t * dst = priv;
  gavl_dictionary_set(dst, name, val);
  }

/* Not replacing */
static void merge_func_nr(void * priv, const char * name, const gavl_value_t * val)
  {
  gavl_dictionary_t * dst = priv;
  if(!gavl_dictionary_get(dst, name))
    gavl_dictionary_set(dst, name, val);
  }
  
void gavl_dictionary_merge(gavl_dictionary_t * dst,
                           const gavl_dictionary_t * src1,
                           const gavl_dictionary_t * src2)
  {
  /* src1 has priority */
  gavl_dictionary_foreach(src1, merge_func_r, dst);
  /* From src2 we take only the tags, which aren't available */
  gavl_dictionary_foreach(src2, merge_func_nr, dst);
  }

void gavl_dictionary_merge2(gavl_dictionary_t * dst,
                            const gavl_dictionary_t * src)
  {
  gavl_dictionary_foreach(src, merge_func_nr, dst);
  }

void gavl_dictionary_update_fields(gavl_dictionary_t * dst,
                                   const gavl_dictionary_t * src)
  {
  gavl_dictionary_foreach(src, merge_func_r, dst);
  }

int
gavl_dictionary_compare(const gavl_dictionary_t * m1,
                        const gavl_dictionary_t * m2)
  {
  int i;
  int result = 1;
  const gavl_value_t * v1;
  const gavl_value_t * v2;

  /* Check if tags from m1 are present in m2 */
  for(i = 0; i < m1->num_entries; i++)
    {
    v1 = &m1->entries[i].v;
    v2 = gavl_dictionary_get(m2, m1->entries[i].name);
    if(!v2 || (result = gavl_value_compare(v1, v2)))
      return result;
    }
  
  /* Check if tags from m2 are present in m1 */
  for(i = 0; i < m2->num_entries; i++)
    {
    if(!gavl_dictionary_get(m1, m2->entries[i].name))
      return 1;
    }
  return 0;
  }

void
gavl_dictionary_delete_fields(gavl_dictionary_t * m, const char * fields[])
  {
  int found;
  int i, j;

  i = 0;
  while(i < m->num_entries)
    {
    j = 0;

    found = 0;
    
    while(fields[j])
      {
      if(!strcmp(fields[j], m->entries[i].name))
        {
        gavl_dictionary_set(m, fields[j], NULL);
        found = 1;
        break;
        }
      j++;
      }
    if(!found)
      i++;
    }
  }

static void dict_append_internal(gavl_dictionary_t * d, const char * name,
                                 gavl_value_t * val,
                                 const gavl_value_t * val_c,
                                 int ign)
  {
  int idx;
  gavl_dict_entry_t * e;
  
  if((idx = gavl_dictionary_find(d, name, ign)) >= 0)
    e = &d->entries[idx];
  else
    e = dict_append(d, name);

  if(val)
    gavl_value_append_nocopy(&e->v, val);
  else
    gavl_value_append(&e->v, val_c);
  }

void gavl_dictionary_append(gavl_dictionary_t * d, const char * name,
                            const gavl_value_t * val)
  {
  dict_append_internal(d, name, NULL, val, 0);
  }

void gavl_dictionary_append_i(gavl_dictionary_t * d, const char * name,
                              const gavl_value_t * val)
  {
  dict_append_internal(d, name, NULL, val, 1);
  }

void gavl_dictionary_append_nocopy(gavl_dictionary_t * d, const char * name,
                                   gavl_value_t * val)
  {
  dict_append_internal(d, name, val, NULL, 0);

  }


void gavl_dictionary_append_nocopy_i(gavl_dictionary_t * d, const char * name,
                                     gavl_value_t * val)
  {
  dict_append_internal(d, name, val, NULL, 1);
  }

gavl_dictionary_t *
gavl_dictionary_get_dictionary_create(gavl_dictionary_t * d, const char * name)
  {
  gavl_value_t val;
  int idx;
  if((idx = gavl_dictionary_find(d, name, 0)) >= 0)
    {
    if(d->entries[idx].v.type == GAVL_TYPE_DICTIONARY)
      return d->entries[idx].v.v.dictionary;
    else
      return NULL; // Should never happen if the naming schemes are sane
    }

  gavl_value_init(&val);
  gavl_value_set_dictionary(&val);
  dict_set(d, name, &val, 0 /* ign */, 0 /* cpy */ );

  idx = gavl_dictionary_find(d, name, 0);
  return d->entries[idx].v.v.dictionary;
  }

gavl_array_t *
gavl_dictionary_get_array_create(gavl_dictionary_t * d, const char * name)
  {
  gavl_value_t val;
  int idx;
  if((idx = gavl_dictionary_find(d, name, 0)) >= 0)
    {
    if(d->entries[idx].v.type == GAVL_TYPE_ARRAY)
      return d->entries[idx].v.v.array;
    else
      return NULL; // Should never happen if the naming schemes are sane
    }

  gavl_value_init(&val);
  gavl_value_set_array(&val);
  dict_set(d, name, &val, 0 /* ign */, 0 /* cpy */ );

  idx = gavl_dictionary_find(d, name, 0);
  return d->entries[idx].v.v.array;
  }

int gavl_dictionary_is_last(const gavl_dictionary_t * d, const char * name)
  {
  if(!strcmp(d->entries[d->num_entries-1].name, name))
    return 1;
  else
    return 0;
  }

gavl_dictionary_t *  gavl_dictionary_create()
  {
  gavl_dictionary_t * ret = malloc(sizeof(*ret));
  gavl_dictionary_init(ret);
  return ret;
  }
  

void gavl_dictionary_destroy(gavl_dictionary_t * d)
  {
  gavl_dictionary_free(d);
  free(d);
  }

void gavl_dictionary_set_audio_format(gavl_dictionary_t * d, const char * name,
                                      const gavl_audio_format_t * f)
  {
  gavl_audio_format_t * fmt;
  gavl_value_t val;
  gavl_value_init(&val);
  
  fmt = gavl_value_set_audio_format(&val);
  gavl_audio_format_copy(fmt, f);
  gavl_dictionary_set_nocopy(d, name, &val);
  }

void gavl_dictionary_set_video_format(gavl_dictionary_t * d, const char * name,
                                      const gavl_video_format_t * f)
  {
  gavl_video_format_t * fmt;
  gavl_value_t val;
  gavl_value_init(&val);

  fmt = gavl_value_set_video_format(&val);
  gavl_video_format_copy(fmt, f);
  gavl_dictionary_set_nocopy(d, name, &val);
  }

const gavl_audio_format_t * gavl_dictionary_get_audio_format(const gavl_dictionary_t * d, const char * name)
  {
  const gavl_value_t * v;
  if(!(v = gavl_dictionary_get(d, name)))
    return NULL;
  return gavl_value_get_audio_format(v);
  }

const gavl_video_format_t * gavl_dictionary_get_video_format(const gavl_dictionary_t * d, const char * name)
  {
  const gavl_value_t * v;
  if(!(v = gavl_dictionary_get(d, name)))
    return NULL;
  return gavl_value_get_video_format(v);
  }

gavl_audio_format_t * gavl_dictionary_get_audio_format_nc(gavl_dictionary_t * d, const char * name)
  {
  gavl_value_t * v;
  if(!(v = gavl_dictionary_get_nc(d, name)))
    return NULL;
  return gavl_value_get_audio_format_nc(v);
  }

gavl_video_format_t * gavl_dictionary_get_video_format_nc(gavl_dictionary_t * d, const char * name)
  {
  gavl_value_t * v;
  if(!(v = gavl_dictionary_get_nc(d, name)))
    return NULL;
  return gavl_value_get_video_format_nc(v);
  
  }
