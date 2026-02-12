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




#ifndef GAVL_HW_MEMFD_H_INCLUDED
#define GAVL_HW_MEMFD_H_INCLUDED

typedef struct gavl_memfd_s gavl_memfd_t;

/* gavl memfd is always one buffer per frame. For uncompressed
   video frames, the values returned gavl_video_format_get_frame_layout()
   are used to specify the plane offsets and strides */


GAVL_PUBLIC gavl_hw_context_t * gavl_hw_ctx_create_memfd(void);

#endif // GAVL_HW_SHM_H_INCLUDED
