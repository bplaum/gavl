
#include <config.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>

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

/* Opaque address structure so we can support IPv6 */

struct gavl_socket_address_s 
  {
  struct sockaddr_storage addr;
  size_t len;
  //  struct addrinfo * addr;
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
  memcpy(dst, src, sizeof(*dst));
  }

void gavl_socket_address_destroy(gavl_socket_address_t * a)
  {
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

static struct addrinfo *
hostbyname(const char * hostname, int socktype)
  {
  int err;
  struct in_addr ipv4_addr;
  struct in6_addr ipv6_addr;
  
  struct addrinfo hints;
  struct addrinfo * ret;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = PF_UNSPEC;
  hints.ai_socktype = socktype; // SOCK_STREAM, SOCK_DGRAM
  hints.ai_protocol = 0; // 0
  hints.ai_flags    = 0;

  /* prevent DNS lookup for numeric IP addresses */

  if(inet_pton(AF_INET, hostname, &ipv4_addr))
    {
    hints.ai_flags |= AI_NUMERICHOST;
    hints.ai_family   = PF_INET;
    }
  else if(inet_pton(AF_INET6, hostname, &ipv6_addr))
    {
    hints.ai_flags |= AI_NUMERICHOST;
    hints.ai_family   = PF_INET6;
    }
  
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

/* Client connection (stream oriented) */

int gavl_socket_connect_inet(gavl_socket_address_t * a, int milliseconds)
  {
  int ret = -1;
  int err;
  socklen_t err_len;

                                                                               
  /* Create the socket */
  if((ret = create_socket(a->addr.ss_family, SOCK_STREAM, 0)) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot create TCP socket");
    return -1;
    }
  
  /* Set nonblocking mode */
  if(fcntl(ret, F_SETFL, O_NONBLOCK) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot set nonblocking mode");
    return -1;
    }
  
  /* Connect the thing */
  if(connect(ret, (struct sockaddr*)&a->addr, a->len) < 0)
    {
    if(errno == EINPROGRESS)
      {
      if(!gavl_socket_can_write(ret, milliseconds))
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

  err_len = sizeof(err);
  getsockopt(ret, SOL_SOCKET, SO_ERROR, &err, &err_len);

  if(err)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Connecting failed: %s",
           strerror(err));
    return -1;
    }
  
  /* Set back to blocking mode */
  
  if(fcntl(ret, F_SETFL, 0) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot set blocking mode");
    return -1;
    }
  return ret;
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
  
  if((milliseconds >= 0) && !gavl_socket_can_read(sock, milliseconds))
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

int gavl_socket_can_read(int fd, int milliseconds)
  {
  int result;
  fd_set set;
  struct timeval timeout;
  FD_ZERO (&set);
  FD_SET  (fd, &set);

  timeout.tv_sec  = milliseconds / 1000;
  timeout.tv_usec = (milliseconds % 1000) * 1000;
    
  if((result = select(fd+1, &set, NULL, NULL, &timeout) <= 0))
    {
    if(result < 0 && (errno == EINVAL))
      {
      fprintf(stderr, "EINVAL %d\n", fd);
      }
    
    if(result < 0)
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Select for reading failed: %s", strerror(errno));
    return 0;
    }

  
  return 1;
  }


int gavl_socket_can_write(int fd, int milliseconds)
  {
  int result;
  fd_set set;
  struct timeval timeout;
  FD_ZERO (&set);
  FD_SET  (fd, &set);

  timeout.tv_sec  = milliseconds / 1000;
  timeout.tv_usec = (milliseconds % 1000) * 1000;
    
  if((result = select(fd+1, NULL, &set, NULL, &timeout) <= 0))
    {
    if(result < 0)
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Select for writing failed: %s", strerror(errno));
    // gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got read timeout");
    return 0;
    }
  return 1;
  }

int gavl_socket_read_data_noblock(int fd, uint8_t * data, int len)
  {
  int result;
  
  if(!gavl_socket_can_read(fd, 0))
    return 0;
  
  result = recv(fd, data, len, 0);

  if(result < 0)
    {
    gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Nonblocking read failed: %s", strerror(errno));
    return 0;
    }
  return result;
  }

int gavl_socket_read_data(int fd, uint8_t * data, int len, int milliseconds)
  {
  int result;
  
  if((milliseconds >= 0) && !gavl_socket_can_read(fd, milliseconds))
    {
    gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Got read timeout (t: %d, len: %d)", milliseconds, len);

    
    return 0;
    }

  while(1)
    {

    result = recv(fd, data, len, MSG_WAITALL);
    if(result < 0)
      {
      if((errno == EAGAIN) || (errno == EWOULDBLOCK))
        {
        gavl_time_t delay_time = GAVL_TIME_SCALE / 20;
        gavl_time_delay(&delay_time);
        fprintf(stderr, "gavl_socket_read_data: trying again\n");
        }
      else
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,  "Reading %d bytes failed: %s (timeout was %d)",
               len, strerror(errno), milliseconds);
        
        return 0;
        }
      }
    else if(result == 0)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN,  "Reading %d bytes failed: Connection closed (timeout was %d)",
             len, milliseconds);
      break;
      }
    else
      break;
    }
  
  return result;
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

