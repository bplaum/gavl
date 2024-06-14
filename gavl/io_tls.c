
#include <config.h>

#include <pthread.h>
#include <stdlib.h>
#include <string.h>


#include <unistd.h>
#include <fcntl.h>

#include <gavl/gavf.h>
#include <gavl/log.h>
#include <gavl/gavlsocket.h>

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include <gavl/io.h>
#include <io_private.h>

/* Global initialization */

static int tls_initialized = 0;
static pthread_mutex_t initialized_mutex = PTHREAD_MUTEX_INITIALIZER;
static gnutls_certificate_credentials_t xcred;

#define LOG_DOMAIN "tls"

// #define BUFFER_SIZE 10240

static void tls_global_init()
  {
  pthread_mutex_lock(&initialized_mutex);

  if(tls_initialized)
    {
    pthread_mutex_unlock(&initialized_mutex);
    return;
    }

  if(gnutls_global_init() < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "gnutls_global_init failed");
    return;
    }
  
  if(gnutls_certificate_allocate_credentials(&xcred) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "gnutls_certificate_allocate_credentials failed");
    return;
    }
  
  if(gnutls_certificate_set_x509_system_trust(xcred) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "gnutls_certificate_set_x509_system_trust failed");
    return;
    }
  
  tls_initialized = 1;
  pthread_mutex_unlock(&initialized_mutex);
  }


#if defined(__GNUC__)

static void cleanup_tls() __attribute__ ((destructor));

static void cleanup_tls()
  {
  pthread_mutex_lock(&initialized_mutex);

  if(!tls_initialized)
    {
    pthread_mutex_unlock(&initialized_mutex);
    return;
    }

  gnutls_certificate_free_credentials(xcred);
  gnutls_global_deinit();
  pthread_mutex_unlock(&initialized_mutex);
  }

#endif

#define WAIT_STATE_NONE   0
#define WAIT_STATE_READ   1
#define WAIT_STATE_WRITE  2

typedef struct
  {
  int fd;
  gnutls_session_t session;  
  
  gavl_buffer_t read_buffer;
  gavl_buffer_t write_buffer;
  
  char * server_name;
  
  int flags;

  int wait_state;
  int buffer_size;

  gavl_timer_t * timer;
  
  } tls_t;

static int do_wait(tls_t * p, int timeout)
  {
  if(timeout > 0)
    {
    switch(p->wait_state)
      {
      case WAIT_STATE_NONE:
        break;
      case WAIT_STATE_READ:
        if(!gavl_fd_can_read(p->fd, timeout))
          {
          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Read timeout (%d ms) exceeded", timeout);
          return 0;
          }
        break;
      case WAIT_STATE_WRITE:
        if(!gavl_fd_can_write(p->fd, timeout))
          {
          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Write timeout (%d ms) exceeded", timeout);
          return 0;
          }
        break;
      }
    }
  return 1;
  }

static int do_flush(void * priv, int block)
  {
  tls_t * p = priv;

  //  fprintf(stderr, "flush_tls\n");
  //  gavl_hexdump(p->write_buffer.buf, p->write_buffer.len, 16);

  //  if(!p->write_buffer.len)
  //    return 1;
  
  while(p->write_buffer.pos < p->write_buffer.len)
    {
    ssize_t result;

    do
      {
      result = gnutls_record_send(p->session,
                                  p->write_buffer.buf + p->write_buffer.pos,
                                  p->write_buffer.len - p->write_buffer.pos);
      
      if((result == GNUTLS_E_AGAIN))
        {
        p->wait_state = 1 + gnutls_record_get_direction(p->session);
        
        if(!block || !do_wait(p, 3000))
          break;
        }
      
      } while((result == GNUTLS_E_AGAIN) || (result == GNUTLS_E_INTERRUPTED));

    if(result < 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
               "gnutls_record_send failed: %s", gnutls_strerror(result));
      }
    if(!result)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
               "gnutls_record_send returned zero");
      }
    
    if(result <= 0)
      return 0;

    if(result > 0)
      {
      p->write_buffer.pos += result;
      p->wait_state = WAIT_STATE_NONE;
      }
    }

  if(p->write_buffer.pos == p->write_buffer.len)
    gavl_buffer_reset(&p->write_buffer);
  
  return 1;
  }


static int flush_tls(void * priv)
  {
  return do_flush(priv, 1);
  }
  
