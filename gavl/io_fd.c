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


#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/buffer.h>
#include <gavl/io.h>
#include <gavl/utils.h>

#include <io_private.h>

#include <gavl/log.h>
#define LOG_DOMAIN "io_fd"

typedef struct
  {
  int fd;
  int do_close;
  int block;    // fd block flag
  } fd_t;

static int read_fd(void * priv, uint8_t * data, int len, int block)
  {
  int ret;
  fd_t * f = priv;
  
  if(f->block != block)
    {
    gavl_fd_set_block(f->fd, block);
    f->block = block;
    }
  ret = read(f->fd, data, len);

  if(ret < 0)
    {
    if((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR))
      return 0; // Try again
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "read() returned %d: %s", ret, strerror(errno));
    }
  return ret;
  }
  
static int read_block_fd(void * priv, uint8_t * data, int len)
  {
  return read_fd(priv, data, len, 1);
  }

static int read_nonblock_fd(void * priv, uint8_t * data, int len)
  {
  return read_fd(priv, data, len, 0);
  }

static int write_fd(void * priv, const uint8_t * data, int len)
  {
  fd_t * f = priv;
  return write(f->fd, data, len);
  }

static void close_fd(void * priv)
  {
  fd_t * f = priv;
  if(f->do_close)
    close(f->fd);
  free(f);
  }

static int poll_fd(void * priv, int timeout, int wr)
  {
  fd_t * s = priv;

  if(wr)
    return gavl_fd_can_write(s->fd, timeout);
  else
    return gavl_fd_can_read(s->fd, timeout);
  }

GAVL_PUBLIC
gavl_io_t * gavl_io_create_fd(int fd, int wr, int close)
  {
  gavl_read_func rf;
  gavl_write_func wf;
  
  int flags = 0;
  struct stat st;
  gavl_io_t * ret;

  fd_t * f = calloc(1, sizeof(*f));

  f->fd = fd;
  
  if(isatty(fd))
    flags |= GAVL_IO_IS_TTY;

  if(wr)
    flags |= GAVL_IO_CAN_WRITE;
  else
    {
    flags |= GAVL_IO_CAN_READ;
    f->block = gavl_fd_get_block(fd);
    }
  if(fstat(fd, &st))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot stat file");
    return NULL;
    }
  if(S_ISFIFO(st.st_mode)) /* Pipe: Use local connection */
    {
    flags |= (GAVL_IO_IS_LOCAL|GAVL_IO_IS_PIPE);
    }
  else if(S_ISREG(st.st_mode))
    {
    flags |= GAVL_IO_IS_REGULAR;
    }
  
  if(wr)
    {
    wf = write_fd;
    rf = NULL;
    }
  else
    {
    wf = NULL;
    rf = read_block_fd;
    }

  ret = gavl_io_create(rf, wf, NULL, close_fd, NULL, flags, f);
  gavl_io_set_poll_func(ret, poll_fd);
  gavl_io_set_nonblock_read(ret, read_nonblock_fd);
  
  gavl_io_set_info(ret, st.st_size, NULL, NULL, flags);
  
  return ret;
  }

