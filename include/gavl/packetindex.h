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



#ifndef GAVLPACKETINDEX_H_INCLUDED
#define GAVLPACKETINDEX_H_INCLUDED

typedef struct gavl_io_s gavl_io_t;


#define GAVL_PACKET_INDEX_INTERLEAVED (1<<0)
#define GAVL_PACKET_INDEX_SPARSE      (1<<1) // Not all packets are here, use only for seeking

typedef struct 
  {
  int num_entries;
  int entries_alloc;
  uint32_t flags;
  
  struct
    {
    uint64_t position;
    uint32_t size;
    int32_t stream_id;
    uint32_t flags;
    int64_t pts;  /* Time is scaled with the timescale of the stream */
    int64_t duration;  /* In timescale tics, can be 0 if unknown */
    } * entries;
  } gavl_packet_index_t;

GAVL_PUBLIC
gavl_packet_index_t * gavl_packet_index_create(int size);

GAVL_PUBLIC
void gavl_packet_index_destroy(gavl_packet_index_t *);
/* Delete entries */
GAVL_PUBLIC
void gavl_packet_index_clear(gavl_packet_index_t *);

GAVL_PUBLIC
void gavl_packet_index_add(gavl_packet_index_t * idx,
                           int64_t offset,
                           uint32_t size,
                           int stream_id,
                           int64_t timestamp,
                           int flags, int duration);

GAVL_PUBLIC
void gavl_packet_index_add_packet(gavl_packet_index_t * idx,
                                  const gavl_packet_t * p);

GAVL_PUBLIC
int gavl_packet_index_get_first(gavl_packet_index_t * idx,
                                 int stream);

GAVL_PUBLIC
int gavl_packet_index_get_last(gavl_packet_index_t * idx,
                               int stream);

GAVL_PUBLIC
void gavl_packet_index_set_size(gavl_packet_index_t * ret, int size);

GAVL_PUBLIC
void gavl_packet_index_dump(gavl_packet_index_t * idx);

GAVL_PUBLIC
void gavl_packet_index_set_stream_stats(gavl_packet_index_t * idx,
                                        int stream_id, gavl_stream_stats_t * stats);

GAVL_PUBLIC
int gavl_packet_index_read(gavl_packet_index_t * idx, gavl_io_t * io);

GAVL_PUBLIC
int gavl_packet_index_write(const gavl_packet_index_t * idx, gavl_io_t * io);

GAVL_PUBLIC
gavl_packet_index_t * gavl_packet_index_load(const char * filename);

GAVL_PUBLIC
int gavl_packet_index_save(gavl_packet_index_t * idx, const char * filename);

GAVL_PUBLIC
void gavl_packet_index_sort_by_position(gavl_packet_index_t * idx);

GAVL_PUBLIC
void gavl_packet_index_sort_by_pts(gavl_packet_index_t * idx);

GAVL_PUBLIC
void gavl_packet_index_extract_stream(const gavl_packet_index_t * src, gavl_packet_index_t * dst, int stream_id);

GAVL_PUBLIC
int gavl_packet_index_seek(const gavl_packet_index_t * idx, int stream_id, int64_t pts);

GAVL_PUBLIC
int gavl_packet_index_get_keyframe_before(const gavl_packet_index_t * idx, int stream_id, int pos);

GAVL_PUBLIC
int gavl_packet_index_get_next_keyframe(const gavl_packet_index_t * idx, int stream_id, int pos);

GAVL_PUBLIC
int gavl_packet_index_get_next_packet(const gavl_packet_index_t * idx, int stream_id, int pos);

GAVL_PUBLIC
int64_t gavl_packet_index_packet_number_to_pts(const gavl_packet_index_t * idx,
                                               int stream_id,
                                               int64_t number);


#endif // GAVLPACKETINDEX_H_INCLUDED
