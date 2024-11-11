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



#include <stdlib.h>
#include <string.h>

#include <config.h>
#include <gavl/gavl.h>
#include <gavl/packetindex.h>
#include <gavl/utils.h>
#include <gavl/io.h>

#include <gavl/gavf.h>

#define NUM_ALLOC 1024

#include <gavl/log.h>
#define LOG_DOMAIN "packetindex"


gavl_packet_index_t * gavl_packet_index_create(int size)
  {
  gavl_packet_index_t * ret;
  ret = calloc(1, sizeof(*ret));

  if(size)
    {
    ret->entries_alloc = size;
    ret->entries = calloc(ret->entries_alloc, sizeof(*(ret->entries)));
    }
  return ret;
  }

void gavl_packet_index_set_size(gavl_packet_index_t * ret, int size)
  {
  if(size > ret->entries_alloc)
    {
    ret->entries_alloc = size;
    ret->entries = realloc(ret->entries, ret->entries_alloc * sizeof(*(ret->entries)));
    memset(ret->entries + ret->num_entries, 0,
           sizeof(*ret->entries) * (ret->entries_alloc - ret->num_entries));
    }
  ret->num_entries = size;
  }


void gavl_packet_index_destroy(gavl_packet_index_t * idx)
  {
  if(idx->entries)
    free(idx->entries);
  free(idx);
  }

void gavl_packet_index_add(gavl_packet_index_t * idx,
                           int64_t position,
                           uint32_t size,
                           int stream_id,
                           int64_t timestamp,
                           int flags, int duration)
  {
  /* Realloc */
  
  if(idx->num_entries >= idx->entries_alloc)
    {
    idx->entries_alloc += NUM_ALLOC;
    idx->entries = realloc(idx->entries,
                           idx->entries_alloc * sizeof(*idx->entries));
    memset(idx->entries + idx->num_entries, 0,
           NUM_ALLOC * sizeof(*idx->entries));
    }
  /* Set fields */
  idx->entries[idx->num_entries].position    = position;
  idx->entries[idx->num_entries].size      = size;
  idx->entries[idx->num_entries].stream_id = stream_id;
  idx->entries[idx->num_entries].pts = timestamp;
  idx->entries[idx->num_entries].flags = flags;
  idx->entries[idx->num_entries].duration   = duration;
  
  idx->num_entries++;
  }

void gavl_packet_index_add_packet(gavl_packet_index_t * idx,
                                  const gavl_packet_t * p)
  {
  //  fprintf(stderr, "gavl_packet_index_add_packet\n");
  //  gavl_packet_dump(p);
  
  gavl_packet_index_add(idx, p->position, p->buf.len,
                        p->id, p->pts, p->flags, p->duration);
  }


void gavl_packet_index_dump(gavl_packet_index_t * idx)
  {
  int i;
  gavl_dprintf( "packet index %d entries:\n", idx->num_entries);
  for(i = 0; i < idx->num_entries; i++)
    {
    gavl_dprintf( "  No: %6d ID: %d K: %d O: %" PRId64 " T: %" PRId64 " D: %"PRId64" S: %6d PT: %s\n", 
                  i,
                  idx->entries[i].stream_id,
                  !!(idx->entries[i].flags & GAVL_PACKET_KEYFRAME),
                  idx->entries[i].position,
                  idx->entries[i].pts,
                  idx->entries[i].duration,
                  idx->entries[i].size,
                  gavl_coding_type_to_string(idx->entries[i].flags));
                  }
  }


void gavl_packet_index_clear(gavl_packet_index_t * si)
  {
  si->num_entries = 0;
  si->flags = 0;
  memset(si->entries, 0, sizeof(*si->entries) * si->entries_alloc);
  }

int gavl_packet_index_get_first(gavl_packet_index_t * idx,
                                 int stream)
  {
  int i = 0;
  while(i < idx->num_entries)
    {
    if(idx->entries[i].stream_id == stream)
      return i;
    i++;
    }
  return -1;
  }

int gavl_packet_index_get_last(gavl_packet_index_t * idx,
                               int stream)
  {
  int i = idx->num_entries-1;
  while(i >= 0)
    {
    if(idx->entries[i].stream_id == stream)
      return i;
    i--;
    }
  return -1;
  }

