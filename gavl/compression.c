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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <config.h>

#include <gavl/gavl.h>
#include <gavl/compression.h>
#include <gavl/utils.h>
#include <gavl/numptr.h>
#include <gavl/log.h>
#define LOG_DOMAIN "compression"


void gavl_compression_info_free(gavl_compression_info_t * info)
  {
  gavl_buffer_free(&info->codec_header);
  }

void gavl_compression_info_init(gavl_compression_info_t * info)
  {
  memset(info, 0, sizeof(*info));
  }



#define FLAG_SEPARATE           (1<<0)
#define FLAG_NEEDS_PIXELFORMAT  (1<<1)
#define FLAG_CFS                (1<<2) // Constant Frame Samples

struct
  {
  gavl_codec_id_t id;
  const char * extension;
  const char * short_name; // Has no spaces
  const char * long_name;
  const char * mimetype;
  int flags;
  int sample_size;
  }
compression_ids[] =
  {
    /* Audio */
    { GAVL_CODEC_ID_ALAW,      NULL,       "alaw",   "alaw",         "audio/x-alaw", 0,       1 },
    { GAVL_CODEC_ID_ULAW,      NULL,       "ulaw",   "ulaw",         "audio/x-mulaw", 0,       1 },
    { GAVL_CODEC_ID_MP2,       "mp2",      "mp2",    "MPEG layer 2", "audio/mpeg", FLAG_CFS },
    { GAVL_CODEC_ID_MP3,       "mp3",      "mp3",    "MPEG layer 3", "audio/mpeg", FLAG_CFS },
    { GAVL_CODEC_ID_AC3,       "ac3",      "ac3",    "AC3",          "audio/x-ac3",FLAG_CFS },
    { GAVL_CODEC_ID_AAC,       NULL,       "aac",    "AAC",          NULL, FLAG_CFS },
    { GAVL_CODEC_ID_VORBIS,    NULL,       "vorbis", "Vorbis",       "audio/x-vorbis", },
    { GAVL_CODEC_ID_FLAC,      NULL,       "flac",   "Flac",         "audio/x-flac", },
    { GAVL_CODEC_ID_OPUS,      NULL,       "opus",   "Opus",         "audio/opus", },
    { GAVL_CODEC_ID_SPEEX,     NULL,       "speex",  "Speex",        "audio/x-speex", FLAG_CFS },
    { GAVL_CODEC_ID_DTS,       NULL,       "dts",    "DTS",          NULL, },
    
    /* Video */
    { GAVL_CODEC_ID_JPEG,      "jpg",      "jpeg",   "JPEG image",    "image/jpeg", FLAG_SEPARATE | FLAG_NEEDS_PIXELFORMAT },
    { GAVL_CODEC_ID_PNG,       "png",      "png",    "PNG image",     "image/png",  FLAG_SEPARATE | FLAG_NEEDS_PIXELFORMAT },
    { GAVL_CODEC_ID_TIFF,      "tif",      "tiff",   "TIFF image",    "image/tiff", FLAG_SEPARATE | FLAG_NEEDS_PIXELFORMAT },
    { GAVL_CODEC_ID_TGA,       "tga",      "tga",    "TGA image",     "image/x-tga",  FLAG_SEPARATE | FLAG_NEEDS_PIXELFORMAT },
    { GAVL_CODEC_ID_MPEG1,     "mpv",      "mpeg1",  "MPEG-1",        "video/mpeg", },
    { GAVL_CODEC_ID_MPEG2,     "mpv",      "mpeg2",  "MPEG-2",        "video/mpeg", FLAG_NEEDS_PIXELFORMAT },
    { GAVL_CODEC_ID_MPEG4_ASP, "m4v",      "mpeg4",  "MPEG-4",        NULL, }, // ISO/IEC 14496-2
    { GAVL_CODEC_ID_H264,      "h264",     "h264",   "H.264",         NULL, },
    { GAVL_CODEC_ID_THEORA,    NULL,       "theora", "Theora",        NULL, },
    { GAVL_CODEC_ID_DIRAC,     NULL,       "dirac",  "Dirac",         "video/x-dirac", },
    { GAVL_CODEC_ID_DV,        "dv",       "dv",     "DV",            NULL, FLAG_NEEDS_PIXELFORMAT },
    { GAVL_CODEC_ID_VP8,       NULL,       "vp8",    "VP8",           "video/x-vp8", },
    { GAVL_CODEC_ID_DIV3,      NULL,       "divx3",  "DivX 3",        NULL, },
    { GAVL_CODEC_ID_DVDSUB,    NULL,       "dvdsub", "DVD subtitles", NULL, },

  };

