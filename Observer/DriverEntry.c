#include <ntddk.h>

#include "Log\Log.h"
#include "RegistryFilter.h"
#include "Registry\ActionFlags.h"

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
	UNICODE_STRING uTestFilter;

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

	RtlInitUnicodeString(
		&uTestFilter,
		L"\\REGISTRY\\USER\\S-1-5-21-945703056-3533603644-3919355012-1000\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
	);

	Status = RegistryFilterAdd(
		g_RegistryFilterContext,
		&uTestFilter,
		ACTIONFLAG_BLOCK,
		NULL
	);
	RtlInitUnicodeString(
		&uTestFilter,
		L"\\REGISTRY\\USER\\S-1-5-21-945703056-3533603644-3919355012-1000\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"
	);

	Status = RegistryFilterAdd(
		g_RegistryFilterContext,
		&uTestFilter,
		ACTIONFLAG_NOTIFY,
		NULL
	);

	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("DriverEntry: RegistryFilterAdd failed with error 0x%.8X", Status);
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
