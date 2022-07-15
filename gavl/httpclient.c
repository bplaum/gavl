
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <config.h>

#include <gavl/gavf.h>
#include <gavl/http.h>
#include <gavl/log.h>
#include <gavl/gavlsocket.h>
#include <gavfprivate.h>

#define LOG_DOMAIN "httpclient"

#define FLAG_USE_PROXY        (1<<0)
#define FLAG_KEEPALIVE        (1<<1)
#define FLAG_CHUNKED          (1<<2)
#define FLAG_USE_PROXY_TUNNEL (1<<3)

typedef struct
  {
  gavf_io_t * io_int;
  
  char * host;
  char * protocol;
  int port;
  
  gavl_dictionary_t resp;

  gavl_socket_address_t * addr;

  int chunk_length;

  int64_t position;
  int64_t total_bytes;
  
  int64_t pos;
  int started;

  /* Set by gavl_http_client_open() */
  gavl_dictionary_t vars;
  char * uri;
  char * method;

  int flags;
  
  char * proxy_auth;
  } gavl_http_client_t;

pthread_mutex_t proxy_mutex = PTHREAD_MUTEX_INITIALIZER;
static int proxies_initialized = 0;
static char * http_proxy = NULL;
static char * https_proxy = NULL;
static gavl_array_t no_proxy;
static int no_tunnel = 1;

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

#if 0
static void check_proxy(const char * host, int port, int tls, int * use_proxy)
  {
  const char * proxy = NULL;
  
  pthread_mutex_lock(&proxy_mutex);
  init_proxies();

  if(tls)
    proxy = https_proxy;
  else
    proxy = http_proxy;
  
  pthread_mutex_unlock(&proxy_mutex);
  }
#endif

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

  if(c->flags & FLAG_CHUNKED)
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
    if(c->total_bytes)
      {
      if(c->pos + len > c->total_bytes)
        len = c->total_bytes - c->pos;

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

static int64_t seek_http(void * priv, int64_t pos1, int whence)
  {
  int64_t pos = -1;
  gavl_http_client_t * c = priv;

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
  if(pos < 0)
    return c->position;
  else if(pos >= c->total_bytes)
    return c->position;
  else
    {
    /* TODO */
    }
  
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
                   seek_http,
                   close_func,
                   /* gavf_flush_func f */ NULL,
                   GAVF_IO_CAN_READ | GAVF_IO_CAN_WRITE,
                   c);
  
  gavf_io_set_poll_func(ret, poll_func);
  
  return ret;
  }


