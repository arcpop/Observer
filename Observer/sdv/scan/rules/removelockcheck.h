#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
  #if ((removecheck_SDV_RESULT == SDV_FAILED)&&(checkbusdriver_SDV_RESULT!=SDV_FAILED))
     #define SDV_HARNESS_COMPLETION_ROUTINE
     #define SDV_IRP_MN_REMOVE_DEVICE
     #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
     #define No_DriverEntry
     #define No_AddDevice
     #define SDV_IRP_MN_REMOVE_DEVICE_ONLY
     #define SDV_HARNESS SDV_PNP_BASIC_REMOVE_DEVICE_HARNESS
  #else
     #pragma message("SDV_NA")  
   #endif
#else
#pragma message("removecheck==SDV_FAILED")
#pragma message("checkbusdriver==SDV_FAILED")
#endif
