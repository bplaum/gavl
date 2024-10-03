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



#include <config.h>

#include <sys/ioctl.h>
#include <errno.h>
// #include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <glob.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>

#include <poll.h>

#include <gavl/gavl.h>
#include <gavl/compression.h>
#include <gavl/metatags.h>
#include <gavl/trackinfo.h>

#include <gavl/log.h>
#define LOG_DOMAIN "v4l2"

#include <gavl/utils.h>

#include <gavl/hw_v4l2.h>
#include <gavl/hw_dmabuf.h>

#include <hw_private.h>

#ifdef HAVE_DRM_DRM_FOURCC_H 
#define HAVE_DRM
#else // !HAVE_DRM_DRM_FOURCC_H 

#ifdef HAVE_LIBDRM_DRM_FOURCC_H 
#define HAVE_DRM
#endif
#endif // !HAVE_DRM_DRM_FOURCC_H 

// #define MAX_BUFFERS 16 // From libavcodec

// Low-Bitrate streams from ZDF need 32 packets to get the format!

#define MAX_BUFFERS 16  
#define DECODER_NUM_PACKETS 16
#define DECODER_NUM_FRAMES  16

#define MAX_FORMAT_FRAMES 16

// #define DUMP_PACKETS
// #define DUMP_EXTRADATA

#define POLL_TIMEOUT 2000

#define DECODER_HAVE_FORMAT (1<<0)
#define DECODER_SENT_EOS    (1<<1)
#define DECODER_GOT_EOS     (1<<2)

#define DECODER_HAVE_FRAME  (1<<3)

#define GAVL_V4L2_BUFFER_FLAG_QUEUED (1<<0)
#define GAVL_V4L2_BUFFER_FLAG_VALID  (1<<1)

static gavl_v4l2_device_t * gavl_v4l2_device_open(const gavl_dictionary_t * dev);

static void format_gavl_to_v4l2(const gavl_video_format_t * gavl,
                                struct v4l2_format * v4l, int type);

static void format_v4l2_to_gavl(const struct v4l2_format * v4l,
                                const struct v4l2_streamparm * parm,
                                gavl_video_format_t * gavl, int type);
  
static int my_ioctl(int fd, int request, void * arg)
  {
  int r;
  
  do{
    r = ioctl (fd, request, arg);
  } while (-1 == r && EINTR == errno);
  
  return r;
  }

static struct
  {
  int cap_flag;
  const char * name;
  }
capabilities[] =
  {

   { V4L2_CAP_VIDEO_CAPTURE, "Capture" },                   // Is a video capture device
   { V4L2_CAP_VIDEO_OUTPUT,  "Output" },                    // Is a video output device
   { V4L2_CAP_VIDEO_OVERLAY, "Overlay" },                   // Can do video overlay
   { V4L2_CAP_VBI_CAPTURE,   "VBI Capture" },               // Is a raw VBI capture device
   { V4L2_CAP_VBI_OUTPUT,    "VBO Output" },                // Is a raw VBI output device
   { V4L2_CAP_SLICED_VBI_CAPTURE, "Sliced VBI Capture" },   // Is a sliced VBI capture devic
   { V4L2_CAP_SLICED_VBI_OUTPUT,  "Sliced VBI Output" },    // Is a sliced VBI output device
   { V4L2_CAP_RDS_CAPTURE, "RDS Capture"  },                // RDS data capture
   { V4L2_CAP_VIDEO_OUTPUT_OVERLAY, "Output Overlay" },     // Can do video output overlay
   { V4L2_CAP_HW_FREQ_SEEK, "Freq Seek" },                  // Can do hardware frequency seek
   { V4L2_CAP_RDS_OUTPUT, "RDS Output" },                   // Is an RDS encoder

   /* Is a video capture device that supports multiplanar formats */
   { V4L2_CAP_VIDEO_CAPTURE_MPLANE, "Capture multiplane" },  
   /* Is a video output device that supports multiplanar formats */
   { V4L2_CAP_VIDEO_OUTPUT_MPLANE, "Output multiplane" },
   /* Is a video mem-to-mem device that supports multiplanar formats */
   { V4L2_CAP_VIDEO_M2M_MPLANE, "M2M multiplane" },
   /* Is a video mem-to-mem device */
   { V4L2_CAP_VIDEO_M2M, "M2M" },

   { V4L2_CAP_TUNER, "Tuner" },                             // has a tuner
   { V4L2_CAP_AUDIO, "Audio" },                             // has audio support
   { V4L2_CAP_RADIO, "Radio" },                             // is a radio device
   { V4L2_CAP_MODULATOR, "Modulator" },                     // has a modulator

   { V4L2_CAP_SDR_CAPTURE, "SDR Capture" },                 // Is a SDR capture device
   { V4L2_CAP_EXT_PIX_FORMAT, "Extended Pixelformat" },     // Supports the extended pixel format
   { V4L2_CAP_SDR_OUTPUT, "SDR Output" },                   // Is a SDR output device
   { V4L2_CAP_META_CAPTURE, "Metadata Capture" },           // Is a metadata capture device

   { V4L2_CAP_READWRITE, "read/write" },                    // read/write systemcalls
   { V4L2_CAP_ASYNCIO, "Async I/O" },                       // async I/O
   { V4L2_CAP_STREAMING, "Streaming" },                     // streaming I/O ioctls

#ifdef V4L2_CAP_META_OUTPUT
   { V4L2_CAP_META_OUTPUT, "metadata output" },             // Is a metadata output device
#endif
   
   { V4L2_CAP_TOUCH, "Touch" },                             // Is a touch device

   { V4L2_CAP_DEVICE_CAPS, "Device caps" },                 // sets device capabilities field
   { /* End */ },
   
  };

static void enum_formats(int fd, int type)
  {
  int idx = 0;
  struct v4l2_fmtdesc fmt;

  while(1)
    {
    fmt.index = idx++;
    fmt.type = type;

    if(my_ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == -1)
      {
      return;
      }
    
    gavl_dprintf("  Format %d: %c%c%c%c %s\n", idx,
                 fmt.pixelformat & 0xff,
                 (fmt.pixelformat>>8) & 0xff,
                 (fmt.pixelformat>>16) & 0xff,
                 (fmt.pixelformat>>24) & 0xff,
                 fmt.description);
    }
  }

void gavl_v4l2_device_info(const char * dev)
  {
  char * flag_str = NULL;
  int fd;
  int idx;
  
  struct v4l2_capability cap;

  if((fd = open(dev, O_RDWR /* required */ | O_NONBLOCK, 0)) < 0)
    {
    return;
    }

  if(my_ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
    
    }

  gavl_dprintf("Device:       %s\n", dev);
  gavl_dprintf("Driver:       %s\n", cap.driver);
  gavl_dprintf("Card:         %s\n", cap.card);
  gavl_dprintf("Bus info:     %s\n", cap.bus_info);
  gavl_dprintf("Version:      %d\n", cap.version);
  
  idx = 0;
  while(capabilities[idx].cap_flag)
    {
    if(capabilities[idx].cap_flag & cap.capabilities)
      {
      if(flag_str)
        flag_str = gavl_strcat(flag_str, ", ");
      flag_str = gavl_strcat(flag_str, capabilities[idx].name);
      }
    idx++;
    }

  gavl_dprintf("Capabilities: %s\n", flag_str);

  if(flag_str)
    free(flag_str);


  if(cap.capabilities & (V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_VIDEO_M2M))
    {
    gavl_dprintf("Output formats\n");
    enum_formats(fd, V4L2_BUF_TYPE_VIDEO_OUTPUT);
    }
  
  if(cap.capabilities & (V4L2_CAP_VIDEO_OUTPUT_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE))
    {
    gavl_dprintf("Output formats (planar)\n");
    enum_formats(fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
    }

  if(cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_M2M))
    {
    gavl_dprintf("Capture formats\n");
    enum_formats(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE);
    }
  
  if(cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE))
    {
    gavl_dprintf("Capture formats (planar)\n");
    enum_formats(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    }
  
  close(fd);

  gavl_dprintf("\n");
  
  }

void gavl_v4l2_device_infos()
  {
  int i;
  char * dev;

  for(i = 0; i < 128; i++)
    {
    dev = gavl_sprintf("/dev/video%d", i);
    gavl_v4l2_device_info(dev);
    free(dev);
    }
  }

static void query_formats(int fd, int buf_type, gavl_array_t * ret)
  {
  int idx = 0;
  struct v4l2_fmtdesc fmt;

  gavl_value_t val;
  gavl_dictionary_t * dict;
  
  while(1)
    {
    memset(&fmt, 0, sizeof(fmt));
    
    fmt.index = idx++;
    fmt.type = buf_type;
  
    if(my_ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == -1)
      break;

    gavl_value_init(&val);
    dict = gavl_value_set_dictionary(&val);

    gavl_dictionary_set_int(dict, GAVL_V4L2_FORMAT_V4L_PIX_FMT, fmt.pixelformat);
    gavl_dictionary_set_int(dict, GAVL_V4L2_FORMAT_V4L_FLAGS, fmt.flags);

    gavl_dictionary_set_string(dict, GAVL_META_LABEL, (char*)fmt.description);

    if(fmt.flags & V4L2_FMT_FLAG_COMPRESSED)
      gavl_dictionary_set_int(dict, GAVL_V4L2_FORMAT_GAVL_CODEC_ID,
                              gavl_v4l2_pix_fmt_to_codec_id(fmt.pixelformat));
    else
      gavl_dictionary_set_int(dict, GAVL_V4L2_FORMAT_GAVL_PIXELFORMAT,
                              gavl_v4l2_pix_fmt_to_pixelformat(fmt.pixelformat, V4L2_QUANTIZATION_DEFAULT));
    
    gavl_array_splice_val_nocopy(ret, -1, 0, &val);
    }
  
  }

#if 0
static void query_framesizes(int fd, int buf_type, gavl_array_t * ret)
  {
  
  }
#endif

static const struct
  {
  gavl_v4l2_device_type_t type;
  const char * name;
  }
device_types[] =
  {
    { GAVL_V4L2_DEVICE_SOURCE,  "Source"  },
    { GAVL_V4L2_DEVICE_SINK,    "Sink"    },
    { GAVL_V4L2_DEVICE_ENCODER, "Encoder" },
    { GAVL_V4L2_DEVICE_DECODER, "Decoder" },
    { GAVL_V4L2_DEVICE_CONVERTER, "Converter" },
    { GAVL_V4L2_DEVICE_UNKNOWN, "Unknown" },
    { },
  };

static const char * get_type_label(gavl_v4l2_device_type_t type)
  {
  int idx = 0;
  
  while(device_types[idx].name)
    {
    if(type == device_types[idx].type)
      return device_types[idx].name;
    idx++;
    }
  return NULL;
  }

static int formats_compressed(const gavl_array_t * arr)
  {
  const gavl_dictionary_t * fmt;
  int flags;
  
  /* Just checking the first format should be sufficient */

  if(arr->num_entries < 1)
    return 0;
  
  if((fmt = gavl_value_get_dictionary(&arr->entries[0])) &&
     gavl_dictionary_get_int(fmt, GAVL_V4L2_FORMAT_V4L_FLAGS, &flags) &&
     (flags & V4L2_FMT_FLAG_COMPRESSED))
    return 1;
  else
    return 0;
  }

