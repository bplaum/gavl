
#define _GNU_SOURCE

#include <config.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>

// #include <poll.h>


#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#endif

#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#include <net/if.h>
#endif

#ifdef HAVE_SYS_SENDFILE_H
#include <sys/sendfile.h>
#endif

#include <netinet/tcp.h> // IPPROTO_TCP, TCP_MAXSEG

#include <unistd.h>

#include <netdb.h> /* gethostbyname */

#include <fcntl.h>
//#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>
#include <arpa/inet.h>


#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/gavlsocket.h>

#include <gavl/log.h>
#define LOG_DOMAIN "socket"

#if !HAVE_DECL_MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define FLAG_LOOKUP_ACTIVE (1<<0)

#define DUMP_RECEIVED_SEARCH_REQUESTS

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

gavl_socket_address_t * gavl_socket_address_create()
  {
  gavl_socket_address_t * ret;
  ret = calloc(1, sizeof(*ret));
  return ret;
  }

int gavl_socket_set_block(int fd, int block)
  {
  int flags;

  if(!block)
    flags = O_NONBLOCK;
  else
    flags = 0;

  if(fcntl(fd, F_SETFL, flags) < 0)
    {
    if(block)
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot set blocking mode: %s", strerror(errno));
    else
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot set non-blocking mode: %s", strerror(errno));
    return 0;
    }
  return 1;
  }

void gavl_socket_address_copy(gavl_socket_address_t * dst,
                              const gavl_socket_address_t * src)
  {
  memcpy(&dst->addr, &src->addr, src->len);
  dst->len = src->len;
  }

void gavl_socket_address_destroy(gavl_socket_address_t * a)
  {
  int err;

  if(a->gai.ar_result)
    freeaddrinfo(a->gai.ar_result);
  
  if(a->flags & FLAG_LOOKUP_ACTIVE)
    {
    if((err = gai_cancel(&a->gai)) == EAI_CANCELED)
      gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Cancelled host lookup for %s", a->gai_ar_name);
    else
      gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Cancelling host lookup for %s failed: %s",
               a->gai_ar_name, gai_strerror(err));
    
    }
  
  if(a->gai_ar_name)
    free(a->gai_ar_name);

  free(a);
  }

static int create_socket(int domain, int type, int protocol)
  {
  int ret;
#if HAVE_DECL_SO_NOSIGPIPE // OSX
  int value = 1;
#endif

  ret = socket(domain, type, protocol);

#if HAVE_DECL_SO_NOSIGPIPE // OSX
  if(ret < 0)
    return ret;
  if(setsockopt(ret, SOL_SOCKET, SO_NOSIGPIPE, &value,
                sizeof(int)) == -1)
    return -1;
#endif
  return ret;
  }

/* */

void gavl_socket_address_set_port(gavl_socket_address_t * addr, int port)
  {
  switch(addr->addr.ss_family)
    {
    case AF_INET:
      {
      struct sockaddr_in * a;
      a = (struct sockaddr_in*)&addr->addr;
      a->sin_port = htons(port);
      }
      break;
    case AF_INET6:
      {
      struct sockaddr_in6 * a;
      a = (struct sockaddr_in6*)&addr->addr;
      a->sin6_port = htons(port);
      }
      break;
    default:
      break;
    }
  }

int gavl_socket_address_get_port(gavl_socket_address_t * addr)
  {
  switch(addr->addr.ss_family)
    {
    case AF_INET:
      {
      struct sockaddr_in * a;
      a = (struct sockaddr_in*)&addr->addr;
      return ntohs(a->sin_port);
      }
      break;
    case AF_INET6:
      {
      struct sockaddr_in6 * a;
      a = (struct sockaddr_in6*)&addr->addr;
      return ntohs(a->sin6_port);
      }
      break;
    default:
      break;
    }
  return 0;
  }


char * gavl_socket_address_to_string(const gavl_socket_address_t * addr, char * str)
  {
  switch(addr->addr.ss_family)
    {
    case AF_INET:
      {
      char buf[INET_ADDRSTRLEN];

      struct sockaddr_in * a;
      a = (struct sockaddr_in*)&addr->addr;

      inet_ntop(AF_INET, &a->sin_addr, buf, INET_ADDRSTRLEN);
      snprintf(str, GAVL_SOCKET_ADDR_STR_LEN, "%s:%d", buf, ntohs(a->sin_port));
      return str;
      }
      break;
    case AF_INET6:
      {
      char buf[INET6_ADDRSTRLEN];
      struct sockaddr_in6 * a;
      a = (struct sockaddr_in6*)&addr->addr;

      inet_ntop(AF_INET6, &a->sin6_addr, buf, INET6_ADDRSTRLEN);
      snprintf(str, GAVL_SOCKET_ADDR_STR_LEN, "[%s]:%d", buf, ntohs(a->sin6_port));
      return str;
      }
      break;
    default:
      break;
    }
  return NULL;
  }

