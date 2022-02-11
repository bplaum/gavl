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

/* Array */

void gavl_array_init(gavl_array_t * d)
  {
  memset(d, 0, sizeof(*d));
  }

void gavl_array_dump(const gavl_array_t * a, int indent)
  {
  int i;
  gavl_diprintf(indent, "[\n");
  
  for(i = 0; i < a->num_entries; i++)
    {
    gavl_value_dump(&a->entries[i], indent + 2);
    
    if(i < a->num_entries - 1)
      gavl_diprintf(0, ",\n");
    else
      gavl_diprintf(0, "\n");
    }
  gavl_diprintf(indent, "]\n");
  }


const gavl_value_t * gavl_array_get(const gavl_array_t * d, int idx)
  {
  if((idx < 0) || (idx >= d->num_entries))
    return NULL;
  return d->entries + idx;
  }

gavl_value_t * gavl_array_get_nc(gavl_array_t * d, int idx)
  {
  if((idx < 0) || (idx >= d->num_entries))
    return NULL;
  return d->entries + idx;
  }

void gavl_array_free(gavl_array_t * d)
  {
  int i;
  if(d->entries)
    {
    for(i = 0; i < d->num_entries; i++)
      gavl_value_free(d->entries + i);
    free(d->entries);
    }
  }

void gavl_array_reset(gavl_array_t * d)
  {
  gavl_array_free(d);
  gavl_array_init(d);
  }

void gavl_array_copy_sub(gavl_array_t * dst, const gavl_array_t * src, int start, int num)
  {
  int i;
  
  if(start + num > src->num_entries)
    num = src->num_entries - start;

  
  gavl_array_reset(dst);

  if(num < 0)
    return;
  
  dst->entries_alloc = num;
  dst->num_entries   = num;
  
  if(dst->num_entries)
    {
    dst->entries = calloc(dst->entries_alloc, sizeof(*dst->entries));
  
    for(i = 0; i < num; i++)
      gavl_value_copy(dst->entries + i, src->entries + start + i);
    }
  }

void gavl_array_copy(gavl_array_t * dst, const gavl_array_t * src)
  {
  gavl_array_copy_sub(dst, src, 0, src->num_entries);
  }

void gavl_array_move(gavl_array_t * dst, gavl_array_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  gavl_array_init(src);
  }

void gavl_array_foreach(const gavl_array_t * a,
                        gavl_array_foreach_func func, void * priv)
  {
  int i;
  for(i = 0; i < a->num_entries; i++)
    func(priv, i, &a->entries[i]);
  }

int
gavl_array_compare(const gavl_array_t * m1,
                   const gavl_array_t * m2)
  {
  int i;
  int result = 1;
  
  if(m1->num_entries != m2->num_entries)
    return 1;
  
  for(i = 0; i < m1->num_entries; i++)
    {
    if((result = gavl_value_compare(&m1->entries[i], &m2->entries[i])))
      return result;
    }
  return 0;
  }

static gavl_value_t * do_splice(gavl_array_t * arr,
                                int idx, int del, int num)
  {
  int i;
  int after;
  
  if((idx < 0) || (idx > arr->num_entries)) // Append
    idx = arr->num_entries;
  
  if(del)
    {
    if(del < 0)
      del = arr->num_entries - idx;
    
    if(idx + del > arr->num_entries)
      del = arr->num_entries - idx;

    after = arr->num_entries - (idx + del);
    
    for(i = 0; i < del; i++)
      gavl_value_free(&arr->entries[idx + i]);

    arr->num_entries -= del;

    if(after)
      {
      memmove(arr->entries + idx,
              arr->entries + idx + del,
              sizeof(*arr->entries) * after);
      }
    }

  if(num)
    {
    if(arr->num_entries + num > arr->entries_alloc)
      {
      arr->entries_alloc = arr->num_entries + num + 128;
      arr->entries = realloc(arr->entries,
                             arr->entries_alloc * sizeof(*arr->entries));
      }

    if(idx < arr->num_entries)
      {
      memmove(arr->entries + idx + num,
              arr->entries + idx,
              (sizeof(*arr->entries) * (arr->num_entries - idx)));
      }
    arr->num_entries += num;

    memset(arr->entries + idx, 0, sizeof(*arr->entries) * num);
    return arr->entries + idx;
    }
  else
    return NULL;
  }

void gavl_array_splice_val(gavl_array_t * arr,
                           int idx, int del, const gavl_value_t * add)
  {
  gavl_value_t * val;
  int num = 0;

  if(add && (add->type == GAVL_TYPE_UNDEFINED))
    add = NULL;

  if(add)
    num = 1;
  
  val = do_splice(arr, idx, del, num);

  if(add)
    gavl_value_copy(val, add);
  }

void gavl_array_splice_array(gavl_array_t * arr,
                             int idx, int del, const gavl_array_t * add)
  {
  int i;
  gavl_value_t * val;
  int num = 0;
  
  if(add)
    num = add->num_entries;

  val = do_splice(arr, idx, del, num);

  if(add)
    {
    for(i = 0; i < num; i++)
      gavl_value_copy(val + i , add->entries + i);
    }
  }

void gavl_array_splice_val_nocopy(gavl_array_t * arr,
                                  int idx, int del, gavl_value_t * add)
  {
  gavl_value_t * val;
  int num = 0;

  if(add)
    num = 1;

  val = do_splice(arr, idx, del, num);

  if(add)
    {
    memcpy(val, add, sizeof(*add));
    gavl_value_init(add);
    }
  }