typedef struct
  {
  int num_bufs;
  gavl_v4l2_buffer_t bufs[MAX_BUFFERS]; // Output
  gavl_v4l2_buffer_t * current_buf;
  gavl_video_frame_t * vframe;
  
  gavl_packet_t packet;

  int buf_type;
  struct v4l2_format fmt;
  struct v4l2_streamparm parm;
  
  gavl_video_format_t format;
  gavl_compression_info_t ci;
  
  } port_t; // Output or capture

struct gavl_v4l2_device_s
  {
  port_t output;
  port_t capture;
  
  gavl_dictionary_t dev;
  int fd;

  int64_t pts;
  gavl_timer_t * timer;
  
  int decoder_delay; // packets sent - frames read
  
  gavl_packet_t packet;

  int timescale;

  gavl_packet_source_t * psrc_src;
  
  gavl_packet_source_t * psrc_priv;
  gavl_video_source_t * vsrc_priv;
  gavl_video_sink_t * vsink_priv;
  
  //  gavl_packet_source_t * psrc_priv;
  
  gavl_packet_pts_cache_t * cache;
  
  int flags;

  /* hwctx we belong to */
  gavl_hw_context_t * hwctx;

  gavl_dictionary_t * s; // Stream (not owned by us)
  
#ifdef HAVE_DRM
  /* For importig DMA buffers */
  gavl_hw_context_t * hwctx_dmabuf;
#endif // HAVE_DRM
  
  };

#define IS_PLANAR(buf_type) \
  ((buf_type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) || (buf_type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE))

#define IS_CAPTURE(buf_type)                                             \
  ((buf_type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) || (buf_type == V4L2_BUF_TYPE_VIDEO_CAPTURE))

#define IS_OUTPUT(buf_type)                                            \
  ((buf_type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) || (buf_type == V4L2_BUF_TYPE_VIDEO_OUTPUT))

static int dequeue_buffer(gavl_v4l2_device_t * dev, int type, int memory, int * flags, gavl_time_t * timestamp)
  {
  int i;
  struct v4l2_buffer buf;
  struct v4l2_plane planes[GAVL_MAX_PLANES];

  if(type == dev->output.buf_type)
    {
    for(i = 0; i < dev->output.num_bufs; i++)
      {
      if(!(dev->output.bufs[i].flags & GAVL_V4L2_BUFFER_FLAG_QUEUED))
        return i;
      }
    }
  
  /* Dequeue buffer */
  memset(&buf, 0, sizeof(buf));

  if(IS_PLANAR(type))
    {
    memset(planes, 0, GAVL_MAX_PLANES*sizeof(planes[0]));
    buf.m.planes = planes;
    buf.length = GAVL_MAX_PLANES;
    }
  
  buf.type = type;
  buf.memory = memory;
  
  if(my_ioctl(dev->fd, VIDIOC_DQBUF, &buf) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_DQBUF failed: %s", strerror(errno));
    return -1;
    }

  if(type == dev->capture.buf_type)
    {
    dev->capture.bufs[buf.index].bytesused = buf.bytesused;
    dev->capture.current_buf = &dev->capture.bufs[buf.index];
    }
  
  if(flags)
    *flags = buf.flags;

  if(timestamp)
    {
    *timestamp = buf.timestamp.tv_sec;
    *timestamp *= GAVL_TIME_SCALE;
    *timestamp += buf.timestamp.tv_usec;
    }
  
  return buf.index;
  }

static gavl_v4l2_buffer_t * get_buffer_output(gavl_v4l2_device_t * dev)
  {
  int idx;
  
  idx = dequeue_buffer(dev, dev->output.buf_type, V4L2_MEMORY_MMAP, NULL, NULL);
  
  dev->output.current_buf = &dev->output.bufs[idx];
  dev->output.current_buf->flags &= ~GAVL_V4L2_BUFFER_FLAG_QUEUED;
  
  return dev->output.current_buf;
  }

static int done_buffer_capture(gavl_v4l2_device_t * dev)
  {
  struct v4l2_buffer buf;
  struct v4l2_plane planes[GAVL_MAX_PLANES];
  
  if(dev->capture.current_buf)
    {
    /* Queue buffer */
    memset(&buf, 0, sizeof(buf));
    
    if(IS_PLANAR(dev->capture.buf_type))
      {
      memset(planes, 0, GAVL_MAX_PLANES*sizeof(planes[0]));
      buf.m.planes = planes;
      buf.length = dev->capture.fmt.fmt.pix_mp.num_planes;
      }
    
    buf.type = dev->capture.buf_type;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index =  dev->capture.current_buf->index;

    dev->capture.current_buf->flags |= GAVL_V4L2_BUFFER_FLAG_QUEUED;
    dev->capture.current_buf = NULL;
    
    if(my_ioctl(dev->fd, VIDIOC_QBUF, &buf) == -1)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_QBUF failed for capture (index: %d): %s",
               buf.index, strerror(errno));
      return 0;
      }
    //    fprintf(stderr, "Queueing buffer %d\n", buf.index);
    }

  return 1;
  }

static gavl_packet_t * gavl_v4l2_device_get_packet_write(gavl_v4l2_device_t * dev)
  {
  
  if(!get_buffer_output(dev))
    return NULL;
  
  dev->packet.buf.buf   = dev->output.current_buf->planes[0].buf;
  dev->packet.buf.alloc = dev->output.current_buf->planes[0].size;
  dev->packet.buf.len   = 0;
  
  return &dev->packet;
  
  }

static gavl_sink_status_t gavl_v4l2_device_put_packet_write(gavl_v4l2_device_t * dev)
  {
  
  /* Queue compressed frame */
  struct v4l2_buffer buf;
  struct v4l2_plane planes[GAVL_MAX_PLANES];

  /* Queue buffer */
  memset(&buf, 0, sizeof(buf));

#ifdef DUMP_PACKETS
  fprintf(stderr, "Sending packet\n");
  gavl_packet_dump(&dev->packet);
#endif

  //  fprintf(stderr, "Packet pts: %"PRId64"\n", dev->packet.pts);
  
  if(IS_PLANAR(dev->output.buf_type))
    {
    memset(planes, 0, GAVL_MAX_PLANES*sizeof(planes[0]));
    buf.m.planes = planes;
    buf.m.planes[0].bytesused = dev->packet.buf.len;
    buf.length = 1;
    }
  else
    {
    buf.bytesused = dev->packet.buf.len;
    }
  
  if(dev->packet.flags & GAVL_PACKET_KEYFRAME)
    buf.flags |= V4L2_BUF_FLAG_KEYFRAME;
  
  buf.type = dev->output.buf_type;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index =  dev->output.current_buf->index;

  if(dev->packet.pts < 0)
    {
    buf.timestamp.tv_sec = dev->packet.pts / 1000000 - 1;
    }
  else
    {
    buf.timestamp.tv_sec = dev->packet.pts / 1000000;
    }
  
  buf.timestamp.tv_usec = dev->packet.pts - buf.timestamp.tv_sec * 1000000;
  
  buf.field = V4L2_FIELD_NONE;
  
  if(my_ioctl(dev->fd, VIDIOC_QBUF, &buf) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_QBUF failed for output: %s", strerror(errno));
    return GAVL_SINK_ERROR;
    }

  dev->output.current_buf->flags |= GAVL_V4L2_BUFFER_FLAG_QUEUED;

  
  return GAVL_SINK_OK;
  }

static int send_decoder_packet(gavl_v4l2_device_t * dev)
  {
  gavl_source_status_t st;
  gavl_packet_t * p = gavl_v4l2_device_get_packet_write(dev);

  /* Send EOF */
  
  if(((st = gavl_packet_source_read_packet(dev->psrc_src, &p)) != GAVL_SOURCE_OK))
    {
    // EOF is no error
    // gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "send_decoder_packet failed, got no packet (source status %d)", st);
    
    if(!(dev->flags & DECODER_SENT_EOS))
      {
      /* Send EOS */
    
      struct v4l2_decoder_cmd cmd;
      memset(&cmd, 0, sizeof(cmd));
    
      cmd.cmd = V4L2_DEC_CMD_STOP;

      if(my_ioctl(dev->fd, VIDIOC_DECODER_CMD, &cmd) == -1)
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "V4L2_DEC_CMD_STOP failed %s", strerror(errno));
        }
      dev->flags |= DECODER_SENT_EOS;
      //      fprintf(stderr, "Sent EOS\n");
      return 1;
      }
    else
      {
      return 0;
      }
    }
  
  if(gavl_v4l2_device_put_packet_write(dev) != GAVL_SINK_OK)
    return 0;
  
  dev->decoder_delay++;
  gavl_packet_pts_cache_push_packet(dev->cache, p);
  return 1;
  }

static int request_buffers_mmap(gavl_v4l2_device_t * dev, int type, int count, gavl_v4l2_buffer_t * bufs)
  {
  int i, j;
  
  struct v4l2_buffer buf;
  struct v4l2_requestbuffers req;

  struct v4l2_plane planes[GAVL_MAX_PLANES];
  
  memset(&req, 0, sizeof(req));
  
  req.count = count;
  req.type = type;
  req.memory = V4L2_MEMORY_MMAP;

  if(my_ioctl(dev->fd, VIDIOC_REQBUFS, &req) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Requesting buffers failed: %s", strerror(errno));
    return 0;
    }

  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Requested %d buffers, got %d", count, req.count);

  for(i = 0; i < req.count; i++)
    {
    memset(&buf, 0, sizeof(buf));
    
    buf.index = i;
    buf.type = type;
    
    if(IS_PLANAR(type))
      {
      memset(planes, 0, GAVL_MAX_PLANES*sizeof(planes[0]));
      buf.length = GAVL_MAX_PLANES;
      buf.m.planes = planes;
      }
    
    if(my_ioctl(dev->fd, VIDIOC_QUERYBUF, &buf) == -1)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_QUERYBUF failed: %s", strerror(errno));
      return 0;
      }

#if 0    
    if(buf.flags & V4L2_BUF_FLAG_TIMESTAMP_COPY)
      {
      fprintf(stderr, "Copy timestamps\n");
      }
#endif
    
    bufs[i].index = i;
    bufs[i].type = type;
    bufs[i].total = req.count;
    
    if(IS_PLANAR(type))
      {
      if(buf.length > GAVL_MAX_PLANES)
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "%d planes not supported", buf.length);
        return 0;
        }

      bufs[i].num_planes = buf.length;
      
      for(j = 0; j < buf.length; j++)
        {
        bufs[i].planes[j].buf = mmap(NULL, buf.m.planes[j].length,
                                     PROT_READ | PROT_WRITE, MAP_SHARED,
                                     dev->fd, buf.m.planes[j].m.mem_offset);
        bufs[i].planes[j].size = buf.m.planes[j].length;
        }

      }
    else
      {
      bufs[i].planes[0].buf = mmap(NULL, buf.length,
                                   PROT_READ | PROT_WRITE, MAP_SHARED,
                                   dev->fd, buf.m.offset);
      bufs[i].planes[0].size = buf.length;
      bufs[i].num_planes = 1;
      }
    
    }
  return req.count;
  }


static void release_buffers_mmap(gavl_v4l2_device_t * dev, int type, int count, gavl_v4l2_buffer_t * bufs)
  {
  int i, j;
  struct v4l2_requestbuffers req;
  
  memset(&req, 0, sizeof(req));
  
  for(i = 0; i < count; i++)
    {
    for(j = 0; j < bufs[i].num_planes; j++)
      munmap(bufs[i].planes[j].buf, bufs[i].planes[j].size);
    }
  
  req.count = 0;
  req.type = type;
  req.memory = V4L2_MEMORY_MMAP;

  if(my_ioctl(dev->fd, VIDIOC_REQBUFS, &req) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_REQBUFS for releasing failed: %s", strerror(errno));
    } 
  memset(bufs, 0, count * sizeof(*bufs));
  }

