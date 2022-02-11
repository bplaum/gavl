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
#include <inttypes.h>
#include <stdio.h>

#define RECLIP(v, min, max) \
  if(v<min) v=min; if(v>max) v=max;

#define HEADER_NAME "colorspace_tables.h"
#define SOURCE_NAME "colorspace_tables.c"

int main(int argc, char ** argv)
  {
  FILE * header;
  FILE * source;
    
  int i;
  int      tmp_int;
  float    tmp_float;

  header = fopen(HEADER_NAME, "w");

  if(!header)
    {
    fprintf(stderr, "Cannot open header %s\n", HEADER_NAME);
    return 1;
    }
  
  source = fopen(SOURCE_NAME, "w");
  
  if(!source)
    {
    fprintf(stderr, "Cannot open header %s\n", SOURCE_NAME);
    fclose(header);
    return 1;
    }

  /* Some #include's for the .c file */

  fprintf(source, "#include <inttypes.h>\n");
  fprintf(source, "#include \"colorspace_tables.h\"\n");
  
  /* JPEG Quantisation <-> MPEG Quantisation */
    
  /* yj_8 -> y_8 */
  fprintf(header, "extern const uint8_t gavl_yj_8_to_y_8[256];\n");
  fprintf(source, "const uint8_t gavl_yj_8_to_y_8[256] = \n{\n");
  for(i = 0; i < 256; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i) / 255.0)*219.0 + 16.0;
    tmp_int   = (int)(tmp_float+0.5);
    fprintf(source, "0x%02x, ", tmp_int);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  /* uvj_8 -> uv_8 */
  fprintf(header, "extern const uint8_t gavl_uvj_8_to_uv_8[256];\n");
  fprintf(source, "const uint8_t gavl_uvj_8_to_uv_8[256] = \n{\n");
  for(i = 0; i < 256; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i-128) / 255.0)*224.0 + 128.0;
    tmp_int   = (int)(tmp_float+0.5);
    fprintf(source, "0x%02x, ", tmp_int);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  
  /* yj_8 -> y_16 */


  fprintf(header, "extern const uint16_t gavl_yj_8_to_y_16[256];\n");
  fprintf(source, "const uint16_t gavl_yj_8_to_y_16[256] = \n{\n");
  for(i = 0; i < 256; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i) / 255.0)*219.0 + 16.0;
    tmp_int   = (int)(tmp_float*256.0+0.5);
    fprintf(source, "0x%04x, ", tmp_int);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  /* uvj_8 -> uv_16 */
  fprintf(header, "extern const uint16_t gavl_uvj_8_to_uv_16[256];\n");
  fprintf(source, "const uint16_t gavl_uvj_8_to_uv_16[256] = \n{\n");

  for(i = 0; i < 256; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i-128) / 255.0)*224.0 + 128.0;
    tmp_int   = (int)(tmp_float*256.0+0.5);
    fprintf(source, "0x%04x, ", tmp_int);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  
  /* y_8 -> yj_8 */


  fprintf(header, "extern const uint8_t gavl_y_8_to_yj_8[256];\n");
  fprintf(source, "const uint8_t gavl_y_8_to_yj_8[256] = \n{\n");
  for(i = 0; i < 256; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i - 16) / 219.0)*255.0;
    tmp_int   = (int)(tmp_float+0.5);
    RECLIP(tmp_int, 0, 255);
    fprintf(source, "0x%02x, ", tmp_int);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  /* uvj_8 -> uv_8 */
  fprintf(header, "extern const uint8_t gavl_uv_8_to_uvj_8[256];\n");
  fprintf(source, "const uint8_t gavl_uv_8_to_uvj_8[256] = \n{\n");
  for(i = 0; i < 256; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i-128) / 224.0)*255.0 + 128.0;
    tmp_int   = (int)(tmp_float+0.5);
    RECLIP(tmp_int, 0, 255);
    fprintf(source, "0x%02x, ", tmp_int);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  /* y_8 -> yj_16 */
  
  fprintf(header, "extern const uint16_t gavl_y_8_to_yj_16[256];\n");
  fprintf(source, "const uint16_t gavl_y_8_to_yj_16[256] = \n{\n");
  for(i = 0; i < 256; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i - 16) / 219.0)*65535.0;
    tmp_int   = (int)(tmp_float+0.5);
    RECLIP(tmp_int, 0, 65535);
    fprintf(source, "0x%04x, ", tmp_int);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  /* y_8 -> y_float */
  
  fprintf(header, "extern const float gavl_y_8_to_y_float[256];\n");
  fprintf(source, "const float gavl_y_8_to_y_float[256] = \n{\n");
  for(i = 0; i < 256; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i - 16) / 219.0);
    RECLIP(tmp_float, 0.0, 1.0);
    fprintf(source, "%.10f, ", tmp_float);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  /* uv_8 -> uv_float */
  
  fprintf(header, "extern const float gavl_uv_8_to_uv_float[256];\n");
  fprintf(source, "const float gavl_uv_8_to_uv_float[256] = \n{\n");
  for(i = 0; i < 256; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i - 0x80) / 224.0);
    RECLIP(tmp_float, -0.5, 0.5);
    fprintf(source, "%.10f, ", tmp_float);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  /* yj_8 -> y_float */
  
  fprintf(header, "extern const float gavl_yj_8_to_y_float[256];\n");
  fprintf(source, "const float gavl_yj_8_to_y_float[256] = \n{\n");
  for(i = 0; i < 256; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i) / 255.0);
    RECLIP(tmp_float, 0.0, 1.0);
    fprintf(source, "%.10f, ", tmp_float);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");
  
  
  /* RGB 5/6 bit -> 8 bit */

    
  fprintf(header, "extern const uint8_t gavl_rgb_5_to_8[32];\n");
  fprintf(source, "const uint8_t gavl_rgb_5_to_8[32] = \n{\n");
  for(i = 0; i < 32; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i) / 31.0 * 255.0);
    tmp_int   = (int)(tmp_float+0.5);
    fprintf(source, "0x%02x, ", tmp_int);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const uint8_t gavl_rgb_6_to_8[64];\n");
  fprintf(source, "const uint8_t gavl_rgb_6_to_8[64] = \n{\n");
  for(i = 0; i < 64; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i) / 63.0 * 255.0);
    tmp_int   = (int)(tmp_float+0.5);
    fprintf(source, "0x%02x, ", tmp_int);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");


  /* RGB 5/6 bit -> 16 bit */


  fprintf(header, "extern const uint16_t gavl_rgb_5_to_16[32];\n");
  fprintf(source, "const uint16_t gavl_rgb_5_to_16[32] = \n{\n");
  for(i = 0; i < 32; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i) / 31.0 * 65535.0);
    tmp_int   = (int)(tmp_float+0.5);
    fprintf(source, "0x%04x, ", tmp_int);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const uint16_t gavl_rgb_6_to_16[64];\n");
  fprintf(source, "const uint16_t gavl_rgb_6_to_16[64] = \n{\n");
  for(i = 0; i < 64; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i) / 63.0 * 65535.0);
    tmp_int   = (int)(tmp_float+0.5);
    fprintf(source, "0x%04x, ", tmp_int);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

    
  /* RGB 5/6 bit -> float */


  fprintf(header, "extern const float gavl_rgb_5_to_float[32];\n");
  fprintf(source, "const float gavl_rgb_5_to_float[32] = \n{\n");
  for(i = 0; i < 32; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i) / 31.0);
    fprintf(source, "%8.6f, ", tmp_float);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const float gavl_rgb_6_to_float[64];\n");
  fprintf(source, "const float gavl_rgb_6_to_float[64] = \n{\n");
  for(i = 0; i < 64; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    tmp_float = ((float)(i) / 63.0);
    fprintf(source, "%8.6f, ", tmp_float);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

    
  fprintf(source, "/* RGB -> YUV conversions */\n");

    
  fprintf(header, "extern const int gavl_r_to_y[256];\n");
  fprintf(source, "const int gavl_r_to_y[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)((0.29900*219.0/255.0)*0x10000 * i + 16 * 0x10000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_g_to_y[256];\n");
  fprintf(source, "const int gavl_g_to_y[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)((0.58700*219.0/255.0)*0x10000 * i + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_b_to_y[256];\n");
  fprintf(source, "const int gavl_b_to_y[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)((0.11400*219.0/255.0)*0x10000 * i + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_r_to_u[256];\n");
  fprintf(source, "const int gavl_r_to_u[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)(-(0.16874*224.0/255.0)*0x10000 * i + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");
  
  fprintf(header, "extern const int gavl_g_to_u[256];\n");
  fprintf(source, "const int gavl_g_to_u[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)(-(0.33126*224.0/255.0)*0x10000 * i + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_b_to_u[256];\n");
  fprintf(source, "const int gavl_b_to_u[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)( (0.50000*224.0/255.0)*0x10000 * i + 0x800000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");
    
  fprintf(header, "extern const int gavl_r_to_v[256];\n");
  fprintf(source, "const int gavl_r_to_v[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)( (0.50000*224.0/255.0)*0x10000 * i + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_g_to_v[256];\n");
  fprintf(source, "const int gavl_g_to_v[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)(-(0.41869*224.0/255.0)*0x10000 * i + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_b_to_v[256];\n");
  fprintf(source, "const int gavl_b_to_v[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)(-(0.08131*224.0/255.0)*0x10000 * i + 0x800000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(source, "/* RGB -> YUVJ conversions */\n");
  
  fprintf(header, "extern const int gavl_r_to_yj[256];\n");
  fprintf(source, "const int gavl_r_to_yj[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)((0.29900)*0x10000 * i + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_g_to_yj[256];\n");
  fprintf(source, "const int gavl_g_to_yj[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)((0.58700)*0x10000 * i + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_b_to_yj[256];\n");
  fprintf(source, "const int gavl_b_to_yj[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)((0.11400)*0x10000 * i + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");
    
  fprintf(header, "extern const int gavl_r_to_uj[256];\n");
  fprintf(source, "const int gavl_r_to_uj[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)(-(0.16874)*0x10000 * i + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_g_to_uj[256];\n");
  fprintf(source, "const int gavl_g_to_uj[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)(-(0.33126)*0x10000 * i + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_b_to_uj[256];\n");
  fprintf(source, "const int gavl_b_to_uj[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)( (0.50000)*0x10000 * i + 0x800000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");
    
  fprintf(header, "extern const int gavl_r_to_vj[256];\n");
  fprintf(source, "const int gavl_r_to_vj[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)( (0.50000)*0x10000 * i + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_g_to_vj[256];\n");
  fprintf(source, "const int gavl_g_to_vj[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)(-(0.41869)*0x10000 * i + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_b_to_vj[256];\n");
  fprintf(source, "const int gavl_b_to_vj[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)(-(0.08131)*0x10000 * i + 0x800000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  /* */

  fprintf(source, "/* RGB -> YUV float conversions */\n");
  
  fprintf(header, "extern const float gavl_r_to_y_float[256];\n");
  fprintf(source, "const float gavl_r_to_y_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%.8f, ", (float)i/255.0*0.29900);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const float gavl_g_to_y_float[256];\n");
  fprintf(source, "const float gavl_g_to_y_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%.8f, ", (float)i/255.0*0.58700);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const float gavl_b_to_y_float[256];\n");
  fprintf(source, "const float gavl_b_to_y_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%.8f, ", (float)i/255.0*0.11400);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");
    
  fprintf(header, "extern const float gavl_r_to_u_float[256];\n");
  fprintf(source, "const float gavl_r_to_u_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%.8f, ", (float)i/255.0*(-0.16874));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const float gavl_g_to_u_float[256];\n");
  fprintf(source, "const float gavl_g_to_u_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%.8f, ", (float)i/255.0*(-0.33126));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const float gavl_b_to_u_float[256];\n");
  fprintf(source, "const float gavl_b_to_u_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%.8f, ", (float)i/255.0*(0.50000));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");
    
  fprintf(header, "extern const float gavl_r_to_v_float[256];\n");
  fprintf(source, "const float gavl_r_to_v_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%.8f, ", (float)i/255.0*(0.50000));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const float gavl_g_to_v_float[256];\n");
  fprintf(source, "const float gavl_g_to_v_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%.8f, ", (float)i/255.0*(-0.41869));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const float gavl_b_to_v_float[256];\n");
  fprintf(source, "const float gavl_b_to_v_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%.8f, ", (float)i/255.0*(-0.08131));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");


  
    
  fprintf(source, "/* YUV -> RGB conversions */\n");

  // YCbCr (8bit) -> R'G'B' (integer) according to CCIR 601

 
  fprintf(header, "extern const int gavl_y_to_rgb[256];\n");
  fprintf(source, "const int gavl_y_to_rgb[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    
    fprintf(source, "%d, ", (int)(255.0/219.0*(i-16) * 0x10000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");
  
  fprintf(header, "extern const int gavl_v_to_r[256];\n");
  fprintf(source, "const int gavl_v_to_r[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)( 1.40200*255.0/224.0 * (i - 0x80) * 0x10000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_u_to_g[256];\n");
  fprintf(source, "const int gavl_u_to_g[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)(-0.34414*255.0/224.0 * (i - 0x80) * 0x10000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_v_to_g[256];\n");
  fprintf(source, "const int gavl_v_to_g[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)(-0.71414*255.0/224.0 * (i - 0x80) * 0x10000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_u_to_b[256];\n");
  fprintf(source, "const int gavl_u_to_b[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)( 1.77200*255.0/224.0 * (i - 0x80) * 0x10000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

    
  /* JPEG Quantization */

  
  fprintf(header, "extern const int gavl_yj_to_rgb[256];\n");
  fprintf(source, "const int gavl_yj_to_rgb[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)(i * 0x10000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");
    
  fprintf(header, "extern const int gavl_vj_to_r[256];\n");
  fprintf(source, "const int gavl_vj_to_r[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)( 1.40200 * (i - 0x80) * 0x10000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_uj_to_g[256];\n");
  fprintf(source, "const int gavl_uj_to_g[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)(-0.34414 * (i - 0x80) * 0x10000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_vj_to_g[256];\n");
  fprintf(source, "const int gavl_vj_to_g[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)(-0.71414 * (i - 0x80) * 0x10000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const int gavl_uj_to_b[256];\n");
  fprintf(source, "const int gavl_uj_to_b[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%d, ", (int)( 1.77200 * (i - 0x80) * 0x10000 + 0.5));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  
  // YCbCr (8bit) -> R'G'B' (float) according to CCIR 601

    
  fprintf(header, "extern const float gavl_y_to_rgb_float[256];\n");
  fprintf(source, "const float gavl_y_to_rgb_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    
    fprintf(source, "%f, ", 1.0/219.0*(i-16));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");
  
  fprintf(header, "extern const float gavl_v_to_r_float[256];\n");
  fprintf(source, "const float gavl_v_to_r_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%f, ",  1.40200/224.0 * (i - 0x80));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const float gavl_u_to_g_float[256];\n");
  fprintf(source, "const float gavl_u_to_g_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%f, ", -0.34414/224.0 * (i - 0x80));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const float gavl_v_to_g_float[256];\n");
  fprintf(source, "const float gavl_v_to_g_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%f, ", -0.71414/224.0 * (i - 0x80));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const float gavl_u_to_b_float[256];\n");
  fprintf(source, "const float gavl_u_to_b_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%f, ",  1.77200/224.0 * (i - 0x80));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");
    
  /* JPEG Quantization */

  
  fprintf(header, "extern const float gavl_yj_to_rgb_float[256];\n");
  fprintf(source, "const float gavl_yj_to_rgb_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%f, ", (float)i/255.0);
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");
    
  fprintf(header, "extern const float gavl_vj_to_r_float[256];\n");
  fprintf(source, "const float gavl_vj_to_r_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%f, ",  1.40200/255.0 * (i - 0x80));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const float gavl_uj_to_g_float[256];\n");
  fprintf(source, "const float gavl_uj_to_g_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%f, ", -0.34414/255.0 * (i - 0x80));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const float gavl_vj_to_g_float[256];\n");
  fprintf(source, "const float gavl_vj_to_g_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%f, ", -0.71414/255.0 * (i - 0x80));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fprintf(header, "extern const float gavl_uj_to_b_float[256];\n");
  fprintf(source, "const float gavl_uj_to_b_float[256] = \n{\n");
  for(i = 0; i < 0x100; i++)
    {
    if(!((i)%8))
      fprintf(source, "  ");
    fprintf(source, "%f, ",  1.77200/255.0 * (i - 0x80));
    if(!((i+1)%8))
      fprintf(source, "\n");
    }
  fprintf(source, "};\n\n");

  fclose(header);
  fclose(source);

  fprintf(stderr, "Created %s and %s\n", HEADER_NAME, SOURCE_NAME);
 
  }
