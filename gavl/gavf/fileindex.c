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


#include <string.h>
#include <stdlib.h>

#include <gavfprivate.h>

void gavf_file_index_init(gavf_file_index_t * fi, int num_entries)
  {
  memset(fi, 0, sizeof(*fi));
  fi->entries_alloc = num_entries;
  fi->entries = calloc(fi->entries_alloc, sizeof(*fi->entries));
  }

int gavf_file_index_read(gavl_io_t * io, gavf_file_index_t * fi)
  {
  int i;
  if(!gavl_io_read_uint32v(io, &fi->entries_alloc))
    return 0;
  fi->entries = calloc(fi->entries_alloc, sizeof(*fi->entries));

  for(i = 0; i < fi->entries_alloc; i++)
    {
    if((gavl_io_read_data(io, fi->entries[i].tag, 8) < 8) ||
       !gavl_io_read_uint64f(io, &fi->entries[i].position))
      return 0;
    }

  fi->num_entries = fi->entries_alloc;
  for(i = 0; i < fi->entries_alloc; i++)
    {
    if(fi->entries[i].position == 0)
      {
      fi->num_entries = i;
      break;
      }
    }
  return 1;
  }

int gavf_file_index_write(gavl_io_t * io, const gavf_file_index_t * fi)
  {
  int i;

  if(gavl_io_write_data(io, (uint8_t*)GAVF_TAG_FILE_INDEX, 8) < 8)
    return 0;

  if(!gavl_io_write_uint32v(io, fi->entries_alloc))
    return 0;

  for(i = 0; i < fi->entries_alloc; i++)
    {
    if((gavl_io_write_data(io, fi->entries[i].tag, 8) < 8) ||
       !gavl_io_write_uint64f(io, fi->entries[i].position))
      return 0;
    }
  return 1;
  }

void gavf_file_index_free(gavf_file_index_t * fi)
  {
  if(fi->entries)
    free(fi->entries);
  }

void gavf_file_index_add(gavf_file_index_t * fi, char * tag, int64_t position)
  {
  if(fi->entries_alloc <= fi->num_entries)
    return; // BUG!!!
  memcpy(fi->entries[fi->num_entries].tag, tag, 8);
  fi->entries[fi->num_entries].position = position;
  fi->num_entries++;
  }

void gavf_file_index_dump(const gavf_file_index_t * fi)
  {
  int i;
  fprintf(stderr, "File index (%d entries):\n", fi->num_entries);
  
  for(i = 0; i < fi->num_entries; i++)
    {
    fprintf(stderr, "  Tag: ");
    fwrite(fi->entries[i].tag, 1, 8, stderr);
    fprintf(stderr, "  Pos: %"PRId64"\n", fi->entries[i].position);
    }
  }