static int request_buffers_dmabuf(gavl_v4l2_device_t * dev, int type, int count, gavl_v4l2_buffer_t * bufs)
  {
  int i;
  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof(req));
  
  req.count = count;
  req.type = type;
  req.memory = V4L2_MEMORY_DMABUF;

  if(my_ioctl(dev->fd, VIDIOC_REQBUFS, &req) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Requesting buffers failed: %s", strerror(errno));
    return 0;
    }
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Requested %d buffers, got %d", count, req.count);

  for(i = 0; i < req.count; i++)
    {
    
    }

  return 1;
  }

static void release_buffers_dmabuf(gavl_v4l2_device_t * dev, int type, int count, gavl_v4l2_buffer_t * bufs)
  {
  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof(req));
  req.count = 0;
  req.type = type;
  req.memory = V4L2_MEMORY_DMABUF;

  if(my_ioctl(dev->fd, VIDIOC_REQBUFS, &req) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_REQBUFS for releasing failed: %s", strerror(errno));
    } 
  memset(bufs, 0, count * sizeof(*bufs));
  }

gavl_v4l2_device_t * gavl_v4l2_device_open(const gavl_dictionary_t * dev)
  {
  gavl_v4l2_device_t * ret = calloc(1, sizeof(*ret));
  
  const char * path;

  gavl_dictionary_copy(&ret->dev, dev);
  
  if(!(path = gavl_dictionary_get_string(dev, GAVL_META_URI)))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "BUG: Path member missing");
    goto fail;
    }
  
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Opening %s", path);
  
  if((ret->fd = open(path, O_RDWR /* required */ | O_NONBLOCK, 0)) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Couldn't open %s: %s", path, strerror(errno));
    goto fail;
    }
  
  return ret;

  fail:

  if(ret)
    gavl_v4l2_device_close(ret);
  
  return NULL;
  }

#if 0
static void dump_fmt(gavl_v4l2_device_t * dev, const struct v4l2_format * fmt)
  {
  int i;
  fprintf(stderr, "Format\n");
  if(dev->planar)
    {
    fprintf(stderr, "  Size:        %dx%d\n", fmt->fmt.pix_mp.width, 
            fmt->fmt.pix_mp.height);
    
    fprintf(stderr, "  Pixelformat: %c%c%c%c\n",
            (fmt->fmt.pix_mp.pixelformat) & 0xff,
            (fmt->fmt.pix_mp.pixelformat >> 8) & 0xff,
            (fmt->fmt.pix_mp.pixelformat >> 16) & 0xff,
            (fmt->fmt.pix_mp.pixelformat >> 24) & 0xff);
    
    fprintf(stderr, "  Planes:      %d\n", fmt->fmt.pix_mp.num_planes);

    for(i = 0; i < fmt->fmt.pix_mp.num_planes; i++)
      {
      fprintf(stderr, "  Plane %d:\n", i);
      fprintf(stderr, "    Sizeimage:    %d\n", fmt->fmt.pix_mp.plane_fmt[i].sizeimage);
      fprintf(stderr, "    Bytesperline: %d\n", fmt->fmt.pix_mp.plane_fmt[i].bytesperline);
      }

    
    
    }
  }
#endif

static int stream_on(gavl_v4l2_device_t * dev, int type)
  {
  if(my_ioctl(dev->fd, VIDIOC_STREAMON, &type) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_STREAMON failed: %s", strerror(errno));
    return 0;
    }
  return 1;
  }

static int stream_off(gavl_v4l2_device_t * dev, int type)
  {
  if(my_ioctl(dev->fd, VIDIOC_STREAMOFF, &type) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_STREAMOFF failed: %s", strerror(errno));
    return 0;
    }
  return 1;
  }

static int queue_frame_capture(gavl_v4l2_device_t * dev, int idx)
  {
  struct v4l2_buffer buf;
  struct v4l2_plane planes[GAVL_MAX_PLANES];
  
  memset(&buf, 0, sizeof(buf));

  if(IS_PLANAR(dev->capture.buf_type))
    {
    memset(planes, 0, GAVL_MAX_PLANES*sizeof(planes[0]));
    buf.m.planes = planes;
    buf.length = GAVL_MAX_PLANES;
    }
  
  buf.type = dev->capture.buf_type;
  
  buf.index = idx;
  buf.memory = V4L2_MEMORY_MMAP;
  
  if(my_ioctl(dev->fd, VIDIOC_QBUF, &buf) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_QBUF failed: %s", strerror(errno));
    return 0;
    }
  
  dev->capture.bufs[idx].flags |= GAVL_V4L2_BUFFER_FLAG_QUEUED;
  
  return 1;
  
  }


static void handle_decoder_event(gavl_v4l2_device_t * dev)
  {
  struct v4l2_event ev;
  
  while(!my_ioctl(dev->fd, VIDIOC_DQEVENT, &ev))
    {
    //    fprintf(stderr, "Got event %d\n", ev.type);
    
    switch(ev.type)
      {
      case V4L2_EVENT_SOURCE_CHANGE:
        //        fprintf(stderr, "Source changed\n");

        if(ev.u.src_change.changes & V4L2_EVENT_SRC_CH_RESOLUTION)
          {
          int i;
          memset(&dev->capture.fmt, 0, sizeof(dev->capture.fmt));
          
          //          fprintf(stderr, "Resolution changed\n");
          
          dev->capture.fmt.type = dev->capture.buf_type;
          
          if(my_ioctl(dev->fd, VIDIOC_G_FMT, &dev->capture.fmt) == -1)
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_G_FMT failed: %s", strerror(errno));
            }
          //          dump_fmt(dev, &dev->capture.fmt);
          
          dev->capture.num_bufs = request_buffers_mmap(dev, dev->capture.buf_type,
                                                       DECODER_NUM_FRAMES, dev->capture.bufs);

          for(i = 0; i < dev->capture.num_bufs; i++)
            queue_frame_capture(dev, i);
          
          stream_on(dev, dev->capture.buf_type);
          
          //      fprintf(stderr, "Re-created buffers\n");

          dev->flags |= DECODER_HAVE_FORMAT;
          }

        break;
      case V4L2_EVENT_EOS:
        dev->flags |= DECODER_GOT_EOS;
        //       fprintf(stderr, "Got EOS\n");
        break;
      }
    }
  }

static int do_poll(gavl_v4l2_device_t * dev, int events, int * revents)
  {
  int result;
  struct pollfd fds;

  //  fprintf(stderr, "do_poll\n");
  
  fds.fd = dev->fd;
  
  
  fds.events = events;

  if(fds.events & POLLIN)
    fds.events |= POLLRDNORM;

  if(fds.events & POLLOUT)
    fds.events |= POLLWRNORM;
  
  fds.revents = 0;
  
  result = poll(&fds, 1, POLL_TIMEOUT);

  if(result == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "poll() failed: %s", strerror(errno));
    return 0;
    }

  *revents = 0;
  
  if(fds.revents & (POLLIN|POLLRDNORM))
    *revents |= POLLIN;

  if(fds.revents & (POLLOUT|POLLWRNORM))
    *revents |= POLLOUT;

  if(fds.revents & POLLPRI)
    *revents |= POLLPRI;
  
  return 1;
  }

static void buffer_to_video_frame_mmap(gavl_v4l2_device_t * dev, gavl_v4l2_buffer_t * buf,
                                       const struct v4l2_format * fmt,
                                       gavl_pixelformat_t pixelformat,
                                       int type)
  {
  gavl_video_frame_t * f;

  if((type == V4L2_BUF_TYPE_VIDEO_CAPTURE) ||
     (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE))
    f = dev->capture.vframe;
  else
    f = dev->output.vframe;

  if(pixelformat == GAVL_YUV_420_P)
    {
    uint8_t * ptr;
    int bytesperline = 0;
    int sizeimage = 0;
    int sizeimage_y;
    int sizeimage_cbcr;
    uint32_t pixfmt = 0;
    
    if(IS_PLANAR(type))
      {
      const struct v4l2_pix_format_mplane * m = &fmt->fmt.pix_mp;

      pixfmt = m->pixelformat;
      
      if(m->num_planes == 1)
        {
        sizeimage = m->plane_fmt[0].sizeimage;
        bytesperline = m->plane_fmt[0].bytesperline;
        }
      else
        {
        /* ?? */
        }
      }
    else
      {
      const struct v4l2_pix_format * p = &fmt->fmt.pix;
      pixfmt = p->pixelformat;
      sizeimage = p->sizeimage;
      bytesperline = p->bytesperline;
      }
    
    ptr = buf->planes[0].buf;
    sizeimage_y = (sizeimage * 2) / 3;
    sizeimage_cbcr = (sizeimage - sizeimage_y) / 2;
    
    f->planes[0] = ptr;

    if(pixfmt == V4L2_PIX_FMT_YUV420)
      {
      f->planes[1] = f->planes[0] + sizeimage_y;
      f->planes[2] = f->planes[1] + sizeimage_cbcr;
      }
    else
      {
      f->planes[2] = f->planes[0] + sizeimage_y;
      f->planes[1] = f->planes[2] + sizeimage_cbcr;
      }
    f->strides[0] = bytesperline;
    f->strides[1] = bytesperline / 2;
    f->strides[2] = bytesperline / 2;
    }
  else if(gavl_pixelformat_is_planar(pixelformat))
    {
    /* TODO */
    }
  else // Packed
    {
    int bytesperline = 0;
    
    if(IS_PLANAR(type))
      {
      bytesperline = fmt->fmt.pix_mp.plane_fmt[0].bytesperline;
      }
    else
      {
      bytesperline = fmt->fmt.pix.bytesperline;;
      }

    f->planes[0] = buf->planes[0].buf;
    f->strides[0] = bytesperline;
    }

  f->hwctx = dev->hwctx;
  f->storage = buf;
  f->buf_idx = buf->index;
  }


static gavl_source_status_t get_frame_decoder(void * priv, gavl_video_frame_t ** frame)
  {
  int pollev = 0;
  gavl_v4l2_device_t * dev = priv;

  int events_requested;

  
  /* Send frame back to the queue */
  
  done_buffer_capture(dev);
  
  if(dev->flags & DECODER_GOT_EOS)
    return GAVL_SOURCE_EOF;
  
  while(1)
    {
    if(dev->flags & DECODER_GOT_EOS)
      return GAVL_SOURCE_EOF;
    
    events_requested = POLLIN | POLLPRI;
    
    if(!(dev->flags & DECODER_SENT_EOS) && (dev->decoder_delay < DECODER_NUM_PACKETS))
      events_requested |= POLLOUT;
    
    do_poll(dev, events_requested, &pollev);

#if 0
    fprintf(stderr, "Do poll decode: %d %d %d %d\n",
            !!(pollev & POLLIN), !!(pollev & POLLOUT), !!(pollev & POLLPRI),
            dev->decoder_delay);
#endif
    
    if(pollev & POLLPRI)
      {
      handle_decoder_event(dev);
      }
    
    if(pollev & POLLOUT)
      {
      if(!send_decoder_packet(dev))
        return GAVL_SOURCE_EOF;
      }
    
    if(pollev & POLLIN)
      {
      int idx;
      
      idx = dequeue_buffer(dev, dev->capture.buf_type, V4L2_MEMORY_MMAP, NULL, NULL);
      
      if(idx < 0)
        return GAVL_SOURCE_EOF;

      /* Set output buffer to frame */
      
      buffer_to_video_frame_mmap(dev, dev->capture.current_buf, &dev->capture.fmt, dev->capture.format.pixelformat,
                                 dev->capture.buf_type);
      
      gavl_packet_pts_cache_get_first(dev->cache, dev->capture.vframe);
      
      //      fprintf(stderr, "Frame pts: %"PRId64"\n", dev->vframe_cap->timestamp);

      if(frame)
        *frame = dev->capture.vframe;
      
      dev->decoder_delay--;
      return GAVL_SOURCE_OK;
      }
    
    }
  return GAVL_SOURCE_EOF;
  }