int gavl_socket_address_from_string(gavl_socket_address_t * addr, const char * str1)
  {
  char * str = gavl_strdup(str1);
  char * pos;
  char * host_pos;
  char * port_pos;
  
  if(str[0] == '[')
    {
    struct sockaddr_in6 * a;
    a = (struct sockaddr_in6*)&addr->addr;
    
    addr->addr.ss_family = AF_INET6;

    host_pos = str+1;

    if(!(pos = strchr(str, ']')))
      goto fail;

    *pos = '\0';

    if(!(pos = strchr(str, ':')))
      goto fail;

    port_pos = pos+1;
    
    if(!inet_pton(AF_INET6, host_pos, &a->sin6_addr))
      goto fail;
    
    a->sin6_port = htons(atoi(port_pos));
    
    }
  else
    {
    struct sockaddr_in * a;
    
    a = (struct sockaddr_in*)&addr->addr;
    addr->addr.ss_family = AF_INET;

    host_pos = str;
    if(!(pos = strchr(host_pos, ':')))
      goto fail;
    
    *pos = '\0';
    port_pos = pos+1;
    
    if(!inet_pton(AF_INET, host_pos, &a->sin_addr))
      goto fail;
    
    a->sin_port = htons(atoi(port_pos));
    }
  
  free(str);
  return 1;
  
  fail:
  free(str);
  gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot convert %s to socket address", str1);
  return 0;
  }

static void set_addr_hints(struct addrinfo * hints, int socktype, const char * hostname)
  {
  struct in_addr ipv4_addr;
  struct in6_addr ipv6_addr;

  memset(hints, 0, sizeof(*hints));
  hints->ai_family   = PF_UNSPEC;
  hints->ai_socktype = socktype; // SOCK_STREAM, SOCK_DGRAM
  hints->ai_protocol = 0; // 0
  hints->ai_flags    = 0;

  /* prevent DNS lookup for numeric IP addresses */

  if(inet_pton(AF_INET, hostname, &ipv4_addr))
    {
    hints->ai_flags |= AI_NUMERICHOST;
    hints->ai_family   = PF_INET;
    }
  else if(inet_pton(AF_INET6, hostname, &ipv6_addr))
    {
    hints->ai_flags |= AI_NUMERICHOST;
    hints->ai_family   = PF_INET6;
    }

  
  }

static struct addrinfo *
hostbyname(const char * hostname, int socktype)
  {
  int err;
  
  struct addrinfo hints;
  struct addrinfo * ret = NULL;
  
  set_addr_hints(&hints, socktype, hostname);
  
  if((err = getaddrinfo(hostname, NULL /* service */,
                        &hints, &ret)))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot resolve address of %s: %s",
           hostname, gai_strerror(err));
    return NULL;
    }
#if 0
  if(ret[0].ai_addr->sa_family == AF_INET)
    fprintf(stderr, "Got IPV4 address\n");
  else if(ret[0].ai_addr->sa_family == AF_INET6)
    fprintf(stderr, "Got IPV6 address\n");
#endif  
  
  return ret;
  }

/* Do reverse DNS lookup */
char * gavl_socket_address_get_hostname(gavl_socket_address_t * addr)
  {
  char name_buf[NI_MAXHOST];
  if(getnameinfo((struct sockaddr *)&addr->addr, addr->len,
                 name_buf, NI_MAXHOST,
                 NULL, 0, 0))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Reverse DNS lookup failed");
    return NULL;
    }
  return gavl_strdup(name_buf);
  }

/* Do full DNS lookup */
gavl_socket_address_t ** gavl_lookup_hostname_full(const char * hostname, int socktype)
  {
  gavl_socket_address_t ** ret;
  int count = 0;
  int i;
  struct addrinfo * ptr;
  struct addrinfo * addr = hostbyname(hostname, socktype);

  if(!addr)
    return NULL;

  /* Count entries */
  ptr = addr;
  while(ptr)
    {
    count++;
    ptr = ptr->ai_next;
    }

  /* Copy addresses */
  ptr = addr;
  ret = calloc(count + 1, sizeof(*ret));
  
  for(i = 0; i < count; i++)
    {
    ret[i] = gavl_socket_address_create();

    memcpy(&ret[i]->addr, ptr->ai_addr, ptr->ai_addrlen);
    ret[i]->len = ptr->ai_addrlen;
    
    ptr = ptr->ai_next;
    }
  
  freeaddrinfo(addr);
  return ret;
  }

void gavl_socket_address_free_array(gavl_socket_address_t ** addr)
  {
  int i = 0;
  while(addr[i])
    {
    gavl_socket_address_destroy(addr[i]);
    i++;
    }
  free(addr);
  }

int gavl_socket_address_set(gavl_socket_address_t * a, const char * hostname,
                            int port, int socktype)
  {
  struct addrinfo * addr;
  
  addr = hostbyname(hostname, socktype);
  if(!addr)
    return 0;
  
  memcpy(&a->addr, addr->ai_addr, addr->ai_addrlen);
  a->len = addr->ai_addrlen;
  //  if(!hostbyname(a, hostname))
  //    return 0;
  //  a->port = port;

  freeaddrinfo(addr);

  gavl_socket_address_set_port(a, port);
  
  return 1;
  }

