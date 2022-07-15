
#include <config.h>

#include <linux/videodev2.h>
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
#define LOG_DOMAIN "v4l"


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

static gavl_v4l2_device_t * gavl_v4l2_device_open(const gavl_dictionary_t * dev);

  
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
                              gavl_v4l2_pix_fmt_to_pixelformat(fmt.pixelformat));

    gavl_array_splice_val_nocopy(ret, -1, 0, &val);
    }
  
  }

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
  

struct gavl_v4l2_device_s
  {
  gavl_dictionary_t dev;
  int fd;

  int decoder_delay; // packets sent - frames read
  
  int num_output_bufs;
  int num_capture_bufs;
  
  gavl_v4l2_buffer_t output_bufs[MAX_BUFFERS]; // Output
  gavl_v4l2_buffer_t capture_bufs[MAX_BUFFERS];  // Capture
  
  gavl_v4l2_buffer_t * out_buf;
  gavl_v4l2_buffer_t * in_buf;
  
  int planar;

  int buf_type_capture;
  int buf_type_output;
    
  gavl_video_frame_t * vframe;
  gavl_packet_t packet;

  int timescale;

  
  gavl_video_source_t * vsrc_priv;
  gavl_packet_source_t * psrc;
  
  //  gavl_video_source_t * vsrc_;
  
  //  gavl_packet_source_t * psrc_priv;
  
  gavl_packet_pts_cache_t * cache;
  
  struct v4l2_format capture_fmt;
  gavl_video_format_t capture_format;

  int flags;

  gavl_hw_context_t * hwctx;
  
  };

static int dequeue_buffer(gavl_v4l2_device_t * dev, int type)
  {
  int i;
  struct v4l2_buffer buf;
  struct v4l2_plane planes[GAVL_MAX_PLANES];

  if(type == dev->buf_type_output)
    {
    for(i = 0; i < dev->num_output_bufs; i++)
      {
      if(!(dev->output_bufs[i].flags & GAVL_V4L2_BUFFER_FLAG_QUEUED))
        return i;
      }
    }
  
  /* Dequeue buffer */
  memset(&buf, 0, sizeof(buf));

  if(dev->planar)
    {
    memset(planes, 0, GAVL_MAX_PLANES*sizeof(planes[0]));
    buf.m.planes = planes;
    buf.length = GAVL_MAX_PLANES;
    }
  
  buf.type = type;
  buf.memory = V4L2_MEMORY_MMAP;
  
  if(my_ioctl(dev->fd, VIDIOC_DQBUF, &buf) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_DQBUF failed: %s", strerror(errno));
    return -1;
    }
  return buf.index;
  }

static gavl_v4l2_buffer_t * get_buffer_output(gavl_v4l2_device_t * dev)
  {
  int idx;

  idx = dequeue_buffer(dev, dev->buf_type_output);
  
  dev->out_buf = &dev->output_bufs[idx];
  dev->out_buf->flags &= ~GAVL_V4L2_BUFFER_FLAG_QUEUED;
  
  return dev->out_buf;
  }

static int done_buffer_capture(gavl_v4l2_device_t * dev)
  {
  struct v4l2_buffer buf;
  struct v4l2_plane planes[GAVL_MAX_PLANES];
  
  if(dev->in_buf)
    {
    /* Queue buffer */
    memset(&buf, 0, sizeof(buf));
    
    if(dev->planar)
      {
      memset(planes, 0, GAVL_MAX_PLANES*sizeof(planes[0]));
      buf.m.planes = planes;
      buf.length = dev->capture_fmt.fmt.pix_mp.num_planes;
      }
    
    buf.type = dev->buf_type_capture;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index =  dev->in_buf->index;

    dev->in_buf->flags |= GAVL_V4L2_BUFFER_FLAG_QUEUED;
    dev->in_buf = NULL;
    
    if(my_ioctl(dev->fd, VIDIOC_QBUF, &buf) == -1)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_QBUF failed for capture: %s", strerror(errno));
      return 0;
      }
    
    }

  return 1;
  }

