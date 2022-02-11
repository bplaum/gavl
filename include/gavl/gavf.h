
#ifndef GAVF_H_INCLUDED
#define GAVF_H_INCLUDED


#include <inttypes.h>
#include <gavl/gavl.h>
#include <gavl/compression.h>
#include <gavl/chapterlist.h>
#include <gavl/metadata.h>
#include <gavl/gavldefs.h>
#include <gavl/connectors.h>
#include <gavl/gavldsp.h>
#include <gavl/utils.h>
#include <gavl/msg.h>
#include <gavl/value.h>
#include <gavl/trackinfo.h>

#include <stdio.h>

/* Stream properties */
#define GAVF_FLAG_MULTITRACK      (1<<0)
#define GAVF_FLAG_MULTI_HEADER    (1<<1)
#define GAVF_FLAG_HAVE_PKT_HEADER (1<<2)
#define GAVF_FLAG_WRITE           (1<<3)
#define GAVF_FLAG_EOF             (1<<4)

/* No seek support */
#define GAVF_FLAG_STREAMING       (1<<5)

/* gavf file structure */

/*

  The gavf layer works only on track level. Track switching must be
  hanbled by a wrapper (e.g. bgplug in libgmerlin)
  
  CHUNK("name"):
  name: 8 characters, starts with "GAVF"
  len:  64 bit len (or offset in case of GAVFTAIL) as signed, 0 if unknown
  
  Chunks *always* start at 8 byte boundaries. Preceeding bytes (up to 7) are
  filled with zeros
  
  CHUNK("GAVFPHDR")
  len bytes header (dictionary)
  
  CHUNK("GAVFPKTS")
  len bytes packets

    CHUNK("GAVFSYNC")
    sync_chunk (variable)
    
      P:        1 byte
      packet (variable)

      P:        1 byte
      packet (variable)

      [...]

    CHUNK("GAVFSYNC")
    sync_chunk (variable)
    
      P:        1 byte
      packet (variable)

      P:        1 byte
      packet (variable)

      [...]

    [...]
    

  CHUNK("GAVFPEND")
  len:      8 bytes, 0 

    
  CHUNK("GAVFFOOT")
    footer
      {
      stats [ { stats } { stats } { stats } { stats } ]
      }
   
  Optional:
  CHUNK("GAVFSIDX") 8 bytes
  len:      8 bytes, bytes to follow, 0 if unknown
  len bytes sync index
  
  Optional:
  GAVFPIDX: 8 bytes
  len:      8 bytes, bytes to follow, 0 if unknown
  len bytes packet index
   
  GAVFTAIL:      8 bytes
  footer_offset: 8 bytes, file offset of the GAVFFOOT tag
  size:          8 bytes, byte position after the last byte of these 8 bytes
  

  
*/

#define GAVF_TAG_PROGRAM_HEADER "GAVFPHDR"
#define GAVF_TAG_PROGRAM_END    "GAVFPEND"

#define GAVF_TAG_PACKETS        "GAVFPKTS"

#define GAVF_TAG_SYNC_HEADER    "GAVFSYNC"
#define GAVF_TAG_SYNC_INDEX     "GAVFSIDX"
#define GAVF_TAG_PACKET_INDEX   "GAVFPIDX"
#define GAVF_TAG_FOOTER         "GAVFFOOT"
#define GAVF_TAG_TAIL           "GAVFTAIL"


typedef struct gavf_s gavf_t;
typedef struct gavf_options_s gavf_options_t;

//#define GAVF_IO_CB_PROGRAM_HEADER_START 1
//#define GAVF_IO_CB_PROGRAM_HEADER_END   (1|0x100)
//#define GAVF_IO_CB_PACKET_START         2
//#define GAVF_IO_CB_PACKET_END           (2|0x100)
//#define GAVF_IO_CB_SYNC_HEADER_START    4
//#define GAVF_IO_CB_SYNC_HEADER_END      (4|0x100)

//#define GAVF_IO_CB_TYPE_END(t) (!!(t & 0x100))

/* Chunk structure */

typedef struct
  {
  char eightcc[9];
  int64_t start; // gavf_io_position();
  int64_t len;   // gavf_io_position() - start;
  } gavf_chunk_t;

