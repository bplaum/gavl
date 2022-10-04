#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <config.h>

#include <gavfprivate.h>
#include <gavl/log.h>
#include <gavl/gavlsocket.h>

#define LOG_DOMAIN "gavlstructs"

/* Formats */

#define MAX_EXT_SIZE_AF 16
#define MAX_EXT_SIZE_VF 32
#define MAX_EXT_SIZE_PK 32
#define MAX_EXT_SIZE_CI 16

int gavf_read_audio_format(gavf_io_t * io, gavl_audio_format_t * format)
  {
  int i;
  uint32_t num_extensions;
  
  gavl_extension_header_t eh;

  memset(format, 0, sizeof(*format));

  if(!gavf_io_read_uint32v(io, &format->samplerate) ||
     !gavf_io_read_uint32v(io, &format->num_channels))
    return 0;

  for(i = 0; i < format->num_channels; i++)
    {
    if(!gavf_io_read_uint32v(io, &format->channel_locations[i]))
      return 0;
    }

  if(!gavf_io_read_uint32v(io, &num_extensions))
    return 0;

  for(i = 0; i < num_extensions; i++)
    {
    if(!gavf_extension_header_read(io, &eh))
      return 0;
    switch(eh.key)
      {
      case GAVF_EXT_AF_SAMPLESPERFRAME:
        if(!gavf_io_read_uint32v(io, &format->samples_per_frame))
          return 0;
        break;
      case GAVF_EXT_AF_SAMPLEFORMAT:
        if(!gavf_io_read_uint32v(io, &format->sample_format))
          return 0;
        break;
      case GAVF_EXT_AF_INTERLEAVE:
        if(!gavf_io_read_uint32v(io, &format->interleave_mode))
          return 0;
        break;
      case GAVF_EXT_AF_CENTER_LEVEL:
        if(!gavf_io_read_float(io, &format->center_level))
          return 0;
        break;
      case GAVF_EXT_AF_REAR_LEVEL:
        if(!gavf_io_read_float(io, &format->rear_level))
          return 0;
        break;
      default:
        /* Skip */
        gavf_io_skip(io, eh.len);
        break;
      }
    }
  return 1;
  }

int gavf_write_audio_format(gavf_io_t * io, const gavl_audio_format_t * format)
  {
  int ret = 0;

  int num_extensions;
  uint8_t data[MAX_EXT_SIZE_AF];
  gavl_buffer_t buf;
  gavf_io_t bufio;
  int i;

  gavl_buffer_init_static(&buf, data, MAX_EXT_SIZE_AF);
  gavf_io_init_buf_write(&bufio, &buf);
  
  /* Write common stuff */
  if(!gavf_io_write_uint32v(io, format->samplerate) ||
     !gavf_io_write_uint32v(io, format->num_channels))
    goto fail;
  
  for(i = 0; i < format->num_channels; i++)
    {
    if(!gavf_io_write_uint32v(io, format->channel_locations[i]))
      goto fail;
    }

  /* Count extensions */
  num_extensions = 0;

  if(format->samples_per_frame != 0)
    num_extensions++;

  if(format->interleave_mode != GAVL_INTERLEAVE_NONE)
    num_extensions++;

  if(format->sample_format != GAVL_SAMPLE_NONE)
    num_extensions++;

  if(format->center_level != 0.0)
    num_extensions++;

  if(format->rear_level != 0.0)
    num_extensions++;
  
  /* Write extensions */
  if(!gavf_io_write_uint32v(io, num_extensions))
    goto fail;
  
  if(!num_extensions)
    {
    ret = 1;
    goto fail;
    }
  
  if(format->samples_per_frame != 0)
    {
    gavf_io_buf_reset(&bufio);
    if(!gavf_io_write_uint32v(&bufio, format->samples_per_frame) ||
       !gavf_extension_write(io, GAVF_EXT_AF_SAMPLESPERFRAME,
                            buf.len, buf.buf))
      goto fail;
    }
  if(format->interleave_mode != GAVL_INTERLEAVE_NONE)
    {
    gavf_io_buf_reset(&bufio);
    if(!gavf_io_write_uint32v(&bufio, format->interleave_mode) ||
       !gavf_extension_write(io, GAVF_EXT_AF_INTERLEAVE,
                            buf.len, buf.buf))
      goto fail;
    }
  if(format->sample_format != GAVL_SAMPLE_NONE)
    {
    gavf_io_buf_reset(&bufio);
    if(!gavf_io_write_uint32v(&bufio, format->sample_format) ||
       !gavf_extension_write(io, GAVF_EXT_AF_SAMPLEFORMAT,
                            buf.len, buf.buf))
      goto fail;
    }
  if(format->center_level != 0.0)
    {
    gavf_io_buf_reset(&bufio);
    if(!gavf_io_write_float(&bufio, format->center_level) ||
       !gavf_extension_write(io, GAVF_EXT_AF_CENTER_LEVEL,
                            buf.len, buf.buf))
      goto fail;
    }
  if(format->rear_level != 0.0)
    {
    gavf_io_buf_reset(&bufio);
    if(!gavf_io_write_float(&bufio, format->rear_level) ||
       !gavf_extension_write(io, GAVF_EXT_AF_REAR_LEVEL,
                            buf.len, buf.buf))
      goto fail;
    }

  ret = 1;
  fail:
  
  gavf_io_cleanup(&bufio);
  return ret;
  }

