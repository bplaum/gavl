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



typedef struct gavl_v4l_device_s gavl_v4l_device_t;

gavl_v4l_device_t * gavl_v4l_device_open(const char * dev);

int gavl_v4l_device_reads_video(gavl_v4l_device_t *);
int gavl_v4l_device_writes_video(gavl_v4l_device_t *);

int gavl_v4l_device_reads_packets(gavl_v4l_device_t *);
int gavl_v4l_device_writes_packets(gavl_v4l_device_t *);

gavl_packet_sink_t * gavl_v4l_device_get_packet_sink(gavl_v4l_device_t * dev);
gavl_packet_source_t * gavl_v4l_device_get_packet_source(gavl_v4l_device_t * dev);
gavl_video_sink_t * gavl_v4l_device_get_video_sink(gavl_v4l_device_t * dev);
gavl_video_source_t * gavl_v4l_device_get_video_source(gavl_v4l_device_t * dev);
 

int v4l_device_init_capture(gavl_v4l_device_t * dev,
                            gavl_video_format_t * fmt);

int v4l_device_init_output(gavl_v4l_device_t * dev,
                           gavl_video_format_t * fmt);

int v4l_device_init_encoder(gavl_v4l_device_t * dev,
                            gavl_video_format_t * fmt,
                            gavl_compression_info_t * cmp);

int v4l_device_init_decoder(gavl_v4l_device_t * dev,
                            gavl_video_format_t * fmt,
                            const gavl_compression_info_t * cmp);

