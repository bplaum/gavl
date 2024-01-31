#include <stdlib.h>
#include <string.h>

#include <gavfprivate.h>

/* Buffer as io */

static int64_t seek_buf(void * priv, int64_t pos1, int whence)
  {
  int64_t pos;
  gavl_buffer_t * buf = priv;
  
  switch(whence)
    {
    case SEEK_CUR:
      pos = buf->pos + pos1;
      break;
    case SEEK_END:
      pos = buf->len + pos1;
      break;
    default:
      pos = pos1;
      break;
    }

  if((pos < 0) || (pos > buf->len))
    {
    fprintf(stderr, "Bug in seek_buf: pos: %"PRId64"\n", pos);
    return buf->pos;
    }
  buf->pos = pos;
  return buf->pos;
  }

static int read_buf(void * priv, uint8_t * data, int len)
  {
  gavl_buffer_t * buf = priv;
  if(len > buf->len - buf->pos)
    len = buf->len - buf->pos;
  
  if(len > 0)
    {
    memcpy(data, buf->buf + buf->pos, len);
    buf->pos += len;
    }
  return len;
  }

static int write_buf(void * priv, const uint8_t * data, int len)
  {
  gavl_buffer_t * buf = priv;

  if(buf->pos + len > buf->len)
    {
    if(!gavl_buffer_alloc(buf, buf->pos + len))
      return 0;
    }
  
  memcpy(buf->buf + buf->pos, data, len);
  buf->pos += len;
  
  if(buf->len < buf->pos)
    buf->len = buf->pos;
  return len;
  }

static void close_buf(void * priv)
  {
  gavl_buffer_free(priv);
  free(priv);
  }

gavl_io_t * gavl_io_create_buf_read()
  {
  gavl_buffer_t * buf = calloc(1, sizeof(*buf));
  
  return gavl_io_create(read_buf,
                        NULL,
                        seek_buf,
                        close_buf,
                        NULL,
                        GAVF_IO_CAN_READ | GAVF_IO_CAN_SEEK,
                        buf);
  }

gavl_io_t * gavl_io_create_buf_write()
  {
  gavl_buffer_t * buf = calloc(1, sizeof(*buf));

  return gavl_io_create(NULL,
                        write_buf,
                        seek_buf,
                        close_buf,
                        NULL,
                        GAVF_IO_CAN_WRITE | GAVF_IO_CAN_SEEK,
                        buf);
  
  }

gavl_buffer_t * gavl_io_buf_get(gavl_io_t * io)
  {
  return io->priv;
  }

void gavl_io_buf_reset(gavl_io_t * io)
  {
  gavl_buffer_reset(io->priv);
  io->position = 0;
  }

void gavl_io_init_buf_read(gavl_io_t * io, gavl_buffer_t * buf)
  {
  gavl_io_init(io,
               read_buf,
               NULL,
               seek_buf,
               NULL,
               NULL,
               GAVF_IO_CAN_READ | GAVF_IO_CAN_SEEK,
               buf);
  }

void gavl_io_init_buf_write(gavl_io_t * io, gavl_buffer_t * buf)
  {
  gavl_io_init(io,
               NULL,
               write_buf,
               seek_buf,
               NULL,
               NULL,
               GAVF_IO_CAN_WRITE | GAVF_IO_CAN_SEEK,
               buf);
  }
