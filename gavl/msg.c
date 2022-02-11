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

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include <gavl/gavl.h>
#include <gavl/metadata.h>
#include <gavl/utils.h>
#include <gavl/msg.h>
#include <gavl/gavf.h>
#include <gavl/trackinfo.h>


void gavl_msg_set_client_id(gavl_msg_t * msg, const char * id)
  {
  if(gavl_dictionary_get(&msg->header, GAVL_MSG_CLIENT_ID))
    return;
  gavl_dictionary_set_string(&msg->header, GAVL_MSG_CLIENT_ID, id);
  }

const char * gavl_msg_get_client_id(const gavl_msg_t * msg)
  {
  return gavl_dictionary_get_string(&msg->header, GAVL_MSG_CLIENT_ID);
  }

void gavl_msg_set_id_ns(gavl_msg_t * msg, int id, int ns)
  {
  gavl_dictionary_init(&msg->header);
  gavl_dictionary_set_int(&msg->header, GAVL_MSG_ID, id);
  msg->ID = id;

  gavl_dictionary_set_int(&msg->header, GAVL_MSG_NS, ns);
  msg->NS = ns;
    
  msg->num_args = 0;

  /* Zero everything */
  memset(&msg->args, 0, sizeof(msg->args));
  }

void gavl_msg_set_id(gavl_msg_t * msg, int id)
  {
  gavl_msg_set_id_ns(msg, id, 0);
  }

int gavl_msg_get_id_ns(gavl_msg_t * msg, int * ns)
  {
  *ns = msg->NS;
  return msg->ID;
  }

void gavl_msg_copy(gavl_msg_t * dst, const gavl_msg_t * src)
  {
  int i;

  /* Also clears all memory */
  gavl_msg_set_id_ns(dst, src->ID, src->NS);
  
  dst->num_args = src->num_args;
  gavl_dictionary_copy(&dst->header, &src->header);
  
  for(i = 0; i < src->num_args; i++)
    gavl_value_copy(&dst->args[i], &src->args[i]); 
  }

static int check_arg(int arg)
  {
  if(arg < 0)
    return 0;
  assert(arg < GAVL_MSG_MAX_ARGS);
  return 1;
  }

/* Set argument to a basic type */

