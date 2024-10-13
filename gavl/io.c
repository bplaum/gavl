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


#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/numptr.h>
#include <gavl/value.h>
#include <gavl/metatags.h>
#include <gavl/log.h>
#define LOG_DOMAIN "io"

#include <io.h>
#include <io_private.h>


// #define DUMP_MSG_WRITE
// #define DUMP_MSG_READ

void gavl_io_init(gavl_io_t * ret,
                  gavl_read_func  r,
                  gavl_write_func w,
                  gavl_seek_func  s,
                  gavl_close_func c,
                  gavl_flush_func f,
                  int flags,
                  void * priv)
  {
  memset(ret, 0, sizeof(*ret));
  ret->read_func = r;
  ret->write_func = w;
  ret->seek_func = s;
  ret->close_func = c;
  ret->flush_func = f;
  ret->priv = priv;
  ret->flags = flags;
  }

void gavl_io_set_nonblock_read(gavl_io_t * io, gavl_read_func read_nonblock)
  {
  io->read_func_nonblock = read_nonblock;
  }

void gavl_io_set_nonblock_write(gavl_io_t * io, gavl_write_func write_nonblock)
  {
  io->write_func_nonblock = write_nonblock;
  }

void * gavl_io_get_priv(gavl_io_t * io)
  {
  return io->priv;
  }

int gavl_io_can_seek(gavl_io_t * io)
  {
  return (io->seek_func && (io->flags & GAVL_IO_CAN_SEEK)) ? 1 : 0;
  }

int gavl_io_can_read(gavl_io_t * io, int timeout)
  {
  if(io->get_buf.len > 0)
    return 1;

  if(gavl_io_got_eof(io))
    return 1;
  
  if(io->poll_func)
    return io->poll_func(io->priv, timeout, 0);
  else
    return 1;
  }

int gavl_io_can_write(gavl_io_t * io, int timeout)
  {
  if(io->poll_func)
    return io->poll_func(io->priv, timeout, 1);
  else
    return 1;
  }



gavl_io_t * gavl_io_create(gavl_read_func  r,
                           gavl_write_func w,
                           gavl_seek_func  s,
                           gavl_close_func c,
                           gavl_flush_func f,
                           int flags,
                           void * priv)
  {
  gavl_io_t * ret;
  ret = malloc(sizeof(*ret));
  if(!ret)
    return NULL;
  gavl_io_init(ret, r, w, s, c, f, flags, priv);
  return ret;
  }

void gavl_io_cleanup(gavl_io_t * io)
  {
  if(io->flush_func)
    io->flush_func(io->priv);
  if(io->close_func)
    io->close_func(io->priv);

  gavl_dictionary_free(&io->info);
  
  gavl_buffer_free(&io->get_buf);
  }

void gavl_io_destroy(gavl_io_t * io)
  {
  gavl_io_cleanup(io);
  free(io);
  }

void gavl_io_set_info(gavl_io_t * io, int64_t total_bytes,
                      const char * filename, const char * mimetype, int flags)
  {
  if(total_bytes > 0)
    io->total_bytes = total_bytes;

  gavl_dictionary_set_string(&io->info, GAVL_META_URI, filename);
  gavl_dictionary_set_string(&io->info, GAVL_META_MIMETYPE, mimetype);
  
  io->position = 0;

  io->flags &= ~(GAVL_IO_ERROR|GAVL_IO_EOF|
                 GAVL_IO_CAN_SEEK|
                 GAVL_IO_CAN_READ|
                 GAVL_IO_CAN_WRITE);

  io->flags |= (flags & (GAVL_IO_CAN_SEEK|
                         GAVL_IO_CAN_READ|
                         GAVL_IO_CAN_WRITE));
  
  //  fprintf(stderr, "gavl_io_set_info %s\n", filename);
  
  }

void gavl_io_set_poll_func(gavl_io_t * io, gavl_poll_func f)
  {
  io->poll_func = f;
  }


int64_t gavl_io_total_bytes(gavl_io_t * io)
  {
  return io->total_bytes;
  }

const char * gavl_io_filename(gavl_io_t * io)
  {
  return gavl_dictionary_get_string(&io->info, GAVL_META_URI);
  }

const char * gavl_io_mimetype(gavl_io_t * io)
  {
  return gavl_dictionary_get_string(&io->info, GAVL_META_MIMETYPE);
  }

gavl_dictionary_t * gavl_io_info(gavl_io_t * io)
  {
  return &io->info;
  }

int gavl_io_flush(gavl_io_t * io)
  {
  int ret = 1;
  if(gavl_io_got_error(io))
    return 0;
  
  if(io->flush_func)
    ret = io->flush_func(io->priv);
  if(!ret)
    gavl_io_set_error(io);
  return ret;
  }

int gavl_io_got_error(gavl_io_t * io)
  {
  return io->flags & GAVL_IO_ERROR;
  }

int gavl_io_got_eof(gavl_io_t * io)
  {
  return io->flags & GAVL_IO_EOF;
  }

void gavl_io_set_error(gavl_io_t * io)
  {
  io->flags |= GAVL_IO_ERROR;
  }

void gavl_io_clear_error(gavl_io_t * io)
  {
  io->flags &= ~GAVL_IO_ERROR;
  }

void gavl_io_set_eof(gavl_io_t * io)
  {
  io->flags |= GAVL_IO_EOF;
  }

void gavl_io_clear_eof(gavl_io_t * io)
  {
  io->flags &= ~GAVL_IO_EOF;
  }

int64_t gavl_io_position(gavl_io_t * io)
  {
  return io->position;
  }

void gavl_io_reset_position(gavl_io_t * io)
  {
  io->position = 0;
  }


