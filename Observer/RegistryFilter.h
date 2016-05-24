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

NTSTATUS RegistryFilterAdd(
	_In_ PVOID pContext,
	_In_ PUNICODE_STRING KeyPath,
	_In_ LONG ActionFlags,
	_In_ PVOID Reserved
);

#endif // !_REGISTRY_FILTER_H_