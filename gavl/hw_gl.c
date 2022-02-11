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

#include <GL/gl.h>
#include <stdlib.h>
#include <stdio.h>

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/hw_gl.h>
#include <gavl/log.h>
#define LOG_DOMAIN "gl"

#include <hw_private.h>

/*

1) *egl_image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_IMAGE_BRCM_MULTIMEDIA, mmal_buffer_opaque, NULL);
2) glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, *egl_image);

*/


static const struct
  {
  const gavl_pixelformat_t fmt;
  const GLenum format;
  const GLenum type;
  
  }
pixelformats[] =
  {
    { GAVL_RGB_24,     GL_RGB,  GL_UNSIGNED_BYTE },
    { GAVL_RGBA_32,    GL_RGBA, GL_UNSIGNED_BYTE },
    { GAVL_RGB_48,     GL_RGB,  GL_SHORT },
    { GAVL_RGBA_64,    GL_RGBA, GL_SHORT },
    { GAVL_RGB_FLOAT,  GL_RGB,  GL_FLOAT },
    { GAVL_RGBA_FLOAT, GL_RGBA, GL_FLOAT },
    
    { GAVL_YUV_420_P,    GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUV_410_P,    GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUV_411_P,    GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUV_422_P,    GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUV_422_P_16, GL_RED,  GL_UNSIGNED_SHORT },
    { GAVL_YUV_444_P,    GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUV_444_P_16, GL_RED,  GL_UNSIGNED_SHORT },
    { GAVL_YUVJ_420_P,   GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUVJ_422_P,   GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUVJ_444_P,   GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_PIXELFORMAT_NONE    /* End */ },
  };

#define NUM_PIXELFORMATS (sizeof(pixelformats)/sizeof(pixelformats[0]))

int gavl_get_gl_format(gavl_pixelformat_t fmt, GLenum * format, GLenum * type)
  {
  int i = 0;

  while(pixelformats[i].fmt != GAVL_PIXELFORMAT_NONE)
    {
    if(pixelformats[i].fmt == fmt)
      {
      *format = pixelformats[i].format;
      *type   = pixelformats[i].type;
      return 1;
      }
    i++;
    }
  return 0;
  }

gavl_pixelformat_t * gavl_gl_get_image_formats(gavl_hw_context_t * ctx)
  {
  int idx = 0;
  
  gavl_pixelformat_t * ret;
  //  glx_t * priv = ctx->native;

  ret = calloc(NUM_PIXELFORMATS, sizeof(*ret));

  while(1)
    {
    ret[idx] = pixelformats[idx].fmt;
    if(pixelformats[idx].fmt == GAVL_PIXELFORMAT_NONE)
      break;
    idx++;
    }
  return ret;
  }

gavl_pixelformat_t * gavl_gl_get_overlay_formats(gavl_hw_context_t * ctx)
  {
  int idx1 = 0;
  int idx2 = 0;
  
  gavl_pixelformat_t * ret;
  //  glx_t * priv = ctx->native;

  ret = calloc(NUM_PIXELFORMATS, sizeof(*ret));

  while(1)
    {
    if(pixelformats[idx1].fmt == GAVL_PIXELFORMAT_NONE)
      break;
    
    if(gavl_pixelformat_has_alpha(pixelformats[idx1].fmt))
      {
      ret[idx2] = pixelformats[idx1].fmt;
      idx2++;
      }
    idx1++;
    }

  ret[idx2] = GAVL_PIXELFORMAT_NONE;
  
  return ret;
  }

void gavl_gl_adjust_video_format(gavl_hw_context_t * ctx,
                                 gavl_video_format_t * fmt)
  {
  gavl_video_format_set_frame_size(fmt, 8, 1);
  }

static int get_frame_planes(const gavl_video_frame_t * f)
  {
  int num_planes = 0;
  const GLuint * tex = f->user_data;
  
  while(tex[num_planes] != GL_NONE)
    num_planes++;
  return num_planes;
  }
                            