static int io_read_data(gavl_io_t * io, uint8_t * buf, int len, int block)
  {
  int ret = 0;
  int result;
  int num_get = 0;

  if(!io->read_func)
    return 0;
  
  if(gavl_io_got_eof(io))
    return 0;
  
  if(io->get_buf.len > 0)
    {
    num_get = io->get_buf.len > len ? len : io->get_buf.len;
    memcpy(buf, io->get_buf.buf, num_get);

    if(io->get_buf.len > num_get)
      memmove(io->get_buf.buf, io->get_buf.buf + num_get, io->get_buf.len - num_get);
    
    io->get_buf.len -= num_get;
    
    buf += num_get;
    len -= num_get;
    io->position += num_get;
    ret = num_get;
    }

  if(len > 0)
    {
    if(!block && io->read_func_nonblock)
      result = io->read_func_nonblock(io->priv, buf, len);
    else
      {
      result = io->read_func(io->priv, buf, len);
      if(result < len)
        gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Requested %d bytes, got %d", len, result);
      }
    
    if(result < 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Read returned %d (block: %d)", result, block);
      gavl_io_set_error(io);
      }
    if(result > 0)
      {
      io->position += result;
      ret += result;
      }
    
    if(((io->total_bytes > 0) && (io->position == io->total_bytes)) ||
       (!result && block))
      gavl_io_set_eof(io);
    }
  
  return ret;
  }

int gavl_io_read_data(gavl_io_t * io, uint8_t * buf, int len)
  {
  if(io->flags & (GAVL_IO_ERROR|GAVL_IO_EOF))
    return 0;
  return io_read_data(io, buf, len, 1);
  }

int gavl_io_read_data_nonblock(gavl_io_t * io, uint8_t * buf, int len)
  {
  if(io->flags & (GAVL_IO_ERROR|GAVL_IO_EOF))
    return 0;
  return io_read_data(io, buf, len, 0);
  }



void gavl_io_unread_data(gavl_io_t * io, const uint8_t * buf, int len)
  {
  gavl_buffer_prepend_data(&io->get_buf, buf, len);
  io->position -= len;
  }


int gavl_io_get_data(gavl_io_t * io, uint8_t * buf, int len)
  {
  int ret;
  if(!io->read_func)
    return 0;

  if(io->get_buf.len < len)
    {
    gavl_buffer_alloc(&io->get_buf, len);

    ret = io->read_func(io->priv, io->get_buf.buf + io->get_buf.len, len - io->get_buf.len);

    if(ret > 0)
      io->get_buf.len += ret;
    else
      return ret;
    }

  /* Unlikely */
  if(len > io->get_buf.len)
    len = io->get_buf.len;
  
  memcpy(buf, io->get_buf.buf, len);
  return len;
  }


int gavl_io_write_data(gavl_io_t * io, const uint8_t * buf, int len)
  {
  int ret;
  if(gavl_io_got_error(io))
    return -1;

  if(!io->write_func)
    return 0;
  ret = io->write_func(io->priv, buf, len);
  if(ret > 0)
    io->position += ret;

  if(ret < len)
    gavl_io_set_error(io);
  return ret;
  }

int gavl_io_write_data_nonblock(gavl_io_t * io, const uint8_t * buf, int len)
  {
  int ret;
  if(gavl_io_got_error(io))
    return -1;

  if(io->write_func_nonblock)
    ret = io->write_func_nonblock(io->priv, buf, len);
  else if(io->write_func)
    ret = io->write_func(io->priv, buf, len);
  else
    ret = 0;

#if 0
  if(!ret)
    {
    fprintf(stderr, "gavl_io_write_data_nonblock: write returned null %p %p\n",
            io->write_func_nonblock, io->write_func);
    } 
#endif

  if(ret > 0)
    io->position += ret;
  
  return ret;
  }



void gavl_io_skip(gavl_io_t * io, int bytes)
  {
  if(io->get_buf.len > 0)
    {
    int num_skip = io->get_buf.len > bytes ? bytes : io->get_buf.len;
    
    if(io->get_buf.len > num_skip)
      memmove(io->get_buf.buf, io->get_buf.buf + num_skip, io->get_buf.len - num_skip);
    
    io->get_buf.len -= num_skip;
    
    bytes -= num_skip;
    io->position += num_skip;
    
    if(!bytes)
      return;
    }
  
  
  if(io->seek_func)
    io->position = io->seek_func(io->priv, bytes, SEEK_CUR);
  else
    {
    int i;
    uint8_t c;
    for(i = 0; i < bytes; i++)
      {
      if(gavl_io_read_data(io, &c, 1) < 1)
        break;
      io->position++;
      }
    }
  }

int64_t gavl_io_seek(gavl_io_t * io, int64_t pos, int whence)
  {
  if(!io->seek_func)
    return -1;
  io->position = io->seek_func(io->priv, pos, whence);
  io->flags &= ~GAVL_IO_EOF;
  return io->position;
  }


/* Fixed size integers (BE) */

int gavl_io_write_uint64f(gavl_io_t * io, uint64_t num)
 {
  uint8_t buf[8];
  GAVL_64BE_2_PTR(num, buf);
  return (gavl_io_write_data(io, buf, 8) < 8) ? 0 : 1;
  }

int gavl_io_read_uint64f(gavl_io_t * io, uint64_t * num)
  {
  uint8_t buf[8];
  if(gavl_io_read_data(io, buf, 8) < 8)
    return 0;
  *num = GAVL_PTR_2_64BE(buf);
  return 1;
  }

