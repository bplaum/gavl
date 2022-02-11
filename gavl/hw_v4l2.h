
typedef struct gavl_v4l_device_s gavl_v4l_device_t;

gavl_v4l_device_t * gavl_v4l_device_open(const char * dev);

int gavl_v4l_device_reads_video(gavl_v4l_device_t *);
int gavl_v4l_device_writes_video(gavl_v4l_device_t *);

int gavl_v4l_device_reads_packets(gavl_v4l_device_t *);
int gavl_v4l_device_writes_packets(gavl_v4l_device_t *);

gavl_packet_sink_t * gavl_v4l_device_get_packet_sink(gavl_v4l_device_t * dev);
gavl_packet_source_t * gavl_v4l_device_get_packet_source(gavl_v4l_device_t * dev);
gavl_video_sink_t * gavl_v4l_device_get_video_sink(gavl_v4l_device_t * dev);
gavl_video_source_t * gavl_v4l_device_get_video_source(gavl_v4l_device_t * dev);
 

int v4l_device_init_capture(gavl_v4l_device_t * dev,
                            gavl_video_format_t * fmt);

int v4l_device_init_output(gavl_v4l_device_t * dev,
                           gavl_video_format_t * fmt);

int v4l_device_init_encoder(gavl_v4l_device_t * dev,
                            gavl_video_format_t * fmt,
                            gavl_compression_info_t * cmp);

int v4l_device_init_decoder(gavl_v4l_device_t * dev,
                            gavl_video_format_t * fmt,
                            const gavl_compression_info_t * cmp);