void gavl_packet_index_set_stream_stats(gavl_packet_index_t * idx,
                                        int stream_id, gavl_stream_stats_t * stats)
  {
  int i;
  gavl_stream_stats_init(stats);
  
  for(i = 0; i < idx->num_entries; i++)
    {
    if(idx->entries[i].stream_id != stream_id)
      continue;
    
    gavl_stream_stats_update_params(stats,
                                    idx->entries[i].pts,
                                    idx->entries[i].duration,
                                    idx->entries[i].size,
                                    idx->entries[i].flags & 0xFFFF);
    }
  }

/* Sort index by file position */
void gavl_packet_index_sort_by_position(gavl_packet_index_t * idx)
  {
  int i, n;
  int swapped;
  uint8_t swp[sizeof(*idx->entries)];
  
#define SWAP(j) \
  memcpy(swp, idx->entries + j, sizeof(*idx->entries)); \
  memcpy(idx->entries + j, idx->entries + (j+1), sizeof(*idx->entries)); \
  memcpy(idx->entries + (j+1), swp, sizeof(*idx->entries));

  n = idx->num_entries;

  do{
    swapped = 0;
  
    for(i = 0; i < n-1; i++)
     {
      if(idx->entries[i].position > idx->entries[i+1].position)
        {
        SWAP(i);
        swapped = 1;
        }
      }
    } while(swapped);
    
  
  }

void gavl_packet_index_sort_by_pts(gavl_packet_index_t * idx)
  {
  int i, n;
  int swapped;
  uint8_t swp[sizeof(idx->entries[0])];
  
#define SWAP(j) \
  memcpy(swp, idx->entries + j, sizeof(*idx->entries)); \
  memcpy(idx->entries + j, idx->entries + (j+1), sizeof(*idx->entries)); \
  memcpy(idx->entries + (j+1), swp, sizeof(*idx->entries));

  n = idx->num_entries;

  do{
    swapped = 0;
  
    for(i = 0; i < n-1; i++)
      {
      if(idx->entries[i].pts > idx->entries[i+1].pts)
        {
        SWAP(i);
        swapped = 1;
        }
      }
    n--;
    } while(swapped);
  }

void gavl_packet_index_extract_stream(const gavl_packet_index_t * src, gavl_packet_index_t * dst, int stream_id)
  {
  int i, idx;
  /* 1. Count entries */

  idx = 0;
  for(i = 0; i < src->num_entries; i++)
    {
    if(src->entries[i].stream_id == stream_id)
      idx++;
    }
  
  /* 2. Alloc destination */
  gavl_packet_index_set_size(dst, idx);
  
  /* 3. Copy entries */
  idx = 0;
  for(i = 0; i < src->num_entries; i++)
    {
    if(src->entries[i].stream_id == stream_id)
      {
      memcpy(&dst->entries[idx], &src->entries[i], sizeof(src->entries[i]));
      idx++;
      }
    }
  }

int gavl_packet_index_seek(const gavl_packet_index_t * idx, int stream_id, int64_t pts)
  {
  int i = idx->num_entries - 1;

  while(i >= 0)
    {
    if((idx->entries[i].stream_id == stream_id) &&
       (idx->entries[i].pts <= pts))
      break;
    i--;
    }
  if(i < 0)
    return -1;
  
  while((i > 0) && (idx->entries[i-1].position == idx->entries[i].position))
    i--;
  
  return i;
  }

int gavl_packet_index_get_keyframe_before(const gavl_packet_index_t * idx, int stream_id, int pos)
  {
  while(pos >= 0)
    {
    if((idx->entries[pos].stream_id == stream_id) &&
       (idx->entries[pos].flags & GAVL_PACKET_KEYFRAME))
      return pos;
    pos--;
    }
  return -1;
  }  

int gavl_packet_index_get_next_keyframe(const gavl_packet_index_t * idx, int stream_id, int pos)
  {
  while(pos < idx->num_entries)
    {
    if((idx->entries[pos].stream_id == stream_id) &&
       (idx->entries[pos].flags & GAVL_PACKET_KEYFRAME))
      return pos;
    pos++;
    }
  return -1;
  }

int gavl_packet_index_get_next_packet(const gavl_packet_index_t * idx, int stream_id, int pos)
  {
  while(pos < idx->num_entries)
    {
    if(idx->entries[pos].stream_id == stream_id)
      return pos;
    pos++;
    }
  return -1;
  }

