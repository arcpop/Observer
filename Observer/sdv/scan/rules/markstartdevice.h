#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
#if (checkirpmjpower_SDV_RESULT == SDV_FAILED)
  #define SDV_HARNESS SDV_SMALL_START_SEQUENCE_HARNESS
  #define No_AddDevice
#else
  #pragma message("SDV_NA")  
#endif
#else
#pragma message("checkirpmjpower==SDV_FAILED")
#endif
