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



#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/buffer.h>
#include <gavl/utils.h>
#include <gavl/http.h>
#include <gavl/log.h>
#include <gavl/gavlsocket.h>
#include <gavl/metatags.h>

#include <gavl/io.h>

#include <io_private.h>


// #define DUMP_HEADERS

#define LOG_DOMAIN "httpclient"

#define FLAG_USE_PROXY          (1<<0)
#define FLAG_KEEPALIVE          (1<<1)
#define FLAG_CHUNKED            (1<<2)
#define FLAG_USE_PROXY_TUNNEL   (1<<3)
#define FLAG_USE_TLS            (1<<4)
#define FLAG_GOT_REDIRECTION    (1<<5)
#define FLAG_HAS_RESPONSE_BODY  (1<<6)
#define FLAG_USE_KEEPALIVE      (1<<7)
#define FLAG_WAIT               (1<<8)
#define FLAG_RESPONSE_BODY_DONE (1<<9)
#define FLAG_ERROR              (1<<10)
// #define FLAG_RECONNECT          (1<<11)

#define STATE_START                 0
#define STATE_RESOLVE               1
#define STATE_CONNECT               2
#define STATE_SEND_CONNECT_REQUEST  3
#define STATE_READ_CONNECT_RESPONSE 4
#define STATE_TLS_HANDSHAKE         5
#define STATE_SEND_REQUEST          6
#define STATE_SEND_BODY             7
#define STATE_READ_RESPONSE         8

#define STATE_READ_BODY_NORMAL      9 // Use Content-Length

#define STATE_READ_CHUNK_HEADER    10 // Chunked
#define STATE_READ_CHUNK           11 // Chunked
#define STATE_READ_CHUNK_TAIL      12 // Chunked

#define STATE_COMPLETE              13
#define STATE_PAUSED                14

/* Read chunk header in these byte increments */
#define CHUNK_HEADER_BYTES 4

#define MAX_RECONNECTS 5


typedef struct
  {
  gavl_io_t * io_int;
  gavl_io_t * io_proxy; // Plain http connection for sending the CONNECT request to a proxy

  gavl_io_t * io; // External handle
  
  char * host;
  char * protocol;
  int port;

  char * proxy_host;
  int proxy_port;
  
  gavl_dictionary_t resp;
  gavl_dictionary_t req;
  
  gavl_socket_address_t * addr;

  int chunk_length;

  int64_t position;
  int64_t total_bytes;
  
  int64_t pos;
  int started;

  /* Set by gavl_http_client_open() */
  gavl_dictionary_t vars;
  gavl_dictionary_t vars_from_uri;
  
  char * uri;
  char * method;

  int flags;
  
  char * proxy_auth;

  int64_t range_start;
  int64_t range_end;

  gavl_buffer_t * req_body;
  gavl_buffer_t * res_body;
  
  /* Async stuff */
  gavl_buffer_t header_buffer;
  gavl_buffer_t chunk_header_buffer;
  
  int state;
  int num_redirections;

  int fd;
  
  char * redirect_uri;

  const char * real_uri;
  gavl_timer_t * timer;
  int num_reconnects;
  
  } gavl_http_client_t;

pthread_mutex_t proxy_mutex = PTHREAD_MUTEX_INITIALIZER;
static int proxies_initialized = 0;
static char * http_proxy = NULL;
static char * https_proxy = NULL;
static gavl_array_t no_proxy;
static int no_tunnel = 0;

static int
reopen(gavl_io_t * io);

static void do_reset_connection(gavl_http_client_t * c)
  {
  gavl_dictionary_reset(&c->req);
  gavl_dictionary_reset(&c->resp);

  if(c->io_proxy)
    {
    gavl_io_destroy(c->io_proxy);
    c->io_proxy = NULL;
    }
  
  gavl_buffer_reset(&c->header_buffer);
  gavl_buffer_reset(&c->chunk_header_buffer);
  c->real_uri     = NULL;

  c->total_bytes = 0;
  c->chunk_length = 0;
  c->pos = 0;

  if(c->state != STATE_PAUSED)
    c->state        = STATE_START;
  
  c->num_redirections = 0;
  
  gavl_io_clear_eof(c->io);
  gavl_io_clear_error(c->io);

  c->flags &= ~(FLAG_KEEPALIVE|
                FLAG_CHUNKED|
                FLAG_HAS_RESPONSE_BODY|FLAG_ERROR);
  c->num_reconnects = 0;
  }

static void reset_connection(gavl_http_client_t * c)
  {
  //  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Resetting keepalive connection to %s://%s:%d", c->protocol, c->host, c->port);
  do_reset_connection(c);
  }

/* */
static int handle_remote_close(gavl_http_client_t * c)
  {
  if(c->num_reconnects > MAX_RECONNECTS)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Too many re-connects");
    return -1;
    }
  
  switch(c->state)
    {
    case STATE_READ_CHUNK_HEADER:
    case STATE_READ_CHUNK:
    case STATE_READ_CHUNK_TAIL:
      /* Chunked reading */
      if(!c->res_body)
        goto error;
      /* Close and repeat request */
      gavl_buffer_reset(c->res_body);
      c->pos = 0;
      c->chunk_length = 0;
      break;
    case STATE_READ_BODY_NORMAL:

      //      fprintf(stderr, "Connection closed while reading body\n");
      //      gavl_dictionary_dump(&c->resp, 2);
      //      fprintf(stderr, "\nBytes read: %d\n", c->res_body->len);
      
      if(c->total_bytes <= 0)
        goto error;
      
      if(gavl_io_can_seek(c->io))
        gavl_http_client_set_range(c->io, c->pos, -1);
      else
        {
        gavl_buffer_reset(c->res_body);
        c->pos = 0;
        }
      break;
    default:
      goto error;
    }
  
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Connection closed unexpectedly (state: %d), reopening", c->state);
          
  /* Re-connect */
  c->num_redirections = 0;
  c->num_reconnects++;

  if(c->req_body)
    c->req_body->pos = 0;
  
  if(c->io_int)
    {
    gavl_io_destroy(c->io_int);
    c->io_int = NULL;
    }
  if(c->io_proxy)
    {
    gavl_io_destroy(c->io_proxy);
    c->io_proxy = NULL;
    }
  
  c->state = STATE_CONNECT;
  c->fd = gavl_socket_connect_inet(c->addr, 0);
  if(c->fd < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Reopening failed: %d", c->state);
    return -1;
    }
  return 0;
  
  error:

  gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Connection closed unexpectedly (state: %d)", c->state);
  
  return -1;
  }

