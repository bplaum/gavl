
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include <config.h>

#include <gavl/gavf.h>
#include <gavl/http.h>
#include <gavl/log.h>
#include <gavl/gavlsocket.h>
#include <gavfprivate.h>

#define LOG_DOMAIN "httpclient"

typedef struct
  {
  gavf_io_t * io_int;
  
  char * host;
  char * protocol;
  int port;
  
  gavl_dictionary_t resp;

  int chunked;
  gavl_socket_address_t * addr;

  int chunk_length;
  int64_t content_length;
  int64_t pos;
  int started;

  int keepalive;
  
  } gavl_http_client_t;

static int read_chunk_length(gavl_http_client_t * c)
  {
  int ret = 0;
  char * length_str = NULL;
  int length_alloc = 0;
  
  /* Read length */
  if(!gavf_io_read_line(c->io_int, &length_str,
                        &length_alloc, 10000) ||
     (sscanf(length_str, "%x", &c->chunk_length) < 1))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Reading chunk length failed");
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
  
  if((gavf_io_read_data(c->io_int, crlf, 2) < 2) ||
     (crlf[0] != '\r') ||
     (crlf[1] != '\n'))
    return 0;
  return 1;
  }

static int read_func(void * priv, uint8_t * data, int len)
  {
  int bytes_read = 0;
  int bytes_to_read = 0;
  int result;
  gavl_http_client_t * c = priv;

  if(c->chunked)
    {
    while(bytes_read < len)
      {
      if(c->pos == c->chunk_length)
        {
        if(c->chunk_length)
          {
          if(!read_chunk_tail(c))
            return bytes_read;
          }
      
        /* Read chunk_len */

        if(!read_chunk_length(c))
          {
          goto fail;
          }
        c->pos = 0;
        }

      bytes_to_read = len - bytes_read;

      if(c->pos + bytes_to_read >= c->chunk_length)
        bytes_to_read = c->chunk_length - c->pos;

      result = gavf_io_read_data(c->io_int, data + bytes_read, bytes_to_read);

      if(result < bytes_to_read)
        {
        goto fail;
        }
      bytes_read += result;
      c->pos += result;
      }
    }
  else
    {
    if(c->content_length)
      {
      if(c->pos + len > c->content_length)
        len = c->content_length - c->pos;

      if(len)
        {
        bytes_read = gavf_io_read_data(c->io_int, data, len);

        if(bytes_read < len)
          {
          goto fail;
          }
        
        c->pos += len;
        }
      
      return len;
      }
    else
      return gavf_io_read_data(c->io_int, data, len);
    }

  return bytes_read;
  
  fail:

  return -1;
  
  }

static int write_func(void * priv, const uint8_t * data, int len)
  {
  //  gavl_http_client_t * c = priv;
  return 0;
  }

static int poll_func(void * priv, int timeout)
  {
  gavl_http_client_t * c = priv;
  
  return gavf_io_can_read(c->io_int, timeout);
  }

static void close_func(void * priv)
  {
  gavl_http_client_t * c = priv;

  if(c->io_int)
    gavf_io_destroy(c->io_int);

  if(c->addr)
    gavl_socket_address_destroy(c->addr);

  if(c->host)
    free(c->host);
  if(c->protocol)
    free(c->protocol);

  gavl_dictionary_free(&c->resp);
  free(c);
  }




gavf_io_t * gavl_http_client_create()
  {
  gavl_http_client_t * c;
  gavf_io_t * ret;
  
  c = calloc(1, sizeof(*c));
  c->addr = gavl_socket_address_create();

  ret = 
    gavf_io_create(read_func,
                   write_func,
                   /* gavf_seek_func  s */ NULL,
                   close_func,
                   /* gavf_flush_func f */ NULL,
                   GAVF_IO_CAN_READ | GAVF_IO_CAN_WRITE,
                   c);
  
  gavf_io_set_poll_func(ret, poll_func);
  
  return ret;
  }


