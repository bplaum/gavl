
#include <gavl/gavl.h>
#include <gavl/compression.h>
#include <gavl/value.h>

#include <gavl/hw_v4l2.h>


int main()
  {
  gavl_array_t arr;
  gavl_array_init(&arr);

  gavl_v4l2_devices_scan(&arr);

  gavl_array_dump(&arr, 0);
  gavl_array_free(&arr);
  
  }

  
