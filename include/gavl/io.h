/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2012 Members of the Gmerlin project
 * gmerlin-general@lists.sourceforge.net
 * http://gmerlin.sourceforge.net
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

typedef int (*gavf_read_func)(void * priv, uint8_t * data, int len);
typedef int (*gavf_write_func)(void * priv, const uint8_t * data, int len);
typedef int64_t (*gavf_seek_func)(void * priv, int64_t pos, int whence);
typedef void (*gavf_close_func)(void * priv);
typedef int (*gavf_flush_func)(void * priv);

typedef int (*gavf_poll_func)(void * priv, int timeout, int wr);



// typedef int (*gavl_io_cb_func)(void * priv, int type, const void * data);

typedef struct gavl_io_s gavl_io_t;

GAVL_PUBLIC
void gavl_io_set_poll_func(gavl_io_t * io, gavf_poll_func f);

GAVL_PUBLIC
int gavl_io_can_read(gavl_io_t * io, int timeout);

GAVL_PUBLIC
int gavl_io_can_write(gavl_io_t * io, int timeout);

GAVL_PUBLIC
gavl_io_t * gavl_io_create(gavf_read_func  r,
                           gavf_write_func w,
                           gavf_seek_func  s,
                           gavf_close_func c,
                           gavf_flush_func f,
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
gavl_io_t * gavl_io_create_file(FILE * f, int wr, int can_seek, int do_close);

#define GAVF_IO_SOCKET_DO_CLOSE     (1<<0)

#define GAVF_IO_SOCKET_BUFFER_READ  (1<<1)
// #define GAVF_IO_SOCKET_BUFFER_WRITE (1<<2)


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

#if 0
GAVL_PUBLIC
gavl_io_t * gavl_io_create_cipher_read(gavl_cipher_algo_t algo,
                                       gavl_cipher_mode_t mode,
                                       gavl_cipher_padding_t padding);
GAVL_PUBLIC
void gavl_io_create_cipher_init(gavl_io_t * io,
                                gavl_io_t * src,
                                const uint8_t * key,
                                const uint8_t * iv);


GAVL_PUBLIC
gavl_io_t * gavl_io_create_cipher_write(gavl_io_t * base,
                                       gavl_cipher_algo_t algo,
                                       gavl_cipher_mode_t mode,
                                       gavl_cipher_padding_t padding);
#endif
                                       


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


  
#ifdef __cplusplus
}
#endif
 

#endif // GAVL_IO_H_INCLUDED
