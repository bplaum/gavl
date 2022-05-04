
#include <gavl/gavl.h>
#include <gavl/value.h>
#include <gavl/state.h>

void gavl_msg_set_state(gavl_msg_t * msg,
                      int id,
                      int last,
                      const char * ctx,
                      const char * var,
                      const gavl_value_t * val)
  {
  gavl_msg_set_id_ns(msg, id, GAVL_MSG_NS_STATE);
  gavl_msg_set_arg_int(msg, 0, last);
  
  gavl_msg_set_arg_string(msg, 1, ctx);
  gavl_msg_set_arg_string(msg, 2, var);
  gavl_msg_set_arg(msg, 3, val);
 
  }

void gavl_msg_set_state_nocopy(gavl_msg_t * msg,
                             int id,
                             int last,
                             const char * ctx,
                             const char * var,
                             gavl_value_t * val)
  {
  gavl_msg_set_id_ns(msg, id, GAVL_MSG_NS_STATE);
  gavl_msg_set_arg_int(msg, 0, last);
  gavl_msg_set_arg_string(msg, 1, ctx);
  gavl_msg_set_arg_string(msg, 2, var);
  gavl_msg_set_arg_nocopy(msg, 3, val);
  }


void gavl_msg_get_state(const gavl_msg_t * msg,
                      int * last_p,
                      const char ** ctx_p,
                      const char ** var_p,
                      gavl_value_t * val_p,
                      gavl_dictionary_t * dict)
  {
  const char * ctx;
  const char * var;
  const gavl_value_t * val;

  int last;

  last = gavl_msg_get_arg_int(msg, 0);
  
  ctx = gavl_msg_get_arg_string_c(msg, 1);
  var = gavl_msg_get_arg_string_c(msg, 2);
  
  if(last_p)
    *last_p = last;
  
  if(ctx_p)
    *ctx_p = ctx;
  if(var_p)
    *var_p = var;

  val = gavl_msg_get_arg_c(msg, 3);
  if(val_p)
    gavl_value_copy(val_p, val);
  
  if(dict)
    {
    gavl_dictionary_t * child;
    child = gavl_dictionary_get_dictionary_create(dict, ctx);
    gavl_dictionary_set(child, var, val);
    }

  }