static int
do_connect(gavf_io_t * io,
           const char * host,
           int port,
           const char * protocol)
  {
  int ret = 0;
  int do_close = 0;
  int use_tls = 0;
  const char * proxy = NULL;
  int i;

  char * proxy_host = NULL;
  int proxy_port = -1;

  char * proxy_user = NULL;
  char * proxy_password = NULL;
  
  gavl_dictionary_t req;
  gavl_dictionary_t res;
  
  gavl_http_client_t * c = gavf_io_get_priv(io);

  

  gavl_dictionary_init(&req);
  gavl_dictionary_init(&res);
  
  c->flags = 0;

  c->proxy_auth = gavl_strrep(c->proxy_auth, NULL);
  
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
    use_tls = 1;
    }
  else
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Unknown protocol %s", protocol);
      goto fail;
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
    if(use_tls)
      {
      if(!no_tunnel)
        {
        c->flags = FLAG_USE_PROXY_TUNNEL;
        gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Using https proxy %s (tunneling mode)", proxy);
        }
      else
        {
        c->flags = FLAG_USE_PROXY;
        gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Using https proxy %s (http mode)", proxy);
        use_tls = 0;
        }
      }
    else
      {
      c->flags = FLAG_USE_PROXY;
      gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Using http proxy %s", proxy);
      }
    
    /* TODO: Proxy user and password */
    if(!gavl_url_split(proxy,
                       NULL,
                       &proxy_user,
                       &proxy_password,
                       &proxy_host,
                       &proxy_port,
                       NULL))
      goto fail;

    if(proxy_user && proxy_password)
      c->proxy_auth = gavl_make_basic_auth(proxy_user, proxy_password);
    
    }
  
  pthread_mutex_unlock(&proxy_mutex);

  
  /* */
  
  if(gavf_io_got_error(io))
    {
    do_close = 1;
    gavf_io_clear_error(io);
    }
  else if(!(c->flags & FLAG_KEEPALIVE))
    do_close = 1;
  
  if(!do_close && c->io_int && c->host && c->protocol && !(c->flags & FLAG_USE_PROXY))
    {
    if(strcmp(host, c->host) || strcmp(protocol, c->protocol) || (c->port != port))
      {
      do_close = 1;
      gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Closing keepalive connection (adress or protocol changed)");
      }
    }
  
  if(c->io_int && do_close)
    {
    gavf_io_destroy(c->io_int);
    c->io_int = NULL;
    }

  c->host     = gavl_strrep(c->host, host);
  c->protocol = gavl_strrep(c->protocol, protocol);
  c->port     = port;
  
  if(!c->io_int)
    {
    int fd;

    if(port <= 0)
      {
      if(use_tls)
        port = 443;
      else
        port = 80;
      c->port     = port;
      }
    
    if(c->flags & (FLAG_USE_PROXY | FLAG_USE_PROXY_TUNNEL))
      gavl_socket_address_set(c->addr, proxy_host, proxy_port, SOCK_STREAM);
    else
      gavl_socket_address_set(c->addr, c->host, c->port, SOCK_STREAM);
    
    fd = gavl_socket_connect_inet(c->addr, 5000);
    if(fd < 0)
      goto fail;
    
    if(use_tls)
      {
      if(c->flags & FLAG_USE_PROXY_TUNNEL)
        {
        int result = 0;
        char * dst_host;
        gavf_io_t * io = gavf_io_create_socket(fd, 5000, 0);
        
        /* Establish https tunnel */

        dst_host = gavl_sprintf("%s:%d", c->host, c->port);
        
        gavl_http_request_init(&req, "CONNECT", dst_host, "HTTP/1.1");
        if(c->proxy_auth)
          gavl_dictionary_set_string(&req, "Proxy-Authorization",
                                     c->proxy_auth);
        
        free(dst_host);
        
        if(!gavl_http_request_write(io, &req))
          {
          gavf_io_destroy(io);
          goto fail;
          }
        fprintf(stderr, "Sent request\n");
        gavl_dictionary_dump(&req, 2);
        
        if(!gavl_http_response_read(io, &res))
          {
          gavf_io_destroy(io);
          goto fail;
          }
        
        if(gavl_http_response_get_status_int(&res) != 200)
          {
          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Establishing proxy tunnel failed: %d %s",
                   gavl_http_response_get_status_int(&res),
                   gavl_http_response_get_status_str(&res));
          }
        else
          result = 1;
        
        gavf_io_destroy(io);
        
        if(!result)
          goto fail;
        }
      
      c->io_int = gavf_io_create_tls_client(fd, c->host, GAVF_IO_SOCKET_DO_CLOSE);
      }
    else
      c->io_int = gavf_io_create_socket(fd, 5000, GAVF_IO_SOCKET_DO_CLOSE);
    
    }
  ret = 1;
  fail:

  gavl_dictionary_free(&req);
  gavl_dictionary_free(&res);
  
  if(proxy_host)
    free(proxy_host);
  if(proxy_user)
    free(proxy_user);
  if(proxy_password)
    free(proxy_password);
  
  return ret;
  }