static void close_connection(gavl_http_client_t * c)
  {
  gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Closing connection: %s", c->uri);

  do_reset_connection(c);

  if(c->io_proxy)
    {
    gavl_io_destroy(c->io_proxy);
    c->io_proxy = NULL;
    }
  
  if(c->io_int)
    {
    gavl_io_destroy(c->io_int);
    c->io_int = NULL;
    }

  if(c->fd >= 0)
    {
    gavl_socket_close(c->fd);
    c->fd = -1;
    }
  
  c->host         = gavl_strrep(c->host, NULL);
  c->protocol     = gavl_strrep(c->protocol, NULL);
  c->proxy_host   = gavl_strrep(c->proxy_host, NULL);
  c->proxy_auth   = gavl_strrep(c->proxy_auth, NULL);
  /* Clear all flags except redirection */
  c->flags        &= FLAG_GOT_REDIRECTION;
  c->port         = -1;
  c->proxy_port   = -1;
  c->num_reconnects = 0;
  }

int gavl_http_client_get_state(gavl_io_t * io)
  {
  gavl_http_client_t * c = gavl_io_get_priv(io);
  return c->state;
  }

static void check_keepalive(gavl_http_client_t * c)
  {
  const char * protocol;
  const char * var;

  c->flags &= ~FLAG_KEEPALIVE;

  
  protocol = gavl_http_response_get_protocol(&c->resp);

  /* Under HTTP/1.1, connections are keep-alive by default */
  if(!strcmp(protocol, "HTTP/1.1"))
    {
    if(!(var = gavl_dictionary_get_string(&c->resp, "Connection")) ||
       !strcasecmp(var, "keep-alive"))
      c->flags |= FLAG_KEEPALIVE;
    }
  else
    {
    if((var = gavl_dictionary_get_string(&c->resp, "Connection")) &&
       !strcasecmp(var, "keep-alive"))
      c->flags |= FLAG_KEEPALIVE;
    }
  }


static void init_proxies()
  {
  char * var;
  
  if(proxies_initialized)
    return;

  proxies_initialized = 1;
  gavl_array_init(&no_proxy);
  
  if((var = getenv("http_proxy")) ||
     (var = getenv("HTTP_PROXY")))
    {
    if(!strstr(var, "://"))
      http_proxy = gavl_sprintf("http://%s", var);
    else
      http_proxy = gavl_strdup(var);
    }
  if((var = getenv("https_proxy")) ||
     (var = getenv("HTTPS_PROXY")))
    {
    if(!strstr(var, "://"))
      https_proxy = gavl_sprintf("https://%s", var);
    else
      https_proxy = gavl_strdup(var);
    }
  
  if((var = getenv("NO_PROXY")))
    {
    int idx = 0;
    char ** hosts = gavl_strbreak(var, ',');

    while(hosts[idx])
      {
      gavl_string_array_add(&no_proxy, hosts[idx]);
      idx++;
      }
    }
  }

static int read_chunk_header_async(gavl_http_client_t * c, int timeout)
  {
  const char * pos;
  int result, rest;

  c->pos = 0;
  
  while(1)
    {
    if(!gavl_io_can_read(c->io_int, timeout))
      {
      if(!timeout)
        return 0;
      else
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got read timeout");
        return -1;
        }
      }
    gavl_buffer_alloc(&c->chunk_header_buffer, c->chunk_header_buffer.len + CHUNK_HEADER_BYTES + 1);
    
    result = gavl_io_read_data_nonblock(c->io_int, c->chunk_header_buffer.buf + c->chunk_header_buffer.len,
                                        CHUNK_HEADER_BYTES);
      
    if(result <= 0)
      return result;

    c->chunk_header_buffer.len += result;

    c->chunk_header_buffer.buf[c->chunk_header_buffer.len] = '\0';
      
    if((pos = strchr((char*)c->chunk_header_buffer.buf, '\n')))
      break;
    }

  /* Parse chunk length */
  if(sscanf((char*)c->chunk_header_buffer.buf, "%x", &c->chunk_length) < 1)
    {
    gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Parsing chunk length failed: %s", (char*)c->chunk_header_buffer.buf);
    return -1;
    }

  //  if(!c->chunk_length)
  //    fprintf(stderr, "Got chunk length: %d\n%s\n", c->chunk_length, (char*)c->chunk_header_buffer.buf);
  
  pos++;

  rest = c->chunk_header_buffer.len - (int)(pos - (char*)c->chunk_header_buffer.buf);

  //  gavl_hexdump((uint8_t*)c->chunk_header_buffer.buf, c->chunk_header_buffer.len, 16);
  // fprintf(stderr, "Rest: %d\n", rest);

  if(rest > 0)
    {
    //    fprintf(stderr, "Unreading data\n");
    //    gavl_hexdump((uint8_t*)pos, rest, 16);
    gavl_io_unread_data(c->io_int, (uint8_t*)pos, rest);
    }
  gavl_buffer_reset(&c->chunk_header_buffer);
  
  
  return 1;
  }

static int read_chunk_tail_async(gavl_http_client_t * c, int timeout)
  {
  int result;
  
  if(!gavl_io_can_read(c->io_int, timeout))
    {
    if(!timeout)
      return 0;
    else
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got read timeout");
      return -1;
      }
    }
  result = gavl_io_read_data_nonblock(c->io_int,
                                      c->chunk_header_buffer.buf + c->chunk_header_buffer.len,
                                      2 - c->chunk_header_buffer.len);
  
  //  fprintf(stderr, "read_chunk_tail_async %d\n", result);
  
  if(result <= 0)
    return result;

  c->chunk_header_buffer.len += result;
  
  if(c->chunk_header_buffer.len < 2)
    return 0;

  if((c->chunk_header_buffer.buf[0] != '\r')  ||
     (c->chunk_header_buffer.buf[1] != '\n'))
    {
    //    fprintf(stderr, "read_chunk_tail_async failed %d %d\n", result, c->chunk_header_buffer.len);
    //    gavl_hexdump(c->chunk_header_buffer.buf, 2, 2);
    return -1;
    }
  return 1;
  }

static int read_chunk_length(gavl_http_client_t * c)
  {
  int ret = 0;
  char * length_str = NULL;
  int length_alloc = 0;
  
  /* Read length */
  if(!gavl_io_read_line(c->io_int, &length_str,
                        &length_alloc, 10000) ||
     (sscanf(length_str, "%x", &c->chunk_length) < 1))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Reading chunk length failed %s", length_str);
    goto fail;
    }
  
  ret = 1;
  fail:
  
  if(length_str)
    free(length_str);
  
  return ret;
  }

static int read_chunk_tail(gavl_http_client_t * c)
  {
  uint8_t crlf[2];
  
  if((gavl_io_read_data(c->io_int, crlf, 2) < 2) ||
     (crlf[0] != '\r') ||
     (crlf[1] != '\n'))
    return 0;
  return 1;
  }

