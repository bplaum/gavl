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

#ifndef GAVL_MSG_H_INCLUDED
#define GAVL_MSG_H_INCLUDED

#include <gavl/gavldefs.h>
#include <gavl/value.h>

#define GAVL_MSG_HEADER_TIMESTAMP "timestamp" // Always GAVL_TIME_SCALE

#define GAVL_MSG_NONE     -1 //!< Reserved ID for non valid message
#define GAVL_MSG_MAX_ARGS  16 //!< Maximum number of args

#define GAVL_MSG_NS_LOG       1

#define GAVL_MSG_NS_GENERIC   2
#define GAVL_MSG_NS_SRC       3
#define GAVL_MSG_NS_GUI       4
#define GAVL_MSG_NS_GAVF      5

/* Message IDs */

/*
 *  GAVL_MSG_NS_GENERIC
 */

/** \brief Quit
 */

#define GAVL_CMD_QUIT      1

/** \brief Ping
 *  The other end responds with pong. This is used to keep the
 *  connections alive
 */

#define GAVL_CMD_PING      2


/** \brief Quit (acknowledge)
 */

#define GAVL_MSG_QUIT      100


#define GAVL_MSG_PONG      101


/** \brief Generic progress callback
 *
 *  arg1: Activity (string) 
 *  arg0: Percentage (0.0..1.0)
 */

#define GAVL_MSG_PROGRESS      101

/*
 *  GAVL_MSG_NS_SRC
 */

/** \brief Global metadata changed
 *
 *  header field GAVL_META_MSG_TIMESTAMP: timestamp (in us)
 *
 *  arg0: Metadata
 */

#define GAVL_MSG_SRC_METADATA_CHANGED  1

/** \brief Pixel aspect ratio changed
 *
 *  arg0: time (time)
 *  arg1: scale (int)
 *  arg2: Stream
 *  arg3: new pixel width
 *  arg4: new pixel width
 */

#define GAVL_MSG_SRC_ASPECT_CHANGED    2

/** \brief Buffering notification
 *
 *  arg0: Percentage (float), negative means buffering finished
 */

#define GAVL_MSG_SRC_BUFFERING        3

/** \brief Authentication request
 *
 *  arg0: String (e.g. domain name) (string)
 */

#define GAVL_MSG_SRC_AUTHENTICATE     4

/** \brief Resync
 *
 *  Sent after seeking. Must be the last packet before the new
 *  sync header for GAVF streams. Scale is taken from the seek command.
 *
 *  This can be sent asynchronously to update the timestamps e.g. after packet loss.
 *
 *  After this event was sent, all sources are set to EOF to interrupt the playback.
 *  The caller then adjusts the time accordingly and calls bg_media_source_set_eof with
 *  zero argument to clear the EOF state of the source.
 *
 *  arg0: Time  (long)
 *  arg1: Scale (int)
 *  arg2: Discard (int)
 *  arg3: Discont (int)
 *
 *  Discard means to throw away all packets *before* this message
 *  Discont means to reset the decoders *after* this message
 */

#define GAVL_MSG_SRC_RESYNC           5

/** \brief End of file
 *
 *  Transmitted as the last packet of a track. After that, another
 *  track (or the same again) can be selected and the stream can newly be set up
 */

#define GAVL_MSG_SRC_EOF              6

/** \brief Authentication request
 *
 *  arg0: Username (string)
 *  arg1: Password (string)
 */

#define GAVL_CMD_SRC_AUTHENTICATE     101

/** \brief Multitrack support
 *
 *  arg0: Track (int)
 *
 *  Argument 1 decides, if the operation is expected in
 *  synchronous mode or not
 *
 *  Calling this is only allowed right after GAVL_MSG_SRC_EOF was received
 */

#define GAVL_CMD_SRC_SELECT_TRACK     102

/** \brief Seek support
 *
 *  arg0: Time  (long)
 *  arg1: Scale (int)
 *
 *  Argument 2 decides, if the seek operation is expected in
 *  synchronout mode or not
 */

#define GAVL_CMD_SRC_SEEK              103

/** \brief Start the source
 */

#define GAVL_CMD_SRC_START             105

/** \brief Pause the source
 *
 *  This signals, that we will pause the read functions for a while.
 */

#define GAVL_CMD_SRC_PAUSE             106

/** \brief Resume the source
 *
 *  Reverse the effect of an earlier pause command
 */

#define GAVL_CMD_SRC_RESUME            107

/* GUI Events */

/*
 *  GAVL_MSG_NS_GUI
 */

