#include <string.h>


#include <gavl/gavl.h>
#include <video.h>


static struct
  {
  gavl_image_orientation_t orient;
  int transpose;
  int flip_h_src;
  int flip_v_src;
  }
orientations[8] =
  {
    
    { GAVL_IMAGE_ORIENT_NORMAL,       0, 0, 0 }, // EXIF: 1
    { GAVL_IMAGE_ORIENT_ROT90_CW,     1, 1, 0 }, // EXIF: 8
    { GAVL_IMAGE_ORIENT_ROT180_CW,    0, 1, 1 }, // EXIF: 3
    { GAVL_IMAGE_ORIENT_ROT270_CW,    1, 0, 1 }, // EXIF: 6
    
    { GAVL_IMAGE_ORIENT_FH,           0, 1, 0 }, // EXIF: 2
    { GAVL_IMAGE_ORIENT_FH_ROT90_CW,  1, 1, 1 }, // EXIF: 7
    { GAVL_IMAGE_ORIENT_FH_ROT180_CW, 0, 0, 1 }, // EXIF: 4
    { GAVL_IMAGE_ORIENT_FH_ROT270_CW, 1, 0, 0 }, // EXIF: 5
              
  };

typedef void (*scanline_func_t)(const uint8_t * in, uint8_t * out, int in_advance, int num);


static void scanline_func_8(uint8_t * in, uint8_t * out, int in_advance, int num)
  {
  int i;
  for(i = 0; i < num; i++)
    {
    *out = *in;
    
    in += in_advance;
    out++;
    }
  }

static void scanline_func_16(uint8_t * in, uint8_t * out, int in_advance, int num)
  {
  int i;
  for(i = 0; i < num; i++)
    {
    out[0] = in[0];
    out[1] = in[1];
    
    in += in_advance;
    out += 2;
    }
  
  }

static void scanline_func_24(uint8_t * in, uint8_t * out, int in_advance, int num)
  {
  int i;
  for(i = 0; i < num; i++)
    {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    in += in_advance;
    out += 3;
    }
  
  }

static void scanline_func_32(const uint8_t * inp, uint8_t * outp, int in_advance, int num)
  {
  int i;

  const uint32_t * in = (uint32_t *)inp;
  uint32_t * out = (uint32_t *)outp;
  in_advance /= 4;
  for(i = 0; i < num; i++)
    {
    out[0] = in[0];
    in += in_advance;
    out++;
    }
  }

static void scanline_func_48(const uint8_t * in, uint8_t * out, int in_advance, int num)
  {
  int i;

  for(i = 0; i < num; i++)
    {
    memcpy(out, in, 6);
    in += in_advance;
    out+=6;
    }
  
  }

static void scanline_func_64(const uint8_t * inp, uint8_t * outp, int in_advance, int num)
  {
  int i;

  const uint64_t * in = (uint64_t *)inp;
  uint64_t * out = (uint64_t *)outp;
  in_advance /= 8;
  for(i = 0; i < num; i++)
    {
    out[0] = in[0];
    in += in_advance;
    out++;
    }
  }

static void normalize_plane(int out_width, int out_height,
                            const gavl_video_frame_t * in_frame,
                            gavl_video_frame_t * out_frame, int plane, int bytes, int idx, scanline_func_t scanline_func)
  {
  const uint8_t * in;
  uint8_t * out;
  int i;
  int in_advance_x;
  int in_advance_y;
  
  in = in_frame->planes[plane];
  out = in_frame->planes[plane];


  
  if(orientations[idx].transpose)
    {
    in_advance_x = in_frame->strides[plane];
    in_advance_y = bytes;

    if(orientations[idx].flip_h_src)
      {
      
      
      }
    if(orientations[idx].flip_v_src)
      {
      }

    }
  else
    {
    in_advance_x = bytes;
    in_advance_y = in_frame->strides[plane];

    if(orientations[idx].flip_h_src)
      {

      }
    if(orientations[idx].flip_v_src)
      {
      
      }
    
    }

  for(i = 0; i < out_height; i++)
    {
    scanline_func(in, out, in_advance_x, out_width);
    out += out_frame->strides[plane];
    in += in_advance_y;
    }
  
  }

static scanline_func_t get_scanline_func(int bytes)
  {
  switch(bytes)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 6:
    case 8:
    case 3*sizeof(float):
    case 4*sizeof(float):
      break;
    }
  return NULL;
  }
  
void gavl_video_frame_normalize_orientation(const gavl_video_format_t * in_format,
                                            const gavl_video_format_t * out_format,
                                            const gavl_video_frame_t * in_frame,
                                            gavl_video_frame_t * out_frame)
  {
  if(!gavl_pixelformat_is_planar(in_format->pixelformat))
    {
    }
  else
    {
    int i;
    int num_planes = 
      gavl_pixelformat_num_planes(in_format->pixelformat);

    for(i = 0; i < num_planes; i++)
      {
      
      }
    
    }
     
  }

void gavl_video_format_normalize_orientation(gavl_video_format_t * in_format,
                                             gavl_video_format_t * out_format)
  {
  
  }