int gavf_read_video_format(gavf_io_t * io, gavl_video_format_t * format)
  {
  int i;
  uint32_t num_extensions;
  
  gavl_extension_header_t eh;

  memset(format, 0, sizeof(*format));
  
  /* Read mandatory stuff */
  if(!gavf_io_read_uint32v(io, &format->image_width) ||
     !gavf_io_read_uint32v(io, &format->image_height) ||
     !gavf_io_read_int32v(io, &format->framerate_mode) ||
     !gavf_io_read_uint32v(io, &format->timescale))
    return 0;

  if(format->framerate_mode != GAVL_FRAMERATE_STILL)
    {
    if(!gavf_io_read_uint32v(io, &format->frame_duration))
      return 0;
    }

  /* Set defaults */
  format->pixel_width = 1;
  format->pixel_height = 1;
  
  /* Read extensions */
  if(!gavf_io_read_uint32v(io, &num_extensions))
    return 0;

  for(i = 0; i < num_extensions; i++)
    {
    if(!gavf_extension_header_read(io, &eh))
      return 0;
    switch(eh.key)
      {
      case GAVF_EXT_VF_PIXELFORMAT:
        if(!gavf_io_read_uint32v(io, &format->pixelformat))
          return 0;
        break;
      case GAVF_EXT_VF_PIXEL_ASPECT:
        if(!gavf_io_read_uint32v(io, &format->pixel_width) ||
           !gavf_io_read_uint32v(io, &format->pixel_height))
          return 0;
        break;
      case GAVF_EXT_VF_INTERLACE:
        if(!gavf_io_read_int32v(io, &format->interlace_mode))
          return 0;
        break;
      case GAVF_EXT_VF_FRAME_SIZE:
        if(!gavf_io_read_uint32v(io, &format->frame_width) ||
           !gavf_io_read_uint32v(io, &format->frame_height))
          return 0;
        break;
      case GAVF_EXT_VF_TC_RATE:
        if(!gavf_io_read_uint32v(io, &format->timecode_format.int_framerate))
          return 0;
        break;
      case GAVF_EXT_VF_TC_FLAGS:
        if(!gavf_io_read_uint32v(io, &format->timecode_format.flags))
          return 0;
        break;
      default:
        /* Skip */
        gavf_io_skip(io, eh.len);
        break;
        
      }
    }

  if(!format->frame_width || !format->frame_height)
    gavl_video_format_set_frame_size(format, 0, 0);
  
  return 1;
  }

