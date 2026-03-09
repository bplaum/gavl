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



#ifndef GAVL_SAP_H_INCLUDED
#define GAVL_SAP_H_INCLUDED

#include <gavl/gavlsocket.h>

/* De-encode SAP packets */

#define GAVL_SAP_HASH    "hash" /* 16 bit hash */
#define GAVL_SAP_ADDRESS "addr" /* Originating address */
#define GAVL_SAP_SDP     "sdp"  /* Session Description Protocol */
#define GAVL_SAP_HEADER  "head"  /* SAP header as binary */

GAVL_PUBLIC
void gavl_sap_init(gavl_buffer_t * ret, const gavl_socket_address_t * addr);

GAVL_PUBLIC
int gavl_sap_decode(const gavl_buffer_t * buf, int * del, gavl_dictionary_t * ret);

GAVL_PUBLIC
int gavl_sap_encode(gavl_buffer_t * ret, int del, const gavl_dictionary_t * dict);





#endif // GAVL_SAP_H_INCLUDED
