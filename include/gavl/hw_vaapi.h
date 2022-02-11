/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2012 Members of the Gmerlin project
 * gmerlin-general@lists.sourceforge.net
 * http://gmerlin.sourceforge.net
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

/* VA-API specific backend functions */

GAVL_PUBLIC VAImageID   gavl_vaapi_get_image_id(const gavl_video_frame_t *);

GAVL_PUBLIC VASurfaceID gavl_vaapi_get_surface_id(const gavl_video_frame_t *);

/* Use only in specific create routines */
GAVL_PUBLIC void gavl_vaapi_set_surface_id(gavl_video_frame_t *, VASurfaceID id);

GAVL_PUBLIC VASubpictureID gavl_vaapi_get_subpicture_id(const gavl_video_frame_t *);

/* Map into userspace for CPU access */
GAVL_PUBLIC void gavl_vaapi_map_frame(gavl_video_frame_t *);

/* Unmap */
GAVL_PUBLIC void gavl_vaapi_unmap_frame(gavl_video_frame_t *);

GAVL_PUBLIC void gavl_vaapi_video_frame_swap_bytes(const gavl_video_format_t * fmt,
                                                   gavl_video_frame_t * f,
                                                   int to_gavl);
