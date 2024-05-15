#ifndef SOCKET_PRIVATE_H_INCLUDED
#define SOCKET_PRIVATE_H_INCLUDED


/* Opaque address structure so we can support IPv6 */

struct gavl_socket_address_s 
  {
  struct sockaddr_storage addr;
  size_t len;

  /* For asynchronous calls */

  struct addrinfo gai_ar_request;
  struct gaicb gai;
  struct gaicb * gai_arr[1];
  
  char * gai_ar_name;
  int gai_port;
  
  int flags;
  };

#endif // SOCKET_PRIVATE_H_INCLUDED
