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



#ifndef GAVL_METATAGS_H_INCLUDED
#define GAVL_METATAGS_H_INCLUDED

/** \defgroup metatags Defined metadata keys
 *  \ingroup metadata
 *
 *  For interoperability reasons, try to use these macros
 *  as metadata keys whenever possible.
 *
 *  Since 1.4.0
 *
 * @{
 */

/** \brief Title of the song/movie... */

#define GAVL_META_TITLE       "Title"

/** \brief Title of the song/movie... */
#define GAVL_META_SEARCH_TITLE "SearchTitle"

#define GAVL_META_ORIGINAL_TITLE "OriginalTitle"

/** \brief Unique ID within this database */

#define GAVL_META_ID          "ID"
#define GAVL_META_UUID        "UUID"
#define GAVL_META_HASH        "Hash"

/* Object is locked for access */
#define GAVL_META_LOCKED      "Locked"

#define GAVL_META_NEXT_ID     "NextID"
#define GAVL_META_PREVIOUS_ID "PreviousID"

/** \brief Performing artist */

#define GAVL_META_ARTIST      "Artist"

/** \brief Artist of the Album */

#define GAVL_META_ALBUMARTIST "Albumartist"

/** \brief Author */

#define GAVL_META_AUTHOR      "Author"

/** \brief Copyright */

#define GAVL_META_COPYRIGHT   "Copyright"

/** \brief Album */

#define GAVL_META_ALBUM       "Album"

/** \brief Genre */

#define GAVL_META_GENRE       "Genre"

/** \brief Category */

#define GAVL_META_CATEGORY     "Category"


/** \brief Tracknumber within the album or (more generically) index within parent container
 *
 *  Integer starting with 1
 */

#define GAVL_META_TRACKNUMBER "Track"

/** \brief Generic date
 *
 *  YYYY-MM-DD [HH:MM:SS]
 */

#define GAVL_META_DATE        "Date"

/** \brief Creation date
 *
 *  YYYY-MM-DD [HH:MM:SS]
 */

#define GAVL_META_DATE_CREATE "CreationDate"     // YYYY-MM-DD [HH:MM:SS]

/** \brief Modification date
 *
 *  YYYY-MM-DD [HH:MM:SS]
 */

#define GAVL_META_DATE_MODIFY "ModificationDate" // YYYY-MM-DD [HH:MM:SS]

#define GAVL_META_MTIME       "mtime" // time_t (from stat()) as long

/* Size of the oject in bytes */
#define GAVL_META_TOTAL_BYTES    "totalbytes"

/** \brief Generic year
 */

#define GAVL_META_YEAR        "Year"        // YYYY

/** \brief Language
 *
 *  Use this for subtitles or audio streams
 */

#define GAVL_META_LANGUAGE    "Language"    // ISO 639-2/B 3 letter code

/** \brief Comment
 */

#define GAVL_META_COMMENT     "Comment"

/** \brief Related URL
 */

#define GAVL_META_RELURL         "RelURL"

/** \brief Referenced URL (means: Redirector)
 */

// #define GAVL_META_REFURL      "RefURL"

/** \brief Software
 *
 *  For media files, this is the multiplexer software.
 *  For single stream this is the encoder software
 */

#define GAVL_META_SOFTWARE    "Software"

/** \brief Person, who created the file
 */

#define GAVL_META_CREATOR     "Creator"

/** \brief Format
 *
 *  For media files, this is the container format.
 *  For single stream this is the name of the codec
 */
#define GAVL_META_FORMAT      "Format"

/* Some defined formats */

#define GAVL_META_FORMAT_MP3   "MP3"
#define GAVL_META_FORMAT_FLAC  "Flac"

/** \brief Label
 *
 * For streams it's the label (e.g. "Directors comments")
 * to display in the stream menu
 *
 * In global metadata it's the label which should be displayed when
 * this file is played
 */

