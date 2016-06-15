
#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
#if (startioroutine_SDV_RESULT == SDV_FAILED || dispatchroutine_SDV_RESULT == SDV_FAILED || isrroutine_SDV_RESULT == SDV_FAILED || kedpcroutine_SDV_RESULT == SDV_FAILED || iodpcroutine_SDV_RESULT  == SDV_FAILED || iocompletion_SDV_RESULT == SDV_FAILED || workerthread_SDV_RESULT == SDV_FAILED)
  #define SDV_HARNESS SDV_FLAT_SIMPLE_HARNESS_WITH_COMPLETION_NO_CANCEL
#else
  #pragma message("SDV_NA")  
#endif
#else
#pragma message("startioroutine==SDV_FAILED")
#pragma message("dispatchroutine==SDV_FAILED")
#pragma message("isrroutine==SDV_FAILED")
#pragma message("kedpcroutine==SDV_FAILED")
#pragma message("cancelroutine==SDV_FAILED")
#pragma message("iodpcroutine==SDV_FAILED") 
#pragma message("iocompletion==SDV_FAILED")
#pragma message("workerthread==SDV_FAILED")
#endif
