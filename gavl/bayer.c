#include <gavl/gavl.h>

/*
 * lib4lconvert, video4linux2 format conversion lib
 *             (C) 2008 Hans de Goede <hdegoede@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Note: original bayer_to_bgr24 code from :
 * 1394-Based Digital Camera Control Library
 *
 * Bayer pattern decoding functions
 *
 * Written by Damien Douxchamps and Frederic Devernay
 *
 * Note that the original bayer.c in libdc1394 supports many different
 * bayer decode algorithms, for lib4lconvert the one in this file has been
 * chosen (and optimized a bit) and the other algorithm's have been removed,
 * see bayer.c from libdc1394 for all supported algorithms
 */

/* Adapted for gavl */

/**************************************************************
 *     Color conversion functions for cameras that can        *
 * output raw-Bayer pattern images, such as some Basler and   *
 * Point Grey camera. Most of the algos presented here come   *
 * from http://www-ise.stanford.edu/~tingchen/ and have been  *
 * converted from Matlab to C and extended to all elementary  *
 * patterns.                                                  *
 **************************************************************/

/* insprired by OpenCV's Bayer decoding */
static void v4lconvert_border_bayer_line_to_bgr24(
  const unsigned char* bayer, const unsigned char* adjacent_bayer,
  unsigned char *bgr, int width, int start_with_green, int blue_line)
{
  int t0, t1;

  if (start_with_green) {
    /* First pixel */
    if (blue_line) {
      *bgr++ = bayer[1];
      *bgr++ = bayer[0];
      *bgr++ = adjacent_bayer[0];
    } else {
      *bgr++ = adjacent_bayer[0];
      *bgr++ = bayer[0];
      *bgr++ = bayer[1];
    }
    /* Second pixel */
    t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
    t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
    if (blue_line) {
      *bgr++ = bayer[1];
      *bgr++ = t0;
      *bgr++ = t1;
    } else {
      *bgr++ = t1;
      *bgr++ = t0;
      *bgr++ = bayer[1];
    }
    bayer++;
    adjacent_bayer++;
    width -= 2;
  } else {
    /* First pixel */
    t0 = (bayer[1] + adjacent_bayer[0] + 1) >> 1;
    if (blue_line) {
      *bgr++ = bayer[0];
      *bgr++ = t0;
      *bgr++ = adjacent_bayer[1];
    } else {
      *bgr++ = adjacent_bayer[1];
      *bgr++ = t0;
      *bgr++ = bayer[0];
    }
    width--;
  }

  if (blue_line) {
    for ( ; width > 2; width -= 2) {
      t0 = (bayer[0] + bayer[2] + 1) >> 1;
      *bgr++ = t0;
      *bgr++ = bayer[1];
      *bgr++ = adjacent_bayer[1];
      bayer++;
      adjacent_bayer++;

      t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
      t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
      *bgr++ = bayer[1];
      *bgr++ = t0;
      *bgr++ = t1;
      bayer++;
      adjacent_bayer++;
    }
  } else {
    for ( ; width > 2; width -= 2) {
      t0 = (bayer[0] + bayer[2] + 1) >> 1;
      *bgr++ = adjacent_bayer[1];
      *bgr++ = bayer[1];
      *bgr++ = t0;
      bayer++;
      adjacent_bayer++;

      t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
      t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
      *bgr++ = t1;
      *bgr++ = t0;
      *bgr++ = bayer[1];
      bayer++;
      adjacent_bayer++;
    }
  }

  if (width == 2) {
    /* Second to last pixel */
    t0 = (bayer[0] + bayer[2] + 1) >> 1;
    if (blue_line) {
      *bgr++ = t0;
      *bgr++ = bayer[1];
      *bgr++ = adjacent_bayer[1];
    } else {
      *bgr++ = adjacent_bayer[1];
      *bgr++ = bayer[1];
      *bgr++ = t0;
    }
    /* Last pixel */
    t0 = (bayer[1] + adjacent_bayer[2] + 1) >> 1;
    if (blue_line) {
      *bgr++ = bayer[2];
      *bgr++ = t0;
      *bgr++ = adjacent_bayer[1];
    } else {
      *bgr++ = adjacent_bayer[1];
      *bgr++ = t0;
      *bgr++ = bayer[2];
    }
  } else {
    /* Last pixel */
    if (blue_line) {
      *bgr++ = bayer[0];
      *bgr++ = bayer[1];
      *bgr++ = adjacent_bayer[1];
    } else {
      *bgr++ = adjacent_bayer[1];
      *bgr++ = bayer[1];
      *bgr++ = bayer[0];
    }
  }
}

