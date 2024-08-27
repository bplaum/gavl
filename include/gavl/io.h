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



#ifndef GAVL_IO_H_INCLUDED
#define GAVL_IO_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <inttypes.h>

#include <gavl/msg.h>

  
/* IO flags */

#define GAVL_IO_CAN_READ          (1<<0)
#define GAVL_IO_CAN_WRITE         (1<<1)
#define GAVL_IO_CAN_SEEK          (1<<2)
#define GAVL_IO_IS_DUPLEX         (1<<3) // Duplex means we have the backchannel for messages
#define GAVL_IO_IS_REGULAR        (1<<4)
#define GAVL_IO_IS_SOCKET         (1<<5)
#define GAVL_IO_IS_UNIX_SOCKET    (1<<6)
#define GAVL_IO_IS_LOCAL          (1<<7)
#define GAVL_IO_IS_PIPE           (1<<8)
#define GAVL_IO_IS_TTY            (1<<9)
 
#define GAVL_IO_EOF               (1<<16)
#define GAVL_IO_ERROR             (1<<17)
  
/* Crypto I/O */
typedef enum
  {
    GAVL_CIPHER_NONE = 0,
    GAVL_CIPHER_AES128,
  } gavl_cipher_algo_t;

typedef enum
  {
    GAVL_CIPHER_MODE_NONE = 0,
    GAVL_CIPHER_MODE_CBC,
    
  } gavl_cipher_mode_t;

typedef enum
  {
    GAVL_CIPHER_PADDING_NONE = 0,
    GAVL_CIPHER_PADDING_PKCS7,
  } gavl_cipher_padding_t;

  
/* I/O Structure */

typedef int (*gavl_read_func)(void * priv, uint8_t * data, int len);
typedef int (*gavl_write_func)(void * priv, const uint8_t * data, int len);
typedef int64_t (*gavl_seek_func)(void * priv, int64_t pos, int whence);
typedef void (*gavl_close_func)(void * priv);
typedef int (*gavl_flush_func)(void * priv);
typedef int (*gavl_poll_func)(void * priv, int timeout, int wr);



// typedef int (*gavl_io_cb_func)(void * priv, int type, const void * data);

typedef struct gavl_io_s gavl_io_t;

GAVL_PUBLIC
void gavl_io_set_poll_func(gavl_io_t * io, gavl_poll_func f);

GAVL_PUBLIC
int gavl_io_can_read(gavl_io_t * io, int timeout);

GAVL_PUBLIC
int gavl_io_can_write(gavl_io_t * io, int timeout);

GAVL_PUBLIC
gavl_io_t * gavl_io_create(gavl_read_func  r,
                           gavl_write_func w,
                           gavl_seek_func  s,
                           gavl_close_func c,
                           gavl_flush_func f,
                           int flags,
                           void * data);

GAVL_PUBLIC
int gavl_io_align_write(gavl_io_t * io);

GAVL_PUBLIC
int gavl_io_align_read(gavl_io_t * io);


GAVL_PUBLIC
void * gavl_io_get_priv(gavl_io_t * io);

GAVL_PUBLIC
void gavl_io_destroy(gavl_io_t *);

GAVL_PUBLIC
int gavl_io_flush(gavl_io_t *);

GAVL_PUBLIC
void gavl_io_skip(gavl_io_t * io, int bytes);
  
GAVL_PUBLIC
gavl_io_t * gavl_io_create_file(FILE * f, int wr, int can_seek, int do_close);

GAVL_PUBLIC
gavl_io_t * gavl_io_from_filename(const char * filename, int wr);

  
#define GAVL_IO_SOCKET_DO_CLOSE     (1<<0)

#define GAVL_IO_SOCKET_BUFFER_READ  (1<<1)
// #define GAVL_IO_SOCKET_BUFFER_WRITE (1<<2)


GAVL_PUBLIC
gavl_io_t * gavl_io_create_socket(int fd, int read_timeout, int flags);