#define GAVL_META_LABEL       "Label"

/** \brief Bitrate
 *
 *  Bitrate as integer in bits/s. Can also be a
 *  string (like VBR)
 */

#define GAVL_META_BITRATE     "Bitrate"

/* Framerate (float, exact value is in the video format) */

#define GAVL_META_FRAMERATE   "Framerate"

/** \brief Valid bits per audio sample
 */

#define GAVL_META_AUDIO_BITS  "BitsPerSample"

/** \brief Valid bits per pixel
 */

#define GAVL_META_VIDEO_BPP   "BitsPerPixel"

/** \brief H.264 profile_idc
 *
 *  Raw value from the header
 */

#define GAVL_META_PROFILE    "Profile"
#define GAVL_META_LEVEL     "Level"

/* MPEG-2 Profiles */

#define GAVL_META_MPEG2_PROFILE_422                "422"
#define GAVL_META_MPEG2_PROFILE_HIGH               "High"
#define GAVL_META_MPEG2_PROFILE_SPATIALLY_SCALABLE "SpatiallyScalable"
#define GAVL_META_MPEG2_PROFILE_SNR_SCALABLE       "SNRScalable"
#define GAVL_META_MPEG2_PROFILE_MAIN               "Main"
#define GAVL_META_MPEG2_PROFILE_SIMPLE             "Simple"

/* MPEG-2 Levels */

#define GAVL_META_MPEG2_LEVEL_HIGH                 "High"
#define GAVL_META_MPEG2_LEVEL_HIGH1440             "High1440"
#define GAVL_META_MPEG2_LEVEL_MAIN                 "Main"
#define GAVL_META_MPEG2_LEVEL_LOW                  "Low"

/* MPEG-4 Profiles */

#define GAVL_META_MPEG4_PROFILE_SIMPLE                    "Simple"
#define GAVL_META_MPEG4_PROFILE_SIMPLE_SCALABLE           "SimpleScalable"
#define GAVL_META_MPEG4_PROFILE_CORE                      "Core"
#define GAVL_META_MPEG4_PROFILE_MAIN                      "Main"
#define GAVL_META_MPEG4_PROFILE_N_BIT                     "N-Bit"
#define GAVL_META_MPEG4_PROFILE_SCALABLE_TEXTURE          "ScalableTexture"
#define GAVL_META_MPEG4_PROFILE_SIMPLE_FACE_ANIMATION     "SimpleFaceAnimation"
#define GAVL_META_MPEG4_PROFILE_BASIC_ANIMATED_TEXTURE    "BasicAnimatedTexture"
#define GAVL_META_MPEG4_PROFILE_HYBRID                    "Hybrid"
#define GAVL_META_MPEG4_PROFILE_ADVANCED_REAL_TIME        "AdvancedRealTime"
#define GAVL_META_MPEG4_PROFILE_CORE_SCALABLE             "CoreScalable"
#define GAVL_META_MPEG4_PROFILE_ADVANCED_CODING           "AdvancedCoding"
#define GAVL_META_MPEG4_PROFILE_ADVANCED_CORE             "AdvancedCore"
#define GAVL_META_MPEG4_PROFILE_ADVANCED_SCALABLE_TEXTURE "AdvancedScalableTexture"
#define GAVL_META_MPEG4_PROFILE_SIMPLE_STUDIO             "AdvancedSimpleStudio"
#define GAVL_META_MPEG4_PROFILE_ADVANCED_SIMPLE           "AdvancedSimple"


