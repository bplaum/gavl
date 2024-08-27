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
#endif

#include <sys/types.h>


#ifdef HAVE_SYS_SENDFILE_H
#include <sys/sendfile.h>
#endif

#include <netinet/tcp.h> // IPPROTO_TCP, TCP_MAXSEG

#include <unistd.h>

#include <netdb.h> /* gethostbyname */

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>


#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/gavlsocket.h>

#include <gavl/log.h>
#define LOG_DOMAIN "socket"

#if !HAVE_DECL_MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#include <socket_private.h>



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
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot create TCP socket: %s", strerror(errno));
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
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot create unix socket: %s", strerror(errno));
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
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot create TCP server socket: %s", strerror(errno));
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
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot create unix server socket: %s", strerror(errno));
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