/* I/O Structure */

typedef int (*gavf_read_func)(void * priv, uint8_t * data, int len);
typedef int (*gavf_write_func)(void * priv, const uint8_t * data, int len);
typedef int64_t (*gavf_seek_func)(void * priv, int64_t pos, int whence);
typedef void (*gavf_close_func)(void * priv);
typedef int (*gavf_flush_func)(void * priv);
typedef int (*gavf_poll_func)(void * priv, int timeout);



// typedef int (*gavf_io_cb_func)(void * priv, int type, const void * data);

typedef struct gavl_io_s gavf_io_t;

GAVL_PUBLIC
void gavf_io_set_poll_func(gavf_io_t * io, gavf_poll_func f);

GAVL_PUBLIC
int gavf_io_can_read(gavf_io_t * io, int timeout);

GAVL_PUBLIC
int gavf_read_dictionary(gavf_io_t * io,
                         gavf_chunk_t * head,
                         gavl_dictionary_t * ret);


GAVL_PUBLIC
int gavf_chunk_read_header(gavf_io_t * io, gavf_chunk_t * head);

GAVL_PUBLIC
int gavf_chunk_is(const gavf_chunk_t * head, const char * eightcc);

GAVL_PUBLIC
int gavf_chunk_start(gavf_io_t * io, gavf_chunk_t * head, const char * eightcc);

GAVL_PUBLIC
int gavf_chunk_finish(gavf_io_t * io, gavf_chunk_t * head, int write_size);

GAVL_PUBLIC
gavf_io_t * gavf_chunk_start_io(gavf_io_t * io, gavf_chunk_t * head, const char * eightcc);

GAVL_PUBLIC
int gavf_chunk_finish_io(gavf_io_t * io, gavf_chunk_t * head, gavf_io_t * sub_io);

GAVL_PUBLIC
gavf_io_t * gavf_io_create(gavf_read_func  r,
                           gavf_write_func w,
                           gavf_seek_func  s,
                           gavf_close_func c,
                           gavf_flush_func f,
                           void * data);

GAVL_PUBLIC
int gavf_io_align_write(gavf_io_t * io);

GAVL_PUBLIC
int gavf_io_align_read(gavf_io_t * io);


GAVL_PUBLIC
void * gavf_io_get_priv(gavf_io_t * io);

GAVL_PUBLIC
void gavf_io_destroy(gavf_io_t *);

GAVL_PUBLIC
int gavf_io_flush(gavf_io_t *);

GAVL_PUBLIC
gavf_io_t * gavf_io_create_file(FILE * f, int wr, int can_seek, int do_close);

#define GAVF_IO_SOCKET_DO_CLOSE     (1<<0)

#define GAVF_IO_SOCKET_BUFFER_READ  (1<<1)
// #define GAVF_IO_SOCKET_BUFFER_WRITE (1<<2)


GAVL_PUBLIC
gavf_io_t * gavf_io_create_socket_1(int fd, int read_timeout, int flags);

GAVL_PUBLIC
int gavf_io_get_socket(gavf_io_t * io);

GAVL_PUBLIC
gavf_io_t * gavf_io_create_sub_read(gavf_io_t * io, int64_t offset, int64_t len);

/* Data is appended to the current position */
GAVL_PUBLIC
gavf_io_t * gavf_io_create_sub_write(gavf_io_t * io);

GAVL_PUBLIC
uint8_t * gavf_io_mem_get_buf(gavf_io_t * io, int * len);

GAVL_PUBLIC
gavf_io_t * gavf_io_create_mem_read(const uint8_t * ptr, int len);

GAVL_PUBLIC
gavf_io_t * gavf_io_create_mem_write();

GAVL_PUBLIC
gavf_io_t * gavf_io_create_tls_client(int fd, const char * server_name, int flags);

// GAVL_PUBLIC
// void gavf_io_set_cb(gavf_io_t * io, gavf_io_cb_func cb, void * cb_priv);

GAVL_PUBLIC
void gavf_io_set_msg_cb(gavf_io_t * io, gavl_handle_msg_func msg_callback, void * msg_data);

GAVL_PUBLIC
int gavf_io_got_error(gavf_io_t * io);

