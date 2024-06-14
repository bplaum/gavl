#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <config.h>

#include <gavfprivate.h>
#include <gavl/log.h>
#include <gavl/gavlsocket.h>

#define LOG_DOMAIN "gavlstructs"

/* Formats */

#define MAX_EXT_SIZE_PK 32

/* Packet */

int gavf_write_gavl_packet(gavl_io_t * io,
                           const gavf_stream_t * s,
                           const gavl_packet_t * p)
  {
#if 0
  uint32_t num_extensions;

  uint8_t data[MAX_EXT_SIZE_PK];
  gavl_buffer_t buf;
  gavl_io_t bufio;
  uint32_t flags;
  int write_duration = 0;
  
  /* Count extensions */
  num_extensions = 0;
  
  if(!s || !s->packet_duration || (p->duration != s->packet_duration))
    {
    num_extensions++;
    write_duration = 1;
    }
  
  if(p->header_size)
    num_extensions++;

  if(p->sequence_end_pos)
    num_extensions++;

  if(p->field2_offset)
    num_extensions++;
  
  if(p->timecode != GAVL_TIMECODE_UNDEFINED)
    num_extensions++;

  if(p->interlace_mode != GAVL_INTERLACE_NONE)
    num_extensions++;

  if(p->src_rect.w && p->src_rect.h)
    num_extensions++;

  if(p->dst_x || p->dst_y)
    num_extensions++;
  
  /* Flags */

  flags = p->flags;
  if(num_extensions)
    flags |= GAVL_PACKET_EXT;

  /*
   * 1 byte "P"
   * stream_id: int32v
   * pts:       int64v
   * flags:     uint32v
   * if(flags & GAVL_PACKET_EXT)
   *   {
   *   extension[i]
   *   }
   * 
   */
  
  if(!gavl_io_write_data(io, (uint8_t*)GAVF_TAG_PACKET_HEADER, 1))
    goto fail;
  
  if(!gavl_io_write_int32v(io, p->id))
    goto fail;

  /* PTS */
  if(!gavl_io_write_int64v(io, p->pts))
    goto fail;
  
  if(!gavl_io_write_uint32v(io, flags))
    goto fail;
  
  /* Write Extensions */

  if(num_extensions)
    {
    if(!gavl_io_write_uint32v(io, num_extensions))
      goto fail;
    
    gavl_buffer_init_static(&buf, data, MAX_EXT_SIZE_AF);
    gavl_io_init_buf_write(&bufio, &buf);
    
    if(write_duration)
      {
      gavl_io_buf_reset(&bufio);
      if(!gavl_io_write_int64v(&bufio, p->duration) ||
         !gavf_extension_write(io, GAVF_EXT_PK_DURATION,
                               buf.len, buf.buf))
        goto fail;
      }
    
    /* Field2 offset */
    if(p->field2_offset)
      {
      gavl_io_buf_reset(&bufio);
      if(!gavl_io_write_uint32v(&bufio, p->field2_offset) ||
         !gavf_extension_write(io, GAVF_EXT_PK_FIELD2,
                               buf.len, buf.buf))
        goto fail;
      }
    
    /* Interlace mode */
    if(p->interlace_mode != GAVL_INTERLACE_NONE)
      {
      gavl_io_buf_reset(&bufio);
      if(!gavl_io_write_uint32v(&bufio, p->interlace_mode) ||
         !gavf_extension_write(io, GAVF_EXT_PK_INTERLACE,
                               buf.len, buf.buf))
        goto fail;
      }
    
    if(p->header_size)
      {
      gavl_io_buf_reset(&bufio);
      if(!gavl_io_write_uint32v(&bufio, p->header_size) ||
         !gavf_extension_write(io, GAVF_EXT_PK_HEADER_SIZE,
                               buf.len, buf.buf))
        goto fail;
      }
    if(p->sequence_end_pos)
      {
      gavl_io_buf_reset(&bufio);
      if(!gavl_io_write_uint32v(&bufio, p->sequence_end_pos) ||
         !gavf_extension_write(io, GAVF_EXT_PK_SEQ_END,
                               buf.len, buf.buf))
        goto fail;
      }

    if(p->timecode != GAVL_TIMECODE_UNDEFINED)
      {
      gavl_io_buf_reset(&bufio);
      if(!gavl_io_write_uint64f(&bufio, p->timecode) ||
         !gavf_extension_write(io, GAVF_EXT_PK_TIMECODE,
                               buf.len, buf.buf))
        goto fail;
      }

    if(p->src_rect.w && p->src_rect.h)
      {
      gavl_io_buf_reset(&bufio);
      
      if(!gavl_io_write_int32v(&bufio, p->src_rect.x) ||
         !gavl_io_write_int32v(&bufio, p->src_rect.y) ||
         !gavl_io_write_int32v(&bufio, p->src_rect.w) ||
         !gavl_io_write_int32v(&bufio, p->src_rect.h) ||
         !gavf_extension_write(io, GAVF_EXT_PK_SRC_RECT,
                               buf.len, buf.buf))
        goto fail;
      }

    if(p->dst_x || p->dst_y)
      {
      gavl_io_buf_reset(&bufio);
      if(!gavl_io_write_int32v(&bufio, p->dst_x) ||
         !gavl_io_write_int32v(&bufio, p->dst_y) ||
         !gavf_extension_write(io, GAVF_EXT_PK_DST_COORDS,
                               buf.len, buf.buf))
        goto fail;
      }
#if 0
    if(p->num_fds)
      {
      gavl_io_buf_reset(&bufio);
      if(!gavl_io_write_uint32v(&bufio, p->num_fds) ||
         !gavf_extension_write(io, GAVF_EXT_PK_FDS,
                               buf.len, buf.buf))
        goto fail;
      }
#endif
    if(p->buf_idx >= 0)
      {
      gavl_io_buf_reset(&bufio);
      if(!gavl_io_write_uint32v(&bufio, p->buf_idx) ||
         !gavf_extension_write(io, GAVF_EXT_PK_BUF_IDX,
                               buf.len, buf.buf))
        goto fail;
      }
    
    }

  if(!gavl_io_write_uint32v(io, p->buf.len))
    goto fail;

  if(p->buf.len && (gavl_io_write_data(io, p->buf.buf, p->buf.len) < p->buf.len))
    goto fail;

#if 0  
  if(p->num_fds)    
    {
    int sock;

    if(!(gavl_io_get_flags(io) & GAVF_IO_IS_UNIX_SOCKET))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Passing FDs works only with UNIX-Sockets");
      return 0;
      }
    
    sock = gavl_io_get_socket(io);
    
    if(!gavl_socket_send_fds(sock, p->fds, p->num_fds))
      goto fail;
    }
