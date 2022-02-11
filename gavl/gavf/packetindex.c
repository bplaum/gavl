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

int gavf_packet_index_read(gavf_io_t * io, gavf_packet_index_t * idx)
  {
  uint64_t i;

  if(!gavf_io_read_uint64v(io, &idx->num_entries))
    return 0;

  idx->entries = malloc(idx->num_entries * sizeof(*idx->entries));
  
  for(i = 0; i < idx->num_entries; i++)
    {
    if(!gavf_io_read_uint32v(io, &idx->entries[i].id) ||
       !gavf_io_read_uint32v(io, &idx->entries[i].flags) ||
       !gavf_io_read_uint64v(io, &idx->entries[i].pos) ||
       !gavf_io_read_int64v(io, &idx->entries[i].pts))
      return 0;
    }
  return 1;
  }

int gavf_packet_index_write(gavf_io_t * io, const gavf_packet_index_t * idx)
  {
  uint64_t i;
  if(gavf_io_write_data(io, (uint8_t*)GAVF_TAG_PACKET_INDEX, 8) < 8)
    return 0;

  if(!gavf_io_write_uint64v(io, idx->num_entries))
    return 0;

  for(i = 0; i < idx->num_entries; i++)
    {
    if(!gavf_io_write_uint32v(io, idx->entries[i].id) ||
       !gavf_io_write_uint32v(io, idx->entries[i].flags) ||
       !gavf_io_write_uint64v(io, idx->entries[i].pos) ||
       !gavf_io_write_int64v(io, idx->entries[i].pts))
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