int
gavl_http_client_open(gavf_io_t * io,
                      const char * host,
                      int port,
                      const char * protocol)
  {
  int ret = 0;
  int do_close = 0;
  int use_tls = 0;
  gavl_http_client_t * c = gavf_io_get_priv(io);

  if(gavf_io_got_error(io))
    {
    do_close = 1;
    gavf_io_clear_error(io);
    }
  else if(!c->keepalive)
    do_close = 1;
  
  if(!do_close && c->io_int && c->host && c->protocol)
    {
    if(strcmp(host, c->host) || strcmp(protocol, c->protocol) || (c->port != port))
      do_close = 1;
    }
  
  if(c->io_int && do_close)
    {
    gavf_io_destroy(c->io_int);
    c->io_int = NULL;
    }

  if(!c->io_int)
    {
    int fd;
    
    if(!strcmp(protocol, "http") ||
       !strcmp(protocol, "hls"))
      {
      if(port <= 0)
        port = 80;
      }
    else if(!strcmp(protocol, "https") ||
            !strcmp(protocol, "hlss"))
      {
      use_tls = 1;
      if(port <= 0)
        port = 443;
      }
    else
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Unknown protocol %s", protocol);
      goto fail;
      }
    
    gavl_socket_address_set(c->addr, host, port, SOCK_STREAM);
    c->host = gavl_strrep(c->host, host);
    
    fd = gavl_socket_connect_inet(c->addr, 5000);
    if(fd < 0)
      goto fail;
    
    if(use_tls)
      c->io_int = gavf_io_create_tls_client(fd, c->host, GAVF_IO_SOCKET_DO_CLOSE);
    else
      c->io_int = gavf_io_create_socket(fd, 5000, GAVF_IO_SOCKET_DO_CLOSE);
    
    }
  ret = 1;
  fail:

  return ret;
  }

int
gavl_http_client_send_request(gavf_io_t * io,
                              const gavl_dictionary_t * request,
                              gavl_dictionary_t ** resp)
  {
  gavl_http_client_t * c = gavf_io_get_priv(io);
  
  c->keepalive = 0;
  c->content_length = 0;
  c->chunked = 0;
  c->chunk_length = 0;
  c->pos = 0;
  gavl_dictionary_reset(&c->resp);
  gavf_io_clear_eof(io);
  
  if(!gavl_http_request_write(c->io_int, request))
    return 0;

  if(!gavl_http_response_read(c->io_int, &c->resp))
    return 0;

  *resp = &c->resp;
  return 1;
  }

static int check_keepalive(gavl_dictionary_t * res)
  {
  const char * protocol;
  const char * var;
  
  protocol = gavl_http_response_get_protocol(res);

  /* Under HTTP/1.1, connections are keep-alive by default */
  if(!strcmp(protocol, "HTTP/1.1"))
    {
    if((var = gavl_dictionary_get_string(res, "Connection")) &&
       !strcasecmp(var, "close"))
      return 0;
    else
      return 1;
    }
  else
    {
    if((var = gavl_dictionary_get_string(res, "Connection")) &&
       !strcasecmp(var, "keep-alive"))
      return 1;
    else
      return 0;
    }
  return 0;
  }


