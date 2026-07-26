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

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <config.h>


#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/gavlsocket.h>
#include <gavl/sap.h>
#include <gavl/metatags.h>

#include <socket_private.h>

#include <gavl/log.h>
#define LOG_DOMAIN "sap"

static const char * mimetype = "application/sdp";

void gavl_sap_init(gavl_dictionary_t * dict, const gavl_socket_address_t * addr)
  {
  int header_len = 4;
  unsigned int hash = time(NULL);
  gavl_buffer_t header;
  gavl_buffer_init(&header);
      
  rand_r(&hash);
  
  if(addr->addr.ss_family == AF_INET)
    {
    header_len += 4;
    }
  else if(addr->addr.ss_family == AF_INET6)
    {
    header_len += 16;
    }

  header_len += (strlen(mimetype) + 1);

  gavl_buffer_alloc(&header, header_len);

  /* flags: version=1, IPv4, announce, no compression */
  header.buf[0] = 0x20;

  if(addr->addr.ss_family == AF_INET6)
    {
    header.buf[0] |= 0x10;
    }

  header.buf[1] = 0x00; // Auth len

  header.buf[2] = (hash >> 8) & 0xff;
  header.buf[3] = hash & 0xff;

  header.len = 4;

  if(addr->addr.ss_family == AF_INET)
    {
    struct sockaddr_in * a = (struct sockaddr_in*)&addr->addr;
    memcpy(header.buf + 4, &a->sin_addr, 4);
    header.len += 4;
    }
  else if(addr->addr.ss_family == AF_INET6)
    {
    struct sockaddr_in6 * a = (struct sockaddr_in6*)&addr->addr;
    memcpy(header.buf + 4, &a->sin6_addr, 16);
    header.len += 16;
    }

  gavl_buffer_append_data(&header, (const uint8_t*)mimetype, strlen(mimetype)+1);
  gavl_dictionary_set_binary(dict, GAVL_SAP_HEADER, header.buf, header.len);
  gavl_buffer_free(&header);
  
  
  //  gavl_dictionary_set_int(ret, GAVL_SAP_HASH, hash);
  
  // gavl_dictionary_set_string(ret, GAVL_SAP_ADDRESS, addr);
  
  }

int gavl_sap_decode(const gavl_buffer_t * buf, int * del, gavl_dictionary_t * ret)
  {
  int flags;
  int auth_len;
  int addr_len;
  const char * str;
  const char * end;
  const char * pos;
  const uint8_t * data = buf->buf;
  
  flags = data[0];

  if(((flags >> 5) & 0x07) != 1) // Version
    {
    gavl_log(GAVL_LOG_WARNING, LOG_DOMAIN, "Version mismatch");
    return 0;
    }
  
  if(flags & (1<<4)) // Originating address IPV4 or IPV6?
    {
    char str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, buf->buf + 4, str, INET6_ADDRSTRLEN);
    addr_len = 16;
    gavl_dictionary_set_string(ret, GAVL_SAP_ADDRESS, str);
    }
  else
    {
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, buf->buf + 4, str, INET_ADDRSTRLEN);
    addr_len = 4;
    gavl_dictionary_set_string(ret, GAVL_SAP_ADDRESS, str);
    }
  
  if(flags & (1<<2))
    *del = 1;
  else
    *del = 0;

  if(flags & (1<<1))
    {
    gavl_log(GAVL_LOG_WARNING, LOG_DOMAIN, "Got encrypted SAP packet");
    return 0;
    }

  if(flags & (1<<0))
    {
    gavl_log(GAVL_LOG_WARNING, LOG_DOMAIN, "Got compressed SAP packet");
    return 0;
    }
  
  auth_len = data[1] * 4;
  
  str = (const char*)(data + (4 + addr_len + auth_len));
  end = (const char*)(data + buf->len);

  //  fprintf(stderr, "Got str: %s\n", str);

  if((pos = memchr(str, '\0', end - str)) &&
     (pos != end) &&
     !strcasecmp(str, "application/sdp"))
    {
    char id[GAVL_MD5_LENGTH];
    int64_t version;
    
    pos++;

    gavl_dictionary_set_string(ret, GAVL_SAP_SDP, pos);

    gavl_sdp_get_session_id(pos, id, &version);
    
    gavl_dictionary_set_string(ret, GAVL_META_ID, id);
    gavl_dictionary_set_long(ret, GAVL_SAP_SESSION_VERSION, version);
    
    return 1;
    }
  return 0;
  
  }

int gavl_sap_encode(gavl_buffer_t * ret, int del, const gavl_dictionary_t * dict)
  {
  const char * sdp;
  const gavl_buffer_t * buf = gavl_dictionary_get_binary(dict, GAVL_SAP_HEADER);
  
  gavl_buffer_reset(ret);
  gavl_buffer_append(ret, buf);

  if(del)
    ret->buf[0] |= 0x04;
  
  if((sdp = gavl_dictionary_get_string(dict, GAVL_SAP_SDP)))
    gavl_buffer_append_data(ret, (const uint8_t*)sdp, strlen(sdp));
  
  return 1;
  }

int gavl_parse_sdp_o(const char * sdp,
                     char ** user,
                     int64_t * id,
                     int64_t * version,
                     char ** nettype,
                     char ** addrtype,
                     char ** addr)
  {
  const char *start;
  const char *end;
  char *tmp_string;
  char ** el;
  
  if(!(start = strstr(sdp, "o=")))
    return 0;

  start+=2;
  
  end = strchr(start, '\r');

  if(!end)
    end = start + strlen(start);

  tmp_string = gavl_strndup(start, end);
  el = gavl_strbreak(tmp_string, ' ');

  if(!el || !el[0] || !el[1] || !el[2] || !el[3] || !el[4] || !el[5] || el[6])
    {
    free(tmp_string);
    gavl_strbreak_free(el);
    return 0;
    }

  fprintf(stderr, "* * * ID: %s\n", el[1]);
  
  if(user)
    *user = gavl_strdup(el[0]);
  if(id)
    *id = strtoll(el[1], NULL, 10);
  if(version)
    *version = strtoll(el[2], NULL, 10);
  if(nettype)
    *nettype = gavl_strdup(el[3]);
  if(addrtype)
    *addrtype = gavl_strdup(el[4]);
  if(addr)
    *addr = gavl_strdup(el[5]);
 
    
  free(tmp_string);
  gavl_strbreak_free(el);
  return 1;
  }

/* Get a unique session id from the sdp */
int gavl_sdp_get_session_id(const char * sdp,
                            char ret[GAVL_MD5_LENGTH], int64_t * version)
  {
  char * user = NULL;
  int64_t id;
  char * addr = NULL;
  char * tmp_string = NULL;
  
  if(!gavl_parse_sdp_o(sdp, &user,
                       &id,
                       version,
                       NULL,
                       NULL,
                       &addr))
    return 0;

  tmp_string = gavl_sprintf("%s-%"PRId64"-%s", user, id, addr);

  gavl_md5_buffer_str(tmp_string, strlen(tmp_string), ret);

  //  fprintf(stderr, "Get session id: %s -> %s", tmp_string, ret);
  
  free(tmp_string);
  free(addr);
  free(user);
  return 1;
  }
                               
                               