int gavf_write_video_format(gavf_io_t * io, const gavl_video_format_t * format)
  {
  int ret = 0;
  int num_extensions;
  uint8_t data[MAX_EXT_SIZE_VF];
  gavl_buffer_t buf;
  gavf_io_t bufio;

  gavl_buffer_init_static(&buf, data, MAX_EXT_SIZE_VF);
  gavf_io_init_buf_write(&bufio, &buf);
  
  /* Write mandatory stuff */
  if(!gavf_io_write_uint32v(io, format->image_width) ||
     !gavf_io_write_uint32v(io, format->image_height) ||
     !gavf_io_write_int32v(io, format->framerate_mode) ||
     !gavf_io_write_uint32v(io, format->timescale))
    goto fail;
    	
  if(format->framerate_mode != GAVL_FRAMERATE_STILL)
    {
    if(!gavf_io_write_uint32v(io, format->frame_duration))
      goto fail;
    }
  
  /* Count extensions */
  num_extensions = 0;

  if(format->pixelformat != GAVL_PIXELFORMAT_NONE)
    num_extensions++;
  
  if(format->pixel_width != format->pixel_height)
    num_extensions++;
    
  if(format->interlace_mode != GAVL_INTERLACE_NONE)
    num_extensions++;

  if((format->image_width != format->frame_width) ||
     (format->frame_width != format->frame_height))
    num_extensions++;

  if(format->timecode_format.int_framerate)
    {
    num_extensions++;
    if(format->timecode_format.flags)
      num_extensions++;
    }
  
  /* Write extensions */

  if(!gavf_io_write_uint32v(io, num_extensions))
    goto fail;
    
  if(!num_extensions)
    {
    ret = 1;
    goto fail;
    }
  
  if(format->pixelformat != GAVL_PIXELFORMAT_NONE)
    {
    gavf_io_buf_reset(&bufio);
    if(!gavf_io_write_uint32v(&bufio, format->pixelformat) ||
       !gavf_extension_write(io, GAVF_EXT_VF_PIXELFORMAT,
                            buf.len, buf.buf))
      goto fail;
    }
  
  if(format->pixel_width != format->pixel_height)
    {
    gavf_io_buf_reset(&bufio);
    if(!gavf_io_write_uint32v(&bufio, format->pixel_width) ||
       !gavf_io_write_uint32v(&bufio, format->pixel_height) ||
       !gavf_extension_write(io, GAVF_EXT_VF_PIXEL_ASPECT,
                            buf.len, buf.buf))
      goto fail;
    }
  
  if(format->interlace_mode != GAVL_INTERLACE_NONE)
    {
    gavf_io_buf_reset(&bufio);
    if(!gavf_io_write_int32v(&bufio, format->interlace_mode) ||
       !gavf_extension_write(io, GAVF_EXT_VF_INTERLACE,
                             buf.len, buf.buf))
      goto fail;

    }

  if((format->image_width != format->frame_width) ||
     (format->frame_width != format->frame_height))
    {
    gavf_io_buf_reset(&bufio);
    if(!gavf_io_write_uint32v(&bufio, format->frame_width) ||
       !gavf_io_write_uint32v(&bufio, format->frame_height) ||
       !gavf_extension_write(io, GAVF_EXT_VF_FRAME_SIZE,
                             buf.len, buf.buf))
      goto fail;

    }

  if(format->timecode_format.int_framerate)
    {
    gavf_io_buf_reset(&bufio);
    if(!gavf_io_write_uint32v(&bufio, format->timecode_format.int_framerate) ||
       !gavf_extension_write(io, GAVF_EXT_VF_TC_RATE,
                             buf.len, buf.buf))
      goto fail;

    
    if(format->timecode_format.flags)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_uint32v(&bufio, format->timecode_format.flags) ||
         !gavf_extension_write(io, GAVF_EXT_VF_TC_FLAGS,
                               buf.len, buf.buf))
        goto fail;
      }
    }

  ret = 1;
  fail:
  
  gavf_io_cleanup(&bufio);
  
  return ret;
  }

