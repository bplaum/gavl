#include <gavl/gavl.h>
#include <gavl/msg.h>

typedef enum
  {
    GAVL_LOG_ERROR   = (1<<0),
    GAVL_LOG_WARNING = (1<<1),
    GAVL_LOG_INFO    = (1<<2),
    GAVL_LOG_DEBUG   = (1<<3),
    
  } gavl_log_level_t;

/* Logging is done via a single callback function, which does nice stderr console
   logging by default and can be set to any other function once during initialization
   before any other threads are launched */

GAVL_PUBLIC
void gavl_log_translate(const char * translation_domain,
                        gavl_log_level_t level, const char * domain,
                        const char * format, ...) __attribute__ ((format (printf, 4, 5)));

#define gavl_log(level, domain, ...) \
   gavl_log_translate(PACKAGE, level, domain, __VA_ARGS__)


GAVL_PUBLIC
int gavl_log_stderr(void * data, gavl_msg_t * msg);

GAVL_PUBLIC
int gavl_log_msg_get(const gavl_msg_t * msg,
                     gavl_log_level_t * level,
                     const char ** domain,
                     const char ** message);

GAVL_PUBLIC
void gavl_log_msg_set(gavl_msg_t * msg,
                      gavl_log_level_t level,
                      const char * domain,
                      const char * message);

GAVL_PUBLIC
const char * gavl_log_level_to_string(gavl_log_level_t level);

GAVL_PUBLIC
void gavl_set_log_callback(gavl_handle_msg_func handler, void * priv);

GAVL_PUBLIC
void gavl_set_log_verbose(int level);

GAVL_PUBLIC
void gavl_set_log_mask(int mask);

GAVL_PUBLIC
void gavl_set_log_verbose(int level);
