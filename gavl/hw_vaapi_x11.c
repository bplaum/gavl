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



#include <stdlib.h>

#include <X11/Xlib.h>
#include <va/va_x11.h>

#include <gavl/gavl.h>
#include <gavl/hw_vaapi_x11.h>

#include <hw_private.h>
#include <vaapi.h>


typedef struct
  {
  gavl_hw_vaapi_t va;
  
  Display * private_display;
  Display * display;
  } vaapi_x11_t;

static void destroy_native(void * native)
  {
  vaapi_x11_t * priv = native;
  gavl_vaapi_cleanup(native);
  if(priv->private_display)
    XCloseDisplay(priv->private_display);
  free(priv);
  }

static const gavl_hw_funcs_t funcs =
  {
    .destroy_native = destroy_native,
    .get_image_formats = gavl_vaapi_get_image_formats,
    .get_overlay_formats = gavl_vaapi_get_overlay_formats,
    .video_frame_create_hw = gavl_vaapi_video_frame_create_hw,
    .video_frame_create_ram = gavl_vaapi_video_frame_create_ram,
    .video_frame_create_ovl = gavl_vaapi_video_frame_create_ovl,
    .video_frame_destroy = gavl_vaapi_video_frame_destroy,
    .video_frame_to_ram = gavl_vaapi_video_frame_to_ram,
    .video_frame_to_hw  = gavl_vaapi_video_frame_to_hw,
    .video_format_adjust  = gavl_vaapi_video_format_adjust,
    
  };

gavl_hw_context_t * gavl_hw_ctx_create_vaapi_x11(Display * dpy)
  {
  int major, minor;
  vaapi_x11_t * priv;
  VAStatus result;
  int support_flags = GAVL_HW_SUPPORTS_VIDEO;
  
  priv = calloc(1, sizeof(*priv));

  if(dpy)
    {
    priv->display = dpy;
    }
  else
    {
    priv->private_display = XOpenDisplay(NULL);

    if(!priv->private_display)
      return NULL;
    
    priv->display = priv->private_display;
    }
  
  priv->va.dpy = vaGetDisplay(priv->display);
  priv->va.no_derive = 1;
  
  if(!priv->va.dpy)
    return NULL;
  
  if((result = vaInitialize(priv->va.dpy,
                            &major,
                            &minor)) != VA_STATUS_SUCCESS)
    {
    return NULL;
    }

  return gavl_hw_context_create_internal(priv, &funcs,
                                         GAVL_HW_VAAPI_X11, support_flags);
  }

Display * gavl_hw_ctx_vaapi_x11_get_display(gavl_hw_context_t * ctx)
  {
  vaapi_x11_t * p = ctx->native;
  return p->display;
  }

VADisplay gavl_hw_ctx_vaapi_x11_get_va_display(gavl_hw_context_t * ctx)
  {
  vaapi_x11_t * p = ctx->native;
  return p->va.dpy;
  }

