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



#ifndef GAVL_HW_EGL_H_INCLUDED
#define GAVL_HW_EGL_H_INCLUDED

#include <EGL/egl.h>
#include <EGL/eglext.h>

// gavl_hw_context_t * gavl_hw_ctx_create_egl(EGLint const * attrs);

GAVL_PUBLIC gavl_hw_context_t *
gavl_hw_ctx_create_egl(EGLenum platform,
                       EGLint const * attrs, gavl_hw_type_t type, void * native_display);

GAVL_PUBLIC void gavl_hw_egl_swap_buffers(gavl_hw_context_t * ctx);

/* Returned value (zero terminated) must be free()d */
GAVL_PUBLIC 
uint32_t * gavl_hw_ctx_egl_get_dma_import_formats(gavl_hw_context_t * ctx);

GAVL_PUBLIC EGLSurface gavl_hw_ctx_egl_create_window_surface(gavl_hw_context_t * ctx, void * native_window);
GAVL_PUBLIC void gavl_hw_ctx_egl_destroy_surface(gavl_hw_context_t * ctx, EGLSurface surf);

GAVL_PUBLIC EGLDisplay gavl_hw_ctx_egl_get_egl_display(gavl_hw_context_t * ctx);

GAVL_PUBLIC void gavl_hw_egl_set_current(gavl_hw_context_t * ctx, EGLSurface surf);
GAVL_PUBLIC void gavl_hw_egl_unset_current(gavl_hw_context_t * ctx);

/* Wait until drawing operations are done */
GAVL_PUBLIC void gavl_hw_egl_wait(gavl_hw_context_t * ctx);

GAVL_PUBLIC int gavl_hw_egl_get_max_texture_size(gavl_hw_context_t * ctx);

#endif // GAVL_HW_EGL_H_INCLUDED
