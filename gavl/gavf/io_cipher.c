#include <stdlib.h>

#include <nettle/aes.h>
#include <nettle/cbc.h>
#include <nettle/nettle-meta.h>

#include <gavfprivate.h>


typedef struct
  {
  void * ctx;
  
  
  
  gavl_buffer_t buf;
  
  char * buf_in;
  char * buf_out;

  const struct nettle_cipher * cipher;
  
  } cipher_t;

static int read_cipher_cbc(void * priv, uint8_t * data, int len)
  {
  cipher_t * c = priv;
  
  //  return fread(data, 1, len, (FILE*)priv);
  return 0;
  }

static int write_cipher_cbc(void * priv, uint8_t * data, int len)
  {
  cipher_t * c = priv;
  
  //  return fread(data, 1, len, (FILE*)priv);
  return 0;
  }
                      
gavf_io_t * gavf_io_create_cipher(gavf_io_t * base,
                                  gavl_cipher_algo_t algo,
                                  gavl_cipher_mode_t mode,
                                  gavl_cipher_padding_t padding, int wr)
  {
  cipher_t * priv = calloc(1, sizeof(*priv));
  
  switch(algo)
    {
    case GAVL_CIPHER_AES128:
      priv->cipher = &nettle_aes128;
      break;
    }
  
  priv->ctx = calloc(1, priv->cipher->context_size);
    
  switch(mode)
    {
    case GAVL_CIPHER_MODE_CBC:
      
      break;
    }

  
  }

void gavf_io_create_cipher_init(gavf_io_t * io,
                                const uint8_t * key,
                                const uint8_t * iv)
  {
  
  }