GAVL_PUBLIC
int gavf_io_can_seek(gavf_io_t * io);

GAVL_PUBLIC
int gavf_io_read_data(gavf_io_t * io, uint8_t * buf, int len);

GAVL_PUBLIC
int gavf_io_read_data_nonblock(gavf_io_t * io, uint8_t * buf, int len);

/* Get data but don't remove from input */
GAVL_PUBLIC
int gavf_io_get_data(gavf_io_t * io, uint8_t * buf, int len);

GAVL_PUBLIC
int gavf_io_write_data(gavf_io_t * io, const uint8_t * buf, int len);

GAVL_PUBLIC
int gavf_io_read_line(gavf_io_t * io, char ** ret, int * ret_alloc, int max_len);

GAVL_PUBLIC
int64_t gavf_io_seek(gavf_io_t * io, int64_t pos, int whence);

GAVL_PUBLIC
int64_t gavf_io_total_bytes(gavf_io_t * io);

GAVL_PUBLIC
const char * gavf_io_filename(gavf_io_t * io);

GAVL_PUBLIC
const char * gavf_io_mimetype(gavf_io_t * io);

GAVL_PUBLIC
void gavf_io_set_info(gavf_io_t * io, int64_t total_bytes,
                      const char * filename, const char * mimetype);

GAVL_PUBLIC
int64_t gavf_io_position(gavf_io_t * io);

/* Reset position to zero. This is necessary for socket based
   io to subtract the handshake headers from the file offsets */

GAVL_PUBLIC
void gavf_io_reset_position(gavf_io_t * io);

GAVL_PUBLIC
int gavf_io_write_uint64f(gavf_io_t * io, uint64_t num);

GAVL_PUBLIC
int gavf_io_read_uint64f(gavf_io_t * io, uint64_t * num);

GAVL_PUBLIC
int gavf_io_write_int64f(gavf_io_t * io, int64_t num);

GAVL_PUBLIC
int gavf_io_read_int64f(gavf_io_t * io, int64_t * num);

GAVL_PUBLIC
int gavf_io_write_uint64v(gavf_io_t * io, uint64_t num);

GAVL_PUBLIC
int gavf_io_read_uint64v(gavf_io_t * io, uint64_t * num);

GAVL_PUBLIC
int gavf_io_write_uint32v(gavf_io_t * io, uint32_t num);

GAVL_PUBLIC
int gavf_io_read_uint32v(gavf_io_t * io, uint32_t * num);

GAVL_PUBLIC
int gavf_io_write_int64v(gavf_io_t * io, int64_t num);

GAVL_PUBLIC
int gavf_io_read_int64v(gavf_io_t * io, int64_t * num);

GAVL_PUBLIC
int gavf_io_write_int32v(gavf_io_t * io, int32_t num);

GAVL_PUBLIC
int gavf_io_read_int32v(gavf_io_t * io, int32_t * num);

GAVL_PUBLIC
int gavf_io_read_string(gavf_io_t * io, char **);

GAVL_PUBLIC
int gavf_io_write_string(gavf_io_t * io, const char * );

GAVL_PUBLIC
int gavf_io_read_buffer(gavf_io_t * io, gavl_buffer_t * ret);

GAVL_PUBLIC
int gavf_io_write_buffer(gavf_io_t * io,  const gavl_buffer_t * buf);

GAVL_PUBLIC
int gavf_io_read_float(gavf_io_t * io, float * num);

GAVL_PUBLIC
int gavf_io_write_float(gavf_io_t * io, float num);

GAVL_PUBLIC
int gavf_io_read_double(gavf_io_t * io, double * num);

GAVL_PUBLIC
int gavf_io_write_double(gavf_io_t * io, double num);

GAVL_PUBLIC
int gavl_value_read(gavf_io_t * io, gavl_value_t * v);

GAVL_PUBLIC
int gavl_value_write(gavf_io_t * io, const gavl_value_t * v);

GAVL_PUBLIC
int gavl_dictionary_write(gavf_io_t * io, const gavl_dictionary_t * dict);

GAVL_PUBLIC
int gavl_dictionary_read(gavf_io_t * io, gavl_dictionary_t * dict);

/* Read/write fixed length integers */

