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

#include <string.h>
#include <stdio.h>
#include <gavl/gavl.h>
#include <gavl/value.h>

void gavl_video_format_copy(gavl_video_format_t * dst,
                            const gavl_video_format_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  }

static void do_indent(int num)
  {
  int i;
  for(i = 0; i < num; i++)
    fprintf(stderr, " ");
  }

void gavl_video_format_dump(const gavl_video_format_t * format)
  {
  gavl_video_format_dumpi(format, 0);
  }

void gavl_video_format_dumpi(const gavl_video_format_t * format, int indent)
  {
  do_indent(indent);
  fprintf(stderr, "  Frame size:       %d x %d\n",
          format->frame_width, format->frame_height);
  do_indent(indent);
  fprintf(stderr, "  Image size:       %d x %d\n",
          format->image_width, format->image_height);
  do_indent(indent);
  fprintf(stderr, "  Pixel size:       %d x %d\n",
          format->pixel_width, format->pixel_height);
  do_indent(indent);
  fprintf(stderr, "  Pixel format:     %s\n",
          gavl_pixelformat_to_string(format->pixelformat));
  
  if(format->framerate_mode != GAVL_FRAMERATE_STILL)
    {
    if((!format->frame_duration) &&
       (format->framerate_mode == GAVL_FRAMERATE_VARIABLE))
      {
      do_indent(indent);
      fprintf(stderr, "  Framerate:        Variable (timescale: %d)\n",
              format->timescale);
      }
    else
      {
      do_indent(indent);
      fprintf(stderr, "  Framerate:        %f",
              (float)(format->timescale)/((float)format->frame_duration));
      fprintf(stderr, " [%d / %d]", format->timescale,
              format->frame_duration);
      fprintf(stderr, " fps");
    
      if(format->framerate_mode == GAVL_FRAMERATE_CONSTANT)
        fprintf(stderr, " (Constant)\n");
      else
        fprintf(stderr, " (Not constant)\n");
      }
    }
  else
    {
    do_indent(indent);
    fprintf(stderr, "  Still image\n");
    }
  do_indent(indent);
  fprintf(stderr, "  Interlace mode:   %s\n",
          gavl_interlace_mode_to_string(format->interlace_mode));  

  if((format->pixelformat == GAVL_YUV_420_P) ||
     (format->pixelformat == GAVL_YUVJ_420_P))
    {
    do_indent(indent);
    fprintf(stderr, "  Chroma placement: %s\n",
            gavl_chroma_placement_to_string(format->chroma_placement));
    }
  if(format->timecode_format.int_framerate)
    {
    do_indent(indent);
    fprintf(stderr, "  Timecode framerate: %d\n", format->timecode_format.int_framerate);
    do_indent(indent);
    fprintf(stderr, "  Timecode flags:     ");
    if(format->timecode_format.flags & GAVL_TIMECODE_DROP_FRAME)
      fprintf(stderr, "Drop");
    fprintf(stderr, "\n");
    }
  fprintf(stderr, "  orientation:     %s\n", gavl_image_orientation_to_string(format->orientation));
  
  if(format->hwctx)
    fprintf(stderr, "  Frame storage:    %s\n", gavl_hw_type_to_string(gavl_hw_ctx_get_type(format->hwctx)));
  }

/* We always enlarge the image */

void gavl_video_format_fit_to_source(gavl_video_format_t * dst,
                                     const gavl_video_format_t * src)
  {
  int f_1, f_2;
  f_1 = src->pixel_width * dst->pixel_height;
  f_2 = dst->pixel_width * src->pixel_height;
  
  if(f_1 > f_2) /* Make dst wider */
    {
    dst->image_width =
      (src->image_width * f_1) / f_2;
    dst->image_height = src->image_height;
    }
  else if(f_1 < f_2) /* Make dst higher */
    {
    dst->image_height =
      (src->image_height * f_2) / f_1;
    dst->image_width = src->image_width;
    }
  else
    {
    dst->image_width = src->image_width;
    dst->image_height = src->image_height;
    }
  }

typedef const struct
  {
  gavl_interlace_mode_t mode;
  const char * name;
  const char * short_name;
  } interlace_mode_tab_t;