void gavl_msg_set_arg_int(gavl_msg_t * msg, int arg, int value)
  {
  gavl_value_t * val;
  if(!check_arg(arg))
    return;
  val = &msg->args[arg];
  gavl_value_set_int(val, value);
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_long(gavl_msg_t * msg, int arg, int64_t value)
  {
  if(!check_arg(arg))
    return;
  gavl_value_set_long(&msg->args[arg], value);
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_string(gavl_msg_t * msg, int arg, const char * value)
  {
  if(!check_arg(arg))
    return;
  gavl_value_set_string(&msg->args[arg], value);
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_float(gavl_msg_t * msg, int arg, double value)
  {
  if(!check_arg(arg))
    return;
  gavl_value_set_float(&msg->args[arg], value);
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_color_rgb(gavl_msg_t * msg, int arg, const float * value)
  {
  double * col;
  if(!check_arg(arg))
    return;
  col = gavl_value_set_color_rgb(&msg->args[arg]);
  col[0] = value[0];
  col[1] = value[1];
  col[2] = value[2];
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_color_rgba(gavl_msg_t * msg, int arg, const float * value)
  {
  double * col;
  if(!check_arg(arg))
    return;
  col = gavl_value_set_color_rgba(&msg->args[arg]);
  col[0] = value[0];
  col[1] = value[1];
  col[2] = value[2];
  col[3] = value[3];

  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_position(gavl_msg_t * msg, int arg, const double * value)
  {
  double * pos;
  if(!check_arg(arg))
    return;
  pos = gavl_value_set_position(&msg->args[arg]);
  pos[0] = value[0];
  pos[1] = value[1];

  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

/* Get/Set value */

int gavl_msg_set_arg(gavl_msg_t * msg, int arg, const gavl_value_t * val)
  {
  if(!check_arg(arg))
    return 0;

  gavl_value_copy(&msg->args[arg], val);
  
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  
  return 1;
  }

int gavl_msg_set_arg_nocopy(gavl_msg_t * msg, int arg, gavl_value_t * val)
  {
  if(!check_arg(arg))
    return 0;

  gavl_value_move(&msg->args[arg], val);
  
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;

  return 1;
  }

const gavl_value_t * gavl_msg_get_arg_c(const gavl_msg_t * msg, int arg)
  {
  if(!check_arg(arg))
    return NULL;
  return &msg->args[arg];
  }

gavl_value_t * gavl_msg_get_arg_nc(gavl_msg_t * msg, int arg)
  {
  if(!check_arg(arg))
    return NULL;
  return &msg->args[arg];
  }

void gavl_msg_get_arg(gavl_msg_t * msg, int arg, gavl_value_t * val)
  {
  if(!check_arg(arg))
    return;
  gavl_value_move(val, &msg->args[arg]);
  }
  
/* Get basic types */

int gavl_msg_get_id(gavl_msg_t * msg)
  {
  return msg->ID;
  }

int gavl_msg_get_arg_int(const gavl_msg_t * msg, int arg)
  {
  if(!check_arg(arg))
    return 0;
  return msg->args[arg].v.i;
  }

int64_t gavl_msg_get_arg_long(const gavl_msg_t * msg, int arg)
  {
  if(!check_arg(arg))
    return 0;
  return msg->args[arg].v.l;
  }

double gavl_msg_get_arg_float(const gavl_msg_t * msg, int arg)
  {
  if(!check_arg(arg))
    return 0.0;
  return msg->args[arg].v.d;
  }

void gavl_msg_get_arg_color_rgb(gavl_msg_t * msg, int arg, float * val)
  {
  if(!check_arg(arg))
    return;
  val[0] = msg->args[arg].v.color[0];
  val[1] = msg->args[arg].v.color[1];
  val[2] = msg->args[arg].v.color[2];
  }

void gavl_msg_get_arg_color_rgba(gavl_msg_t * msg, int arg, float * val)
  {
  if(!check_arg(arg))
    return;
  val[0] = msg->args[arg].v.color[0];
  val[1] = msg->args[arg].v.color[1];
  val[2] = msg->args[arg].v.color[2];
  val[3] = msg->args[arg].v.color[3];
  }

void gavl_msg_get_arg_position(gavl_msg_t * msg, int arg, double * val)
  {
  if(!check_arg(arg))
    return;
  val[0] = msg->args[arg].v.position[0];
  val[1] = msg->args[arg].v.position[1];
  }


#if 0
char * gavl_msg_get_arg_string(gavl_msg_t * msg, int arg)
  {
  char * ret;
  if(!check_arg(arg))
    return NULL;
  ret = msg->args[arg].v.str;
  msg->args[arg].v.str = NULL;
  return ret;
  }
#endif

const char * gavl_msg_get_arg_string_c(const gavl_msg_t * msg, int arg)
  {
  if(!check_arg(arg))
    return NULL;
  return (char *)msg->args[arg].v.str;
  }

/* Get/Set routines for structures */

void gavl_msg_set_arg_audio_format(gavl_msg_t * msg, int arg,
                                   const gavl_audio_format_t * format)
  {
  gavl_audio_format_copy(gavl_value_set_audio_format(&msg->args[arg]), format);
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_get_arg_audio_format(gavl_msg_t * msg, int arg,
                                 gavl_audio_format_t * format)
  {
  gavl_audio_format_copy(format, msg->args[arg].v.audioformat);
  }

/* Video format */

void gavl_msg_set_arg_video_format(gavl_msg_t * msg, int arg,
                                 const gavl_video_format_t * format)
  {
  gavl_video_format_copy(gavl_value_set_video_format(&msg->args[arg]), format);
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_get_arg_video_format(gavl_msg_t * msg, int arg,
                                 gavl_video_format_t * format)
  {
  gavl_video_format_copy(format, msg->args[arg].v.videoformat);
  }

void gavl_msg_set_arg_dictionary(gavl_msg_t * msg, int arg,
                               const gavl_dictionary_t * m)
  {
  gavl_dictionary_copy(gavl_value_set_dictionary(&msg->args[arg]), m);
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_dictionary_nocopy(gavl_msg_t * msg, int arg,
                                        gavl_dictionary_t * m)
  {
  gavl_value_set_dictionary_nocopy(&msg->args[arg], m);
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_array(gavl_msg_t * msg, int arg,
                            const gavl_array_t * m)
  {
  gavl_array_copy(gavl_value_set_array(&msg->args[arg]), m);
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_array_nocopy(gavl_msg_t * msg, int arg,
                                   gavl_array_t * m)
  {
  gavl_value_set_array_nocopy(&msg->args[arg], m);
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

int gavl_msg_get_arg_dictionary_c(const gavl_msg_t * msg, int arg,
                                  gavl_dictionary_t * m)
  {
  gavl_dictionary_reset(m);

  if(msg->args[arg].type != GAVL_TYPE_DICTIONARY)
    return 0;
  
  gavl_dictionary_copy(m, msg->args[arg].v.dictionary);
  return 1;
  }

int gavl_msg_get_arg_dictionary(gavl_msg_t * msg, int arg,
                                gavl_dictionary_t * m)
  {
  gavl_dictionary_reset(m);

  if(msg->args[arg].type != GAVL_TYPE_DICTIONARY)
    return 0;
  
  memcpy(m, msg->args[arg].v.dictionary, sizeof(*m));
  gavl_dictionary_init(msg->args[arg].v.dictionary);
  return 1;
  }

int gavl_msg_get_arg_array(gavl_msg_t * msg, int arg,
                           gavl_array_t * arr)
  {
  gavl_array_reset(arr);
  
  if(msg->args[arg].type != GAVL_TYPE_ARRAY)
    return 0;
  
  gavl_array_move(arr, msg->args[arg].v.array);
  return 1;
  }

int gavl_msg_get_arg_array_c(const gavl_msg_t * msg, int arg,
                             gavl_array_t * arr)
  {
  gavl_array_reset(arr);

  if(msg->args[arg].type != GAVL_TYPE_ARRAY)
    return 0;
  gavl_array_copy(arr, msg->args[arg].v.array);
  return 1;
  }


void gavl_msg_init(gavl_msg_t * m)
  {
  memset(m, 0, sizeof(*m));
  m->ID = -1;
  }

gavl_msg_t * gavl_msg_create()
  {
  gavl_msg_t * ret;
  ret = malloc(sizeof(*ret));
  gavl_msg_init(ret);
  return ret;
  }


void gavl_msg_free(gavl_msg_t * m)
  {
  int i;
  for(i = 0; i < m->num_args; i++)
    gavl_value_free(&m->args[i]);
  
  gavl_dictionary_reset(&m->header);
  memset(m, 0, sizeof(*m));
  m->ID = -1;
  }

void gavl_msg_destroy(gavl_msg_t * m)
  {
  gavl_msg_free(m);
  free(m);
  }

void gavl_msg_dump(const gavl_msg_t * msg, int indent)
  {
  int i;

  gavl_diprintf(indent, "Message\n");
  gavl_dictionary_dump(&msg->header, indent + 2);
  gavl_dprintf("\n");
    
  for(i = 0; i < msg->num_args; i++)
    {
    gavl_diprintf(indent + 2, "arg[%d]: %s: ", i, gavl_type_to_string(msg->args[i].type));
    gavl_value_dump(&msg->args[i], 0);
    gavl_dprintf("\n");
    }
  
  }

/* Set specific messages */

/** \brief Generic progress callback
 *
 *  arg0: Activity (string) 
 *  arg1: Percentage (0.0..1.0)
 */

void
gavl_msg_set_progress(gavl_msg_t * msg, const char * activity, float perc)
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_PROGRESS, GAVL_MSG_NS_GENERIC);
  gavl_msg_set_arg_string(msg, 0, activity);
  gavl_msg_set_arg_float(msg, 1, perc);
  }

void
gavl_msg_get_progress(gavl_msg_t * msg, const char ** activity, float * perc)
  {
  if(activity)
    *activity = gavl_msg_get_arg_string_c(msg, 0);
  if(perc)
    *perc = gavl_msg_get_arg_float(msg, 1);
  }

void
gavl_msg_set_src_metadata(gavl_msg_t * msg, int64_t time, const gavl_dictionary_t * m)
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_SRC_METADATA_CHANGED, GAVL_MSG_NS_SRC);
  gavl_msg_set_arg_long(msg, 0, time);
  gavl_msg_set_arg_dictionary(msg, 1, m);
  }

void
gavl_msg_get_src_metadata(gavl_msg_t * msg, int64_t * time, gavl_dictionary_t * m)
  {
  if(time)
    *time = gavl_msg_get_arg_long(msg, 0);
  if(m)
    gavl_msg_get_arg_dictionary(msg, 1, m);
  }


void
gavl_msg_set_src_aspect(gavl_msg_t * msg, int64_t time, int scale, int stream,
                        int pixel_width, int pixel_height)
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_SRC_ASPECT_CHANGED, GAVL_MSG_NS_SRC);
  gavl_msg_set_arg_long(msg, 0, time);
  gavl_msg_set_arg_int(msg, 1, scale);
  gavl_msg_set_arg_int(msg, 2, stream);
  gavl_msg_set_arg_int(msg, 3, pixel_width);
  gavl_msg_set_arg_int(msg, 4, pixel_height);
  }

void
gavl_msg_get_src_aspect(gavl_msg_t * msg,
                        int64_t * time,
                        int * scale, int * stream,
                        int * pixel_width, int * pixel_height)
  {
  if(time)
    *time = gavl_msg_get_arg_long(msg, 0);
  if(scale)
    *scale = gavl_msg_get_arg_int(msg, 1);
  if(stream)
    *stream = gavl_msg_get_arg_int(msg, 2);
  if(pixel_width)
    *pixel_width = gavl_msg_get_arg_int(msg, 3);
  if(pixel_height)
    *pixel_height = gavl_msg_get_arg_int(msg, 4);
  }

void
gavl_msg_set_src_buffering(gavl_msg_t * msg, float perc)
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_SRC_BUFFERING, GAVL_MSG_NS_SRC);
  gavl_msg_set_arg_float(msg, 0, perc);
  }

void
gavl_msg_get_src_buffering(gavl_msg_t * msg, float * perc)
  {
  if(perc)
    *perc = gavl_msg_get_arg_float(msg, 0);
  }

static void set_button_args(gavl_msg_t * msg, int button,
                            int mask, int x, int y, const double * pos)
  {
  gavl_msg_set_arg_int(msg, 0, button);
  gavl_msg_set_arg_int(msg, 1, mask);
  gavl_msg_set_arg_int(msg, 2, x);
  gavl_msg_set_arg_int(msg, 3, y);
  gavl_msg_set_arg_position(msg, 4, pos);
  }

void
gavl_msg_set_gui_button_press(gavl_msg_t * msg, int button,
                              int mask, int x, int y, const double * pos)
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_GUI_BUTTON_PRESS, GAVL_MSG_NS_GUI);
  set_button_args(msg, button, mask, x, y, pos);
  }

void
gavl_msg_set_gui_button_release(gavl_msg_t * msg, int button,
                                int mask, int x, int y, const double * pos)
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_GUI_BUTTON_RELEASE, GAVL_MSG_NS_GUI);
  set_button_args(msg, button, mask, x, y, pos);
  }

void
gavl_msg_get_gui_button(gavl_msg_t * msg, int * button,
                        int * mask, int * x, int * y, double * pos)
 
  {
  if(button)
    *button = gavl_msg_get_arg_int(msg, 0);
  if(mask)
    *mask = gavl_msg_get_arg_int(msg, 1);
  if(x)
    *x = gavl_msg_get_arg_int(msg, 2);
  if(y)
    *y = gavl_msg_get_arg_int(msg, 3);
  if(pos)
    gavl_msg_get_arg_position(msg, 4, pos);
  }

static void set_key_args(gavl_msg_t * msg, int key,
                         int mask, int x, int y, const double * pos)
  {
  gavl_msg_set_arg_int(msg, 0, key);
  gavl_msg_set_arg_int(msg, 1, mask);
  gavl_msg_set_arg_int(msg, 2, x);
  gavl_msg_set_arg_int(msg, 3, y);
  gavl_msg_set_arg_position(msg, 4, pos);
  }

void
gavl_msg_set_gui_key_press(gavl_msg_t * msg, int key,
                           int mask, int x, int y, const double * pos)
  
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_GUI_KEY_PRESS, GAVL_MSG_NS_GUI);
  set_key_args(msg, key, mask, x, y, pos);
  }