GAVL_PUBLIC
int gavl_io_get_socket(gavl_io_t * io);

GAVL_PUBLIC
gavl_io_t * gavl_io_create_cipher(gavl_cipher_algo_t algo,
                                  gavl_cipher_mode_t mode,
                                  gavl_cipher_padding_t padding, int wr);

GAVL_PUBLIC
void gavl_io_cipher_init(gavl_io_t * io,
                         gavl_io_t * src,
                         const uint8_t * key,
                         const uint8_t * iv);

GAVL_PUBLIC
void gavl_io_cipher_set_src(gavl_io_t * io,
                            gavl_io_t * src);


GAVL_PUBLIC
int gavl_io_get_flags(gavl_io_t * io);

GAVL_PUBLIC
gavl_io_t * gavl_io_create_sub_read(gavl_io_t * io, int64_t offset, int64_t len);

/* Data is appended to the current position */
GAVL_PUBLIC
gavl_io_t * gavl_io_create_sub_write(gavl_io_t * io);

GAVL_PUBLIC
uint8_t * gavl_io_mem_get_buf(gavl_io_t * io, int * len);

GAVL_PUBLIC
gavl_io_t * gavl_io_create_mem_read(const uint8_t * ptr, int len);

GAVL_PUBLIC
gavl_io_t * gavl_io_create_mem_write();

/* Buffer I/O */

GAVL_PUBLIC
gavl_io_t * gavl_io_create_buffer_write(gavl_buffer_t * buf);

GAVL_PUBLIC
gavl_io_t * gavl_io_create_buffer_read(const gavl_buffer_t * buf);



GAVL_PUBLIC
gavl_io_t * gavl_io_create_tls_client(int fd, const char * server_name, int flags);

GAVL_PUBLIC
gavl_io_t * gavl_io_create_tls_client_async(int fd, const char * server_name, int socket_flags);

GAVL_PUBLIC
int gavl_io_create_tls_client_async_done(gavl_io_t * io, int timeout);

GAVL_PUBLIC
void gavl_io_set_error(gavl_io_t * io);

GAVL_PUBLIC
void gavl_io_clear_error(gavl_io_t * io);

GAVL_PUBLIC
void gavl_io_set_eof(gavl_io_t * io);

GAVL_PUBLIC
void gavl_io_clear_eof(gavl_io_t * io);

GAVL_PUBLIC
int gavl_io_got_error(gavl_io_t * io);

GAVL_PUBLIC
int gavl_io_got_eof(gavl_io_t * io);

GAVL_PUBLIC
int gavl_io_can_seek(gavl_io_t * io);

GAVL_PUBLIC
int gavl_io_read_data(gavl_io_t * io, uint8_t * buf, int len);

GAVL_PUBLIC
void gavl_io_unread_data(gavl_io_t * io, const uint8_t * buf, int len);


GAVL_PUBLIC
int gavl_io_read_data_nonblock(gavl_io_t * io, uint8_t * buf, int len);

GAVL_PUBLIC
int gavl_io_write_data_nonblock(gavl_io_t * io, const uint8_t * buf, int len);

/* Get data but don't remove from input */
GAVL_PUBLIC
int gavl_io_get_data(gavl_io_t * io, uint8_t * buf, int len);

GAVL_PUBLIC
int gavl_io_write_data(gavl_io_t * io, const uint8_t * buf, int len);

GAVL_PUBLIC
int gavl_io_read_line(gavl_io_t * io, char ** ret, int * ret_alloc, int max_len);

GAVL_PUBLIC
int64_t gavl_io_seek(gavl_io_t * io, int64_t pos, int whence);

GAVL_PUBLIC
int64_t gavl_io_total_bytes(gavl_io_t * io);

GAVL_PUBLIC
const char * gavl_io_filename(gavl_io_t * io);