static int read_record(tls_t * p, int timeout_init)
  {
  ssize_t result;
  int timeout;
  int bytes_to_read;

  timeout = timeout_init;
  
  if(!do_flush(p, !!timeout))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot read TLS record: Flushing failed");
    return 0;
    }
  bytes_to_read = p->buffer_size;

  gavl_timer_set(p->timer, 0);
  gavl_timer_start(p->timer);
  
  do
    {
    result = gnutls_record_recv(p->session, p->read_buffer.buf, bytes_to_read);

    if(!result)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "gnutls_record_recv returned zero (EOF)");
      }
    
    if((result == GNUTLS_E_AGAIN))
      {
      p->wait_state = 1 + gnutls_record_get_direction(p->session);
      
      if(!timeout || !do_wait(p, timeout))
        {
        if(timeout_init > 0)
          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "gnutls_record_recv: Timeout (%d ms) expired", timeout_init);
        break;
        }
      }
    
    timeout = timeout_init - gavl_timer_get(p->timer) / 1000;
    
    if(timeout < 0)
      timeout = 0;
    
    } while((result == GNUTLS_E_AGAIN) || (result == GNUTLS_E_INTERRUPTED));

    
  gavl_timer_stop(p->timer);
  
  if((result < 0) && gnutls_error_is_fatal(result) && (result != GNUTLS_E_PREMATURE_TERMINATION))
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Reading TLS record failed: %s (timeout was %d ms)",
             gnutls_strerror(result), timeout);
  
  if(result > 0)
    {
    p->read_buffer.len = result;
    p->read_buffer.pos = 0;
    p->wait_state = WAIT_STATE_NONE;
    return 1;
    }
  return 0;
  }

static int do_read_tls(void * priv, uint8_t * data, int len, int block)
  {
  tls_t * p = priv;
  int bytes_read = 0;
  int bytes_to_read;
  
  while(bytes_read < len)
    {
    if(p->read_buffer.pos >= p->read_buffer.len)
      {
      if(!read_record(p, block ? 3000 : 0))
        {
        if(block)
          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Reading TLS record failed");
        break;
        }
      }

    bytes_to_read = len - bytes_read;

    if(bytes_to_read > p->read_buffer.len - p->read_buffer.pos)
      {
      bytes_to_read = p->read_buffer.len - p->read_buffer.pos;
      }

    memcpy(data + bytes_read, p->read_buffer.buf + p->read_buffer.pos, bytes_to_read);
    
    bytes_read += bytes_to_read;
    
    p->read_buffer.pos += bytes_to_read;
    }

#if 0  
  if((bytes_read < len) && block)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "do_read_tls requested: %d got %d", len, bytes_read);
    }
#endif

  return bytes_read;
  }

static int read_tls(void * priv, uint8_t * data, int len)
  {
  return do_read_tls(priv, data, len, 1);
  }

static int read_nonblock_tls(void * priv, uint8_t * data, int len)
  {
  return do_read_tls(priv, data, len, 0);
  }


static int do_write(void * priv, const uint8_t * data, int len, int block)
  {
  int bytes_to_copy;
  int bytes_sent = 0;
  tls_t * p = priv;

  while(bytes_sent < len)
    {
    if(p->write_buffer.len == p->buffer_size)
      {
      if(!do_flush(priv, block))
        {
        fprintf(stderr, "Flushing data failed\n");
        return -1;
        }
      if(p->write_buffer.len == p->buffer_size)
        {
        if(!bytes_sent && !block)
          fprintf(stderr, "write_nonblock returned 0 [1] buffer_size: %d wait_state: %d\n",
                  p->buffer_size, p->wait_state);
        
        return bytes_sent;
        }
      }
    
    bytes_to_copy = len - bytes_sent;

    if(bytes_to_copy > p->buffer_size - p->write_buffer.len)
      {
      bytes_to_copy = p->buffer_size - p->write_buffer.len;
      }

    memcpy(p->write_buffer.buf + p->write_buffer.len, data + bytes_sent, bytes_to_copy);
    
    p->write_buffer.len += bytes_to_copy;
    bytes_sent += bytes_to_copy;
    }
  if(!bytes_sent && !block)
    fprintf(stderr, "write_nonblock returned 0 [2]\n");
  return bytes_sent;
  }

static int write_tls(void * priv, const uint8_t * data, int len)
  {
  return do_write(priv, data, len, 1);
  }

static int write_nonblock_tls(void * priv, const uint8_t * data, int len)
  {
  return do_write(priv, data, len, 0);
  }

// static int64_t (*gavf_seek_func)(void * priv, int64_t pos, int whence);

static void close_tls(void * priv)
  {
  tls_t * p = priv;

  gnutls_bye(p->session, GNUTLS_SHUT_WR);
  gnutls_deinit(p->session);

  gavl_buffer_free(&p->read_buffer);
  gavl_buffer_free(&p->write_buffer);
  if(p->server_name)
    free(p->server_name);

  if(p->flags & GAVL_IO_SOCKET_DO_CLOSE)
    gavl_socket_close(p->fd);

  if(p->timer)
    gavl_timer_destroy(p->timer);
  
  free(p);
  }

