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



/*
 *  This file was taken from libsamplerate, original copyright below:
 */

/*
** Copyright (C) 2002-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*/

/*
** API documentation is available here:
**     http://www.mega-nerd.com/SRC/api.html
*/

#ifndef SAMPLERATE_H_INCLUDED
#define SAMPLERATE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */


/* Opaque data type SRC_STATE. */
typedef struct SRC_STATE_tag SRC_STATE ;

/* SRC_DATA is used to pass data to src_simple() and src_process(). */
typedef struct
{	float	*data_in_f, *data_out_f ;
        double	*data_in_d, *data_out_d ;

	long	input_frames, output_frames ;
	long	input_frames_used, output_frames_gen ;

	int		end_of_input ;

	double	src_ratio ;
} SRC_DATA ;

/* SRC_CB_DATA is used with callback based API. */
typedef struct
{	long	frames ;
	float	*data_in ;
} SRC_CB_DATA ;

/*
** User supplied callback function type for use with src_callback_new()
** and src_callback_read(). First parameter is the same pointer that was
** passed into src_callback_new(). Second parameter is pointer to a
** pointer. The user supplied callback function must modify *data to
** point to the start of the user supplied float array. The user supplied
** function must return the number of frames that **data points to.
*/

typedef long (*src_callback_t) (void *cb_data, float **data) ;

/*
**	Standard initialisation function : return an anonymous pointer to the
**	internal state of the converter. Choose a converter from the enums below.
**	Error returned in *error.
*/

SRC_STATE* gavl_src_new (int converter_type, int channels, int *error, int d) ;

/*
**	Initilisation for callback based API : return an anonymous pointer to the
**	internal state of the converter. Choose a converter from the enums below.
**	The cb_data pointer can point to any data or be set to NULL. Whatever the
**	value, when processing, user supplied function "func" gets called with
**	cb_data as first parameter.
*/

SRC_STATE* gavl_src_callback_new (src_callback_t func, int converter_type, int channels,
				int *error, void* cb_data, int d) ;

/*
**	Cleanup all internal allocations.
**	Always returns NULL.
*/

SRC_STATE* gavl_src_delete (SRC_STATE *state) ;

/*
**	Standard processing function.
**	Returns non zero on error.
*/

int gavl_src_process (SRC_STATE *state, SRC_DATA *data) ;

/*
**	Callback based processing function. Read up to frames worth of data from
**	the converter int *data and return frames read or -1 on error.
*/
long gavl_src_callback_read (SRC_STATE *state, double src_ratio, long frames, float *data) ;

/*
**	Simple interface for performing a single conversion from input buffer to
**	output buffer at a fixed conversion ratio.
**	Simple interface does not require initialisation as it can only operate on
**	a single buffer worth of audio.
*/

int gavl_src_simple (SRC_DATA *data, int converter_type, int channels) ;

/*
** This library contains a number of different sample rate converters,
** numbered 0 through N.
**
** Return a string giving either a name or a more full description of each
** sample rate converter or NULL if no sample rate converter exists for
** the given value. The converters are sequentially numbered from 0 to N.
*/

const char *gavl_src_get_name (int converter_type) ;
const char *gavl_src_get_description (int converter_type) ;
const char *gavl_src_get_version (void) ;

/*
**	Set a new SRC ratio. This allows step responses
**	in the conversion ratio.
**	Returns non zero on error.
*/

int gavl_src_set_ratio (SRC_STATE *state, double new_ratio) ;

/*
**	Reset the internal SRC state.
**	Does not modify the quality settings.
**	Does not free any memory allocations.
**	Returns non zero on error.
*/

int gavl_src_reset (SRC_STATE *state) ;

/*
** Return TRUE if ratio is a valid conversion ratio, FALSE
** otherwise.
*/

int gavl_src_is_valid_ratio (double ratio) ;

/*
**	Return an error number.
*/

int gavl_src_error (SRC_STATE *state) ;

/*
**	Convert the error number into a string.
*/
const char* gavl_src_strerror (int error) ;

/*
** The following enums can be used to set the interpolator type
** using the function src_set_converter().
*/

enum
{
	SRC_SINC_BEST_QUALITY		= 0,
	SRC_SINC_MEDIUM_QUALITY		= 1,
	SRC_SINC_FASTEST			= 2,
	SRC_ZERO_ORDER_HOLD			= 3,
	SRC_LINEAR					= 4
} ;

/*
** Extra helper functions for converting from short to float and
** back again.
*/

void gavl_src_short_to_float_array (const short *in, float *out, int len) ;
void gavl_src_float_to_short_array (const float *in, short *out, int len) ;


#ifdef __cplusplus
}		/* extern "C" */
#endif	/* __cplusplus */

#endif	/* SAMPLERATE_H_INCLUDED */

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: 5421ef3e-c898-4ec3-8671-ea03d943ee00
*/