#if 1
void gavl_v4l2_device_resync_decoder(gavl_v4l2_device_t * dev)
  {
  int i;
  int pollev = 0;
  
  //  fprintf(stderr, "Resync...\n");
  
  stream_off(dev, dev->capture.buf_type);
  stream_off(dev, dev->output.buf_type);

  gavl_packet_pts_cache_clear(dev->cache);

  dev->capture.current_buf = NULL;
  dev->output.current_buf = NULL;
  
  /* Queue frames */
  for(i = 0; i < dev->capture.num_bufs; i++)
    {
    queue_frame_capture(dev, i);
    }
  
  for(i = 0; i < dev->output.num_bufs; i++)
    {
    dev->output.bufs[i].flags &= ~GAVL_V4L2_BUFFER_FLAG_QUEUED;
    }

  stream_on(dev, dev->output.buf_type);
  stream_on(dev, dev->capture.buf_type);

  dev->decoder_delay = 0;
  
  for(i = 0; i < dev->output.num_bufs; i++)
    send_decoder_packet(dev);
  
  do_poll(dev, POLLIN, &pollev);

#if 0  
  fprintf(stderr, "Do poll resync: %d %d %d\n",
          !!(pollev & POLLIN), !!(pollev & POLLOUT), !!(pollev & POLLPRI));
  
  fprintf(stderr, "Resync done\n");
#endif
  }
#endif

static int64_t get_capture_pts(gavl_v4l2_device_t * dev, int flags, gavl_time_t v4l_pts)
  {
  
  switch(flags & V4L2_BUF_FLAG_TIMESTAMP_MASK)
    {
    case V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC:
    case V4L2_BUF_FLAG_TIMESTAMP_COPY:
      return v4l_pts;
      break;
    default:
      if(!dev->timer)
        {
        dev->timer = gavl_timer_create();
        gavl_timer_start(dev->timer);
        }
      return gavl_timer_get(dev->timer);
      break;
    }
  
  }

static gavl_source_status_t read_packet_capture(void * priv, gavl_packet_t ** p)
  {
  int idx;
  gavl_v4l2_device_t * dev = priv;
  int events;
  int revents;

  int flags;
  gavl_time_t pts = 0;

  if(dev->capture.current_buf && (dev->capture.current_buf->flags & GAVL_V4L2_BUFFER_FLAG_VALID))
    {
    if(p)
      {
      *p = &dev->capture.packet;
      dev->capture.current_buf->flags &= ~GAVL_V4L2_BUFFER_FLAG_VALID;
      }
    
    return GAVL_SOURCE_OK;
    }
    
  done_buffer_capture(dev);

  events = POLLIN;

  //  fprintf(stderr, "Poll...");
  
  if(!do_poll(dev, events, &revents) ||
     !(revents & POLLIN))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got no packet");
    return GAVL_SOURCE_EOF;
    }

  //  fprintf(stderr, "Poll done\n");
  
  if((idx = dequeue_buffer(dev, dev->capture.buf_type, V4L2_MEMORY_MMAP, &flags, &pts)) < 0)
    return GAVL_SOURCE_EOF;

  gavl_packet_init(&dev->capture.packet);
  
  dev->capture.packet.buf.buf = dev->capture.current_buf->planes[0].buf;
  dev->capture.packet.buf.len = dev->capture.current_buf->bytesused;

  if(flags & V4L2_BUF_FLAG_KEYFRAME)
    dev->capture.packet.flags |= GAVL_PACKET_KEYFRAME;  
  
  dev->capture.packet.pts = get_capture_pts(dev, flags, pts);
  
  //  fprintf(stderr, "Dequeued buffer %d (%d bytes)\n", idx, dev->capture.current_buf->bytesused);
  //  gavl_packet_dump(&dev->capture.packet);

  if(p)
    *p = &dev->capture.packet;
  else
    dev->capture.current_buf->flags |= GAVL_V4L2_BUFFER_FLAG_VALID;
  
  return GAVL_SOURCE_OK;
  }

static gavl_source_status_t read_frame_capture(void * priv, gavl_video_frame_t ** frame)
  {
  int idx;
  gavl_v4l2_device_t * dev = priv;
  int flags;
  gavl_time_t pts = 0;
  int events, revents = 0;
  
  if(dev->capture.current_buf && (dev->capture.current_buf->flags & GAVL_V4L2_BUFFER_FLAG_VALID))
    {
    if(frame)
      {
      *frame = dev->capture.vframe;
      dev->capture.current_buf->flags &= ~GAVL_V4L2_BUFFER_FLAG_VALID;
      }
    return GAVL_SOURCE_OK;
    }
  
  done_buffer_capture(dev);

  events = POLLIN;

  //  fprintf(stderr, "Poll...");
  
  if(!do_poll(dev, events, &revents) ||
     !(revents & POLLIN))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got no frame");
    return GAVL_SOURCE_EOF;
    }
  
  if((idx = dequeue_buffer(dev, dev->capture.buf_type, V4L2_MEMORY_MMAP, &flags, &pts) < 0))
    return GAVL_SOURCE_EOF;
  
  buffer_to_video_frame_mmap(dev, dev->capture.current_buf, &dev->capture.fmt, dev->capture.format.pixelformat,
                             dev->capture.buf_type);

  dev->capture.vframe->timestamp = get_capture_pts(dev, flags, pts);

  if(frame)
    *frame = dev->capture.vframe;
  else
    dev->capture.current_buf->flags |= GAVL_V4L2_BUFFER_FLAG_VALID;

  return GAVL_SOURCE_OK;
  }

int gavl_v4l_device_init_capture(gavl_v4l2_device_t * dev, gavl_dictionary_t * stream)
  {
  int caps = 0;
  uint32_t pixelformat;
  int ret = 0;
  int i;

  dev->s = stream;

  if(gavl_dictionary_get_int(&dev->dev, GAVL_V4L2_CAPABILITIES, &caps) &&
     (caps & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
    dev->capture.buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  else
    dev->capture.buf_type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  //    fprintf(stderr, "Blupp 5\n");
  
  dev->capture.fmt.type = dev->capture.buf_type;
  dev->capture.parm.type = dev->capture.buf_type;
  
  if(my_ioctl(dev->fd, VIDIOC_G_FMT, &dev->capture.fmt) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_G_FMT failed: %s", strerror(errno));
    goto fail;
    }

  /* TODO: Configure format */
  
  if(my_ioctl(dev->fd, VIDIOC_S_FMT, &dev->capture.fmt) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_S_FMT failed: %s", strerror(errno));
    goto fail;
    }

  if(my_ioctl(dev->fd, VIDIOC_G_PARM, &dev->capture.parm) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_G_PARM failed: %s", strerror(errno));
    goto fail;
    }
  
  if(IS_PLANAR(dev->capture.buf_type))
    pixelformat = dev->capture.fmt.fmt.pix_mp.pixelformat;
  else
    pixelformat = dev->capture.fmt.fmt.pix.pixelformat;
  
  dev->capture.ci.id = gavl_v4l2_pix_fmt_to_codec_id(pixelformat);
  
  fprintf(stderr, "Video pixelformat %c%c%c%c\n",
          (pixelformat) & 0xff,
          (pixelformat >> 8) & 0xff,
          (pixelformat >> 16) & 0xff,
          (pixelformat >> 24) & 0xff);
  
  if(dev->capture.ci.id != GAVL_CODEC_ID_NONE)
    {
    format_v4l2_to_gavl(&dev->capture.fmt,
                        &dev->capture.parm,
                        &dev->capture.format,
                        dev->capture.buf_type);

    if(dev->capture.ci.id == GAVL_CODEC_ID_EXTENDED)
      {
      /* Fourccs in v4l2 are swapped */
      dev->capture.ci.codec_tag =
        ((pixelformat & 0x000000ff) << 24) |
        ((pixelformat & 0x0000ff00) << 8) |
        ((pixelformat & 0x00ff0000) >> 8) |
        ((pixelformat & 0xff000000) >> 24);
      }
    
    gavl_stream_set_compression_info(stream, &dev->capture.ci);
    }
  else
    {
    format_v4l2_to_gavl(&dev->capture.fmt,
                        &dev->capture.parm,
                        &dev->capture.format,
                        dev->capture.buf_type);
    gavl_video_format_dump(&dev->capture.format);
    dev->capture.format.hwctx = dev->hwctx;
    }
  
  gavl_video_format_copy(gavl_stream_get_video_format_nc(stream),
                         &dev->capture.format);

  /* Request buffers */
  if(!(dev->capture.num_bufs = request_buffers_mmap(dev, dev->capture.buf_type, 2, dev->capture.bufs)))
    goto fail;

  /* Queue frames */
  for(i = 0; i < dev->capture.num_bufs; i++)
    queue_frame_capture(dev, i);
  

  if(dev->capture.ci.id != GAVL_CODEC_ID_NONE)
    {
    /* Compressed capture */
    dev->psrc_priv = gavl_packet_source_create(read_packet_capture, dev, GAVL_SOURCE_SRC_ALLOC, stream);
    }
  else
    {
    /* Uncompressed capture */
    dev->vsrc_priv = gavl_video_source_create(read_frame_capture, dev, GAVL_SOURCE_SRC_ALLOC,
                                              &dev->capture.format);
    
    }

  dev->capture.vframe = gavl_video_frame_create(NULL);
  
  ret = 1;
  fail:
  
  return ret;
  }

int gavl_v4l_device_start_capture(gavl_v4l2_device_t * dev)
  {
  gavl_stream_stats_t stats;

  gavl_stream_stats_init(&stats);
  
  if(!stream_on(dev, dev->capture.buf_type))
    return 0;
  
  /* Peek frames and set initial timestamps */
  if(dev->psrc_priv)
    {
    read_packet_capture(dev, NULL);
    stats.pts_start = dev->capture.packet.pts;
    }
  else if(dev->vsrc_priv)
    {
    if(!read_frame_capture(dev, NULL))
      return 0;
    stats.pts_start = dev->capture.vframe->timestamp;
    }
  gavl_stream_set_stats(dev->s, &stats);  
  return 1;
  }

int gavl_v4l2_device_init_decoder(gavl_v4l2_device_t * dev, gavl_dictionary_t * stream,
                                  gavl_packet_source_t * psrc)
  {
  int format_packets = 0;
  
  int caps = 0;
  int ret = 0;
  gavl_video_format_t * gavl_format;
  gavl_compression_info_t ci;
  gavl_stream_stats_t stats;
  
  int pollev = 0;
  
  int max_packet_size;
  struct v4l2_event_subscription sub;
  int packets_to_send;

  int num;
  
  struct v4l2_cropcap cropcap;

  //  fprintf(stderr, "gavl_v4l2_device_init_decoder\n");
  //  gavl_dictionary_dump(stream, 2);

  gavl_format = gavl_stream_get_video_format_nc(stream);

  gavl_format->hwctx = dev->hwctx;
  
  dev->timescale = gavl_format->timescale;

  dev->psrc_src = psrc;
  
  dev->cache = gavl_packet_pts_cache_create(2*MAX_BUFFERS);

  /* Subscribe to events */
  
  memset(&sub, 0, sizeof(sub));

  memset(&ci, 0, sizeof(ci));
  
  if(!gavl_stream_get_compression_info(stream, &ci))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got no compression info");
    goto fail;
    }

  //  if(ci.flags & GAVL_COMPRESSION_HAS_P_FRAMES)
  //    {
    sub.type = V4L2_EVENT_EOS;
    if(my_ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub) == -1)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_SUBSCRIBE_EVENT V4L2_EVENT_EOS failed: %s",
               strerror(errno));
      goto fail;
      }

    sub.type = V4L2_EVENT_SOURCE_CHANGE;
    if(my_ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub) == -1)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_SUBSCRIBE_EVENT V4L2_EVENT_SOURCE_CHANGE failed: %s",
               strerror(errno));
      goto fail;
      }
    //    }
  
  
  gavl_stream_stats_init(&stats);
  gavl_stream_get_stats(stream, &stats);

  gavl_video_format_set_frame_size(gavl_format, 16, 16);
  
  if(stats.size_max > 0)
    max_packet_size = stats.size_max + 128;
  else
    max_packet_size = (gavl_format->frame_width * gavl_format->frame_width * 3) / 4 + 128;
  
  memset(&dev->output.fmt, 0, sizeof(dev->output.fmt));

  /* */
  
  if(gavl_dictionary_get_int(&dev->dev, GAVL_V4L2_CAPABILITIES, &caps) &&
     (caps & V4L2_CAP_VIDEO_M2M_MPLANE))
    {
    dev->output.buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    dev->capture.buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    
    //    fprintf(stderr, "Using planar API\n");

    dev->output.fmt.type = dev->output.buf_type;
          
    //    fmt.fmt.pix_mp.width = gavl_format->image_width;
    //    fmt.fmt.pix_mp.height = gavl_format->image_height;
    
    dev->output.fmt.fmt.pix_mp.pixelformat = gavl_v4l2_codec_id_to_pix_fmt(ci.id);
    dev->output.fmt.fmt.pix_mp.colorspace = V4L2_COLORSPACE_DEFAULT;
    dev->output.fmt.fmt.pix_mp.field      = V4L2_FIELD_NONE;
    
    dev->output.fmt.fmt.pix_mp.num_planes = 1;

    dev->output.fmt.fmt.pix_mp.plane_fmt[0].sizeimage = max_packet_size;
    //fmt.fmt.pix_mp.plane_fmt[0].bytesperline = 0;
    
    }
  else
    {
    dev->output.buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    dev->capture.buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    /* Untested */
    dev->output.fmt.type = dev->output.buf_type;
    
    //    fmt.fmt.pix.width = gavl_format->image_width;
    //    fmt.fmt.pix.height = gavl_format->image_height;

    dev->output.fmt.fmt.pix.pixelformat = gavl_v4l2_codec_id_to_pix_fmt(ci.id);
    dev->output.fmt.fmt.pix.colorspace = V4L2_COLORSPACE_DEFAULT;
    dev->output.fmt.fmt.pix.sizeimage = max_packet_size;
    dev->output.fmt.fmt.pix.field      = V4L2_FIELD_NONE;
    }
  
  if(my_ioctl(dev->fd, VIDIOC_S_FMT, &dev->output.fmt) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_S_FMT failed: %s", strerror(errno));
    goto fail;
    }


  if(ci.flags & GAVL_COMPRESSION_HAS_P_FRAMES)
    num = DECODER_NUM_PACKETS;
  else
    num = 1;
  
  if(!(dev->output.num_bufs = request_buffers_mmap(dev, dev->output.buf_type, num, dev->output.bufs)))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Requesting buffers failed");
    goto fail;
    }

  packets_to_send = dev->output.num_bufs;
  
  /* */

  /* Queue header */
  if(!stream_on(dev, dev->output.buf_type))
    goto fail;
        
  if(ci.codec_header.len)
    {
    gavl_packet_t * p;

#ifdef DUMP_EXTRADATA
    fprintf(stderr, "Sending global header %d bytes\n", ci.codec_header.len);
    gavl_hexdump(ci.codec_header.buf, ci.codec_header.len, 16);
#endif

    p = gavl_v4l2_device_get_packet_write(dev);

    gavl_buffer_reset(&p->buf);
    gavl_buffer_append(&p->buf, &ci.codec_header);
    
    if(gavl_v4l2_device_put_packet_write(dev) != GAVL_SINK_OK)
      goto fail;

    //    gavl_time_delay(&t);
    
    packets_to_send--;
    }
  else
    {
    if(!send_decoder_packet(dev))
      fprintf(stderr, "sending decoder packet failed\n");
    }
  

