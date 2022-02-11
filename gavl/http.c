
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include <config.h>

#include <gavl/gavf.h>
#include <gavl/http.h>
#include <gavl/log.h>

#define LOG_DOMAIN "gavl.http"

// #define DUMP_REQUESTS_READ



/* Utils */

int gavl_http_parse_vars_line(gavl_dictionary_t * m, char * line)
  {
  char * pos;

  if(*line == '\0') // Final line
    return 1;
  
  pos = strchr(line, ':');
  if(!pos)
    return 0;
  *pos = '\0';
  pos++;
  while(isspace(*pos) && (*pos != '\0'))
    pos++;
  if(*pos == '\0')
    gavl_dictionary_set_string(m, line, GAVL_HTTP_META_EMPTY);
  else
    gavl_dictionary_set_string(m, line, pos);
  return 1;
  }

static int read_vars(gavf_io_t * io, char ** line, int * line_alloc,
                     gavl_dictionary_t * m)
  {
  while(1)
    {
    if(!gavf_io_read_line(io, line, line_alloc, 1024))
      return 0;
    
    //     fprintf(stderr, "Got line: %s\n", *line);
    
    if(**line == '\0')
      return 1;
    
    if(!gavl_http_parse_vars_line(m, *line))
      return 0;
    }
  return 1;
  }

static void put_string(gavf_io_t * io, const char * str)
  {
  gavf_io_write_data(io, (const uint8_t *)str, strlen(str));
  }

static void write_vars_func(void * priv, const char * name, const gavl_value_t * val)
  {
  char * buf = NULL;
  gavf_io_t * io;
  io = priv;

  if(*name == '$')
    return;
  
  switch(val->type)
    {
    case GAVL_TYPE_INT:
      buf = gavl_sprintf("%d", val->v.i);
      break;
    case GAVL_TYPE_LONG:
      buf = gavl_sprintf("%"PRId64, val->v.l);
      break;
    case GAVL_TYPE_FLOAT:
      buf = gavl_sprintf("%f", val->v.d);
      break;
    case GAVL_TYPE_STRING:
      if(!strcmp(val->v.str, GAVL_HTTP_META_EMPTY))
        {
        put_string(io, name);
        put_string(io, ":\r\n");
        }
      else
        {
        put_string(io, name);
        put_string(io, ": ");
        put_string(io, val->v.str);
        put_string(io, "\r\n");
        }
      break;
    default:
      fprintf(stderr, "Type %s not supported in http headers\n",
             gavl_type_to_string(val->type));
    }

  if(buf)
    {
    put_string(io, name);
    put_string(io, ": ");
    put_string(io, buf);
    put_string(io, "\r\n");
    free(buf);
    }
  
  }

static void write_vars(gavf_io_t * io, const gavl_dictionary_t * m)
  {
  gavl_dictionary_foreach(m, write_vars_func, io);
  }

 
/* Request */

void gavl_http_request_init(gavl_dictionary_t * req,
                            const char * method,
                            const char * path,
                            const char * protocol)
  {
  gavl_dictionary_init(req);
  gavl_dictionary_set_string(req, GAVL_HTTP_META_METHOD, method);
  gavl_dictionary_set_string(req, GAVL_HTTP_META_PATH, path);
  gavl_dictionary_set_string(req, GAVL_HTTP_META_PROTOCOL, protocol);
  }

void gavl_http_request_set_path(gavl_dictionary_t * req,
                              const char * path)
  {
  gavl_dictionary_set_string(req, GAVL_HTTP_META_PATH, path);
  }


/* Read a http request (server) */

static int parse_request_line(gavl_dictionary_t * req, char * line)
  {
  char * pos1, *pos2;
  pos1 = strchr(line, ' ');
  pos2 = strrchr(line, ' ');

  if(!pos1 || !pos2 || (pos1 == pos2))
    return 0;
  
  gavl_dictionary_set_string_nocopy(req, GAVL_HTTP_META_METHOD,
                                   gavl_strndup( line, pos1));
  gavl_dictionary_set_string_nocopy(req, GAVL_HTTP_META_PATH,
                                   gavl_strndup( pos1+1, pos2));
  gavl_dictionary_set_string_nocopy(req, GAVL_HTTP_META_PROTOCOL,
                                   gavl_strdup(pos2+1));
  return 1;
  }

