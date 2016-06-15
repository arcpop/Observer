#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
#if (dispatchroutine_SDV_RESULT == SDV_FAILED)
   #define SDV_FLAT_HARNESS_MODIFIER_NO_DRIVER_CANCEL
   #define No_DriverEntry
   #define No_AddDevice
   #define SDV_NO_DISPATCH_ROUTINE
   #define SDV_HARNESS SDV_PNP_HARNESS
#else
  #pragma message("SDV_NA")  
#endif
#else
#pragma message("dispatchroutine==SDV_FAILED")
#endif