static int read_chunked(void * priv, uint8_t * data, int len, int block)
  {
  int bytes_read = 0;
  int bytes_to_read = 0;
  int result;
  gavl_http_client_t * c = priv;
  int timeout;

  if(block)
    timeout = 3000;
  else
    timeout = 0;
  
  while(bytes_read < len)
    {
    switch(c->state)
      {
      case STATE_READ_CHUNK_HEADER:
        result = read_chunk_header_async(c, timeout);
        if(result <= 0)
          return result;

        if(!c->chunk_length)
          {
          c->state = STATE_READ_CHUNK_TAIL;
          continue;
          }
        else
          {
          c->state = STATE_READ_CHUNK;
          c->pos   = 0;
          }
        break;
      case STATE_READ_CHUNK_TAIL:
        result = read_chunk_tail_async(c, timeout);
        if(result <= 0)
          return result;
        
        if(!c->chunk_length)
          {
          /* End of body */
          c->state = STATE_COMPLETE;
          c->flags |= FLAG_RESPONSE_BODY_DONE;
          check_keepalive(c);
          return bytes_read;
          }
        else
          {
          gavl_buffer_reset(&c->chunk_header_buffer);
          c->state = STATE_READ_CHUNK_HEADER;
          }
        break;
      case STATE_READ_CHUNK:
        bytes_to_read = len - bytes_read;
        if(bytes_to_read > c->chunk_length - c->pos)
          bytes_to_read = c->chunk_length - c->pos;
        
        if(!block)
          {
          result = gavl_io_read_data_nonblock(c->io_int, data + bytes_read, bytes_to_read);
          if(result < 0)
            return -1;
          }
        else
          {
          result = gavl_io_read_data(c->io_int, data + bytes_read, bytes_to_read);
          if(result < bytes_to_read)
            return -1;
          }
        
        c->pos += result;
        bytes_read += result;

        if(c->pos == c->chunk_length)
          c->state = STATE_READ_CHUNK_TAIL;
        
        break;
      }
    }
  return bytes_read;
  }

static int read_normal(void * priv, uint8_t * data, int len, int block)
  {
  int result = 0;
  gavl_http_client_t * c = priv;
  
  if(c->total_bytes)
    {
    if(c->pos + len > c->total_bytes)
      len = c->total_bytes - c->pos;
    }
  
  if(len)
    {
    if(!block && !gavl_io_can_read(c->io_int, 0))
      return 0;

    if(block)
      result = gavl_io_read_data(c->io_int, data, len);
    else
      result = gavl_io_read_data_nonblock(c->io_int, data, len);
  
    if(result < 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Read function returned %d, block: %d",
               result, block);
      goto fail;
      }
    if(block && (result < len))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Requested %d bytes, got %d", len, result);
      goto fail;
      }
    c->pos += result;
    c->position += result;
    }

  if(c->pos == c->total_bytes)
    {
    c->flags |= FLAG_RESPONSE_BODY_DONE;
    //    fprintf(stderr, "httpclient: Detected EOF %"PRId64" %d %"PRId64"\n",
    //            c->pos, len, c->total_bytes);
    check_keepalive(c);
    }
  
  return result;

  fail:
  return -1;
  }

static int read_func(void * priv, uint8_t * data, int len)
  {
  int result;

  gavl_http_client_t * c = priv;
  
  if(!c->io_int)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Input not ready: %d", c->state);
    return -1;
    }
  
  if(c->flags & FLAG_CHUNKED)
    result = read_chunked(priv, data, len, 1);
  else
    result = read_normal(priv, data, len, 1);

  if((result < len) && (gavl_io_got_error(c->io_int)))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got read error");
    gavl_io_set_error(c->io);
    }
  return result;
  }

static int read_nonblock_func(void * priv, uint8_t * data, int len)
  {
  gavl_http_client_t * c = priv;
  if(c->flags & FLAG_CHUNKED)
    return read_chunked(priv, data, len, 0);
  else
    return read_normal(priv, data, len, 0);
  }

static int write_func(void * priv, const uint8_t * data, int len)
  {
  //  gavl_http_client_t * c = priv;
  return 0;
  }

static int poll_func(void * priv, int timeout, int wr)
  {
  gavl_http_client_t * c = priv;

  if(wr)
    return c->io_int ? gavl_io_can_write(c->io_int, timeout) : 0;
  else
    return c->io_int ? gavl_io_can_read(c->io_int, timeout) : 0;
  }

static void close_func(void * priv)
  {
  gavl_http_client_t * c = priv;
  
  close_connection(c);
  
  if(c->addr)
    gavl_socket_address_destroy(c->addr);
  
  gavl_strrep(c->method, NULL);
  gavl_strrep(c->uri, NULL);
  
  gavl_dictionary_free(&c->vars);
  gavl_dictionary_free(&c->vars_from_uri);

  gavl_buffer_free(&c->header_buffer);
  gavl_buffer_free(&c->chunk_header_buffer);

  if(c->timer)
    gavl_timer_destroy(c->timer);
  
  free(c);
  }

static int64_t seek_http(void * priv, int64_t pos1, int whence)
  {
  int64_t pos = -1;
  gavl_http_client_t * c = priv;

  //  fprintf(stderr, "seek_http %p\n", c->io_int);

  switch(whence)
    {
    case SEEK_SET:
      pos = pos1;
      break;
    case SEEK_END:
      pos = c->total_bytes + pos1;
      break;
    case SEEK_CUR:
      pos = c->position + pos1;
      break;
    }

  //  fprintf(stderr, "seek_http 1 %"PRId64" %"PRId64" %p\n", c->position, pos, c->io_int);

  if(c->position == pos)
    return c->position;
  
  if(pos < 0)
    return c->position;
  else if(pos >= c->total_bytes)
    return c->position;
  else
    {
    /* TODO */
    c->range_start = pos;
    c->range_end = -1;
    
    if(c->io_int)
      {
      close_connection(c);

      //      gavl_io_destroy(c->io_int);
      //      c->io_int = NULL;

      //      fprintf(stderr, "reopen 1 %p\n", c->io_int);

      reopen(c->io);

      //      fprintf(stderr, "reopen 2 %p\n", c->io_int);
      }
    }

  //  fprintf(stderr, "seek_http done %p\n", c->io_int);

  //  fprintf(stderr, "seek_http 2 %"PRId64"\n", c->position);
  
  return pos;
  }

gavl_io_t * gavl_http_client_create()
  {
  gavl_http_client_t * c;
  gavl_io_t * ret;
  
  c = calloc(1, sizeof(*c));
  c->addr = gavl_socket_address_create();

  ret = 
    gavl_io_create(read_func,
                   write_func,
                   seek_http,
                   close_func,
                   /* gavf_flush_func f */ NULL,
                   GAVL_IO_CAN_READ | GAVL_IO_CAN_WRITE,
                   c);
  
  c->io = ret;
  
  gavl_io_set_poll_func(ret, poll_func);
  gavl_io_set_nonblock_read(ret, read_nonblock_func);
  
  c->port         = -1;
  c->proxy_port   = -1;
  c->fd           = -1;
  
  return ret;
  }