int gavl_socket_address_set_async(gavl_socket_address_t * a, const char * host,
                                  int port, int socktype)
  {
  gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Looking up %s", host);
  set_addr_hints(&a->gai_ar_request, socktype, host);

  /* TODO: Make this a noop whe host and port didn't change */

  if(a->gai_ar_name && !strcmp(a->gai_ar_name, host) &&
     (a->gai_port == port))
    {
    gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Using cached host address for %s:%d", host, port);
    return 1; // Nothing to do, take last result
    }

  a->gai_ar_name = gavl_strrep(a->gai_ar_name, host);
  a->gai_port = port;
  
  if(a->gai.ar_result)
    freeaddrinfo(a->gai.ar_result);
  
  memset(&a->gai, 0, sizeof(a->gai));
  
  a->gai.ar_request = &a->gai_ar_request;
  a->gai.ar_name = a->gai_ar_name;

  a->gai_arr[0] = &a->gai;
  
  if(getaddrinfo_a(GAI_NOWAIT, a->gai_arr, 1, NULL))
    return 0;

  a->flags |= FLAG_LOOKUP_ACTIVE;
  
  return 1;
  }

int gavl_socket_address_set_async_done(gavl_socket_address_t * a, int timeout)
  {
  int result;
  
  if(!(a->flags & FLAG_LOOKUP_ACTIVE))
    return 1; // Nothing to do
  
  //  fprintf(stderr, "gavl_socket_address_set_async_done\n");
  
  if(timeout >= 0)
    {
    struct timespec to;
    to.tv_sec = timeout / 1000;
    to.tv_nsec = (timeout % 1000) * 1000000;
    result = gai_suspend((const struct gaicb**)a->gai_arr, 1, &to);
    }
  else
    result = gai_suspend((const struct gaicb**)a->gai_arr, 1, NULL);

  //  fprintf(stderr, "gavl_socket_address_set_async_done 2 %d\n", result);
  
  switch(result)
    {
    case 0:
    case EAI_ALLDONE:
      break;
    case EAI_SYSTEM:
      return 0;
      break;
    case EAI_AGAIN:
    case EAI_INTR:
      return 0;
      break;
    default:
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Looking up host %s failed: %d %s",
               a->gai_ar_name, result, gai_strerror(result));
      a->flags &= ~FLAG_LOOKUP_ACTIVE;
      return -1;
      break;
    }

  result = gai_error(&a->gai);

  switch(result)
    {
    case 0:
    case EAI_ALLDONE:
      {
      char str[GAVL_SOCKET_ADDR_STR_LEN];

      //      memcpy(&a->addr, addr->ai_addr, addr->ai_addrlen);
      //      a->len = addr->ai_addrlen;
      
      memcpy(&a->addr, a->gai.ar_result->ai_addr, a->gai.ar_result->ai_addrlen);
      a->len = a->gai.ar_result->ai_addrlen;
      
      //  if(!hostbyname(a, hostname))
      //    return 0;
      //  a->port = port;

      gavl_socket_address_set_port(a, a->gai_port);


      gavl_socket_address_to_string(a, str);
      gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Looking up host %s succeeded: %s", a->gai_ar_name, str);
      a->flags &= ~FLAG_LOOKUP_ACTIVE;
      return 1;
      }
      break;
    case EAI_INPROGRESS:
      return 0;
      break;
    case EAI_CANCELED:
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot resolve address of %s: Canceled",
               a->gai_ar_name);
      a->flags &= ~FLAG_LOOKUP_ACTIVE;
      return -1;
      break;
    default:
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot resolve address of %s: %d %s",
               a->gai_ar_name, result, gai_strerror(result));
      a->flags &= ~FLAG_LOOKUP_ACTIVE;
      return -1;
    }
  a->flags &= ~FLAG_LOOKUP_ACTIVE;
  return -1;
  }


int gavl_socket_address_set_local(gavl_socket_address_t * a, int port, const char * wildcard)
  {
#ifdef HAVE_IFADDRS_H  
  int ret = 0;
  int family_req = 0;
  struct ifaddrs * ifap = NULL;
  struct ifaddrs * addr;
  if(getifaddrs(&ifap))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "getifaddrs failed: %s", strerror(errno));
    return 0;
    }
  if(wildcard)
    {
    if(!strcmp(wildcard, "0.0.0.0"))
      family_req = AF_INET;
    else if(!strcmp(wildcard, "::"))
      family_req = AF_INET6;
    }

  addr = ifap;
  while(addr)
    {
    if(!(addr->ifa_flags & IFF_LOOPBACK))
      {
      if((addr->ifa_addr->sa_family == AF_INET) &&
         (!wildcard || (family_req == AF_INET)))
        a->len = sizeof(struct sockaddr_in);

      else if((addr->ifa_addr->sa_family == AF_INET6) &&
              (!wildcard || (family_req == AF_INET6)))
        a->len = sizeof(struct sockaddr_in6);
      else
        {
        addr = addr->ifa_next;
        continue;
        }
      memcpy(&a->addr, addr->ifa_addr, a->len);
      gavl_socket_address_set_port(a, port);
      ret = 1;
      break;
      }
    addr = addr->ifa_next;
    }
  
  if(!ret)
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "getifaddrs returned only loopback addresses");
  
  freeifaddrs(ifap);
  return ret;
#else
  return 0;
#endif
  //  return gavl_socket_address_set(a, hostname, port, socktype);
  }

static int addr_match(struct ifaddrs * a, int flags)
  {
  if((a->ifa_flags & IFF_LOOPBACK) && !(flags & GAVL_NI_LOOPBACK))
    return 0;

  if(a->ifa_addr->sa_family == AF_INET)
    {
    if(flags & GAVL_NI_IPV4)
      return 1;
    else
      return 0;
    }
  
  else if(a->ifa_addr->sa_family == AF_INET6)
    {
    if(flags & GAVL_NI_IPV6)
      return 1;
    else
      return 0;
    }

  return 0;
  }

