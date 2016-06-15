
#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
#if (danglingdeviceobjectcheck_SDV_RESULT==SDV_PASSED||danglingdeviceobjectcheck_SDV_RESULT==SDV_NA)
  #define SDV_HARNESS SDV_PNP_HARNESS_UNLOAD
#else
  #pragma message("SDV_NA")  
#endif
#else
#pragma message("danglingdeviceobjectcheck==SDV_PASSED,SDV_NA")
#endif
