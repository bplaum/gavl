#include <stdlib.h>
#include <string.h>


#include <gavfprivate.h>

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

int gavf_read_gavl_packet(gavf_io_t * io,
                          int default_duration,
                          int packet_flags,
                          int64_t last_sync_pts,
                          int64_t * next_pts,
                          int64_t pts_offset,
                          gavl_packet_t * p)
  {
  uint64_t start_pos;
  uint32_t len;
  int result;
  gavl_msg_t msg;

  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_READ_PACKET_START, GAVL_MSG_NS_GAVF);
  result = gavl_msg_send(&msg, io->msg_callback, io->msg_data);
  gavl_msg_free(&msg);

  if(!result)
    goto fail;
  
  if(!gavf_io_read_uint32v(io, &len))
    goto fail;
  
  start_pos = io->position;

  gavl_packet_reset(p);

  
  /* Flags */
  if(!gavf_io_read_uint32v(io, (uint32_t*)&p->flags))
    goto fail;
  
  /* PTS */
  if(packet_flags & GAVF_PACKET_WRITE_PTS)
    {
    if(!gavf_io_read_int64v(io, &p->pts))
      goto fail;
    p->pts += last_sync_pts;
    }
  else if(next_pts)
    p->pts = *next_pts;
  else
    p->pts = GAVL_TIME_UNDEFINED;
  
  /* Duration */
  if(packet_flags & GAVF_PACKET_WRITE_DURATION)
    {
    if(!gavf_io_read_int64v(io, &p->duration))
      goto fail;
    }
  
  /* Field2 */
  if(packet_flags & GAVF_PACKET_WRITE_FIELD2)
    {
    if(!gavf_io_read_uint32v(io, &p->field2_offset))
      goto fail;
    }

  /* Interlace mode */
  if(packet_flags & GAVF_PACKET_WRITE_INTERLACE)
    {
    if(!gavf_io_read_uint32v(io, (uint32_t*)&p->interlace_mode))
      goto fail;
    }
 
  if(p->flags & GAVL_PACKET_EXT)
    {
    uint32_t num_extensions;
    int i;
    gavl_extension_header_t eh;
    
    /* Extensions */
    if(!gavf_io_read_uint32v(io, &num_extensions))
      goto fail;

    for(i = 0; i < num_extensions; i++)
      {
      if(!gavf_extension_header_read(io, &eh))
        goto fail;
      switch(eh.key)
        {
        case GAVF_EXT_PK_DURATION:
          if(!gavf_io_read_int64v(io, &p->duration))
            goto fail;
          break;
        case GAVF_EXT_PK_HEADER_SIZE:
          if(!gavf_io_read_uint32v(io, &p->header_size))
            goto fail;
          break;
        case GAVF_EXT_PK_SEQ_END:
          if(!gavf_io_read_uint32v(io, &p->sequence_end_pos))
            goto fail;
          break;
        case GAVF_EXT_PK_TIMECODE:
          if(!gavf_io_read_uint64f(io, &p->timecode))
            goto fail;
          break;
        case GAVF_EXT_PK_SRC_RECT:
          if(!gavf_io_read_int32v(io, &p->src_rect.x) ||
             !gavf_io_read_int32v(io, &p->src_rect.y) ||
             !gavf_io_read_int32v(io, &p->src_rect.w) ||
             !gavf_io_read_int32v(io, &p->src_rect.h))
            goto fail;
          break;
        case GAVF_EXT_PK_DST_COORDS:
          if(!gavf_io_read_int32v(io, &p->dst_x) ||
             !gavf_io_read_int32v(io, &p->dst_y))
            goto fail;
          break;
        default:
          /* Skip */
          gavf_io_skip(io, eh.len);
          break;
        }
      }
    }
  
  p->flags &= ~GAVL_PACKET_EXT;
  
  /* Payload */
  p->data_len = len - (io->position - start_pos);
  gavl_packet_alloc(p, p->data_len);
  if(gavf_io_read_data(io, p->data, p->data_len) < p->data_len)
    goto fail;

  /* Duration */
  if(!p->duration)
    {
    if(default_duration)
      p->duration = default_duration;
    }
  /* p->duration can be 0 for the first packet in a vorbis stream */
  
  /* Set pts */
  if(next_pts)
    *next_pts += p->duration;
  
  /* Add offset */
  p->pts += pts_offset;

  gavl_msg_init(&msg);
  gavl_msg_set_id_ns(&msg, GAVL_MSG_GAVF_READ_PACKET_END, GAVL_MSG_NS_GAVF);
  result = gavl_msg_send(&msg, io->msg_callback, io->msg_data);
  gavl_msg_free(&msg);
  
  if(!result)
    goto fail;
  
  return 1;
  
  fail:

  //  if(s->g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
  //    fprintf(stderr, "Got EOF while reading packet\n");
  return 0;
  }

