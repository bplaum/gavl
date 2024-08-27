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

#include <gavfprivate.h>

void gavf_sync_index_init(gavf_sync_index_t * idx, int num_streams)
  {
  idx->num_streams = num_streams;
  idx->pts_len =
    idx->num_streams * sizeof(int64_t);
  }

void gavf_sync_index_add(gavf_t * g, uint64_t pos)
  {
  int i;
  gavf_sync_index_t * idx = &g->si;
  
  if(idx->num_entries >= idx->entries_alloc)
    {
    idx->entries_alloc += 1024;
    idx->entries = realloc(idx->entries,
                           idx->entries_alloc * sizeof(*idx->entries));
    }

  idx->entries[idx->num_entries].pos = pos;
  idx->entries[idx->num_entries].pts = malloc(idx->pts_len);
  
  for(i = 0; i < g->num_streams; i++)
    idx->entries[idx->num_entries].pts[i] = g->streams[i].sync_pts;
  
  idx->num_entries++;
  }

int gavf_sync_index_read(gavl_io_t * io, gavf_sync_index_t * idx)
  {
  uint64_t i;
  int j;
  
  if(!gavl_io_read_uint64v(io, &idx->num_entries))
    return 0;
  idx->entries = malloc(idx->num_entries * sizeof(*idx->entries));

  for(i = 0; i < idx->num_entries; i++)
    {
    if(!gavl_io_read_uint64v(io, &idx->entries[i].pos))
      return 0;
    
    idx->entries[i].pts = malloc(idx->pts_len);
    for(j = 0; j < idx->num_streams; j++)
      {
      if(!gavl_io_read_int64v(io, &idx->entries[i].pts[j]))
        return 0;
      }
    }
  return 1;
  }

int gavf_sync_index_write(gavl_io_t * io, const gavf_sync_index_t * idx)
  {
  uint64_t i;
  int j;

  if(gavl_io_write_data(io, (uint8_t*)GAVF_TAG_SYNC_INDEX, 8) < 8)
    return 0;
  
  if(!gavl_io_write_uint64v(io, idx->num_entries))
    return 0;

  for(i = 0; i < idx->num_entries; i++)
    {
    if(!gavl_io_write_uint64v(io, idx->entries[i].pos))
      return 0;
    for(j = 0; j < idx->num_streams; j++)
      {
      if(!gavl_io_write_int64v(io, idx->entries[i].pts[j]))
        return 0;
      }
    }
  return 1;
  }

void gavf_sync_index_free(gavf_sync_index_t * idx)
  {
  int i;
  for(i = 0; i < idx->num_entries; i++)
    {
    if(idx->entries[i].pts)
      free(idx->entries[i].pts);
    }
          

  if(idx->entries)
    free(idx->entries);
  }

void gavf_sync_index_dump(const gavf_sync_index_t * idx)
  {
  uint64_t i;
  int j;
  
  fprintf(stderr, "Sync index (%"PRId64" entries)\n", idx->num_entries);

  for(i = 0; i < idx->num_entries; i++)
    {
    fprintf(stderr, "  Pos: %"PRId64"\n", idx->entries[i].pos);
    for(j = 0; j < idx->num_streams; j++)
      {
      fprintf(stderr, "    PTS %02d: %"PRId64"\n", j, idx->entries[i].pts[j]);
      }
    }
  
  }