#if 0  
  for(i = 0; i < packets_to_send; i++)
    {
    if(!send_decoder_packet(dev))
      fprintf(stderr, "sending decoder packet failed\n");
    }
#endif
  
  memset(&cropcap, 0, sizeof(cropcap));
  cropcap.type = dev->output.buf_type;
  
  if(my_ioctl(dev->fd, VIDIOC_CROPCAP, &cropcap) == 0)
    {
    struct v4l2_crop crop;
    
    fprintf(stderr, "Setting crop\n");
        
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; /* reset to default */

    if(my_ioctl(dev->fd, VIDIOC_S_CROP, &crop) == -1)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_S_CROP failed: %s", strerror(errno));
      goto fail;
      }
    }
  
  /* Get format */

  memset(&dev->capture.fmt, 0, sizeof(dev->capture.fmt));
  
  dev->capture.fmt.type = dev->capture.buf_type;
  
  if(my_ioctl(dev->fd, VIDIOC_G_FMT, &dev->capture.fmt) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_G_FMT failed: %s", strerror(errno));
    goto fail;
    }
  
  //  dump_fmt(dev, &fmt);
  
  if(IS_PLANAR(dev->capture.buf_type))
    {
    gavl_format->pixelformat = gavl_v4l2_pix_fmt_to_pixelformat(dev->capture.fmt.fmt.pix_mp.pixelformat,
                                                                dev->capture.fmt.fmt.pix_mp.quantization);
    }
  else
    {
    gavl_format->pixelformat = gavl_v4l2_pix_fmt_to_pixelformat(dev->capture.fmt.fmt.pix.pixelformat,
                                                                dev->capture.fmt.fmt.pix.quantization);
    }

  if(gavl_format->pixelformat == GAVL_PIXELFORMAT_NONE)
    {
    /* TODO: Negotiate format */
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got no decodable pixelformat");
    goto fail;
    }

  
  /* Wait for source_change event */

  while(!(dev->flags & DECODER_HAVE_FORMAT) && (format_packets < MAX_FORMAT_FRAMES))
    {
    do_poll(dev, POLLOUT | POLLPRI, &pollev);

    // fprintf(stderr, "Polled: %d %d %d\n", !!(pollev & POLLPRI), !!(pollev & POLLOUT), !!(pollev & POLLIN));
    
    if(pollev & POLLPRI)
      handle_decoder_event(dev);
    
    else if(pollev & POLLOUT)
      {
      if(!send_decoder_packet(dev) || (dev->flags & DECODER_SENT_EOS))
        {
        gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "EOF during decoder initialization (sent %d packets)", format_packets);
        goto fail;
        }


      //      gavl_time_delay(&t);
      format_packets++;
      }
    }
  
  if(!(dev->flags & DECODER_HAVE_FORMAT))
    {
    /* Poll specifically for events */
    do_poll(dev, POLLPRI, &pollev);
    if(pollev & POLLPRI)
      handle_decoder_event(dev);

    if(!(dev->flags & DECODER_HAVE_FORMAT))
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Couldn't get decoder format after sending %d packets", format_packets);
      goto fail;
      }
    }
  
  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Got decoder format after sending %d packets", format_packets);
  
  //  fprintf(stderr, "Do poll init: %d %d %d\n",
  //          !!(pollev & POLLIN), !!(pollev & POLLOUT), !!(pollev & POLLPRI));
  
  //  handle_decoder_event(dev);
  
  dev->vsrc_priv = gavl_video_source_create(get_frame_decoder, dev,
                                            GAVL_SOURCE_SRC_ALLOC,
                                            gavl_format);
  
  dev->capture.vframe = gavl_video_frame_create(NULL);

  gavl_video_format_copy(&dev->capture.format, gavl_format);
  
  ret = 1;
  fail:
  
  
  return ret;
  
  }

/* Generic video sink functions. Can hopefully be used by converters, encoders and outputs. */

static gavl_video_frame_t * sink_get_func_mmap(void * priv)
  {
  gavl_v4l2_device_t * dev = priv;

  if(!dev->output.vframe)
    dev->output.vframe = gavl_video_frame_create(NULL);

  if(!get_buffer_output(dev))
    return NULL;

  buffer_to_video_frame_mmap(dev, dev->output.current_buf,
                             &dev->output.fmt,
                             dev->output.format.pixelformat,
                             dev->output.buf_type);
  
  return dev->output.vframe;
  }

#ifdef HAVE_DRM
static gavl_sink_status_t sink_put_func_dmabuf(void * priv, gavl_video_frame_t * frame)
  {
  struct v4l2_buffer buf;
  gavl_video_frame_t * dma_frame;
  gavl_v4l2_device_t * dev = priv;

  gavl_dmabuf_video_frame_t * dmabuf;
  
  if(!gavl_hw_ctx_transfer_video_frame(frame, dev->hwctx_dmabuf, &dma_frame,
                                       &dev->output.format))
    return GAVL_SINK_ERROR;

  dmabuf = dma_frame->storage;
  
  memset(&buf, 0, sizeof buf);
  buf.type = dev->output.buf_type;
  buf.memory = V4L2_MEMORY_DMABUF;
  buf.index = dev->output.current_buf->index;

  buf.m.fd = dmabuf->buffers[0].fd;
  
  dev->output.current_buf->flags |= GAVL_V4L2_BUFFER_FLAG_QUEUED;

  return GAVL_SINK_OK;
  }

static gavl_sink_status_t sink_put_func_dmabuf_planar(void * priv, gavl_video_frame_t * frame)
  {
  struct v4l2_buffer buf;
  int num_planes;
  gavl_video_frame_t * dma_frame;
  gavl_v4l2_device_t * dev = priv;
  gavl_dmabuf_video_frame_t * dmabuf;
  int i;
  struct v4l2_plane planes[VIDEO_MAX_PLANES];

  num_planes = gavl_pixelformat_num_planes(dev->output.format.pixelformat);
  
  if(!gavl_hw_ctx_transfer_video_frame(frame, dev->hwctx_dmabuf, &dma_frame,
                                       &dev->output.format))
    return GAVL_SINK_ERROR;

  dmabuf = dma_frame->storage;

  memset(&buf, 0, sizeof buf);
  buf.type = dev->output.buf_type;
  buf.memory = V4L2_MEMORY_DMABUF;
  buf.index = dev->output.current_buf->index;
  buf.m.planes = planes;
  buf.length = num_planes;
  
  for(i = 0; i < num_planes; i++)
    {
    buf.m.planes[i].m.fd = dmabuf->buffers[i].fd;
    }
  
  dev->output.current_buf->flags |= GAVL_V4L2_BUFFER_FLAG_QUEUED;
  return GAVL_SINK_OK;
  }
#endif