gavl_socket_address_t ** gavl_get_network_interfaces(int flags)
  {
#ifdef HAVE_IFADDRS_H  

  gavl_socket_address_t ** ret;
  int num = 0;
  int idx;
  struct ifaddrs * ifap = NULL;
  struct ifaddrs * addr;
  if(getifaddrs(&ifap))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "getifaddrs failed: %s", strerror(errno));
    return 0;
    }

  /* Count entries */
  
  addr = ifap;
  while(addr)
    {
    if(addr_match(addr, flags))
      num++;
    addr = addr->ifa_next;
    }

  ret = calloc(num+1, sizeof(*ret));
  idx = 0;
  
  addr = ifap;
  while(addr)
    {
    if(addr_match(addr, flags))
      {
      ret[idx] = gavl_socket_address_create();
      
      if(addr->ifa_addr->sa_family == AF_INET6)
        ret[idx]->len = sizeof(struct sockaddr_in6);
      else if(addr->ifa_addr->sa_family == AF_INET)
        ret[idx]->len = sizeof(struct sockaddr_in);
      
      memcpy(&ret[idx]->addr, addr->ifa_addr, ret[idx]->len);

      //      gavl_socket_address_to_string(ret[idx], str);
      //      fprintf(stderr, "Got network interface: %s\n", str);

      idx++;
      }
    addr = addr->ifa_next;
    }
  
  freeifaddrs(ifap);
  return ret;
#else
  return NULL;
#endif
  }

#ifdef HAVE_IFADDRS_H
static struct ifaddrs * iface_by_address(struct ifaddrs * addr, const gavl_socket_address_t * a1)
  {
  int len = 0;
  gavl_socket_address_t * a = gavl_socket_address_create();
  gavl_socket_address_copy(a, a1);
  gavl_socket_address_set_port(a, 0);

  while(addr)
    {
    if(addr->ifa_addr->sa_family == AF_INET6)
      len = sizeof(struct sockaddr_in6);
    else if(addr->ifa_addr->sa_family == AF_INET)
      len = sizeof(struct sockaddr_in);
    
    if((len == a->len) &&
       !memcmp(&a->addr, addr->ifa_addr, a->len))
      {
      break;
      }
    addr = addr->ifa_next;
    }
  gavl_socket_address_destroy(a);
  return addr;
  }
#endif

char * gavl_interface_name_from_address(const gavl_socket_address_t * a)
  {
  char * ret = NULL;
#ifdef HAVE_IFADDRS_H
  struct ifaddrs * ifap = NULL;
  struct ifaddrs * addr;
  
  if(getifaddrs(&ifap))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "getifaddrs failed: %s", strerror(errno));
    goto fail;
    }
  
  if((addr = iface_by_address(ifap, a)))
    ret = gavl_strdup(addr->ifa_name);
  
  fail:
  
  freeifaddrs(ifap);
  
#endif
  
  return ret;
  }

int gavl_interface_index_from_address(const gavl_socket_address_t * a)
  {
  int ret = 0;
#ifdef HAVE_IFADDRS_H
  struct ifaddrs * ifap = NULL;
  struct ifaddrs * addr;
  
  if(getifaddrs(&ifap))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "getifaddrs failed: %s", strerror(errno));
    goto fail;
    }
  
  if((addr = iface_by_address(ifap, a)))
    ret = if_nametoindex(addr->ifa_name);
  
  fail:
  
  freeifaddrs(ifap);
  
#endif
  
  return ret;
  }

void gavl_socket_address_destroy_array(gavl_socket_address_t ** addr)
  {
  int idx = 0;

  while(addr[idx])
    gavl_socket_address_destroy(addr[idx++]);

  free(addr);
  
  }


int gavl_socket_get_address(int sock, gavl_socket_address_t * local,
                          gavl_socket_address_t * remote)
  {
  socklen_t len;
  if(local)
    {
    len = sizeof(local->addr); 
    if(getsockname(sock, (struct sockaddr *)&local->addr, &len))
      return 0;
    local->len = len;
    }
  if(remote) 
    { 
    len = sizeof(remote->addr);
    if(getpeername(sock, (struct sockaddr *)&remote->addr, &len))
      return 0;
    remote->len = len;
    }
  return 1;
  }

int gavl_socket_get_errno(int fd)
  {
  int err = 0;
  socklen_t err_len;

  err_len = sizeof(err);
  if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &err_len))
    {
    err = errno;
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "getsockopt failed: %s",
             strerror(err));
    }
  return err;
  }

/* Client connection (stream oriented) */

static int finalize_connection(int ret)
  {
  int err;

  err = gavl_socket_get_errno(ret);
  
  if(err)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Connecting failed: %s",
             strerror(err));
    return -1;
    }
  
  /* Set back to blocking mode */
  
  if(!gavl_socket_set_block(ret, 1))
    return -1;
  
  return 1;
  }