static int prepare_connection(gavl_io_t * io,
                              const char * host,
                              int port,
                              const char * protocol)
  {
  int ret = 0;
  int do_close = 0;

  const char * proxy = NULL;
  int i;

  //  char * proxy_host = NULL;
  //  int proxy_port = -1;

  char * proxy_user = NULL;
  char * proxy_password = NULL;
  
  gavl_http_client_t * c = gavl_io_get_priv(io);

  /* Check if we close the old connection */
  
  if(gavl_io_got_error(io))
    {
    do_close = 1;
    gavl_io_clear_error(io);
    }
  else if(c->io_int && !(c->flags & FLAG_KEEPALIVE))
    {
    gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Closing connection (keepalive wasn't specified)");
    do_close = 1;
    }
  else if((c->flags & (FLAG_HAS_RESPONSE_BODY|FLAG_RESPONSE_BODY_DONE)) == FLAG_HAS_RESPONSE_BODY)
    {
    gavl_log(GAVL_LOG_INFO , LOG_DOMAIN, "Closing connection (response body wasn't read)");
    do_close = 1;
    }
  
  if(!do_close && c->io_int && c->host && c->protocol && !(c->flags & FLAG_USE_PROXY))
    {
    if(strcmp(host, c->host))
      {
      do_close = 1;
      gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Closing keepalive connection (host changed fromm %s to %s)",
               c->host, host);
      }
    else if(strcmp(protocol, c->protocol))
      {
      do_close = 1;
      gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Closing keepalive connection (protocol changed from %s to %s)",
               c->protocol, protocol);
      }
    else
      {
      if(port == -1)
        {
        if(c->flags & FLAG_USE_TLS)
          port = 443;
        else
          port = 80;
        }
            
      if(c->port != port)
        {
        do_close = 1;
        gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Closing keepalive connection (port changed from %d to %d)",
                 c->port, port);
        }
      
      }
    }
  
  if(c->io_int && do_close)
    {
    close_connection(c);
    }
  else if(c->io_int)
    {
    /* Keep alive */
    reset_connection(c);
    }

  /* Handle redirect */
  if((c->flags & FLAG_GOT_REDIRECTION) &&
     c->redirect_uri)
    c->real_uri = c->redirect_uri;
  else
    c->real_uri = c->uri;
  
  if(c->io_int)
    {
    ret = 1;
    gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Re-using keepalive connection");
    c->flags |= FLAG_USE_KEEPALIVE;
    goto end;
    }
  
  /* Check for proxy */
  pthread_mutex_lock(&proxy_mutex);

  init_proxies();
  
  if(!strcmp(protocol, "http") || !strcmp(protocol, "hls"))
    {
    proxy = http_proxy;
    }
  else if(!strcmp(protocol, "https") || !strcmp(protocol, "hlss"))
    {
    proxy = https_proxy;
    c->flags |= FLAG_USE_TLS;
    }
  else
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Unknown protocol %s", protocol);
      goto end;
    }
  
  for(i = 0; i < no_proxy.num_entries; i++)
    {
    const char * str = gavl_string_array_get(&no_proxy, i);

    if(!strcmp(str, "*"))
      {
      proxy = NULL;
      break;
      }

    else if(gavl_string_ends_with(host, str))
      {
      proxy = NULL;
      break;
      }
    }

  if(proxy)
    {
    if(c->flags & FLAG_USE_TLS)
      {
      if(!no_tunnel)
        {
        c->flags |= FLAG_USE_PROXY_TUNNEL;
        gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Using https proxy %s (tunneling mode)", proxy);
        }
      else
        {
        c->flags = FLAG_USE_PROXY;
        gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Using https proxy %s (http mode)", proxy);

        c->flags &= ~FLAG_USE_TLS;
        }
      }
    else
      {
      c->flags = FLAG_USE_PROXY;
      gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Using http proxy %s", proxy);
      }
    
    if(!gavl_url_split(proxy,
                       NULL,
                       &proxy_user,
                       &proxy_password,
                       &c->proxy_host,
                       &c->proxy_port,
                       NULL))
      goto end;

    if(proxy_user && proxy_password)
      c->proxy_auth = gavl_make_basic_auth(proxy_user, proxy_password);
    
    }
  
  pthread_mutex_unlock(&proxy_mutex);
  
  /* */
  
  c->host     = gavl_strrep(c->host, host);
  c->protocol = gavl_strrep(c->protocol, protocol);
  c->port     = port;
  
  if(port <= 0)
    {
    if(c->flags & FLAG_USE_TLS)
      port = 443;
    else
      port = 80;
    c->port     = port;
    }
    
  ret = 1;
  
  end:

  if(!proxy_user)
    free(proxy_user);
  if(!proxy_password)
    free(proxy_password);
  
  return ret;
  
  }

static void setup_connect_header(gavl_io_t * io, gavl_dictionary_t * ret)
  {
  char * dst_host;
  gavl_http_client_t * c = gavl_io_get_priv(io);

  dst_host = gavl_sprintf("%s:%d", c->host, c->port);
        
  gavl_http_request_init(ret, "CONNECT", dst_host, "HTTP/1.1");
  if(c->proxy_auth)
    gavl_dictionary_set_string(ret, "Proxy-Authorization",
                               c->proxy_auth);
        
  free(dst_host);
  }



static void append_header_var(gavl_dictionary_t * header, const char * name, const char * val)
  {
  if(!gavl_dictionary_get_string_i(header, name))
    gavl_dictionary_set_string(header, name, val);
  }

static void prepare_header(gavl_http_client_t * c,
                           gavl_dictionary_t * request,
                           const char * host,
                           int port,
                           const char * path)
  {
  if(c->flags & FLAG_USE_PROXY)
    {
    char * real_uri = NULL;
    /* Rename gmerlin specific protocols */

    if(gavl_string_starts_with(c->uri, "hls://"))
      {
      real_uri = gavl_sprintf("http%s", strstr(c->real_uri, "://"));
      }
    else if(gavl_string_starts_with(c->uri, "hlss://"))
      {
      real_uri = gavl_sprintf("https%s", strstr(c->real_uri, "://"));
      }
      
    if(real_uri)
      {
      gavl_http_request_init(request, c->method, real_uri, "HTTP/1.1");
      free(real_uri);
      }
    else
      gavl_http_request_init(request, c->method, c->real_uri, "HTTP/1.1");
    
    if(c->proxy_auth)
      gavl_dictionary_set_string(request, "Proxy-Authorization",
                                 c->proxy_auth);
    }
  else
    gavl_http_request_init(request, c->method, path, "HTTP/1.1");
  
  /* Append http variables from URI */
  gavl_dictionary_merge2(request, &c->vars_from_uri);
  gavl_dictionary_merge2(request, &c->vars);

  if(!gavl_dictionary_get_string_i(request, "Host"))
    {
    if(port > 0)
      gavl_dictionary_set_string_nocopy(request, "Host", gavl_sprintf("%s:%d", host, port));
    else
      gavl_dictionary_set_string(request, "Host", host);
    }
  
  /* Send standard headers */
  append_header_var(request, "Accept", "*/*");
  append_header_var(request, "User-Agent", "gavl/"VERSION);

  if(c->req_body && c->req_body->len)
    gavl_dictionary_set_int(request, "Content-Length", c->req_body->len);
  
  if((c->range_start > 0) || (c->range_end > 0))
    {
    if(c->range_end <= 0)
      gavl_dictionary_set_string_nocopy(request, "Range", gavl_sprintf("bytes=%"PRId64"-", c->range_start));
    else
      gavl_dictionary_set_string_nocopy(request, "Range", gavl_sprintf("bytes=%"PRId64"-%"PRId64,
                                                                        c->range_start, c->range_end - 1));

    //    fprintf(stderr, "Set range: %s\n", gavl_dictionary_get_string(request, "Range"));
    }

#if 0
  if(c->range_start > 0)
    gavl_dictionary_dump(request, 2);
#endif
  }