GAVL_PUBLIC
int gavf_io_read_8(gavf_io_t * ctx, uint8_t * ret);

GAVL_PUBLIC
int gavf_io_read_16_le(gavf_io_t * ctx,uint16_t * ret);

GAVL_PUBLIC
int gavf_io_read_32_le(gavf_io_t * ctx,uint32_t * ret);

GAVL_PUBLIC
int gavf_io_read_24_le(gavf_io_t * ctx,uint32_t * ret);

GAVL_PUBLIC
int gavf_io_read_64_le(gavf_io_t * ctx,uint64_t * ret);

GAVL_PUBLIC
int gavf_io_read_16_be(gavf_io_t * ctx,uint16_t * ret);

GAVL_PUBLIC
int gavf_io_read_24_be(gavf_io_t * ctx,uint32_t * ret);

GAVL_PUBLIC
int gavf_io_read_32_be(gavf_io_t * ctx,uint32_t * ret);

GAVL_PUBLIC
int gavf_io_read_64_be(gavf_io_t * ctx, uint64_t * ret);

/* Write */

GAVL_PUBLIC
int gavf_io_write_8(gavf_io_t * ctx, uint8_t val);

GAVL_PUBLIC
int gavf_io_write_16_le(gavf_io_t * ctx, uint16_t val);

GAVL_PUBLIC
int gavf_io_write_32_le(gavf_io_t * ctx,uint32_t val);

GAVL_PUBLIC
int gavf_io_write_24_le(gavf_io_t * ctx,uint32_t val);

GAVL_PUBLIC
int gavf_io_write_64_le(gavf_io_t * ctx,uint64_t val);

GAVL_PUBLIC
int gavf_io_write_16_be(gavf_io_t * ctx,uint16_t val);

GAVL_PUBLIC
int gavf_io_write_24_be(gavf_io_t * ctx,uint32_t val);

GAVL_PUBLIC
int gavf_io_write_32_be(gavf_io_t * ctx,uint32_t val);

GAVL_PUBLIC
int gavf_io_write_64_be(gavf_io_t * ctx, uint64_t val);

/** \brief Read a message using a callback
 *  \param ret Where the message will be copied
 *  \param io I/O context
 *  \returns 1 on success, 0 on error
 */

GAVL_PUBLIC
int gavl_msg_read(gavl_msg_t * ret, gavf_io_t * io);

/** \brief Write a message using a callback
 *  \param msg A message
 *  \param io I/O context
 *  \returns 1 on success, 0 on error
 */

GAVL_PUBLIC
int gavl_msg_write(const gavl_msg_t * msg, gavf_io_t * io);

/* Buffer as io */

GAVL_PUBLIC
gavf_io_t * gavf_io_create_buf_read(void);

GAVL_PUBLIC
gavf_io_t * gavf_io_create_buf_write(void);

GAVL_PUBLIC
gavl_buffer_t * gavf_io_buf_get(gavf_io_t * io);

GAVL_PUBLIC
void gavf_io_buf_reset(gavf_io_t * io);

GAVL_PUBLIC
void gavl_buffer_flush(gavl_buffer_t * buf, int len);

/* Return short name */
GAVL_PUBLIC
const char * gavf_stream_type_name(gavl_stream_type_t t);


typedef struct
  {
  int32_t stream_id;
  } gavf_packet_header_t;

typedef void (*gavf_packet_unref_func)(gavl_packet_t * p, void * priv);

/* Options */

#define GAVF_OPT_FLAG_SYNC_INDEX   (1<<0)
#define GAVF_OPT_FLAG_PACKET_INDEX (1<<1)
#define GAVF_OPT_FLAG_INTERLEAVE   (1<<2)
#define GAVF_OPT_FLAG_BUFFER_READ  (1<<3)

#define GAVF_OPT_FLAG_DUMP_HEADERS (1<<4)
#define GAVF_OPT_FLAG_DUMP_INDICES (1<<5)
#define GAVF_OPT_FLAG_DUMP_PACKETS (1<<6)
#define GAVF_OPT_FLAG_DUMP_METADATA (1<<7)

#define GAVF_OPT_FLAG_ORIG_PTS     (1<<8)


