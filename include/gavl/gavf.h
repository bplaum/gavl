
#ifndef GAVLGAVF_H_INCLUDED
#define GAVLGAVF_H_INCLUDED


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
#include <gavl/io.h>

#include <stdio.h>

/* Stream properties */
// #define GAVF_FLAG_MULTITRACK      (1<<0)
// #define GAVF_FLAG_MULTI_HEADER    (1<<1)
// #define GAVF_FLAG_HAVE_PKT_HEADER (1<<2)
#define GAVF_FLAG_WRITE           (1<<3)
#define GAVF_FLAG_EOF             (1<<4)

/* No seek support */
// #define GAVF_FLAG_STREAMING       (1<<5)

#define GAVF_FLAG_INTERACTIVE      (1<<6)
#define GAVF_FLAG_UNIX             (1<<7)
#define GAVF_FLAG_SEPARATE_STREAMS (1<<8)

/* on-disk */
#define GAVF_FLAG_ONDISK           (1<<9)
#define GAVF_FLAG_STARTED          (1<<10)


/* Flags which change at runtime */


#define GAVF_PROTOCOL_TCP      "gavf-tcp"
#define GAVF_PROTOCOL_TCPSERV  "gavf-tcpserv"
#define GAVF_PROTOCOL_UNIX     "gavf-unix"
#define GAVF_PROTOCOL_UNIXSERV "gavf-unixserv"

/* Generic protocol for selecting the right plugin with gmerlin */
#define GAVF_PROTOCOL          "gavf"

#define GAVF_EXTENSION         "gavf"

/* gavf specific dictionary */
#define GAVF_DICT              "gavf"
#define GAVF_META_HWSTORAGE    "hw" // Where the frames are stored

/* Packet flags (per stream) */
#define GAVL_PACKET_HAS_STREAM_ID  (1<<0)
#define GAVL_PACKET_HAS_DURATION   (1<<1)
#define GAVL_PACKET_HAS_INTERLACE  (1<<2)

/* Packet flags (per packet) */
#define GAVL_PACKET_HAS_RECTANGLE (1<<0)
#define GAVL_PACKET_HAS_TIMECODE  (1<<1)
#define GAVL_PACKET_HAS_HEADER    (1<<2)
#define GAVL_PACKET_HAS_SEQ_END   (1<<3)
#define GAVL_PACKET_HAS_FIELD2    (1<<4)


/*
 *  Also allowed in a GAVF_DICT:
 *  GAVL_META_URI Address for passing stream packets
 */



/* gavf is a stream format, which is basically a serialized version 
   of the internal stream structures, which are used throughout the
   whole gmerlin ecosystem.

   It was designed mainly for two reasons:

   1. As an on-disk format, which supports *all* variants of uncopressed
      media supported by gavl. Mostly for experiments.

   2. As a format, which allows to send multimedia between processes. This implies
      the support of packets in shared memory or dma-buffers.
      
   It is *not* meant to be a format for achiving movies or music. In particular,
   the fileformat will *not* be garantueed to be compatible between versions.
   You have been warned.
*/

/*

  General structures:
  CHUNK("name"):
  name: 8 characters, starts with "GAVF"
  len:  64 bit len (or offset in case of GAVFTAIL) as signed, 0 if unknown
  
  Chunks *always* start at 8 byte boundaries (counted from the start of the GAVFPHDR).
  Preceeding bytes (up to 7) are filled with zeros.
  
  CHUNK("GAVFPHDR")
  len bytes program header (dictionary)

  For the on-disk format, the program header is followed by

  CHUNK("GAVFPKTS")
  len bytes packets

  After that, multiplexed gavf packets follow until the end of the GAVFPKTS chunk.
  After the packets, the GAVFFOOT chunk contains the stream stats for each stream
  
  CHUNK("GAVFFOOT")
    footer (dictionary)
      {
      stats [ { stats } { stats } { stats } { stats } ]
      }
      
  Optional:
  GAVFPIDX: 8 bytes
  len:      8 bytes, bytes to follow, 0 if unknown
  len bytes packet index
   
  GAVFTAIL:      8 bytes
  size:          8 bytes, byte position after the last byte of these 8 bytes

  Files on the on-disk format can be joined using "cat" and the result will be
  a multi-track file

  The stream format sends the program header through the main I/O channel.
  Each stream has an entry GAVL_META_URI, which contains the address of a UNIX-socket
  where the stream packets are sent.
    
*/

#define GAVF_TAG_PROGRAM_HEADER "GAVFPHDR"
#define GAVF_TAG_PROGRAM_END    "GAVFPEND"

