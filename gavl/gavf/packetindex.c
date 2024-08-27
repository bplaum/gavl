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

#include <gavfprivate.h>

void gavf_packet_index_add(gavf_packet_index_t * idx,
                           uint32_t id, uint32_t flags, uint64_t pos,
                           int64_t pts)
  {
  if(idx->num_entries >= idx->entries_alloc)
    {
    idx->entries_alloc += 1024;
    idx->entries = realloc(idx->entries,
                           idx->entries_alloc * sizeof(*idx->entries));
    }
  idx->entries[idx->num_entries].id    = id;
  idx->entries[idx->num_entries].flags = flags;
  idx->entries[idx->num_entries].pos   = pos;
  idx->entries[idx->num_entries].pts   = pts;
  idx->num_entries++;
  }

int gavf_packet_index_read(gavl_io_t * io, gavf_packet_index_t * idx)
  {
  uint64_t i;

  if(!gavl_io_read_uint64v(io, &idx->num_entries))
    return 0;

  idx->entries = malloc(idx->num_entries * sizeof(*idx->entries));
  
  for(i = 0; i < idx->num_entries; i++)
    {
    if(!gavl_io_read_uint32v(io, &idx->entries[i].id) ||
       !gavl_io_read_uint32v(io, &idx->entries[i].flags) ||
       !gavl_io_read_uint64v(io, &idx->entries[i].pos) ||
       !gavl_io_read_int64v(io, &idx->entries[i].pts))
      return 0;
    }
  return 1;
  }

int gavf_packet_index_write(gavl_io_t * io, const gavf_packet_index_t * idx)
  {
  uint64_t i;
  if(gavl_io_write_data(io, (uint8_t*)GAVF_TAG_PACKET_INDEX, 8) < 8)
    return 0;

  if(!gavl_io_write_uint64v(io, idx->num_entries))
    return 0;

  for(i = 0; i < idx->num_entries; i++)
    {
    if(!gavl_io_write_uint32v(io, idx->entries[i].id) ||
       !gavl_io_write_uint32v(io, idx->entries[i].flags) ||
       !gavl_io_write_uint64v(io, idx->entries[i].pos) ||
       !gavl_io_write_int64v(io, idx->entries[i].pts))
      return 0;
    }
  return 1;
  }

void gavf_packet_index_free(gavf_packet_index_t * idx)
  {
  if(idx->entries)
    free(idx->entries);
  }

void gavf_packet_index_dump(gavf_packet_index_t * idx)
  {
  uint64_t i;
  fprintf(stderr, "Packet index (%"PRId64" Entries)\n", idx->num_entries);
  for(i = 0; i < idx->num_entries; i++)
    {
    fprintf(stderr, "  id: %02d flags: %08x pos: %"PRId64" pts: %"PRId64"\n",
            idx->entries[i].id, idx->entries[i].flags,
            idx->entries[i].pos, idx->entries[i].pts);
    }
  }