/* Packet */


int gavf_write_gavl_packet(gavf_io_t * io,
                           const gavf_stream_t * s,
                           const gavl_packet_t * p)
  {
  uint32_t num_extensions;

  uint8_t data[MAX_EXT_SIZE_PK];
  gavl_buffer_t buf;
  gavf_io_t bufio;
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
  
  if(!gavf_io_write_data(io, (uint8_t*)GAVF_TAG_PACKET_HEADER, 1))
    goto fail;
  
  if(!gavf_io_write_int32v(io, p->id))
    goto fail;

  /* PTS */
  if(!gavf_io_write_int64v(io, p->pts))
    goto fail;
  
  if(!gavf_io_write_uint32v(io, flags))
    goto fail;
  
  /* Write Extensions */

  if(num_extensions)
    {
    if(!gavf_io_write_uint32v(io, num_extensions))
      goto fail;
    
    gavl_buffer_init_static(&buf, data, MAX_EXT_SIZE_AF);
    gavf_io_init_buf_write(&bufio, &buf);
    
    if(write_duration)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_int64v(&bufio, p->duration) ||
         !gavf_extension_write(io, GAVF_EXT_PK_DURATION,
                               buf.len, buf.buf))
        goto fail;
      }
    
    /* Field2 offset */
    if(p->field2_offset)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_uint32v(&bufio, p->field2_offset) ||
         !gavf_extension_write(io, GAVF_EXT_PK_FIELD2,
                               buf.len, buf.buf))
        goto fail;
      }
    
    /* Interlace mode */
    if(p->interlace_mode != GAVL_INTERLACE_NONE)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_uint32v(&bufio, p->interlace_mode) ||
         !gavf_extension_write(io, GAVF_EXT_PK_INTERLACE,
                               buf.len, buf.buf))
        goto fail;
      }
    
    if(p->header_size)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_uint32v(&bufio, p->header_size) ||
         !gavf_extension_write(io, GAVF_EXT_PK_HEADER_SIZE,
                               buf.len, buf.buf))
        goto fail;
      }
    if(p->sequence_end_pos)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_uint32v(&bufio, p->sequence_end_pos) ||
         !gavf_extension_write(io, GAVF_EXT_PK_SEQ_END,
                               buf.len, buf.buf))
        goto fail;
      }

    if(p->timecode != GAVL_TIMECODE_UNDEFINED)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_uint64f(&bufio, p->timecode) ||
         !gavf_extension_write(io, GAVF_EXT_PK_TIMECODE,
                               buf.len, buf.buf))
        goto fail;
      }

    if(p->src_rect.w && p->src_rect.h)
      {
      gavf_io_buf_reset(&bufio);
      
      if(!gavf_io_write_int32v(&bufio, p->src_rect.x) ||
         !gavf_io_write_int32v(&bufio, p->src_rect.y) ||
         !gavf_io_write_int32v(&bufio, p->src_rect.w) ||
         !gavf_io_write_int32v(&bufio, p->src_rect.h) ||
         !gavf_extension_write(io, GAVF_EXT_PK_SRC_RECT,
                               buf.len, buf.buf))
        goto fail;
      }

    if(p->dst_x || p->dst_y)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_int32v(&bufio, p->dst_x) ||
         !gavf_io_write_int32v(&bufio, p->dst_y) ||
         !gavf_extension_write(io, GAVF_EXT_PK_DST_COORDS,
                               buf.len, buf.buf))
        goto fail;
      }

    if(p->num_fds)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_uint32v(&bufio, p->num_fds) ||
         !gavf_extension_write(io, GAVF_EXT_PK_FDS,
                               buf.len, buf.buf))
        goto fail;
      }

    if(p->buf_idx >= 0)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_uint32v(&bufio, p->buf_idx) ||
         !gavf_extension_write(io, GAVF_EXT_PK_BUF_IDX,
                               buf.len, buf.buf))
        goto fail;
      }
    
    }

  if(!gavf_io_write_uint32v(io, p->buf.len))
    goto fail;

  if(p->buf.len && (gavf_io_write_data(io, p->buf.buf, p->buf.len) < p->buf.len))
    goto fail;

  if(p->num_fds)    
    {
    int sock;

    if(!(gavf_io_get_flags(io) & GAVF_IO_IS_UNIX_SOCKET))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Passing FDs works only with UNIX-Sockets");
      return 0;
      }
    
    sock = gavf_io_get_socket(io);
    
    if(!gavl_socket_send_fds(sock, p->fds, p->num_fds))
      goto fail;
    }
    
  return 1;

  fail:
  return 0;
  }


