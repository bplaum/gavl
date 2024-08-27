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


#ifndef GAVFPRIVATE_H_INCLUDED
#define GAVFPRIVATE_H_INCLUDED


#include <gavl/gavf.h>

/* Dictionary added to the track or stream dictionaries */

/* Stream */

#define STREAM_FLAG_DISCONTINUOUS (1<<3)
#define STREAM_FLAG_SKIP          (1<<4)
#define STREAM_FLAG_READY         (1<<5)

typedef struct
  {
  gavl_dictionary_t * h;
  gavl_dictionary_t * m;
  
  /* Secondary variables */
  int flags;

  int packet_flags;
    
  int timescale;
  int packet_duration;
  
  // PTS Offset (to make all PTSes start near zero)
  //  int64_t pts_offset;

  //  gavf_packet_buffer_t * pb;
  
  gavl_packet_source_t * psrc;
  gavl_packet_sink_t * psink;

  gavl_audio_source_t * asrc;
  gavl_audio_sink_t * asink;
  
  gavl_video_source_t * vsrc;
  gavl_video_sink_t * vsink;

  gavl_hw_context_t * hwctx; // For importing frames sent off-band
  gavl_io_t * io; // Streams can have separate IO
  int server_fd;
  
  gavf_t * g;

  /* For reading uncompressed audio and video */
  gavl_audio_frame_t * aframe;
  gavl_video_frame_t * vframe;
  
  gavl_packet_t * p; // For sink

  gavl_dsp_context_t * dsp; // For swapping endianess
  
  gavf_packet_unref_func unref_func;
  void * unref_priv;

  gavl_stream_stats_t stats;
  
  gavl_audio_format_t * afmt;
  gavl_video_format_t * vfmt;
  gavl_compression_info_t ci;
  
  int id;
  gavl_stream_type_t type;
  
  } gavf_stream_t;

void gavf_stream_create_packet_src(gavf_t * g, gavf_stream_t * s);

void gavf_stream_create_packet_sink(gavf_t * g, gavf_stream_t * s);

gavl_sink_status_t gavf_flush_packets(gavf_t * g, gavf_stream_t * s);

gavf_stream_t * gavf_find_stream_by_id(gavf_t * g, int32_t id);

// int gavf_stream_get_timescale(const gavf_stream_header_t * sh);

/* Packet */

// #define GAVF_PACKET_WRITE_INTERLACE (1<<0)
// #define GAVF_PACKET_WRITE_DURATION  (1<<1)
// #define GAVF_PACKET_WRITE_FIELD2    (1<<2)

int gavf_write_gavl_packet_header(gavl_io_t * io,
                                  int default_duration,
                                  int packet_flags,
                                  const gavl_packet_t * p);

/* Options */

struct gavf_options_s
  {
  uint32_t flags;
  };

/* Extension header */

typedef struct
  {
  uint32_t key;
  uint32_t len;
  } gavl_extension_header_t;

int gavf_extension_header_read(gavl_io_t * io, gavl_extension_header_t * eh);
int gavf_extension_write(gavl_io_t * io, uint32_t key, uint32_t len,
                         uint8_t * data);

/* Known extensions */

/* Audio format */

#define GAVF_EXT_AF_SAMPLESPERFRAME 0
#define GAVF_EXT_AF_SAMPLEFORMAT    1
#define GAVF_EXT_AF_INTERLEAVE      2
#define GAVF_EXT_AF_CENTER_LEVEL    3
#define GAVF_EXT_AF_REAR_LEVEL      4

/* Video format */

#define GAVF_EXT_VF_PIXELFORMAT     0
#define GAVF_EXT_VF_PIXEL_ASPECT    1
#define GAVF_EXT_VF_INTERLACE       2
#define GAVF_EXT_VF_FRAME_SIZE      3
#define GAVF_EXT_VF_TC_RATE         4
#define GAVF_EXT_VF_TC_FLAGS        5

/* Compresson info */

