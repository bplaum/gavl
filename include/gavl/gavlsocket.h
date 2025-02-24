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



#ifndef GAVLSOCKET_H_INCLUDED
#define GAVLSOCKET_H_INCLUDED


#include <inttypes.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <gavl/gavl.h>

/* Opaque address structure so we can support IPv6 in the future */

#define GAVL_SOCKET_ADDR_STR_LEN (46+3+5+1) // INET6_ADDRSTRLEN + []: + port + \0

typedef struct gavl_socket_address_s gavl_socket_address_t;

GAVL_PUBLIC
gavl_socket_address_t * gavl_socket_address_create();

GAVL_PUBLIC
void gavl_socket_address_destroy(gavl_socket_address_t *);

GAVL_PUBLIC
void gavl_socket_address_copy(gavl_socket_address_t * dst,
                            const gavl_socket_address_t * src);


/* Get address from hostname and port */

GAVL_PUBLIC
int gavl_socket_address_set(gavl_socket_address_t *, const char * host,
                            int port, int socktype);

GAVL_PUBLIC
int gavl_socket_address_set_async(gavl_socket_address_t *, const char * host,
                                  int port, int socktype);

GAVL_PUBLIC
int gavl_socket_address_set_async_done(gavl_socket_address_t *, int timeout);

GAVL_PUBLIC
void gavl_socket_address_set_async_cancel(gavl_socket_address_t *);

GAVL_PUBLIC
void gavl_socket_address_set_port(gavl_socket_address_t * addr, int port);

GAVL_PUBLIC
int gavl_socket_address_get_port(gavl_socket_address_t * addr);

// str must be at least GAVL_SOCKET_ADDR_STR_LEN bytes long
GAVL_PUBLIC
char * gavl_socket_address_to_string(const gavl_socket_address_t * addr, char * str);

GAVL_PUBLIC
int gavl_socket_address_from_string(gavl_socket_address_t * addr, const char * str1);

#define GAVL_NI_LOOPBACK (1<<0) // Return loopback interfaces
#define GAVL_NI_IPV4     (1<<1) // Return IPV4 capable interfaces
#define GAVL_NI_IPV6     (1<<2) // Return IPV6 capable interfaces

#define GAVL_NI_ALL      (~0)

GAVL_PUBLIC
gavl_socket_address_t ** gavl_get_network_interfaces(int flags);

GAVL_PUBLIC
void gavl_socket_address_destroy_array(gavl_socket_address_t ** addr);

GAVL_PUBLIC
char * gavl_interface_name_from_address(const gavl_socket_address_t * a1);

GAVL_PUBLIC
int gavl_interface_index_from_address(const gavl_socket_address_t * a1);

/* Reverse DNS */
GAVL_PUBLIC
char * gavl_socket_address_get_hostname(gavl_socket_address_t * addr);

/* Returns AF_* macros */
GAVL_PUBLIC
int gavl_socket_address_get_address_family(gavl_socket_address_t * addr);

GAVL_PUBLIC
gavl_socket_address_t ** gavl_lookup_hostname_full(const char * hostname, int socktype);

GAVL_PUBLIC
void gavl_socket_address_free_array(gavl_socket_address_t ** addr);

/* Wildcard can be "0.0.0.0" for IPV4 or "::" for IPV6 */
GAVL_PUBLIC
int gavl_socket_address_set_local(gavl_socket_address_t * a, int port, const char * wildcard);

/*
 *  Client connection (stream oriented)
 *  timeout is in milliseconds
 */

GAVL_PUBLIC
int gavl_socket_connect_inet(gavl_socket_address_t*, int timeout);

/* Check if a previous call with zero timeout succeeded
   Retur value: -1: Error, 0: In Progress, 1. Connected */
GAVL_PUBLIC
int gavl_socket_connect_inet_complete(int fd, int milliseconds);

GAVL_PUBLIC
int gavl_socket_connect_unix(const char *);

GAVL_PUBLIC
void gavl_socket_disconnect(int);


/* Get last error */
GAVL_PUBLIC
int gavl_socket_get_errno(int fd);

/* Wait for a specified time until the socket is readable.
   If yes, try to peek one byte from the socket (leaving it in the receive buffer).
   It zero bytes are read, the remote end disconnected */

GAVL_PUBLIC
int gavl_socket_is_disconnected(int fd, int timeout);

/* Server socket (stream oriented) */

// #define GAVL_SOCKET_IPV6     (1<<0)
#define GAVL_SOCKET_LOOPBACK (1<<1)
#define GAVL_SOCKET_REUSEADDR (1<<2)

GAVL_PUBLIC
int gavl_listen_socket_create_inet(gavl_socket_address_t * addr,
                                   int port,
                                   int queue_size,
                                   int flags);

GAVL_PUBLIC
int gavl_listen_socket_create_unix(const char * name,
                                 int queue_size);

GAVL_PUBLIC
int gavl_unix_socket_create(char ** name, int queue_size);


/* Accept a new client connection, return -1 if there is none */

GAVL_PUBLIC
int gavl_listen_socket_accept(int sock, int milliseconds,
                              gavl_socket_address_t * from);

GAVL_PUBLIC
void gavl_listen_socket_destroy(int);

GAVL_PUBLIC
int gavl_socket_get_address(int sock, gavl_socket_address_t * local, gavl_socket_address_t * remote);

/* UDP */

GAVL_PUBLIC
int gavl_udp_socket_create(gavl_socket_address_t * addr);

GAVL_PUBLIC
int gavl_udp_socket_connect(int fd, gavl_socket_address_t * dst);

GAVL_PUBLIC
int gavl_udp_socket_create_multicast(gavl_socket_address_t * multicast_addr,
                                     gavl_socket_address_t * interface_addr);

GAVL_PUBLIC
int gavl_udp_socket_set_multicast_interface(int fd, gavl_socket_address_t * interface_addr);

GAVL_PUBLIC
int gavl_udp_socket_receive(int fd, uint8_t * data, int data_size,
                          gavl_socket_address_t * addr);

GAVL_PUBLIC
int gavl_udp_socket_send(int fd, const uint8_t * data, int data_size,
                       gavl_socket_address_t * addr);

/* I/0 functions */

GAVL_PUBLIC
int gavl_socket_read_data(int fd, uint8_t * data, int len, int milliseconds);

GAVL_PUBLIC
int gavl_socket_read_data_noblock(int fd, uint8_t * data, int len);

GAVL_PUBLIC
int gavl_socket_write_data(int fd, const void * data, int len);

GAVL_PUBLIC
int gavl_socket_write_data_nonblock(int fd, const uint8_t * data, int len);

GAVL_PUBLIC
int gavl_socket_is_local(int fd);

GAVL_PUBLIC
int gavl_socket_is_unix(int fd);


GAVL_PUBLIC
int gavl_socket_send_file(int fd, const char * filename,
                        int64_t offset, int64_t len);

GAVL_PUBLIC
int gavl_socket_send_fds(int socket, const int * fds, int n);

GAVL_PUBLIC
int gavl_socket_recv_fds(int socket, int * fds, int n);


GAVL_PUBLIC
void gavl_socket_close(int fd);

#endif // GAVLSOCKET_H_INCLUDED
