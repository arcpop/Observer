#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
  #if (checkbusdriver_SDV_RESULT == SDV_FAILED)
    #define IS_WDM_BUS_DRIVER
  #endif
  #if (checkirpmjpower_SDV_RESULT==SDV_FAILED)
      #define SDV_HARNESS SDV_SMALL_POWERDOWN_HARNESS
  #endif
#else
#pragma message("checkbusdriver==SDV_FAILED")
#pragma message("checkirpmjpower==SDV_FAILED")
#endif
