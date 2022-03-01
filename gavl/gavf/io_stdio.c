#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

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
  int ret;
  ret =  fwrite(data, 1, len, (FILE*)priv);
  //  if(ret < len)
  //    fprintf(stderr, "write failed: %d < %d\n", ret, len);
  return ret;
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
gavf_io_t * gavf_io_create_file(FILE * f, int wr, int can_seek, int close)
  {
  gavf_read_func rf;
  gavf_write_func wf;
  gavf_seek_func sf;
  gavf_flush_func ff;
  int flags = 0;
  struct stat st;
  int fd = fileno(f);

  if(isatty(fd))
    flags |= GAVF_IO_IS_TTY;

  if(wr)
    flags |= GAVF_IO_CAN_WRITE;
  else
    flags |= GAVF_IO_CAN_READ;
  
  if(fstat(fd, &st))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot stat file");
    return NULL;
    }
  if(S_ISFIFO(st.st_mode)) /* Pipe: Use local connection */
    {
    flags |= (GAVF_IO_IS_LOCAL|GAVF_IO_IS_PIPE);
    }
  else if(S_ISREG(st.st_mode) && can_seek)
    {
    flags |= GAVF_IO_IS_REGULAR;
    }

  if(can_seek)
    flags |= GAVF_IO_CAN_SEEK;
  
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

  return gavf_io_create(rf, wf, sf, close ? close_file : NULL, ff, flags, f);
  }