static gavl_packet_t * gavl_v4l2_device_get_packet_write(gavl_v4l2_device_t * dev)
  {
  
  if(!get_buffer_output(dev))
    return NULL;
  
  dev->packet.data       = dev->out_buf->planes[0].buf;
  dev->packet.data_alloc = dev->out_buf->planes[0].size;
  dev->packet.data_len   = 0;
  
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
  
  if(dev->planar)
    {
    memset(planes, 0, GAVL_MAX_PLANES*sizeof(planes[0]));
    buf.m.planes = planes;
    buf.m.planes[0].bytesused = dev->packet.data_len;
    buf.length = 1;
    }
  else
    {
    buf.bytesused = dev->packet.data_len;
    }
  
  if(dev->packet.flags & GAVL_PACKET_KEYFRAME)
    buf.flags |= V4L2_BUF_FLAG_KEYFRAME;
  
  buf.type = dev->buf_type_output;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index =  dev->out_buf->index;

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

  dev->out_buf->flags |= GAVL_V4L2_BUFFER_FLAG_QUEUED;

  
  return GAVL_SINK_OK;
  }

static int send_decoder_packet(gavl_v4l2_device_t * dev)
  {
  gavl_packet_t * p = gavl_v4l2_device_get_packet_write(dev);

  /* Send EOF */
  
  if((gavl_packet_source_read_packet(dev->psrc, &p) != GAVL_SOURCE_OK))
    {

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
      fprintf(stderr, "Sent EOS\n");
      return 1;
      }
    else
      {
      return 0;
      }
    }
  
  if(gavl_v4l2_device_put_packet_write(dev) != GAVL_SINK_OK)
    return 0;
  
  //  gavl_packet_dump(p);
  
  dev->decoder_delay++;
  gavl_packet_pts_cache_push(dev->cache, p);
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
    
    if(dev->planar)
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
    
    if(dev->planar)
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
      {
      if(bufs[i].planes[j].dma_fd > 0)
        close(bufs[i].planes[j].dma_fd);

      munmap(bufs[i].planes[j].buf, bufs[i].planes[j].size);
      }
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

static int queue_frame_decoder(gavl_v4l2_device_t * dev, int idx)
  {
  struct v4l2_buffer buf;
  struct v4l2_plane planes[GAVL_MAX_PLANES];
  
  memset(&buf, 0, sizeof(buf));

  if(dev->planar)
    {
    memset(planes, 0, GAVL_MAX_PLANES*sizeof(planes[0]));
    buf.m.planes = planes;
    buf.length = GAVL_MAX_PLANES;
    }
  
  buf.type = dev->buf_type_capture;
  
  buf.index = idx;
  buf.memory = V4L2_MEMORY_MMAP;
  
  if(my_ioctl(dev->fd, VIDIOC_QBUF, &buf) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_QBUF failed: %s", strerror(errno));
    return 0;
    }
  
  dev->capture_bufs[idx].flags |= GAVL_V4L2_BUFFER_FLAG_QUEUED;
  
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
          memset(&dev->capture_fmt, 0, sizeof(dev->capture_fmt));
          
          //          fprintf(stderr, "Resolution changed\n");
          
          dev->capture_fmt.type = dev->buf_type_capture;
          
          if(my_ioctl(dev->fd, VIDIOC_G_FMT, &dev->capture_fmt) == -1)
            {
            gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_G_FMT failed: %s", strerror(errno));
            }
          //          dump_fmt(dev, &dev->capture_fmt);
#if 0
          /* Stop capturing and re-create the buffers */
          stream_off(dev, dev->buf_type_capture);

          /* Unmap buffers */
          release_buffers_mmap(dev, dev->buf_type_capture, dev->num_capture_bufs, dev->capture_bufs);
#endif
          
          dev->num_capture_bufs = request_buffers_mmap(dev, dev->buf_type_capture,
                                                       DECODER_NUM_FRAMES, dev->capture_bufs);

          for(i = 0; i < dev->num_capture_bufs; i++)
            queue_frame_decoder(dev, i);
          
          stream_on(dev, dev->buf_type_capture);
          
          //      fprintf(stderr, "Re-created buffers\n");

          dev->flags |= DECODER_HAVE_FORMAT;
          }

        break;
      case V4L2_EVENT_EOS:
        dev->flags |= DECODER_GOT_EOS;
        fprintf(stderr, "Got EOS\n");
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

