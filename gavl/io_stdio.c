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

#include <config.h>
#include <gavfprivate.h>

#include <gavl/log.h>
#define LOG_DOMAIN "io_stdio"

#ifdef _WIN32
#define GAVL_FSEEK(a,b,c) fseeko64(a,b,c)
#define GAVL_FTELL(a) ftello64(a)
#else
#ifdef HAVE_FSEEKO
#define GAVL_FSEEK fseeko
#else
#define GAVL_FSEEK fseek
#endif

#ifdef HAVE_FTELLO
#define GAVL_FTELL ftello
#else
#define GAVL_FTELL ftell
#endif
#endif

static int read_file(void * priv, uint8_t * data, int len)
  {
  return fread(data, 1, len, (FILE*)priv);
  }

static int write_file(void * priv, const uint8_t * data, int len)
  {
  return fwrite(data, 1, len, (FILE*)priv);
  }

static int64_t seek_file(void * priv, int64_t pos, int whence)
  {
  GAVL_FSEEK((FILE*)priv, pos, whence);
  return GAVL_FTELL((FILE*)priv);
  }

static int flush_file(void * priv)
  {

  if(!fflush((FILE*)priv))
    return 1;
  //  fprintf(stderr, "flush failed\n");
  return 0;
  }

static void close_file(void * priv)
  {
  fclose((FILE*)priv);
  }

GAVL_PUBLIC
gavl_io_t * gavl_io_create_file(FILE * f, int wr, int can_seek, int close)
  {
  gavl_read_func rf;
  gavl_write_func wf;
  gavl_seek_func sf;
  gavl_flush_func ff;
  int flags = 0;
  struct stat st;
  int fd = fileno(f);

  if(isatty(fd))
    flags |= GAVL_IO_IS_TTY;

  if(wr)
    flags |= GAVL_IO_CAN_WRITE;
  else
    flags |= GAVL_IO_CAN_READ;
  
  if(fstat(fd, &st))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot stat file");
    return NULL;
    }
  if(S_ISFIFO(st.st_mode)) /* Pipe: Use local connection */
    {
    flags |= (GAVL_IO_IS_LOCAL|GAVL_IO_IS_PIPE);
    }
  else if(S_ISREG(st.st_mode) && can_seek)
    {
    flags |= GAVL_IO_IS_REGULAR;
    }

  if(!(flags & GAVL_IO_IS_REGULAR))
    can_seek = 0;
  
  if(can_seek)
    flags |= GAVL_IO_CAN_SEEK;
  
  if(wr)
    {
    wf = write_file;
    rf = NULL;
    ff = flush_file;
    }
  else
    {
    wf = NULL;
    rf = read_file;
    ff = NULL;
    }
  if(can_seek)
    sf = seek_file;
  else
    sf = NULL;

  return gavl_io_create(rf, wf, sf, close ? close_file : NULL, ff, flags, f);
  }

gavl_io_t * gavl_io_from_filename(const char * filename, int wr)
  {
  FILE * f;
  
  if(wr)
    {
    if(!(f = fopen(filename, "w")))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot open %s for writing: %s", filename, strerror(errno));
      return NULL;
      }
    }
  else
    {
    if(!(f = fopen(filename, "r")))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot open %s for reading: %s", filename, strerror(errno));
      return NULL;
      }
    }
  
  return gavl_io_create_file(f, wr, 1, 1);
  }