GAVL_PUBLIC
const char * gavl_io_mimetype(gavl_io_t * io);

GAVL_PUBLIC
void gavl_io_set_info(gavl_io_t * io, int64_t total_bytes,
                      const char * filename, const char * mimetype, int flags);

GAVL_PUBLIC
gavl_dictionary_t * gavl_io_info(gavl_io_t * io);


GAVL_PUBLIC
int64_t gavl_io_position(gavl_io_t * io);



/* Reset position to zero. This is necessary for socket based
   io to subtract the handshake headers from the file offsets */

GAVL_PUBLIC
void gavl_io_reset_position(gavl_io_t * io);

GAVL_PUBLIC
int gavl_io_write_uint64f(gavl_io_t * io, uint64_t num);

GAVL_PUBLIC
int gavl_io_read_uint64f(gavl_io_t * io, uint64_t * num);

GAVL_PUBLIC
int gavl_io_write_int64f(gavl_io_t * io, int64_t num);

GAVL_PUBLIC
int gavl_io_read_int64f(gavl_io_t * io, int64_t * num);

GAVL_PUBLIC
int gavl_io_write_uint64v(gavl_io_t * io, uint64_t num);

GAVL_PUBLIC
int gavl_io_read_uint64v(gavl_io_t * io, uint64_t * num);

GAVL_PUBLIC
int gavl_io_write_uint32v(gavl_io_t * io, uint32_t num);

GAVL_PUBLIC
int gavl_io_read_uint32v(gavl_io_t * io, uint32_t * num);

GAVL_PUBLIC
int gavl_io_write_int64v(gavl_io_t * io, int64_t num);

GAVL_PUBLIC
int gavl_io_read_int64v(gavl_io_t * io, int64_t * num);

GAVL_PUBLIC
int gavl_io_write_int32v(gavl_io_t * io, int32_t num);

GAVL_PUBLIC
int gavl_io_read_int32v(gavl_io_t * io, int32_t * num);

GAVL_PUBLIC
int gavl_io_read_string(gavl_io_t * io, char **);

GAVL_PUBLIC
int gavl_io_write_string(gavl_io_t * io, const char * );

GAVL_PUBLIC
int gavl_io_read_buffer(gavl_io_t * io, gavl_buffer_t * ret);

GAVL_PUBLIC
int gavl_io_write_buffer(gavl_io_t * io,  const gavl_buffer_t * buf);

GAVL_PUBLIC
int gavl_io_read_float(gavl_io_t * io, float * num);

GAVL_PUBLIC
int gavl_io_write_float(gavl_io_t * io, float num);

GAVL_PUBLIC
int gavl_io_read_double(gavl_io_t * io, double * num);

GAVL_PUBLIC
int gavl_io_write_double(gavl_io_t * io, double num);

GAVL_PUBLIC
int gavl_value_read(gavl_io_t * io, gavl_value_t * v);

GAVL_PUBLIC
int gavl_value_write(gavl_io_t * io, const gavl_value_t * v);

GAVL_PUBLIC
int gavl_dictionary_write(gavl_io_t * io, const gavl_dictionary_t * dict);

GAVL_PUBLIC
int gavl_dictionary_read(gavl_io_t * io, gavl_dictionary_t * dict);

/* Read/write fixed length integers */

GAVL_PUBLIC
int gavl_io_read_8(gavl_io_t * ctx, uint8_t * ret);

GAVL_PUBLIC
int gavl_io_read_16_le(gavl_io_t * ctx,uint16_t * ret);

GAVL_PUBLIC
int gavl_io_read_32_le(gavl_io_t * ctx,uint32_t * ret);

GAVL_PUBLIC
int gavl_io_read_24_le(gavl_io_t * ctx,uint32_t * ret);

GAVL_PUBLIC
int gavl_io_read_64_le(gavl_io_t * ctx,uint64_t * ret);