int gavf_write_gavl_packet_header(gavf_io_t * io,
                                  int default_duration,
                                  int packet_flags,
                                  int64_t last_sync_pts,
                                  const gavl_packet_t * p)
  {
  uint32_t num_extensions;

  uint8_t data[MAX_EXT_SIZE_PK];
  gavl_buffer_t buf;
  gavf_io_t bufio;

  uint32_t flags;
  
  /* Count extensions */
  num_extensions = 0;
  
  if(default_duration && (p->duration < default_duration))
    num_extensions++;

  if(p->header_size)
    num_extensions++;

  if(p->sequence_end_pos)
    num_extensions++;

  if(p->timecode != GAVL_TIMECODE_UNDEFINED)
    num_extensions++;

  if(p->src_rect.w && p->src_rect.h)
    num_extensions++;

  if(p->dst_x || p->dst_y)
    num_extensions++;
  
  /* Flags */

  flags = p->flags;
  if(num_extensions)
    flags |= GAVL_PACKET_EXT;
  if(!gavf_io_write_uint32v(io, flags))
    return 0;
    
  /* PTS */
  if(packet_flags & GAVF_PACKET_WRITE_PTS)
    {
    if(p->pts == GAVL_TIME_UNDEFINED)
      {
      if(!gavf_io_write_int64v(io, p->pts))
        return 0;
      }
    else
      {
      if(!gavf_io_write_int64v(io, p->pts - last_sync_pts))
        return 0;
      }
    }
  
  /* Duration */
  if(packet_flags & GAVF_PACKET_WRITE_DURATION)
    {
    if(!gavf_io_write_int64v(io, p->duration))
      return 0;
    }

  /* Field2 */
  if(packet_flags & GAVF_PACKET_WRITE_FIELD2)
    {
    if(!gavf_io_write_uint32v(io, p->field2_offset))
      return 0;
    }

  /* Interlace mode */
  if(packet_flags & GAVF_PACKET_WRITE_INTERLACE)
    {
    if(!gavf_io_write_uint32v(io, p->interlace_mode))
      return 0;
    }
  
  /* Write Extensions */

  if(num_extensions)
    {
    if(!gavf_io_write_uint32v(io, num_extensions))
      return 0;
    
    gavl_buffer_init_static(&buf, data, MAX_EXT_SIZE_AF);
    gavf_io_init_buf_write(&bufio, &buf);
    
    if(default_duration && (p->duration < default_duration))
      {
      gavf_io_buf_reset(&bufio);
      buf.len = 0;
      if(!gavf_io_write_int64v(&bufio, p->duration) ||
         !gavf_extension_write(io, GAVF_EXT_PK_DURATION,
                               buf.len, buf.buf))
        return 0;
      }

    if(p->header_size)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_uint32v(&bufio, p->header_size) ||
         !gavf_extension_write(io, GAVF_EXT_PK_HEADER_SIZE,
                               buf.len, buf.buf))
        return 0;
      }
    if(p->sequence_end_pos)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_uint32v(&bufio, p->sequence_end_pos) ||
         !gavf_extension_write(io, GAVF_EXT_PK_SEQ_END,
                               buf.len, buf.buf))
        return 0;
      }

    if(p->timecode != GAVL_TIMECODE_UNDEFINED)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_uint64f(&bufio, p->timecode) ||
         !gavf_extension_write(io, GAVF_EXT_PK_TIMECODE,
                               buf.len, buf.buf))
        return 0;
      
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
        return 0;
      }

    if(p->dst_x || p->dst_y)
      {
      gavf_io_buf_reset(&bufio);
      if(!gavf_io_write_int32v(&bufio, p->dst_x) ||
         !gavf_io_write_int32v(&bufio, p->dst_y) ||
         !gavf_extension_write(io, GAVF_EXT_PK_DST_COORDS,
                               buf.len, buf.buf))
        return 0;
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

int gavl_metadata_from_buffer(const uint8_t * buf, int len, gavl_dictionary_t * fmt)
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
