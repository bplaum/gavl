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



#ifndef DSP_H_INCLUDED
#define DSP_H_INCLUDED

struct gavl_dsp_context_s
  {
  int quality;
  int accel_flags;
  gavl_dsp_funcs_t funcs;
  };

void gavl_dsp_init_c(gavl_dsp_funcs_t * funcs, 
                     int quality);

#ifdef HAVE_MMX
void gavl_dsp_init_mmx(gavl_dsp_funcs_t * funcs, 
                       int quality);
void gavl_dsp_init_mmxext(gavl_dsp_funcs_t * funcs, 
                          int quality);
#endif

#ifdef HAVE_SSE
void gavl_dsp_init_sse(gavl_dsp_funcs_t * funcs, 
                       int quality);
#endif


#endif // DSP_H_INCLUDED