int gavl_socket_connect_inet(gavl_socket_address_t * a, int milliseconds)
  {
  int ret = -1;
  char str[GAVL_SOCKET_ADDR_STR_LEN];

  gavl_socket_address_to_string(a, str);
  gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Connecting to %s", str);
  
  /* Create the socket */
  if((ret = create_socket(a->addr.ss_family, SOCK_STREAM, 0)) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot create TCP socket");
    return -1;
    }

  /* Set nonblocking mode */
  if(!gavl_socket_set_block(ret, 0))
    {
    gavl_socket_close(ret);
    return -1;
    }
  
  /* Connect the thing */
  if(connect(ret, (struct sockaddr*)&a->addr, a->len) < 0)
    {
    if(errno == EINPROGRESS)
      {
      if(!milliseconds)
        return ret;
      
      if(!gavl_fd_can_write(ret, milliseconds))
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Connection timed out");
        return -1;
        }
      }
    else
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Connecting failed: %s",
             strerror(errno));
      return -1;
      }
    }

  /* Check for error */
  if(finalize_connection(ret) < 0)
    {
    gavl_socket_close(ret);
    return -1;
    }
  
  return ret;
  }

int gavl_socket_connect_inet_complete(int fd, int milliseconds)
  {
  if(!gavl_fd_can_write(fd, milliseconds))
    {
    return 0;
    }

  gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Connected");

  return finalize_connection(fd);
  }

int gavl_socket_connect_unix(const char * name)
  {
  struct sockaddr_un addr;
  int addr_len;
  int ret;
  ret = create_socket(PF_LOCAL, SOCK_STREAM, 0);
  if(ret < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot create unix socket");
    return -1;
    }

  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, name, sizeof(addr.sun_path));
  addr.sun_path[sizeof (addr.sun_path) - 1] = '\0';
  addr_len = SUN_LEN(&addr);
  
  if(connect(ret,(struct sockaddr*)(&addr),addr_len)<0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Connecting unix socket %s failed: %s",
           name, strerror(errno));
    return -1;
    }
  return ret;
  }

void gavl_socket_disconnect(int sock)
  {
  gavl_socket_close(sock);
  }

/* Older systems assign an ipv4 address to localhost,
   newer systems (e.g. Ubuntu Karmic) assign an ipv6 address to localhost.

   To test this, we make a name lookup for "localhost" and test if it returns
   an IPV4 or an IPV6 address */

static int have_ipv6()
  {
  struct addrinfo hints;
  struct addrinfo * ret;
  int err, has_ipv6;
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM; // SOCK_DGRAM
  hints.ai_protocol = 0; // 0
  hints.ai_flags    = 0;

  if((err = getaddrinfo("localhost", NULL /* service */,
                        &hints, &ret)))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot resolve address of localhost: %s",
           gai_strerror(err));
    return 0;
    }

  if(ret[0].ai_addr->sa_family == AF_INET6)
    has_ipv6 = 1;
  else
    has_ipv6 = 0;

  freeaddrinfo(ret);
  return has_ipv6;
  }

/* Server socket (stream oriented) */

int gavl_listen_socket_create_inet(gavl_socket_address_t * addr,
                                 int port,
                                 int queue_size,
                                 int flags)
  {
  int ret, err, use_ipv6 = 0;
  struct sockaddr_in  name_ipv4;
  struct sockaddr_in6 name_ipv6;

  if(addr)
    {
    if(addr->addr.ss_family == AF_INET6)
      use_ipv6 = 1;
    }
  else if(flags & GAVL_SOCKET_LOOPBACK)
    use_ipv6 = have_ipv6();
  
  memset(&name_ipv4, 0, sizeof(name_ipv4));
  memset(&name_ipv6, 0, sizeof(name_ipv6));

  /* Create the socket. */
  
  if(use_ipv6)
    ret = create_socket(PF_INET6, SOCK_STREAM, 0);
  else
    ret = create_socket(PF_INET, SOCK_STREAM, 0);
  
  if(ret < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot create TCP server socket");
    return -1;
    }
  
  if(flags & GAVL_SOCKET_REUSEADDR)
    {
    int optval = 1;
    setsockopt(ret, SOL_SOCKET, SO_REUSEADDR,
               &optval, sizeof(optval));
    }
  
  /* Give the socket a name. */

  if(addr)
    {
    err = bind(ret, (struct sockaddr*)&addr->addr, addr->len);
    }
  else if(use_ipv6)
    {
    name_ipv6.sin6_family = AF_INET6;
    name_ipv6.sin6_port   = htons(port);
    if(flags & GAVL_SOCKET_LOOPBACK)
      name_ipv6.sin6_addr = in6addr_loopback;
    else
      name_ipv6.sin6_addr = in6addr_any;

    err = bind(ret, (struct sockaddr *)&name_ipv6, sizeof(name_ipv6));
    }
  else
    {
    name_ipv4.sin_family = AF_INET;
    name_ipv4.sin_port = htons(port);
    if(flags & GAVL_SOCKET_LOOPBACK)
      name_ipv4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    else
      name_ipv4.sin_addr.s_addr = htonl(INADDR_ANY);
    err = bind(ret, (struct sockaddr *)&name_ipv4, sizeof(name_ipv4));
    }
  
  if(err < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot bind inet socket: %s", strerror(errno));
    return -1;
    }
  if(listen(ret, queue_size))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot put socket into listening mode");
    return -1;
    }
  return ret;
  }