static int
gavl_http_client_send_request(gavf_io_t * io,
                              const gavl_dictionary_t * request,
                              gavl_dictionary_t ** resp)
  {
  gavl_http_client_t * c = gavf_io_get_priv(io);
  
  c->total_bytes = 0;
  c->position = 0;
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

static void append_header_var(gavl_dictionary_t * header, const char * name, const char * val)
  {
  if(!gavl_dictionary_get_string_i(header, name))
    gavl_dictionary_set_string(header, name, val);
  }

static int
http_client_open(gavf_io_t * io,
                 const char * method,
                 const char * uri,
                 const gavl_dictionary_t * vars, // http variables
                 char ** redirect)
  {
  int ret = 0;
  char * protocol = NULL;
  char * host = NULL;
  char * path = NULL;
  int port = 0;
  gavl_dictionary_t request;
  //  char * uri;
  gavl_dictionary_t * resp;
  int status;
  gavl_http_client_t * c = gavf_io_get_priv(io);

  //  fprintf(stderr, "http_client_open %s\n", uri1);
  
  //  uri = gavl_strdup(uri1);
  
  gavl_dictionary_init(&request);
  
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

  if(!do_connect(io, host, port, protocol))
    goto fail;

  if(c->flags & FLAG_USE_PROXY)
    {
    char * real_uri = NULL;
    /* Rename gmerlin specific protocols */

    if(gavl_string_starts_with(uri, "hls://"))
      {
      real_uri = gavl_sprintf("http%s", strstr(uri, "://"));
      }
    else if(gavl_string_starts_with(uri, "hlss://"))
      {
      real_uri = gavl_sprintf("https%s", strstr(uri, "://"));
      }
      
    if(real_uri)
      {
      gavl_http_request_init(&request, method, real_uri, "HTTP/1.1");
      free(real_uri);
      }
    else
      gavl_http_request_init(&request, method, uri, "HTTP/1.1");
    
    if(c->proxy_auth)
      gavl_dictionary_set_string(&request, "Proxy-Authorization",
                                 c->proxy_auth);
    }
  else
    gavl_http_request_init(&request, method, path, "HTTP/1.1");
  
  if(port > 0)
    gavl_dictionary_set_string_nocopy(&request, "Host", gavl_sprintf("%s:%d", host, port));
  else
    gavl_dictionary_set_string(&request, "Host", host);
  
  /* Append http variables from URI */
  if(vars)
    gavl_dictionary_merge2(&request, vars);

  /* Send standard headers */
  append_header_var(&request, "Accept", "*/*");
  append_header_var(&request, "User-Agent", "gavl/"VERSION);
  
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
      int flags;

      if(!strcmp(method, "GET"))
        {
        var = gavl_dictionary_get_string_i(resp, "Transfer-Encoding");
        if(var && !strcasecmp(var, "chunked"))
          {
          c->flags |= FLAG_CHUNKED;
          }
        else
          {
          c->flags &= ~FLAG_CHUNKED;
          gavl_dictionary_get_long(resp, "Content-Length", &c->total_bytes);
          }
        
        flags = GAVF_IO_CAN_READ;

        if((var = gavl_dictionary_get_string_i(resp, "Accept-Ranges")) &&
           !strcasecmp(var, "bytes") && (c->total_bytes > 0))
          flags |= GAVF_IO_CAN_SEEK;
        
        }
      else // PUT?
        flags = GAVF_IO_CAN_WRITE;

      if(check_keepalive(resp))
        c->flags |= FLAG_KEEPALIVE;

      gavf_io_set_info(io, c->total_bytes, uri,
                       gavl_dictionary_get_string_i(resp, "Content-Type"),
                       flags);
      }
    else
      goto fail;
    }
  else if(status < 400)
    {
    /* Redirection */
    const char * location = gavl_dictionary_get_string_i(resp, "Location");

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

  
  if(protocol)
    free(protocol);
  if(host)
    free(host);
  if(path)
    free(path);
  
  gavl_dictionary_free(&request);
  
  return ret;
  }

int
gavl_http_client_open(gavf_io_t * io,
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

  if(vars && (vars != &c->vars))
    {
    gavl_dictionary_reset(&c->vars);
    gavl_dictionary_copy(&c->vars, vars);
    }

  if(!c->method || strcmp(c->method, method))
    c->method = gavl_strrep(c->method, method);
  
  if(!c->uri || strcmp(c->uri, uri1))
    c->uri = gavl_strrep(c->uri, uri1);
  
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
      gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Got http redirection to %s", uri);
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
  
  if(c->flags & FLAG_CHUNKED)
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
  else if(c->total_bytes > 0)
    {
    gavl_buffer_alloc(buf, buf->len + c->total_bytes + 1);
    bytes_read = gavf_io_read_data(c->io_int, buf->buf + buf->len, 
                                   c->total_bytes);
    if(bytes_read < c->total_bytes)
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