static gavl_sink_status_t sink_put_func_mmap_planar(void * priv, gavl_video_frame_t * frame)
  {
  int i;
  int num_planes;
  struct v4l2_buffer buf;
  struct v4l2_plane planes[GAVL_MAX_PLANES];
  gavl_v4l2_device_t * dev = priv;

  memset(&buf, 0, sizeof(buf));
  memset(planes, 0, GAVL_MAX_PLANES*sizeof(planes[0]));

  num_planes = gavl_pixelformat_num_planes(dev->output.format.pixelformat);
  buf.m.planes = planes;

  for(i = 0; i < num_planes; i++)
    {
    buf.m.planes[i].bytesused = 0;
    buf.m.planes[i].bytesused = 0;
    
    }
  
  buf.length = num_planes;
  buf.type = dev->output.buf_type;
  buf.memory = V4L2_MEMORY_DMABUF;
  buf.index =  dev->output.current_buf->index;

  buf.m.planes = planes;
  
  buf.length = 1;
    
  dev->output.current_buf->flags |= GAVL_V4L2_BUFFER_FLAG_QUEUED;
  return GAVL_SINK_OK;
  
  }

static gavl_sink_status_t sink_put_func_mmap(void * priv, gavl_video_frame_t * frame)
  {
  gavl_v4l2_device_t * dev = priv;
  struct v4l2_buffer buf;
  
  /* Queue buffer */
  memset(&buf, 0, sizeof(buf));
  
  buf.type = dev->output.buf_type;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index =  dev->output.current_buf->index;
  buf.field = V4L2_FIELD_NONE;
  
  if(my_ioctl(dev->fd, VIDIOC_QBUF, &buf) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_QBUF failed for output: %s", strerror(errno));
    return GAVL_SINK_ERROR;
    }

  dev->output.current_buf->flags |= GAVL_V4L2_BUFFER_FLAG_QUEUED;
  
  return GAVL_SINK_OK;
  }


static int init_video_output(gavl_v4l2_device_t * dev,
                             const gavl_video_format_t * fmt, int num_bufs)
  {
  gavl_video_sink_get_func get_func = NULL;
  gavl_video_sink_put_func put_func = NULL;
  
  format_gavl_to_v4l2(fmt, &dev->output.fmt, dev->output.buf_type);
  gavl_video_format_copy(&dev->output.format, fmt);
  
  if(my_ioctl(dev->fd, VIDIOC_S_FMT, &dev->output.fmt) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_S_FMT failed: %s", strerror(errno));
    goto fail;
    }

  if(my_ioctl(dev->fd, VIDIOC_G_FMT, &dev->output.fmt) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_G_FMT failed: %s", strerror(errno));
    goto fail;
    }
  
#ifdef HAVE_DRM
  /* Initialize output (src) */
  if(fmt->hwctx && gavl_hw_ctx_exports_type(fmt->hwctx, GAVL_HW_DMABUFFER))
    {
    /* Try to allocate dmabuf buffer */

    if(request_buffers_dmabuf(dev, dev->output.buf_type, num_bufs, dev->output.bufs))
      {
      gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Importing DMA buffer");
      dev->hwctx_dmabuf = gavl_hw_ctx_create_dma();

      if(IS_PLANAR(dev->output.buf_type))
        put_func = sink_put_func_dmabuf_planar;
      else
        put_func = sink_put_func_dmabuf;
      
      }
    }
#endif

  /* Initialize sink for mmap */
  if(!put_func)
    {
    get_func = sink_get_func_mmap;
    if(dev->output.buf_type)
      put_func = sink_put_func_mmap_planar;
    else
      put_func = sink_put_func_mmap;
    }

  gavl_video_format_copy(&dev->output.format, fmt);
  
  if(!stream_on(dev, dev->output.buf_type))
    goto fail;
  
  dev->vsink_priv = gavl_video_sink_create(get_func, put_func, dev, fmt);
  
  return 1;
  
  fail:
  return 0;
  }

static gavl_source_status_t get_frame_capture(void * priv, gavl_video_frame_t ** frame)
  {
  
  return GAVL_SOURCE_OK;
  }

static int init_video_capture(gavl_v4l2_device_t * dev, gavl_video_format_t * fmt, int num_bufs)
  {

  if(!(dev->capture.num_bufs = request_buffers_mmap(dev, dev->capture.buf_type, num_bufs, dev->capture.bufs)))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Requesting capture buffers failed");
    goto fail;
    }
  
  stream_on(dev, dev->capture.buf_type);

  dev->vsrc_priv = gavl_video_source_create(get_frame_capture, dev, GAVL_SOURCE_SRC_ALLOC, fmt);
  fmt->hwctx = dev->hwctx;
  
  return 1;
  fail:
  return 0;
  
  }

int gavl_v4l2_device_init_converter(gavl_v4l2_device_t * dev,
                                    const gavl_video_format_t * src_fmt,
                                    gavl_video_format_t * dst_fmt)
  {
  int caps = 0;

  if(gavl_dictionary_get_int(&dev->dev, GAVL_V4L2_CAPABILITIES, &caps) &&
     (caps & V4L2_CAP_VIDEO_M2M_MPLANE))
    {
    dev->output.buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    dev->capture.buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    }
  else
    {
    dev->output.buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    dev->capture.buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    }

  if(!init_video_output(dev, src_fmt, 1))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot open V4L video converter: Input format not supported");
    return 0;
    }

  format_gavl_to_v4l2(src_fmt, &dev->capture.fmt, dev->capture.buf_type);

  if(my_ioctl(dev->fd, VIDIOC_S_FMT, &dev->capture.fmt) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_S_FMT failed: %s", strerror(errno));
    return 0;
    }
  
  if(!init_video_capture(dev, dst_fmt, 1))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot open V4L video converter: Output format not supported");
    return 0;
    }
  
  //  pix_fmt_src = gavl_v4l2_pixelformat_to_pix_fmt(src_fmt->pixelformat);
  //  pix_fmt_dst = gavl_v4l2_pixelformat_to_pix_fmt(dst_fmt->pixelformat);
  
#if 0
  if(!pix_fmt_dst)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot open V4L video converter: Output format not supported");
    return 0;
    }
#endif
  
  /* Initialize capture (src) */
  
  return 0;
  }
  
int gavl_v4l2_device_get_fd(gavl_v4l2_device_t * dev)
  {
  return dev->fd;
  }

void gavl_v4l2_device_close(gavl_v4l2_device_t * dev)
  {
  stream_off(dev, dev->output.buf_type);
  stream_off(dev, dev->capture.buf_type);

#ifdef HAVE_DRM  
  if(dev->hwctx_dmabuf)
    release_buffers_dmabuf(dev, dev->output.buf_type, dev->output.num_bufs, dev->output.bufs);
  else
#endif
    {
    if(dev->output.num_bufs)
      release_buffers_mmap(dev, dev->output.buf_type, dev->output.num_bufs, dev->output.bufs);
    }

  if(dev->capture.num_bufs)
    release_buffers_mmap(dev, dev->capture.buf_type, dev->capture.num_bufs, dev->capture.bufs);
  
  gavl_dictionary_free(&dev->dev);

  if(dev->capture.vframe)
    {
    gavl_video_frame_null(dev->capture.vframe);
    gavl_video_frame_destroy(dev->capture.vframe);
    }

  if(dev->output.vframe)
    {
    gavl_video_frame_null(dev->output.vframe);
    gavl_video_frame_destroy(dev->output.vframe);
    }

#ifdef HAVE_DRM  
  if(dev->hwctx_dmabuf)
    gavl_hw_ctx_destroy(dev->hwctx_dmabuf);
#endif
  
  if(dev->fd >= 0)
    close(dev->fd);
  
  if(dev->timer)
    gavl_timer_destroy(dev->timer);

  free(dev);
  }


gavl_video_source_t * gavl_v4l2_device_get_video_source(gavl_v4l2_device_t * dev)
  {
  return dev->vsrc_priv;
  }


gavl_video_sink_t * gavl_v4l2_device_get_video_sink(gavl_v4l2_device_t * dev)
  {
  return dev->vsink_priv;
  }

gavl_packet_source_t * gavl_v4l2_device_get_packet_source(gavl_v4l2_device_t * dev)
  {
  return dev->psrc_priv;
  }



#if 0

gavl_packet_sink_t * gavl_v4l2_device_get_packet_sink(gavl_v4l2_device_t * dev)
  {

  }


int gavl_v4l2_device_init_capture(gavl_v4l2_device_t * dev,
                                 gavl_video_format_t * fmt)
  {
  
  } 

int gavl_v4l2_device_init_output(gavl_v4l2_device_t * dev,
                           gavl_video_format_t * fmt)
  {
  
  }

/* Unused for now */
int v4l_device_init_encoder(gavl_v4l2_device_t * dev,
                            gavl_video_format_t * fmt,
                            gavl_compression_info_t * cmp);

#endif

int gavl_v4l2_get_device_info(const char * path, gavl_dictionary_t * dev)
  {
  int fd;
  gavl_v4l2_device_type_t type;
  struct v4l2_capability cap;
  gavl_array_t * src_formats;
  gavl_array_t * sink_formats;
  
  
  memset(&cap, 0, sizeof(cap));
  
  src_formats = NULL;
  sink_formats = NULL;
    
  if((fd = open(path, O_RDWR /* required */ | O_NONBLOCK, 0)) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Opening %s failed: %s",
             path, strerror(errno));
    return 0;
    }
  
  if(my_ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Getting capabilities for %s failed: %s",
             path, strerror(errno));
    close(fd);
    return 0;
    } 
    

  //    fprintf(stderr, "Card:\n");
  //    gavl_hexdump(cap->card, 32, 16);
    
  //    fprintf(stderr, "Blupp 1 %s\n", (const char*)cap->card);
    
  gavl_dictionary_set_string(dev, GAVL_META_LABEL, (const char*)cap.card);

  //    fprintf(stderr, "Blupp 2 %s\n", g.gl_pathv[i]);

  //    fprintf(stderr, "Blupp 3 %s\n", tmp_string);
    
  gavl_dictionary_set_string(dev, GAVL_META_URI, path);

  //    fprintf(stderr, "Blupp 4\n");
    
  gavl_dictionary_set_int(dev, GAVL_V4L2_CAPABILITIES, cap.capabilities);

  //    fprintf(stderr, "Blupp 5\n");
    
  /* Get source formats */
  if(cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE))
    {
    src_formats = gavl_dictionary_get_array_create(dev, GAVL_V4L2_SRC_FORMATS);
    query_formats(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, src_formats);
    }
  else if(cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_M2M))
    {
    src_formats = gavl_dictionary_get_array_create(dev, GAVL_V4L2_SRC_FORMATS);
    query_formats(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, src_formats);
    }
    
  /* Get output formats */
  if(cap.capabilities & (V4L2_CAP_VIDEO_OUTPUT_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE))
    {
    sink_formats = gavl_dictionary_get_array_create(dev, GAVL_V4L2_SINK_FORMATS);
    query_formats(fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, sink_formats);
    }
  else if(cap.capabilities & (V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_VIDEO_M2M))
    {
    sink_formats = gavl_dictionary_get_array_create(dev, GAVL_V4L2_SINK_FORMATS);
    query_formats(fd, V4L2_BUF_TYPE_VIDEO_OUTPUT, sink_formats);
    }

  if(src_formats && !src_formats->num_entries)
    {
    gavl_dictionary_set(dev, GAVL_V4L2_SRC_FORMATS, NULL);
    src_formats = NULL;
    }

  if(sink_formats && !sink_formats->num_entries)
    {
    gavl_dictionary_set(dev, GAVL_V4L2_SINK_FORMATS, NULL);
    sink_formats = NULL;
    }
  
  if(src_formats && !sink_formats)
    {
    type = GAVL_V4L2_DEVICE_SOURCE;
      
    }
  else if(!src_formats && sink_formats)
    {
    type = GAVL_V4L2_DEVICE_SINK;
      
    }
  else if(src_formats && sink_formats)
    {
    int src_compressed;
    int sink_compressed;
      
    src_compressed = formats_compressed(src_formats);
    sink_compressed = formats_compressed(sink_formats);

    if(!src_compressed && sink_compressed)
      type = GAVL_V4L2_DEVICE_DECODER;
    else if(src_compressed && !sink_compressed)
      type = GAVL_V4L2_DEVICE_ENCODER;
    else if(!src_compressed && !sink_compressed)
      type = GAVL_V4L2_DEVICE_CONVERTER;
    else
      type = GAVL_V4L2_DEVICE_UNKNOWN;
    }
  else
    type = GAVL_V4L2_DEVICE_UNKNOWN;
    
  gavl_dictionary_set_int(dev, GAVL_V4L2_TYPE, type);
  gavl_dictionary_set_string(dev, GAVL_V4L2_TYPE_STRING, get_type_label(type));

  /* Query controls */
  
  close(fd);
  
  return 1;
  }