/*
 *  Read a single line from a filedescriptor
 *
 *  ret will be reallocated if neccesary and ret_alloc will
 *  be updated then
 *
 *  The string will be 0 terminated, a trailing \r or \n will
 *  be removed
 */

#define BYTES_TO_ALLOC 1024

int gavl_socket_read_line(int fd, char ** ret,
                        int * ret_alloc, int milliseconds)
  {
  char * pos;
  char c;
  int bytes_read;
  bytes_read = 0;
  /* Allocate Memory for the case we have none */
  if(!(*ret_alloc))
    {
    *ret_alloc = BYTES_TO_ALLOC;
    *ret = realloc(*ret, *ret_alloc);
    }
  pos = *ret;
  while(1)
    {
    c = 0;
    if(!gavl_socket_read_data(fd, (uint8_t*)(&c), 1, milliseconds))
      {
      //  gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Reading line failed: %s", strerror(errno));

      if(!bytes_read)
        {
        return 0;
        }
      break;
      }

    // fprintf(stderr, "%c", c);
    
    if((c > 0x00) && (c < 0x20) && (c != '\r') && (c != '\n'))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got non ASCII character (%d) while reading line", c);
      return 0;
      }
    
    /*
     *  Line break sequence
     *  is starting, remove the rest from the stream
     */
    if(c == '\n')
      {
      break;
      }
    /* Reallocate buffer */
    else if((c != '\r') && (c != '\0'))
      {
      if(bytes_read+2 >= *ret_alloc)
        {
        *ret_alloc += BYTES_TO_ALLOC;
        *ret = realloc(*ret, *ret_alloc);
        pos = &((*ret)[bytes_read]);
        }
      /* Put the byte and advance pointer */
      *pos = c;
      pos++;
      bytes_read++;
      }
    }
  *pos = '\0';
  return 1;
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


int gavl_udp_socket_create_multicast(gavl_socket_address_t * addr)
  {
  int reuse = 1;
  int ret;
  int err;
  uint8_t loop = 1;

  gavl_socket_address_t bind_addr;
  
  if((ret = create_socket(addr->addr.ss_family, SOCK_DGRAM, 0)) < 0)
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
  
  memset(&bind_addr, 0, sizeof(bind_addr));
  
  gavl_socket_address_set(&bind_addr, "0.0.0.0",
                        gavl_socket_address_get_port(addr), SOCK_DGRAM);

  err = bind(ret, (struct sockaddr*)&addr->addr, addr->len);

  if(err)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot bind UDP socket: %s", strerror(errno));
    gavl_socket_close(ret);
    return -1;
    }
  
  if(addr->addr.ss_family == AF_INET)
    {
    struct ip_mreq req;
    struct sockaddr_in * a = (struct sockaddr_in *)(&addr->addr);
    memcpy(&req.imr_multiaddr, &a->sin_addr, sizeof(req.imr_multiaddr));
    req.imr_interface.s_addr = INADDR_ANY;

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

void gavl_unix_socket_close(int fd, char * name)
  {
  unlink(name);
  free(name);
  gavl_socket_close(fd);
  }

void gavl_socket_close(int fd)
  {
  gavl_log(GAVL_LOG_DEBUG, LOG_DOMAIN, "Closing socket %d", fd);
  close(fd);
  }