/* H.264 Profiles */
#define GAVL_META_H264_PROFILE_BASELINE             "Baseline"
#define GAVL_META_H264_PROFILE_CONSTRAINED_BASELINE "ConstrainedBaseline"
#define GAVL_META_H264_PROFILE_MAIN                 "Main"
#define GAVL_META_H264_PROFILE_EXTENDED             "Extended"
#define GAVL_META_H264_PROFILE_HIGH                 "High"
#define GAVL_META_H264_PROFILE_CONSTRAINED_HIGH     "ConstrainedHigh"
#define GAVL_META_H264_PROFILE_PROGRESSIVE_HIGH     "ProgressiveHigh"
#define GAVL_META_H264_PROFILE_HIGH_10              "High10"
#define GAVL_META_H264_PROFILE_HIGH_10_INTRA        "High10Intra"
#define GAVL_META_H264_PROFILE_HIGH_422             "High422"
#define GAVL_META_H264_PROFILE_HIGH_422_INTRA       "High422Intra"
#define GAVL_META_H264_PROFILE_HIGH_444_PREDICTIVE  "High444Predictive"
#define GAVL_META_H264_PROFILE_HIGH_444_INTRA       "High444Intra"
#define GAVL_META_H264_PROFILE_CAVLC_444_INTRA      "CAVLC444Intra"
#define GAVL_META_H264_PROFILE_SCALABLE_BASELINE             "ScalableBaseline"
#define GAVL_META_H264_PROFILE_SCALABLE_CONSTRAINED_BASELINE "ScalableConstrainedBaseline"
#define GAVL_META_H264_PROFILE_SCALABLE_HIGH                 "ScalableHigh"
#define GAVL_META_H264_PROFILE_SCALABLE_CONSTRAINED_HIGH     "ScalableConstrainedHigh"
#define GAVL_META_H264_PROFILE_SCALABLE_HIGH_INTRA           "ScalableHighIntra"
#define GAVL_META_H264_PROFILE_STEREO_HIGH                   "StereoHigh"
#define GAVL_META_H264_PROFILE_MULTIVIEW_HIGH                "MultiviewHigh"
#define GAVL_META_H264_PROFILE_MULTIVIEW_DEPTH_HIGH          "MultiviewDepthHigh"

#define GAVL_META_LEVEL "Level"

/** \brief Vendor of the device/software, which created the file
 */

#define GAVL_META_VENDOR      "Vendor"

#define GAVL_META_DISK_NAME   GAVL_META_LABEL

/** \brief Model name of the device, which created the file
 */

#define GAVL_META_DEVICE      "Device"

/** \brief Name of the station for Radio or TV streams
 */

#define GAVL_META_STATION           "Station"
#define GAVL_META_STATION_URL       "StationURL"
#define GAVL_META_LOGO_URL          "LogoURL"

/** \brief Approximate duration
 */

#define GAVL_META_APPROX_DURATION  "ApproxDuration"

/** \brief MimeType associated with an item
 */
#define GAVL_META_MIMETYPE         "MimeType"

/** \brief Location used for opening this resource
 */
#define GAVL_META_URI              "URI"

/** \brief Actual location (i.e. after http redirection)
 */
#define GAVL_META_REAL_URI         "RealURI"

/** \brief src attribute, can be either an array (for multiple sources) or a dictionary
    with at least GAVL_META_URI set
 */

#define GAVL_META_SRC              "src"


/* Total tracks of the logical parent */
#define GAVL_META_TOTAL_TRACKS     "TotalTracks"

/** \brief "1"  is big endian, 0 else
 */

#define GAVL_META_BIG_ENDIAN       "BigEndian"

/** \brief Movie Actor. Can be array for multiple entries
 */
#define GAVL_META_ACTOR            "Actor"

/** \brief Movie Director. Can be array for multiple entries
 */

#define GAVL_META_DIRECTOR         "Director"

/** \brief Country. Can be array for multiple entries
 */

#define GAVL_META_COUNTRY          "Country"


/** \brief Country (). Can be array for multiple entries
 */

// ISO 3166-1 alpha-3
#define GAVL_META_COUNTRY_CODE_3   "CountryCode3"

// ISO 3166-1 alpha-2
#define GAVL_META_COUNTRY_CODE_2   "CountryCode2"

