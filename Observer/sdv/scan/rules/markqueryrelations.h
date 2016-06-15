#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
  #if (checkbusdriver_SDV_RESULT == SDV_FAILED)
    #define IS_WDM_BUS_DRIVER
    #define SDV_HARNESS_COMPLETION_ROUTINE
    #define SDV_HARNESS SDV_FLAT_QUERY_DEVICE_RELATIONS_HARNESS
  #else
    #pragma message("SDV_NA")  
  #endif
#else
#pragma message("checkbusdriver==SDV_FAILED")
#endif
