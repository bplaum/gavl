#include <stdlib.h>
#include <string.h>

#include <gavfprivate.h>

#define INCREMENT 8

struct gavf_packet_buffer_s
  {
  gavl_packet_t ** packets;
  int num_packets;
  int packets_alloc;
  int timescale;

  gavf_packet_unref_func unref_func;
  void *                 unref_data;
  };

void gavf_packet_buffer_set_unref_func(gavf_packet_buffer_t * b,
                                       gavf_packet_unref_func unref_func,
                                       void *                 unref_data)
  {
  b->unref_func = unref_func;
  b->unref_data = unref_data;
  }

void gavf_packet_buffer_clear(gavf_packet_buffer_t * b)
  {
  int i;

  if(b->unref_func)
    {
    for(i = 0; i < b->num_packets; i++)
      b->unref_func(b->packets[i], b->unref_data);
    }
  
  b->num_packets = 0;
  }

gavf_packet_buffer_t * gavf_packet_buffer_create(int timescale)
  {
  gavf_packet_buffer_t * ret = calloc(1, sizeof(*ret));
  ret->timescale = timescale;
  return ret;
  }

gavl_packet_t * gavf_packet_buffer_get_write(gavf_packet_buffer_t * b)
  {
  gavl_packet_t * ret;
  if(b->packets_alloc - b->num_packets <= 0)
    {
    int i;
    b->packets =
      realloc(b->packets, (b->packets_alloc+INCREMENT) * sizeof(*b->packets));

    for(i = b->packets_alloc; i < b->packets_alloc+INCREMENT; i++)
      b->packets[i] = calloc(1, sizeof(*b->packets[i]));
    b->packets_alloc+=INCREMENT;
    }
  ret = b->packets[b->num_packets];
  gavl_packet_reset(ret);
  return ret;
  }

void gavf_packet_buffer_done_write(gavf_packet_buffer_t * b)
  {
  b->num_packets++;
  }

gavl_packet_t * gavf_packet_buffer_get_read(gavf_packet_buffer_t * b)
  {
  gavl_packet_t * ret;
  if(!b->num_packets)
    return NULL;
  ret = b->packets[0];
  
  b->num_packets--;
  if(b->num_packets)
    memmove(b->packets, b->packets+1, b->num_packets * sizeof(*b->packets));

  /* Requeue */
  b->packets[b->num_packets] = ret;
  
  return ret;
  }

gavl_packet_t * gavf_packet_buffer_peek_read(gavf_packet_buffer_t * b)
  {
  if(!b->num_packets)
    return NULL;
  //  if(b->packets[0]->data_len == 0)
  //    return NULL;
  return b->packets[0];
  }

gavl_time_t gavf_packet_buffer_get_min_pts(gavf_packet_buffer_t * b)
  {
  int i;
  gavl_time_t ret = GAVL_TIME_UNDEFINED;
  
  if(!b->num_packets)
    return ret;

  //  if(!b->packets[0]->data_len)
  //    return ret;
  
  for(i = 0; i < b->num_packets; i++)
    {
    if(!i || (ret > b->packets[i]->pts))
      ret = b->packets[i]->pts;
    }
  return gavl_time_unscale(b->timescale, ret);
  }

void gavf_packet_buffer_destroy(gavf_packet_buffer_t * b)
  {
  int i;
  for(i = 0; i < b->packets_alloc; i++)
    {
    gavl_packet_free(b->packets[i]);
    free(b->packets[i]);
    }
  if(b->packets)
    free(b->packets);
  free(b);
  }

const gavl_packet_t * gavf_packet_buffer_get_last(gavf_packet_buffer_t * b)
  {
  if(!b->num_packets)
    return NULL;
  else
    return b->packets[b->num_packets-1];
  }

void gavf_packet_buffer_remove_last(gavf_packet_buffer_t * b)
  {
  if(b->num_packets)
    b->num_packets--;
  }