void
gavl_msg_set_gui_key_release(gavl_msg_t * msg, int key,
                             int mask, int x, int y, const double * pos)
  
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_GUI_KEY_RELEASE, GAVL_MSG_NS_GUI);
  set_key_args(msg, key, mask, x, y, pos);
  }

void
gavl_msg_get_gui_key(gavl_msg_t * msg, int * key,
                     int * mask, int * x, int * y, double * pos)
  
  {
  if(key)
    *key = gavl_msg_get_arg_int(msg, 0);
  if(mask)
    *mask = gavl_msg_get_arg_int(msg, 1);
  if(x)
    *x = gavl_msg_get_arg_int(msg, 2);
  if(y)
    *y = gavl_msg_get_arg_int(msg, 3);
  if(pos)
    gavl_msg_get_arg_position(msg, 4, pos);
  }


void
gavl_msg_set_gui_motion(gavl_msg_t * msg,  
                        int mask, int x, int y, const double * pos)
  
  {
  gavl_msg_set_arg_int(msg, 0, mask);
  gavl_msg_set_arg_int(msg, 1, x);
  gavl_msg_set_arg_int(msg, 2, y);
  gavl_msg_set_arg_position(msg, 3, pos);
  }

void
gavl_msg_get_gui_motion(gavl_msg_t * msg, 
                        int * mask, int * x, int * y, double * pos)
  
  {
  if(mask)
    *mask = gavl_msg_get_arg_int(msg, 0);
  if(x)
    *x = gavl_msg_get_arg_int(msg, 1);
  if(y)
    *y = gavl_msg_get_arg_int(msg, 2);
  if(pos)
    gavl_msg_get_arg_position(msg, 3, pos);

  }

