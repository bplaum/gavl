#include <stdlib.h>
#include <string.h>
#include <gavl/gavl.h>
#include <gavl/utils.h>

#include <nettle/base64.h>

char * 
gavl_base64_encode_data(void * data, int length)
  {
  char * ret;
  struct base64_encode_ctx ctx;
  int len_enc;
  base64url_encode_init(&ctx);

  ret = malloc(BASE64_ENCODE_LENGTH(length) + BASE64_ENCODE_FINAL_LENGTH + 1);

  len_enc = base64_encode_update(&ctx, ret, length, data);
  len_enc += base64_encode_final(&ctx, ret + len_enc);
  ret[len_enc] = '\0';
  return ret;
  }

int
gavl_base64_decode_data(const char * str, gavl_buffer_t * ret)
  {
  struct base64_decode_ctx ctx;
  size_t dst_length;
  int len = strlen(str);
  base64url_decode_init(&ctx);
  
  gavl_buffer_alloc(ret, BASE64_DECODE_LENGTH(len));

  base64_decode_update(&ctx,
                       &dst_length,
                       ret->buf,
                       len,
                       str);

  ret->len = dst_length;
  ret->pos = 0;
  
  base64_decode_final(&ctx);
  return 1;
  }