interlace_mode_tab_t interlace_mode_tab[] =
  {
    { GAVL_INTERLACE_UNKNOWN,      "Unknown", "unknown" },
    { GAVL_INTERLACE_NONE,         "None (Progressive)", "p" },
    { GAVL_INTERLACE_TOP_FIRST,    "Top field first", "t" },
    { GAVL_INTERLACE_BOTTOM_FIRST, "Bottom field first", "b" },
    { GAVL_INTERLACE_MIXED,        "Mixed", "mixed" },
    { GAVL_INTERLACE_MIXED_TOP,    "Top first + progressive", "t+p"  },
    { GAVL_INTERLACE_MIXED_BOTTOM, "Bottom first + progressive", "b+p" }
  };

static const int num_interlace_modes =
  sizeof(interlace_mode_tab)/sizeof(interlace_mode_tab[0]);

const char * gavl_interlace_mode_to_string(gavl_interlace_mode_t mode)
  {
  int i;
  for(i = 0; i < num_interlace_modes; i++)
    {
    if(interlace_mode_tab[i].mode == mode)
      return interlace_mode_tab[i].name;
    }
  return NULL;
  }

const char * gavl_interlace_mode_to_short_string(gavl_interlace_mode_t mode)
  {
  int i;
  for(i = 0; i < num_interlace_modes; i++)
    {
    if(interlace_mode_tab[i].mode == mode)
      return interlace_mode_tab[i].name;
    }
  return interlace_mode_tab[0].short_name;
  }

gavl_interlace_mode_t gavl_short_string_to_interlace_mode(const char * mode)
  {
  int i;
  for(i = 0; i < num_interlace_modes; i++)
    {
    if(!strcmp(interlace_mode_tab[i].short_name, mode))
      return interlace_mode_tab[i].mode;
    }
  return interlace_mode_tab[0].mode;
  }

int gavl_interlace_mode_is_mixed(gavl_interlace_mode_t mode)
  {
  return !!(mode & 0x10);
  }


typedef const struct
  {
  gavl_framerate_mode_t mode;
  const char * name;
  const char * short_name;
  } framerate_mode_tab_t;

framerate_mode_tab_t framerate_mode_tab[] =
  {
    { GAVL_FRAMERATE_UNKNOWN,      "Unknown",   "unknown" },
    { GAVL_FRAMERATE_CONSTANT,     "Constant" , "constant"},
    { GAVL_FRAMERATE_VARIABLE,     "Variable",  "vfr" },
    { GAVL_FRAMERATE_STILL,        "Still",     "still"  },
  };

static const int num_framerate_modes = sizeof(framerate_mode_tab)/sizeof(framerate_mode_tab[0]);

const char * gavl_framerate_mode_to_string(gavl_framerate_mode_t mode)
  {
  int i;
  for(i = 0; i < num_framerate_modes; i++)
    {
    if(framerate_mode_tab[i].mode == mode)
      return framerate_mode_tab[i].name;
    }
  return NULL;
  }

const char * gavl_framerate_mode_to_short_string(gavl_framerate_mode_t mode)
  {
  int i;
  for(i = 0; i < num_framerate_modes; i++)
    {
    if(framerate_mode_tab[i].mode == mode)
      return framerate_mode_tab[i].short_name;
    }
  return framerate_mode_tab[0].short_name;
  }

gavl_framerate_mode_t gavl_short_string_to_framerate_mode(const char * mode)
  {
  int i;
  for(i = 0; i < num_framerate_modes; i++)
    {
    if(!strcmp(framerate_mode_tab[i].short_name, mode))
      return framerate_mode_tab[i].mode;
    }
  return framerate_mode_tab[0].mode;
  }

typedef struct
  {
  gavl_chroma_placement_t mode;
  char * name;
  char * short_name;
  } chroma_placement_tab_t;

const chroma_placement_tab_t chroma_placement_tab[] =
  {
    { GAVL_CHROMA_PLACEMENT_DEFAULT, "MPEG-1/JPEG", "mpeg1" },
    { GAVL_CHROMA_PLACEMENT_MPEG2, "MPEG-2",        "mpeg2" },
    { GAVL_CHROMA_PLACEMENT_DVPAL, "DV PAL",        "dvpal"  }
  };

static const int num_chroma_placements = sizeof(chroma_placement_tab)/sizeof(chroma_placement_tab_t);