static int handle_response(gavl_io_t * io)
  {
  int ret = 0;
  int status;
  gavl_http_client_t * c = gavl_io_get_priv(io);

  c->flags &= ~(FLAG_GOT_REDIRECTION|FLAG_RESPONSE_BODY_DONE);
  
  status = gavl_http_response_get_status_int(&c->resp);

  if(gavl_http_response_is_chunked(&c->resp))
    c->flags |= FLAG_CHUNKED;
  else
    c->flags &= ~FLAG_CHUNKED;
    
  if(gavl_http_response_has_body(&c->resp))
    c->flags |= FLAG_HAS_RESPONSE_BODY;
  else
    c->flags &= ~FLAG_HAS_RESPONSE_BODY;

  if(!c->total_bytes)
    gavl_dictionary_get_long_i(&c->resp, "Content-Length", &c->total_bytes);
  
  if(status < 200)
    {
    /* Not yet supported */
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "HTTP status %d not yet supported", status);
    goto fail;
    }
  else if(status < 300)
    {
    if((status == 200) || (status == 206))
      {
      /* Get variables */
      const char * var;
      int flags;
      int64_t resp_range_start = 0;
      int64_t resp_range_end = 0;
      int64_t resp_range_total = 0;

      c->position = 0;
      
      if(status == 206)
        {
        var = gavl_dictionary_get_string_i(&c->resp, "Content-Range");

        if(!var)
          goto fail;
        
        if(!gavl_string_starts_with(var, "bytes "))
          goto fail;
        
        var += 6;

        while(isspace(*var) && (*var != '\0'))
          var++;

        if(sscanf(var, "%"PRId64"-%"PRId64"/%"PRId64, &resp_range_start, 
                  &resp_range_end, &resp_range_total) < 3)
          {
          goto fail;
          }

        if(c->range_end <= 0)
          c->total_bytes = resp_range_total;
        
        if(c->range_end <= 0)
          c->position = resp_range_start;
        }
      
      if(!strcmp(c->method, "GET"))
        {
#if 0
        if(!(c->flags & FLAG_CHUNKED) && (!c->total_bytes))
          {
          if((status == 200) || (c->range_end > 0))
            gavl_dictionary_get_long_i(&c->resp, "Content-Length", &c->total_bytes);
          else if(status == 206)
            c->total_bytes = resp_range_total;
          }
#endif   
        if(c->flags & FLAG_HAS_RESPONSE_BODY)
          flags = GAVL_IO_CAN_READ;
        
        if((var = gavl_dictionary_get_string_i(&c->resp, "Accept-Ranges")) &&
           !strcasecmp(var, "bytes") && (c->total_bytes > 0))
          flags |= GAVL_IO_CAN_SEEK;
        }
      else if(!strcmp(c->method, "PUT")) // Not supported yet
        flags = GAVL_IO_CAN_WRITE;
      
      if(!(c->flags & FLAG_HAS_RESPONSE_BODY))
        check_keepalive(c);

      if(!strcmp(c->method, "GET") || !strcmp(c->method, "PUT"))
        {
        gavl_io_set_info(io, c->total_bytes, c->uri,
                         gavl_dictionary_get_string_i(&c->resp, "Content-Type"),
                         flags);
        }
      }
    else
      goto fail;
    }
  else if(status < 400)
    {
    char * new_uri = NULL;
    /* Redirection */
    const char * location;

    if(c->num_redirections > 5)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Too many redirections: %d %s", status,
               gavl_http_response_get_status_str(&c->resp));
      goto fail;
      }
    c->num_redirections++;
    
    location = gavl_dictionary_get_string_i(&c->resp, "Location");
    
    if(location)
      {
      if(!c->real_uri)
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Bug: Need to create an absolute uri but real_uri is not set");
        goto fail;
        }
      new_uri = gavl_get_absolute_uri(location, c->real_uri);
      }
    else
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Location header missing, status %d %s", status,
               gavl_http_response_get_status_str(&c->resp));
      goto fail;
      }
    
    if(c->redirect_uri)
      free(c->redirect_uri);

    gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Got http redirection (%d) to %s", c->num_redirections, new_uri);
    
    c->redirect_uri = new_uri;
    c->real_uri = c->redirect_uri;
    c->flags |= FLAG_GOT_REDIRECTION;
    
    gavl_dictionary_set_string(gavl_io_info(c->io), GAVL_META_REAL_URI, c->real_uri);

    if(!(c->flags & FLAG_HAS_RESPONSE_BODY))
      check_keepalive(c);
    }
  else
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got HTTP error: %d %s", status,
             gavl_http_response_get_status_str(&c->resp));
    
    gavl_dictionary_dump(&c->resp, 2);
    
    fprintf(stderr, "Request was:\n");
    
    gavl_dictionary_dump(&c->req, 2);
    
    if(!(c->flags & FLAG_HAS_RESPONSE_BODY))
      check_keepalive(c);

    //    if(status == 416)
    //      fprintf(stderr, "Blupp\n");
    
    c->flags |= FLAG_ERROR;
    }

  ret = 1;
  fail:

  return ret;
  
  }


void
gavl_http_client_set_req_vars(gavl_io_t * io,
                              const gavl_dictionary_t * vars)
  {
  gavl_http_client_t * c = gavl_io_get_priv(io);
  gavl_dictionary_reset(&c->vars);
  gavl_dictionary_copy(&c->vars, vars);
  }

void
gavl_http_client_set_request_body(gavl_io_t * io,
                                  gavl_buffer_t * buf)
  {
  gavl_http_client_t * c = gavl_io_get_priv(io);
  c->req_body = buf;
  }

void
gavl_http_client_set_response_body(gavl_io_t * io,
                                   gavl_buffer_t * buf)
  {
  gavl_http_client_t * c = gavl_io_get_priv(io);
  c->res_body = buf;
  }

const gavl_dictionary_t *
gavl_http_client_get_response(gavl_io_t * io)
  {
  gavl_http_client_t * c = gavl_io_get_priv(io);
  return &c->resp;
  }

static int
reopen(gavl_io_t * io)
  {
  int result;

  gavl_http_client_t * c = gavl_io_get_priv(io);

  c->real_uri = c->uri;
  
  while(1)
    {
    result = gavl_http_client_run_async_done(io, 3000);

    if(result < 0)
      return 0;
    else if(result > 0)
      return 1;
    
    }
  return 0;
  }