/** \brief Key was pressed
 *
 *  arg0: Key  (see keycodes.h)
 *  arg1: Mask (see keycodes.h)
 */

#define GAVL_MSG_GUI_KEY_PRESS       1  // Key was pressed

/** \brief Key was released
 *
 *  arg0: Key  (see keycodes.h)
 *  arg1: Mask (see keycodes.h)
 */

#define GAVL_MSG_GUI_KEY_RELEASE     2  // Key was released

/** \brief Button was pressed
 *
 *  arg0: Button  (1..5)
 *  arg1: Mask (see keycodes.h)
 *  arg2: x (int, in screen pixels)
 *  arg3: y (int, in screen pixels)
 *  arg4: pos (position, 0..1, relative to Video viewport)
 */

#define GAVL_MSG_GUI_BUTTON_PRESS    3  // Mouse button was pressed

/** \brief Button was released
 *
 *  arg0: Button  (1..5)
 *  arg1: Mask (see keycodes.h)
 *  arg2: x (int, in screen pixels)
 *  arg3: y (int, in screen pixels)
 *  arg4: pos (position, 0..1, relative to Video viewport)
 */

#define GAVL_MSG_GUI_BUTTON_RELEASE  4  // Mouse button was released

/** \brief Doubleclick
 *
 *  arg0: Button  (1..5)
 *  arg1: Mask (see keycodes.h)
 *  arg2: x (int, in screen pixels)
 *  arg3: y (int, in screen pixels)
 *  arg4: pos (position, 0..1, relative to Video viewport)
 */

#define GAVL_MSG_GUI_BUTTON_DOUBLECLICK  5  // Mouse button was released

/** \brief Motion callback
 *
 *  arg0: Mask (see keycodes.h)
 *  arg1: x (int, in video pixels)
 *  arg2: y (int, in video pixels)
 *  arg3: pos (position, 0..1, relative to Video viewport)
 */

#define GAVL_MSG_GUI_MOUSE_MOTION    6  // Mouse was moved

/** \brief Accelerator
 *
 *  arg0: id (int)
 */

#define GAVL_MSG_GUI_ACCEL           7  // Accelerator was triggered

/** \brief Window coordinates changed
 *
 *  arg0: x (int)
 *  arg1: y (int)
 *  arg2: w (int)
 *  arg3: h (int)
 */

#define GAVL_MSG_GUI_WINDOW_COORDS   8  // Window coordinates changed

/** \brief Window coordinates changed
 *
 *  arg0: direction (int)
 */

#define GAVL_MSG_GUI_SWIPE           9  // Swipe (touch screen)

#define GAVL_MSG_GUI_SWIPE_UP        1
#define GAVL_MSG_GUI_SWIPE_DOWN      2
#define GAVL_MSG_GUI_SWIPE_LEFT      3
#define GAVL_MSG_GUI_SWIPE_RIGHT     4

/* Commands */



/** \brief Set window title
 *
 *  arg0: title (string)
 */

#define GAVL_CMD_GUI_SET_WINDOW_TITLE 100

// GAVL_MSG_NS_GAVF
/* These messages control the low-level packet flow in order to allow
   format changes, resyncs etc. They are included into a gavf message stream */

/* Receiver -> Source: Acknowledge the arrival of a media (i.e. no message) packet */
#define GAVL_MSG_GAVF_PACKET_ACK   1

/* Source -> receiver: EOF encountered. After this message was sent, one can seek or select another track
   with the Source API */

/* Arg 0: New time */
/* Arg 1: scale    */
/* Arg 2: discard packets (1 or 0) */
/* Arg 3: stream is discontinuous (1 or 0) */

#define GAVL_MSG_GAVF_RESYNC       2

/* Emitted by the decoder immediately after a program header was read */

#define GAVL_MSG_GAVF_READ_PROGRAM_HEADER_START     (0x1000|1)
#define GAVL_MSG_GAVF_READ_PROGRAM_HEADER_END       (0x1000|2)

/* Emitted by the encoder immediately before a program header is written */
#define GAVL_MSG_GAVF_WRITE_HEADER_START            (0x1000|3)

/* Emitted by the encoder immediately after a program header is written */
#define GAVL_MSG_GAVF_WRITE_HEADER_END              (0x1000|4)

#define GAVL_MSG_GAVF_READ_SYNC_HEADER_START        (0x1000|5)
#define GAVL_MSG_GAVF_READ_SYNC_HEADER_END          (0x1000|6)

