
#include <string.h>

#include <gavfprivate.h>
#include <gavl/metatags.h>

/* Footer structure
 *
 * GAVFFOOT
 * array of dictionaries
 * stream footer * num_streams
 * GAVFFOOT
 * Relative start position (64 bit)
 */


#if 0
  int32_t size_min;
  int32_t size_max;
  int64_t duration_min;
  int64_t duration_max;
  int64_t pts_start;
  int64_t pts_end;

  int64_t total_bytes;   // For average bitrate 
  int64_t total_packets; // For average framerate
#endif

int gavf_footer_write(gavf_t * g)
  {
  int i;
  gavl_dictionary_t foot;
  gavl_value_t arr_val;
  gavl_array_t * arr;
  
  gavf_chunk_t footer;
  //  gavf_chunk_t tail;
  
  gavf_chunk_start(g->io, &footer, GAVF_TAG_FOOTER);
  
  gavl_dictionary_init(&foot);

  gavl_value_init(&arr_val);
  arr = gavl_value_set_array(&arr_val);
  
  for(i = 0; i < g->num_streams; i++)
    {
    gavl_value_t stream_val;
    gavl_dictionary_t * stream;
    
    gavl_value_init(&stream_val);
    stream = gavl_value_set_dictionary(&stream_val);
    gavl_stream_set_stats(stream, &g->streams[i].stats);
    gavl_array_splice_val_nocopy(arr, -1, 0, &stream_val);
    }

  gavl_dictionary_set_nocopy(&foot, GAVL_META_STREAMS, &arr_val);
  
  fprintf(stderr, "Writing footer\n");
  gavl_dictionary_dump(&foot, 2);
  
  gavl_dictionary_write(g->io, &foot);
  gavl_dictionary_free(&foot);

#if 1
  
  /* Write indices */  
  if(g->opt.flags & GAVF_OPT_FLAG_SYNC_INDEX)
    {
    if(g->opt.flags & GAVF_OPT_FLAG_DUMP_INDICES)
      gavf_sync_index_dump(&g->si);
    if(!gavf_sync_index_write(g->io, &g->si))
      return 0;
    }
  
  if(g->opt.flags & GAVF_OPT_FLAG_PACKET_INDEX)
    {
    if(g->opt.flags & GAVF_OPT_FLAG_DUMP_INDICES)
      gavf_packet_index_dump(&g->pi);
    if(!gavf_packet_index_write(g->io, &g->pi))
      return 0;
    }

#endif
  
  gavf_chunk_finish(g->io, &footer, 1);
  
  /* Write final tag */
  if((gavf_io_write_data(g->io, (uint8_t*)GAVF_TAG_TAIL, 8) < 8) ||
     !gavf_io_write_uint64f(g->io, footer.start))
    return 0;

  if(!gavf_io_write_uint64f(g->io, gavf_io_position(g->io) + 8))
    return 0;
  
  return 1;
  }

void gavf_footer_init(gavf_t * g)
  {
  int i;
  gavf_stream_t * s;

  for(i = 0; i < g->num_streams; i++)
    {
    s = g->streams + i;
    gavl_stream_stats_init(&s->stats);
    }
  }