int
gavl_http_client_open(gavl_io_t * io,
                      const char * method,
                      const char * uri1)
  {
  int timeout = 10*1000; // 10 seconds
  int result;
  gavl_http_client_t * c = gavl_io_get_priv(io);

  c->flags &= ~FLAG_GOT_REDIRECTION;
  c->num_redirections = 0;

  gavl_io_clear_eof(io);
  gavl_io_clear_error(io);
  
  if(!gavl_http_client_run_async(io, method, uri1))
    return 0;
  
  if(c->res_body)
    timeout = 10*60*1000;
  
  result = gavl_http_client_run_async_done(io, timeout);
  
  if(result > 0)
    return 1;
  else
    return 0;
    
  return 1;
  }

int
gavl_http_client_read_body(gavl_io_t * io,
                           gavl_buffer_t * buf)
  {
  int ret = 0;
  int bytes_read;
  gavl_http_client_t * c = gavl_io_get_priv(io);
  
  if(c->flags & FLAG_CHUNKED)
    {
    while(1)
      {
      if(!read_chunk_length(c))
        goto fail;
      if(c->chunk_length)
        {
        gavl_buffer_alloc(buf, buf->len + c->chunk_length + 1);

        bytes_read = gavl_io_read_data(c->io_int, buf->buf + buf->len, 
                                       c->chunk_length);

        if(bytes_read < c->chunk_length)
          {
          /* Error */
          goto fail;
          }
        buf->len += bytes_read;
        }
      
      if(!read_chunk_tail(c))
        goto fail;
      
      if(!c->chunk_length)
        break;
      }
    
    }
  else if(c->total_bytes > 0)
    {
    gavl_buffer_alloc(buf, buf->len + c->total_bytes + 1);
    bytes_read = gavl_io_read_data(c->io_int, buf->buf + buf->len, 
                                   c->total_bytes);
    if(bytes_read < c->total_bytes)
      {
      /* Error */
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got %d bytes, wanted %"PRId64,
               bytes_read, c->total_bytes);
      goto fail;
      }
    buf->len += bytes_read;
    }
  else
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Body has zero length");
    goto fail;
    }
  
  ret = 1;
  fail:

  if(buf->buf)
    buf->buf[buf->len] = '\0';
  
  if(!ret)
    {
    gavl_io_set_error(io);
    }
  
  return ret;
  }

void
gavl_http_client_set_range(gavl_io_t * io, int64_t start, int64_t end)
  {
  gavl_http_client_t * c = gavl_io_get_priv(io);
  //  fprintf(stderr, "gavl_http_client_set_range %"PRId64" %"PRId64"\n", start, end);
  c->range_start = start;
  c->range_end   = end;
  
  }

int gavl_http_client_can_pause(gavl_io_t * io)
  {
  return gavl_io_can_seek(io);
  }

void gavl_http_client_pause(gavl_io_t * io)
  {
  gavl_http_client_t * c = gavl_io_get_priv(io);

  //  fprintf(stderr, "gavl_http_client_pause %"PRId64"\n", c->position);

  c->state = STATE_PAUSED;
  
  close_connection(c);
  
  //  gavl_io_destroy(c->io_int);
  //  c->io_int = NULL;
  }

void gavl_http_client_resume(gavl_io_t * io)
  {
  gavl_http_client_t * c = gavl_io_get_priv(io);

  //  fprintf(stderr, "gavl_http_client_resume %"PRId64"\n", c->position);
  
  /* Resume after seek */
  if(c->state != STATE_PAUSED)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot resume: client was not paused");
    return;
    }
  
  gavl_http_client_set_range(io, c->position, -1);

  c->state = STATE_START;
  
  if(!reopen(io))
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Re-opening failed");
  //  else
  //    fprintf(stderr, "Re-opened %"PRId64"\n", c->position);
  
  }

/* Asynchronous states */


static int send_buffer_async(gavl_io_t * io,
                             gavl_buffer_t * buf, int timeout)
  {
  int result;

  //  fprintf(stderr, "send_buffer_async 1  %d\n", timeout);

  if(buf->pos == buf->len)
    return 1;
  
  if((timeout > 0) && !gavl_io_can_write(io, timeout))
    return 0;
  
  //  fprintf(stderr, "send_buffer_async 2 %d\n", buf->len - buf->pos);
  result = gavl_io_write_data_nonblock(io, buf->buf + buf->pos, buf->len - buf->pos);
  
  if(result <= 0)
    {
    fprintf(stderr, "send_buffer_async result: %d, timeout was: %d, bytes: %d\n",
            result, timeout, buf->len - buf->pos);
    }
  if(result <= 0)
    return result;
  else
    {
    buf->pos += result;
    if(buf->pos == buf->len)
      return 1;
    }
  
  return 0;
  }

int gavl_http_client_run_async(gavl_io_t * io,
                               const char * method,
                               const char * uri)
  {
  gavl_http_client_t * c = gavl_io_get_priv(io);

  /* Clear earlier redirections */
  c->redirect_uri = gavl_strrep(c->redirect_uri, NULL);
  c->flags &= ~FLAG_GOT_REDIRECTION;
  
  c->state = STATE_START;

  gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Opening %s method: %s", uri, method);

  //  if(gavl_string_ends_with(uri, "webvtt"))
  //    fprintf(stderr, "Blah");
  
  gavl_dictionary_reset(&c->vars_from_uri);
  gavl_dictionary_reset(&c->req);
  gavl_dictionary_reset(&c->resp);
  
  c->method = gavl_strrep(c->method, method);
  c->uri = gavl_strrep(c->uri, uri);
  c->uri = gavl_url_extract_http_vars(c->uri, &c->vars_from_uri);
  /* Needs to be set again after close_connection or reset_connection */
  c->real_uri = c->uri;

  //  fprintf(stderr, "gavl_http_client_run_async %s %s\n", c->uri, c->real_uri);
  
  return 1;
  }

