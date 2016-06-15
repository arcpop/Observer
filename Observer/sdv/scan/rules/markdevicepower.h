#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
  #if (checkbusdriver_SDV_RESULT == SDV_FAILED)
    #define IS_WDM_BUS_DRIVER
  #endif
      #define SDV_HARNESS SDV_SMALL_SMALL_POWERUP_HARNESS
#else
#pragma message("checkbusdriver==SDV_FAILED")
#endif
