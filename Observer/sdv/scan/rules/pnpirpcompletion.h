#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
#if ((checkbusdriver_SDV_RESULT == SDV_PASSED || checkbusdriver_SDV_RESULT == SDV_NA))
  #define SDV_HARNESS SDV_FLAT_DISPATCH_HARNESS_WITH_PNP_IRPS_EXCLUDING_QUERY_AND_START
#else
  #pragma message("SDV_NA")  
#endif
#else
#pragma message("checkbusdriver==SDV_PASSED,SDV_NA")
#endif
