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
#include <gavl.h>
#include <stdio.h>

int main(int argc, char ** argv)
  {
  int i, j, num;
  gavl_pixelformat_t src, dst;
  num = gavl_num_pixelformats();

  for(i = 0; i < num; i++)
    {
    src = gavl_get_pixelformat(i);
    for(j = 0; j < num; j++)
      {
      dst = gavl_get_pixelformat(j);
      fprintf(stderr, "src: %-23s, dst: %-23s, penalty: %6d\n",
              gavl_pixelformat_to_string(src),
              gavl_pixelformat_to_string(dst),
              gavl_pixelformat_conversion_penalty(src, dst));
      }
    }
  return 0;
  }