int gavf_read_gavl_packet_header(gavf_io_t * io,
                                 gavl_packet_t * p)
  {
  int ret = 0;
  uint8_t c;
  
  gavl_packet_reset(p);

  /* P */
  if(!gavf_io_get_data(io, &c, 1) || (c != GAVF_TAG_PACKET_HEADER_C))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet marker");
    goto fail;
    }
  gavf_io_skip(io, 1);
  
  /* Stream ID */
  if(!gavf_io_read_int32v(io, &p->id))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read stream ID");
    goto fail;
    }
  /* PTS */
  if(!gavf_io_read_int64v(io, &p->pts))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet pts");
    goto fail;
    }
  
  /* Flags */
  if(!gavf_io_read_uint32v(io, (uint32_t*)&p->flags))
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
    if(!gavf_io_read_uint32v(io, &num_extensions))
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
          if(!gavf_io_read_int64v(io, &p->duration))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet duration");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_HEADER_SIZE:
          if(!gavf_io_read_uint32v(io, &p->header_size))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet header_size");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_SEQ_END:
          if(!gavf_io_read_uint32v(io, &p->sequence_end_pos))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet sequence_end_pos");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_TIMECODE:
          if(!gavf_io_read_uint64f(io, &p->timecode))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet timecode");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_SRC_RECT:
          if(!gavf_io_read_int32v(io, &p->src_rect.x) ||
             !gavf_io_read_int32v(io, &p->src_rect.y) ||
             !gavf_io_read_int32v(io, &p->src_rect.w) ||
             !gavf_io_read_int32v(io, &p->src_rect.h))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet src_rect");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_DST_COORDS:
          if(!gavf_io_read_int32v(io, &p->dst_x) ||
             !gavf_io_read_int32v(io, &p->dst_y))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet dst_coords");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_FIELD2:
          if(!gavf_io_read_uint32v(io, &p->field2_offset))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet field2_offset");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_INTERLACE:
          if(!gavf_io_read_uint32v(io, (uint32_t*)&p->interlace_mode))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet interlace_mode");
            goto fail;
            }
            break;
        case GAVF_EXT_PK_FDS:
          if(!gavf_io_read_uint32v(io, (uint32_t*)&p->num_fds))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet num_fds");
            goto fail;
            }
          break;
        case GAVF_EXT_PK_BUF_IDX:
          if(!gavf_io_read_uint32v(io, (uint32_t*)&p->buf_idx))
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet buf_idx");
            goto fail;
            }
          break;
        default:
          /* Skip */
          gavf_io_skip(io, eh.len);
          break;
        }
      }
    }

  if(!gavf_io_read_uint32v(io, (uint32_t*)&p->buf.len))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Could not read packet data_len");
    goto fail;
    }
  p->flags &= ~GAVL_PACKET_EXT;

  ret = 1;
  
  fail:
  return ret;
  }

