#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
  #if (removecheck_SDV_RESULT == SDV_FAILED||removecheck2_SDV_RESULT == SDV_FAILED)
    #define SDV_FLAT_HARNESS_MODIFIER_NO_UNLOAD
    #define SDV_HARNESS_COMPLETION_ROUTINE
    #define SDV_NO_MN_REMOVE_DEVICE 
    #define SDV_FLAT_HARNESS_MODIFIER_NO_IRP_MJ_PNP_MN_REMOVE_DEVICE
    #define SDV_NON_BUS_MN_FUNCTIONS
    #define SDV_HARNESS_POWER_COMPLETION_ROUTINE
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE_EX
    #define SDV_HARNESS_QUEUE_WORK_ITEMS_ROUTINE
    #define SDV_HARNESS SDV_PNP_HARNESS_SMALL
  #else
     #pragma message("SDV_NA")  
   #endif
#else
#pragma message("removecheck==SDV_FAILED")
#pragma message("removecheck2==SDV_FAILED")
#endif