#define GAVL_META_GROUP            "Group"


#define GAVL_META_DESCRIPTION "Description"

/** \brief Movie plot.
 */

#define GAVL_META_PLOT     GAVL_META_DESCRIPTION

/** \brief Audio languages
 *  Array of language LABELS (not ISO codes) in the root
 *  metadata dictionary
 */

#define GAVL_META_AUDIO_LANGUAGES   "AudioLanguages"

/** \brief Subtitle language
 *  Array of language LABELS (not ISO codes) in the root
 *  metadata dictionary
 */

#define GAVL_META_SUBTITLE_LANGUAGES   "SubtitleLanguages"

/* Images associated with media content */

/** \brief Cover art
 */



#define GAVL_META_COVER_URL      "CoverURL"


/* For embedded covers: Location is NULL and offset and size are given */
#define GAVL_META_COVER_EMBEDDED "CoverEmbedded"
#define GAVL_META_COVER_OFFSET   "CoverOffset"
#define GAVL_META_COVER_SIZE     "CoverSize"

#define GAVL_META_WALLPAPER_URL  "WallpaperURL"
#define GAVL_META_POSTER_URL     "PosterURL"
#define GAVL_META_ICON_URL       "IconURL"
#define GAVL_META_ICON_NAME      "IconName"

#define GAVL_META_CAN_SEEK        "CanSeek"

#define GAVL_META_CAN_PAUSE       "CanPause"
#define GAVL_META_SAMPLE_ACCURATE "SampleAccurate"

#define GAVL_META_TIME_PTS    "PTS"
#define GAVL_META_TIME_CLOCK  "Clock"
#define GAVL_META_TIME_START  "Start"

#define GAVL_META_TIME_PTS_TO_CLOCK "PTSToClock"
#define GAVL_META_TIME_PTS_TO_START "PTSToStart"

#if 0
#define GAVL_META_CLOCK_TIME_MAP  "ClockTimeMap"
#define GAVL_META_CLOCK_TIME            "ClockTime"
#define GAVL_META_CLOCK_TIME_PTS        "PTS"
#define GAVL_META_CLOCK_TIME_PTS_SCALE  "scale"
#endif

/*
 *  Several commands are expected in asynchronous mode.
 *  Details are specified in libgmerlin
 */
   
#define GAVL_META_ASYNC      "Async"

#define GAVL_META_AVG_BITRATE    "AVGBitrate"   // Float, kbps
#define GAVL_META_AVG_FRAMERATE  "AVGFramerate" // Float

/* Purely informational entries for the global metadata or per Location */
#define GAVL_META_AUDIO_CHANNELS     "Channels"
#define GAVL_META_AUDIO_SAMPLERATE   "Samplerate"
#define GAVL_META_AUDIO_BITRATE      "AudioBitrate"
#define GAVL_META_VIDEO_BITRATE      "VideoBitrate"
#define GAVL_META_AUDIO_CODEC        "AudioCodec"
#define GAVL_META_VIDEO_CODEC        "VideoCodec"

#define GAVL_META_VIDEO_ASPECT_RATIO "VideoAspectRatio" // Human readable e.g. (16:9)

#define GAVL_META_WIDTH  "w"
#define GAVL_META_HEIGHT "h"
#define GAVL_META_X      "x"
#define GAVL_META_Y      "y"

/* Specify that a file got transcoded, i.e. has not the original format */
#define GAVL_META_TRANSCODED "transcoded"


#define GAVL_META_NUM_CHILDREN            "NumChildren" // Number of children for container items
#define GAVL_META_NUM_ITEM_CHILDREN       "NumItemChildren" // Number of children for container items
#define GAVL_META_NUM_CONTAINER_CHILDREN  "NumContainerChildren" // Number of children for container items

#define GAVL_META_CHILDREN           "children" // Generic name for children of an element, which must be an array

