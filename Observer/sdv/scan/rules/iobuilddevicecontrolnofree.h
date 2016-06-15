
#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
#if (dispatchroutine_SDV_RESULT == SDV_FAILED && iobuilddevicecontrol_SDV_RESULT == SDV_FAILED)
   #define SDV_HARNESS SDV_FLAT_DISPATCH_HARNESS_WITH_LINKED_CALLBACKS
#else
  #pragma message("SDV_NA")  
#endif
#else
#pragma message("dispatchroutine==SDV_FAILED")
#pragma message("iobuilddevicecontrol==SDV_FAILED")
#endif