int gavl_listen_socket_create_unix(const char * name,
                                   int queue_size)
  {
  int ret;

  struct sockaddr_un addr;
  int addr_len;
  ret = create_socket(PF_LOCAL, SOCK_STREAM, 0);
  if(ret < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot create unix server socket");
    return -1;
    }

  addr.sun_family = AF_LOCAL;
  strncpy (addr.sun_path, name, sizeof(addr.sun_path));
  addr.sun_path[sizeof (addr.sun_path) - 1] = '\0';
  addr_len = SUN_LEN(&addr);
  if(bind(ret,(struct sockaddr*)(&addr),addr_len)<0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot bind UNIX domain socket: %s",
           strerror(errno));
    return -1;
    }
  if(listen(ret, queue_size))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot put socket into listening mode");
    return -1;
    }
  return ret;
  }

/* Accept a new client connection, return -1 if there is none */

int gavl_listen_socket_accept(int sock, int milliseconds,
                            gavl_socket_address_t * from)
  {
  int ret;
  
  if((milliseconds >= 0) && !gavl_fd_can_read(sock, milliseconds))
    return -1;

  if(from)
    {
    struct sockaddr * a;
    socklen_t len;
    a = (struct sockaddr *)&from->addr;
    len = sizeof(from->addr);
    ret = accept(sock, a, &len);
    from->len = len; 
    }
  else
    ret = accept(sock, NULL, NULL);
  
  if(ret < 0)
    return -1;
  return ret;
  }

void gavl_listen_socket_destroy(int sock)
  {
  union
    {
    struct sockaddr addr;
    struct sockaddr_un addr_un;
    struct sockaddr_storage addr_inet;
    } addr;
  socklen_t len = sizeof(addr);
  
  if(!getsockname(sock, &addr.addr, &len) &&
     (addr.addr.sa_family == AF_LOCAL))
    {
    if(unlink(addr.addr_un.sun_path))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Unlinking socket %s failed: %s",
             addr.addr_un.sun_path, strerror(errno));
      }
    else
      gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Unlinked socket %s", 
             addr.addr_un.sun_path);
    }
  
  gavl_socket_close(sock);
  }


int gavl_socket_read_data_noblock(int fd, uint8_t * data, int len)
  {
  int result;
  
  //  fprintf(stderr, "read_data_noblock 1\n");
  
  if(!gavl_fd_can_read(fd, 0))
    return 0;

  //  fprintf(stderr, "read_data_noblock 2 %d\n", len);
  
  result = recv(fd, data, len, 0);

  //  fprintf(stderr, "read_data_noblock 3 %d\n", result);

  if(result < 0)
    {
    gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Nonblocking read failed: %s", strerror(errno));
    return 0;
    }
  return result;
  }

int gavl_socket_write_data_nonblock(int fd, const uint8_t * data, int len)
  {
  int result;
  
  if(!gavl_fd_can_write(fd, 0))
    {
    return 0;
    }
  result = send(fd, data, len, MSG_DONTWAIT | MSG_NOSIGNAL);

  if(!result)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Nonblocking write returned zero: %s", strerror(errno));
    }
  
  if(result < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Nonblocking write failed: %s", strerror(errno));
    //    gavl_hexdump(data, len, 16);
    return result;
    }
  return result;
  }

int gavl_socket_read_data(int fd, uint8_t * data, int len, int milliseconds)
  {
  int result;
  int bytes_read = 0;
  
  while(bytes_read < len)
    {
    if((milliseconds >= 0) && !gavl_fd_can_read(fd, milliseconds))
      {
      gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Got read timeout (t: %d, len: %d)", milliseconds, len);
      return bytes_read;
      }

    if(milliseconds < 0)
      result = recv(fd, data + bytes_read, len - bytes_read, MSG_WAITALL);
    else
      result = recv(fd, data + bytes_read, len - bytes_read, MSG_DONTWAIT);
    
    if(result < 0)
      {
      if((errno != EAGAIN) && (errno != EWOULDBLOCK))
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,  "Reading %d bytes failed: %s (timeout was %d)",
                 len - bytes_read, strerror(errno), milliseconds);
        
        return bytes_read;
        }
      }
    else if(result == 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,  "Reading %d bytes failed: Connection closed (timeout was %d)",
             len - bytes_read, milliseconds);
      break;
      }
    else
      {
      bytes_read += result;
      }
    }
  
  return bytes_read;
  }

int gavl_socket_write_data(int fd, const void * data, int len)
  {
  int result;
  result = send(fd, data, len, MSG_NOSIGNAL);
  if(result != len)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Sending data failed: %s", 
           strerror(errno));    
    return 0;
    }
  return result;
  }


int gavl_socket_is_unix(int fd)
  {
  socklen_t slen;
  struct sockaddr_storage us;
  slen = sizeof(us);
  if(getsockname(fd, (struct sockaddr*)&us, &slen) == -1)
    return 0;

  if(us.ss_family == AF_LOCAL)
    return 1;
  else
    return 0;
  }