const char * gavl_chroma_placement_to_string(gavl_chroma_placement_t mode)
  {
  int i;
  for(i = 0; i < num_chroma_placements; i++)
    {
    if(chroma_placement_tab[i].mode == mode)
      return chroma_placement_tab[i].name;
    }
  return NULL;
  }

const char * gavl_chroma_placement_to_short_string(gavl_chroma_placement_t mode)
  {
  int i;
  for(i = 0; i < num_chroma_placements; i++)
    {
    if(chroma_placement_tab[i].mode == mode)
      return chroma_placement_tab[i].short_name;
    }
  return chroma_placement_tab[0].short_name;
  }

gavl_chroma_placement_t gavl_short_string_to_chroma_placement(const char * mode)
  {
  int i;
  for(i = 0; i < num_chroma_placements; i++)
    {
    if(!strcmp(chroma_placement_tab[i].short_name, mode))
      return chroma_placement_tab[i].mode;
    }
  return chroma_placement_tab[0].mode;
  }

void gavl_video_format_get_chroma_offset(const gavl_video_format_t * format,
                                         int field, int plane,
                                         float * off_x, float * off_y)
  {
  int sub_h, sub_v;
  if(!plane)
    {
    *off_x = 0.0;
    *off_y = 0.0;
    return;
    }
  gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);

  if((sub_h != 2) || (sub_v != 2))
    {
    *off_x = 0.0;
    *off_y = 0.0;
    return;
    }

  switch(format->chroma_placement)
    {
    case GAVL_CHROMA_PLACEMENT_DEFAULT:
      *off_x = 0.5;
      *off_y = 0.5;
      break;
    case GAVL_CHROMA_PLACEMENT_MPEG2:
      if(format->interlace_mode == GAVL_INTERLACE_NONE)
        {
        *off_x = 0.0;
        *off_y = 0.5;
        }
      else if(field == 0) /* Top field */
        {
        *off_x = 0.0;
        *off_y = 0.25; /* In FIELD coordinates */
        //        *off_y = 0.5; /* In FRAME coordinates */
        }
      else /* Bottom field */
        {
        *off_x = 0.0;
        *off_y = 0.75; /* In FIELD coordinates */
        //        *off_y = 1.5; /* In FRAME coordinates */
        }
      break;
    case GAVL_CHROMA_PLACEMENT_DVPAL:
      if(plane == 1) /* Cb */
        {
        *off_x = 0.0;
        *off_y = 1.0; /* In FIELD coordinates */
        }
      else           /* Cr */
        {
        *off_x = 0.0;
        *off_y = 0.0; /* In FIELD coordinates */
        }
      break;
    }
  
  }

int gavl_video_formats_equal(const gavl_video_format_t * format_1,
                              const gavl_video_format_t * format_2)
  {
  return !memcmp(format_1, format_2, sizeof(*format_1));
  }

void gavl_video_format_set_frame_size(gavl_video_format_t * format,
                                      int pad_h, int pad_v)
  {
  int sub_h = 1, sub_v = 1;
  if(format->pixelformat != GAVL_PIXELFORMAT_NONE)
    gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
  
  if((pad_h < 1) || (pad_v < 1))
    {
    if(pad_h < 1)
      pad_h = sub_h;
    if(pad_v < 1)
      pad_v = sub_v;
    }
  else
    {
    pad_h *= sub_h;
    pad_v *= sub_v;
    }
  
  format->frame_width  = ((format->image_width  + pad_h - 1) / pad_h) * pad_h;
  format->frame_height = ((format->image_height + pad_v - 1) / pad_v) * pad_v;
  }

int gavl_video_format_get_image_size(const gavl_video_format_t * format)
  {
  int i;
  int bytes_per_line;
  int sub_h, sub_v;
  int ret = 0, height;
  
  int num_planes = gavl_pixelformat_num_planes(format->pixelformat);
  bytes_per_line = (num_planes > 1) ?
    format->frame_width * gavl_pixelformat_bytes_per_component(format->pixelformat) :
    format->frame_width * gavl_pixelformat_bytes_per_pixel(format->pixelformat);
  
  gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);

  height = format->frame_height;
  
  for(i = 0; i < num_planes; i++)
    {
    ret += bytes_per_line * height;

    if(!i)
      {
      bytes_per_line /= sub_h;
      height /= sub_v;
      }
    }
  return ret;
  }