int gavl_msg_match(const gavl_msg_t * m, uint32_t id, uint32_t ns)
  {
  return ((m->ID == id) && (m->NS == ns)) ? 1 : 0;
  }

void gavl_msg_apply_header(gavl_msg_t * msg)
  {
  const gavl_value_t * val;
  
  if((val = gavl_dictionary_get(&msg->header, GAVL_MSG_ID)) &&
     (val->type == GAVL_TYPE_INT))
    msg->ID = val->v.i;

  if((val = gavl_dictionary_get(&msg->header, GAVL_MSG_NS)) &&
     (val->type == GAVL_TYPE_INT))
    msg->NS = val->v.i;
  }

void gavl_msg_set_splice_children(gavl_msg_t * msg, int msg_ns, int msg_id,
                                  const char * ctx,
                                  int last, int idx, int del, const gavl_value_t * add)
  {
  gavl_msg_set_id_ns(msg, msg_id, msg_ns);

  if(ctx)
    gavl_dictionary_set_string(&msg->header, GAVL_MSG_CONTEXT_ID, ctx);
  
  gavl_msg_set_last(msg, last);
  gavl_msg_set_arg_int(msg, 0, idx);
  gavl_msg_set_arg_int(msg, 1, del);
  gavl_msg_set_arg(msg,     2, add);
  }