int gavl_http_request_read(gavf_io_t * io,
                           gavl_dictionary_t * req)
  {
  int result = 0;
  char * line = NULL;
  int line_alloc = 0;
  
  if(!gavf_io_read_line(io, &line, &line_alloc, 1024))
    goto fail;

  //  fprintf(stderr, "Got line: %s\n", line);
  //  gavl_hexdump((uint8_t*)line, strlen(line)+1, 16);
  
  if(!parse_request_line(req, line))
    goto fail;
  
  result = read_vars(io, &line, &line_alloc, req);

#ifdef DUMP_REQUESTS_READ
  gavl_dprintf("Got HTTP request\n");
  gavl_dictionary_dump(req, 2);
#endif
  
  //  log_request(req);

  fail:
  
  if(line)
    free(line);

  return result;

  }

static int http_request_write(gavf_io_t * io,
                                   const gavl_dictionary_t * req)
  {
  const char * method;
  const char * path;
  const char * protocol;
  
  if(!(method = gavl_dictionary_get_string(req, GAVL_HTTP_META_METHOD)) ||
     !(path = gavl_dictionary_get_string(req, GAVL_HTTP_META_PATH)) ||
     !(protocol = gavl_dictionary_get_string(req, GAVL_HTTP_META_PROTOCOL)))
    return 0;

  put_string(io, method);
  put_string(io, " ");
  put_string(io, path);
  put_string(io, " ");
  put_string(io, protocol);
  put_string(io, "\r\n");
  
  write_vars(io, req);
  put_string(io, "\r\n");
  gavf_io_flush(io);
  return 1;
  }


char * gavl_http_request_to_string(const gavl_dictionary_t * req, int * lenp)
  {
  gavf_io_t * io;
  uint8_t * buf;
  char * ret;
  int len;
  
  io = gavf_io_create_mem_write();

  if(!http_request_write(io, req))
    {
    gavf_io_destroy(io);
    return NULL;
    }
  
  buf = gavf_io_mem_get_buf(io, &len);
  
  ret = malloc(len + 1);
  memcpy(ret, buf, len);
  ret[len] = '\0';
  
  if(lenp)
    *lenp = len;

  free(buf);
  gavf_io_destroy(io);
  
  return ret;
  }

int gavl_http_request_write(gavf_io_t * io,
                            const gavl_dictionary_t * req)
  {
  char * buf;
  int len;
  int ret;
  
  if(!(buf = gavl_http_request_to_string(req, &len)))
    return 0;

  ret = gavf_io_write_data(io, (uint8_t*)buf, len);
  
  free(buf);
  
  if(ret == len)
    return 1;
  else
    return 0;
  }


const char * gavl_http_request_get_protocol(const gavl_dictionary_t * req)
  {
  return gavl_dictionary_get_string(req, GAVL_HTTP_META_PROTOCOL);
  }

const char * gavl_http_request_get_method(const gavl_dictionary_t * req)
  {
  return gavl_dictionary_get_string(req, GAVL_HTTP_META_METHOD);
  }

const char * gavl_http_request_get_path(const gavl_dictionary_t * req)
  {
  return gavl_dictionary_get_string(req, GAVL_HTTP_META_PATH);
  }


/* Response */

static int parse_response_line(gavl_dictionary_t * res, char * line)
  {
  char * pos1;
  char * pos2;

  pos1 = strchr(line, ' ');
  if(!pos1)
    return 0;

  pos2 = strchr(pos1+1, ' ');
  
  if(!pos2)
    return 0;
  
  gavl_dictionary_set_string_nocopy(res, GAVL_HTTP_META_PROTOCOL,
                          gavl_strndup( line, pos1));
  gavl_dictionary_set_string_nocopy(res, GAVL_HTTP_META_STATUS_INT,
                          gavl_strndup( pos1+1, pos2));

  pos2++;
  gavl_dictionary_set_string(res, GAVL_HTTP_META_STATUS_STR, pos2);
  return 1;
  }

void gavl_http_response_init(gavl_dictionary_t * res,
                           const char * protocol,
                           int status_int, const char * status_str)
  {
  gavl_dictionary_init(res);
  gavl_dictionary_set_string(res, GAVL_HTTP_META_PROTOCOL, protocol);
  gavl_dictionary_set_int(res, GAVL_HTTP_META_STATUS_INT, status_int);
  gavl_dictionary_set_string(res, GAVL_HTTP_META_STATUS_STR, status_str);
  
  }