static int poll_tls(void * priv, int timeout, int wr)
  {
  tls_t * p = priv;

  if(wr)
    {
    do_flush(p, 0);

    if(p->write_buffer.len < p->buffer_size)
      return 1;
    
    if((timeout > 0) && gavl_fd_can_write(p->fd, timeout))
      {
      do_flush(p, 0);
      
      if(p->write_buffer.len < p->buffer_size)
        return 1;
      }
    return 0;
    }
  else
    {
    do_flush(p, 0);
      
    /* Check if we have buffered data */
    if(p->read_buffer.len > p->read_buffer.pos)
      return 1;

    /* Check if TLS has buffered data */
    if(gnutls_record_check_pending(p->session) > 0)
      return 1;
    
    if(read_record(p, timeout) > 0)
      return 1;
    
    return gavl_fd_can_read(p->fd, timeout);
    }
  
  
  }

gavl_io_t * gavl_io_create_tls_client_async(int fd, const char * server_name, int socket_flags)
  {
  tls_t * p;
  //  int result;
  gavl_io_t * io;
  int flags = GAVL_IO_CAN_READ | GAVL_IO_CAN_WRITE | GAVL_IO_IS_DUPLEX | GAVL_IO_IS_SOCKET;

  tls_global_init();
  
  p = calloc(1, sizeof(*p));

  p->flags = socket_flags;
  p->fd = fd;

  p->timer = gavl_timer_create();
  
  gavl_socket_set_block(p->fd, 0);
  
  gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Establishing TLS connection with %s", server_name);
  
  p->server_name = gavl_strdup(server_name);
  
  gnutls_init(&p->session, GNUTLS_CLIENT | GNUTLS_NONBLOCK);

  gnutls_server_name_set(p->session, GNUTLS_NAME_DNS, server_name,
                         strlen(server_name));

  gnutls_set_default_priority(p->session);

  gnutls_credentials_set(p->session, GNUTLS_CRD_CERTIFICATE, xcred);

  //  gnutls_session_set_verify_cert(p->session, server_name, 0);

  gnutls_transport_set_int(p->session, fd);
  //  gnutls_handshake_set_timeout(p->session,
  //                               GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);

  io = gavl_io_create(read_tls,
                      write_tls,
                      NULL,
                      close_tls,
                      flush_tls,
                      flags,
                      p);
  
  gavl_io_set_nonblock_read(io, read_nonblock_tls);
  gavl_io_set_nonblock_write(io, write_nonblock_tls);
  gavl_io_set_poll_func(io, poll_tls);
  
  return io;
  }

int gavl_io_create_tls_client_async_done(gavl_io_t * io, int timeout)
  {
  int result;
  
  tls_t * p = io->priv;

  while(1)
    {
    if(!do_wait(p, timeout))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "gavl_io_create_tls_client_async: Got timeout");
      return 0;
      }
    result = gnutls_handshake(p->session);
    
    //   while(result < 0 && gnutls_error_is_fatal(result) == 0);

    if(result < 0)
      {
      if(gnutls_error_is_fatal(result))
        {
        if(result == GNUTLS_E_CERTIFICATE_VERIFICATION_ERROR)
          {
          gnutls_datum_t out;
          unsigned status;
          /* check certificate verification status */
          int type = gnutls_certificate_type_get(p->session);
          status = gnutls_session_get_verify_cert_status(p->session);

          gnutls_certificate_verification_status_print(status, type, &out, 0);

          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Server certificate verification failed: %s", out.data);
      
          gnutls_free(out.data);
          }

        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "TLS Handshake failed: %s", gnutls_strerror(result));
        return -1;
        }
      else
        {
        /* Non-fatal: Try again */

        if(result == GNUTLS_E_AGAIN)
          {
          p->wait_state = 1 + gnutls_record_get_direction(p->session);
          if(timeout <= 0)
            return 0;
          }
        /* Try again with waiting */
        }
      }
    else
      {
      char * desc = gnutls_session_get_desc(p->session);
      gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Established TLS connection: %s", desc);
      gnutls_free(desc);

      p->buffer_size = gnutls_record_get_max_size(p->session);

      gavl_buffer_alloc(&p->read_buffer, p->buffer_size);
      gavl_buffer_alloc(&p->write_buffer, p->buffer_size);
      p->wait_state = WAIT_STATE_NONE;
      return 1;
      }
    }
  }

gavl_io_t * gavl_io_create_tls_client(int fd, const char * server_name, int socket_flags)
  {
  gavl_io_t * io;
  int ret = 0;
  int result;
  
  io = gavl_io_create_tls_client_async(fd, server_name, socket_flags);

  while(1)
    {
    result = gavl_io_create_tls_client_async_done(io, 3000);

    if(result == 1)
      break;
    else if(result < 0)
      goto fail;
    else
      {
      fprintf(stderr, "gavl_io_create_tls_client_async_done failed %d\n", result);
      goto fail;
      }
    }
  
  ret = 1;
  fail:
  if(!ret)
    {
    gavl_io_destroy(io);
    return NULL;
    }
  else
    return io;
  }