/* The following functions require a current GL context */
gavl_video_frame_t * gavl_gl_create_frame(const gavl_video_format_t * fmt)
  {
  int i;
  int num_planes = 1;
  GLenum type = 0, format = 0;
  GLuint * tex;
  GLint internalformat;

  GLsizei width = fmt->image_width;
  GLsizei height = fmt->image_height;

  gavl_video_frame_t * ret;
  
  ret = gavl_video_frame_create(NULL);
  
  tex = calloc(1, sizeof(*tex));

  
  if(!gavl_get_gl_format(fmt->pixelformat, &format, &type))
    return 0;
  
  num_planes = gavl_pixelformat_num_planes(fmt->pixelformat);

  if(num_planes > 1)
    internalformat = GL_RED;
  else
    internalformat = GL_RGBA;
  
  tex = calloc(num_planes+1, sizeof(*tex));
  
  glGenTextures(num_planes, tex);
  tex[num_planes] = GL_NONE;

  for(i = 0; i < num_planes; i++)
    {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, tex[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    gavl_gl_flush_errors();

    if(i == 1)
      {
      int sub_h, sub_v;
      gavl_pixelformat_chroma_sub(fmt->pixelformat,
                                  &sub_h, &sub_v);
      width /= sub_h;
      height /= sub_v;
      }
    
    glTexImage2D(GL_TEXTURE_2D, 0,
                 internalformat,
                 width,
                 height,
                 0,
                 format,
                 type,
                 NULL);
    
    gavl_gl_log_error("glTexImage2D");
    
    }
  
  
  ret->user_data = tex;
  return ret;
  }

void gavl_gl_destroy_frame(gavl_video_frame_t * f)
  {
  if(f->user_data)
    {
    int num_planes = get_frame_planes(f);
    GLuint * tex = f->user_data;
    glDeleteTextures(num_planes, tex);
    free(tex);
    }
  f->hwctx = NULL;
  gavl_video_frame_destroy(f);
  return;
  }

void gavl_gl_frame_to_ram(const gavl_video_format_t * fmt,
                          gavl_video_frame_t * dst,
                          const gavl_video_frame_t * src)
  {
  int i;
  GLenum type = 0, format = 0;
  int num_planes = get_frame_planes(src);
  const GLuint * tex = src->user_data;
  
  gavl_get_gl_format(fmt->pixelformat, &format, &type);
  
  for(i = 0; i < num_planes; i++)
    {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, tex[i]);

    if(num_planes == 1)
      {
      /* Implies i = 0 */
      glPixelStorei(GL_PACK_ROW_LENGTH, dst->strides[i] / gavl_pixelformat_bytes_per_pixel(fmt->pixelformat));
      }
    else
      {
      glPixelStorei(GL_PACK_ROW_LENGTH, dst->strides[i] / gavl_pixelformat_bytes_per_component(fmt->pixelformat));
      }
    
    glGetTexImage(GL_TEXTURE_2D, 0, format, type, dst->planes[i]);
    }
  
  }

void gavl_gl_frame_to_hw(const gavl_video_format_t * fmt,
                         gavl_video_frame_t * dst,
                         const gavl_video_frame_t * src)
  {
  int i;
  GLenum type = 0, format = 0;
  int num_planes = get_frame_planes(dst);
  GLuint * tex = dst->user_data;
  int width, height;
  
  gavl_get_gl_format(fmt->pixelformat, &format, &type);

  width = fmt->image_width;
  height = fmt->image_height;

  for(i = 0; i < num_planes; i++)
    {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, tex[i]);

    
    if(num_planes == 1)
      {
      /* Implies i = 0 */
      glPixelStorei(GL_UNPACK_ROW_LENGTH, src->strides[i] / gavl_pixelformat_bytes_per_pixel(fmt->pixelformat));
      }
    else
      {
      glPixelStorei(GL_UNPACK_ROW_LENGTH, src->strides[i] / gavl_pixelformat_bytes_per_component(fmt->pixelformat));
      }
    
    if(i == 1)
      {
      int sub_h, sub_v;
      gavl_pixelformat_chroma_sub(fmt->pixelformat,
                                  &sub_h, &sub_v);
      width /= sub_h;
      height /= sub_v;
      }
    //    fprintf(stderr, "glTexSubImage2D %d\n", i);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0, 0, 0, width, height,
                    format, type, src->planes[i]);
    gavl_gl_log_error("glTexSubImage2D");
    }
  
  }


static const struct
  {
  GLenum err;
  const char * msg;
  }
error_codes[] =
  {
    { GL_INVALID_ENUM,                  "Invalid enum"      },
    { GL_INVALID_VALUE,                 "Invalid value"     },
    { GL_INVALID_OPERATION,             "Invalid operation" },
    { GL_STACK_OVERFLOW,                "Stack overflow"    },
    { GL_STACK_UNDERFLOW,               "Stack underflow"   },
    { GL_OUT_OF_MEMORY,                 "Out of memory"     },
    { GL_INVALID_FRAMEBUFFER_OPERATION, "Invalid framebuffer operation" },
    { 0, NULL                          },
  };

const char * gavl_gl_get_error_string(GLenum err)
  {
  int idx = 0;

  if(!err)
    return "No error";
  
  while(error_codes[idx].msg)
    {
    if(error_codes[idx].err == err)
      return error_codes[idx].msg;
    idx++;
    }
  return "Unknown error";
  }

void gavl_gl_log_error(const char * funcname)
  {
  GLenum err;
  //  glFlush();
  while((err = glGetError()))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "%s failed: %s\n",
             funcname, gavl_gl_get_error_string(err));
    }
  
  }

void gavl_gl_flush_errors()
  {
  while((glGetError()))
    ;
  }

