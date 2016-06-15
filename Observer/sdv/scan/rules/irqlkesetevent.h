#ifndef SDV_PRE_RUN
#include "..\..\sdv-pre-results.h"
#if (initializedpcrequest_SDV_RESULT == SDV_FAILED)
    #define SDV_HARNESS SDV_PNP_HARNESS_SMALL_WITH_LINKED_CALLBACKS
#else
    #define SDV_HARNESS SDV_FLAT_DISPATCH_HARNESS_WITH_LINKED_CALLBACKS
#endif
#else
#pragma message("initializedpcrequest==SDV_FAILED")
#endif