GAVL_PUBLIC
void gavf_options_set_flags(gavf_options_t *, int flags);

GAVL_PUBLIC
int gavf_options_get_flags(gavf_options_t *);

GAVL_PUBLIC
void gavf_options_set_sync_distance(gavf_options_t *,
                                    gavl_time_t sync_distance);

GAVL_PUBLIC
gavf_options_t * gavf_options_create();

GAVL_PUBLIC
void gavf_options_destroy(gavf_options_t *);

// GAVL_PUBLIC
// void gavf_options_set_msg_callback(gavf_options_t *, 
//                                   gavl_handle_msg_func cb,
//                                   void *cb_priv);


/* General functions */

GAVL_PUBLIC
gavf_t * gavf_create();

GAVL_PUBLIC
void gavf_close(gavf_t *, int discard);

GAVL_PUBLIC
gavf_options_t * gavf_get_options(gavf_t *);

GAVL_PUBLIC
void gavf_set_options(gavf_t *, const gavf_options_t *);

GAVL_PUBLIC
int gavf_get_flags(gavf_t *);

GAVL_PUBLIC
void gavf_write_resync(gavf_t * g, int64_t time, int scale, int discard, int discont);

/* Read support */

GAVL_PUBLIC
int gavf_open_read(gavf_t * g, gavf_io_t * io);

/* Clear the EOF flag of the demuxer */
GAVL_PUBLIC
void gavf_clear_eof(gavf_t * g);

GAVL_PUBLIC
gavl_dictionary_t * gavf_get_media_info_nc(gavf_t * g);

GAVL_PUBLIC
const gavl_dictionary_t * gavf_get_media_info(const gavf_t * g);

GAVL_PUBLIC
gavl_dictionary_t * gavf_get_current_track_nc(gavf_t * g);

GAVL_PUBLIC
const gavl_dictionary_t * gavf_get_current_track(const gavf_t * g);

GAVL_PUBLIC
int gavf_select_track(gavf_t * g, int track);

GAVL_PUBLIC
gavl_source_status_t gavf_demux_iteration(gavf_t * g);

GAVL_PUBLIC
void gavf_clear_buffers(gavf_t * g);

#if 0

GAVL_PUBLIC
gavf_program_header_t * gavf_get_program_header(gavf_t *);

GAVL_PUBLIC
int gavf_get_num_streams(gavf_t *, int type);

GAVL_PUBLIC
const gavf_stream_header_t * gavf_get_stream(gavf_t *, int index, int type);

#endif

GAVL_PUBLIC
const gavf_packet_header_t * gavf_packet_read_header(gavf_t * gavf);

/* Mark this stream as skipable. */

GAVL_PUBLIC
void gavf_stream_set_skip(gavf_t * g, uint32_t id);

GAVL_PUBLIC
void gavf_stream_set_unref(gavf_t * gavf, uint32_t id,
                           gavf_packet_unref_func func, void * priv);

GAVL_PUBLIC
void gavf_packet_skip(gavf_t * gavf);

GAVL_PUBLIC
int gavf_packet_read_packet(gavf_t * gavf, gavl_packet_t * p);

GAVL_PUBLIC
int gavf_reset(gavf_t * gavf);

/* Get the first PTS of all streams */

GAVL_PUBLIC
const int64_t * gavf_first_pts(gavf_t * gavf);

/* Get the last PTS of the streams */

GAVL_PUBLIC
const int64_t * gavf_end_pts(gavf_t * gavf);

/* Seek to a specific time. Return the sync timestamps of
   all streams at the current position */

GAVL_PUBLIC
const int64_t * gavf_seek(gavf_t * gavf, int64_t time, int scale);

/* Read uncompressed frames */

GAVL_PUBLIC
void gavf_packet_to_video_frame(gavl_packet_t * p, gavl_video_frame_t * frame,
                                const gavl_video_format_t * format,
                                const gavl_dictionary_t * m,
                                gavl_dsp_context_t ** ctx);

GAVL_PUBLIC
void gavf_packet_to_audio_frame(gavl_packet_t * p, gavl_audio_frame_t * frame,
                                const gavl_audio_format_t * format,
                                const gavl_dictionary_t * m,
                                gavl_dsp_context_t ** ctx);