int gavl_io_write_int64f(gavl_io_t * io, int64_t num)
  {
  uint8_t buf[8];
  GAVL_64BE_2_PTR(num, buf);
  return (gavl_io_write_data(io, buf, 8) < 8) ? 0 : 1;
  }

int gavl_io_read_int64f(gavl_io_t * io, int64_t * num)
  {
  uint8_t buf[8];
  if(gavl_io_read_data(io, buf, 8) < 8)
    return 0;
  *num = GAVL_PTR_2_64BE(buf);
  return 1;
  }

static int write_uint32f(gavl_io_t * io, uint32_t num)
  {
  uint8_t buf[4];

  GAVL_32BE_2_PTR(num, buf);

  //  fprintf(stderr, "write_uint32f\n");
  //  gavl_hexdump(buf, 4, 4);

  return (gavl_io_write_data(io, buf, 4) < 4) ? 0 : 1;
  }

static int read_uint32f(gavl_io_t * io, uint32_t * num)
  {
  uint8_t buf[4];
  if(gavl_io_read_data(io, buf, 4) < 4)
    return 0;

  //  fprintf(stderr, "read_uint32f\n");
  //  gavl_hexdump(buf, 4, 4);

  *num = GAVL_PTR_2_32BE(buf);
  return 1;
  }


/*
 *  Integer formats:
 *
 *  7 bit (0..127, -64..63)
 *  1xxx xxxx
 *
 *  14 bit (0..16383, -8192..8193)
 *  01xx xxxx xxxx xxxx
 *
 *  21 bit (0..2097151, -1048576..1048575)
 *  001x xxxx xxxx xxxx xxxx xxxx
 *
 *  28 bit (0..268435455, -134217728..134217727)
 *  0001 xxxx xxxx xxxx xxxx xxxx xxxx xxxx
 *
 *  35 bit
 *  0000 1xxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
 *
 *  42 bit
 *  0000 01xx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
 *
 *  49 bit
 *  0000 001x xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
 *
 *  56 bit
 *  0000 0001 xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
 *
 *  64 bit
 *  0000 0000 xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
 */

static const int64_t int_offsets[] =
  {
    ((uint64_t)1)<<6,
    ((uint64_t)1)<<13,
    ((uint64_t)1)<<20,
    ((uint64_t)1)<<27,
    ((uint64_t)1)<<34,
    ((uint64_t)1)<<41,
    ((uint64_t)1)<<48,
    ((uint64_t)1)<<55,
  };

static const uint64_t uint_limits[] =
  {
    ((uint64_t)1)<<7,  // 128
    ((uint64_t)1)<<14, // 16384
    ((uint64_t)1)<<21, // 2097152
    ((uint64_t)1)<<28, // 268435456
    ((uint64_t)1)<<35, // 34359738368
    ((uint64_t)1)<<42, // 4398046511104
    ((uint64_t)1)<<49, // 562949953421312
    ((uint64_t)1)<<56, // 72057594037927936
  };

static int get_len_uint(uint64_t num)
  {
  int i;
  
  for(i = 0; i < 8; i++)
    {
    if(num < uint_limits[i])
      return i+1;
    }
  return 9;
  }

static int get_len_int(int64_t num)
  {
  int i;
  for(i = 0; i < 8; i++)
    {
    if((num < int_offsets[i]) && (num >= -int_offsets[i]))
      return i+1;
    }
  return 9;
  }

static int get_len_read(uint8_t num)
  {
  int i;
  for(i = 0; i < 8; i++)
    {
    if(num & (0x80 >> i))
      return i+1;
    }
  return 9;
  }

static int write_uint64v(gavl_io_t * io, uint64_t num, int len)
  {
  int idx;
  uint8_t buf[9];

  /* Length byte */
  buf[0] = 0x80 >> (len-1);

  idx = len - 1;

  while(idx)
    {
    buf[idx] = num & 0xff;
    num >>= 8;
    idx--;
    }

  if(len < 8)
    buf[0] |= (num & (0xff >> len));
  
  return (gavl_io_write_data(io, buf, len) < len) ? 0 : 1;
  }

int gavl_io_write_uint64v(gavl_io_t * io, uint64_t num)
  {
  return write_uint64v(io, num, get_len_uint(num));
  }

int gavl_io_write_int64v(gavl_io_t * io, int64_t num)
  {
  uint64_t num_u;
  int len = get_len_int(num);
  
  if(len == 9)
    num_u = num ^ 0x8000000000000000LL;
  else
    num_u = num + int_offsets[len-1];
  return write_uint64v(io, num_u, len);
  }

static int read_uint64v(gavl_io_t * io, uint64_t * num, int * len)
  {
  int i;
  int len1;
  uint8_t buf[9];

  if(!gavl_io_read_data(io, buf, 1))
    return 0;
  len1 = get_len_read(buf[0]);
  
  if(len1 > 1)
    {
    if(gavl_io_read_data(io, &buf[1], len1 - 1) < len1 - 1)
      return 0;
    }
  
  *num = buf[0] & (0xff >> len1);

  for(i = 1; i < len1; i++)
    {
    *num <<= 8;
    *num |= buf[i];
    }
  if(len)
    *len = len1;
  return 1;
  }
  
int gavl_io_read_int64v(gavl_io_t * io, int64_t * num)
  {
  int len;
  if(!read_uint64v(io, (uint64_t*)num, &len))
    return 0;

  if(len == 9)
    *num ^= 0x8000000000000000LL;
  else
    *num -= int_offsets[len-1];
  
  return 1;
  }


int gavl_io_read_uint64v(gavl_io_t * io, uint64_t * num)
  {
  return read_uint64v(io, num, NULL);
  }