#endif
  
  return 1;

  fail:
#endif
  return 0;
  }


int gavf_read_gavl_packet_header(gavl_io_t * io,
                                 gavl_packet_t * p)
  {
#if 0
  int ret = 0;
  uint8_t c;
  
  gavl_packet_reset(p);

  /* P */
  if(!gavl_io_get_data(io, &c, 1) || (c != GAVF_TAG_PACKET_HEADER_C))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet marker");
    goto fail;
    }
  gavl_io_skip(io, 1);
  
  /* Stream ID */
  if(!gavl_io_read_int32v(io, &p->id))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read stream ID");
    goto fail;
    }
  /* PTS */
  if(!gavl_io_read_int64v(io, &p->pts))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet pts");
    goto fail;
    }
  
  /* Flags */
  if(!gavl_io_read_uint32v(io, (uint32_t*)&p->flags))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet flags");
    goto fail;
    }
  if(p->flags & GAVL_PACKET_EXT)
    {
    uint32_t num_extensions;
    int i;
    gavl_extension_header_t eh;
    
    /* Extensions */
    if(!gavl_io_read_uint32v(io, &num_extensions))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read number of packet extensions");
      goto fail;
      }
    for(i = 0; i < num_extensions; i++)
      {
      if(!gavf_extension_header_read(io, &eh))
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet extension header");
        goto fail;
        }
      switch(eh.key)
        {
        case GAVF_EXT_PK_DURATION:
          if(!gavl_io_read_int64v(io, &p->duration))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet duration");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_HEADER_SIZE:
          if(!gavl_io_read_uint32v(io, &p->header_size))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet header_size");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_SEQ_END:
          if(!gavl_io_read_uint32v(io, &p->sequence_end_pos))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet sequence_end_pos");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_TIMECODE:
          if(!gavl_io_read_uint64f(io, &p->timecode))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet timecode");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_SRC_RECT:
          if(!gavl_io_read_int32v(io, &p->src_rect.x) ||
             !gavl_io_read_int32v(io, &p->src_rect.y) ||
             !gavl_io_read_int32v(io, &p->src_rect.w) ||
             !gavl_io_read_int32v(io, &p->src_rect.h))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet src_rect");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_DST_COORDS:
          if(!gavl_io_read_int32v(io, &p->dst_x) ||
             !gavl_io_read_int32v(io, &p->dst_y))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet dst_coords");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_FIELD2:
          if(!gavl_io_read_uint32v(io, &p->field2_offset))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet field2_offset");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_INTERLACE:
          if(!gavl_io_read_uint32v(io, (uint32_t*)&p->interlace_mode))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet interlace_mode");
            goto fail;
            }
            break;
