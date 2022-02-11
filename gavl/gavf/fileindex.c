#include <string.h>
#include <stdlib.h>

#include <gavfprivate.h>

void gavf_file_index_init(gavf_file_index_t * fi, int num_entries)
  {
  memset(fi, 0, sizeof(*fi));
  fi->entries_alloc = num_entries;
  fi->entries = calloc(fi->entries_alloc, sizeof(*fi->entries));
  }

int gavf_file_index_read(gavf_io_t * io, gavf_file_index_t * fi)
  {
  int i;
  if(!gavf_io_read_uint32v(io, &fi->entries_alloc))
    return 0;
  fi->entries = calloc(fi->entries_alloc, sizeof(*fi->entries));

  for(i = 0; i < fi->entries_alloc; i++)
    {
    if((gavf_io_read_data(io, fi->entries[i].tag, 8) < 8) ||
       !gavf_io_read_uint64f(io, &fi->entries[i].position))
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

int gavf_file_index_write(gavf_io_t * io, const gavf_file_index_t * fi)
  {
  int i;

  if(gavf_io_write_data(io, (uint8_t*)GAVF_TAG_FILE_INDEX, 8) < 8)
    return 0;

  if(!gavf_io_write_uint32v(io, fi->entries_alloc))
    return 0;

  for(i = 0; i < fi->entries_alloc; i++)
    {
    if((gavf_io_write_data(io, fi->entries[i].tag, 8) < 8) ||
       !gavf_io_write_uint64f(io, fi->entries[i].position))
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
