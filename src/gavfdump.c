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

#include <gavf.h>

int main(int argc, char ** argv)
  {
  gavf_io_t * io;
  gavf_t * dec;
  gavf_options_t * opt;
  FILE * f;
  gavl_packet_t p;
  const gavf_packet_header_t * h;
  
  dec = gavf_create();

  opt = gavf_get_options(dec);

  gavf_options_set_flags(opt,
                         GAVF_OPT_FLAG_DUMP_HEADERS |
                         GAVF_OPT_FLAG_DUMP_INDICES |
                         GAVF_OPT_FLAG_DUMP_PACKETS);

  f = fopen(argv[1], "rb");
  if(!f)
    {
    fprintf(stderr, "Opening file failed\n");
    return 0;
    }

  io = gavf_io_create_file(f, 0, 1, 1);
  
  if(!gavf_open_read(dec, io))
    {
    fprintf(stderr, "Opening decoder failed\n");
    return 0;
    }

  gavl_packet_init(&p);
  
  while(1)
    {
    if(!(h = gavf_packet_read_header(dec)) ||
       !gavf_packet_read_packet(dec, &p))
      break;
    }
  
  /* Cleanup */

  gavf_close(dec, 0);
  gavl_packet_free(&p);
  return 0;
  }