int gavl_socket_is_local(int fd)
  {
  struct sockaddr_storage us;
  struct sockaddr_storage them;
  socklen_t slen;
  slen = sizeof(us);
  
  if(getsockname(fd, (struct sockaddr*)&us, &slen) == -1)
    return 1;

  if(slen == 0 || us.ss_family == AF_LOCAL)
    return 1;

#if 1
  if(us.ss_family == AF_INET)
    {
    struct sockaddr_in * a1, *a2;
    a1 = (struct sockaddr_in *)&us;
    if(a1->sin_addr.s_addr == INADDR_LOOPBACK)
      return 1;

    slen = sizeof(them);
    
    if(getpeername(fd, (struct sockaddr*)&them, &slen) == -1)
      return 0;

    a2 = (struct sockaddr_in *)&them;

    if(!memcmp(&a1->sin_addr.s_addr, &a2->sin_addr.s_addr,
               sizeof(a2->sin_addr.s_addr)))
      return 1;
    }
  else if(us.ss_family == AF_INET6)
    {
    struct sockaddr_in6 * a1, *a2;
    a1 = (struct sockaddr_in6 *)&us;

    /* Detect loopback */
    if(!memcmp(&a1->sin6_addr.s6_addr, &in6addr_loopback,
               sizeof(in6addr_loopback)))
      return 1;

    if(getpeername(fd, (struct sockaddr*)&them, &slen) == -1)
      return 0;

    a2 = (struct sockaddr_in6 *)&them;

    if(!memcmp(&a1->sin6_addr.s6_addr, &a2->sin6_addr.s6_addr,
               sizeof(a2->sin6_addr.s6_addr)))
      return 1;

    }
#endif
  return 0;
  }

/* Send an entire file */
int gavl_socket_send_file(int fd, const char * filename,
                        int64_t offset, int64_t len)
  {
  int ret = 0;
  int buf_size;
  uint8_t * buf = NULL;
  int file_fd;
  socklen_t size_len;
  int64_t result;
  int64_t bytes_written;
  int bytes;
  
  file_fd = open(filename, O_RDONLY);
  if(file_fd < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot open local file: %s", strerror(errno));
    return 0;
    }
  
  if(len <= 0)
    len = lseek(file_fd, 0, SEEK_END);
  lseek(file_fd, offset, SEEK_SET);
  
  /* Try sendfile */
#ifdef HAVE_SYS_SENDFILE_H
  // fprintf(stderr, "Sendfile: %d <- %d, off: %"PRId64" len: %"PRId64" \n", fd, file_fd, offset, len);
  result = sendfile(fd, file_fd, &offset, len);
  if(result < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "sendfile to %d failed: %s", fd, strerror(errno));
    
    if((errno != EINVAL) && (errno != ENOSYS))
      goto end;
    }
  else
    {
    //  fprintf(stderr, "Sendfile done: %"PRId64" bytes, off: %"PRId64"\n", len, offset);
    ret = 1;
    goto end;
    }
#endif
  
  if(getsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, &buf_size, &size_len))
    buf_size = 576 - 40;

  bytes_written = 0;
  
  buf = malloc(buf_size);

  while(bytes_written < len)
    {
    bytes = len - bytes_written;
    
    if(bytes > buf_size)
      bytes = buf_size;
    
    bytes = read(file_fd, buf, bytes);
    if(bytes < 0)
      goto end;
    
    if(gavl_socket_write_data(fd, buf, bytes) < bytes)
      goto end;
    bytes_written += bytes;
    }

  ret = 1;
  end:

  close(file_fd);
  
  if(buf)
    free(buf);
  return ret;
  }

/* These functions do not test if the socket is local */

int gavl_socket_send_fds(int socket, const int * fds, int n)
  {
  struct msghdr msg = {0};
  struct cmsghdr *cmsg;
  char payload = '\0';

  char buf[CMSG_SPACE(GAVL_MAX_PLANES * sizeof(int))];
  struct iovec io = { .iov_base = &payload, .iov_len = sizeof(payload) };
  memset(buf, '\0', sizeof(buf));
  
  msg.msg_iov = &io;
  msg.msg_iovlen = 1;
  msg.msg_control = buf;
  msg.msg_controllen = sizeof(buf);
  
  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(n * sizeof(int));
  
  memcpy ((int *) CMSG_DATA(cmsg), fds, n * sizeof (int));
  
  if(sendmsg (socket, &msg, 0) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Failed to send filedescriptors: %s", strerror(errno));
    return 0;
    }
  else
    return 1;
  }

int gavl_socket_recv_fds(int socket, int * fds, int n)
  {
  struct msghdr msg = {0};
  struct cmsghdr *cmsg;
  char buf[CMSG_SPACE(GAVL_MAX_PLANES * sizeof(int))], dup[256];
  struct iovec io = { .iov_base = &dup, .iov_len = sizeof(dup) };
  memset(buf, '\0', sizeof(buf));

  msg.msg_iov = &io;
  msg.msg_iovlen = 1;
  msg.msg_control = buf;
  msg.msg_controllen = sizeof(buf);

  if(recvmsg (socket, &msg, 0) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Failed to receive filedescriptors: %s", strerror(errno));
    return 0;
    }

  cmsg = CMSG_FIRSTHDR(&msg);

  memcpy (fds, (int *) CMSG_DATA(cmsg), n * sizeof(int));

  return 1;
  }

int gavl_udp_socket_create(gavl_socket_address_t * addr)
  {
  int ret;
  int err;
  int get_port = (gavl_socket_address_get_port(addr) == 0) ? 1 : 0;
 
  if((ret = create_socket(addr->addr.ss_family, SOCK_DGRAM, 0)) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot create UDP socket: %s", strerror(errno));
    return -1;
    }
  err = bind(ret, (struct sockaddr*)&addr->addr, addr->len);

  if(err)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot bind UDP socket: %s", strerror(errno));
    return -1;
    }

  if(get_port)
    gavl_socket_get_address(ret, addr, NULL);

  return ret;
  }