void gavl_array_splice_array_nocopy(gavl_array_t * arr,
                                    int idx, int del, gavl_array_t * add)
  {
  gavl_value_t * val;
  int num = 0;
  
  if(add)
    num = add->num_entries;

  val = do_splice(arr, idx, del, num);

  if(num)
    {
    memcpy(val, add->entries, sizeof(*add->entries) * num);
    memset(add->entries, 0, sizeof(*add->entries) * num);
    add->num_entries = 0;
    }
  }

void gavl_array_push(gavl_array_t * d, const gavl_value_t * val)
  {
  gavl_array_splice_val(d, -1, 0, val);
  }

void gavl_array_push_nocopy(gavl_array_t * d, gavl_value_t * val)
  {
  gavl_array_splice_val_nocopy(d, -1, 0, val);
  }

void gavl_array_unshift(gavl_array_t * d, const gavl_value_t * val)
  {
  gavl_array_splice_val(d, 0, 0, val);
  }

void gavl_array_unshift_nocopy(gavl_array_t * d, gavl_value_t * val)
  {
  gavl_array_splice_val_nocopy(d, 0, 0, val);
  }

int gavl_array_pop(gavl_array_t * d, gavl_value_t * val)
  {
  if(d->num_entries < 1)
    return 0;
  
  gavl_value_move(val, &d->entries[d->num_entries-1]);
  gavl_array_splice_val(d, d->num_entries-1, 1, NULL);
  return 1;
  }

int gavl_array_shift(gavl_array_t * d, gavl_value_t * val)
  {
  if(d->num_entries < 1)
    return 0;
  
  gavl_value_move(val, &d->entries[0]);
  gavl_array_splice_val(d, 0, 1, NULL);
  return 1;
  }

int gavl_array_move_entry(gavl_array_t * m1,
                          int src_pos, int dst_pos)
  {
  gavl_value_t save;
  if((src_pos < 0) || (src_pos >= m1->num_entries) ||
     (dst_pos < 0) || (dst_pos >= m1->num_entries))
    return 0;

  gavl_value_move(&save, m1->entries + src_pos);

  gavl_array_splice_val(m1, src_pos, 1, NULL);
  gavl_array_splice_val_nocopy(m1, dst_pos, 0, &save);
  
  return 1;
  }

gavl_array_t *  gavl_array_create()
  {
  gavl_array_t * ret = malloc(sizeof(*ret));
  gavl_array_init(ret);
  return ret;
  }

void gavl_array_destroy(gavl_array_t * arr)
  {
  gavl_array_free(arr);
  free(arr);
  }

void gavl_array_sort(gavl_array_t * arr, int (*compare)(const void *, const void *))
  {
  qsort(arr->entries, arr->num_entries, sizeof(*arr->entries), compare);
  }

/* String array */

void gavl_string_array_add(gavl_array_t * arr, const char * str)
  {
  gavl_value_t val;

  if(gavl_string_array_indexof(arr, str) >= 0)
    return;
  
  gavl_value_init(&val);
  gavl_value_set_string(&val, str);
  gavl_array_splice_val_nocopy(arr, -1, 0, &val);
  }

void gavl_string_array_insert_at(gavl_array_t * arr, int idx, const char * str)
  {
  gavl_value_t val;
  
  gavl_value_init(&val);
  gavl_value_set_string(&val, str);
  gavl_array_splice_val_nocopy(arr, idx, 0, &val);
  }

void gavl_string_array_add_nocopy(gavl_array_t * arr, char * str)
  {
  gavl_value_t val;

  if(gavl_string_array_indexof(arr, str) >= 0)
    return;
  
  gavl_value_init(&val);
  gavl_value_set_string_nocopy(&val, str);
  gavl_array_splice_val_nocopy(arr, -1, 0, &val);
  }


const char * gavl_string_array_get(const gavl_array_t * arr, int idx)
  {
  const gavl_value_t * val;

  if((val = gavl_array_get(arr, idx)))
    return gavl_value_get_string(val);
  return NULL;
  }

void gavl_string_array_delete(gavl_array_t * arr, const char * str)
  {
  int idx;

  if((idx = gavl_string_array_indexof(arr, str)) < 0)
    return;
  gavl_array_splice_val(arr, idx, 1, NULL);
  }

int gavl_string_array_indexof(const gavl_array_t * arr, const char * str)
  {
  int i;
  const char * str1;

  for(i = 0; i < arr->num_entries; i++)
    {
    if((str1 = gavl_value_get_string(&arr->entries[i])) &&
       !strcmp(str1, str))
      return i;
    }
  return -1;
  }

char * gavl_string_array_join(const gavl_array_t * arr, const char * glue)
  {
  char * ret;
  int ret_len;
  int glue_len;
  int i;
  int idx;
  
  glue_len = strlen(glue);
    
  ret_len = 1; // "\0"

  idx = 0;
    
  for(i = 0; i < arr->num_entries; i++)
    {
    if((arr->entries[i].type != GAVL_TYPE_STRING) || !arr->entries[i].v.str)
      continue;
      
    if(idx)
      ret_len += glue_len;
      
    ret_len += strlen(arr->entries[i].v.str);
    idx++;
    }

  ret = malloc(ret_len);
  *ret = '\0';

  if(!idx)
    return ret;
    
  idx = 0;
  for(i = 0; i < arr->num_entries; i++)
    {
    if((arr->entries[i].type != GAVL_TYPE_STRING) || !arr->entries[i].v.str)
      continue;
      
    if(idx)
      strncat(ret, glue, ret_len - strlen(ret));

    strncat(ret, arr->entries[i].v.str, ret_len - strlen(ret));
    idx++;
    }
  return ret;

  
  }
