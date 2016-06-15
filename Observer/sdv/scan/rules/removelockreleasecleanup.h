#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
#if (checkadddevice_SDV_RESULT == SDV_FAILED && checkirpmjpnp_SDV_RESULT == SDV_FAILED)
   #define SDV_HARNESS SDV_FLAT_DISPATCH_HARNESS_WITH_CLEANUP_LINKED_CALLBACKS
#else
  #pragma message("SDV_NA")  
#endif
#else
#pragma message("checkadddevice==SDV_FAILED")
#pragma message("checkirpmjpnp==SDV_FAILED")
#endif