#define GAVL_META_IDX                "idx"         // Index in parent container
#define GAVL_META_TOTAL              "total"       // Total number (maximum idx + 1)
#define GAVL_META_SHOW               "Show"        // TV Show, this item belongs to
#define GAVL_META_SEASON             "Season"      // Season, this episode belongs to (integer but can be non-continuos
#define GAVL_META_PODCAST            "Podcast"     // Name of the pocast this item belongs to

#define GAVL_META_EPISODENUMBER      "EPNum"       // Number of the Episode (starting with 1)
#define GAVL_META_RATING             "Rating"      // Rating (float, 0.0..1.0)
#define GAVL_META_PARENTAL_CONTROL   "ParentalControl" // mpaa or FSK (or whatever) rating
#define GAVL_META_TAG                "Tag" // Arbitrary tag

#define GAVL_META_NFO_FILE           "NFOFILE" // 
#define GAVL_META_TOOLTIP            "tooltip" // Tooltip to be shown in GUIs

#define GAVL_META_STREAM_DURATION  "duration"
#define GAVL_META_STREAM_FORMAT    "fmt"
#define GAVL_META_STREAM_PACKET_TIMESCALE "pscale"
#define GAVL_META_STREAM_SAMPLE_TIMESCALE "sscale"
#define GAVL_META_STREAM_ENABLED          "enabled"

#define GAVL_META_STREAM_PACKET_DURATION_DIVISOR "packet_duration_divisor"

/* Set to the absolute stream index offset by one by default
   but can be changed to anything */

#define GAVL_META_STREAM_ID               "streamid"

/* Handled inside of gavl */
#define GAVL_META_STREAM_ID_MSG_GAVF   -2
#define GAVL_META_STREAM_ID_MSG_PROGRAM -1

#define GAVL_META_STREAM_ID_MEDIA_START    1

#define GAVL_META_STREAM_COMPRESSION_INFO "cmp"


#define GAVL_META_STREAM_STATS "stats"

#define GAVL_META_STREAM_STATS_NUM_BYTES           GAVL_META_TOTAL_BYTES
#define GAVL_META_STREAM_STATS_NUM_PACKETS         "TotalPackets"
#define GAVL_META_STREAM_STATS_PTS_START           "PTSStart"
#define GAVL_META_STREAM_STATS_PTS_END             "PTSEnd"
#define GAVL_META_STREAM_STATS_PACKET_SIZE_MIN     "minsize"
#define GAVL_META_STREAM_STATS_PACKET_SIZE_MAX     "maxsize"
#define GAVL_META_STREAM_STATS_PACKET_DURATION_MIN "mindur"
#define GAVL_META_STREAM_STATS_PACKET_DURATION_MAX "maxdur"

#define GAVL_META_STREAMS      "streams"
#define GAVL_META_STREAMS_EXT  "streams-ext" // External streams with separate URIs (e.g. subtitles)
#define GAVL_META_STREAM_TYPE "streamtype"

#define GAVL_META_METADATA         "metadata"
#define GAVL_META_EDL              "edl"
#define GAVL_META_TRACKS           GAVL_META_CHILDREN // Array containing single tracks

// Additional url variables passed to the open() function
#define GAVL_META_URLVARS          "urlvars"

#define GAVL_META_CURIDX           "curidx" // Index of "current" child

#define GAVL_META_PARTS            "parts" // Parts of a multipart movie

#define GAVL_META_VARIANTS         "variants" // Multiple variants of the same content (typically with different bitrates)

// #define GAVL_META_DATA_FORMAT_MSG  "msg"
// #define GAVL_META_DATA_ROLE_EVENTS "evt"

#define GAVL_META_MSG_TIMESTAMP    "timestamp" // Timestamp of messages embedded into A/V streams


/**
 *
 *
 */

#define GAVL_META_CLASS                     "MediaClass"
#define GAVL_META_CHILD_CLASS                     "ChildClass"

