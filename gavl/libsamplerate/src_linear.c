/*
** Copyright (C) 2002-2008 Erik de Castro Lopo <erikd@mega-nerd.com>
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
** This code is part of Secret Rabibt Code aka libsamplerate. A commercial
** use license for this code is available, please see:
**		http://www.mega-nerd.com/SRC/procedure.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "common.h"

static int linear_vari_process_f (SRC_PRIVATE *psrc, SRC_DATA *data) ;
static int linear_vari_process_d (SRC_PRIVATE *psrc, SRC_DATA *data) ;
static void linear_reset (SRC_PRIVATE *psrc) ;

/*========================================================================================
*/

#define	LINEAR_MAGIC_MARKER	MAKE_MAGIC ('l', 'i', 'n', 'e', 'a', 'r')

#define	SRC_DEBUG	0

typedef struct
{	int		linear_magic_marker ;
	int		channels ;
	int		reset ;
	long	in_count, in_used ;
	long	out_count, out_gen ;
	float	last_value_f [1] ;
        double	last_value_d [1] ;
        int d;
} LINEAR_DATA ;

/*----------------------------------------------------------------------------------------
*/

static int
linear_vari_process_f (SRC_PRIVATE *psrc, SRC_DATA *data)
{	LINEAR_DATA *linear ;
	double		src_ratio, input_index, rem ;
	int			ch ;
	if (psrc->private_data == NULL)
		return SRC_ERR_NO_PRIVATE ;

	linear = (LINEAR_DATA*) psrc->private_data ;

	if (linear->reset)
	{	/* If we have just been reset, set the last_value data. */
		for (ch = 0 ; ch < linear->channels ; ch++)
			linear->last_value_f [ch] = data->data_in_f [ch] ;
		linear->reset = 0 ;
		} ;

	linear->in_count = data->input_frames * linear->channels ;
	linear->out_count = data->output_frames * linear->channels ;
	linear->in_used = linear->out_gen = 0 ;

	src_ratio = psrc->last_ratio ;
	input_index = psrc->last_position ;

	/* Calculate samples before first sample in input array. */
	while (input_index < 1.0 && linear->out_gen < linear->out_count)
	{
		if (linear->in_used + linear->channels * input_index > linear->in_count)
			break ;

		if (linear->out_count > 0 && fabs (psrc->last_ratio - data->src_ratio) > SRC_MIN_RATIO_DIFF)
			src_ratio = psrc->last_ratio + linear->out_gen * (data->src_ratio - psrc->last_ratio) / linear->out_count ;

		for (ch = 0 ; ch < linear->channels ; ch++)
		{	data->data_out_f [linear->out_gen] = (float) (linear->last_value_f [ch] + input_index *
										(data->data_in_f [ch] - linear->last_value_f [ch])) ;
			linear->out_gen ++ ;
			} ;

		/* Figure out the next index. */
		input_index += 1.0 / src_ratio ;
		} ;

	rem = fmod_one (input_index) ;
	linear->in_used += linear->channels * lrint (input_index - rem) ;
	input_index = rem ;

	/* Main processing loop. */
	while (linear->out_gen < linear->out_count && linear->in_used + linear->channels * input_index <= linear->in_count)
	{
		if (linear->out_count > 0 && fabs (psrc->last_ratio - data->src_ratio) > SRC_MIN_RATIO_DIFF)
			src_ratio = psrc->last_ratio + linear->out_gen * (data->src_ratio - psrc->last_ratio) / linear->out_count ;

		if (SRC_DEBUG && linear->in_used < linear->channels && input_index < 1.0)
		{	printf ("Whoops!!!!   in_used : %ld     channels : %d     input_index : %f\n", linear->in_used, linear->channels, input_index) ;
			exit (1) ;
			} ;

		for (ch = 0 ; ch < linear->channels ; ch++)
		{	data->data_out_f [linear->out_gen] = (float) (data->data_in_f [linear->in_used - linear->channels + ch] + input_index *
						(data->data_in_f [linear->in_used + ch] - data->data_in_f [linear->in_used - linear->channels + ch])) ;
			linear->out_gen ++ ;
			} ;

		/* Figure out the next index. */
		input_index += 1.0 / src_ratio ;
		rem = fmod_one (input_index) ;

		linear->in_used += linear->channels * lrint (input_index - rem) ;
		input_index = rem ;
		} ;

	if (linear->in_used > linear->in_count)
	{	input_index += (linear->in_used - linear->in_count) / linear->channels ;
		linear->in_used = linear->in_count ;
		} ;

	psrc->last_position = input_index ;

	if (linear->in_used > 0)
		for (ch = 0 ; ch < linear->channels ; ch++)
			linear->last_value_f [ch] = data->data_in_f [linear->in_used - linear->channels + ch] ;

	/* Save current ratio rather then target ratio. */
	psrc->last_ratio = src_ratio ;

	data->input_frames_used = linear->in_used / linear->channels ;
	data->output_frames_gen = linear->out_gen / linear->channels ;

	return SRC_ERR_NO_ERROR ;
} /* linear_vari_process_f */