#define GAVF_EXT_CI_GLOBAL_HEADER     0
#define GAVF_EXT_CI_BITRATE           1
#define GAVF_EXT_CI_PRE_SKIP          2
#define GAVF_EXT_CI_MAX_PACKET_SIZE   3
#define GAVF_EXT_CI_VIDEO_BUFFER_SIZE 4
#define GAVF_EXT_CI_MAX_REF_FRAMES    5

/* Packet */

#define GAVF_EXT_PK_DURATION         1
#define GAVF_EXT_PK_HEADER_SIZE      2
#define GAVF_EXT_PK_SEQ_END          3
#define GAVF_EXT_PK_TIMECODE         4
#define GAVF_EXT_PK_SRC_RECT         5
#define GAVF_EXT_PK_DST_COORDS       6
#define GAVF_EXT_PK_BUF_IDX          7
#define GAVF_EXT_PK_INTERLACE        8
#define GAVF_EXT_PK_FIELD2           9
#define GAVF_EXT_PK_FDS              10

/* File index */

#define GAVF_TAG_PACKET_HEADER    "P"
#define GAVF_TAG_PACKET_HEADER_C  'P'

typedef struct
  {
  uint64_t num_entries;
  uint64_t entries_alloc;
  
  struct
    {
    uint32_t id;
    uint32_t flags;    // Same as the packet flags
    
    uint64_t pos;
    int64_t pts;
    } * entries;
  } gavf_packet_index_t;

void gavf_packet_index_add(gavf_packet_index_t * idx,
                           uint32_t id, uint32_t flags, uint64_t pos,
                           int64_t pts);

int gavf_packet_index_read(gavl_io_t * io, gavf_packet_index_t * idx);
int gavf_packet_index_write(gavl_io_t * io, const gavf_packet_index_t * idx);
void gavf_packet_index_free(gavf_packet_index_t * idx);
void gavf_packet_index_dump(gavf_packet_index_t * idx);


/* Global gavf structure */

typedef enum
  {
    ENC_STARTING = 0,
    ENC_SYNCHRONOUS,
    ENC_INTERLEAVE
  } encoding_mode_t;



#define GAVF_SET_FLAG(g, f)    g->flags |= f
#define GAVF_CLEAR_FLAG(g, f) (g->flags &= ~f)
#define GAVF_HAS_FLAG(g, f)   (g->flags & f)


struct gavf_s
  {
  int flags;
  
  gavl_io_t * io_orig;
  gavl_io_t * io;
  
  gavl_dictionary_t cur;
  
  gavl_dictionary_t mi;
  
  gavf_packet_index_t   pi;

  gavf_stream_t * streams;
  int num_streams;
  
  gavl_handle_msg_func msg_callback;
  void * msg_data;

  gavl_chunk_t packets_chunk;
  //  gavf_chunk_t sync_chunk;
  
  gavf_options_t opt;
  
  gavl_io_t * pkt_io;
  
  //  uint64_t first_sync_pos;
  
  //  gavl_packet_t write_pkt;
  //  gavl_packet_t skip_pkt;
  
  gavl_video_frame_t * write_vframe;
  gavl_audio_frame_t * write_aframe;

  //  encoding_mode_t encoding_mode;
  //  encoding_mode_t final_encoding_mode;
  
  };

/* Footer */

int gavf_footer_check(gavf_t * g);
int gavf_footer_write(gavf_t * g);
void gavf_footer_init(gavf_t * g);

int gavf_program_header_write(gavf_t * g);

/* Packet */

int gavf_read_gavl_packet(gavl_io_t * io,
                          gavl_packet_t * p);

int gavf_skip_gavl_packet(gavl_io_t * io,
                          gavl_packet_t * p);

int gavf_read_gavl_packet_header(gavl_io_t * io,
                                 gavl_packet_t * p);

int gavf_write_gavl_packet(gavl_io_t * io,
                           const gavf_stream_t * s,
                           const gavl_packet_t * p);


#endif // GAVFPRIVATE_H_INCLUDED