static void buffer_to_video_frame(gavl_v4l2_device_t * dev, gavl_v4l2_buffer_t * buf,
                                  const struct v4l2_format * fmt)
  {
  if(dev->capture_format.pixelformat == GAVL_YUV_420_P)
    {
    uint8_t * ptr;
    int bytesperline = 0;
    int sizeimage = 0;
    int sizeimage_y;
    int sizeimage_cbcr;
    uint32_t pixfmt = 0;
    
    if(dev->planar)
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
    
    dev->vframe->planes[0] = ptr;

    if(pixfmt == V4L2_PIX_FMT_YUV420)
      {
      dev->vframe->planes[1] = dev->vframe->planes[0] + sizeimage_y;
      dev->vframe->planes[2] = dev->vframe->planes[1] + sizeimage_cbcr;
      }
    else
      {
      dev->vframe->planes[2] = dev->vframe->planes[0] + sizeimage_y;
      dev->vframe->planes[1] = dev->vframe->planes[2] + sizeimage_cbcr;
      }
    dev->vframe->strides[0] = bytesperline;
    dev->vframe->strides[1] = bytesperline / 2;
    dev->vframe->strides[2] = bytesperline / 2;
    }
  else if(gavl_pixelformat_is_planar(dev->capture_format.pixelformat))
    {
    /* TODO */
    }
  else // Packed
    {
    int bytesperline = 0;
    
    if(dev->planar)
      {
      bytesperline = fmt->fmt.pix_mp.plane_fmt[0].bytesperline;
      }
    else
      {
      bytesperline = fmt->fmt.pix.bytesperline;;
      }

    dev->vframe->planes[0] = buf->planes[0].buf;
    dev->vframe->strides[0] = bytesperline;
    }

  dev->vframe->hwctx = dev->hwctx;
  dev->vframe->storage = buf;
  dev->vframe->buf_idx = buf->index;
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
      gavl_packet_t pkt;
      
      idx = dequeue_buffer(dev, dev->buf_type_capture);
      
      if(idx < 0)
        return GAVL_SOURCE_EOF;

      /* Set output buffer to frame */

      dev->in_buf = &dev->capture_bufs[idx];
      
      buffer_to_video_frame(dev, dev->in_buf, &dev->capture_fmt);

      gavl_packet_pts_cache_get_first(dev->cache, &pkt);
      gavl_packet_to_videoframe(&pkt, dev->vframe);

      //      fprintf(stderr, "Frame pts: %"PRId64"\n", dev->vframe->timestamp);

      if(frame)
        *frame = dev->vframe;
      
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
  
  stream_off(dev, dev->buf_type_capture);
  stream_off(dev, dev->buf_type_output);

  gavl_packet_pts_cache_clear(dev->cache);

  dev->in_buf = NULL;
  dev->out_buf = NULL;
  
  /* Queue frames */
  for(i = 0; i < dev->num_capture_bufs; i++)
    {
    queue_frame_decoder(dev, i);
    }
  
  for(i = 0; i < dev->num_output_bufs; i++)
    {
    dev->output_bufs[i].flags &= ~GAVL_V4L2_BUFFER_FLAG_QUEUED;
    }

  stream_on(dev, dev->buf_type_output);
  stream_on(dev, dev->buf_type_capture);

  dev->decoder_delay = 0;
  
  for(i = 0; i < dev->num_output_bufs; i++)
    send_decoder_packet(dev);
  
  do_poll(dev, POLLIN, &pollev);

#if 0  
  fprintf(stderr, "Do poll resync: %d %d %d\n",
          !!(pollev & POLLIN), !!(pollev & POLLOUT), !!(pollev & POLLPRI));
  
  fprintf(stderr, "Resync done\n");
#endif
  }
#endif

#if 0
int gavl_v4l_device_init_capture(gavl_v4l2_device_t * dev,
                                 gavl_video_format_t * fmt)
  {
  return 0;
  }
#endif