/* From libdc1394, which on turn was based on OpenCV's Bayer decoding */
static void bayer_to_rgbbgr24(gavl_video_frame_t * src, gavl_video_frame_t * dst,
                              int image_width, int height,
                              int row,
                              int start_with_green, int blue_line)
  {
  const unsigned char *bayer;
  unsigned char *bgr;

  int t0, t1;
  /* (width - 2) because of the border */
  const unsigned char *bayerEnd;
  int bstride = src->strides[0];

  bgr = dst->planes[0] + dst->strides[0] * row;
  
  if(!row)
    /* render the first line */
    {
    bayer = src->planes[0];
    
    v4lconvert_border_bayer_line_to_bgr24(bayer, bayer + bstride, bgr, image_width,
                                          start_with_green, blue_line);
    return;
    }
  else if(row == height - 1)
    {
    bayer = src->planes[0] + row * bstride;
    
    v4lconvert_border_bayer_line_to_bgr24(bayer, bayer - bstride, bgr, image_width,
                                          !start_with_green, !blue_line);
    return;
    }

  bayer = src->planes[0] + (row-1) * bstride;
  
  //  /* reduce height by 2 because of the special case top/bottom line */
  //  for (height -= 2; height; height--) {
  /* (width - 2) because of the border */
  bayerEnd = bayer + (image_width - 2);

  if (start_with_green)
    {
    /* OpenCV has a bug in the next line, which was
       t0 = (bayer[0] + bayer[width * 2] + 1) >> 1; */
    t0 = (bayer[1] + bayer[bstride * 2 + 1] + 1) >> 1;
    /* Write first pixel */
    t1 = (bayer[0] + bayer[bstride * 2] + bayer[bstride + 1] + 1) / 3;
    if (blue_line)
      {
      *bgr++ = t0;
      *bgr++ = t1;
      *bgr++ = bayer[bstride];
      }
    else
      {
      *bgr++ = bayer[bstride];
      *bgr++ = t1;
      *bgr++ = t0;
      }
    
    /* Write second pixel */
    t1 = (bayer[bstride] + bayer[bstride + 2] + 1) >> 1;
    if (blue_line)
      {
      *bgr++ = t0;
      *bgr++ = bayer[bstride + 1];
      *bgr++ = t1;
      }
    else
      {
      *bgr++ = t1;
      *bgr++ = bayer[bstride + 1];
      *bgr++ = t0;
      }
    bayer++;
    }
  else
    {
    /* Write first pixel */
    t0 = (bayer[0] + bayer[bstride * 2] + 1) >> 1;
    if (blue_line)
      {
      *bgr++ = t0;
      *bgr++ = bayer[bstride];
      *bgr++ = bayer[bstride + 1];
      }
    else
      {
      *bgr++ = bayer[bstride + 1];
      *bgr++ = bayer[bstride];
      *bgr++ = t0;
      }
    }
  
  if (blue_line)
    {
    for (; bayer <= bayerEnd - 2; bayer += 2)
      {
      t0 = (bayer[0] + bayer[2] + bayer[bstride * 2] +
            bayer[bstride * 2 + 2] + 2) >> 2;
      t1 = (bayer[1] + bayer[bstride] +
            bayer[bstride + 2] + bayer[bstride * 2 + 1] +
            2) >> 2;
      *bgr++ = t0;
      *bgr++ = t1;
      *bgr++ = bayer[bstride + 1];
      
      t0 = (bayer[2] + bayer[bstride * 2 + 2] + 1) >> 1;
      t1 = (bayer[bstride + 1] + bayer[bstride + 3] +
            1) >> 1;
      *bgr++ = t0;
      *bgr++ = bayer[bstride + 2];
      *bgr++ = t1;
      }
    }
  else
    {
    for (; bayer <= bayerEnd - 2; bayer += 2)
      {
      t0 = (bayer[0] + bayer[2] + bayer[bstride * 2] +
            bayer[bstride * 2 + 2] + 2) >> 2;
      t1 = (bayer[1] + bayer[bstride] +
            bayer[bstride + 2] + bayer[bstride * 2 + 1] +
            2) >> 2;
      *bgr++ = bayer[bstride + 1];
      *bgr++ = t1;
      *bgr++ = t0;
      
      t0 = (bayer[2] + bayer[bstride * 2 + 2] + 1) >> 1;
      t1 = (bayer[bstride + 1] + bayer[bstride + 3] +
            1) >> 1;
      *bgr++ = t1;
      *bgr++ = bayer[bstride + 2];
      *bgr++ = t0;
      }
    }
  
  if (bayer < bayerEnd)
    {
    /* write second to last pixel */
    t0 = (bayer[0] + bayer[2] + bayer[bstride * 2] +
          bayer[bstride * 2 + 2] + 2) >> 2;
    t1 = (bayer[1] + bayer[bstride] +
          bayer[bstride + 2] + bayer[bstride * 2 + 1] +
          2) >> 2;
    if (blue_line)
      {
      *bgr++ = t0;
      *bgr++ = t1;
      *bgr++ = bayer[bstride + 1];
      }
    else
      {
      *bgr++ = bayer[bstride + 1];
      *bgr++ = t1;
      *bgr++ = t0;
      }
    /* write last pixel */
    t0 = (bayer[2] + bayer[bstride * 2 + 2] + 1) >> 1;
    if (blue_line)
      {
      *bgr++ = t0;
      *bgr++ = bayer[bstride + 2];
      *bgr++ = bayer[bstride + 1];
      }
    else
      {
      *bgr++ = bayer[bstride + 1];
      *bgr++ = bayer[bstride + 2];
      *bgr++ = t0;
      }
    bayer++;
    }
  else
    {
    /* write last pixel */
    t0 = (bayer[0] + bayer[bstride * 2] + 1) >> 1;
    t1 = (bayer[1] + bayer[bstride * 2 + 1] + bayer[bstride] + 1) / 3;
    if (blue_line)
      {
      *bgr++ = t0;
      *bgr++ = t1;
      *bgr++ = bayer[bstride + 1];
      }
    else
      {
      *bgr++ = bayer[bstride + 1];
      *bgr++ = t1;
      *bgr++ = t0;
      }
    }
  }


static void debayer_slice(gavl_video_frame_t * src, gavl_video_frame_t * dst,
                          int bayer_format, gavl_video_format_t * dst_format,
                          int start_row, int num_rows)
  {
  int i;
  int swap_rgb = 1;
  
  int blue_line = bayer_format & GAVL_BAYER_BLUE_LINE;
  int green_first = bayer_format & GAVL_BAYER_GREEN_FIRST;

  if(start_row & 1)
    {
    blue_line = !blue_line;
    green_first = !green_first;
    }

  if(swap_rgb)
    blue_line = !blue_line;
  
  for(i = start_row; i < start_row + num_rows; i++)
    {
    bayer_to_rgbbgr24(src, dst,
                      dst_format->image_width, dst_format->image_height,
                      i,
                      green_first, blue_line);
    
    blue_line = !blue_line;
    green_first = !green_first;
    }
  
  }

void gavl_video_frame_debayer(gavl_video_options_t * opt,
                              gavl_video_frame_t * src, gavl_video_frame_t * dst,
                              int bayer_format, gavl_video_format_t * dst_format)
  {
  debayer_slice(src, dst, bayer_format, dst_format, 0, dst_format->image_height);
  
  }