void gavl_v4l2_devices_scan_by_type(int type_mask, gavl_array_t * ret)
  {
  int i;
  glob_t g;
  int type;
  
  memset(&g, 0, sizeof(g));
  
  glob("/dev/video*", 0, NULL, &g);

  for(i = 0; i < g.gl_pathc; i++)
    {
    gavl_value_t dev_val;
    gavl_dictionary_t * dev;
    
    gavl_value_init(&dev_val);
    dev = gavl_value_set_dictionary(&dev_val);
    gavl_v4l2_get_device_info(g.gl_pathv[i], dev);

    gavl_dictionary_get_int(dev, GAVL_V4L2_TYPE, &type);
    
    if(type_mask && !(type & type_mask))
      gavl_value_free(&dev_val);
    else
      gavl_array_splice_val_nocopy(ret, -1, 0, &dev_val);
    }

  globfree(&g);
  }

const gavl_dictionary_t * gavl_v4l2_get_decoder(const gavl_array_t * arr, gavl_codec_id_t id)
  {
  int i, j;

  const gavl_array_t * formats;
  
  const gavl_dictionary_t * dev;
  const gavl_dictionary_t * fmt;

  int type = GAVL_V4L2_DEVICE_UNKNOWN;
  int codec_id;
  
  for(i = 0; i < arr->num_entries; i++)
    {
    if((dev = gavl_value_get_dictionary(&arr->entries[i])) &&
       gavl_dictionary_get_int(dev, GAVL_V4L2_TYPE, &type) &&
       (type == GAVL_V4L2_DEVICE_DECODER) &&
       (formats = gavl_dictionary_get_array(dev, GAVL_V4L2_SINK_FORMATS)))
      {
      for(j = 0; j < formats->num_entries; j++)
        {
        if((fmt = gavl_value_get_dictionary(&formats->entries[j])) &&
           gavl_dictionary_get_int(fmt, GAVL_V4L2_FORMAT_GAVL_CODEC_ID, &codec_id) &&
           (codec_id == id))
          return dev;
        }
      }
    }
  
  return NULL;
  }


void gavl_v4l2_devices_scan(gavl_array_t * ret)
  {
  return gavl_v4l2_devices_scan_by_type(0, ret);
  }

  
/* v4l formats vs gavl_pixelformat_t and 
   gavl_compression_id_t */

static const struct
  {
  uint32_t           v4l2;

  gavl_pixelformat_t pixelformat;

  
  gavl_codec_id_t codec_id;
  }
