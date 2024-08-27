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



#ifndef GAVL_HW_VAAPI_X11_H_INCLUDED
#define GAVL_HW_VAAPI_X11_H_INCLUDED


#include <gavl/gavldefs.h>
#include <X11/Xlib.h>

/**
 * @file hw_vaapi_x11.h
 * external api header.
 */


GAVL_PUBLIC gavl_hw_context_t * gavl_hw_ctx_create_vaapi_x11(Display * dpy);

GAVL_PUBLIC Display * gavl_hw_ctx_vaapi_x11_get_display(gavl_hw_context_t *);
GAVL_PUBLIC VADisplay gavl_hw_ctx_vaapi_x11_get_va_display(gavl_hw_context_t *);

#endif // GAVL_HW_VAAPI_X11_H_INCLUDED
