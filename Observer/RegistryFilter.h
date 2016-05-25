#ifndef REGISTRY_FILTER_H
#define REGISTRY_FILTER_H

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

#endif // !REGISTRY_FILTER_H