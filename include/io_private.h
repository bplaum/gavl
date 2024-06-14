/* I/O */



struct gavl_io_s
  {
  gavl_read_func read_func;
  gavl_read_func read_func_nonblock;
  
  gavl_write_func write_func;
  gavl_write_func write_func_nonblock;
  gavl_seek_func seek_func;
  gavl_close_func close_func;
  gavl_flush_func flush_func;
  gavl_poll_func poll_func;
  
  void * priv;
  
  int64_t position;
  
  /* Informational data */
  
  int64_t total_bytes;
  
  gavl_dictionary_t info;
  
  gavl_buffer_t get_buf;

  int flags;
  };

void gavl_io_init(gavl_io_t * ret,
                  gavl_read_func  r,
                  gavl_write_func w,
                  gavl_seek_func  s,
                  gavl_close_func c,
                  gavl_flush_func f,
                  int flags,
                  void * priv);



void gavl_io_init_buf_read(gavl_io_t * io, gavl_buffer_t * buf);
void gavl_io_init_buf_write(gavl_io_t * io, gavl_buffer_t * buf);

void gavl_io_set_nonblock_read(gavl_io_t * io, gavl_read_func read_nonblock);
void gavl_io_set_nonblock_write(gavl_io_t * io, gavl_write_func write_nonblock);
void gavl_io_cleanup(gavl_io_t * io);