/* frame must be allocated with the maximum size already! */

GAVL_PUBLIC
void gavf_packet_to_overlay(gavl_packet_t * p,
                            gavl_video_frame_t * frame,
                            const gavl_video_format_t * format);

/* Packet must be allocated with the maximum size already! */
GAVL_PUBLIC
void gavf_overlay_to_packet(gavl_video_frame_t * frame, 
                            gavl_packet_t * p,
                            const gavl_video_format_t * format);

/* These copy *only* the metadata */
GAVL_PUBLIC
void gavf_video_frame_to_packet_metadata(const gavl_video_frame_t * frame,
                                         gavl_packet_t * p);

GAVL_PUBLIC
void gavf_audio_frame_to_packet_metadata(const gavl_audio_frame_t * frame,
                                         gavl_packet_t * p);

GAVL_PUBLIC
void gavf_shrink_audio_frame(gavl_audio_frame_t * f, 
                             gavl_packet_t * p, 
                             const gavl_audio_format_t * format);

/* Write support */

GAVL_PUBLIC
int gavf_open_write(gavf_t * g, gavf_io_t * io,
                    const gavl_dictionary_t * m);

/*
 *  Return value: >= 0 is the stream index passed to gavf_write_packet()
 *  < 0 means error
 */

GAVL_PUBLIC
int gavf_append_audio_stream(gavf_t * g,
                          const gavl_compression_info_t * ci,
                          const gavl_audio_format_t * format,
                          const gavl_dictionary_t * m);

GAVL_PUBLIC
int gavf_append_video_stream(gavf_t * g,
                          const gavl_compression_info_t * ci,
                          const gavl_video_format_t * format,
                          const gavl_dictionary_t * m);

GAVL_PUBLIC
int gavf_append_text_stream(gavf_t * g,
                         uint32_t timescale,
                         const gavl_dictionary_t * m);

GAVL_PUBLIC
int gavf_append_overlay_stream(gavf_t * g,
                            const gavl_compression_info_t * ci,
                            const gavl_video_format_t * format,
                            const gavl_dictionary_t * m);

GAVL_PUBLIC
void gavf_add_msg_stream(gavf_t * g, int id);

GAVL_PUBLIC
void gavf_add_streams(gavf_t * g, const gavl_dictionary_t * track);

/* Call this after adding all streams and before writing the first packet */
GAVL_PUBLIC
int gavf_start(gavf_t * g);

// GAVL_PUBLIC
// int gavf_write_packet(gavf_t *, int stream, const gavl_packet_t * p);

GAVL_PUBLIC
int gavf_write_video_frame(gavf_t *, int stream, gavl_video_frame_t * frame);

GAVL_PUBLIC
int gavf_write_audio_frame(gavf_t *, int stream, gavl_audio_frame_t * frame);

GAVL_PUBLIC
int gavf_put_message(void*, gavl_msg_t * m);

GAVL_PUBLIC gavl_packet_sink_t *
gavf_get_packet_sink(gavf_t *, uint32_t id);

GAVL_PUBLIC gavl_audio_sink_t *
gavf_get_audio_sink(gavf_t *, uint32_t id);

GAVL_PUBLIC gavl_video_sink_t *
gavf_get_video_sink(gavf_t *, uint32_t id);

GAVL_PUBLIC gavl_packet_source_t *
gavf_get_packet_source(gavf_t *, uint32_t id);

GAVL_PUBLIC gavl_audio_source_t *
gavf_get_audio_source(gavf_t *, uint32_t id);

GAVL_PUBLIC gavl_video_source_t *
gavf_get_video_source(gavf_t *, uint32_t id);

/* Utility functions */

GAVL_PUBLIC 
int gavf_get_max_audio_packet_size(const gavl_audio_format_t * fmt,
                                   const gavl_compression_info_t * ci);

GAVL_PUBLIC 
int gavf_get_max_video_packet_size(const gavl_video_format_t * fmt,
                                   const gavl_compression_info_t * ci);



/* Ultra simple image format */

GAVL_PUBLIC
int gavl_image_write_header(gavf_io_t * io,
                            const gavl_dictionary_t * m,
                            const gavl_video_format_t * v);

