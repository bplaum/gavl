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


/* This file was taken from libgdither, original copyright below */

#ifndef NOISE_H
#define NOISE_H

/* Can be overrriden with any code that produces whitenoise between 0.0f and
 * 1.0f, eg (random() / (float)RAND_MAX) should be a good source of noise, but
 * its expensive */
#ifndef GDITHER_NOISE
#define GDITHER_NOISE gdither_noise()
#endif

#include <stdint.h>

inline static float gdither_noise()
{
    static uint32_t rnd = 23232323;
    rnd = (rnd * 196314165) + 907633515;

    return rnd * 2.3283064365387e-10f;
}

#endif
