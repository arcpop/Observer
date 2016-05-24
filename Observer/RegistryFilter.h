#ifndef _REGISTRY_FILTER_H_
#define _REGISTRY_FILTER_H_

#include <ntddk.h>


NTSTATUS RegistryFilterInitialize(
	_In_     PDRIVER_OBJECT pDriverObject,
	_Out_	 PVOID* ppContext
);

#endif // !_REGISTRY_FILTER_H_