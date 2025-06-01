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
   GAVL_V4L2_DEVICE_LOOPBACK  = (1<<5),
  } gavl_v4l2_device_type_t;

/* Video frames have a gavl_v4l2_buffer_t as storage */

typedef struct
  {
  int total; /* Total number of buffers */
  
  int flags;
  int bytesused; // For packets

  struct v4l2_buffer buf;
  struct v4l2_plane planes[GAVL_MAX_PLANES];

  /* mmapped pointers */
  void * ptrs[GAVL_MAX_PLANES];
  
  } gavl_v4l2_buffer_t;

typedef struct
  {
  struct v4l2_query_ext_ctrl ctrl;
  struct v4l2_querymenu * menu_items;
  int num_menu_items;
  } gavl_v4l2_control_t;


/*
 * Metadata tags for devices.
 * Also supported are:
 *
 * GAVL_META_LABEL
 * GAVL_META_URI
 *
 */

#define GAVL_V4L2_TYPE         "type"
#define GAVL_V4L2_TYPE_STRING  "typestr"
#define GAVL_V4L2_CAPABILITIES "caps"
#define GAVL_V4L2_DRIVER       "driver"

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


GAVL_PUBLIC void gavl_v4l2_device_close(gavl_v4l2_device_t * dev);

GAVL_PUBLIC int gavl_v4l2_device_get_fd(gavl_v4l2_device_t * dev);

GAVL_PUBLIC const gavl_v4l2_control_t *
gavl_v4l2_device_get_controls(gavl_v4l2_device_t * dev, int * num_controls);

GAVL_PUBLIC const gavl_dictionary_t *
gavl_v4l2_get_decoder(const gavl_array_t * arr, gavl_codec_id_t id, const gavl_dictionary_t * stream);

GAVL_PUBLIC gavl_packet_sink_t * gavl_v4l2_device_get_packet_sink(gavl_v4l2_device_t * dev);

GAVL_PUBLIC gavl_packet_source_t * gavl_v4l2_device_get_packet_source(gavl_v4l2_device_t * dev);
GAVL_PUBLIC gavl_video_sink_t * gavl_v4l2_device_get_video_sink(gavl_v4l2_device_t * dev);
GAVL_PUBLIC gavl_video_source_t * gavl_v4l2_device_get_video_source(gavl_v4l2_device_t * dev);

GAVL_PUBLIC int gavl_v4l_device_init_capture(gavl_v4l2_device_t * dev, gavl_dictionary_t * dict);
GAVL_PUBLIC int gavl_v4l_device_start_capture(gavl_v4l2_device_t * dev);

GAVL_PUBLIC int gavl_v4l2_device_init_output(gavl_v4l2_device_t * dev,
                                             gavl_video_format_t * fmt);

GAVL_PUBLIC int gavl_v4l2_device_init_decoder(gavl_v4l2_device_t * dev, gavl_dictionary_t * stream,
                                              gavl_packet_source_t * psrc);

GAVL_PUBLIC int gavl_v4l2_device_init_converter(gavl_v4l2_device_t * dev,
                                                gavl_video_format_t * in_format,
                                                gavl_video_format_t * out_format);

GAVL_PUBLIC void gavl_v4l2_device_resync_decoder(gavl_v4l2_device_t * dev);

GAVL_PUBLIC void gavl_v4l2_device_info(const char * dev);
GAVL_PUBLIC void gavl_v4l2_device_infos();

GAVL_PUBLIC gavl_hw_context_t * gavl_hw_ctx_create_v4l2(const gavl_dictionary_t * dev_info);

GAVL_PUBLIC gavl_v4l2_device_t * gavl_hw_ctx_v4l2_get_device(gavl_hw_context_t * ctx);

#endif // GAVL_HW_V4L2_H_INCLUDED