int gavl_io_write_uint32v(gavl_io_t * io, uint32_t num)
  {
  return gavl_io_write_uint64v(io, num);
  }

int gavl_io_read_uint32v(gavl_io_t * io, uint32_t * num)
  {
  uint64_t ret;

  if(!gavl_io_read_uint64v(io, &ret))
    return 0;
  *num = ret;
  return 1;
  }

int gavl_io_write_int32v(gavl_io_t * io, int32_t num)
  {
  return gavl_io_write_int64v(io, num);
  }

int gavl_io_read_int32v(gavl_io_t * io, int32_t * num)
  {
  int64_t ret = 0;
  if(!gavl_io_read_int64v(io, &ret))
    return 0;
  *num = ret;
  return 1;
  }

/* Fixed size LE/BE I/O */

int gavl_io_read_8(gavl_io_t * ctx, uint8_t * ret)
  {
  if(gavl_io_read_data(ctx, ret, 1) < 1)
    return 0;
  return 1;
  }

int gavl_io_read_16_le(gavl_io_t * ctx,uint16_t * ret)
  {
  uint8_t data[2];
  if(gavl_io_read_data(ctx, data, 2) < 2)
    return 0;
  *ret = GAVL_PTR_2_16LE(data);
  
  return 1;
  }

int gavl_io_read_32_le(gavl_io_t * ctx,uint32_t * ret)
  {
  uint8_t data[4];
  if(gavl_io_read_data(ctx, data, 4) < 4)
    return 0;
  *ret = GAVL_PTR_2_32LE(data);
  return 1;
  }

int gavl_io_read_24_le(gavl_io_t * ctx,uint32_t * ret)
  {
  uint8_t data[3];
  if(gavl_io_read_data(ctx, data, 3) < 3)
    return 0;
  *ret = GAVL_PTR_2_24LE(data);
  return 1;
  }

int gavl_io_read_64_le(gavl_io_t * ctx,uint64_t * ret)
  {
  uint8_t data[8];
  if(gavl_io_read_data(ctx, data, 8) < 8)
    return 0;
  *ret = GAVL_PTR_2_64LE(data);
  return 1;
  }

int gavl_io_read_16_be(gavl_io_t * ctx,uint16_t * ret)
  {
  uint8_t data[2];
  if(gavl_io_read_data(ctx, data, 2) < 2)
    return 0;

  *ret = GAVL_PTR_2_16BE(data);
  return 1;
  }

int gavl_io_read_24_be(gavl_io_t * ctx,uint32_t * ret)
  {
  uint8_t data[3];
  if(gavl_io_read_data(ctx, data, 3) < 3)
    return 0;
  *ret = GAVL_PTR_2_24BE(data);
  return 1;
  }


int gavl_io_read_32_be(gavl_io_t * ctx,uint32_t * ret)
  {
  uint8_t data[4];
  if(gavl_io_read_data(ctx, data, 4) < 4)
    return 0;

  *ret = GAVL_PTR_2_32BE(data);
  return 1;
  }
    
int gavl_io_read_64_be(gavl_io_t * ctx, uint64_t * ret)
  {
  uint8_t data[8];
  if(gavl_io_read_data(ctx, data, 8) < 8)
    return 0;
  
  *ret = GAVL_PTR_2_64BE(data);
  return 1;
  }

/* Write */

int gavl_io_write_8(gavl_io_t * ctx, uint8_t val)
  {
  if(gavl_io_write_data(ctx, &val, 1) < 1)
    return 0;
  return 1;
  }

int gavl_io_write_16_le(gavl_io_t * ctx, uint16_t val)
  {
  uint8_t data[2];
  GAVL_16LE_2_PTR(val, data);

  if(gavl_io_write_data(ctx, data, 2) < 2)
    return 0;

  return 1;
  }

int gavl_io_write_32_le(gavl_io_t * ctx,uint32_t val)
  {
  uint8_t data[4];
  GAVL_32LE_2_PTR(val, data);


  if(gavl_io_write_data(ctx, data, 4) < 4)
    return 0;
  return 1;
  }

int gavl_io_write_24_le(gavl_io_t * ctx,uint32_t val)
  {
  uint8_t data[3];
  GAVL_24LE_2_PTR(val, data);

  if(gavl_io_write_data(ctx, data, 3) < 3)
    return 0;
  return 1;
  }

int gavl_io_write_64_le(gavl_io_t * ctx,uint64_t val)
  {
  uint8_t data[8];
  GAVL_64LE_2_PTR(val, data);
  if(gavl_io_write_data(ctx, data, 8) < 8)
    return 0;
  return 1;
  }

int gavl_io_write_16_be(gavl_io_t * ctx,uint16_t val)
  {
  uint8_t data[2];
  GAVL_16BE_2_PTR(val, data);
  if(gavl_io_write_data(ctx, data, 2) < 2)
    return 0;
  return 1;
  }

int gavl_io_write_24_be(gavl_io_t * ctx,uint32_t val)
  {
  uint8_t data[3];
  GAVL_24BE_2_PTR(val, data);
  if(gavl_io_write_data(ctx, data, 3) < 3)
    return 0;
  return 1;
  }


int gavl_io_write_32_be(gavl_io_t * ctx,uint32_t val)
  {
  uint8_t data[4];
  GAVL_32BE_2_PTR(val, data);
  if(gavl_io_write_data(ctx, data, 4) < 4)
    return 0;
  return 1;
  }
    
int gavl_io_write_64_be(gavl_io_t * ctx, uint64_t val)
  {
  uint8_t data[8];
  GAVL_64BE_2_PTR(val, data);
  if(gavl_io_write_data(ctx, data, 8) < 8)
    return 0;
  return 1;
  }