pixelformats[] =
  {
    /*      Pixel format         FOURCC                        depth  Description  */
    // #define V4L2_PIX_FMT_RGB332  v4l2_fourcc('R','G','B','1') /*  8  RGB-3-3-2     */
    // #define V4L2_PIX_FMT_RGB444  v4l2_fourcc('R','4','4','4') /* 16  xxxxrrrr ggggbbbb */
    // #define V4L2_PIX_FMT_RGB555  v4l2_fourcc('R','G','B','O') /* 16  RGB-5-5-5     */
    // #define V4L2_PIX_FMT_RGB565  v4l2_fourcc('R','G','B','P') /* 16  RGB-5-6-5     */
    // #define V4L2_PIX_FMT_RGB555X v4l2_fourcc('R','G','B','Q') /* 16  RGB-5-5-5 BE  */
    // #define V4L2_PIX_FMT_RGB565X v4l2_fourcc('R','G','B','R') /* 16  RGB-5-6-5 BE  */
    // #define V4L2_PIX_FMT_BGR24   v4l2_fourcc('B','G','R','3') /* 24  BGR-8-8-8     */
   { V4L2_PIX_FMT_BGR24, GAVL_BGR_24, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_RGB24   v4l2_fourcc('R','G','B','3') /* 24  RGB-8-8-8     */
   { V4L2_PIX_FMT_RGB24, GAVL_RGB_24, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_BGR32   v4l2_fourcc('B','G','R','4') /* 32  BGR-8-8-8-8   */
   { V4L2_PIX_FMT_BGR32, GAVL_BGR_32, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_RGB32   v4l2_fourcc('R','G','B','4') /* 32  RGB-8-8-8-8   */
   { V4L2_PIX_FMT_RGB32, GAVL_RGB_32, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_GREY    v4l2_fourcc('G','R','E','Y') /*  8  Greyscale     */
   { V4L2_PIX_FMT_GREY, GAVL_GRAY_8,  GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_PAL8    v4l2_fourcc('P','A','L','8') /*  8  8-bit palette */
    // #define V4L2_PIX_FMT_YVU410  v4l2_fourcc('Y','V','U','9') /*  9  YVU 4:1:0     */
   { V4L2_PIX_FMT_YVU410, GAVL_YUV_410_P, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_YVU420  v4l2_fourcc('Y','V','1','2') /* 12  YVU 4:2:0     */
   { V4L2_PIX_FMT_YVU420, GAVL_YUV_420_P, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_YUYV    v4l2_fourcc('Y','U','Y','V') /* 16  YUV 4:2:2     */
   { V4L2_PIX_FMT_YUYV, GAVL_YUY2, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_UYVY    v4l2_fourcc('U','Y','V','Y') /* 16  YUV 4:2:2     */
   { V4L2_PIX_FMT_UYVY, GAVL_UYVY, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_YUV422P v4l2_fourcc('4','2','2','P') /* 16  YVU422 planar */
   { V4L2_PIX_FMT_YUV422P, GAVL_YUV_422_P, GAVL_CODEC_ID_NONE },
   { V4L2_PIX_FMT_YUV422P, GAVL_YUVJ_422_P, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_YUV411P v4l2_fourcc('4','1','1','P') /* 16  YVU411 planar */
   { V4L2_PIX_FMT_YUV411P, GAVL_YUV_411_P, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_Y41P    v4l2_fourcc('Y','4','1','P') /* 12  YUV 4:1:1     */
   { V4L2_PIX_FMT_Y41P, GAVL_YUV_411_P, GAVL_CODEC_ID_NONE},
    // #define V4L2_PIX_FMT_YUV444  v4l2_fourcc('Y','4','4','4') /* 16  xxxxyyyy uuuuvvvv */
    // #define V4L2_PIX_FMT_YUV555  v4l2_fourcc('Y','U','V','O') /* 16  YUV-5-5-5     */
    // #define V4L2_PIX_FMT_YUV565  v4l2_fourcc('Y','U','V','P') /* 16  YUV-5-6-5     */
    // #define V4L2_PIX_FMT_YUV32   v4l2_fourcc('Y','U','V','4') /* 32  YUV-8-8-8-8   */

/* two planes -- one Y, one Cr + Cb interleaved  */
    // #define V4L2_PIX_FMT_NV12    v4l2_fourcc('N','V','1','2') /* 12  Y/CbCr 4:2:0  */
    // #define V4L2_PIX_FMT_NV21    v4l2_fourcc('N','V','2','1') /* 12  Y/CrCb 4:2:0  */

/*  The following formats are not defined in the V4L2 specification */
    // #define V4L2_PIX_FMT_YUV410  v4l2_fourcc('Y','U','V','9') /*  9  YUV 4:1:0     */
    // #define V4L2_PIX_FMT_YUV420  v4l2_fourcc('Y','U','1','2') /* 12  YUV 4:2:0     */
   { V4L2_PIX_FMT_YUV420, GAVL_YUV_420_P, GAVL_CODEC_ID_NONE },
   { V4L2_PIX_FMT_YUV420, GAVL_YUVJ_420_P, GAVL_CODEC_ID_NONE },
    
    // #define V4L2_PIX_FMT_YYUV    v4l2_fourcc('Y','Y','U','V') /* 16  YUV 4:2:2     */
    // #define V4L2_PIX_FMT_HI240   v4l2_fourcc('H','I','2','4') /*  8  8-bit color   */
    // #define V4L2_PIX_FMT_HM12    v4l2_fourcc('H','M','1','2') /*  8  YUV 4:2:0 16x16 macroblocks */

/* see http://www.siliconimaging.com/RGB%20Bayer.htm */
    // #define V4L2_PIX_FMT_SBGGR8  v4l2_fourcc('B','A','8','1') /*  8  BGBG.. GRGR.. */

/* compressed formats */

/* compressed formats */
// #define V4L2_PIX_FMT_MJPEG    v4l2_fourcc('M', 'J', 'P', 'G') /* Motion-JPEG   */

    
// #define V4L2_PIX_FMT_JPEG     v4l2_fourcc('J', 'P', 'E', 'G') /* JFIF JPEG     */
   { V4L2_PIX_FMT_JPEG, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_JPEG },
   { V4L2_PIX_FMT_MJPEG, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_JPEG },
   
// #define V4L2_PIX_FMT_DV       v4l2_fourcc('d', 'v', 's', 'd') /* 1394          */

   // #define V4L2_PIX_FMT_MPEG     v4l2_fourcc('M', 'P', 'E', 'G') /* MPEG-1/2/4 Multiplexed */
    // #define V4L2_PIX_FMT_H264     v4l2_fourcc('H', '2', '6', '4') /* H264 with start codes */
   { V4L2_PIX_FMT_H264, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_H264 },
    // #define V4L2_PIX_FMT_H264_NO_SC v4l2_fourcc('A', 'V', 'C', '1') /* H264 without start codes */
    // #define V4L2_PIX_FMT_H264_MVC v4l2_fourcc('M', '2', '6', '4') /* H264 MVC */
    // #define V4L2_PIX_FMT_H263     v4l2_fourcc('H', '2', '6', '3') /* H263          */
    // #define V4L2_PIX_FMT_MPEG1    v4l2_fourcc('M', 'P', 'G', '1') /* MPEG-1 ES     */
   { V4L2_PIX_FMT_MPEG1, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_MPEG1 },
    // #define V4L2_PIX_FMT_MPEG2    v4l2_fourcc('M', 'P', 'G', '2') /* MPEG-2 ES     */
   { V4L2_PIX_FMT_MPEG2, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_MPEG2 },
    // #define V4L2_PIX_FMT_MPEG2_SLICE v4l2_fourcc('M', 'G', '2', 'S') /* MPEG-2 parsed slice data */
    // #define V4L2_PIX_FMT_MPEG4    v4l2_fourcc('M', 'P', 'G', '4') /* MPEG-4 part 2 ES */
   { V4L2_PIX_FMT_MPEG4, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_MPEG4_ASP },
    // #define V4L2_PIX_FMT_XVID     v4l2_fourcc('X', 'V', 'I', 'D') /* Xvid           */
    // #define V4L2_PIX_FMT_VC1_ANNEX_G v4l2_fourcc('V', 'C', '1', 'G') /* SMPTE 421M Annex G compliant stream */
    // #define V4L2_PIX_FMT_VC1_ANNEX_L v4l2_fourcc('V', 'C', '1', 'L') /* SMPTE 421M Annex L compliant stream */
    // #define V4L2_PIX_FMT_VP8      v4l2_fourcc('V', 'P', '8', '0') /* VP8 */
   { V4L2_PIX_FMT_VP8, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_VP8 },
    // #define V4L2_PIX_FMT_VP9      v4l2_fourcc('V', 'P', '9', '0') /* VP9 */
    // #define V4L2_PIX_FMT_HEVC     v4l2_fourcc('H', 'E', 'V', 'C') /* HEVC aka H.265 */
    // #define V4L2_PIX_FMT_FWHT     v4l2_fourcc('F', 'W', 'H', 'T') /* Fast Walsh Hadamard Transform (vicodec) */
    // #define V4L2_PIX_FMT_FWHT_STATELESS     v4l2_fourcc('S', 'F', 'W', 'H') /* Stateless FWHT (vicodec) */
    
/*  Vendor-specific formats   */
    // #define V4L2_PIX_FMT_WNVA     v4l2_fourcc('W','N','V','A') /* Winnov hw compress */
    // #define V4L2_PIX_FMT_SN9C10X  v4l2_fourcc('S','9','1','0') /* SN9C10x compression */
    // #define V4L2_PIX_FMT_PWC1     v4l2_fourcc('P','W','C','1') /* pwc older webcam */
    // #define V4L2_PIX_FMT_PWC2     v4l2_fourcc('P','W','C','2') /* pwc newer webcam */
    // #define V4L2_PIX_FMT_ET61X251 v4l2_fourcc('E','6','2','5') /* ET61X251 compression */
   
   { },
  };

gavl_codec_id_t gavl_v4l2_pix_fmt_to_codec_id(uint32_t fmt)
  {
  int idx = 0;

  while(pixelformats[idx].v4l2)
    {
    if(pixelformats[idx].v4l2 == fmt)
      return pixelformats[idx].codec_id;
    idx++;
    }

  if(gavl_v4l2_pix_fmt_to_pixelformat(fmt, V4L2_QUANTIZATION_DEFAULT) == GAVL_PIXELFORMAT_NONE)
    return GAVL_CODEC_ID_EXTENDED;
  
  return GAVL_CODEC_ID_NONE;
  }

uint32_t gavl_v4l2_codec_id_to_pix_fmt(gavl_codec_id_t id)
  {
  int idx = 0;

  while(pixelformats[idx].v4l2)
    {
    if(pixelformats[idx].codec_id == id)
      return pixelformats[idx].v4l2;
    idx++;
    }
  return 0;
  
  }


gavl_pixelformat_t gavl_v4l2_pix_fmt_to_pixelformat(uint32_t fmt, int quantization)
  {
  int idx = 0;

  if(quantization == V4L2_QUANTIZATION_FULL_RANGE)
    {
    if(pixelformats[idx].v4l2 == V4L2_PIX_FMT_YVU420)
      return GAVL_YUVJ_420_P;
    if(pixelformats[idx].v4l2 == V4L2_PIX_FMT_YUV422P)
      return GAVL_YUVJ_422_P;
    }
  
  while(pixelformats[idx].v4l2)
    {
    if(pixelformats[idx].v4l2 == fmt)
      {
      return pixelformats[idx].pixelformat;
      }
    idx++;
    }
  return GAVL_PIXELFORMAT_NONE;
  }

uint32_t gavl_v4l2_pixelformat_to_pix_fmt(gavl_pixelformat_t fmt, int * quantization)
  {
  int idx = 0;

  while(pixelformats[idx].v4l2)
    {
    if(pixelformats[idx].pixelformat == fmt)
      {
      if(gavl_pixelformat_is_yuv(fmt) &&
         gavl_pixelformat_is_jpeg_scaled(fmt))
        *quantization = V4L2_QUANTIZATION_FULL_RANGE;
      else
        *quantization = V4L2_QUANTIZATION_DEFAULT;
      
      return pixelformats[idx].v4l2;
      }
    idx++;
    }
  return 0;
  }

static void format_v4l2_to_gavl(const struct v4l2_format * v4l,
                                const struct v4l2_streamparm * parm,
                                gavl_video_format_t * gavl, int type)
  {
  if(IS_PLANAR(type))
    {
    gavl->image_width = v4l->fmt.pix_mp.width;
    gavl->image_height = v4l->fmt.pix_mp.height;
    gavl->pixelformat = gavl_v4l2_pix_fmt_to_pixelformat(v4l->fmt.pix_mp.pixelformat, v4l->fmt.pix_mp.quantization);
    }
  else
    {
    gavl->image_width = v4l->fmt.pix.width;
    gavl->image_height = v4l->fmt.pix.height;
    gavl->pixelformat = gavl_v4l2_pix_fmt_to_pixelformat(v4l->fmt.pix.pixelformat, v4l->fmt.pix.quantization);
    }
  gavl->pixel_width = 1;
  gavl->pixel_height = 1;
  gavl->frame_width = gavl->image_width;
  gavl->frame_height = gavl->image_height;
  
  if(IS_CAPTURE(type))
    {
#if 0
    if(parm->parm.capture.capability & V4L2_CAP_TIMEPERFRAME)
      {
      gavl->framerate_mode = GAVL_FRAMERATE_CONSTANT;
      gavl->timescale      = parm->parm.capture.timeperframe.denominator;
      gavl->frame_duration = parm->parm.capture.timeperframe.numerator;
      }
    else
      {
#endif
      gavl->framerate_mode = GAVL_FRAMERATE_VARIABLE;
      gavl->timescale      = GAVL_TIME_SCALE;
#if 0
      }
#endif
    }
  }


static void format_gavl_to_v4l2(const gavl_video_format_t * gavl,
                                struct v4l2_format * v4l, int type)
  {
  int quantization = 0;

  memset(v4l, 0, sizeof(*v4l));

  v4l->type = type;
  
  if(IS_PLANAR(type))
    {
    v4l->fmt.pix_mp.width  = gavl->image_width;
    v4l->fmt.pix_mp.height = gavl->image_height;
    v4l->fmt.pix_mp.field = V4L2_FIELD_ANY;
    
    v4l->fmt.pix_mp.pixelformat = gavl_v4l2_pixelformat_to_pix_fmt(gavl->pixelformat,
                                                                   &quantization);
    v4l->fmt.pix_mp.quantization = quantization;
    
    }
  else
    {
    v4l->fmt.pix.width  = gavl->image_width;
    v4l->fmt.pix.height = gavl->image_height;
    v4l->fmt.pix_mp.field = V4L2_FIELD_ANY;
    
    v4l->fmt.pix.pixelformat = gavl_v4l2_pixelformat_to_pix_fmt(gavl->pixelformat,
                                                                &quantization);
    v4l->fmt.pix.quantization = quantization;
    }
  
  }

static void destroy_native_v4l2(void * native)
  {
  gavl_v4l2_device_close(native);
  }

static int exports_type_v4l2(gavl_hw_context_t * ctx, gavl_hw_type_t hw)
  {
  switch(hw)
    {
#ifdef HAVE_DRM
    case GAVL_HW_DMABUFFER:
      return 1;
      break;
#endif
    default:
      break;
    }
  return 0;
  }

static int export_video_frame_v4l2(gavl_hw_context_t * ctx, const gavl_video_format_t * fmt,
                                   gavl_video_frame_t * src, gavl_video_frame_t * dst)
  {
  gavl_v4l2_device_t * dev = ctx->native;
  gavl_hw_type_t dst_hw_type = gavl_hw_ctx_get_type(dst->hwctx);

  switch(dst_hw_type)
    {
#ifdef HAVE_DRM
    case GAVL_HW_DMABUFFER:
      {
      int i;
      gavl_dmabuf_video_frame_t * dma_frame = dst->storage;
      gavl_v4l2_buffer_t * v4l2_storage = src->storage;

      dma_frame->num_buffers = v4l2_storage->num_planes;
      dma_frame->num_planes = gavl_pixelformat_num_planes(fmt->pixelformat);
      dma_frame->fourcc = gavl_drm_fourcc_from_gavl(fmt->pixelformat);
      
      /* Export buffers */
      for(i = 0; i < v4l2_storage->num_planes; i++)
        {
        struct v4l2_exportbuffer expbuf;
    
        memset(&expbuf, 0, sizeof(expbuf));
        expbuf.type = v4l2_storage->type;
        expbuf.index = v4l2_storage->index;
        expbuf.plane = i;
    
        if(my_ioctl(dev->fd, VIDIOC_EXPBUF, &expbuf) == -1)
          {
          gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Exporting buffer failed: %s", strerror(errno));
          return 0;
          }
        dma_frame->buffers[i].fd = expbuf.fd;
        }
      
      /* Setup planes */
      for(i = 0; i < dma_frame->num_planes; i++)
        {
        dst->strides[i] = src->strides[i];

        if(!i)
          {
          dma_frame->planes[i].buf_idx = 0;
          dma_frame->planes[i].offset = 0;
          }
        else if(dma_frame->num_buffers == 1)
          {
          dma_frame->planes[i].buf_idx = 0;
          dma_frame->planes[i].offset = src->planes[i] - src->planes[0];
          }
        else
          {
          dma_frame->planes[i].buf_idx = 1;
          dma_frame->planes[i].offset = 0;
          }
        }
      
      dst->buf_idx = src->buf_idx;
      }
#endif
    default:
      break;
    }
  return 0;
  }


static const gavl_hw_funcs_t funcs =
  {
   .destroy_native         = destroy_native_v4l2,
   //    .get_image_formats      = gavl_gl_get_image_formats,
   //    .get_overlay_formats    = gavl_gl_get_overlay_formats,
   //    .video_frame_create_hw  = video_frame_create_hw_egl,
   //    .video_frame_destroy    = video_frame_destroy_egl,
   //    .video_frame_to_ram     = video_frame_to_ram_egl,
   //    .video_frame_to_hw      = video_frame_to_hw_egl,
   //    .video_format_adjust    = gavl_gl_adjust_video_format,
   //    .overlay_format_adjust  = gavl_gl_adjust_video_format,
    .can_export             = exports_type_v4l2,
    .export_video_frame = export_video_frame_v4l2,
  };


/* hw context associated with a device */

gavl_hw_context_t * gavl_hw_ctx_create_v4l2(const gavl_dictionary_t * dev_info)
  {
  gavl_hw_context_t * ret;
  gavl_v4l2_device_t * dev = gavl_v4l2_device_open(dev_info);
  
  ret = gavl_hw_context_create_internal(dev, &funcs, GAVL_HW_V4L2_BUFFER, GAVL_HW_SUPPORTS_VIDEO);
  dev->hwctx = ret;
  return ret;
  }

gavl_v4l2_device_t * gavl_hw_ctx_v4l2_get_device(gavl_hw_context_t * ctx)
  {
  return ctx->native;
  }

