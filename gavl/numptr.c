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

#include <stdlib.h>
#include <string.h>
#include <math.h>


#include <gavl/numptr.h>

float
GAVL_PTR_2_FLOAT32BE(const void * p)
  {
  int             exponent, mantissa, negative ;
  float   fvalue ;
  const unsigned char *cptr = p;
  negative = cptr [0] & 0x80 ;
  exponent = ((cptr [0] & 0x7F) << 1) | ((cptr [1] & 0x80) ? 1 : 0) ;
  mantissa = ((cptr [1] & 0x7F) << 16) | (cptr [2] << 8) | (cptr [3]) ;

  if (! (exponent || mantissa))
    return 0.0 ;

  mantissa |= 0x800000 ;
  exponent = exponent ? exponent - 127 : 0 ;

  fvalue = mantissa ? ((float) mantissa) / ((float) 0x800000) : 0.0 ;

  if (negative)
    fvalue *= -1 ;

  if (exponent > 0)
    fvalue *= (1 << exponent) ;
  else if (exponent < 0)
    fvalue /= (1 << abs (exponent)) ;

  return fvalue ;
  } /* float32_be_read */

float
GAVL_PTR_2_FLOAT32LE(const void * p)
  {
  int             exponent, mantissa, negative ;
  float   fvalue ;
  const unsigned char *cptr = p;
  
  negative = cptr [3] & 0x80 ;
  exponent = ((cptr [3] & 0x7F) << 1) | ((cptr [2] & 0x80) ? 1 : 0) ;
  mantissa = ((cptr [2] & 0x7F) << 16) | (cptr [1] << 8) | (cptr [0]) ;

  if (! (exponent || mantissa))
    return 0.0 ;

  mantissa |= 0x800000 ;
  exponent = exponent ? exponent - 127 : 0 ;

  fvalue = mantissa ? ((float) mantissa) / ((float) 0x800000) : 0.0 ;

  if (negative)
    fvalue *= -1 ;

  if (exponent > 0)
    fvalue *= (1 << exponent) ;
  else if (exponent < 0)
    fvalue /= (1 << abs (exponent)) ;

  return fvalue ;
  } /* float32_le_read */

double
GAVL_PTR_2_DOUBLE64BE(const void * p)
  {
  int             exponent, negative ;
  double  dvalue ;
  const unsigned char *cptr = p;

  negative = (cptr [0] & 0x80) ? 1 : 0 ;
  exponent = ((cptr [0] & 0x7F) << 4) | ((cptr [1] >> 4) & 0xF) ;

  /* Might not have a 64 bit long, so load the mantissa into a double. */
  dvalue = (((cptr [1] & 0xF) << 24) | (cptr [2] << 16) | (cptr [3] << 8) | cptr [4]) ;
  dvalue += ((cptr [5] << 16) | (cptr [6] << 8) | cptr [7]) / ((double) 0x1000000) ;

  if (exponent == 0 && dvalue == 0.0)
    return 0.0 ;

  dvalue += 0x10000000 ;

  exponent = exponent - 0x3FF ;

  dvalue = dvalue / ((double) 0x10000000) ;

  if (negative)
    dvalue *= -1 ;

  if (exponent > 0)
    dvalue *= (1 << exponent) ;
  else if (exponent < 0)
    dvalue /= (1 << abs (exponent)) ;

  return dvalue ;
  } /* double64_be_read */

double
GAVL_PTR_2_DOUBLE64LE(const void * p)
  {
  int             exponent, negative ;
  double  dvalue ;
  const unsigned char *cptr = p;

  negative = (cptr [7] & 0x80) ? 1 : 0 ;
  exponent = ((cptr [7] & 0x7F) << 4) | ((cptr [6] >> 4) & 0xF) ;

  /* Might not have a 64 bit long, so load the mantissa into a double. */
  dvalue = (((cptr [6] & 0xF) << 24) | (cptr [5] << 16) | (cptr [4] << 8) | cptr [3]) ;
  dvalue += ((cptr [2] << 16) | (cptr [1] << 8) | cptr [0]) / ((double) 0x1000000) ;

  if (exponent == 0 && dvalue == 0.0)
    return 0.0 ;

  dvalue += 0x10000000 ;

  exponent = exponent - 0x3FF ;

  dvalue = dvalue / ((double) 0x10000000) ;

  if (negative)
    dvalue *= -1 ;

  if (exponent > 0)
    dvalue *= (1 << exponent) ;
  else if (exponent < 0)
    dvalue /= (1 << abs (exponent)) ;

  return dvalue ;
  } /* double64_le_read */


