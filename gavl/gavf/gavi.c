#include <stdlib.h>
#include <string.h>


#include <gavfprivate.h>

/*
 *  Layout of a gavi file:
 *
 *  GAVLIMAG: Signature
 *  metadata
 *  format
 *  frame
 */

static const uint8_t sig[8] = "GAVLIMAG";

int gavl_image_write_header(gavf_io_t * io,
                            const gavl_dictionary_t * m,
                            const gavl_video_format_t * v)
  {
  if((gavf_io_write_data(io, sig, 8) < 8))
    return 0;

  if(m)
    {
    if(!gavl_dictionary_write(io, m))
      return 0;
    }
  else
    {
    gavl_dictionary_t m1;
    gavl_dictionary_init(&m1);
    if(!gavl_dictionary_write(io, &m1))
      return 0;
    }
  
  if(!gavf_write_video_format(io, v))
    return 0;
  return 1;
  }

int gavl_image_write_image(gavf_io_t * io,
                           const gavl_video_format_t * v,
                           gavl_video_frame_t * f)
  {
  int len = gavl_video_format_get_image_size(v);
  if((gavf_io_write_data(io, f->planes[0], len) < len))
    return 0;
  return 1;
  }

int gavl_image_read_header(gavf_io_t * io,
                           gavl_dictionary_t * m,
                           gavl_video_format_t * v)
  {
  uint8_t sig_test[8];
  if((gavf_io_read_data(io, sig_test, 8) < 8) ||
     memcmp(sig_test, sig, 8) ||
     !gavl_dictionary_read(io, m) ||
     !gavf_read_video_format(io, v))
    return 0;
  return 1;
  }

int gavl_image_read_image(gavf_io_t * io,
                          gavl_video_format_t * v,
                          gavl_video_frame_t * f)
  {
  int len = gavl_video_format_get_image_size(v);
  if(gavf_io_read_data(io, f->planes[0], len) < len)
    return 0;
  return 1;
  }