#define NUM_CODEC_IDS (sizeof(compression_ids)/sizeof(compression_ids[0]))

const char * gavl_compression_get_extension(gavl_codec_id_t id, int * separate)
  {
  int i;
  for(i = 0; i < NUM_CODEC_IDS; i++)
    {
    if(compression_ids[i].id == id)
      {
      if(separate)
        *separate = !!(compression_ids[i].flags & FLAG_SEPARATE);
      return compression_ids[i].extension;
      }
    }
  return NULL;
  }

static const char * MIME_AACP = "audio/aacp";
static const char * MIME_AAC  = "audio/aac";

const char * gavl_compression_get_mimetype(const gavl_compression_info_t * ci)
  {
  int i;
  for(i = 0; i < NUM_CODEC_IDS; i++)
    {
    if(compression_ids[i].id == ci->id)
      {
      if(compression_ids[i].mimetype)
        return compression_ids[i].extension;
      break;
      }
    }

  /* Special cases */

  if(compression_ids[i].id == GAVL_CODEC_ID_AAC)
    {
    if(ci->flags & GAVL_COMPRESSION_SBR)
      return MIME_AACP;
    else
      return MIME_AAC;
    }
  return NULL;
  }

int gavl_compression_get_sample_size(gavl_codec_id_t id)
  {
  int i;
  for(i = 0; i < NUM_CODEC_IDS; i++)
    {
    if(compression_ids[i].id == id)
      {
      return compression_ids[i].sample_size;
      }
    }
  return 0;
  }

int gavl_compression_need_pixelformat(gavl_codec_id_t id)
  {
  int i;
  for(i = 0; i < NUM_CODEC_IDS; i++)
    {
    if(compression_ids[i].id == id)
      return !!(compression_ids[i].flags & FLAG_NEEDS_PIXELFORMAT);
    }
  return 0;
  }

int gavl_compression_constant_frame_samples(gavl_codec_id_t id)
  {
  int i;
  for(i = 0; i < NUM_CODEC_IDS; i++)
    {
    if(compression_ids[i].id == id)
      return !!(compression_ids[i].flags & FLAG_CFS);
    }
  return 0;
  }


const char *
gavl_compression_get_long_name(gavl_codec_id_t id)
  {
  int i;
  for(i = 0; i < NUM_CODEC_IDS; i++)
    {
    if(compression_ids[i].id == id)
      return compression_ids[i].long_name;
    }
  return NULL;
  }

const char *
gavl_compression_get_short_name(gavl_codec_id_t id)
  {
  int i;
  for(i = 0; i < NUM_CODEC_IDS; i++)
    {
    if(compression_ids[i].id == id)
      return compression_ids[i].short_name;
    }
  return NULL;
  
  }

gavl_codec_id_t 
gavl_compression_from_short_name(const char * name)
  {
  int i;
  for(i = 0; i < NUM_CODEC_IDS; i++)
    {
    if(!strcmp(compression_ids[i].short_name, name))
      return compression_ids[i].id;
    }
  return GAVL_CODEC_ID_NONE;
  }

int gavl_num_compressions()
  {
  return NUM_CODEC_IDS;
  }