void
GAVL_FLOAT32LE_2_PTR(float in, void * p)
  {
  int             exponent, mantissa, negative = 0 ;
  unsigned char *out = p;
  memset (out, 0, sizeof (int)) ;

  if (in == 0.0)
    return ;

  if (in < 0.0)
    {       in *= -1.0 ;
    negative = 1 ;
    } ;

  in = frexp (in, &exponent) ;

  exponent += 126 ;

  in *= (float) 0x1000000 ;
  mantissa = (((int) in) & 0x7FFFFF) ;

  if (negative)
    out [3] |= 0x80 ;

  if (exponent & 0x01)
    out [2] |= 0x80 ;

  out [0] = mantissa & 0xFF ;
  out [1] = (mantissa >> 8) & 0xFF ;
  out [2] |= (mantissa >> 16) & 0x7F ;
  out [3] |= (exponent >> 1) & 0x7F ;

  return ;
  } /* float32_le_write */

void
GAVL_FLOAT32BE_2_PTR(float in, void * p)
  {
  int             exponent, mantissa, negative = 0 ;
  unsigned char *out = p;
  
  memset (out, 0, sizeof (int)) ;

  if (in == 0.0)
    return ;

  if (in < 0.0)
    {       in *= -1.0 ;
    negative = 1 ;
    } ;

  in = frexp (in, &exponent) ;

  exponent += 126 ;

  in *= (float) 0x1000000 ;
  mantissa = (((int) in) & 0x7FFFFF) ;

  if (negative)
    out [0] |= 0x80 ;

  if (exponent & 0x01)
    out [1] |= 0x80 ;

  out [3] = mantissa & 0xFF ;
  out [2] = (mantissa >> 8) & 0xFF ;
  out [1] |= (mantissa >> 16) & 0x7F ;
  out [0] |= (exponent >> 1) & 0x7F ;

  return ;
  } /* float32_be_write */

void
GAVL_DOUBLE64BE_2_PTR(double in, void * p)
  {
  int             exponent, mantissa ;
  unsigned char *out = p;
  
  memset (out, 0, sizeof (double)) ;

  if (in == 0.0)
    return ;

  if (in < 0.0)
    {       in *= -1.0 ;
    out [0] |= 0x80 ;
    } ;

  in = frexp (in, &exponent) ;

  exponent += 1022 ;

  out [0] |= (exponent >> 4) & 0x7F ;
  out [1] |= (exponent << 4) & 0xF0 ;

  in *= 0x20000000 ;
  mantissa = lrint (floor (in)) ;

  out [1] |= (mantissa >> 24) & 0xF ;
  out [2] = (mantissa >> 16) & 0xFF ;
  out [3] = (mantissa >> 8) & 0xFF ;
  out [4] = mantissa & 0xFF ;

  in = fmod (in, 1.0) ;
  in *= 0x1000000 ;
  mantissa = lrint (floor (in)) ;

  out [5] = (mantissa >> 16) & 0xFF ;
  out [6] = (mantissa >> 8) & 0xFF ;
  out [7] = mantissa & 0xFF ;

  return ;
  } /* double64_be_write */


void
GAVL_DOUBLE64LE_2_PTR(double in, void * p)
  {
  int             exponent, mantissa ;
  unsigned char *out = p;
  
  memset (out, 0, sizeof (double)) ;

  if (in == 0.0)
    return ;

  if (in < 0.0)
    {       in *= -1.0 ;
    out [7] |= 0x80 ;
    } ;

  in = frexp (in, &exponent) ;

  exponent += 1022 ;

  out [7] |= (exponent >> 4) & 0x7F ;
  out [6] |= (exponent << 4) & 0xF0 ;

  in *= 0x20000000 ;
  mantissa = lrint (floor (in)) ;

  out [6] |= (mantissa >> 24) & 0xF ;
  out [5] = (mantissa >> 16) & 0xFF ;
  out [4] = (mantissa >> 8) & 0xFF ;
  out [3] = mantissa & 0xFF ;

  in = fmod (in, 1.0) ;
  in *= 0x1000000 ;
  mantissa = lrint (floor (in)) ;

  out [2] = (mantissa >> 16) & 0xFF ;
  out [1] = (mantissa >> 8) & 0xFF ;
  out [0] = mantissa & 0xFF ;

  return ;
  } /* double64_le_write */