/* int <-> float conversion routines taken from
   ffmpeg */

typedef union {
    uint32_t i;
    float    f;
} intfloat32_t;

typedef union  {
    uint64_t i;
    double   f;
} intfloat64_t;


static float int2flt(uint32_t i)
  {
  intfloat32_t v;
  v.i = i;
  return v.f;
  }

static uint32_t flt2int(float f)
  {
  intfloat32_t v;
  v.f = f;
  return v.i;
  }

static double int2dbl(uint64_t i)
  {
  intfloat64_t v;
  v.i = i;
  return v.f;
  }

static uint64_t dbl2int(double f)
  {
  intfloat64_t v;
  v.f = f;
  return v.i;
  }

int gavl_io_read_float(gavl_io_t * io, float * num)
  {
  uint32_t val;
  if(!read_uint32f(io, &val))
    return 0;
  *num = int2flt(val);
  return 1;
  }

int gavl_io_write_float(gavl_io_t * io, float num)
  {
  uint32_t val = flt2int(num);
  return write_uint32f(io, val);
  }

int gavl_io_read_double(gavl_io_t * io, double * num)
  {
  uint64_t val;
  if(!gavl_io_read_uint64f(io, &val))
    return 0;
  *num = int2dbl(val);
  return 1;
  }

int gavl_io_write_double(gavl_io_t * io, double num)
  {
  uint64_t val = dbl2int(num);
  return gavl_io_write_uint64f(io, val);
  }


int gavl_io_read_string(gavl_io_t * io, char ** ret)
  {
  uint32_t len = 0;
  
  if(!gavl_io_read_uint32v(io, &len))
    return 0;

  if(!len)
    {
    *ret = NULL;
    return 1;
    }
  if(!ret)
    return 0;

  *ret = malloc(len + 1);
  if(gavl_io_read_data(io, (uint8_t*)(*ret), len) < len)
    return 0;

  /* Zero terminate */
  (*ret)[len] = '\0';
  return 1;
  }

int gavl_io_write_string(gavl_io_t * io, const char * str)
  {
  uint32_t len;

  if(!str)
    return gavl_io_write_uint32v(io, 0);
  
  len = strlen(str);
  if(!gavl_io_write_uint32v(io, len) ||
     gavl_io_write_data(io, (const uint8_t*)str, len) < len)
    return 0;
  return 1;
  }

int gavl_io_read_buffer(gavl_io_t * io, gavl_buffer_t * ret)
  {
  uint32_t len;
  
  if(!gavl_io_read_uint32v(io, &len))
    return 0;

  if(!len)
    {
    gavl_buffer_free(ret);
    gavl_buffer_init(ret);
    }
  else if(!gavl_buffer_alloc(ret, len) ||
          (gavl_io_read_data(io, ret->buf, len) < len))
    return 0;
  ret->len = len;
  return 1;
  }

int gavl_io_write_buffer(gavl_io_t * io, const gavl_buffer_t * buf)
  {
  if(!gavl_io_write_uint32v(io, buf->len) ||
     (gavl_io_write_data(io, buf->buf, buf->len) < buf->len))
    return 0;
  return 1;
  }



/* */


int gavl_msg_read(gavl_msg_t * ret, gavl_io_t * io)
  {
  int i;
  
  int32_t val_i;

  memset(ret, 0, sizeof(*ret));

  /* Message Namespace */

  gavl_dictionary_read(io, &ret->header);
  
  //  fprintf(stderr, "Got ID: %d\n", ret->id);
  
  /* Number of arguments */

  if(!gavl_io_read_int32v(io, &val_i))
    return 0;
  
  ret->num_args = val_i;

  for(i = 0; i < ret->num_args; i++)
    gavl_value_read(io, &ret->args[i]);

  gavl_msg_apply_header(ret);
  
#ifdef DUMP_MSG_READ
  fprintf(stderr, "read message:\n");
  gavl_msg_dump(ret, 1);
#endif
  return 1;
  }

int gavl_msg_write(const gavl_msg_t * msg, gavl_io_t * io)
  {
  int i;

#ifdef DUMP_MSG_WRITE
  fprintf(stderr, "writing message:\n");
  gavl_msg_dump(msg, 1);
#endif
  
  gavl_dictionary_write(io, &msg->header);
  
  /* Number of arguments */

  if(!gavl_io_write_int32v(io, msg->num_args))
    return 0;
  
  /* Arguments */

  for(i = 0; i < msg->num_args; i++)
    gavl_value_write(io, &msg->args[i]);
  
  return 1;
  }

/* */