static int async_iteration(gavl_io_t * io, int timeout)
  {
  int result;
  gavl_http_client_t * c = gavl_io_get_priv(io);

  c->flags &= ~FLAG_WAIT;
  
  //  fprintf(stderr, "gavl_http_client_run_async_done %d %d\n", c->state, c->chunk_header_buffer.len);
  
  /* Initialize */
  
  if(c->state == STATE_START)
    {
    char * protocol = NULL;
    char * host = NULL;
    char * path = NULL;
    int port = -1;

    if(!gavl_url_split(c->real_uri,
                       &protocol,
                       NULL,
                       NULL,
                       &host,
                       &port,
                       &path))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Invalid URI: %s", c->real_uri);
      return -1;
      }
    
    if(!path)
      path = gavl_strdup("/");
    
    if(!prepare_connection(io, host, port, protocol))
      {
      if(protocol)
        free(protocol);
      if(host)
        free(host);
      if(path)
        free(path);
      return -1;
      }
    prepare_header(c, &c->req, host, port, path);

    if(protocol)
      free(protocol);
    if(host)
      free(host);
    if(path)
      free(path);
    
    if(!c->io_int)
      {
      /* Resolve */

      if(c->flags & (FLAG_USE_PROXY | FLAG_USE_PROXY_TUNNEL))
        gavl_socket_address_set_async(c->addr, c->proxy_host, c->proxy_port, SOCK_STREAM);
      else
        gavl_socket_address_set_async(c->addr, c->host, c->port, SOCK_STREAM);

      c->flags |= FLAG_WAIT;
      
      c->state = STATE_RESOLVE;
      return 0;
      }
    else
      {
      c->state = STATE_SEND_REQUEST;
      gavl_buffer_reset(&c->header_buffer);
      }
    }

  if(c->state == STATE_RESOLVE)
    {
    result =
      gavl_socket_address_set_async_done(c->addr, timeout);

    if(!result)
      c->flags |= FLAG_WAIT;
    
    if(result <= 0)
      return result;
    else
      {
      c->state = STATE_CONNECT;
      c->fd = gavl_socket_connect_inet(c->addr, 0);
      if(c->fd < 0)
        return -1;
      }

    c->flags |= FLAG_WAIT;
    return 0;
    }

  if(c->state == STATE_CONNECT)
    {
    result = gavl_socket_connect_inet_complete(c->fd, timeout);

    if(!result)
      {
      //      fprintf(stderr, "Waiting for connection %d\n", timeout);
      c->flags |= FLAG_WAIT;
      return 0;
      }
    else if(result < 0)
      return result;
    
    if(c->flags & FLAG_USE_PROXY_TUNNEL)
      {
      gavl_dictionary_t req;
      gavl_dictionary_init(&req);

      c->io_proxy = gavl_io_create_socket(c->fd, 5000, 0);
      
      setup_connect_header(io, &req);

#ifdef DUMP_HEADERS
      fprintf(stderr, "Sending connect command\n");
      gavl_dictionary_dump(&req, 2);
#endif // DUMP_HEADERS
      
      gavl_buffer_reset(&c->header_buffer);
      gavl_http_request_to_buffer(&req, &c->header_buffer);
      
      c->state = STATE_SEND_CONNECT_REQUEST;
      gavl_dictionary_free(&req);
      }
    else
      {
      gavl_buffer_reset(&c->header_buffer);

      if(c->flags & FLAG_USE_TLS)
        {
        c->io_int = gavl_io_create_tls_client_async(c->fd, c->host, GAVL_IO_SOCKET_DO_CLOSE);
        c->state = STATE_TLS_HANDSHAKE;
        }
      else
        {
        c->io_int = gavl_io_create_socket(c->fd, 5000, GAVL_IO_SOCKET_DO_CLOSE);
        c->state = STATE_SEND_REQUEST;
        }
      c->fd = -1;

      if(!c->io_int)
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not create internal I/O handle");
        return -1;
        }
      }
    }
  
  if(c->state == STATE_SEND_CONNECT_REQUEST)
    {
    result = send_buffer_async(c->io_proxy, &c->header_buffer, timeout);

    if(!result)
      c->flags |= FLAG_WAIT;
    
    if(result <= 0)
      return result;

    c->state = STATE_READ_CONNECT_RESPONSE;
    gavl_buffer_reset(&c->header_buffer);
    }
  
  if(c->state == STATE_READ_CONNECT_RESPONSE)
    {
    result = gavl_http_response_read_async(c->io_proxy,
                                               &c->header_buffer,
                                               &c->resp, timeout);

    if(!result)
      c->flags |= FLAG_WAIT;
    
    if(result <= 0)
      return result;

    gavl_io_destroy(c->io_proxy);
    c->io_proxy = NULL;

#ifdef DUMP_HEADERS
    fprintf(stderr, "Got connect response\n");
    gavl_dictionary_dump(&c->resp, 2);
#endif // DUMP_HEADERS
    
    /* Check response status */
    if(gavl_http_response_get_status_int(&c->resp) != 200)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Establishing proxy tunnel failed: %d %s",
               gavl_http_response_get_status_int(&c->resp),
               gavl_http_response_get_status_str(&c->resp));
      
      gavl_dictionary_reset(&c->resp);
      return -1;
      }
    
    c->io_int = gavl_io_create_tls_client_async(c->fd, c->host, GAVL_IO_SOCKET_DO_CLOSE);
    c->state = STATE_TLS_HANDSHAKE;
    
    c->fd = -1;
    
    gavl_buffer_reset(&c->header_buffer);
    gavl_dictionary_reset(&c->resp);
    }
  
  if(c->state == STATE_TLS_HANDSHAKE)
    {
    result = gavl_io_create_tls_client_async_done(c->io_int, timeout);

    if(!result)
      c->flags |= FLAG_WAIT;

    if(result <= 0)
      return result;
    
    c->state = STATE_SEND_REQUEST;
    }
  
  if(c->state == STATE_SEND_REQUEST)
    {
    if(!c->header_buffer.len)
      {
      // Create request header 
      gavl_http_request_to_buffer(&c->req, &c->header_buffer);

#ifdef DUMP_HEADERS
        gavl_dprintf("Sending request %d\n", c->header_buffer.len);
        gavl_dictionary_dump(&c->req, 2);
#endif
      }
    
    result = send_buffer_async(c->io_int, &c->header_buffer, timeout);

    //    fprintf(stderr, "Got result: %d %d\n", result, c->header_buffer.len);

    if(!result)
      c->flags |= FLAG_WAIT;
    
    if(result <= 0)
      {
      if(result < 0)
        {
        if(c->flags & FLAG_USE_KEEPALIVE)
          {
          gavl_log(GAVL_LOG_WARNING, LOG_DOMAIN, "Keep-alive connection closed, reopening");
          close_connection(c);
          c->state = STATE_START;
          return 0;
          }
        else
          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Sending request failed");
        }
      return result;
      }
#ifdef DUMP_HEADERS
    gavl_dprintf("Sent request\n");
#endif
    
    gavl_buffer_reset(&c->header_buffer);

    if(c->req_body && c->req_body->len)
      c->state = STATE_SEND_BODY;
    else
      c->state = STATE_READ_RESPONSE;
    }
  
  if(c->state == STATE_SEND_BODY)
    {
    result = send_buffer_async(c->io_int, c->req_body, timeout);

    //    fprintf(stderr, "Got result: %d %d\n", result, c->header_buffer.len);

    if(!result)
      c->flags |= FLAG_WAIT;
    
    if(result <= 0)
      {
      if(result < 0)
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Sending request body failed");
      return result;
      }
    c->state = STATE_READ_RESPONSE;
    }
  
  if(c->state == STATE_READ_RESPONSE)
    {
    result = gavl_http_response_read_async(c->io_int,
                                           &c->header_buffer,
                                           &c->resp, timeout);
    
    if(!result)
      c->flags |= FLAG_WAIT;

    if(result <= 0)
      return result;

#ifdef DUMP_HEADERS
    fprintf(stderr, "Got response\n");
    gavl_dictionary_dump(&c->resp, 2);
#endif
    
    if(!handle_response(io))
      return -1;
    
    /* Handle redirection */

    if(c->flags & FLAG_GOT_REDIRECTION)
      {
      c->state = STATE_START;
      return 0;
      }

    if(!(c->flags & FLAG_HAS_RESPONSE_BODY))
      {
      c->state = STATE_COMPLETE;
      check_keepalive(c);
      return (c->flags & FLAG_ERROR) ? -1 : 1;
      }

    if(c->res_body)
      gavl_buffer_reset(c->res_body);
    
    if(c->flags & FLAG_CHUNKED)
      {
      gavl_buffer_reset(&c->chunk_header_buffer);
      c->state = STATE_READ_CHUNK_HEADER;
      }
    else
      {
      c->state = STATE_READ_BODY_NORMAL;
      
      if(c->res_body)
        {
        if(c->total_bytes <= 0)
          {
          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
                   "Cannot read body: No Content-Length given and no chunked encoding.");
          gavl_dictionary_dump(&c->resp, 2);
          return -1;
          }
        gavl_buffer_alloc(c->res_body, c->total_bytes);
        }
      }

    /* Body will be read with gavl_io_read_data() or gavl_io_read_data_nonblock() */
    if(!c->res_body)
      return (c->flags & FLAG_ERROR) ? -1 : 1;
    
    //    return 0;
    }

  if(c->state == STATE_READ_BODY_NORMAL)
    {
    /* Should never happen */
    if(!c->res_body)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,
               "BUG: Asynchronous reading requested but no buffer of the body given");
      return -1;
      }
    
    if(timeout >= 0)
      {
      if(!gavl_io_can_read(c->io_int, timeout))
        {
        if(!timeout)
          {
          c->flags |= FLAG_WAIT;
          return 0;
          }
        else
          {
          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got read timeout");
          return -1;
          }
        }
      }
    
    result = gavl_io_read_data_nonblock(c->io_int, c->res_body->buf + c->res_body->len,
                                        c->total_bytes - c->res_body->len);

    // fprintf(stderr, "Got result: %d %"PRId64" %d (timeout: %d)\n", result, c->total_bytes, c->res_body->len, timeout);
    
    if(!result && (timeout >= 0))
      {
      return handle_remote_close(c);
      }
    else if(result < 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Reading data failed");
      return -1;
      }
    c->res_body->len += result;

    if(c->res_body->len == c->total_bytes)
      {
      c->state = STATE_COMPLETE;
      c->flags |= FLAG_RESPONSE_BODY_DONE;
      check_keepalive(c);
      return (c->flags & FLAG_ERROR) ? -1 : 1;
      }
    /* Not yet enough */
    c->flags |= FLAG_WAIT;
    return 0;
    }
  
  if(c->state == STATE_READ_CHUNK_HEADER)
    {
    result = read_chunk_header_async(c, timeout);

    if(!result)
      c->flags |= FLAG_WAIT;
    
    if(result <= 0)
      return result;
    
    if(!c->chunk_length)
      {
      c->state = STATE_READ_CHUNK_TAIL;
      gavl_buffer_reset(&c->chunk_header_buffer);
      gavl_buffer_alloc(&c->chunk_header_buffer, 2);
      }
    else
      {
      c->state = STATE_READ_CHUNK;
      c->pos   = 0;
      gavl_buffer_alloc(c->res_body, c->res_body->len + c->chunk_length);
      }
    }

  if(c->state == STATE_READ_CHUNK)
    {
    int tries = 0;
    
    if(!gavl_io_can_read(c->io_int, timeout))
      {
      if(!timeout)
        {
        c->flags |= FLAG_WAIT;
        return 0;
        }
      else
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Read timeout");
        return -1;
        }
      }
    
    while(1)
      {
      result = gavl_io_read_data_nonblock(c->io_int, c->res_body->buf + c->res_body->len,
                                          c->chunk_length - c->pos);

      //      fprintf(stderr, "Read chunk %d %"PRId64" %"PRId64" %d\n",
      //              c->chunk_length, c->pos, c->chunk_length - c->pos, result);

      if(!result)
        {
        if(!tries && (timeout >= 0))
          return handle_remote_close(c);
        else
          c->flags |= FLAG_WAIT;
        }
      if(result <= 0)
        return result;

      c->res_body->len += result;
      c->pos += result;

      if(c->pos == c->chunk_length)
        {
        c->state = STATE_READ_CHUNK_TAIL;
        gavl_buffer_reset(&c->chunk_header_buffer);
        gavl_buffer_alloc(&c->chunk_header_buffer, 2);
        }
      tries++;
      }
    }

  if(c->state == STATE_READ_CHUNK_TAIL)
    {
    result = read_chunk_tail_async(c, timeout);

    if(!result)
      c->flags |= FLAG_WAIT;
    
    if(result <= 0)
      return result;
    
    if(!c->chunk_length) /* Tail after a zero length chunk: body finished */
      {
      c->state = STATE_COMPLETE;
      c->flags |= FLAG_RESPONSE_BODY_DONE;
      check_keepalive(c);
      return (c->flags & FLAG_ERROR) ? -1 : 1;
      }
    else
      {
      gavl_buffer_reset(&c->chunk_header_buffer);
      c->state = STATE_READ_CHUNK_HEADER;
      return 0;
      }
    }
  
  if(c->state == STATE_COMPLETE)
    {
    //  fprintf(stderr, "async_iteration done %d\n", !!(c->flags & FLAG_ERROR));
    return (c->flags & FLAG_ERROR) ? -1 : 1;
    }
  gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Unknown state %d", c->state);
  /* Unknown state */
  return -1;
  }