int gavl_packet_index_get_previous_packet(const gavl_packet_index_t * idx, int stream_id, int pos)
  {
  if(pos < 0)
    pos = idx->num_entries-1;
  
  while(pos > idx->num_entries)
    {
    if(idx->entries[pos].stream_id == stream_id)
      return pos;
    pos--;
    }
  return -1;
  }

/* Serialisation */

int gavl_packet_index_read(gavl_packet_index_t * idx, gavl_io_t * io)
  {
  int i;
  uint32_t num;

  if(!gavl_io_read_uint32v(io, &idx->flags) ||
     !gavl_io_read_uint32v(io, &num))
    return 0;

  gavl_packet_index_set_size(idx, num);
  
  for(i = 0; i < idx->num_entries; i++)
    {
    if(!gavl_io_read_int32v(io, &idx->entries[i].stream_id) ||
       !gavl_io_read_uint32v(io, &idx->entries[i].flags) ||
       !gavl_io_read_uint32v(io, &idx->entries[i].size) ||
       !gavl_io_read_uint64v(io, &idx->entries[i].position) ||
       !gavl_io_read_int64v(io, &idx->entries[i].pts) ||
       !gavl_io_read_int64v(io, &idx->entries[i].duration))       
      return 0;
    }
  return 1;
  
  }

int gavl_packet_index_write(const gavl_packet_index_t * idx, gavl_io_t * io)
  {
  int i;
  if(!gavl_io_write_uint32v(io, idx->flags) ||
     !gavl_io_write_uint32v(io, idx->num_entries))
    return 0;

  for(i = 0; i < idx->num_entries; i++)
    {
    if(!gavl_io_write_int32v(io, idx->entries[i].stream_id) ||
       !gavl_io_write_uint32v(io, idx->entries[i].flags) ||
       !gavl_io_write_uint32v(io, idx->entries[i].size) ||
       !gavl_io_write_uint64v(io, idx->entries[i].position) ||
       !gavl_io_write_int64v(io, idx->entries[i].pts) ||
       !gavl_io_write_int64v(io, idx->entries[i].duration))       
      return 0;
    }
  return 1;
  }

gavl_packet_index_t * gavl_packet_index_load(const char * filename)
  {
  gavl_chunk_t head;
  gavl_packet_index_t * idx;
  
  gavl_io_t * io;

  if(!(io = gavl_io_from_filename(filename, 0)))
    return 0;

  if(!gavl_chunk_read_header(io, &head) ||
     !gavl_chunk_is(&head, GAVF_TAG_PACKET_INDEX))
    return 0;

  idx = gavl_packet_index_create(0);
  if(!gavl_packet_index_read(idx, io))
    {
    gavl_packet_index_destroy(idx);
    idx = NULL;
    }
  
  gavl_io_destroy(io);
  return idx;
  }

int gavl_packet_index_save(gavl_packet_index_t * idx, const char * filename)
  {
  gavl_chunk_t head;

  gavl_io_t * io;

  if(!(io = gavl_io_from_filename(filename, 1)))
    return 0;
  
  gavl_chunk_start(io, &head, GAVF_TAG_PACKET_INDEX);
  gavl_packet_index_write(idx, io);
  gavl_chunk_finish(io, &head, 1);
  gavl_io_destroy(io);
  return 1;
  }

int64_t gavl_packet_index_packet_number_to_pts(const gavl_packet_index_t * idx,
                                               int stream_id,
                                               int64_t number)
  {
  int64_t i;
  int64_t idx_pos = 0;
  
  if(idx < 0)
    return GAVL_TIME_UNDEFINED;

  idx_pos = gavl_packet_index_get_next_packet(idx, stream_id, 0);

  if(idx_pos < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "No packets for stream %d in index", stream_id);
    return GAVL_TIME_UNDEFINED;
    }
  for(i = 0; i < number; i++)
    {
    if((idx_pos = gavl_packet_index_get_next_packet(idx, stream_id, idx_pos+1)) < 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "No packet %"PRId64" in stream", number);
      return GAVL_TIME_UNDEFINED;
      }
    }
  return idx->entries[idx_pos].pts;
  }

