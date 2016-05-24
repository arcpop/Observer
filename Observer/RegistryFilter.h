#ifndef _REGISTRY_FILTER_H_
#define _REGISTRY_FILTER_H_

#include <ntddk.h>


NTSTATUS RegistryFilterInitialize(
	_In_     PDRIVER_OBJECT pDriverObject,
	_Out_	 PVOID* ppContext
);

NTSTATUS RegistryFilterUnload(
	_In_     PVOID pContext
);

#endif // !_REGISTRY_FILTER_H_