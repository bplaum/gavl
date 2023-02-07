#ifndef GAVLHTTP_H_INCLUDED
#define GAVLHTTP_H_INCLUDED

#include <gavl/gavf.h>

/* Special variables for the first line of the HTTP requests and responses */

#define GAVL_HTTP_META_PROTOCOL   "$PROTOCOL"
#define GAVL_HTTP_META_PATH       "$PATH"
#define GAVL_HTTP_META_METHOD     "$METHOD"
#define GAVL_HTTP_META_STATUS_INT "$STATUS_INT"
#define GAVL_HTTP_META_STATUS_STR "$STATUS_STR"
#define GAVL_HTTP_META_EMPTY      "$EMPTY"

/*
 *  http utility routines
 *
 *  The main reaon this code is here is because it's shared between gmerlin
 *  and gmerlin_avdecoder
 */
 
/* Request */

GAVL_PUBLIC
int gavl_http_read_body(gavf_io_t * io, const gavl_dictionary_t * res, gavl_buffer_t * buf);

GAVL_PUBLIC
void gavl_http_request_init(gavl_dictionary_t * req,
                            const char * method,
                            const char * path,
                            const char * protocol);

GAVL_PUBLIC
int gavl_http_request_read(gavf_io_t * io,
                           gavl_dictionary_t * req);

GAVL_PUBLIC
int gavl_http_request_write(gavf_io_t * io,
                            const gavl_dictionary_t * req);

GAVL_PUBLIC
int gavl_http_request_write_async(gavf_io_t * io,
                                  const gavl_dictionary_t * req,
                                  gavl_buffer_t * buf);

GAVL_PUBLIC
int gavl_http_request_write_async_done(gavf_io_t * io,
                                       gavl_buffer_t * buf, int timeout);


GAVL_PUBLIC
char * gavl_http_request_to_string(const gavl_dictionary_t * req, int * lenp);

GAVL_PUBLIC
int gavl_http_request_to_buffer(const gavl_dictionary_t * req, gavl_buffer_t * ret);


GAVL_PUBLIC
void gavl_http_request_set_path(gavl_dictionary_t * req,
                                const char * path);

GAVL_PUBLIC
const char * gavl_http_request_get_protocol(const gavl_dictionary_t * req);

GAVL_PUBLIC
const char * gavl_http_request_get_method(const gavl_dictionary_t * req);

GAVL_PUBLIC
const char * gavl_http_request_get_path(const gavl_dictionary_t * req);

/* Response */


GAVL_PUBLIC
void gavl_http_response_init(gavl_dictionary_t * res,
                           const char * protocol,
                           int status_int, const char * status_str);


GAVL_PUBLIC
int gavl_http_response_read(gavf_io_t * io,
                            gavl_dictionary_t * res);

GAVL_PUBLIC
int gavl_http_response_read_async(gavf_io_t * io,
                                  gavl_buffer_t * buf,
                                  gavl_dictionary_t * res, int timeout);

GAVL_PUBLIC
int gavl_http_response_write(gavf_io_t * io,
                             const gavl_dictionary_t * res);




GAVL_PUBLIC
char * gavl_http_response_to_string(const gavl_dictionary_t * res, int * lenp);

GAVL_PUBLIC
int gavl_http_response_from_string(gavl_dictionary_t * res, const char * buf);

GAVL_PUBLIC
int gavl_http_request_from_string(gavl_dictionary_t * req, const char * buf);

GAVL_PUBLIC
int gavl_http_parse_vars_line(gavl_dictionary_t * m, char * line);


GAVL_PUBLIC
const char * gavl_http_response_get_protocol(const gavl_dictionary_t * res);

GAVL_PUBLIC
const int gavl_http_response_get_status_int(const gavl_dictionary_t * res);

GAVL_PUBLIC
const char * gavl_http_response_get_status_str(const gavl_dictionary_t * res);

GAVL_PUBLIC
int gavl_http_response_has_body(const gavl_dictionary_t * res);

GAVL_PUBLIC
int gavl_http_response_is_chunked(const gavl_dictionary_t * res);

GAVL_PUBLIC
void gavl_http_header_set_empty_var(gavl_dictionary_t * h, const char * name);

GAVL_PUBLIC
void gavl_http_header_set_date(gavl_dictionary_t * h, const char * name);

/* http client */

GAVL_PUBLIC
gavf_io_t * gavl_http_client_create();


GAVL_PUBLIC int
gavl_http_client_read_body(gavf_io_t *,
                           gavl_buffer_t * buf);

/* Call before gavl_http_client_open() to specify a byte range */
GAVL_PUBLIC void
gavl_http_client_set_range(gavf_io_t * io, int64_t start, int64_t end);

/*
 *  Call before gavl_http_client_open() to specify extra header
 *  variables
 */

GAVL_PUBLIC void
gavl_http_client_set_req_vars(gavf_io_t * io,
                              const gavl_dictionary_t * vars);

/* Set request and response bodies */
GAVL_PUBLIC void
gavl_http_client_set_request_body(gavf_io_t * io,
                                  gavl_buffer_t * buf);

GAVL_PUBLIC void
gavl_http_client_set_response_body(gavf_io_t * io,
                                   gavl_buffer_t * buf);


GAVL_PUBLIC const gavl_dictionary_t *
gavl_http_client_get_response(gavf_io_t * io);

/* Open a connection, send a request and read the response.
   Handles redirections (300 codes), proxies, https */

GAVL_PUBLIC int
gavl_http_client_open(gavf_io_t * io,
                      const char * method,
                      const char * uri1);

GAVL_PUBLIC int
gavl_http_client_can_pause(gavf_io_t * io);

GAVL_PUBLIC void
gavl_http_client_pause(gavf_io_t * io);

GAVL_PUBLIC void
gavl_http_client_resume(gavf_io_t * io);

/* Asynchronous operation */

GAVL_PUBLIC int
gavl_http_client_run_async(gavf_io_t * io, const char * method, const char * uri);

GAVL_PUBLIC int
gavl_http_client_run_async_done(gavf_io_t * io, int timeout);

/* URL variables */

GAVL_PUBLIC
void gavl_url_get_vars_c(const char * path,
                         gavl_dictionary_t * vars);

GAVL_PUBLIC
void gavl_url_get_vars(char * path,
                       gavl_dictionary_t * vars);

GAVL_PUBLIC
char * gavl_url_append_vars(char * path,
                            const gavl_dictionary_t * vars);


/* Append/Remove http variables from the URL */

GAVL_PUBLIC
char * gavl_url_append_http_vars(char * url, const gavl_dictionary_t * vars);

GAVL_PUBLIC
char * gavl_url_extract_http_vars(char * url, gavl_dictionary_t * vars);

GAVL_PUBLIC
char * gavl_make_basic_auth(const char * username, const char * password);

#endif // GAVLHTTP_H_INCLUDED