int gavl_value_read(gavl_io_t * io, gavl_value_t * v)
  {
  gavl_type_t gavl_type;

  if(!gavl_io_read_int32v(io, (int32_t*)&gavl_type))
    return 0;

  gavl_value_set_type(v, gavl_type);
  
  switch(v->type)
    {
    case GAVL_TYPE_UNDEFINED:
      break;
    case GAVL_TYPE_INT:
      if(!gavl_io_read_int32v(io, &v->v.i))
        return 0;
      break;
    case GAVL_TYPE_LONG:
      if(!gavl_io_read_int64v(io, &v->v.l))
        return 0;
      break;
    case GAVL_TYPE_FLOAT:
      if(!gavl_io_read_double(io, &v->v.d))
        return 0;
      break;
    case GAVL_TYPE_STRING:
      if(!gavl_io_read_string(io, &v->v.str))
        return 0;
      break;
    case GAVL_TYPE_BINARY:
      {
      if(!gavl_io_read_buffer(io, v->v.buffer))
        return 0;
      }
      break;
    case GAVL_TYPE_AUDIOFORMAT:
      {
      int result;
      gavl_audio_format_t * afmt;
      gavl_dictionary_t dict;
      gavl_dictionary_init(&dict);
      afmt = gavl_value_get_audio_format_nc(v);

      if((result = gavl_dictionary_read(io, &dict)))
        result = gavl_audio_format_from_dictionary(afmt, &dict);
      
      gavl_dictionary_free(&dict);
      return result;
      }
      break;
    case GAVL_TYPE_VIDEOFORMAT:
      {
      int result;
      gavl_video_format_t * vfmt;
      gavl_dictionary_t dict;
      gavl_dictionary_init(&dict);
      vfmt = gavl_value_get_video_format_nc(v);

      if((result = gavl_dictionary_read(io, &dict)))
        result = gavl_video_format_from_dictionary(vfmt, &dict);
      
      gavl_dictionary_free(&dict);
      return result;
      }
      break;
    case GAVL_TYPE_COLOR_RGB:
      if(!gavl_io_read_double(io, &v->v.color[0]) ||
         !gavl_io_read_double(io, &v->v.color[1]) ||
         !gavl_io_read_double(io, &v->v.color[2]))
        return 0;
      break;
    case GAVL_TYPE_COLOR_RGBA:
      if(!gavl_io_read_double(io, &v->v.color[0]) ||
         !gavl_io_read_double(io, &v->v.color[1]) ||
         !gavl_io_read_double(io, &v->v.color[2]) ||
         !gavl_io_read_double(io, &v->v.color[3]))
        return 0;
      break;
    case GAVL_TYPE_POSITION:
      if(!gavl_io_read_double(io, &v->v.position[0]) ||
         !gavl_io_read_double(io, &v->v.position[1]))
        return 0;
      break;
    case GAVL_TYPE_DICTIONARY:
      {
      if(!gavl_dictionary_read(io, v->v.dictionary))
        return 0;
      }
      break;
    case GAVL_TYPE_ARRAY:
      {
      int i;
      if(!gavl_io_read_int32v(io, &v->v.array->num_entries))
        return 0;

      v->v.array->entries_alloc = v->v.array->num_entries;
      v->v.array->entries = calloc(v->v.array->entries_alloc, sizeof(*v->v.array->entries));

      for(i = 0; i < v->v.array->num_entries; i++)
        {
        if(!gavl_value_read(io, &v->v.array->entries[i]))
          return 0;
        }
      break;
      }
      
      break;
    }
  return 1;
  }

int gavl_dictionary_write(gavl_io_t * io, const gavl_dictionary_t * dict)
  {
  int i;
  if(!gavl_io_write_int32v(io, dict->num_entries))
    return 0;
  for(i = 0; i < dict->num_entries; i++)
    {
    if(!gavl_io_write_string(io, dict->entries[i].name) ||
       !gavl_value_write(io, &dict->entries[i].v))
      return 0;
    }
  return 1;
  }

int gavl_dictionary_read(gavl_io_t * io, gavl_dictionary_t * dict)
  {
  int i;
  if(!gavl_io_read_int32v(io, &dict->num_entries))
    return 0;
  
  dict->entries_alloc = dict->num_entries;
  dict->entries = calloc(dict->entries_alloc, sizeof(*dict->entries));
  
  for(i = 0; i < dict->num_entries; i++)
    {
    if(!gavl_io_read_string(io, &dict->entries[i].name) ||
       !gavl_value_read(io, &dict->entries[i].v))
      return 0;
    }
  return 1;
  }

int gavl_value_write(gavl_io_t * io, const gavl_value_t * v)
  {
  if(!gavl_io_write_int32v(io, v->type))
    return 0;
  
  switch(v->type)
    {
    case GAVL_TYPE_UNDEFINED:
      break;
    case GAVL_TYPE_INT:
      if(!gavl_io_write_int32v(io, v->v.i))
        return 0;
      break;
    case GAVL_TYPE_LONG:
      if(!gavl_io_write_int64v(io, v->v.l))
        return 0;
      break;
    case GAVL_TYPE_FLOAT:
      if(!gavl_io_write_double(io, v->v.d))
        return 0;
      break;
    case GAVL_TYPE_STRING:
      if(!gavl_io_write_string(io, v->v.str))
        return 0;
      break;
    case GAVL_TYPE_BINARY:
      if(!gavl_io_write_buffer(io, v->v.buffer))
        return 0;
      break;
    case GAVL_TYPE_AUDIOFORMAT:
      {
      int result;
      gavl_dictionary_t dict;
      gavl_dictionary_init(&dict);
      gavl_audio_format_to_dictionary(v->v.audioformat, &dict);
      result = gavl_dictionary_write(io, &dict);
      gavl_dictionary_free(&dict);
      if(!result)
        return 0;
      }
      break;
    case GAVL_TYPE_VIDEOFORMAT:
      {
      int result;
      gavl_dictionary_t dict;
      gavl_dictionary_init(&dict);
      gavl_video_format_to_dictionary(v->v.videoformat, &dict);
      result = gavl_dictionary_write(io, &dict);
      gavl_dictionary_free(&dict);
      if(!result)
        return 0;
      }
      break;
    case GAVL_TYPE_COLOR_RGB:
      if(!gavl_io_write_double(io, v->v.color[0]) ||
         !gavl_io_write_double(io, v->v.color[1]) ||
         !gavl_io_write_double(io, v->v.color[2]))
        return 0;
      break;
    case GAVL_TYPE_COLOR_RGBA:
      if(!gavl_io_write_double(io, v->v.color[0]) ||
         !gavl_io_write_double(io, v->v.color[1]) ||
         !gavl_io_write_double(io, v->v.color[2]) ||
         !gavl_io_write_double(io, v->v.color[3]))
        return 0;
      break;
    case GAVL_TYPE_POSITION:
      if(!gavl_io_write_double(io, v->v.position[0]) ||
         !gavl_io_write_double(io, v->v.position[1]))
        return 0;
      break;
    case GAVL_TYPE_DICTIONARY:
      if(!gavl_dictionary_write(io, v->v.dictionary))
        return 0;
      break;
    case GAVL_TYPE_ARRAY:
      {
      int i;
      if(!gavl_io_write_int32v(io, v->v.array->num_entries))
        return 0;
      for(i = 0; i < v->v.array->num_entries; i++)
        {
        if(!gavl_value_write(io, &v->v.array->entries[i]))
          return 0;
        }
      }
      break;
    }
  return 1;
  }