int gavl_v4l2_device_init_decoder(gavl_v4l2_device_t * dev, gavl_dictionary_t * stream,
                                 gavl_packet_source_t * psrc)
  {
  int format_packets = 0;
  
  int caps = 0;
  int ret = 0;
  gavl_video_format_t * gavl_format;
  gavl_compression_info_t ci;
  struct v4l2_format fmt;
  gavl_stream_stats_t stats;
  
  int pollev = 0;
  
  int max_packet_size;
  struct v4l2_event_subscription sub;
  int packets_to_send;
  //  int i;

  struct v4l2_cropcap cropcap;

  //  fprintf(stderr, "gavl_v4l2_device_init_decoder\n");
  //  gavl_dictionary_dump(stream, 2);

  gavl_format = gavl_stream_get_video_format_nc(stream);

  gavl_format->hwctx = dev->hwctx;
  
  dev->timescale = gavl_format->timescale;

  dev->cache = gavl_packet_pts_cache_create(2*MAX_BUFFERS);

  /* Subscribe to events */
  
  memset(&sub, 0, sizeof(sub));
  
  sub.type = V4L2_EVENT_EOS;
  if(my_ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_SUBSCRIBE_EVENT failed: %s", strerror(errno));
    goto fail;
    }
  
  sub.type = V4L2_EVENT_SOURCE_CHANGE;
  if(my_ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_SUBSCRIBE_EVENT failed: %s", strerror(errno));
    goto fail;
    }
  
  memset(&ci, 0, sizeof(ci));
  
  if(!gavl_stream_get_compression_info(stream, &ci))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Got no compression info");
    goto fail;
    }
  gavl_stream_stats_init(&stats);
  gavl_stream_get_stats(stream, &stats);

  gavl_video_format_set_frame_size(gavl_format, 16, 16);
  
  if(stats.size_max > 0)
    max_packet_size = stats.size_max + 128;
  else
    max_packet_size = (gavl_format->frame_width * gavl_format->frame_width * 3) / 4 + 128;
  
  memset(&fmt, 0, sizeof(fmt));

  /* */
  
  
  if(gavl_dictionary_get_int(&dev->dev, GAVL_V4L2_CAPABILITIES, &caps) &&
     (caps & V4L2_CAP_VIDEO_M2M_MPLANE))
    {
    dev->planar = 1;
    dev->buf_type_output = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    dev->buf_type_capture = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    
    //    fprintf(stderr, "Using planar API\n");

    fmt.type = dev->buf_type_output;
          
    //    fmt.fmt.pix_mp.width = gavl_format->image_width;
    //    fmt.fmt.pix_mp.height = gavl_format->image_height;

    fmt.fmt.pix_mp.pixelformat = gavl_v4l2_codec_id_to_pix_fmt(ci.id);
    fmt.fmt.pix_mp.colorspace = V4L2_COLORSPACE_DEFAULT;
    fmt.fmt.pix_mp.field      = V4L2_FIELD_NONE;
    
    fmt.fmt.pix_mp.num_planes = 1;

    fmt.fmt.pix_mp.plane_fmt[0].sizeimage = max_packet_size;
    //fmt.fmt.pix_mp.plane_fmt[0].bytesperline = 0;
    
    }
  else
    {
    dev->buf_type_output = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    dev->buf_type_capture = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    /* Untested */
    fmt.type = dev->buf_type_output;
    
    //    fmt.fmt.pix.width = gavl_format->image_width;
    //    fmt.fmt.pix.height = gavl_format->image_height;

    fmt.fmt.pix.pixelformat = gavl_v4l2_codec_id_to_pix_fmt(ci.id);
    fmt.fmt.pix.colorspace = V4L2_COLORSPACE_DEFAULT;
    fmt.fmt.pix.sizeimage = max_packet_size;
    fmt.fmt.pix.field      = V4L2_FIELD_NONE;
    }
  
  if(my_ioctl(dev->fd, VIDIOC_S_FMT, &fmt) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_S_FMT failed: %s", strerror(errno));
    goto fail;
    }
  
  
  if(!(dev->num_output_bufs = request_buffers_mmap(dev, dev->buf_type_output, DECODER_NUM_PACKETS, dev->output_bufs)))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Requesting buffers failed");
    goto fail;
    }

  packets_to_send = dev->num_output_bufs;
  
  /* */

  /* Queue header */
  if(!stream_on(dev, dev->buf_type_output))
    goto fail;
        
  if(ci.global_header)
    {
    gavl_packet_t * p;

#ifdef DUMP_EXTRADATA
    fprintf(stderr, "Sending global header %d bytes\n", ci.global_header_len);
    gavl_hexdump(ci.global_header, ci.global_header_len, 16);
#endif

    p = gavl_v4l2_device_get_packet_write(dev);
    memcpy(p->data, ci.global_header, ci.global_header_len);
    p->data_len = ci.global_header_len;
    
    if(gavl_v4l2_device_put_packet_write(dev) != GAVL_SINK_OK)
      goto fail;

    //    gavl_time_delay(&t);
    
    packets_to_send--;
    }
  
  dev->psrc = psrc;

#if 0  
  for(i = 0; i < packets_to_send; i++)
    {
    if(!send_decoder_packet(dev))
      fprintf(stderr, "sending decoder packet failed\n");
    }