int gavl_http_response_read(gavf_io_t * io,
                            gavl_dictionary_t * res)
  {
  int result = 0;
  char * line = NULL;
  int line_alloc = 0;
  
  if(!gavf_io_read_line(io, &line, &line_alloc, 1024))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Reading http status line failed"); 
    goto fail;
    }
  
  if(!parse_response_line(res, line))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Parsing http status line \"%s\" failed", line); 
    goto fail;
    }
  if(!read_vars(io, &line, &line_alloc, res))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Reading http response variables failed"); 
    goto fail;
    }
  result = 1;
  fail:
  if(line)
    free(line);
  
  return result;
  }

int gavl_http_response_write(gavf_io_t * io,
                             const gavl_dictionary_t * res)
  {
  int status_int;
  const char * status_str;
  const char * protocol;


  char tmp_string[128];
  
  if(!gavl_dictionary_get_int(res, GAVL_HTTP_META_STATUS_INT, &status_int) ||
     !(status_str = gavl_dictionary_get_string(res, GAVL_HTTP_META_STATUS_STR)) ||
     !(protocol = gavl_dictionary_get_string(res, GAVL_HTTP_META_PROTOCOL)))
    return 0;


  put_string(io, protocol);
  put_string(io, " ");

  snprintf(tmp_string, 128, "%d", status_int);
  put_string(io, tmp_string);
  put_string(io, " ");
  put_string(io, status_str);
  put_string(io, "\r\n");
  
  write_vars(io, res);
  
  put_string(io, "\r\n");

  return 1;
  }


char * gavl_http_response_to_string(const gavl_dictionary_t * res, int * lenp)
  {
  gavf_io_t * io;
  uint8_t * buf;
  char * ret;
  int len;
  
  io = gavf_io_create_mem_write();

  gavl_http_response_write(io, res);
  
  buf = gavf_io_mem_get_buf(io, &len);
  
  ret = malloc(len + 1);
  memcpy(ret, buf, len);
  ret[len] = '\0';
  
  if(lenp)
    *lenp = len;

  free(buf);
  gavf_io_destroy(io);
  
  return ret;

  
  }



int gavl_http_response_from_string(gavl_dictionary_t * res, const char * buf)
  {
  int i;
  char * pos;
  int result = 0;
  char ** str = gavl_strbreak(buf, '\n');

  i = 0;
  while(str[i])
    {
    if((pos = strchr(str[i], '\r')))
      *pos = '\0';
    i++;
    }

  if(!parse_response_line(res, str[0]))
    goto fail;
  
  i = 1;
  while(str[i])
    {
    if(!gavl_http_parse_vars_line(res, str[i]))
      goto fail;
    i++;
    }

  result = 1;
  fail:
  gavl_strbreak_free(str);
  
  return result;
  }


const char * gavl_http_response_get_protocol(gavl_dictionary_t * res)
  {
  return gavl_dictionary_get_string(res, GAVL_HTTP_META_PROTOCOL);
  }

const int gavl_http_response_get_status_int(gavl_dictionary_t * res)
  {
  int ret = 0;
  gavl_dictionary_get_int(res, GAVL_HTTP_META_STATUS_INT, &ret);
  return ret;
  }

const char * gavl_http_response_get_status_str(gavl_dictionary_t * res)
  {
  return gavl_dictionary_get_string(res, GAVL_HTTP_META_STATUS_STR);
  }

void gavl_http_header_set_empty_var(gavl_dictionary_t * h, const char * name)
  {
  gavl_dictionary_set_string(h, name, GAVL_HTTP_META_EMPTY);
  }

void gavl_http_header_set_date(gavl_dictionary_t * h, const char * name)
  {
  char date[30];
  time_t curtime = time(NULL);
  struct tm tm;
  
  strftime(date, 30,"%a, %d %b %Y %H:%M:%S GMT", gmtime_r(&curtime, &tm));
  gavl_dictionary_set_string(h, name, date);
  }

int gavl_http_request_from_string(gavl_dictionary_t * req, const char * buf)
  {
  int i;
  char * pos;
  int result = 0;
  char ** str = gavl_strbreak(buf, '\n');

  i = 0;
  while(str[i])
    {
    if((pos = strchr(str[i], '\r')))
      *pos = '\0';
    i++;
    }

  if(!parse_request_line(req, str[0]))
    goto fail;
  
  i = 1;
  while(str[i])
    {
    if(!gavl_http_parse_vars_line(req, str[i]))
      goto fail;
    i++;
    }
  result = 1;
  fail:
  gavl_strbreak_free(str);
  return result;
  }
