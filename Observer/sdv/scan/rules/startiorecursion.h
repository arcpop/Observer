
#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
#if (startioroutine_SDV_RESULT == SDV_FAILED&&(adddeviceioattribcheck_SDV_RESULT == SDV_PASSED||adddeviceioattribcheck_SDV_RESULT == SDV_NA)&&(driverentryioattribcheck_SDV_RESULT == SDV_PASSED || driverentryioattribcheck_SDV_RESULT == SDV_NA))
  #define SDV_HARNESS SDV_FLAT_SIMPLE_HARNESS
#else
  #pragma message("SDV_NA")  
#endif
#else
#pragma message("startioroutine==SDV_FAILED")
#pragma message("adddeviceioattribcheck==SDV_PASSED,SDV_NA")
#pragma message("driverentryioattribcheck==SDV_PASSED,SDV_NA")
#endif
