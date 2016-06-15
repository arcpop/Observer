
#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
#if ((checkbusdriver_SDV_RESULT == SDV_NA || checkbusdriver_SDV_RESULT==SDV_PASSED))
    #define SDV_HARNESS SDV_FLAT_SIMPLE_HARNESS_WITH_WMI_ONLY
#else
    #pragma message("SDV_NA")  
#endif
#else
    #pragma message("checkbusdriver==SDV_PASSED,SDV_NA")
#endif
