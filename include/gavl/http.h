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


#ifndef GAVLHTTP_H_INCLUDED
#define GAVLHTTP_H_INCLUDED

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
int gavl_http_read_body(gavl_io_t * io, const gavl_dictionary_t * res, gavl_buffer_t * buf);

GAVL_PUBLIC
void gavl_http_request_init(gavl_dictionary_t * req,
                            const char * method,
                            const char * path,
                            const char * protocol);

GAVL_PUBLIC
int gavl_http_request_read(gavl_io_t * io,
                           gavl_dictionary_t * req);

GAVL_PUBLIC
int gavl_http_request_write(gavl_io_t * io,
                            const gavl_dictionary_t * req);

GAVL_PUBLIC
int gavl_http_request_write_async(gavl_io_t * io,
                                  const gavl_dictionary_t * req,
                                  gavl_buffer_t * buf);

GAVL_PUBLIC
int gavl_http_request_write_async_done(gavl_io_t * io,
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
int gavl_http_response_read(gavl_io_t * io,
                            gavl_dictionary_t * res);

GAVL_PUBLIC
int gavl_http_response_read_async(gavl_io_t * io,
                                  gavl_buffer_t * buf,
                                  gavl_dictionary_t * res, int timeout);

GAVL_PUBLIC
int gavl_http_response_write(gavl_io_t * io,
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
gavl_io_t * gavl_http_client_create(void);

GAVL_PUBLIC int
gavl_http_client_read_body(gavl_io_t *,
                           gavl_buffer_t * buf);

/* Call before gavl_http_client_open() to specify a byte range */
GAVL_PUBLIC void
gavl_http_client_set_range(gavl_io_t * io, int64_t start, int64_t end);

GAVL_PUBLIC
int gavl_http_client_get_state(gavl_io_t * io);

/*
 *  Call before gavl_http_client_open() to specify extra header
 *  variables
 */

GAVL_PUBLIC void
gavl_http_client_set_req_vars(gavl_io_t * io,
                              const gavl_dictionary_t * vars);

/* Set request and response bodies */
GAVL_PUBLIC void
gavl_http_client_set_request_body(gavl_io_t * io,
                                  gavl_buffer_t * buf);

GAVL_PUBLIC void
gavl_http_client_set_response_body(gavl_io_t * io,
                                   gavl_buffer_t * buf);


GAVL_PUBLIC const gavl_dictionary_t *
gavl_http_client_get_response(gavl_io_t * io);

/* Open a connection, send a request and read the response.
   Handles redirections (300 codes), proxies, https */

GAVL_PUBLIC int
gavl_http_client_open(gavl_io_t * io,
                      const char * method,
                      const char * uri1);

GAVL_PUBLIC int
gavl_http_client_can_pause(gavl_io_t * io);

GAVL_PUBLIC void
gavl_http_client_pause(gavl_io_t * io);

GAVL_PUBLIC void
gavl_http_client_resume(gavl_io_t * io);

/* Asynchronous operation */

GAVL_PUBLIC int
gavl_http_client_run_async(gavl_io_t * io, const char * method, const char * uri);

GAVL_PUBLIC int
gavl_http_client_run_async_done(gavl_io_t * io, int timeout);


GAVL_PUBLIC
char * gavl_make_basic_auth(const char * username, const char * password);

#endif // GAVLHTTP_H_INCLUDED
