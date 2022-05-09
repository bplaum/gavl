#include <stdlib.h>
#include <string.h>

#include <nettle/aes.h>
#include <nettle/cbc.h>
#include <nettle/nettle-meta.h>

#include <config.h>

#include <gavfprivate.h>
#include <gavl/log.h>
#define LOG_DOMAIN "cipher"

typedef struct cipher_s cipher_t;

typedef void (*decrypt_block_func)(cipher_t*);

struct cipher_s
  {
  gavf_io_t * src;
  void * ctx;

  uint8_t * next_buf_in;
  uint8_t * buf_in;

  uint8_t * buf_out;
  uint8_t * iv;
  
  int buf_pos;
  int block_size; // might be smaller for the last block
  int in_bufs;

  int last_block;
  
  const struct nettle_cipher * cipher;

  decrypt_block_func decrypt_block;
  
  gavl_cipher_padding_t padding;
  
  };

static void decrypt_block_cbc(cipher_t * c)
  {
  cbc_decrypt(c->ctx, c->cipher->decrypt, c->cipher->block_size, c->iv,
              c->cipher->block_size, c->buf_out, c->buf_in);
  }

static int next_block(cipher_t * c)
  {
  //  int start = 0;

  if(c->last_block)
    return 0;
  
  if(!c->in_bufs)
    {
    if(gavf_io_read_data(c->src, c->buf_in, c->cipher->block_size) < c->cipher->block_size)
      {
      if(gavf_io_got_eof(c->src))
        return 0;
      else if(gavf_io_got_error(c->src))
        return -1;
      }
    c->in_bufs++;
    //    start = 1;
    }
  if(c->in_bufs == 1)
    {
    if(gavf_io_read_data(c->src, c->next_buf_in, c->cipher->block_size) < c->cipher->block_size)
      {
      if(gavf_io_got_eof(c->src))
        c->last_block = 1;
      else if(gavf_io_got_error(c->src))
        return -1;
      }
    else
      c->in_bufs++;
    }

#if 0  
  if(start)
    {
    fprintf(stderr, "Next block\n");
    fprintf(stderr, "Buf in:\n");
    gavl_hexdump(c->buf_in, c->cipher->block_size, c->cipher->block_size);
    }
#endif
  
  c->decrypt_block(c);

#if 0  
  if(start)
    {
    fprintf(stderr, "Buf out:\n");
    gavl_hexdump(c->buf_out, c->cipher->block_size, c->cipher->block_size);
    }
#endif
  
  if(c->last_block)
    {
    fprintf(stderr, "Got last block:\n");
    gavl_hexdump(c->buf_out, c->cipher->block_size, c->cipher->block_size);

    switch(c->padding)
      {
      case GAVL_CIPHER_PADDING_PKCS7:
        {
        int i;
        int num = c->buf_out[c->cipher->block_size-1];
        if(!num || (num > c->cipher->block_size))
          {
          /* Error */
          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Padding error");
          return -1;
          }
        
        for(i = 1; i <= num; i++)
          {
          if(c->buf_out[c->cipher->block_size-i] != num)
            {
            /* Error */
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Padding error");
            return -1;
            }
          }
        c->block_size = c->cipher->block_size - num;
        }
        break;
      case GAVL_CIPHER_PADDING_NONE:
        break;
      }

    }
  else
    {
    uint8_t * swp;
    swp = c->buf_in;
    c->buf_in = c->next_buf_in;
    c->next_buf_in = swp;
    c->in_bufs--;
    c->block_size = c->cipher->block_size;
    }
  c->buf_pos = 0;
  return 1;
  }

static int read_cipher(void * priv, uint8_t * data, int len)
  {
  int result;
  int bytes_read = 0;
  int bytes_to_read;

  cipher_t * c = priv;

  //  fprintf(stderr, "Read cipher %d\n", len);
  
  while(bytes_read < len)
    {
    if(c->buf_pos == c->block_size)
      {
      result = next_block(c);

      if(!result)
        return bytes_read;
      else if(result < 0)
        return -1;
      }
    
    bytes_to_read = len - bytes_read;

    if(bytes_to_read > c->block_size - c->buf_pos)
      bytes_to_read = c->block_size - c->buf_pos;

    if(bytes_to_read > 0)
      {
      memcpy(data + bytes_read, c->buf_out + c->buf_pos, bytes_to_read);
      c->buf_pos += bytes_to_read;
      bytes_read += bytes_to_read;
      }
    }
  
  //  return fread(data, 1, len, (FILE*)priv);
  return bytes_read;
  }

#if 0
static int write_cipher_cbc(void * priv, uint8_t * data, int len)
  {
  cipher_t * c = priv;
  
  //  return fread(data, 1, len, (FILE*)priv);
  return 0;
  }
#endif

static void close_cipher(void * priv)
  {
  cipher_t * c = priv;

  if(c->ctx)
    free(c->ctx);
  if(c->iv)
    free(c->iv);
  if(c->buf_in)
    free(c->buf_in);
  if(c->buf_out)
    free(c->buf_out);
  if(c->next_buf_in)
    free(c->next_buf_in);

  free(c);
  }

gavf_io_t * gavf_io_create_cipher(gavf_io_t * src,
                                  gavl_cipher_algo_t algo,
                                  gavl_cipher_mode_t mode,
                                  gavl_cipher_padding_t padding, int wr)
  {
  gavf_io_t * ret;
  cipher_t * priv = calloc(1, sizeof(*priv));

  priv->src = src;
  
  switch(algo)
    {
    case GAVL_CIPHER_AES128:
      priv->cipher = &nettle_aes128;
      break;
    case GAVL_CIPHER_NONE:
      return 0;
      break;
    }
  
  priv->ctx = calloc(1, priv->cipher->context_size);
  
  priv->iv =          malloc(priv->cipher->block_size);
  priv->buf_in      = malloc(priv->cipher->block_size);
  priv->buf_out     = malloc(priv->cipher->block_size);
  priv->next_buf_in = malloc(priv->cipher->block_size);

  priv->padding = padding;
  
  switch(mode)
    {
    case GAVL_CIPHER_MODE_CBC:
      priv->decrypt_block = decrypt_block_cbc;
      break;
    case GAVL_CIPHER_MODE_NONE:
      return 0;
      break;
    }

  ret = gavf_io_create(read_cipher,
                       NULL,
                       NULL,
                       close_cipher,
                       NULL,
                       GAVF_IO_CAN_READ,
                       priv);
  
  return ret;
  }

void gavf_io_cipher_init(gavf_io_t * io,
                         const uint8_t * key,
                         const uint8_t * iv)
  {
  cipher_t * c = io->priv;
  
  memcpy(c->iv, iv, c->cipher->block_size);
  c->cipher->set_decrypt_key(c->ctx, key);
  c->buf_pos = 0;
  c->block_size = 0;
  c->in_bufs = 0;
  c->last_block = 0;
  
  gavf_io_clear_error(io);
  gavf_io_clear_eof(io);
  }
