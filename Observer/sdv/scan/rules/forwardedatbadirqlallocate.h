
#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
#if ((startioroutine_SDV_RESULT == SDV_FAILED || dispatchroutine_SDV_RESULT == SDV_FAILED || cancelroutine_SDV_RESULT == SDV_FAILED)&&ioallocate_SDV_RESULT == SDV_FAILED )
  #define SDV_HARNESS SDV_FLAT_SIMPLE_HARNESS_WITH_COMPLETION_ONLY
#else
  #pragma message("SDV_NA")  
#endif
#else
#pragma message("startioroutine==SDV_FAILED")
#pragma message("dispatchroutine==SDV_FAILED")
#pragma message("cancelroutine==SDV_FAILED")
#pragma message("ioallocate==SDV_FAILED")
#endif
