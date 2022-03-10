

#ifndef GAVL_HW_SHM_H_INCLUDED
#define GAVL_HW_SHM_H_INCLUDED

typedef struct
  {
  struct
    {
    int fd;
    } buffers[GAVL_MAX_PLANES];
  
  struct
    {
    int buf_idx;
    int64_t offset;
    } planes[GAVL_MAX_PLANES];
  
  uint32_t fourcc; // drm fourcc
  
  int num_buffers;
  int num_planes;
  } gavl_dmabuf_video_frame_t;

GAVL_PUBLIC gavl_hw_context_t * gavl_hw_ctx_create_shm(int wr);

#endif // GAVL_HW_SHM_H_INCLUDED