int gavl_udp_socket_connect(int fd, gavl_socket_address_t * dst)
  {
  if(connect(fd, (struct sockaddr*)&dst->addr, dst->len) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "connect() failed for UDP socket: %s",
             strerror(errno));
    return 0;
    }
  return 1;
  }

int gavl_udp_socket_set_multicast_interface(int fd, gavl_socket_address_t * interface_addr)
  {
  if(interface_addr->addr.ss_family == AF_INET)
    {
    struct sockaddr_in * a = (struct sockaddr_in *)&interface_addr->addr;
    
    if(setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &a->sin_addr, sizeof(a->sin_addr)) < 0) 
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Setting multicast interface failed: %s", strerror(errno));
      return 0;
      }
    else
      return 1;
    }
  
  gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot set multicast interface for this address family");
  return 0;
  
  }

int gavl_udp_socket_create_multicast(gavl_socket_address_t * multicast_addr,
                                     gavl_socket_address_t * interface_addr)
  {
  int reuse = 1;
  int ret;
  int err;
  uint8_t loop = 1;
  
#ifndef __linux__
  gavl_socket_address_t bind_addr;
#endif
  
  if((ret = create_socket(multicast_addr->addr.ss_family, SOCK_DGRAM, 0)) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot create UDP multicast socket: %s", strerror(errno));
    return -1;
    }
  
  if(setsockopt(ret, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)

    {
    gavl_socket_close(ret);
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot set SO_REUSEADDR: %s", strerror(errno));
    return -1;
    }

  /* Bind to proper port */
  

  /* On linux we bind to the multicast address (that's what minidlna does) */  
#ifdef __linux__
  err = bind(ret, (struct sockaddr*)&multicast_addr->addr, multicast_addr->len);
#else
  memset(&bind_addr, 0, sizeof(bind_addr));
  gavl_socket_address_set(&bind_addr, "0.0.0.0",
                          gavl_socket_address_get_port(addr), SOCK_DGRAM);
  err = bind(ret, (struct sockaddr*)&bind_addr.addr, bind_addr.len);
#endif
  //  err = bind(ret, (struct sockaddr*)&addr->addr, addr->len);
  
  if(err)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot bind UDP socket: %s", strerror(errno));
    gavl_socket_close(ret);
    return -1;
    }
  
  if(multicast_addr->addr.ss_family == AF_INET)
    {
    struct ip_mreq req;
    struct sockaddr_in * a = (struct sockaddr_in *)&multicast_addr->addr;
    memset(&req, 0, sizeof(req));
    
    memcpy(&req.imr_multiaddr, &a->sin_addr, sizeof(req.imr_multiaddr));

    if(interface_addr)
      {
      a = (struct sockaddr_in *)&interface_addr->addr;
      memcpy(&req.imr_interface, &a->sin_addr, sizeof(req.imr_multiaddr));
      }
    else
      req.imr_interface.s_addr = htonl(INADDR_ANY);
    
    if(setsockopt(ret, IPPROTO_IP, IP_ADD_MEMBERSHIP, &req, sizeof(req)))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot join multicast group: %s",
             strerror(errno));
      gavl_socket_close(ret);
      return -1;
      }
    }
  else
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "IPV6 multicast not supported yet");
    gavl_socket_close(ret);
    return -1;
    }
  
  setsockopt(ret, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
  
  return ret;
  }

int gavl_udp_socket_receive(int fd, uint8_t * data, int data_size,
                          gavl_socket_address_t * addr)
  {
  socklen_t len = sizeof(addr->addr);
  ssize_t result;

  if(addr)  
    result = recvfrom(fd, data, data_size, 0 /* int flags */,
                      (struct sockaddr *)&addr->addr, &len);
  else
    result = recv(fd, data, data_size, 0);

  if(result < 0)
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "UDP Receive failed");
  
  addr->len = len;
  return result;
  }

int gavl_udp_socket_send(int fd, const uint8_t * data, int data_size,
                       gavl_socket_address_t * addr)
  {
  if(addr)
    return (sendto(fd, data, data_size, 0 /* int flags */,
                   (struct sockaddr *)&addr->addr, addr->len) == data_size);
  else
    return (send(fd, data, data_size, 0) == data_size);
  }

/* Unix domain server sockets including filename management and cleanup procedures */

pthread_mutex_t unix_socket_mutex = PTHREAD_MUTEX_INITIALIZER;
static int unix_socket_index = 0;

int gavl_unix_socket_create(char ** name, int queue_size)
  {
  pthread_mutex_lock(&unix_socket_mutex);
  *name = gavl_sprintf("%s/gmerlin-unix-%ld-%d", gavl_tempdir(), (long)getpid(), unix_socket_index++);
  pthread_mutex_unlock(&unix_socket_mutex);
  return gavl_listen_socket_create_unix(*name, queue_size);
  }

void gavl_socket_close(int fd)
  {
  gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Closing socket %d", fd);
  close(fd);
  }

int gavl_socket_is_disconnected(int fd, int timeout)
  {
  /* Figure out if a socket is disconnected */
  
  uint8_t buf;
  
  if(gavl_fd_can_read(fd, 0) &&
     (recv(fd, &buf, 1, MSG_PEEK) <= 0))
    return 1;
  else
    return 0;
  }

int gavl_socket_address_get_address_family(gavl_socket_address_t * addr)
  {
  return addr->addr.ss_family;
  }