GAVL_PUBLIC
int gavl_image_write_image(gavf_io_t * io,
                           const gavl_video_format_t * v,
                           gavl_video_frame_t * frame);

GAVL_PUBLIC
int gavl_image_read_header(gavf_io_t * io,
                           gavl_dictionary_t * m,
                           gavl_video_format_t * v);

GAVL_PUBLIC
int gavl_image_read_image(gavf_io_t * io,
                          gavl_video_format_t * v,
                          gavl_video_frame_t * f);

/* (de-)serialize gavl structs to/from buffers */

GAVL_PUBLIC
int gavl_audio_format_from_buffer(const uint8_t * buf, int len, gavl_audio_format_t * fmt);

GAVL_PUBLIC
uint8_t * gavl_audio_format_to_buffer(int * len, const gavl_audio_format_t * fmt);

GAVL_PUBLIC
int gavl_video_format_from_buffer(const uint8_t * buf, int len, gavl_video_format_t * fmt);
  
GAVL_PUBLIC
uint8_t * gavl_video_format_to_buffer(int * len, const gavl_video_format_t * fmt);

GAVL_PUBLIC
int gavl_metadata_from_buffer(const uint8_t * buf, int len, gavl_dictionary_t * fmt);
  
GAVL_PUBLIC
uint8_t * gavl_dictionary_to_buffer(int * len, const gavl_dictionary_t * fmt);

#if 0
GAVL_PUBLIC
int gavl_compression_info_from_buffer(const uint8_t * buf, int len, gavl_compression_info_t * fmt);
 
GAVL_PUBLIC
uint8_t * gavl_compression_info_to_buffer(int * len, const gavl_compression_info_t * fmt);
#endif

GAVL_PUBLIC
uint8_t * gavl_msg_to_buffer(int * len, const gavl_msg_t * msg);

GAVL_PUBLIC
int gavl_msg_from_buffer(const uint8_t * buf, int len, gavl_msg_t * msg);

/* Formats */
GAVL_PUBLIC
int gavf_read_audio_format(gavf_io_t * io, gavl_audio_format_t * format);
GAVL_PUBLIC
int gavf_write_audio_format(gavf_io_t * io, const gavl_audio_format_t * format);

GAVL_PUBLIC
int gavf_read_video_format(gavf_io_t * io, gavl_video_format_t * format);

GAVL_PUBLIC
int gavf_write_video_format(gavf_io_t * io, const gavl_video_format_t * format);

/* Compression info */

#if 0
GAVL_PUBLIC
int gavf_read_compression_info(gavf_io_t * io,
                               gavl_compression_info_t * ci);

GAVL_PUBLIC
int gavf_write_compression_info(gavf_io_t * io,
                                const gavl_compression_info_t * ci);
#endif

/* Metadata */

GAVL_PUBLIC
int gavl_dictionary_read(gavf_io_t * io, gavl_dictionary_t * ci);

GAVL_PUBLIC
int gavl_dictionary_write(gavf_io_t * io, const gavl_dictionary_t * ci);

/* Packet */

GAVL_PUBLIC
int gavf_read_gavl_packet(gavf_io_t * io,
                          int default_duration,
                          int packet_flags,
                          int64_t last_sync_pts,
                          int64_t * next_pts,
                          int64_t pts_offset,
                          gavl_packet_t * p);

GAVL_PUBLIC
int gavf_write_gavl_packet(gavf_io_t * io,
                           gavf_io_t * hdr_io,
                           int packet_duration,
                           int packet_flags,
                           int64_t last_sync_pts,
                           const gavl_packet_t * p);

/* Utilify functions for messages */

GAVL_PUBLIC
void gavf_set_msg_cb(gavf_t * g, gavl_handle_msg_func msg_callback, void * msg_data);

GAVL_PUBLIC
int gavf_msg_to_packet(const gavl_msg_t * msg,
                       gavl_packet_t * dst);

GAVL_PUBLIC
int gavf_packet_to_msg(const gavl_packet_t * src,
                       gavl_msg_t * msg);

GAVL_PUBLIC
void gavf_options_copy(gavf_options_t * dst, const gavf_options_t * src);



#endif // GAVF_H_INCLUDED
