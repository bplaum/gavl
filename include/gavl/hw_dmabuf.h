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


#ifndef GAVL_HW_DMABUF_H_INCLUDED
#define GAVL_HW_DMABUF_H_INCLUDED

/* */

typedef struct
  {
  struct
    {
    int fd;
    } buffers[GAVL_MAX_PLANES];
  
  struct
    {
    int buf_idx;
    int64_t offset;
    } planes[GAVL_MAX_PLANES];
  
  uint32_t fourcc; // drm fourcc
  
  int num_buffers;
  int num_planes;
  } gavl_dmabuf_video_frame_t;

GAVL_PUBLIC gavl_hw_context_t * gavl_hw_ctx_create_dma(void);


GAVL_PUBLIC uint32_t gavl_drm_fourcc_from_gavl(gavl_pixelformat_t pfmt);

#endif // GAVL_HW_DMABUF_H_INCLUDED
