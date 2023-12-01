#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <nettle/md5.h>
#include <string.h>

void *
gavl_md5_buffer(const void *buffer, int len, void *resblock)
  {
  struct md5_ctx ctx;
  md5_init(&ctx);
  md5_update(&ctx, len, buffer);
  md5_digest(&ctx, GAVL_MD5_SIZE, resblock);
  return resblock;
  }

char * gavl_md5_buffer_str(const void *buffer, int len, char * ret)
  {
  uint8_t md5[GAVL_MD5_SIZE];
  gavl_md5_buffer(buffer, len, md5);
  gavl_md5_2_string(md5, ret);
  return ret;
  }

static const char * hex_digits = "0123456789abcdef";

char * gavl_md5_2_string(const void * md5v, char * str)
  {
  int i;
  const uint8_t * md5 = md5v;
  for(i = 0; i < GAVL_MD5_SIZE; i++)
    {
    str[0] = hex_digits[ (*md5 >> 4) & 0xf  ];
    str[1] = hex_digits[ *md5 & 0xf ];
    str += 2;
    md5++;
    }
  *str = '\0';
  return str;
  }

int gavl_string_2_md5(const char * str, void * md5v)
  {
  const char * pos1;
  const char * pos2;
  int i;

  char * md5 = md5v;
  
  for(i = 0; i < GAVL_MD5_SIZE; i++)
    {
    pos1 = strchr(hex_digits, str[2*i]);
    pos2 = strchr(hex_digits, str[2*i+1]);

    if(!pos1 || !pos2)
      return 0;

    *md5 = ((int)(pos1 - hex_digits) << 4) | (int)(pos2 - hex_digits);
    md5++;
    }
  
  return 1;
  }
