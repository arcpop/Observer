#ifndef LOG_H
#define LOG_H
#pragma once


#include <ntddk.h>

#ifdef DBG
#define DEBUG_LOG(fmt, ...) DbgPrint(fmt "\n", __VA_ARGS__)
#else
#define DEBUG_LOG(fmt, ...) 
#endif

#endif // !LOG_H