gavl_codec_id_t gavl_get_compression(int index)
  {
  return compression_ids[index].id;
  }

void gavl_compression_info_dump(const gavl_compression_info_t * info)
  {
  gavl_compression_info_dumpi(info, 0);
  }

static void do_indent(int num)
  {
  int i;
  for(i = 0; i < num; i++)
    fprintf(stderr, " ");
  }

void gavl_compression_info_dumpi(const gavl_compression_info_t * info, int indent)
  {
  do_indent(indent);
  fprintf(stderr, "Compression info\n");
  do_indent(indent+2);
  fprintf(stderr, "Codec:           %s [%s]\n",
          gavl_compression_get_long_name(info->id),
          gavl_compression_get_short_name(info->id));
  do_indent(indent+2);
  if(info->bitrate == GAVL_BITRATE_VBR)
    fprintf(stderr, "Bitrate:         Variable\n");
  else if(!info->bitrate)
    fprintf(stderr, "Bitrate:         Unknown\n");
  else
    fprintf(stderr, "Bitrate:         %d bps\n", info->bitrate);

  if(info->id < 0x10000)
    {
    do_indent(indent+2);
    fprintf(stderr, "pre_skip:        %d\n", info->pre_skip);
    }
  if(info->id >= 0x10000)
    {
    do_indent(indent+2);
    fprintf(stderr, "Palette size:    %d\n", info->palette_size);

    do_indent(indent+2);
    fprintf(stderr, "VBV size:        %d bytes\n",
            info->video_buffer_size);

    do_indent(indent+2);
    fprintf(stderr, "Frame types:     I");
    if(info->flags & GAVL_COMPRESSION_HAS_P_FRAMES)
      fprintf(stderr, ",P");
    if(info->flags & GAVL_COMPRESSION_HAS_B_FRAMES)
      fprintf(stderr, ",B");
    fprintf(stderr, "\n");
    }
  else
    {
    do_indent(indent+2);
    fprintf(stderr, "SBR:             %s\n",
            (info->flags & GAVL_COMPRESSION_SBR ? "Yes" : "No"));
    }

  
  do_indent(indent+2);
  fprintf(stderr, "Global header %d bytes", info->codec_header.len);

  if(info->codec_header.len && (info->codec_header.len < 256))
    {
    fprintf(stderr, " (hexdump follows)\n");
    gavl_hexdumpi(info->codec_header.buf,
                 info->codec_header.len, 16, indent+2);
    }
  else
    fprintf(stderr, "\n");
  }

void gavl_compression_info_copy(gavl_compression_info_t * dst,
                                const gavl_compression_info_t * src)
  {
  if(!src)
    {
    memset(dst, 0, sizeof(*dst));
    return;
    }
  
  memcpy(dst, src, sizeof(*dst));
  
  if(src->codec_header.len)
    {
    gavl_buffer_init(&dst->codec_header);
    gavl_buffer_copy(&dst->codec_header, &src->codec_header);
    }
  }


void gavl_compression_info_set_global_header(gavl_compression_info_t * dst,
                                             const uint8_t * data, int len)
  {
  gavl_buffer_reset(&dst->codec_header);
  gavl_buffer_append_data_pad(&dst->codec_header, data, len, GAVL_PACKET_PADDING);
  }

void gavl_compression_info_append_global_header(gavl_compression_info_t * dst,
                                                const uint8_t * data, int len)
  {
  gavl_buffer_append_data_pad(&dst->codec_header, data, len, GAVL_PACKET_PADDING);
  }



/* Xiph style lacing */

#if 0
// Format of xiph lacing:
// <num_laces - 1>
// 255 255 233
// 255 255 123
// Segment 1
// Segment 2

static uint8_t * get_xiph_len(uint8_t * ptr, int * len)
  {
  *len = 0;
  while(*ptr == 255)
    {
    *len += 255;
    ptr++;
    }
  *len += *ptr;
  ptr++;
  return ptr;
  }