#endif
  
  memset(&cropcap, 0, sizeof(cropcap));
  cropcap.type = dev->buf_type_output;
  
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

  memset(&fmt, 0, sizeof(fmt));
  
  fmt.type = dev->buf_type_capture;
  
  if(my_ioctl(dev->fd, VIDIOC_G_FMT, &fmt) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_G_FMT failed: %s", strerror(errno));
    goto fail;
    }
  
  //  dump_fmt(dev, &fmt);
  
  if(dev->planar)
    {
    gavl_format->pixelformat = gavl_v4l2_pix_fmt_to_pixelformat(fmt.fmt.pix_mp.pixelformat);
    }
  else
    {
    gavl_format->pixelformat = gavl_v4l2_pix_fmt_to_pixelformat(fmt.fmt.pix.pixelformat);
    }

  if(gavl_format->pixelformat == GAVL_PIXELFORMAT_NONE)
    {
    /* TODO: Negotiate format */
    goto fail;
    }

#if 0
  if(my_ioctl(dev->fd, VIDIOC_S_FMT, &fmt) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_S_FMT failed: %s", strerror(errno));
    goto fail;
    }
#endif
  
  
  /* Create frame buffers */

#if 0  
  if(!(dev->num_capture_bufs = request_buffers_mmap(dev, dev->buf_type_capture,
                                               DECODER_NUM_FRAMES, dev->capture_bufs)))
    goto fail;

  for(i = 0; i < dev->num_capture_bufs; i++)
    {
    queue_frame_decoder(dev, i);
    }
  
  if(!stream_on(dev, dev->buf_type_capture))
    goto fail;
