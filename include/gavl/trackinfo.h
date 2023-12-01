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

#ifndef GAVL_TRACKINFO_H_INCLUDED
#define GAVL_TRACKINFO_H_INCLUDED

#include <gavl/compression.h>

/* Stream information */

/* Stream types except GAVL_STREAM_NONE can be ORed */

typedef enum
  {
    GAVL_STREAM_NONE    = 0,
    GAVL_STREAM_AUDIO   = (1<<0),
    GAVL_STREAM_VIDEO   = (1<<1),
    GAVL_STREAM_TEXT    = (1<<2),
    GAVL_STREAM_OVERLAY = (1<<3),
    GAVL_STREAM_MSG     = (1<<4),
  } gavl_stream_type_t;

GAVL_PUBLIC
const char * gavl_stream_type_name(gavl_stream_type_t type);


/*
 *  Standardized info structures for media content.
 *  
 */

GAVL_PUBLIC
int gavl_track_get_num_streams(const gavl_dictionary_t * d, gavl_stream_type_t type);

GAVL_PUBLIC int
gavl_track_get_num_streams_all(const gavl_dictionary_t * d);

/* Support for external streams */
GAVL_PUBLIC gavl_dictionary_t *
gavl_track_append_external_stream(gavl_dictionary_t * track,
                                  gavl_stream_type_t type,
                                  const char * mimetype,
                                  const char * location);

GAVL_PUBLIC int
gavl_track_get_num_external_streams(const gavl_dictionary_t * d);

GAVL_PUBLIC const gavl_dictionary_t *
gavl_track_get_external_stream(const gavl_dictionary_t * d, int i);

GAVL_PUBLIC gavl_dictionary_t *
gavl_track_get_external_stream_nc(gavl_dictionary_t * d, int i);

GAVL_PUBLIC const gavl_dictionary_t *
gavl_track_get_stream_all(const gavl_dictionary_t * d, int idx);

GAVL_PUBLIC
gavl_dictionary_t *
gavl_track_add_src(gavl_dictionary_t * dict, const char * key,
                   const char * mimetype, const char * location);

GAVL_PUBLIC
const gavl_dictionary_t *
gavl_track_get_src(const gavl_dictionary_t * dict, const char * key, int idx,
                   const char ** mimetype, const char ** location);

GAVL_PUBLIC 
gavl_dictionary_t *
gavl_track_get_src_nc(gavl_dictionary_t * m, const char * key, int idx);
  
GAVL_PUBLIC 
int gavl_track_has_src(const gavl_dictionary_t * m, const char * key,
                       const char * location);

GAVL_PUBLIC
void gavl_track_from_location(gavl_dictionary_t * ret, const char * location);


GAVL_PUBLIC gavl_dictionary_t *
gavl_track_get_stream_all_nc(gavl_dictionary_t * d, int idx);

GAVL_PUBLIC gavl_stream_type_t
gavl_stream_get_type(const gavl_dictionary_t * s);


/* Get the start time of the stream */
GAVL_PUBLIC 
gavl_time_t gavl_stream_get_start_time(const gavl_dictionary_t * s);

GAVL_PUBLIC
int gavl_stream_get_sample_timescale(const gavl_dictionary_t * s);

GAVL_PUBLIC int
gavl_stream_is_enabled(const gavl_dictionary_t * s);

GAVL_PUBLIC void
gavl_stream_set_enabled(gavl_dictionary_t * s, int enabled);

GAVL_PUBLIC int
gavl_stream_get_id(const gavl_dictionary_t * s, int * id);

GAVL_PUBLIC void
gavl_stream_set_id(gavl_dictionary_t * s, int id);


GAVL_PUBLIC int
gavl_stream_is_sbr(const gavl_dictionary_t * s);

GAVL_PUBLIC int
gavl_stream_get_pts_range(const gavl_dictionary_t * s, int64_t * start, int64_t * end);

GAVL_PUBLIC gavl_dictionary_t *
gavl_track_find_stream_by_id_nc(gavl_dictionary_t * d, int id);

