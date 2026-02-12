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

/*
typedef struct
  {
  gavl_hw_buffer_t buffers[GAVL_MAX_PLANES];
  
  struct
    {
    int buf_idx;
    int64_t offset;
    } planes[GAVL_MAX_PLANES];
  
  int num_buffers;
  
  } gavl_video_frame_fds_t;
*/
  
/* */

typedef struct
  {
  gavl_hw_buffer_t buffers[GAVL_MAX_PLANES];
  int drm_handles[GAVL_MAX_PLANES];
  
  struct
    {
    int buf_idx;
    int64_t offset;
    int stride;
    uint64_t modifiers;
    } planes[GAVL_MAX_PLANES];
  
  uint32_t fourcc; // drm fourcc
  
  int num_buffers;
  int num_planes;
  
  } gavl_dmabuf_video_frame_t;

GAVL_PUBLIC gavl_hw_context_t * gavl_hw_ctx_create_dma(void);

GAVL_PUBLIC gavl_pixelformat_t gavl_drm_pixelformat_from_fourcc(uint32_t fourcc, int * flags, int * drm_indices);
GAVL_PUBLIC void gavl_hw_ctx_dma_set_supported_formats(gavl_hw_context_t * ctx, uint32_t * formats);

GAVL_PUBLIC gavl_hw_frame_mode_t gavl_hw_ctx_dma_get_frame_mode(uint32_t format);

#define GAVL_DMABUF_FLAG_SWAP_CHROMA (1<<0)
#define GAVL_DMABUF_FLAG_SHUFFLE     (1<<1)

#endif // GAVL_HW_DMABUF_H_INCLUDED