void gavl_msg_set_splice_children_nocopy(gavl_msg_t * msg, int msg_ns, int msg_id,
                                         const char * ctx, int last, int idx, int del, gavl_value_t * add)
  {
  gavl_msg_set_id_ns(msg, msg_id, msg_ns);

  if(ctx)
    gavl_dictionary_set_string(&msg->header, GAVL_MSG_CONTEXT_ID, ctx);

  gavl_msg_set_last(msg, last);
  
  gavl_msg_set_arg_int(msg,    0, idx);
  gavl_msg_set_arg_int(msg,    1, del);
  gavl_msg_set_arg_nocopy(msg, 2, add);
  }

int gavl_msg_get_splice_children(gavl_msg_t * msg,
                                 int * last, int * idx, int * del, gavl_value_t * add)
  {
  *last = gavl_msg_get_last(msg);
  *idx  = gavl_msg_get_arg_int(msg, 0);
  *del  = gavl_msg_get_arg_int(msg, 1);
  gavl_msg_get_arg(msg, 2, add);
  return 1;
  }

int gavl_msg_get_last(const gavl_msg_t * msg)
  {
  int notlast = 0;

  if(gavl_dictionary_get_int(&msg->header, GAVL_MSG_NOT_LAST, &notlast) &&
     notlast)
    return 0;
  return 1;
  }

