
#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
  #if (removecheck_SDV_RESULT == SDV_FAILED||removecheck2_SDV_RESULT == SDV_FAILED)
     #define SDV_HARNESS SDV_FLAT_DISPATCH_HARNESS_WITH_DEVICE_CONTROL_LINKED_CALLBACKS
  #else
     #pragma message("SDV_NA")  
   #endif
#else
#pragma message("removecheck==SDV_FAILED")
#pragma message("removecheck2==SDV_FAILED")
#endif