#define GAVL_MSG_GAVF_WRITE_SYNC_HEADER_START       (0x1000|7)
#define GAVL_MSG_GAVF_WRITE_SYNC_HEADER_END         (0x1000|8)

#define GAVL_MSG_GAVF_READ_PACKET_START             (0x1000|9)
#define GAVL_MSG_GAVF_READ_PACKET_END               (0x1000|10)

#define GAVL_MSG_GAVF_WRITE_PACKET_START            (0x1000|11)

/* Emitted by the encoder immediately after a packet is written */
#define GAVL_MSG_GAVF_WRITE_PACKET_END              (0x1000|12)

/* Emitted from within the demuxer loop */
#define GAVL_MSG_GAVF_GOT_EOF                       (0x1000|13)


/* Header fields */
#define GAVL_MSG_ID         "ID"
#define GAVL_MSG_NS         "NS"
#define GAVL_MSG_CLIENT_ID  "ClientID"
#define GAVL_MSG_CONTEXT_ID "ContextID"
#define GAVL_MSG_TIMESTAMP  "TS"      // gavl_time_t (GAVL_TYPE_LONG)
#define GAVL_MSG_NOT_LAST   "NotLast" // if nonzero means, that other messages follow
#define GAVL_MSG_FUNCTION_TAG "FunctionTag"


/** \brief Message type
 */

struct gavl_msg_s
  {
  gavl_dictionary_t header;
  int NS;  // Namespace
  int ID;
  
  // Where to send the answer. Meaning defined at a higher level.
  
  int num_args;
  gavl_value_t args[GAVL_MSG_MAX_ARGS];
  };

typedef struct gavl_msg_s gavl_msg_t;

typedef int (*gavl_handle_msg_func)(void *, gavl_msg_t *);

GAVL_PUBLIC
int gavl_msg_send(gavl_msg_t * msg, gavl_handle_msg_func func, void * priv);

/** \brief Create a message
 *  \returns A newly allocated message
 */

GAVL_PUBLIC
gavl_msg_t * gavl_msg_create();

/** \brief Initialize a message struct
 *  \param m A message
 */

GAVL_PUBLIC
void gavl_msg_init(gavl_msg_t * m);

/** \brief Destroy a message
 *  \param msg A message
 */

GAVL_PUBLIC
void gavl_msg_destroy(gavl_msg_t * msg);

/** \brief Free internal memory of the message
 *  \param msg A message
 *
 *  Use this, if you want to reuse the message with
 *  a different ID or args
 */

GAVL_PUBLIC
void gavl_msg_free(gavl_msg_t * msg);

/** \brief Copy message
 *  \param dst Destination
 *  \param src Source
 */

GAVL_PUBLIC
void gavl_msg_copy(gavl_msg_t * dst, const gavl_msg_t * src);


GAVL_PUBLIC
int gavl_msg_match(const gavl_msg_t * m, uint32_t id, uint32_t ns);

/* Functions for messages */

/** \brief Set the ID of a message
 *  \param msg A message
 *  \param id The ID
 */

GAVL_PUBLIC
void gavl_msg_set_id(gavl_msg_t * msg, int id);

/** \brief Set the ID of a message with namespace
 *  \param msg A message
 *  \param id The ID
 *  \param ns The Namespace
 */

GAVL_PUBLIC
void gavl_msg_set_id_ns(gavl_msg_t * msg, int id, int ns);

/** \brief Get the ID of a message
 *  \param msg A message
 *  \returns The ID
 */

GAVL_PUBLIC
int gavl_msg_get_id(gavl_msg_t * msg);

/** \brief Get the ID and namespace of a message
 *  \param msg A message
 *  \param ns If non-NULL, returns the namespace
 *  \returns The ID
 */

GAVL_PUBLIC
int gavl_msg_get_id_ns(gavl_msg_t * msg, int * ns);

GAVL_PUBLIC
int gavl_msg_set_arg(gavl_msg_t * msg, int idx, const gavl_value_t * val);

GAVL_PUBLIC
int gavl_msg_set_arg_nocopy(gavl_msg_t * msg, int idx, gavl_value_t * val);

GAVL_PUBLIC
const gavl_value_t * gavl_msg_get_arg_c(const gavl_msg_t * msg, int idx);

GAVL_PUBLIC
gavl_value_t * gavl_msg_get_arg_nc(gavl_msg_t * msg, int arg);

GAVL_PUBLIC
void gavl_msg_get_arg(gavl_msg_t * msg, int idx, gavl_value_t * val);

/** \brief Set an integer argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_int(gavl_msg_t * msg, int arg, int value);

/** \brief Get an integer argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \returns Value
 */