int gavf_read_gavl_packet(gavf_io_t * io,
                          gavl_packet_t * p)
  {
  /* Payload */
  gavl_packet_alloc(p, p->buf.len);
  if(gavf_io_read_data(io, p->buf.buf, p->buf.len) < p->buf.len)
    goto fail;

  if(p->num_fds)
    {
    int sock;

    if(!(gavf_io_get_flags(io) & GAVF_IO_IS_UNIX_SOCKET))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Passing FDs works only with UNIX-Sockets");
      return 0;
      }
    
    sock = gavf_io_get_socket(io);

    if(!gavl_socket_recv_fds(sock, p->fds, p->num_fds))
      goto fail;
    
    }
  
  return 1;
  
  fail:

  //  if(s->g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
  //    fprintf(stderr, "Got EOF while reading packet\n");
  return 0;
  }

int gavf_skip_gavl_packet(gavf_io_t * io,
                          gavl_packet_t * p)
  {
  int i;
  gavf_io_skip(io, p->buf.len);

  if(p->num_fds)
    {
    int sock;

    if(!(gavf_io_get_flags(io) & GAVF_IO_IS_UNIX_SOCKET))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Passing FDs works only with UNIX-Sockets");
      return 0;
      }
    
    sock = gavf_io_get_socket(io);

    if(!gavl_socket_recv_fds(sock, p->fds, p->num_fds))
      return 0;

    for(i = 0; i < p->num_fds; i++)
      {
      close(p->fds[i]);
      p->fds[i] = -1;
      }
    }

  return 1;
  
  }

/* From / To Buffer */

int gavl_audio_format_from_buffer(const uint8_t * buf, int len, gavl_audio_format_t * fmt)
  {
  int result;
  gavf_io_t * io = gavf_io_create_mem_read(buf, len);
  result = gavf_read_audio_format(io, fmt);
  gavf_io_destroy(io);
  return result;
  }
  
uint8_t * gavl_audio_format_to_buffer(int * len, const gavl_audio_format_t * fmt)
  {
  uint8_t * ret;
  gavf_io_t * io = gavf_io_create_mem_write();
  gavf_write_audio_format(io, fmt);
  ret = gavf_io_mem_get_buf(io, len);
  gavf_io_destroy(io);
  return ret;
  }

int gavl_video_format_from_buffer(const uint8_t * buf, int len, gavl_video_format_t * fmt)
  {
  int result;
  gavf_io_t * io = gavf_io_create_mem_read(buf, len);
  result = gavf_read_video_format(io, fmt);
  gavf_io_destroy(io);
  return result;
  }
  
uint8_t * gavl_video_format_to_buffer(int * len, const gavl_video_format_t * fmt)
  {
  uint8_t * ret;
  gavf_io_t * io = gavf_io_create_mem_write();
  gavf_write_video_format(io, fmt);
  ret = gavf_io_mem_get_buf(io, len);
  gavf_io_destroy(io);
  return ret;
  }

int gavl_dictionary_from_buffer(const uint8_t * buf, int len, gavl_dictionary_t * fmt)
  {
  int result;
  gavf_io_t * io = gavf_io_create_mem_read(buf, len);
  result = gavl_dictionary_read(io, fmt);
  gavf_io_destroy(io);
  return result;
  }
  
uint8_t * gavl_dictionary_to_buffer(int * len, const gavl_dictionary_t * fmt)
  {
  uint8_t * ret;
  gavf_io_t * io = gavf_io_create_mem_write();
  gavl_dictionary_write(io, fmt);
  ret = gavf_io_mem_get_buf(io, len);
  gavf_io_destroy(io);
  return ret;
  }

#if 0
int gavl_compression_info_from_buffer(const uint8_t * buf, int len, gavl_compression_info_t * fmt)
  {
  int result;
  gavf_io_t * io = gavf_io_create_mem_read(buf, len);
  result = gavf_read_compression_info(io, fmt);
  gavf_io_destroy(io);
  return result;
  }
  
uint8_t * gavl_compression_info_to_buffer(int * len, const gavl_compression_info_t * fmt)
  {
  uint8_t * ret;
  gavf_io_t * io = gavf_io_create_mem_write();
  gavf_write_compression_info(io, fmt);
  ret = gavf_io_mem_get_buf(io, len);
  gavf_io_destroy(io);
  return ret;
  }
#endif