static int
http_client_open(gavf_io_t * io,
                 const char * method,
                 const char * uri1,
                 const gavl_dictionary_t * vars, // http variables
                 char ** redirect)
  {
  int ret = 0;
  char * protocol = NULL;
  char * host = NULL;
  char * path = NULL;
  int port = 0;
  gavl_dictionary_t request;
  gavl_dictionary_t url_vars;
  char * uri;
  gavl_dictionary_t * resp;
  int status;
  gavl_http_client_t * c = gavf_io_get_priv(io);

  uri = gavl_strdup(uri1);
  
  gavl_dictionary_init(&request);
  gavl_dictionary_init(&url_vars);
  
  if(!gavl_url_split(uri,
                     &protocol,
                     NULL,
                     NULL,
                     &host,
                     &port,
                     &path))
    {
    goto fail;
    }

  if(!gavl_http_client_open(io, host, port, protocol))
    goto fail;

  gavl_http_request_init(&request, method, path, "HTTP/1.1");

  if(port > 0)
    gavl_dictionary_set_string_nocopy(&request, "Host", gavl_sprintf("%s:%d", host, port));
  else
    gavl_dictionary_set_string(&request, "Host", host);
  
  /* Append http variables from URI */
  if(vars)
    gavl_dictionary_merge2(&request, vars);

  if(!gavl_http_client_send_request(io, &request, &resp))
    {
    goto fail;
    }
  
  status = gavl_http_response_get_status_int(resp);

  if(status < 200)
    {
    /* Not yet supported */
    goto fail;
    }
  else if(status < 300)
    {
    if(status == 200)
      {
      /* Get variables */

      const char * var;
  
      var = gavl_dictionary_get_string_i(resp, "Transfer-Encoding");
      if(var && !strcasecmp(var, "chunked"))
        {
        c->chunked = 1;
        }
      else
        {
        c->chunked = 0;
        gavl_dictionary_get_long(resp, "Content-Length", &c->content_length);
        }
      c->keepalive = check_keepalive(resp);

      gavf_io_set_info(io, c->content_length, uri1,
                       gavl_dictionary_get_string_i(resp, "Content-Type"));
      }
    else
      goto fail;
    }
  else if(status < 400)
    {
    /* Redirection */
    const char * location = gavl_dictionary_get_string(resp, "Location");

    if(location)
      *redirect = gavl_get_absolute_uri(location, uri);
    else
      goto fail;
    }
  else
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got HTTP error: %d %s", status,
             gavl_http_response_get_status_str(resp));

    gavl_dictionary_dump(&request, 2);

    goto fail;
    }
  ret = 1;
  fail:

  if(uri)
    free(uri);
  
  if(protocol)
    free(protocol);
  if(host)
    free(host);
  if(path)
    free(path);

  gavl_dictionary_free(&request);
  gavl_dictionary_free(&url_vars);
  
  return ret;
  
  }

int
gavl_http_client_open_full(gavf_io_t * io,
                           const char * method,
                           const char * uri1,
                           const gavl_dictionary_t * vars,
                           gavl_dictionary_t ** resp)
  {
  int i;
  int done;
  int ret = 0;
  char * redirect;
  gavl_dictionary_t http_vars;
  char * uri = gavl_strdup(uri1);
  gavl_http_client_t * c = gavf_io_get_priv(io);
  
  gavl_dictionary_init(&http_vars);

  uri = gavl_url_extract_http_vars(uri, &http_vars);

  
  //  fprintf(stderr, "Opening %s\n", uri1);
  //  fprintf(stderr, "Got http variables:\n");
  //  gavl_dictionary_dump(&http_vars, 2);

  if(vars)
    gavl_dictionary_merge2(&http_vars, vars);
  
  for(i = 0; i < 5; i++)
    {
    redirect = NULL;
    
    if(!http_client_open(io, method, uri, &http_vars, &redirect))
      goto fail;

    if(redirect)
      {
      free(uri);
      uri = redirect;
      }
    else
      {
      done = 1;
      break;
      }
    }
  
  if(!done)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Too many redirections");
    goto fail;
    }

  if(resp)
    *resp = &c->resp;
  
  ret = 1;
  
  fail:

  
  free(uri);
  gavl_dictionary_free(&http_vars);
  
  return ret;
  
  }

int
gavl_http_client_read_body(gavf_io_t * io,
                           gavl_buffer_t * buf)
  {
  int ret = 0;
  int bytes_read;
  gavl_http_client_t * c = gavf_io_get_priv(io);
  
  if(c->chunked)
    {
    while(1)
      {
      if(!read_chunk_length(c))
        goto fail;
      if(c->chunk_length)
        {
        gavl_buffer_alloc(buf, buf->len + c->chunk_length + 1);

        bytes_read = gavf_io_read_data(c->io_int, buf->buf + buf->len, 
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
  else if(c->content_length > 0)
    {
    gavl_buffer_alloc(buf, buf->len + c->content_length + 1);
    bytes_read = gavf_io_read_data(c->io_int, buf->buf + buf->len, 
                                   c->content_length);
    if(bytes_read < c->content_length)
      {
      /* Error */
      goto fail;
      }
    buf->len += bytes_read;
    }
  else
    {
    goto fail;
    }
  
  ret = 1;
  fail:

  buf->buf[buf->len] = '\0';
  
  if(!ret)
    {
    gavf_io_set_error(io);
    }
  
  return ret;
  }