#endif
  
  /* Wait for source_change event */

  while(!(dev->flags & DECODER_HAVE_FORMAT) && (format_packets < MAX_FORMAT_FRAMES))
    {
    do_poll(dev, POLLOUT | POLLPRI, &pollev);

    // fprintf(stderr, "Polled: %d %d %d\n", !!(pollev & POLLPRI), !!(pollev & POLLOUT), !!(pollev & POLLIN));
    
    if(pollev & POLLPRI)
      handle_decoder_event(dev);
    
    else if(pollev & POLLOUT)
      {
      if(!send_decoder_packet(dev))
        goto fail;

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
  
  dev->vframe = gavl_video_frame_create(NULL);

  gavl_video_format_copy(&dev->capture_format, gavl_format);
  
  ret = 1;
  fail:
  
  
  return ret;
  
  }

int gavl_v4l2_device_get_fd(gavl_v4l2_device_t * dev)
  {
  return dev->fd;
  }

void gavl_v4l2_device_close(gavl_v4l2_device_t * dev)
  {
  stream_off(dev, dev->buf_type_output);
  stream_off(dev, dev->buf_type_capture);
  
  release_buffers_mmap(dev, dev->buf_type_output, dev->num_output_bufs, dev->output_bufs);
  release_buffers_mmap(dev, dev->buf_type_capture, dev->num_capture_bufs, dev->capture_bufs);
  
  gavl_dictionary_free(&dev->dev);

  if(dev->vframe)
    {
    gavl_video_frame_null(dev->vframe);
    gavl_video_frame_destroy(dev->vframe);
    }

  if(dev->fd >= 0)
    close(dev->fd);
  
  free(dev);
  }


gavl_video_source_t * gavl_v4l2_device_get_video_source(gavl_v4l2_device_t * dev)
  {
  return dev->vsrc_priv;
  }


#if 0

gavl_packet_sink_t * gavl_v4l2_device_get_packet_sink(gavl_v4l2_device_t * dev)
  {

  }

gavl_packet_source_t * gavl_v4l2_device_get_packet_source(gavl_v4l2_device_t * dev)
  {

  }

gavl_video_sink_t * gavl_v4l2_device_get_video_sink(gavl_v4l2_device_t * dev)
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

void gavl_v4l2_devices_scan_by_type(int type_mask, gavl_array_t * ret)
  {
  int i;
  glob_t g;

  gavl_value_t dev_val;
  gavl_dictionary_t * dev;

  gavl_array_t * src_formats;
  gavl_array_t * sink_formats;
  struct v4l2_capability * cap;
  char * tmp_string;

  cap = calloc(1, sizeof(*cap));
  
  memset(&g, 0, sizeof(g));
  
  glob("/dev/video*", 0, NULL, &g);

  for(i = 0; i < g.gl_pathc; i++)
    {
    int fd;
    gavl_v4l2_device_type_t type;
    
    src_formats = NULL;
    sink_formats = NULL;
    
    if((fd = open(g.gl_pathv[i], O_RDWR /* required */ | O_NONBLOCK, 0)) < 0)
      continue;
    
    if(my_ioctl(fd, VIDIOC_QUERYCAP, cap) == -1)
      {
      close(fd);
      continue;
      } 

    gavl_value_init(&dev_val);
    dev = gavl_value_set_dictionary(&dev_val);

    //    fprintf(stderr, "Card:\n");
    //    gavl_hexdump(cap->card, 32, 16);
    
    //    fprintf(stderr, "Blupp 1 %s\n", (const char*)cap->card);
    
    gavl_dictionary_set_string(dev, GAVL_META_LABEL, (const char*)cap->card);

    //    fprintf(stderr, "Blupp 2 %s\n", g.gl_pathv[i]);

    tmp_string = gavl_strdup(g.gl_pathv[i]);
    
    //    fprintf(stderr, "Blupp 3 %s\n", tmp_string);
    
    gavl_dictionary_set_string_nocopy(dev, GAVL_META_URI, tmp_string);

    //    fprintf(stderr, "Blupp 4\n");
    
    gavl_dictionary_set_int(dev, GAVL_V4L2_CAPABILITIES, cap->capabilities);

    //    fprintf(stderr, "Blupp 5\n");
    
    /* Get source formats */
    if(cap->capabilities & (V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE))
      {
      src_formats = gavl_dictionary_get_array_create(dev, GAVL_V4L2_SRC_FORMATS);
      query_formats(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, src_formats);
      }
    else if(cap->capabilities & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_M2M))
      {
      src_formats = gavl_dictionary_get_array_create(dev, GAVL_V4L2_SRC_FORMATS);
      query_formats(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, src_formats);
      }
    
    /* Get output formats */
    if(cap->capabilities & (V4L2_CAP_VIDEO_OUTPUT_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE))
      {
      sink_formats = gavl_dictionary_get_array_create(dev, GAVL_V4L2_SINK_FORMATS);
      query_formats(fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, sink_formats);
      }
    else if(cap->capabilities & (V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_VIDEO_M2M))
      {
      sink_formats = gavl_dictionary_get_array_create(dev, GAVL_V4L2_SINK_FORMATS);
      query_formats(fd, V4L2_BUF_TYPE_VIDEO_OUTPUT, sink_formats);
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

    close(fd);
    
    if(type_mask && !(type & type_mask))
      {
      gavl_value_free(&dev_val);
      }
    else
      {
      gavl_array_splice_val_nocopy(ret, -1, 0, &dev_val);
      }
    
    }

  free(cap);
  
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

// #define V4L2_PIX_FMT_DV       v4l2_fourcc('d', 'v', 's', 'd') /* 1394          */
   { V4L2_PIX_FMT_JPEG, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_DV },

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


gavl_pixelformat_t gavl_v4l2_pix_fmt_to_pixelformat(uint32_t fmt)
  {
  int idx = 0;

  while(pixelformats[idx].v4l2)
    {
    if(pixelformats[idx].v4l2 == fmt)
      return pixelformats[idx].pixelformat;
    idx++;
    }
  return GAVL_PIXELFORMAT_NONE;
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

static int export_video_frame_v4l2(gavl_hw_context_t * ctx, gavl_video_format_t * fmt,
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

int gavl_v4l2_export_dmabuf_video(gavl_video_frame_t * frame)
  {
  int i;
  gavl_v4l2_device_t * dev = frame->hwctx->native;

  gavl_v4l2_buffer_t * buf = frame->storage;
  
  if(buf->flags & GAVL_V4L2_BUFFER_FLAG_DMA)
    return 1;
  
  for(i = 0; i < buf->num_planes; i++)
    {
    struct v4l2_exportbuffer expbuf;
    
    memset(&expbuf, 0, sizeof(expbuf));
    expbuf.type = buf->type;
    expbuf.index = buf->index;
    expbuf.plane = i;
    
    if(my_ioctl(dev->fd, VIDIOC_EXPBUF, &expbuf) == -1)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Exporting buffer failed: %s", strerror(errno));
      return 0;
      }
    
    buf->planes[i].dma_fd = expbuf.fd;
    }
  
  buf->flags |= GAVL_V4L2_BUFFER_FLAG_DMA;
  return 1;
  }