typedef struct
  {
  int len;
  uint8_t * ptr;
  } xiph_buffer_t;

static int xiph_buffer_get(xiph_buffer_t * ret,
                           uint8_t * global_header, int global_header_len)
  {
  int i;
  uint8_t * ptr = global_header;
  int num_segments = (*ptr) + 1;
  ptr++;

  for(i = 0; i < num_segments-1; i++)
    {
    ptr = get_xiph_len(ptr, &ret[i].len);
    if(ptr - global_header > global_header_len)
      return 0;
    }
  for(i = 0; i < num_segments-1; i++)
    {
    ret[i].ptr = ptr;
    ptr += ret[i].len;
    if(ptr - global_header > global_header_len)
      return 0;
    }
  
  ret[num_segments-1].ptr = ptr;
  ret[num_segments-1].len = global_header_len - (ptr - global_header);

  if(ret[num_segments-1].len <= 0)
    return 0;
  return 1;
  }

static uint8_t * xiph_buffer_put(xiph_buffer_t * buf, int num,
                                 uint32_t * global_header_len)
  {
  int len;
  uint8_t * ret;
  uint8_t * ptr;
  
  int i;
  *global_header_len = 1;

  for(i = 0; i < num-1; i++)
    {
    *global_header_len += buf[i].len / 255 + 1;
    *global_header_len += buf[i].len;
    }

  *global_header_len += buf[num-1].len;

  ret = malloc(*global_header_len + GAVL_PACKET_PADDING);

  ptr = ret;
  *ptr = num - 1;
  ptr++;
  
  for(i = 0; i < num-1; i++)
    {
    len = buf[i].len;

    while(len >= 255)
      {
      *ptr++ = 255;
      len -= 255;
      }
    *ptr++ = len;
    }

  for(i = 0; i < num; i++)
    {
    memcpy(ptr, buf[i].ptr, buf[i].len);
    ptr += buf[i].len;
    }
  memset(ptr, 0, GAVL_PACKET_PADDING);
  return ret;
  }
#endif

void gavl_append_xiph_header(gavl_buffer_t * codec_header,
                             uint8_t * header,
                             int header_len)
  {
  gavl_buffer_alloc(codec_header, codec_header->len + header_len + 4);
  GAVL_32BE_2_PTR(header_len, codec_header->buf + codec_header->len);
  memcpy(codec_header->buf + codec_header->len + 4, header, header_len);
  codec_header->len += 4 + header_len;
  }

uint8_t * gavl_extract_xiph_header(gavl_buffer_t * codec_header,
                                   int idx,
                                   int * header_len)
  {
  int i;
  int len;
  uint8_t * ptr = codec_header->buf;
  uint8_t * end = codec_header->buf + codec_header->len;

  if(codec_header->len < 4)
    {
    /* Error */
    return NULL;
    }

  len = GAVL_PTR_2_32BE(ptr);
  ptr += 4;
  
  for(i = 0; i < idx; i++)
    {
    ptr += len;
        
    if(end - ptr < 4)
      {
      /* Error */
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot extract xiph header: Buffer too small");
      return NULL;
      }
    
    len = GAVL_PTR_2_32BE(ptr);
    ptr += 4;
    
    if(end - ptr < len)
      {
      /* Error */
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Cannot extract xiph header: Buffer too small");
      return NULL;
      }
    }
  *header_len = len;
  return ptr;
  }

void
gavl_packet_to_videoframe(const gavl_packet_t * p, 
                          gavl_video_frame_t * f)
  {
  f->timestamp = p->pts;
  f->duration = p->duration;
  f->timecode = p->timecode;
  f->interlace_mode = p->interlace_mode;
  gavl_rectangle_i_copy(&f->src_rect, &p->src_rect);
  f->dst_x = p->dst_x;
  f->dst_y = p->dst_y;

  }
                          