void gavl_get_field_format(const gavl_video_format_t * frame_format,
                           gavl_video_format_t * field_format,
                           int field)
  {
  gavl_video_format_copy(field_format, frame_format);

  field_format->image_height /= 2;
  field_format->frame_height /= 2;

  if(frame_format->image_height % 2)
    {
    /* Top field gets an extra scanline */
    if(!field)
      {
      field_format->image_height++;
      if(field_format->frame_height < field_format->image_height)
        field_format->frame_height = field_format->image_height;
      }
    }
  }

#define PUT_INT(member) \
  gavl_value_set_int(&val, fmt->member); \
  gavl_dictionary_set_nocopy(dict, #member, &val)


#define PUT_STRING(member, str) \
  gavl_value_set_string(&val, str); \
  gavl_dictionary_set_nocopy(dict, #member, &val)


void gavl_video_format_to_dictionary(const gavl_video_format_t * fmt, gavl_dictionary_t * dict)
  {
  gavl_value_t val;
  gavl_value_init(&val);

  PUT_INT(image_width);
  PUT_INT(image_height);
  PUT_INT(frame_width);
  PUT_INT(frame_height);
  PUT_INT(pixel_width);
  PUT_INT(pixel_height);
  PUT_INT(frame_duration);
  PUT_INT(timescale);

  PUT_STRING(pixelformat, gavl_pixelformat_to_short_string(fmt->pixelformat));
  PUT_STRING(interlace_mode, gavl_interlace_mode_to_short_string(fmt->interlace_mode));
  PUT_STRING(framerate_mode, gavl_framerate_mode_to_short_string(fmt->framerate_mode));
  PUT_STRING(chroma_placement, gavl_chroma_placement_to_short_string(fmt->chroma_placement));

  if(fmt->timecode_format.int_framerate)
    {
    gavl_value_t child;
    gavl_dictionary_t * timecode_format = gavl_value_set_dictionary(&val);
    gavl_value_init(&child);
    
    gavl_value_set_int(&child, fmt->timecode_format.int_framerate);
    gavl_dictionary_set_nocopy(timecode_format, "int_framerate", &child);

    if(fmt->timecode_format.flags)
      {
      gavl_value_set_int(&child, fmt->timecode_format.flags);
      gavl_dictionary_set_nocopy(timecode_format, "flags", &val);
      }
    }
  }

#define GET_INT(member) \
  if(!(val = gavl_dictionary_get(dict, #member)) || \
     !gavl_value_get_int(val, &val_i)) \
    goto fail; \
  fmt->member = val_i;

#define GET_ENUM(member) \
  if(!(str = gavl_dictionary_get_string(dict, #member))) \
    goto fail; \
  fmt->member = gavl_short_string_to_##member (str);


int gavl_video_format_from_dictionary(gavl_video_format_t * fmt, const gavl_dictionary_t * dict)
  {
  int ret = 0; 
  int val_i;
  const gavl_value_t * val;
  const char * str;
  
  GET_INT(image_width);
  GET_INT(image_height);
  GET_INT(frame_width);
  GET_INT(frame_height);
  GET_INT(pixel_width);
  GET_INT(pixel_height);
  GET_INT(frame_duration);
  GET_INT(timescale);

  GET_ENUM(pixelformat);
  GET_ENUM(interlace_mode);
  GET_ENUM(framerate_mode);
  GET_ENUM(chroma_placement);

  if((val = gavl_dictionary_get(dict, "timecode_format")))
    {
    const gavl_value_t * child;
    const gavl_dictionary_t * timecode_format = gavl_value_get_dictionary(val);
    
    if(!timecode_format)
      goto fail;

    if(!(child = gavl_dictionary_get(timecode_format, "int_framerate")) ||
       !gavl_value_get_int(child, &val_i))
      goto fail;
    fmt->timecode_format.int_framerate = val_i;

    if((child = gavl_dictionary_get(timecode_format, "flags")))
      {
      if(!gavl_value_get_int(child, &val_i))
        goto fail;
      fmt->timecode_format.flags = val_i;
      }
    }

  ret = 1;
  fail:
  return ret;
  }
