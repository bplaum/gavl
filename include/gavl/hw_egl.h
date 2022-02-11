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


#include <EGL/egl.h>

#ifdef HAVE_XLIB
#include <X11/Xlib.h>
#endif

// gavl_hw_context_t * gavl_hw_ctx_create_egl(EGLint const * attrs);

GAVL_PUBLIC gavl_hw_context_t * gavl_hw_ctx_create_egl(EGLint const * attrs, gavl_hw_type_t type, void * native_display);
GAVL_PUBLIC void * gavl_hw_ctx_egl_get_native_display(gavl_hw_context_t * ctx);

GAVL_PUBLIC void gavl_hw_egl_swap_buffers(gavl_hw_context_t * ctx);

GAVL_PUBLIC EGLSurface gavl_hw_ctx_egl_create_window_surface(gavl_hw_context_t * ctx, void * native_window);
GAVL_PUBLIC void gavl_hw_ctx_egl_destroy_surface(gavl_hw_context_t * ctx, EGLSurface surf);

GAVL_PUBLIC EGLDisplay gavl_hw_ctx_egl_get_egl_display(gavl_hw_context_t * ctx);

GAVL_PUBLIC void gavl_hw_egl_set_current(gavl_hw_context_t * ctx, EGLSurface surf);
GAVL_PUBLIC void gavl_hw_egl_unset_current(gavl_hw_context_t * ctx);

/* Works only with OpenGL ES contexts! */

GAVL_PUBLIC int gavl_hw_egl_import_v4l2_buffer(gavl_hw_context_t * ctx,
                                               const gavl_video_format_t * fmt,
                                               GLuint texture,
                                               gavl_video_frame_t * v4l2_frame);