#define GAVF_TAG_PACKETS        "GAVFPKTS"

// #define GAVF_TAG_SYNC_HEADER    "GAVFSYNC"
// #define GAVF_TAG_SYNC_INDEX     "GAVFSIDX"
#define GAVF_TAG_PACKET_INDEX   "GAVFPIDX"
#define GAVF_TAG_FOOTER         "GAVFFOOT"
#define GAVF_TAG_TAIL           "GAVFTAIL"


typedef struct gavf_s gavf_t;
typedef struct gavf_options_s gavf_options_t;

/* Chunk structure */



/* Buffer as io */

GAVL_PUBLIC
gavl_io_t * gavl_io_create_buf_read(void);

GAVL_PUBLIC
gavl_io_t * gavl_io_create_buf_write(void);

GAVL_PUBLIC
gavl_buffer_t * gavl_io_buf_get(gavl_io_t * io);

GAVL_PUBLIC
void gavl_io_buf_reset(gavl_io_t * io);

GAVL_PUBLIC
void gavl_buffer_flush(gavl_buffer_t * buf, int len);

/* Return short name */
GAVL_PUBLIC
const char * gavf_stream_type_name(gavl_stream_type_t t);

typedef void (*gavf_packet_unref_func)(gavl_packet_t * p, void * priv);

/* Options */

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
int gavf_handle_reader_command(gavf_t *, const gavl_msg_t * m);

GAVL_PUBLIC
int gavf_read_writer_command(gavf_t * g, int timeout);

GAVL_PUBLIC
int gavf_start_program(gavf_t * g, const gavl_dictionary_t * track);


GAVL_PUBLIC
gavl_io_t * gavf_get_io(gavf_t * g);



/* Read support */

GAVL_PUBLIC
int gavf_open_read(gavf_t * g, gavl_io_t * io);

GAVL_PUBLIC
int gavf_open_uri_read(gavf_t * g, const char * uri);

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


/* Mark this stream as skipable. */

GAVL_PUBLIC
void gavf_stream_set_skip(gavf_t * g, uint32_t id);

GAVL_PUBLIC
void gavf_stream_set_unref(gavf_t * gavf, uint32_t id,
                           gavf_packet_unref_func func, void * priv);

GAVL_PUBLIC
void gavf_packet_skip(gavf_t * gavf);

GAVL_PUBLIC
int gavf_reset(gavf_t * gavf);


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
void gavf_audio_frame_to_packet_metadata(const gavl_audio_frame_t * frame,
                                         gavl_packet_t * p);

GAVL_PUBLIC
void gavf_shrink_audio_frame(gavl_audio_frame_t * f, 
                             gavl_packet_t * p, 
                             const gavl_audio_format_t * format);

/* Write support */


GAVL_PUBLIC
int gavf_open_uri_write(gavf_t * g, const char * uri);

GAVL_PUBLIC
int gavf_set_media_info(gavf_t * g, const gavl_dictionary_t * mi);

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

/* Ultra simple image format */

GAVL_PUBLIC
int gavl_image_write_header(gavl_io_t * io,
                            const gavl_dictionary_t * m,
                            const gavl_video_format_t * v);

GAVL_PUBLIC
int gavl_image_write_image(gavl_io_t * io,
                           const gavl_video_format_t * v,
                           gavl_video_frame_t * frame);

GAVL_PUBLIC
int gavl_image_read_header(gavl_io_t * io,
                           gavl_dictionary_t * m,
                           gavl_video_format_t * v);

GAVL_PUBLIC
int gavl_image_read_image(gavl_io_t * io,
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
int gavl_dictionary_from_buffer(const uint8_t * buf, int len, gavl_dictionary_t * fmt);
  
GAVL_PUBLIC
uint8_t * gavl_dictionary_to_buffer(int * len, const gavl_dictionary_t * fmt);


/* Formats */

/* Metadata */

GAVL_PUBLIC
int gavl_dictionary_read(gavl_io_t * io, gavl_dictionary_t * ci);

GAVL_PUBLIC
int gavl_dictionary_write(gavl_io_t * io, const gavl_dictionary_t * ci);



/* Utilify functions for messages */

GAVL_PUBLIC
void gavf_set_msg_cb(gavf_t * g, gavl_handle_msg_func msg_callback, void * msg_data);


GAVL_PUBLIC
void gavf_options_copy(gavf_options_t * dst, const gavf_options_t * src);



#endif // GAVLGAVF_H_INCLUDED
