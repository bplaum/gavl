
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
char * gavl_http_request_to_string(const gavl_dictionary_t * req, int * lenp);

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
const char * gavl_http_response_get_protocol(gavl_dictionary_t * res);

GAVL_PUBLIC
const int gavl_http_response_get_status_int(gavl_dictionary_t * res);

GAVL_PUBLIC
const char * gavl_http_response_get_status_str(gavl_dictionary_t * res);

GAVL_PUBLIC
void gavl_http_header_set_empty_var(gavl_dictionary_t * h, const char * name);

GAVL_PUBLIC
void gavl_http_header_set_date(gavl_dictionary_t * h, const char * name);

