#include <ntddk.h>

#include "Log\Log.h"
#include "RegistryFilter.h"

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD ObserverUnload;

PVOID g_RegistryFilterContext;

_Use_decl_annotations_
NTSTATUS DriverEntry(
	PDRIVER_OBJECT   DriverObject,
	PUNICODE_STRING  RegistryPath
)
{
	NTSTATUS Status;

	DriverObject->DriverUnload = ObserverUnload;

	DEBUG_LOG("DriverEntry: Starting the driver");

	g_RegistryFilterContext = NULL;

	Status = RegistryFilterInitialize(
		DriverObject, 
		&g_RegistryFilterContext
	);

	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("DriverEntry: RegistryFilterInitialize failed with error 0x%.8X", Status);
		return Status;
	}


	UNREFERENCED_PARAMETER(RegistryPath);
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
VOID ObserverUnload(
	PDRIVER_OBJECT DriverObject
)
{
	NTSTATUS Status;

	Status = RegistryFilterUnload(g_RegistryFilterContext);

	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("ObserverUnload: RegistryFilterUnload failed with error 0x%.8X", Status);
		return;
	}
	DEBUG_LOG("ObserverUnload: Stopping the driver");

	UNREFERENCED_PARAMETER(DriverObject);
	return;
}
