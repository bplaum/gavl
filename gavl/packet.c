/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2012 Members of the Gmerlin project
 * gmerlin-general@lists.sourceforge.net
 * http://gmerlin.sourceforge.net
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#include <config.h>

#include <gavl/gavl.h>
#include <gavl/compression.h>

#include <gavl/log.h>
#define LOG_DOMAIN "packet"

static void free_extradata(gavl_packet_t * p)
  {
  int i;
  for(i = 0; i < GAVL_PACKET_MAX_EXTRADATA; i++)
    {
    if(p->ext_data[i].data)
      {
      switch(p->ext_data[i].type)
        {
        case GAVL_PACKET_EXTRADATA_PALETTE:
          {
          gavl_packet_palette_t * pal = p->ext_data[i].data;
          if(pal->entries)
            free(pal->entries);
          }
          break;
        default:
          break;
        }
      free(p->ext_data[i].data);
      }
    }
  memset(p->ext_data, 0, sizeof(p->ext_data));
  }

void gavl_packet_alloc(gavl_packet_t * p, int len)
  {
  gavl_buffer_alloc(&p->buf, len + GAVL_PACKET_PADDING);
  memset(p->buf.buf + len, 0, GAVL_PACKET_PADDING);
  }

void gavl_packet_free(gavl_packet_t * p)
  {
  free_extradata(p);
  gavl_buffer_free(&p->buf);
  }

void gavl_packet_reset(gavl_packet_t * p)
  {
  gavl_buffer_t buf_save;

  free_extradata(p);
  
  memcpy(&buf_save, &p->buf, sizeof(p->buf));
  gavl_packet_init(p);
  memcpy(&p->buf, &buf_save, sizeof(p->buf));
  gavl_buffer_reset(&p->buf);
  }

void gavl_packet_copy(gavl_packet_t * dst,
                      const gavl_packet_t * src)
  {
  int data_alloc_save;
  uint8_t * data_save;

  data_alloc_save = dst->buf.alloc;
  data_save       = dst->buf.buf;

  memcpy(dst, src, sizeof(*src));

  dst->buf.alloc = data_alloc_save;
  dst->buf.buf   = data_save;

  gavl_packet_alloc(dst, src->buf.len);
  memcpy(dst->buf.buf, src->buf.buf, src->buf.len);
  }

void gavl_packet_copy_metadata(gavl_packet_t * dst,
                               const gavl_packet_t * src)
  {
  gavl_buffer_t buf_save;
  gavl_packet_extradata_t ext_data_save[GAVL_PACKET_MAX_EXTRADATA];
  
  memcpy(&buf_save, &dst->buf, sizeof(buf_save));
  memcpy(ext_data_save, &dst->ext_data, sizeof(ext_data_save));
  
  memcpy(dst, src, sizeof(*src));
  
  memcpy(&dst->buf, &buf_save, sizeof(buf_save));
  memcpy(&dst->ext_data, ext_data_save, sizeof(ext_data_save));
  }

static const char * coding_type_strings[4] =
  {
  "?",
  "I",
  "P",
  "B"
  };

void gavl_packet_dump(const gavl_packet_t * p)
  {
  gavl_dprintf("sz: %d ", p->buf.len);

  if(p->pts != GAVL_TIME_UNDEFINED)
    gavl_dprintf("pts: %"PRId64" ", p->pts);
  else
    gavl_dprintf("pts: None ");

  gavl_dprintf("dur: %"PRId64, p->duration);

  gavl_dprintf(" head: %d, f2: %d",
          p->header_size, p->field2_offset);

  gavl_dprintf(" type: %s ", coding_type_strings[p->flags & GAVL_PACKET_TYPE_MASK]);

  if(p->flags & GAVL_PACKET_NOOUTPUT)
    gavl_dprintf(" nooutput");

  if(p->flags & GAVL_PACKET_REF)
    gavl_dprintf(" ref");
  
  if(p->src_rect.w && p->src_rect.h)
    {
    gavl_dprintf(" src_rect: ");
    gavl_rectangle_i_dump(&p->src_rect);
    }
  if(p->dst_x || p->dst_y)
    gavl_dprintf(" dst: %d %d", p->dst_x, p->dst_y);

  gavl_dprintf("\n");
  gavl_hexdump(p->buf.buf, p->buf.len < 16 ? p->buf.len : 16, 16);
  
  }

void gavl_packet_save(const gavl_packet_t * p,
                      const char * filename)
  {
  FILE * out = fopen(filename, "wb");
  fwrite(p->buf.buf, 1, p->buf.len, out);
  fclose(out);
  }

void gavl_packet_init(gavl_packet_t * p)
  {
  memset(p, 0, sizeof(*p));
  p->timecode   = GAVL_TIMECODE_UNDEFINED;
  p->pts        = GAVL_TIMECODE_UNDEFINED;

  p->buf_idx = -1;
  }

gavl_packet_t * gavl_packet_create()
  {
  gavl_packet_t * ret;
  ret = calloc(1, sizeof(*ret));
  gavl_packet_init(ret);
  return ret;
  }

void gavl_packet_destroy(gavl_packet_t * p)
  {
  gavl_packet_free(p);
  free(p);
  }


void * gavl_packet_add_extradata(gavl_packet_t * p, gavl_packet_extradata_type_t type)
  {
  void * ret;
  int i;
  gavl_packet_extradata_t * d = NULL;

  if(type == GAVL_PACKET_EXTRADATA_NONE)
    {
    gavl_log(GAVL_LOG_WARNING, LOG_DOMAIN, "Cannot add extradata of type None");
    return NULL;
    }
  
  if((ret = gavl_packet_get_extradata(p, type)))
    {
    gavl_log(GAVL_LOG_WARNING, LOG_DOMAIN, "Packet already has extradata of type %d", type);
    return ret;
    }
  
  for(i = 0; i < GAVL_PACKET_MAX_EXTRADATA; i++)
    {
    if(p->ext_data[i].type == GAVL_PACKET_EXTRADATA_NONE)
      d = &p->ext_data[i];
    }

  if(!d)
    {
    gavl_log(GAVL_LOG_WARNING, LOG_DOMAIN, "Cannot add packet extradata: No free slot");
    return NULL;
    }

  d->type = type;
  
  switch(type)
    {
    case GAVL_PACKET_EXTRADATA_PALETTE:
      d->data = calloc(1, sizeof(gavl_packet_palette_t));
      break;
    case GAVL_PACKET_EXTRADATA_FDS:
      d->data = calloc(1, sizeof(gavl_packet_fds_t));
      break;
    case GAVL_PACKET_EXTRADATA_NONE:
      break;
    }
  return d->data;
  }

void * gavl_packet_get_extradata(gavl_packet_t * p, gavl_packet_extradata_type_t type)
  {
  int i;
  for(i = 0; i < GAVL_PACKET_MAX_EXTRADATA; i++)
    {
    if(p->ext_data[i].type == type)
      return p->ext_data[i].data;
    }
  return NULL;
  }