GAVL_PUBLIC
int gavl_msg_get_arg_int(const gavl_msg_t * msg, int arg);

/** \brief Set a time argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_long(gavl_msg_t * msg, int arg, gavl_time_t value);

/** \brief Get a time argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \returns Value
 */

GAVL_PUBLIC
int64_t gavl_msg_get_arg_long(const gavl_msg_t * msg, int arg);

/** \brief Set a string argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_string(gavl_msg_t * msg, int arg, const char * value);

/** \brief Get a string argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \returns The string
 *
 *  You can get the string value only once from each arg
 *  and must free() it, when you are done with it
 */

// GAVL_PUBLIC
// char * gavl_msg_get_arg_string(gavl_msg_t * msg, int arg);

/** \brief Get a string argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \returns The string
 *
 *  Like \ref gavl_msg_set_arg_string except that the returned string is 
 *  owned by the message and must not be freed.
 */

GAVL_PUBLIC
const char * gavl_msg_get_arg_string_c(const gavl_msg_t * msg, int arg);

/** \brief Set a float argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_float(gavl_msg_t * msg, int arg, double value);

/** \brief Get a float argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \returns Value
 */

GAVL_PUBLIC
double gavl_msg_get_arg_float(const gavl_msg_t * msg, int arg);

/** \brief Set an RGB color argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_color_rgb(gavl_msg_t * msg, int arg, const float * value);

/** \brief Get an RGB color argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_get_arg_color_rgb(gavl_msg_t * msg, int arg, float * value);


/** \brief Set an RGBA color argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_color_rgba(gavl_msg_t * msg, int arg, const float * value);

/** \brief Get an RGBA color argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_get_arg_color_rgba(gavl_msg_t * msg, int arg, float * value);

/** \brief Set a position argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_position(gavl_msg_t * msg, int arg, const double * value);

/** \brief Get a position argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_get_arg_position(gavl_msg_t * msg, int arg, double * value);

/** \brief Set an audio format argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param format An audio format
 */

GAVL_PUBLIC
void gavl_msg_set_arg_audio_format(gavl_msg_t * msg, int arg,
                                 const gavl_audio_format_t * format);

/** \brief Get an audio format argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param format Returns the audio format
 */

GAVL_PUBLIC
void gavl_msg_get_arg_audio_format(gavl_msg_t * msg, int arg,
                                 gavl_audio_format_t * format);


/** \brief Set a video format argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param format A video format
 */

GAVL_PUBLIC
void gavl_msg_set_arg_video_format(gavl_msg_t * msg, int arg,
                                 const gavl_video_format_t * format);

/** \brief Get a video format argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param format Returns the video format
 */

GAVL_PUBLIC
void gavl_msg_get_arg_video_format(gavl_msg_t * msg, int arg,
                                 gavl_video_format_t * format);


/** \brief Set a matadata argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param m Metadata
 */

GAVL_PUBLIC
void gavl_msg_set_arg_dictionary(gavl_msg_t * msg, int arg,
                                 const gavl_dictionary_t * m);

GAVL_PUBLIC
void gavl_msg_set_arg_dictionary_nocopy(gavl_msg_t * msg, int arg,
                                        gavl_dictionary_t * m);


/** \brief Get a matadata argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param m Returns dictionary
 *
 *  Don't pass uninitalized memory as dictionary
 */

GAVL_PUBLIC
int gavl_msg_get_arg_dictionary(gavl_msg_t * msg, int arg,
                                gavl_dictionary_t * m);

GAVL_PUBLIC
int gavl_msg_get_arg_dictionary_c(const gavl_msg_t * msg, int arg,
                                  gavl_dictionary_t * m);


GAVL_PUBLIC
void gavl_msg_set_arg_array(gavl_msg_t * msg, int arg,
                            const gavl_array_t * m);

GAVL_PUBLIC
void gavl_msg_set_arg_array_nocopy(gavl_msg_t * msg, int arg,
                                   gavl_array_t * m);

GAVL_PUBLIC
int gavl_msg_get_arg_array(gavl_msg_t * msg, int arg,
                           gavl_array_t * arr);

GAVL_PUBLIC
int gavl_msg_get_arg_array_c(const gavl_msg_t * msg, int arg,
                             gavl_array_t * arr);

GAVL_PUBLIC
void gavl_msg_dump(const gavl_msg_t * msg, int indent);

GAVL_PUBLIC
void gavl_msg_apply_header(gavl_msg_t * msg);

