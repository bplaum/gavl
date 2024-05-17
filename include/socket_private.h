#ifndef SOCKET_PRIVATE_H_INCLUDED
#define SOCKET_PRIVATE_H_INCLUDED

/* Opaque address structure so we can support IPv6 */

typedef struct lookup_async_s lookup_async_t;

struct gavl_socket_address_s 
  {
  struct sockaddr_storage addr;
  size_t len;
  
  lookup_async_t * async;
  
  };

#endif // SOCKET_PRIVATE_H_INCLUDED
