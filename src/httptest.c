
#include <stdlib.h>
#include <stdio.h>

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/http.h>
#include <gavl/log.h>
#define LOG_DOMAIN "httptest"

int main(int argc, char ** argv)
  {
  gavl_io_t * c;
  gavl_buffer_t buf;
  
  if(argc == 1)
    {
    fprintf(stderr, "Usage: %s uri\n", argv[0]);
    return EXIT_FAILURE;
    }

  gavl_buffer_init(&buf);
  
  c = gavl_http_client_create();

  gavl_http_client_set_response_body(c, &buf);
  
  if(!gavl_http_client_open(c, "GET", argv[1]))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "http GET failed");
    return EXIT_FAILURE;
    }
  
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Downladed http body %d bytes", buf.len);
  gavl_buffer_free(&buf);
  
  }
