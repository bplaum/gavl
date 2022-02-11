
#include <string.h>

#include <gavl/gavl.h>
#include <gavl/value.h>
#include <gavl/msg.h>


static void set_dict(gavl_dictionary_t * dict)
  {
  gavl_audio_format_t afmt;
  gavl_video_format_t vfmt;
  
  gavl_dictionary_set_string(dict, "String", "value");
  gavl_dictionary_set_int(dict, "Integer", 123);
  gavl_dictionary_set_float(dict, "Float", 123.0);

  memset(&afmt, 0, sizeof(afmt));
  memset(&vfmt, 0, sizeof(vfmt));
  
  gavl_dictionary_set_audio_format(dict, "afmt", &afmt);
  gavl_dictionary_set_video_format(dict, "vfmt", &vfmt);
  }
#if 0
static void set_msg(gavl_msg_t * msg)
  {
  gavl_msg_init(msg);
  //  gavl_msg_set_
  }
#endif

int main(int argc, char ** argv)
  {
  gavl_dictionary_t dict;
  gavl_dictionary_t * sub_dict;
  
  gavl_dictionary_init(&dict);
  set_dict(&dict);

  sub_dict = gavl_dictionary_get_dictionary_create(&dict, "sub");
  set_dict(sub_dict);

  gavl_dictionary_free(&dict);
  
  }
