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