int gavl_http_client_run_async_done(gavl_io_t * io, int timeout)
  {
  int result;
  gavl_time_t timeout_remaining = 0;
  gavl_time_t time_cur;
  
  gavl_http_client_t * c = gavl_io_get_priv(io);

  // fprintf(stderr, "gavl_http_client_run_async_done %d\n", c->state);
  
  if(timeout > 0)
    {
    if(!c->timer)
      c->timer = gavl_timer_create();
    
    timeout_remaining = (int64_t)timeout * 1000;

    gavl_timer_start(c->timer);

    }
  
  while(1)
    {
    result = async_iteration(io, timeout_remaining / 1000);
    
    if(!result)
      {
      /* Waiting for socket */
      if(!timeout && (c->flags & FLAG_WAIT))
        return 0;

      if(timeout > 0)
        {
        time_cur = gavl_timer_get(c->timer);
        timeout_remaining = (int64_t)timeout * 1000 - time_cur;

        if(timeout_remaining <= 0)
          return 0;
        }
      
      }
    else
      return result;
    }
  return 0;
  }


#if defined(__GNUC__) && defined(__ELF__)
static void cleanup_proxies() __attribute__ ((destructor));
 
static void cleanup_proxies()
  {
  if(http_proxy)
    free(http_proxy);
  if(https_proxy)
    free(https_proxy);
  gavl_array_free(&no_proxy);
  }

#endif
