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

#include <stdlib.h>
#include <string.h>

#include <gavl/gavl.h>
#include <gavl/gavf.h>
#include <gavl/compression.h>

void gavl_seek_index_append_pos_pts(gavl_seek_index_t * idx,
                                    int64_t position, int64_t pts)
  {
  /* If it is not the first packet starting at this position,
     skip this also */

  if(idx->num_entries &&
     (idx->entries[idx->num_entries-1].position == position))
    return;
  
  if(idx->num_entries == idx->entries_alloc)
    {
    idx->entries_alloc += 1024;
    idx->entries = realloc(idx->entries, idx->entries_alloc * sizeof(*idx->entries));
    memset(idx->entries + idx->num_entries, 0,
           (idx->entries_alloc - idx->num_entries) * sizeof(*idx->entries));
    }
  
  idx->entries[idx->num_entries].pts      = pts;
  idx->entries[idx->num_entries].position = position;
  idx->num_entries++;
  }


void gavl_seek_index_append_packet(gavl_seek_index_t * idx,
                                   const gavl_packet_t * pkt, int compression_flags)
  {
  /* No position */
  if(pkt->position < 0)
    return;

  /* Skip non P-frames */
  if((compression_flags & GAVL_COMPRESSION_HAS_P_FRAMES) && 
     !(pkt->flags & GAVL_PACKET_KEYFRAME))
    return;

  gavl_seek_index_append_pos_pts(idx,pkt->position, pkt->pts);
  }

int gavl_seek_index_seek(const gavl_seek_index_t * idx,
                         int64_t pts)
  {
  int i = idx->num_entries-1;

  while(i >= 0)
    {
    if(idx->entries[i].pts <= pts)
      return i;
    i--;
    }
  return 0;
  }

void gavl_seek_index_free(gavl_seek_index_t * idx)
  {
  if(idx->entries)
    free(idx->entries);
  }

/* Serialize */

void gavl_seek_index_write(const gavl_seek_index_t * idx, gavl_io_t * io)
  {
  int i;
  
  gavl_io_write_int32v(io, idx->num_entries);

  for(i = 0; i < idx->num_entries; i++)
    {
    gavl_io_write_int64v(io, idx->entries[i].position);
    gavl_io_write_int64v(io, idx->entries[i].pts);
    }
  }

void gavl_seek_index_to_buffer(const gavl_seek_index_t * idx, gavl_buffer_t * buf)
  {
  gavl_io_t * io = gavl_io_create_buffer_write(buf);
  gavl_seek_index_write(idx, io);
  gavl_io_destroy(io);
  }

int gavl_seek_index_read(gavl_seek_index_t * idx, gavl_io_t * io)
  {
  int i;
  
  if(!gavl_io_read_int32v(io, &idx->num_entries))
    return 0;

  idx->entries_alloc = idx->num_entries;
  idx->entries = calloc(idx->entries_alloc, sizeof(*idx->entries));
  
  for(i = 0; i < idx->num_entries; i++)
    {
    if(!gavl_io_read_int64v(io, &idx->entries[i].position) ||
       !gavl_io_read_int64v(io, &idx->entries[i].pts))
      return 0;
    }
  return 1;
  }

int gavl_seek_index_from_buffer(gavl_seek_index_t * idx, const gavl_buffer_t * buf)
  {
  int ret = 0;
  gavl_io_t * io = gavl_io_create_buffer_read(buf);

  if(!gavl_seek_index_read(idx, io))
    goto fail;
  
  ret = 1;
  
  fail:
  gavl_io_destroy(io);
  return ret;
  }

void gavl_seek_index_dump(gavl_seek_index_t * idx)
  {
  int i;
  gavl_dprintf("Seek index %d entries\n", idx->num_entries);

  for(i = 0; i < idx->num_entries; i++)
    {
    gavl_dprintf("  %"PRId64" %"PRId64" %"PRId64" %"PRId64"\n", idx->entries[i].position, idx->entries[i].pts,
                 (i < idx->num_entries - 1) ? idx->entries[i+1].pts - idx->entries[i].pts : 0,
                 (i < idx->num_entries - 1) ? idx->entries[i+1].position - idx->entries[i].position : 0);
    }
  
  }