GAVL_PUBLIC
void gavl_msg_set_client_id(gavl_msg_t * msg, const char * id);

GAVL_PUBLIC
const char * gavl_msg_get_client_id(const gavl_msg_t * msg);

GAVL_PUBLIC
gavl_time_t gavl_msg_get_timestamp(const gavl_msg_t * msg);

GAVL_PUBLIC
void gavl_msg_set_timestamp(gavl_msg_t * msg, gavl_time_t);

GAVL_PUBLIC 
void gavl_msg_copy_header_field(gavl_msg_t * dst, const gavl_msg_t * src, const char * key);


/*
 *  Utilities
 */

GAVL_PUBLIC void
gavl_msg_set_progress(gavl_msg_t * msg, const char * activity, float perc);

GAVL_PUBLIC void
gavl_msg_get_progress(gavl_msg_t * msg, const char ** activity, float * perc);


GAVL_PUBLIC void
gavl_msg_set_src_metadata(gavl_msg_t * msg, gavl_time_t time, const gavl_dictionary_t * m);

GAVL_PUBLIC void
gavl_msg_get_src_metadata(gavl_msg_t * msg, gavl_time_t * time, gavl_dictionary_t * m);

GAVL_PUBLIC void
gavl_msg_set_src_aspect(gavl_msg_t * msg, int64_t time, int scale, int stream,
                        int pixel_width, int pixel_height);

GAVL_PUBLIC void
gavl_msg_get_src_aspect(gavl_msg_t * msg,
                        int64_t * time,
                        int * scale, int * stream,
                        int * pixel_width, int * pixel_height);
GAVL_PUBLIC void
gavl_msg_set_src_buffering(gavl_msg_t * msg, float perc);

GAVL_PUBLIC void
gavl_msg_get_src_buffering(gavl_msg_t * msg, float * perc);

GAVL_PUBLIC void
gavl_msg_set_gui_button_press(gavl_msg_t * msg, int button,
                              int mask, int x, int y, const double * pos);

GAVL_PUBLIC void
gavl_msg_set_gui_button_release(gavl_msg_t * msg, int button,
                                int mask, int x, int y, const double * pos);

GAVL_PUBLIC void
gavl_msg_get_gui_button(gavl_msg_t * msg, int * button,
                        int * mask, int * x, int * y, double * pos);

GAVL_PUBLIC void
gavl_msg_set_gui_key_press(gavl_msg_t * msg, int key,
                           int mask, int x, int y, const double * pos);

GAVL_PUBLIC void
gavl_msg_set_gui_key_release(gavl_msg_t * msg, int key,
                             int mask, int x, int y, const double * pos);

GAVL_PUBLIC void
gavl_msg_get_gui_key(gavl_msg_t * msg, int * key,
                     int * mask, int * x, int * y, double * pos);

GAVL_PUBLIC void
gavl_msg_set_gui_motion(gavl_msg_t * msg, 
                        int mask, int x, int y, const double * pos);

GAVL_PUBLIC void
gavl_msg_get_gui_motion(gavl_msg_t * msg,
                        int * mask, int * x, int * y, double * pos);
                        

GAVL_PUBLIC void
gavl_msg_set_splice_children(gavl_msg_t * msg, int msg_ns, int msg_id,
                             const char * ctx,
                             int last, int idx, int del, const gavl_value_t * add);

GAVL_PUBLIC void
gavl_msg_set_splice_children_nocopy(gavl_msg_t * msg, int msg_ns, int msg_id,
                                    const char * ctx,
                                    int last, int idx, int del, gavl_value_t * add);

GAVL_PUBLIC 
int gavl_msg_get_splice_children(gavl_msg_t * msg,
                                 int * last, int * idx, int * del, gavl_value_t * add);

GAVL_PUBLIC 
int gavl_msg_splice_children(gavl_msg_t * msg, gavl_dictionary_t * dict);

GAVL_PUBLIC 
int gavl_msg_get_last(const gavl_msg_t * msg);

GAVL_PUBLIC 
void gavl_msg_set_last(gavl_msg_t * msg, int last);

GAVL_PUBLIC 
void gavl_msg_set_resp_for_req(gavl_msg_t * dst, const gavl_msg_t * src);

GAVL_PUBLIC
void gavl_msg_set_src_resync(gavl_msg_t * dst, int64_t t, int scale, int discard, int discont);

GAVL_PUBLIC
void gavl_msg_get_src_resync(const gavl_msg_t * src, int64_t * t, int * scale, int * discard, int * discont);



#endif // GAVL_MSG_H_INCLUDED