static int
linear_vari_process_d (SRC_PRIVATE *psrc, SRC_DATA *data)
{	LINEAR_DATA *linear ;
	double		src_ratio, input_index, rem ;
	int			ch ;

	if (psrc->private_data == NULL)
		return SRC_ERR_NO_PRIVATE ;

	linear = (LINEAR_DATA*) psrc->private_data ;

	if (linear->reset)
	{	/* If we have just been reset, set the last_value data. */
		for (ch = 0 ; ch < linear->channels ; ch++)
			linear->last_value_d [ch] = data->data_in_d [ch] ;
		linear->reset = 0 ;
		} ;

	linear->in_count = data->input_frames * linear->channels ;
	linear->out_count = data->output_frames * linear->channels ;
	linear->in_used = linear->out_gen = 0 ;

	src_ratio = psrc->last_ratio ;
	input_index = psrc->last_position ;

	/* Calculate samples before first sample in input array. */
	while (input_index < 1.0 && linear->out_gen < linear->out_count)
	{
		if (linear->in_used + linear->channels * input_index > linear->in_count)
			break ;

		if (linear->out_count > 0 && fabs (psrc->last_ratio - data->src_ratio) > SRC_MIN_RATIO_DIFF)
			src_ratio = psrc->last_ratio + linear->out_gen * (data->src_ratio - psrc->last_ratio) / linear->out_count ;

		for (ch = 0 ; ch < linear->channels ; ch++)
		{	data->data_out_d [linear->out_gen] = (float) (linear->last_value_d [ch] + input_index *
										(data->data_in_d [ch] - linear->last_value_d [ch])) ;
			linear->out_gen ++ ;
			} ;

		/* Figure out the next index. */
		input_index += 1.0 / src_ratio ;
		} ;

	rem = fmod_one (input_index) ;
	linear->in_used += linear->channels * lrint (input_index - rem) ;
	input_index = rem ;

	/* Main processing loop. */
	while (linear->out_gen < linear->out_count && linear->in_used + linear->channels * input_index <= linear->in_count)
	{
		if (linear->out_count > 0 && fabs (psrc->last_ratio - data->src_ratio) > SRC_MIN_RATIO_DIFF)
			src_ratio = psrc->last_ratio + linear->out_gen * (data->src_ratio - psrc->last_ratio) / linear->out_count ;

		if (SRC_DEBUG && linear->in_used < linear->channels && input_index < 1.0)
		{	printf ("Whoops!!!!   in_used : %ld     channels : %d     input_index : %f\n", linear->in_used, linear->channels, input_index) ;
			exit (1) ;
			} ;

		for (ch = 0 ; ch < linear->channels ; ch++)
		{	data->data_out_d [linear->out_gen] = (float) (data->data_in_d [linear->in_used - linear->channels + ch] + input_index *
						(data->data_in_d [linear->in_used + ch] - data->data_in_d [linear->in_used - linear->channels + ch])) ;
			linear->out_gen ++ ;
			} ;

		/* Figure out the next index. */
		input_index += 1.0 / src_ratio ;
		rem = fmod_one (input_index) ;

		linear->in_used += linear->channels * lrint (input_index - rem) ;
		input_index = rem ;
		} ;

	if (linear->in_used > linear->in_count)
	{	input_index += (linear->in_used - linear->in_count) / linear->channels ;
		linear->in_used = linear->in_count ;
		} ;

	psrc->last_position = input_index ;

	if (linear->in_used > 0)
		for (ch = 0 ; ch < linear->channels ; ch++)
			linear->last_value_d [ch] = data->data_in_d [linear->in_used - linear->channels + ch] ;

	/* Save current ratio rather then target ratio. */
	psrc->last_ratio = src_ratio ;

	data->input_frames_used = linear->in_used / linear->channels ;
	data->output_frames_gen = linear->out_gen / linear->channels ;

	return SRC_ERR_NO_ERROR ;
} /* linear_vari_process_d */


/*------------------------------------------------------------------------------
*/

const char*
linear_get_name (int src_enum)
{
	if (src_enum == SRC_LINEAR)
		return "Linear Interpolator" ;

	return NULL ;
} /* linear_get_name */

const char*
linear_get_description (int src_enum)
{
	if (src_enum == SRC_LINEAR)
		return "Linear interpolator, very fast, poor quality." ;

	return NULL ;
} /* linear_get_descrition */

int
gavl_linear_set_converter (SRC_PRIVATE *psrc, int src_enum, int d)
{	LINEAR_DATA *linear = NULL ;

	if (src_enum != SRC_LINEAR)
		return SRC_ERR_BAD_CONVERTER ;

	if (psrc->private_data != NULL)
	{	linear = (LINEAR_DATA*) psrc->private_data ;
		if (linear->linear_magic_marker != LINEAR_MAGIC_MARKER)
		{	free (psrc->private_data) ;
			psrc->private_data = NULL ;
			} ;
		} ;

	if (psrc->private_data == NULL)
	{	linear = calloc (1, sizeof (*linear) + psrc->channels * sizeof (float)) ;
		if (linear == NULL)
			return SRC_ERR_MALLOC_FAILED ;
		psrc->private_data = linear ;
		} ;

	linear->linear_magic_marker = LINEAR_MAGIC_MARKER ;
	linear->channels = psrc->channels ;
        if(d)
          {
          psrc->const_process = linear_vari_process_d ;
          psrc->vari_process = linear_vari_process_d ;
          }
        else
          {
          psrc->const_process = linear_vari_process_f ;
          psrc->vari_process = linear_vari_process_f ;
          }
        linear->d = d;
	psrc->reset = linear_reset ;

	linear_reset (psrc) ;

	return SRC_ERR_NO_ERROR ;
} /* linear_set_converter */

/*===================================================================================
*/

static void
linear_reset (SRC_PRIVATE *psrc)
{	LINEAR_DATA *linear = NULL ;

	linear = (LINEAR_DATA*) psrc->private_data ;
	if (linear == NULL)
		return ;

	linear->channels = psrc->channels ;
	linear->reset = 1 ;
        if(linear->d)
          memset (linear->last_value_d, 0, sizeof (linear->last_value_d [0]) * linear->channels) ;
        else
          memset (linear->last_value_f, 0, sizeof (linear->last_value_f [0]) * linear->channels) ;
        
} /* linear_reset */

