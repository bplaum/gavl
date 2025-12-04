/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2024 Members of the Gmerlin project
 * http://github.com/bplaum
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * *****************************************************************/


#define _GNU_SOURCE

#include <config.h>



#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#include <net/if.h>
#endif

#include <sys/time.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> /* gethostbyname */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h> // inet_ntop
#include <errno.h>
#include <pthread.h>

#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/gavlsocket.h>
#include <gavl/log.h>

#include <socket_private.h>

#include <gavl/log.h>
#define LOG_DOMAIN "socketaddress"

// typedef struct lookup_async_s lookup_async_t;

// #define FLAG_LOOKUP_ACTIVE (1<<0)

/* Asynchronous host lookup */

struct lookup_async_s
  {
  char * hostname;
  struct addrinfo * addr;
  int port;
  int socktype;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  
  int waiting; // Someone is waiting for us
  int done; // Lookup done
  };


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

void gavl_socket_address_destroy(gavl_socket_address_t * a)
  {
  //  fprintf(stderr, "gavl_socket_address_destroy %p\n", a->async);
  gavl_socket_address_set_async_cancel(a);
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

  gavl_socket_address_set_async_cancel(a);
  
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
      if(!addr->ifa_addr) // ifa_addr can be NULL for VPN interfaces
        {
        addr = addr->ifa_next;
        continue;
        }
      
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

  if(!a->ifa_addr)
    return 0;
  
  if((a->ifa_addr->sa_family == AF_INET))
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
    if(!addr->ifa_addr)
      {
      addr = addr->ifa_next;
      continue;
      }
    
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

/* Asynchronous host lookup */


static lookup_async_t * async_create(const char * hostname,
                                     int port,
                                     int socktype)
  {
  lookup_async_t * ret = calloc(1, sizeof(*ret));
  
  ret->hostname = gavl_strdup(hostname);
  ret->port = port;
  ret->socktype = socktype;
  ret->waiting = 1;

  pthread_mutex_init(&ret->mutex, NULL);
  pthread_cond_init(&ret->cond, NULL);
  
  return ret;
  }

static void async_destroy(lookup_async_t * a)
  {
  pthread_mutex_destroy(&a->mutex);
  pthread_cond_destroy(&a->cond);

  if(a->addr)
    freeaddrinfo(a->addr);
  
  free(a);
  }


static void * lookup_thread(void * data)
  {
  lookup_async_t * a = data;

  struct addrinfo * addr;
  
  addr = hostbyname(a->hostname, a->socktype);

  pthread_mutex_lock(&a->mutex);

  //  fprintf(stderr, "gavl_socket_address_set_async done %p\n", addr);
  
  if(a->waiting)
    {
    a->addr = addr;
    a->done = 1;
    a->addr = addr;
    pthread_cond_broadcast(&a->cond);
    pthread_mutex_unlock(&a->mutex);
    }
  else
    {
    freeaddrinfo(addr);
    pthread_mutex_unlock(&a->mutex);
    async_destroy(a);
    }
  
  return NULL;
  }

int gavl_socket_address_set_async(gavl_socket_address_t * a, const char * host,
                                  int port, int socktype)
  {
  pthread_attr_t attr;
  pthread_t th;

  //  fprintf(stderr, "gavl_socket_address_set_async %s\n", host);
  
  gavl_socket_address_set_async_cancel(a);
  a->async = async_create(host, port, socktype);
  
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&th, &attr, lookup_thread, a->async);
  pthread_attr_destroy(&attr);
  
  return 1;
  }

static void cleanup_async(gavl_socket_address_t * a)
  {
  async_destroy(a->async);
  a->async = NULL;
  }

static int finish_async(gavl_socket_address_t * a)
  {
  if(!a->async->addr)
    {
    cleanup_async(a);
    return -1;
    }

  memcpy(&a->addr, a->async->addr->ai_addr, a->async->addr->ai_addrlen);
  a->len = a->async->addr->ai_addrlen;
  gavl_socket_address_set_port(a, a->async->port);
  cleanup_async(a);
  return 1;
  }

int gavl_socket_address_set_async_done(gavl_socket_address_t * a, int timeout)
  {
  struct timespec time_to_wait;
  struct timeval now;
  int64_t to;

  if(!a->async)
    return -1; // Cancelled by someone
  
  pthread_mutex_lock(&a->async->mutex);

  if(a->async->done)
    {
    pthread_mutex_unlock(&a->async->mutex);
    return finish_async(a);
    }

  if(timeout <= 0)
    {
    pthread_mutex_unlock(&a->async->mutex);
    return 0;
    }
  /* Wait on condition variable */
  gettimeofday(&now,NULL);
  
  time_to_wait.tv_sec  = now.tv_sec;
  time_to_wait.tv_nsec = now.tv_usec;
  time_to_wait.tv_nsec*= 1000;  // us -> ns

  time_to_wait.tv_sec += timeout / 1000;
  to = timeout % 1000;
  to *= (1000*1000); // ms -> us -> ns

  to += time_to_wait.tv_nsec;
  time_to_wait.tv_sec += to / (1000*1000*1000);
  time_to_wait.tv_nsec = to % (1000*1000*1000);

  if(!pthread_cond_timedwait(&a->async->cond, &a->async->mutex, &time_to_wait))
    {
    pthread_mutex_unlock(&a->async->mutex);
    return finish_async(a);
    }
  
  if((errno == ETIMEDOUT) || (errno == EINTR) || (errno == EAGAIN))
    {
    pthread_mutex_unlock(&a->async->mutex);
    return 0;
    }
  else
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "pthread_cond_timedwait failed: %s", strerror(errno));
    pthread_mutex_unlock(&a->async->mutex);
    gavl_socket_address_set_async_cancel(a);
    return -1;
    }
    
  }

void gavl_socket_address_set_async_cancel(gavl_socket_address_t * a)
  {
  if(!a->async)
    return;
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Cancelling asynchronous name lookup");
  pthread_mutex_lock(&a->async->mutex);
  a->async->waiting = 0;
  pthread_mutex_unlock(&a->async->mutex);
  a->async = NULL;
  }