uint8_t * gavl_msg_to_buffer(int * len, const gavl_msg_t * msg)
  {
  uint8_t * ret;
  gavl_io_t * io = gavl_io_create_mem_write();
  gavl_msg_write(msg, io);
  ret = gavl_io_mem_get_buf(io, len);
  gavl_io_destroy(io);
  return ret;
  }

int gavl_msg_from_buffer(const uint8_t * buf, int len, gavl_msg_t * msg)
  {
  int result;
  gavl_io_t * io = gavl_io_create_mem_read(buf, len);
  result = gavl_msg_read(msg, io);
  
  //  fprintf(stderr, "bg_msg_from_buffer: %"PRId64"/%d\n", gavl_io_position(io), len);
  
  gavl_io_destroy(io);
  return result;
  }

int gavl_msg_to_packet(const gavl_msg_t * msg,
                       gavl_packet_t * dst)
  {
  dst->pts = GAVL_TIME_UNDEFINED;  
  gavl_dictionary_get_long(&msg->header, GAVL_MSG_HEADER_TIMESTAMP, &dst->pts);
  gavl_packet_free(dst);
  dst->buf.buf = gavl_msg_to_buffer(&dst->buf.len, msg);
  return 1;
  }

int gavl_packet_to_msg(const gavl_packet_t * src,
                       gavl_msg_t * msg)
  {
  gavl_dictionary_set_long(&msg->header, GAVL_MSG_HEADER_TIMESTAMP, src->pts);
  return gavl_msg_from_buffer(src->buf.buf, src->buf.len, msg);
  }


/* Sub I/O */

typedef struct
  {
  gavl_io_t * io;
  int64_t offset;
  int64_t size;
  int64_t pos;
  } sub_io_t;

static int read_sub(void * priv, uint8_t * data, int len)
  {
  int ret;
  sub_io_t * s = priv;

  if((s->size > 0) && (len > s->size - s->pos))
    len = s->size - s->pos;
  
  if(len < 0)
    return 0;
  
  ret = gavl_io_read_data(s->io, data, len);
  s->pos += ret;

  return ret;
  }

static int64_t seek_sub(void * priv, int64_t pos, int whence)
  {
  sub_io_t * s = priv;

  switch(whence)
    {
    case SEEK_SET:
      s->pos = pos;
      break;
    case SEEK_END:
      s->pos = s->size - pos;
      break;
    case SEEK_CUR:
      s->pos += pos;
      break;
    }

  if(s->pos < 0)
    s->pos = 0;
  if(s->pos > s->size)
    s->pos = s->size;
  
  s->pos = gavl_io_seek(s->io, s->offset + s->pos, SEEK_SET) - s->offset;
  return s->pos;
  }

static void close_sub(void * priv)
  {
  free(priv);
  }

gavl_io_t * gavl_io_create_sub_read(gavl_io_t * io, int64_t offset, int64_t len)
  {
  int64_t (*seek_func)(void * priv, int64_t pos, int whence);
  
  gavl_io_t * ret;
  sub_io_t * s = calloc(1, sizeof(*s));

  s->offset = offset;
  s->size = len;
  s->io = io;

  
  if(gavl_io_can_seek(io))
    {
    gavl_io_seek(s->io, s->offset, SEEK_SET);
    seek_func = seek_sub;
    }
  else
    seek_func = NULL;

  ret = gavl_io_create(read_sub,
                       NULL,
                       seek_func,
                       close_sub,
                       NULL,
                       io->flags,
                       s);
  
  return ret;
  }

static int write_sub(void * priv, const uint8_t * data, int len)
  {
  int ret;
  sub_io_t * s = priv;
  ret = gavl_io_write_data(s->io, data, len);
  s->pos += ret;
  return ret;
  }

static int flush_sub(void * priv)
  {
  sub_io_t * s = priv;
  return gavl_io_flush(s->io);
  }

gavl_io_t * gavl_io_create_sub_write(gavl_io_t * io)
  {
  int64_t (*seek_func)(void * priv, int64_t pos, int whence);
  
  gavl_io_t * ret;
  sub_io_t * s = calloc(1, sizeof(*s));

  s->offset = io->position;
  s->size = 0;
  s->io = io;
  
  if(gavl_io_can_seek(io))
    seek_func = seek_sub;
  else
    seek_func = NULL;

  ret = gavl_io_create(NULL,
                       write_sub,
                       seek_func,
                       close_sub,
                       flush_sub,
                       io->flags,
                       s);
  return ret;
  }

int gavl_io_align_write(gavl_io_t * io)
  {
  int rest;
  int64_t position;
  uint8_t buf[8] = { 0x00, 0x00, 0x00, 0x00, 
                     0x00, 0x00, 0x00, 0x00 };
  
  position = gavl_io_position(io);

  rest = position % 8;
  
  if(rest)
    {
    rest = 8 - rest;
    
    if(gavl_io_write_data(io, (const uint8_t*)buf, rest) < rest)
      return 0;

    }
  gavl_io_flush(io);
  return 1;
  }

