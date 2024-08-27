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



#include <gavl/gavl.h>
#include <gavl/compression.h>
#include <gavl/value.h>

#include <gavl/hw_v4l2.h>


int main()
  {
  gavl_array_t arr;
  gavl_array_init(&arr);

  gavl_v4l2_devices_scan(&arr);

  gavl_array_dump(&arr, 0);
  gavl_array_free(&arr);
  
  }

  