GAVL_PUBLIC const gavl_dictionary_t *
gavl_track_find_stream_by_id(const gavl_dictionary_t * d, int id);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_stream(const gavl_dictionary_t * d,
                                                gavl_stream_type_t type, int idx);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_stream_nc(gavl_dictionary_t * d,
                                             gavl_stream_type_t type, int idx);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_append_stream(gavl_dictionary_t * d, gavl_stream_type_t type);

/* Audio */
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_audio_stream_nc(gavl_dictionary_t * d, int i);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_audio_stream(const gavl_dictionary_t * d, int i);

GAVL_PUBLIC
int gavl_track_get_num_audio_streams(const gavl_dictionary_t * d);
  
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_append_audio_stream(gavl_dictionary_t * d);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_audio_metadata(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_audio_metadata_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
const gavl_audio_format_t * gavl_track_get_audio_format(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_audio_format_t * gavl_track_get_audio_format_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
int gavl_track_delete_audio_stream(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
void gavl_track_set_num_audio_streams(gavl_dictionary_t * d, int num);


/* Video */
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_video_stream_nc(gavl_dictionary_t * d, int i);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_video_stream(const gavl_dictionary_t * d, int i);
  
GAVL_PUBLIC
int gavl_track_get_num_video_streams(const gavl_dictionary_t * d);
  
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_append_video_stream(gavl_dictionary_t * d);

GAVL_PUBLIC
const gavl_video_format_t * gavl_track_get_video_format(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_video_format_t * gavl_track_get_video_format_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_video_metadata(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_video_metadata_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
const gavl_video_format_t * gavl_track_get_video_format(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_video_format_t * gavl_track_get_video_format_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
int gavl_track_delete_video_stream(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
void gavl_track_set_num_video_streams(gavl_dictionary_t * d, int num);

/* Text */
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_text_stream_nc(gavl_dictionary_t * d, int i);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_text_stream(const gavl_dictionary_t * d, int i);

GAVL_PUBLIC
int gavl_track_get_num_text_streams(const gavl_dictionary_t * d);
  
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_append_text_stream(gavl_dictionary_t * d);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_text_metadata(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_text_metadata_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
int gavl_track_delete_text_stream(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
void gavl_track_set_num_text_streams(gavl_dictionary_t * d, int num);


/* Overlay */
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_overlay_stream_nc(gavl_dictionary_t * d, int i);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_overlay_stream(const gavl_dictionary_t * d, int i);

GAVL_PUBLIC
int gavl_track_get_num_overlay_streams(const gavl_dictionary_t * d);
  
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_append_overlay_stream(gavl_dictionary_t * d);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_overlay_metadata(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_overlay_metadata_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
const gavl_video_format_t * gavl_track_get_overlay_format(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_video_format_t * gavl_track_get_overlay_format_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
int gavl_track_delete_overlay_stream(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
void gavl_track_set_num_overlay_streams(gavl_dictionary_t * d, int num);

/* Data */
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_msg_stream_nc(gavl_dictionary_t * d, int i);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_msg_stream(const gavl_dictionary_t * d, int i);

GAVL_PUBLIC
int gavl_track_get_num_msg_streams(const gavl_dictionary_t * d);
  
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_append_msg_stream(gavl_dictionary_t * d, int id);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_msg_metadata(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_msg_metadata_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
int gavl_track_delete_msg_stream(gavl_dictionary_t * d, int stream);

/* Delete stream by absolute index */
GAVL_PUBLIC
int gavl_track_delete_stream(gavl_dictionary_t * d, int stream);

// int gavl_track_get_stream_idx(const gavl_dictionary_t * d, gavl_stream_type_t type, int idx);

GAVL_PUBLIC
int gavl_track_stream_idx_to_abs(const gavl_dictionary_t * d, gavl_stream_type_t type, int idx);

GAVL_PUBLIC
int gavl_track_stream_idx_to_rel(const gavl_dictionary_t * d, int idx);


/* Tracks */
GAVL_PUBLIC
gavl_dictionary_t * gavl_append_track(gavl_dictionary_t*, const gavl_dictionary_t * t);

GAVL_PUBLIC
gavl_dictionary_t * gavl_prepend_track(gavl_dictionary_t*, const gavl_dictionary_t * t);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_get_track(const gavl_dictionary_t*, int idx);

GAVL_PUBLIC
gavl_dictionary_t * gavl_get_track_nc(gavl_dictionary_t*, int idx);

GAVL_PUBLIC
int gavl_get_num_tracks(const gavl_dictionary_t*);

GAVL_PUBLIC
int gavl_get_num_tracks_loaded(const gavl_dictionary_t * dict,
                               int * total);

GAVL_PUBLIC
void gavl_track_apply_footer(gavl_dictionary_t * track,
                             const gavl_dictionary_t * footer);

GAVL_PUBLIC
void gavl_track_delete_implicit_fields(gavl_dictionary_t * track);


GAVL_PUBLIC
void gavl_delete_track(gavl_dictionary_t*, int idx);

GAVL_PUBLIC
int gavl_track_can_seek(const gavl_dictionary_t * track);


GAVL_PUBLIC
int gavl_track_can_pause(const gavl_dictionary_t * track);

GAVL_PUBLIC
int gavl_track_is_async(const gavl_dictionary_t * track);

GAVL_PUBLIC
int gavl_track_set_async(gavl_dictionary_t * track, int async);

GAVL_PUBLIC
gavl_time_t gavl_track_get_pts_to_clock_time(const gavl_dictionary_t * dict);

GAVL_PUBLIC
gavl_time_t gavl_track_get_pts_to_start_time(const gavl_dictionary_t * dict);

GAVL_PUBLIC
void gavl_track_set_pts_to_clock_time(gavl_dictionary_t * dict, gavl_time_t offset);

GAVL_PUBLIC
void gavl_track_set_pts_to_start_time(gavl_dictionary_t * dict, gavl_time_t offset);

#if 0
GAVL_PUBLIC
void gavl_track_set_clock_time_map(gavl_dictionary_t * track,
                                   int64_t pts, int pts_scale, gavl_time_t clock_time);

GAVL_PUBLIC
int gavl_track_get_clock_time_map(const gavl_dictionary_t * track,
                                  int64_t * pts, int * pts_scale, gavl_time_t * clock_time);
#endif

GAVL_PUBLIC
void gavl_track_splice_children(gavl_dictionary_t * dict, int idx, int del,
                                const gavl_value_t * val);

GAVL_PUBLIC
void gavl_track_splice_children_nocopy(gavl_dictionary_t * dict, int idx, int del,
                                       gavl_value_t * val);

GAVL_PUBLIC
void gavl_track_update_children(gavl_dictionary_t * dict);

// GAVL_PUBLIC

GAVL_PUBLIC
gavl_audio_format_t * gavl_stream_get_audio_format_nc(gavl_dictionary_t*);

GAVL_PUBLIC
gavl_video_format_t * gavl_stream_get_video_format_nc(gavl_dictionary_t*);

GAVL_PUBLIC
gavl_dictionary_t * gavl_stream_get_metadata_nc(gavl_dictionary_t*);

GAVL_PUBLIC
const gavl_audio_format_t * gavl_stream_get_audio_format(const gavl_dictionary_t*);

GAVL_PUBLIC
const gavl_video_format_t * gavl_stream_get_video_format(const gavl_dictionary_t*);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_stream_get_metadata(const gavl_dictionary_t*);

/* Compression info */

GAVL_PUBLIC
int gavl_stream_get_compression_info(const gavl_dictionary_t*, gavl_compression_info_t * ret);

GAVL_PUBLIC
int gavl_track_get_audio_compression_info(const gavl_dictionary_t*, int idx, gavl_compression_info_t * ret);

GAVL_PUBLIC
int gavl_track_get_video_compression_info(const gavl_dictionary_t*, int idx, gavl_compression_info_t * ret);

GAVL_PUBLIC
int gavl_track_get_overlay_compression_info(const gavl_dictionary_t*, int idx, gavl_compression_info_t * ret);

GAVL_PUBLIC
void gavl_stream_set_compression_info(gavl_dictionary_t*, const gavl_compression_info_t * ret);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_metadata_nc(gavl_dictionary_t*);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_metadata(const gavl_dictionary_t*);


GAVL_PUBLIC
void gavl_track_compute_duration(gavl_dictionary_t * dict);

GAVL_PUBLIC
void gavl_track_finalize(gavl_dictionary_t * dict);

GAVL_PUBLIC
gavl_time_t gavl_track_get_duration(const gavl_dictionary_t * dict);

GAVL_PUBLIC
gavl_time_t gavl_track_get_display_offset(const gavl_dictionary_t * dict);

GAVL_PUBLIC
void gavl_track_set_duration(gavl_dictionary_t * dict, gavl_time_t dur);

GAVL_PUBLIC
void gavl_track_set_label(gavl_dictionary_t * dict);

GAVL_PUBLIC
void gavl_set_current_track(gavl_dictionary_t * dict, int idx);

GAVL_PUBLIC
int gavl_get_current_track(gavl_dictionary_t * dict);

GAVL_PUBLIC
void gavl_track_set_gui_state(gavl_dictionary_t * track, const char * state, int val);

GAVL_PUBLIC
void gavl_track_copy_gui_state(gavl_dictionary_t * dst, const gavl_dictionary_t * src);

GAVL_PUBLIC
int gavl_track_get_gui_state(const gavl_dictionary_t * track, const char * state);

GAVL_PUBLIC
void gavl_track_clear_gui_state(gavl_dictionary_t * track);

GAVL_PUBLIC
gavl_array_t * gavl_get_tracks_nc(gavl_dictionary_t * dict);

GAVL_PUBLIC
const gavl_array_t * gavl_get_tracks(const gavl_dictionary_t * dict);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_get_track_by_id(const gavl_dictionary_t * dict, const char * id);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_get_track_by_string(const gavl_dictionary_t * dict,
                                                   const char * tag, const char * val);


GAVL_PUBLIC
const gavl_dictionary_t * gavl_get_track_by_id_arr(const gavl_array_t * arr, const char * id);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_get_track_by_string_arr(const gavl_array_t * arr,
                                                       const char * tag, const char * val);

GAVL_PUBLIC
int gavl_get_track_idx_by_id_arr(const gavl_array_t * arr, const char * id);



GAVL_PUBLIC
gavl_dictionary_t * gavl_get_track_by_id_arr_nc(gavl_array_t * arr, const char * id);


GAVL_PUBLIC
gavl_dictionary_t * gavl_get_track_by_id_nc(gavl_dictionary_t * dict, const char * id);

GAVL_PUBLIC
int gavl_get_track_idx_by_id(const gavl_dictionary_t * dict, const char * id);

GAVL_PUBLIC
const char * gavl_track_get_id(const gavl_dictionary_t * dict);

GAVL_PUBLIC
const char * gavl_track_get_media_class(const gavl_dictionary_t * dict);

GAVL_PUBLIC
void gavl_track_set_id_nocopy(gavl_dictionary_t * dict, char * id);

GAVL_PUBLIC
void gavl_track_set_id(gavl_dictionary_t * dict, const char * id);

GAVL_PUBLIC
void gavl_sort_tracks_by_label(gavl_array_t * children);

GAVL_PUBLIC
void gavl_sort_tracks_by_bitrate(gavl_array_t * children);

/* Highest first */
GAVL_PUBLIC
void gavl_sort_tracks_by_quality(gavl_array_t * children);

GAVL_PUBLIC
int gavl_track_get_num_children(const gavl_dictionary_t * track);

GAVL_PUBLIC
int gavl_track_get_num_item_children(const gavl_dictionary_t * track);

GAVL_PUBLIC
int gavl_track_get_num_container_children(const gavl_dictionary_t * track);

GAVL_PUBLIC
void gavl_track_set_num_children(gavl_dictionary_t * track,
                                 int num_container_children,
                                 int num_item_children);

GAVL_PUBLIC
void gavl_track_set_lock(gavl_dictionary_t * track,
                         int lock);

GAVL_PUBLIC
int gavl_track_is_locked(const gavl_dictionary_t * track);

GAVL_PUBLIC
const char * gavl_get_country_label(const char * iso);

GAVL_PUBLIC
void gavl_track_set_countries(gavl_dictionary_t * track);

GAVL_PUBLIC
const char * gavl_get_country_code_2_from_label(const char * label);

GAVL_PUBLIC
const char * gavl_get_country_code_3_from_label(const char * label);

GAVL_PUBLIC
void gavl_init_audio_stream(gavl_dictionary_t * dict);

GAVL_PUBLIC
void gavl_init_video_stream(gavl_dictionary_t * dict);

GAVL_PUBLIC
void gavl_init_text_stream(gavl_dictionary_t * dict);

GAVL_PUBLIC
void gavl_init_overlay_stream(gavl_dictionary_t * dict);

/* Generate pages of results */

GAVL_PUBLIC
void gavl_track_get_page_parent(const gavl_dictionary_t * parent_orig,
                                 int group_size,
                                 gavl_dictionary_t * parent_ret);

GAVL_PUBLIC
void gavl_track_get_page(const gavl_dictionary_t * parent_orig,
                         int group_size,
                         int idx,
                         gavl_dictionary_t * ret);

GAVL_PUBLIC
void gavl_track_get_page_children_start(const gavl_dictionary_t * parent_orig,
                                        int group_size,
                                        int idx,
                                        int * start, int * len);

GAVL_PUBLIC
void gavl_track_get_page_children_end(const gavl_dictionary_t * parent_orig,
                                      int group_size,
                                      int idx,
                                      int start, int len,
                                      gavl_array_t * ret);

/* If the track has more than 1 URIs, set GAVL_META_MULTIVARIANT
   and sort URIs for descreasing quality (best first) */
   
// GAVL_PUBLIC
// void gavl_track_set_multivariant(gavl_dictionary_t * dict);

// Multivariant support
GAVL_PUBLIC
int gavl_track_get_num_variants(const gavl_dictionary_t * dict);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_variant(const gavl_dictionary_t * dict, int idx);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_append_variant(gavl_dictionary_t * dict, const char * mimetype, const char * location);

GAVL_PUBLIC
void gavl_track_sort_variants(gavl_dictionary_t * dict);

// Multipart support
GAVL_PUBLIC
int gavl_track_get_num_parts(const gavl_dictionary_t * dict);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_append_part(gavl_dictionary_t * dict, const char * mimetype, const char * location);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_part(const gavl_dictionary_t * dict, int i);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_part_nc(gavl_dictionary_t * dict, int i);

//

GAVL_PUBLIC
gavl_time_t gavl_track_get_start_time(const gavl_dictionary_t * dict);

/* Compression tags (like AVI four character codes etc.) */

GAVL_PUBLIC
int gavl_stream_get_compression_tag(const gavl_dictionary_t * s);

GAVL_PUBLIC
void gavl_stream_set_compression_tag(gavl_dictionary_t * s, int tag);

GAVL_PUBLIC
void gavl_stream_set_default_packet_timescale(gavl_dictionary_t * s);

GAVL_PUBLIC
void gavl_stream_set_sample_timescale(gavl_dictionary_t * s);

GAVL_PUBLIC
void gavl_stream_set_audio_bits(gavl_dictionary_t * s, int bits);

GAVL_PUBLIC
int gavl_stream_get_audio_bits(const gavl_dictionary_t * s);

GAVL_PUBLIC
int gavl_stream_is_continuous(const gavl_dictionary_t * s);


#define GAVL_MK_FOURCC(a, b, c, d) ((a<<24)|(b<<16)|(c<<8)|d)
#define GAVL_WAVID_2_FOURCC(id)    GAVL_MK_FOURCC(0x00, 0x00, (id>>8), (id&0xff))
#define GAVL_FOURCC_2_WAVID(f)     (f & 0xffff)

/* */

#endif // GAVL_TRACKINFO_H_INCLUDED
