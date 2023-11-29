#include <string.h>
#include <stdlib.h>

#include <config.h>


#include <gavl.h>
#include <utils.h>

#include "pngutil.h"

#include <log.h>
#define LOG_DOMAIN "orientationtest"

static const struct
  {
  gavl_image_orientation_t orient;
  const char * filename;
  }
images[] = 
  {
    { GAVL_IMAGE_ORIENT_NORMAL,       "orientation_1.png" }, // EXIF: 1
    { GAVL_IMAGE_ORIENT_ROT90_CW,     "orientation_8.png" }, // EXIF: 8
    { GAVL_IMAGE_ORIENT_ROT180_CW,    "orientation_3.png" }, // EXIF: 3
    { GAVL_IMAGE_ORIENT_ROT270_CW,    "orientation_6.png" }, // EXIF: 6
    { GAVL_IMAGE_ORIENT_FH,           "orientation_2.png" }, // EXIF: 2
    { GAVL_IMAGE_ORIENT_FH_ROT90_CW,  "orientation_7.png" }, // EXIF: 7
    { GAVL_IMAGE_ORIENT_FH_ROT180_CW, "orientation_4.png" }, // EXIF: 4
    { GAVL_IMAGE_ORIENT_FH_ROT270_CW, "orientation_5.png" }, // EXIF: 5
  };


int main(int argc, char ** argv)
  {
  int i;
  const char * end;
  char * tmp_string;
  char * filename;
  gavl_video_format_t in_format;
  gavl_video_format_t out_format;

  gavl_video_frame_t * in_frame;
  gavl_video_frame_t * out_frame;

  memset(&in_format, 0, sizeof(in_format));
  memset(&out_format, 0, sizeof(out_format));
  
  for(i = 0; i < 8; i++)
    {
    in_frame = read_png(images[i].filename, &in_format, GAVL_PIXELFORMAT_NONE);
    in_format.orientation = images[i].orient;

    gavl_video_format_normalize_orientation(&in_format, &out_format);
    out_frame = gavl_video_frame_create(&out_format);
    gavl_video_frame_normalize_orientation(&in_format, &out_format, in_frame, out_frame);
    
    end = strrchr(images[i].filename, '.');
    tmp_string = gavl_strndup(images[i].filename, end);
    filename = gavl_sprintf("%s_normalized.png", tmp_string);
    write_png(filename, &out_format, out_frame);
    
    gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Wrote %s", filename);
    
    free(filename);
    free(tmp_string);
    gavl_video_frame_destroy(in_frame);
    gavl_video_frame_destroy(out_frame);
    }
  
  }