/* Value for class */
#define GAVL_META_CLASS_ITEM                "item"

#define GAVL_META_CLASS_AUDIO_FILE          "item.audio"
#define GAVL_META_CLASS_VIDEO_FILE          "item.video"
#define GAVL_META_CLASS_AUDIO_DISK_TRACK    "item.audio.disktrack"
#define GAVL_META_CLASS_VIDEO_DISK_TRACK    "item.video.disktrack"
#define GAVL_META_CLASS_SONG                "item.audio.song"
#define GAVL_META_CLASS_MOVIE               "item.video.movie"
#define GAVL_META_CLASS_MOVIE_PART          "item.video.movie.part"

#define GAVL_META_CLASS_AUDIO_PODCAST_EPISODE "item.audio.podcastepisode"
#define GAVL_META_CLASS_VIDEO_PODCAST_EPISODE "item.video.podcastepisode"

#define GAVL_META_CLASS_TV_EPISODE          "item.video.episode"

/* Streams get this this type when it's not clear if they are audio or video broadcasts */
#define GAVL_META_CLASS_BROADCAST           "item.broadcast"
#define GAVL_META_CLASS_AUDIO_BROADCAST     "item.audio.broadcast"
#define GAVL_META_CLASS_VIDEO_BROADCAST     "item.video.broadcast"
#define GAVL_META_CLASS_IMAGE               "item.image"

// non-media file: This is given to filesystem objects, which contain no media
#define GAVL_META_CLASS_FILE                "item.file"     

// Location, which needs to be specified further: This is given to urls in redirector (e.g. m3u) files.
// Loading can be delayed

#define GAVL_META_CLASS_LOCATION            "item.location"

// External subtitle file
#define GAVL_META_CLASS_SUBTITLE_FILE        "item.subtitles"

// Stream comes from recording device
#define GAVL_META_CLASS_AUDIO_RECORDER      "item.recorder.audio" 
#define GAVL_META_CLASS_VIDEO_RECORDER      "item.recorder.video" 

/* Container values */
#define GAVL_META_CLASS_CONTAINER           "container"       // Generic
#define GAVL_META_CLASS_MUSICALBUM          "container.musicalbum" 
#define GAVL_META_CLASS_PHOTOALBUM          "container.photoalbum" 
#define GAVL_META_CLASS_PLAYLIST            "container.playlist"
#define GAVL_META_CLASS_PODCAST             "container.podcast"
#define GAVL_META_CLASS_CONTAINER_CATEGORY  "container.category"
#define GAVL_META_CLASS_CONTAINER_ACTOR     "container.category.actor" 
#define GAVL_META_CLASS_CONTAINER_DIRECTOR  "container.category.director" 
#define GAVL_META_CLASS_CONTAINER_ARTIST    "container.category.artist" 
#define GAVL_META_CLASS_CONTAINER_COUNTRY   "container.category.country" 
#define GAVL_META_CLASS_CONTAINER_GENRE     "container.category.genre"
#define GAVL_META_CLASS_CONTAINER_LANGUAGE  "container.category.language"
#define GAVL_META_CLASS_CONTAINER_TAG       "container.category.tag"
#define GAVL_META_CLASS_CONTAINER_YEAR      "container.category.year"
#define GAVL_META_CLASS_CONTAINER_TV        "container.tv"
#define GAVL_META_CLASS_CONTAINER_RADIO     "container.radio"
#define GAVL_META_CLASS_TV_SEASON           "container.season"
#define GAVL_META_CLASS_TV_SHOW             "container.tvshow"
#define GAVL_META_CLASS_DIRECTORY           "container.directory" // On filesystem

#define GAVL_META_CLASS_MULTITRACK_FILE     "container.multitrackfile"

/* Root Containers */
#define GAVL_META_CLASS_ROOT               "container.root" 

