#define _GNU_SOURCE

#include <config.h>



#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#include <net/if.h>
#endif

#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> /* gethostbyname */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h> // inet_ntop
#include <errno.h>

#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/gavlsocket.h>
#include <gavl/log.h>

#include <socket_private.h>

#include <gavl/log.h>
#define LOG_DOMAIN "socketaddress"


#define FLAG_LOOKUP_ACTIVE (1<<0)


gavl_socket_address_t * gavl_socket_address_create()
  {
  gavl_socket_address_t * ret;
  ret = calloc(1, sizeof(*ret));
  return ret;
  }

void gavl_socket_address_copy(gavl_socket_address_t * dst,
                              const gavl_socket_address_t * src)
  {
  memcpy(&dst->addr, &src->addr, src->len);
  dst->len = src->len;
  }

static void async_cancel(gavl_socket_address_t * a)
  {
  int err;

  if(!a->flags & FLAG_LOOKUP_ACTIVE)
    return;

  if((err = gai_cancel(&a->gai)) == EAI_CANCELED)
    gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Cancelled host lookup for %s", a->gai_ar_name);
  else
    gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Cancelling host lookup for %s failed: %s",
             a->gai_ar_name, gai_strerror(err));

  
  }

void gavl_socket_address_destroy(gavl_socket_address_t * a)
  {
  async_cancel(a);
  
  if(a->gai.ar_result)
    freeaddrinfo(a->gai.ar_result);
  
  if(a->gai_ar_name)
    free(a->gai_ar_name);

  free(a);
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
  /* Cancel earlier request */
  async_cancel(a);
  
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
