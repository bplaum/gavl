#include <stdlib.h>
#include <string.h>



#include <gavl/gavf.h>
#include <gavl/gavlsocket.h>

#include <gavfprivate.h>


/* 10 kB */
#define BUFFER_SIZE 10240




typedef struct
  {
  int fd;
  int timeout;

  int flags;

  gavl_buffer_t buf;

  gavf_io_t * io; // Parent
  } socket_t;

static void do_buffer(socket_t * s)
  {
  int len;

  if(!(s->flags & GAVF_IO_SOCKET_BUFFER_READ))
    return;
  
  if(s->buf.len < BUFFER_SIZE)
    {
    len  = gavl_socket_read_data_noblock(s->fd, s->buf.buf + s->buf.len,
                                         BUFFER_SIZE - s->buf.len);

    if(len > 0)
      {
      fprintf(stderr, "Buffering fd: %d len: %d\n", s->fd, len);
      s->buf.len += len;
      }
    }
  }

static int do_read_socket(void * priv, uint8_t * data, int len, int block)
  {
  int err;
  int result;
  int bytes_to_read;
  int bytes_read = 0;
  socket_t * s = priv;

//  if(len > 1)
//    fprintf(stderr, "Read socket: %d %d %d\n", s->fd, len, s->buf.len);

  /* Take from buffer */
  if(s->buf.len > 0)
    {
    /* Bytes to read */
    bytes_to_read = len;
    
    if(bytes_to_read > s->buf.len)
      bytes_to_read = s->buf.len;

    /* memcpy */
    memcpy(data + bytes_read, s->buf.buf, bytes_to_read);
    
    gavl_buffer_flush(&s->buf, bytes_to_read);
    
    len -= bytes_to_read;
    bytes_read += bytes_to_read;
    }

  if(!len)
    {
    do_buffer(s);

    //    fprintf(stderr, "Read socket 1: %d\n", bytes_read);
    
    return bytes_read;
    }

  if(block)
    {
    result = gavl_socket_read_data(s->fd, data + bytes_read, len, s->timeout);
    
    if((result < len) && (err = gavl_socket_get_errno(s->fd)))
      {
      gavf_io_set_error(s->io);
      fprintf(stderr, "Got socket error: %d [%s]\n", err, strerror(err));
      }
    }
  else
    result = gavl_socket_read_data_noblock(s->fd, data + bytes_read, len);
  
  if(result <= 0)
    {
    //    fprintf(stderr, "Read socket 2: %d\n", bytes_read);
    return bytes_read;
    }
  else
    bytes_read += result;

  do_buffer(s);
  
  //  if(

  //  fprintf(stderr, "Read socket 3: %d\n", bytes_read);
  
  return bytes_read;
  }

static int read_socket(void * priv, uint8_t * data, int len)
  {
  return do_read_socket(priv, data, len, 1);
  }
  
static int read_socket_nonblock(void * priv, uint8_t * data, int len)
  {
  return do_read_socket(priv, data, len, 0);
  }


static int write_socket(void * priv, const uint8_t * data, int len)
  {
  socket_t * s = priv;
  return gavl_socket_write_data(s->fd, data, len);
  }

static int write_socket_nonblock(void * priv, const uint8_t * data, int len)
  {
  socket_t * s = priv;
  return gavl_socket_write_data_nonblock(s->fd, data, len);
  }

static void close_socket(void * priv)
  {
  socket_t * s = priv;

  gavl_buffer_free(&s->buf);
  
  if(s->flags & GAVF_IO_SOCKET_DO_CLOSE)
    gavl_socket_close(s->fd);
  free(s);
  }

static int poll_socket(void * priv, int timeout, int wr)
  {
  socket_t * s = priv;

  if(wr)
    return gavl_socket_can_write(s->fd, timeout);
  
  if(s->buf.len > 0)
    return 1;
  else
    return gavl_socket_can_read(s->fd, timeout);
  }

gavf_io_t * gavf_io_create_socket(int fd, int read_timeout, int socket_flags)
  {
  gavf_io_t * ret = NULL;
  int flags;
  socket_t * s = calloc(1, sizeof(*s));

  s->flags = socket_flags;
  s->timeout = read_timeout;
  s->fd = fd;

  flags = GAVF_IO_CAN_READ | GAVF_IO_CAN_WRITE | GAVF_IO_IS_DUPLEX | GAVF_IO_IS_SOCKET;

  if(gavl_socket_is_local(fd))
    flags |= GAVF_IO_IS_LOCAL;
  if(gavl_socket_is_unix(fd))
    flags |= GAVF_IO_IS_UNIX_SOCKET;
  
  gavl_buffer_alloc(&s->buf, BUFFER_SIZE);
  
  ret = gavf_io_create(read_socket, write_socket,
                       NULL, // seek
                       close_socket,
                       NULL, // flush
                       flags,
                       s);

  gavf_io_set_poll_func(ret, poll_socket);
  gavf_io_set_nonblock_read(ret, read_socket_nonblock);
  gavf_io_set_nonblock_write(ret, write_socket_nonblock);
  
  s->io = ret;
  return ret;
  }

int gavf_io_get_socket(gavf_io_t * io)
  {
  socket_t * s = gavf_io_get_priv(io);
  return s->fd;
  }