// Mapped locally, not part of database
#define GAVL_META_CLASS_ROOT_PLAYQUEUE "container.root.playqueue" 

#define GAVL_META_CLASS_ROOT_MUSICALBUMS   "container.root.musicalbums"
#define GAVL_META_CLASS_ROOT_SONGS         "container.root.songs" 
#define GAVL_META_CLASS_ROOT_MOVIES        "container.root.movies"
#define GAVL_META_CLASS_ROOT_TV_SHOWS      "container.root.tvshows"
#define GAVL_META_CLASS_ROOT_STREAMS       "container.root.streams"
#define GAVL_META_CLASS_ROOT_DIRECTORIES   "container.root.directories"
#define GAVL_META_CLASS_ROOT_PHOTOS        "container.root.photos"
#define GAVL_META_CLASS_ROOT_PODCASTS      "container.root.podcasts"
#define GAVL_META_CLASS_ROOT_RECORDERS     "container.root.recorders"

#define GAVL_META_CLASS_ROOT_FAVORITES     "container.root.favorites" 
#define GAVL_META_CLASS_ROOT_BOOKMARKS     "container.root.bookmarks" 
#define GAVL_META_CLASS_ROOT_LIBRARY       "container.root.library"

#define GAVL_META_CLASS_ROOT_REMOVABLE            "container.root.removable"
#define GAVL_META_CLASS_ROOT_REMOVABLE_AUDIOCD    "container.root.removable.cd.audio"
#define GAVL_META_CLASS_ROOT_REMOVABLE_VCD        "container.root.removable.cd.vcd"
#define GAVL_META_CLASS_ROOT_REMOVABLE_SVCD       "container.root.removable.cd.svcd"
#define GAVL_META_CLASS_ROOT_REMOVABLE_VIDEODVD   "container.root.removable.dvd.video"

#define GAVL_META_CLASS_ROOT_REMOVABLE_FILESYSTEM            "container.root.removable.filesystem"
#define GAVL_META_CLASS_ROOT_REMOVABLE_FILESYSTEM_HDD        "container.root.removable.filesystem.hdd"
#define GAVL_META_CLASS_ROOT_REMOVABLE_FILESYSTEM_PENDRIVE   "container.root.removable.filesystem.pendrive"
#define GAVL_META_CLASS_ROOT_REMOVABLE_FILESYSTEM_MEMORYCARD "container.root.removable.filesystem.memorycard"
#define GAVL_META_CLASS_ROOT_REMOVABLE_FILESYSTEM_MOBILE     "container.root.removable.filesystem.mobile"
#define GAVL_META_CLASS_ROOT_REMOVABLE_FILESYSTEM_CD         "container.root.removable.filesystem.cdrom"
#define GAVL_META_CLASS_ROOT_REMOVABLE_FILESYSTEM_DVD        "container.root.removable.filesystem.dvd"

/* Remote media server */
#define GAVL_META_CLASS_ROOT_SERVER        "container.root.server"

/* Non-media source related media classes */

#define GAVL_META_CLASS_BACKEND_RENDERER      "backend.renderer"
#define GAVL_META_CLASS_BACKEND_MDB           "backend.mdb"
#define GAVL_META_CLASS_BACKEND_CONTROLPANEL  "backend.controlpanel"

#define GAVL_META_CLASS_SINK_AUDIO            "sink.audio"
#define GAVL_META_CLASS_SINK_VIDEO            "sink.video"

#define GAVL_META_CLASS_WEBPAGE               "item.webpage"

/* GUI States */

// Indicate, that an error occurred during loading of a track
#define GAVL_META_GUI_ERROR          "GUIError" 

// Track is selected in a GUI
#define GAVL_META_GUI_SELECTED       "GUISelected"

// Track is current in a GUI
#define GAVL_META_GUI_CURRENT        "GUICurrent"

/**
 * @}
 */

#endif //  GAVL_METATAGS_H_INCLUDED