void gavl_msg_set_last(gavl_msg_t * msg, int last)
  {
  if(!last)
    gavl_dictionary_set_int(&msg->header, GAVL_MSG_NOT_LAST, 1);
  }

int gavl_msg_splice_children(gavl_msg_t * msg, gavl_dictionary_t * dict)
  {
  int ret = 0;
  int last;
  int idx;
  int del;
  gavl_value_t val;
  gavl_value_init(&val);
  
  if(gavl_msg_get_splice_children(msg, &last, &idx, &del, &val))
    {
    gavl_track_splice_children(dict, idx, del, &val);
    ret = 1;
    }
  gavl_value_free(&val);
  return ret;
  }

gavl_time_t gavl_msg_get_timestamp(const gavl_msg_t * msg)
  {
  gavl_time_t ret = GAVL_TIME_UNDEFINED;
  gavl_dictionary_get_long(&msg->header, GAVL_MSG_TIMESTAMP, &ret);
  return ret;
  }


void gavl_msg_set_timestamp(gavl_msg_t * msg, gavl_time_t t)
  {
  gavl_dictionary_set_long(&msg->header, GAVL_MSG_TIMESTAMP, t);
  }

void gavl_msg_copy_header_field(gavl_msg_t * dst, const gavl_msg_t * src, const char * key) 
  {
  if(!strcmp(key, GAVL_MSG_CLIENT_ID))
    gavl_msg_set_client_id(dst, gavl_msg_get_client_id(src));
  else
    gavl_dictionary_set(&dst->header, key, gavl_dictionary_get(&src->header, key));
  }

void gavl_msg_set_resp_for_req(gavl_msg_t * dst, const gavl_msg_t * src)
  {
  gavl_msg_copy_header_field(dst, src, GAVL_MSG_CLIENT_ID);
  gavl_msg_copy_header_field(dst, src, GAVL_MSG_CONTEXT_ID);
  gavl_msg_copy_header_field(dst, src, GAVL_MSG_FUNCTION_TAG);
  
  }

int gavl_msg_send(gavl_msg_t * msg, gavl_handle_msg_func func, void * priv)
  {
  if(!func)
    return 1;

  return func(priv, msg);
  
  }

void gavl_msg_set_src_resync(gavl_msg_t * dst, int64_t t, int scale, int discard, int discont)
  {
  gavl_msg_set_id_ns(dst, GAVL_MSG_SRC_RESYNC, GAVL_MSG_NS_SRC);
            
  gavl_msg_set_arg_long(dst, 0, t);
  gavl_msg_set_arg_int(dst, 1, scale);
  gavl_msg_set_arg_int(dst, 2, discard);
  gavl_msg_set_arg_int(dst, 3, discont);
  }

void gavl_msg_get_src_resync(const gavl_msg_t * src, int64_t * t, int * scale, int * discard, int * discont)
  {
  if(t)
    *t = gavl_msg_get_arg_long(src, 0);
  if(scale)
    *scale = gavl_msg_get_arg_int(src, 1);
  if(discard)
    *discard = gavl_msg_get_arg_int(src, 2);
  if(discont)
    *discont = gavl_msg_get_arg_int(src, 3);
  }
