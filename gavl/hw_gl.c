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



#include <GL/gl.h>
#include <stdlib.h>
#include <stdio.h>

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/hw_gl.h>
#include <gavl/log.h>
#define LOG_DOMAIN "gl"

#include <hw_private.h>


static const struct
  {
  const gavl_pixelformat_t fmt;
  const GLenum format;
  const GLenum internalformat;
  const GLenum type;
  
  }
pixelformats[] =
  {
    { GAVL_RGB_24,       GL_RGB,  GL_RGB,  GL_UNSIGNED_BYTE },
    { GAVL_RGBA_32,      GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE },
    { GAVL_RGB_48,       GL_RGB,  GL_RGB,  GL_UNSIGNED_SHORT },
    { GAVL_RGBA_64,      GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT },
    { GAVL_RGB_FLOAT,    GL_RGB,  GL_RGB,  GL_FLOAT },
    { GAVL_RGBA_FLOAT,   GL_RGBA, GL_RGBA, GL_FLOAT },
    { GAVL_YUV_420_P,    GL_RED,  GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUV_410_P,    GL_RED,  GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUV_411_P,    GL_RED,  GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUV_422_P,    GL_RED,  GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUV_422_P_16, GL_RED,  GL_RED,  GL_UNSIGNED_SHORT },
    { GAVL_YUV_444_P,    GL_RED,  GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUV_444_P_16, GL_RED,  GL_RED,  GL_UNSIGNED_SHORT },
    { GAVL_YUVJ_420_P,   GL_RED,  GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUVJ_422_P,   GL_RED,  GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_YUVJ_444_P,   GL_RED,  GL_RED,  GL_UNSIGNED_BYTE  },
    { GAVL_PIXELFORMAT_NONE    /* End */ },
  };

#define NUM_PIXELFORMATS (sizeof(pixelformats)/sizeof(pixelformats[0]))

int gavl_get_gl_format(gavl_pixelformat_t fmt, GLenum * format, GLenum * internalformat, GLenum * type)
  {
  int i = 0;

  while(pixelformats[i].fmt != GAVL_PIXELFORMAT_NONE)
    {
    if(pixelformats[i].fmt == fmt)
      {
      if(format)
        *format = pixelformats[i].format;

      if(type)
        *type   = pixelformats[i].type;
      
      if(internalformat)
        *internalformat = pixelformats[i].internalformat;
      
      return 1;
      }
    i++;
    }
  return 0;
  }

gavl_pixelformat_t * gavl_gl_get_image_formats(gavl_hw_context_t * ctx, int * num)
  {
  int src_idx = 0;
  int dst_idx = 0;
  
  gavl_pixelformat_t * ret;
  //  glx_t * priv = ctx->native;

  ret = calloc(NUM_PIXELFORMATS, sizeof(*ret));

  while(pixelformats[src_idx].fmt)
    {
    /* Seems that GL ES supports no 16 bit colors */
    if((ctx->type == GAVL_HW_EGL_GLES_X11) &&
       (pixelformats[src_idx].type != GL_UNSIGNED_BYTE))
      {
      src_idx++;
      continue;
      }
    ret[dst_idx] = pixelformats[src_idx].fmt;
    
    src_idx++;
    dst_idx++;

    }
  ret[dst_idx] = GAVL_PIXELFORMAT_NONE;
  if(num)
    *num = dst_idx;
  return ret;
  }


void gavl_gl_adjust_video_format(gavl_hw_context_t * ctx,
                                 gavl_video_format_t * fmt)
  {
  gavl_video_format_set_frame_size(fmt, 8, 1);
  }

static int get_frame_planes(const gavl_video_frame_t * f)
  {
  const gavl_gl_frame_info_t * info = f->storage;
  return info->num_textures;
  }
                            
/* The following functions require a current GL context */
gavl_video_frame_t * gavl_gl_create_frame(const gavl_video_format_t * fmt)
  {
  int i;
  GLenum type = 0, format = 0;
  GLenum internalformat;

  GLsizei width;
  GLsizei height;

  gavl_video_frame_t * ret;
  gavl_gl_frame_info_t * info = calloc(1, sizeof(*info));
  
  ret = gavl_video_frame_create(NULL);
  ret->storage = info;
  
  if(!fmt)
    return ret;

  width = fmt->image_width;
  height = fmt->image_height;

  
  if(!gavl_get_gl_format(fmt->pixelformat, &format, &internalformat, &type))
    return 0;
  
  info->num_textures = gavl_pixelformat_num_planes(fmt->pixelformat);
  info->texture_target = GL_TEXTURE_2D;
  
  glGenTextures(info->num_textures, info->textures);

  for(i = 0; i < info->num_textures; i++)
    {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, info->textures[i]);
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
    
    if(gavl_gl_log_error("glTexImage2D"))
      {
      GLint val;
      fprintf(stderr, "glTexImage2D failed\n");
      gavl_video_format_dump(fmt);
      glGetIntegerv(GL_MAX_TEXTURE_SIZE, &val);
      fprintf(stderr, "Maximum texture size: %d\n", val);
      
      fprintf(stderr, "internalformat: 0x%08x\n", internalformat);
      fprintf(stderr, "width:          %d\n", width);
      fprintf(stderr, "height:         %d\n", height);
      fprintf(stderr, "format:         0x%08x\n", format);
      fprintf(stderr, "type:           0x%08x\n", type);
      }
    
    }
  return ret;
  }

void gavl_gl_destroy_frame(gavl_video_frame_t * f, int destroy_resource)
  {
  gavl_gl_frame_info_t * info;

  if(f->storage)
    {
    info = f->storage;
    if(destroy_resource)
      glDeleteTextures(info->num_textures, info->textures);
    free(info);
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
  gavl_gl_frame_info_t * info;

  info = src->storage;
  
  gavl_get_gl_format(fmt->pixelformat, &format, NULL, &type);
  
  for(i = 0; i < info->num_textures; i++)
    {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, info->textures[i]);

    if(info->num_textures == 1)
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
  int width, height;

  gavl_gl_frame_info_t * info = dst->storage;
  
  gavl_get_gl_format(fmt->pixelformat, &format, NULL, &type);

  width = fmt->image_width;
  height = fmt->image_height;

  for(i = 0; i < num_planes; i++)
    {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, info->textures[i]);

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

    if(gavl_gl_log_error("glTexSubImage2D"))
      {
      fprintf(stderr, "glTexSubImage2D failed\n");
      gavl_video_format_dump(fmt);
      }
    
    }
  gavl_video_frame_copy_metadata(dst, src);
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

int gavl_gl_log_error(const char * funcname)
  {
  int ret = 0;
  GLenum err;
  //  glFlush();
  while((err = glGetError()))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "%s failed: %s",
             funcname, gavl_gl_get_error_string(err));
    ret = 1;
    }
  return ret;
  }

void gavl_gl_flush_errors()
  {
  while((glGetError()))
    ;
  }