#if 0
        case GAVF_EXT_PK_FDS:
          if(!gavl_io_read_uint32v(io, (uint32_t*)&p->num_fds))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet num_fds");
            goto fail;
            }
          break;
#endif
        case GAVF_EXT_PK_BUF_IDX:
          if(!gavl_io_read_uint32v(io, (uint32_t*)&p->buf_idx))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet buf_idx");
            goto fail;
            }
          break;
        default:
          /* Skip */
          gavl_io_skip(io, eh.len);
          break;
        }
      }
    }

  if(!gavl_io_read_uint32v(io, (uint32_t*)&p->buf.len))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet data_len");
    goto fail;
    }
  p->flags &= ~GAVL_PACKET_EXT;

  ret = 1;
  
  fail:
  return ret;
#endif
  return 0;
  }

int gavf_read_gavl_packet(gavl_io_t * io,
                          gavl_packet_t * p)
  {

  /* Payload */
  gavl_packet_alloc(p, p->buf.len);
  if(gavl_io_read_data(io, p->buf.buf, p->buf.len) < p->buf.len)
    goto fail;
#if 0
  if(p->num_fds)
    {
    int sock;

    if(!(gavl_io_get_flags(io) & GAVF_IO_IS_UNIX_SOCKET))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Passing FDs works only with UNIX-Sockets");
      return 0;
      }
    
    sock = gavl_io_get_socket(io);

    if(!gavl_socket_recv_fds(sock, p->fds, p->num_fds))
      goto fail;
    
    }
#endif
  return 1;
  
  fail:

  //  if(s->g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
  //    fprintf(stderr, "Got EOF while reading packet\n");
  return 0;
  }

int gavf_skip_gavl_packet(gavl_io_t * io,
                          gavl_packet_t * p)
  {
  //  int i;
  gavl_io_skip(io, p->buf.len);
#if 0
  if(p->num_fds)
    {
    int sock;

    if(!(gavl_io_get_flags(io) & GAVF_IO_IS_UNIX_SOCKET))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Passing FDs works only with UNIX-Sockets");
      return 0;
      }
    
    sock = gavl_io_get_socket(io);

    if(!gavl_socket_recv_fds(sock, p->fds, p->num_fds))
      return 0;

    for(i = 0; i < p->num_fds; i++)
      {
      close(p->fds[i]);
      p->fds[i] = -1;
      }
    }
#endif
  return 1;
  
  }

