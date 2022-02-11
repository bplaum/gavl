
#include <gavl/gavf.h>


/* I/O */

struct gavl_io_s
  {
  gavf_read_func read_func;
  gavf_read_func read_func_nonblock;
  
  gavf_write_func write_func;
  gavf_seek_func seek_func;
  gavf_close_func close_func;
  gavf_flush_func flush_func;
  gavf_poll_func poll_func;
  
  void * priv;
  
  int64_t position;
  int got_error;    // Got write error
  
  /* Informational data */
  char * filename;
  char * mimetype;
  int64_t total_bytes;

  gavl_handle_msg_func msg_callback;
  void * msg_data;
  
  gavl_buffer_t get_buf;
  };

void gavf_io_init(gavf_io_t * ret,
                  gavf_read_func  r,
                  gavf_write_func w,
                  gavf_seek_func  s,
                  gavf_close_func c,
                  gavf_flush_func f,
                  void * priv);



void gavf_io_skip(gavf_io_t * io, int bytes);

void gavf_io_init_buf_read(gavf_io_t * io, gavl_buffer_t * buf);
void gavf_io_init_buf_write(gavf_io_t * io, gavl_buffer_t * buf);

void gavf_io_set_nonblock_read(gavf_io_t * io, gavf_read_func read_nonblock);

/* Packetbuffer */

typedef struct gavf_packet_buffer_s gavf_packet_buffer_t;

gavf_packet_buffer_t * gavf_packet_buffer_create(int timescale);


void gavf_packet_buffer_set_unref_func(gavf_packet_buffer_t * b,
                                       gavf_packet_unref_func unref_func,
                                       void *                 unref_data);

gavl_packet_t * gavf_packet_buffer_get_write(gavf_packet_buffer_t *);
void gavf_packet_buffer_done_write(gavf_packet_buffer_t * b);

gavl_packet_t * gavf_packet_buffer_get_read(gavf_packet_buffer_t *);
gavl_packet_t * gavf_packet_buffer_peek_read(gavf_packet_buffer_t * b);


gavl_time_t gavf_packet_buffer_get_min_pts(gavf_packet_buffer_t * b);

void gavf_packet_buffer_destroy(gavf_packet_buffer_t *);

const gavl_packet_t * gavf_packet_buffer_get_last(gavf_packet_buffer_t * b);
void gavf_packet_buffer_remove_last(gavf_packet_buffer_t * b);

void gavf_packet_buffer_clear(gavf_packet_buffer_t * b);

/* Stream */

#define STREAM_FLAG_DISCONTINUOUS (1<<3)
#define STREAM_FLAG_SKIP          (1<<4)

typedef struct
  {
  gavl_dictionary_t * h;
  gavl_dictionary_t * m;
  
  /* Secondary variables */
  int flags;

  int packet_flags;
    
  int timescale;
  int packet_duration;

  int64_t last_sync_pts; // PTS of the last snyc header

  // PTS of the next sync header (for streams without B-frames)
  int64_t next_sync_pts; 

  // Next PTS (for streams with implicit PTS)
  int64_t next_pts;

  // PTS Offset (to make all PTSes start near zero)
  int64_t pts_offset;

  int64_t sync_pts;
  
  int packets_since_sync;
  
  gavf_packet_buffer_t * pb;
  
  gavl_packet_source_t * psrc;
  gavl_packet_sink_t * psink;

  gavl_audio_source_t * asrc;
  gavl_audio_sink_t * asink;
  
  gavl_video_source_t * vsrc;
  gavl_video_sink_t * vsink;

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

#define GAVF_PACKET_WRITE_PTS       (1<<0)
#define GAVF_PACKET_WRITE_INTERLACE (1<<1)
#define GAVF_PACKET_WRITE_DURATION  (1<<2)
#define GAVF_PACKET_WRITE_FIELD2    (1<<3)

int gavf_write_gavl_packet_header(gavf_io_t * io,
                                  int default_duration,
                                  int packet_flags,
                                  int64_t last_sync_pts,
                                  const gavl_packet_t * p);

/* Options */

struct gavf_options_s
  {
  uint32_t flags;
  gavl_time_t sync_distance;
  };

/* Extension header */

typedef struct
  {
  uint32_t key;
  uint32_t len;
  } gavl_extension_header_t;

int gavf_extension_header_read(gavf_io_t * io, gavl_extension_header_t * eh);
int gavf_extension_write(gavf_io_t * io, uint32_t key, uint32_t len,
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

#define GAVF_EXT_PK_DURATION         0
#define GAVF_EXT_PK_HEADER_SIZE      1
#define GAVF_EXT_PK_SEQ_END          2
#define GAVF_EXT_PK_TIMECODE         3
#define GAVF_EXT_PK_SRC_RECT         4
#define GAVF_EXT_PK_DST_COORDS       5

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

int gavf_packet_index_read(gavf_io_t * io, gavf_packet_index_t * idx);
int gavf_packet_index_write(gavf_io_t * io, const gavf_packet_index_t * idx);
void gavf_packet_index_free(gavf_packet_index_t * idx);
void gavf_packet_index_dump(gavf_packet_index_t * idx);

void gavf_io_cleanup(gavf_io_t * io);

typedef struct
  {
  uint64_t num_entries;
  uint64_t entries_alloc;

  struct
    {
    uint64_t pos;
    int64_t * pts;
    } * entries;

  /* Secondary variables (not in the file) */
  int num_streams;
  int pts_len;
  
  } gavf_sync_index_t;

void gavf_sync_index_init(gavf_sync_index_t * idx, int num_streams);

void gavf_sync_index_add(gavf_t * g, uint64_t pos);

int gavf_sync_index_read(gavf_io_t * io, gavf_sync_index_t * idx);
int gavf_sync_index_write(gavf_io_t * io, const gavf_sync_index_t * idx);
void gavf_sync_index_free(gavf_sync_index_t * idx);
void gavf_sync_index_dump(const gavf_sync_index_t * idx);

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
  

  gavf_io_t * io;
  
  gavl_dictionary_t * cur;
  
  gavl_dictionary_t mi;
  
  gavf_sync_index_t     si;
  gavf_packet_index_t   pi;

  gavf_packet_header_t  pkthdr;
  
  gavf_stream_t * streams;
  int num_streams;
  
  gavl_handle_msg_func msg_callback;
  void * msg_data;

  gavf_chunk_t packets_chunk;
  gavf_chunk_t sync_chunk;
  
  gavf_options_t opt;
  
  gavf_io_t * pkt_io;
  
  uint64_t first_sync_pos;
  
  //  gavl_packet_t write_pkt;
  gavl_packet_t skip_pkt;
  
  gavl_video_frame_t * write_vframe;
  gavl_audio_frame_t * write_aframe;

  /* Time of the last sync header */
  gavl_time_t last_sync_time;
  gavl_time_t sync_distance;

  encoding_mode_t encoding_mode;
  encoding_mode_t final_encoding_mode;

  
  };

/* Footer */

int gavf_footer_check(gavf_t * g);
int gavf_footer_write(gavf_t * g);
void gavf_footer_init(gavf_t * g);

int gavf_program_header_write(gavf_t * g);