int gavl_io_align_read(gavl_io_t * io)
  {
  int rest;
  int64_t position;
  uint8_t buf[8];
  
  position = gavl_io_position(io);

  rest = position % 8;
  
  if(rest)
    {
    rest = 8 - rest;
    
    if(gavl_io_read_data(io, buf, rest) < rest)
      return 0;
    }
  return 1;
  }

#define BYTES_TO_ALLOC 1024

int gavl_io_read_line(gavl_io_t * io, char ** ret, int * ret_alloc, int max_len)
  {
  char * pos;
  char c;
  int bytes_read;
  bytes_read = 0;
  /* Allocate Memory for the case we have none */
  if(!(*ret_alloc))
    {
    *ret_alloc = BYTES_TO_ALLOC;
    *ret = realloc(*ret, *ret_alloc);
    }
  pos = *ret;
  while(1)
    {
    c = 0;
    if(!gavl_io_read_data(io, (uint8_t*)(&c), 1))
      {
      //  bg_log(BG_LOG_ERROR, LOG_DOMAIN, "Reading line failed: %s", strerror(errno));

      if(!bytes_read)
        {
        return 0;
        }
      break;
      }

    // fprintf(stderr, "%c", c);
    
    if((c > 0x00) && (c < 0x20) && (c != '\r') && (c != '\n'))
      {
      // bg_log(BG_LOG_ERROR, LOG_DOMAIN, "Got non ASCII character (%d) while reading line", c);
      return 0;
      }
    
    /*
     *  Line break sequence
     *  is starting, remove the rest from the stream
     */
    if(c == '\n')
      {
      break;
      }
    /* Reallocate buffer */
    else if((c != '\r') && (c != '\0'))
      {
      if(bytes_read+2 >= *ret_alloc)
        {
        *ret_alloc += BYTES_TO_ALLOC;
        *ret = realloc(*ret, *ret_alloc);
        pos = &((*ret)[bytes_read]);
        }
      /* Put the byte and advance pointer */
      *pos = c;
      pos++;
      bytes_read++;
      }
    }
  *pos = '\0';
  return 1;

  }

int gavl_io_get_flags(gavl_io_t * io)
  {
  return io->flags;
  }

/* Chunk I/O */

int gavl_chunk_is(const gavl_chunk_t * head, const char * eightcc)
  {
  if(!strncmp(head->eightcc, eightcc, 8))
    return 1;
  else
    return 0;
  }

int gavl_chunk_read_header(gavl_io_t * io, gavl_chunk_t * head)
  {
  /* Byte align (if not already aligned) */
  if(!gavl_io_align_read(io))
    return 0;
  
  if((gavl_io_read_data(io, (uint8_t*)head->eightcc, 8) < 8) ||
     !gavl_io_read_int64f(io, &head->len))
    return 0;
  /* Be strcmp friendly */
  head->eightcc[8] = 0x00;
  return 1;
  }

int gavl_chunk_start(gavl_io_t * io, gavl_chunk_t * head, const char * eightcc)
  {
  gavl_io_align_write(io);
  
  head->start = gavl_io_position(io);

  memcpy(head->eightcc, eightcc, 8);
  
  if((gavl_io_write_data(io, (const uint8_t*)eightcc, 8) < 8) ||
     !gavl_io_write_int64f(io, 0))
    return 0;
  return 1;
  }

int gavl_chunk_finish(gavl_io_t * io, gavl_chunk_t * head, int write_size)
  {
  int64_t position;
  int64_t size;
  
  position = gavl_io_position(io);
  
  if(write_size && gavl_io_can_seek(io))
    {
    size = (position - head->start) - 16;

    position = gavl_io_position(io);
    
    gavl_io_seek(io, head->start+8, SEEK_SET);
    gavl_io_write_int64f(io, size);
    gavl_io_seek(io, position, SEEK_SET);
    }

  /* Pad to multiple of 8 */
    
  gavl_io_align_write(io);

  return 1;
  }

gavl_io_t * gavl_chunk_start_io(gavl_io_t * io, gavl_chunk_t * head, const char * eightcc)
  {
  gavl_io_t * sub_io = gavl_io_create_mem_write();
  
  gavl_chunk_start(sub_io, head, eightcc);
  return sub_io;
  }
  

int gavl_chunk_finish_io(gavl_io_t * io, gavl_chunk_t * head, gavl_io_t * sub_io)
  {
  int len = 0;
  uint8_t * ret;
  
  gavl_chunk_finish(sub_io, head, 1);
  ret = gavl_io_mem_get_buf(sub_io, &len);
  gavl_io_destroy(sub_io);
  
  gavl_io_write_data(io, ret, len);
  gavl_io_flush(io);
  free(ret);

  return 1;
  }

/* From / To Buffer */

int gavl_dictionary_from_buffer(const uint8_t * buf, int len, gavl_dictionary_t * fmt)
  {
  int result;
  gavl_io_t * io = gavl_io_create_mem_read(buf, len);
  result = gavl_dictionary_read(io, fmt);
  gavl_io_destroy(io);
  return result;
  }
  
uint8_t * gavl_dictionary_to_buffer(int * len, const gavl_dictionary_t * fmt)
  {
  uint8_t * ret;
  gavl_io_t * io = gavl_io_create_mem_write();
  gavl_dictionary_write(io, fmt);
  ret = gavl_io_mem_get_buf(io, len);
  gavl_io_destroy(io);
  return ret;
  }

