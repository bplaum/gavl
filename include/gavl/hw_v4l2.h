
#ifndef GAVL_HW_V4L2_H_INCLUDED
#define GAVL_HW_V4L2_H_INCLUDED

#include <sys/types.h>
#include <linux/videodev2.h>

#include <gavl/connectors.h>
#include <gavl/hw.h>


typedef enum
  {
   GAVL_V4L2_DEVICE_UNKNOWN   = 0,
   GAVL_V4L2_DEVICE_SOURCE    = (1<<0),
   GAVL_V4L2_DEVICE_SINK      = (1<<1),
   GAVL_V4L2_DEVICE_ENCODER   = (1<<2),
   GAVL_V4L2_DEVICE_DECODER   = (1<<3),
   GAVL_V4L2_DEVICE_CONVERTER = (1<<4),
  } gavl_v4l2_device_type_t;

/* Video frames have a gavl_v4l2_buffer_t as storage */

typedef struct
  {
  /* MMapped frame */
  void * buf;
  size_t size;   // For munmap
  } gavl_v4l2_plane_t;

typedef struct
  {
  int type;
  int index;
  int total; /* Total number of buffers */
  
  gavl_v4l2_plane_t planes[GAVL_MAX_PLANES];
  int num_planes;

  int flags;
  
  int bytesused; // For packets
  } gavl_v4l2_buffer_t;


/*
 * Metadata tags for devices.
 * Also supported are:
 *
 * GAVL_META_LABEL
 * GAVL_META_URI
 *
 */

#define GAVL_V4L2_TYPE       "type"
#define GAVL_V4L2_TYPE_STRING "typestr"
#define GAVL_V4L2_CAPABILITIES "caps"

#define GAVL_V4L2_SRC_FORMATS  "src_fmts"
#define GAVL_V4L2_SINK_FORMATS "sink_fmts"

#define GAVL_V4L2_FORMAT_V4L_PIX_FMT "pixfmt"
#define GAVL_V4L2_FORMAT_V4L_FLAGS   "flags"

#define GAVL_V4L2_FORMAT_GAVL_PIXELFORMAT "pixelformat"
#define GAVL_V4L2_FORMAT_GAVL_CODEC_ID    "codecid"

typedef struct gavl_v4l2_device_s gavl_v4l2_device_t;

GAVL_PUBLIC void gavl_v4l2_devices_scan(gavl_array_t * ret);
GAVL_PUBLIC void gavl_v4l2_devices_scan_by_type(int type_mask, gavl_array_t * ret);

GAVL_PUBLIC int gavl_v4l2_has_decoder(gavl_array_t * arr, gavl_codec_id_t id);

GAVL_PUBLIC gavl_codec_id_t gavl_v4l2_pix_fmt_to_codec_id(uint32_t fmt);
GAVL_PUBLIC gavl_pixelformat_t gavl_v4l2_pix_fmt_to_pixelformat(uint32_t fmt, int quantization);
GAVL_PUBLIC uint32_t gavl_v4l2_pixelformat_to_pix_fmt(gavl_pixelformat_t fmt, int * quantization);

GAVL_PUBLIC uint32_t gavl_v4l2_codec_id_to_pix_fmt(gavl_codec_id_t id);
GAVL_PUBLIC int gavl_v4l2_get_device_info(const char * path, gavl_dictionary_t * dev);


// GAVL_PUBLIC gavl_packet_t * gavl_v4l2_device_get_packet_write(gavl_v4l2_device_t * dev);
// GAVL_PUBLIC gavl_sink_status_t gavl_v4l2_device_put_packet_write(gavl_v4l2_device_t * dev);
// GAVL_PUBLIC gavl_source_status_t gavl_v4l2_device_read_frame(gavl_v4l2_device_t * dev, gavl_video_frame_t ** frame);


GAVL_PUBLIC void gavl_v4l2_device_close(gavl_v4l2_device_t * dev);

GAVL_PUBLIC int gavl_v4l2_device_get_fd(gavl_v4l2_device_t * dev);


GAVL_PUBLIC const gavl_dictionary_t * gavl_v4l2_get_decoder(const gavl_array_t * arr, gavl_codec_id_t id);


GAVL_PUBLIC gavl_packet_sink_t * gavl_v4l2_device_get_packet_sink(gavl_v4l2_device_t * dev);

GAVL_PUBLIC gavl_packet_source_t * gavl_v4l2_device_get_packet_source(gavl_v4l2_device_t * dev);
GAVL_PUBLIC gavl_video_sink_t * gavl_v4l2_device_get_video_sink(gavl_v4l2_device_t * dev);
GAVL_PUBLIC gavl_video_source_t * gavl_v4l2_device_get_video_source(gavl_v4l2_device_t * dev);

GAVL_PUBLIC int gavl_v4l_device_init_capture(gavl_v4l2_device_t * dev, gavl_dictionary_t * dict);
GAVL_PUBLIC int gavl_v4l_device_start_capture(gavl_v4l2_device_t * dev);


GAVL_PUBLIC int gavl_v4l2_device_init_decoder(gavl_v4l2_device_t * dev, gavl_dictionary_t * stream,
                                              gavl_packet_source_t * psrc);

GAVL_PUBLIC int gavl_v4l2_device_init_converter(gavl_v4l2_device_t * dev,
                                                const gavl_video_format_t * in_format,
                                                gavl_video_format_t * out_format);

GAVL_PUBLIC void gavl_v4l2_device_resync_decoder(gavl_v4l2_device_t * dev);

GAVL_PUBLIC void gavl_v4l2_device_info(const char * dev);
GAVL_PUBLIC void gavl_v4l2_device_infos();

GAVL_PUBLIC gavl_hw_context_t * gavl_hw_ctx_create_v4l2(const gavl_dictionary_t * dev_info);

GAVL_PUBLIC gavl_v4l2_device_t * gavl_hw_ctx_v4l2_get_device(gavl_hw_context_t * ctx);

#endif // GAVL_HW_V4L2_H_INCLUDED