GAVL_PUBLIC
int gavl_io_read_16_be(gavl_io_t * ctx,uint16_t * ret);

GAVL_PUBLIC
int gavl_io_read_24_be(gavl_io_t * ctx,uint32_t * ret);

GAVL_PUBLIC
int gavl_io_read_32_be(gavl_io_t * ctx,uint32_t * ret);

GAVL_PUBLIC
int gavl_io_read_64_be(gavl_io_t * ctx, uint64_t * ret);

/* Write */

GAVL_PUBLIC
int gavl_io_write_8(gavl_io_t * ctx, uint8_t val);

GAVL_PUBLIC
int gavl_io_write_16_le(gavl_io_t * ctx, uint16_t val);

GAVL_PUBLIC
int gavl_io_write_32_le(gavl_io_t * ctx,uint32_t val);

GAVL_PUBLIC
int gavl_io_write_24_le(gavl_io_t * ctx,uint32_t val);

GAVL_PUBLIC
int gavl_io_write_64_le(gavl_io_t * ctx,uint64_t val);

GAVL_PUBLIC
int gavl_io_write_16_be(gavl_io_t * ctx,uint16_t val);

GAVL_PUBLIC
int gavl_io_write_24_be(gavl_io_t * ctx,uint32_t val);

GAVL_PUBLIC
int gavl_io_write_32_be(gavl_io_t * ctx,uint32_t val);

GAVL_PUBLIC
int gavl_io_write_64_be(gavl_io_t * ctx, uint64_t val);

/** \brief Read a message
 *  \param ret Where the message will be copied
 *  \param io I/O context
 *  \returns 1 on success, 0 on error
 */

GAVL_PUBLIC
int gavl_msg_read(gavl_msg_t * ret, gavl_io_t * io);

/** \brief Write a message
 *  \param msg A message
 *  \param io I/O context
 *  \returns 1 on success, 0 on error
 */

GAVL_PUBLIC
int gavl_msg_write(const gavl_msg_t * msg, gavl_io_t * io);

GAVL_PUBLIC
uint8_t * gavl_msg_to_buffer(int * len, const gavl_msg_t * msg);

GAVL_PUBLIC
int gavl_msg_from_buffer(const uint8_t * buf, int len, gavl_msg_t * msg);

GAVL_PUBLIC
int gavl_msg_to_packet(const gavl_msg_t * msg,
                       gavl_packet_t * dst);

GAVL_PUBLIC
int gavl_packet_to_msg(const gavl_packet_t * src,
                       gavl_msg_t * msg);
  

typedef struct
  {
  char eightcc[9];
  int64_t start; // gavl_io_position();
  int64_t len;   // gavl_io_position() - start;
  } gavl_chunk_t;

GAVL_PUBLIC
int gavl_chunk_read_header(gavl_io_t * io, gavl_chunk_t * head);

GAVL_PUBLIC
int gavl_chunk_is(const gavl_chunk_t * head, const char * eightcc);

GAVL_PUBLIC
int gavl_chunk_start(gavl_io_t * io, gavl_chunk_t * head, const char * eightcc);

GAVL_PUBLIC
int gavl_chunk_finish(gavl_io_t * io, gavl_chunk_t * head, int write_size);

GAVL_PUBLIC
gavl_io_t * gavl_chunk_start_io(gavl_io_t * io, gavl_chunk_t * head, const char * eightcc);

GAVL_PUBLIC
int gavl_chunk_finish_io(gavl_io_t * io, gavl_chunk_t * head, gavl_io_t * sub_io);

GAVL_PUBLIC
int gavl_dictionary_from_buffer(const uint8_t * buf, int len, gavl_dictionary_t * fmt);

GAVL_PUBLIC
uint8_t * gavl_dictionary_to_buffer(int * len, const gavl_dictionary_t * fmt);
  
#ifdef __cplusplus
}
#endif
 

#endif // GAVL_IO_H_INCLUDED
