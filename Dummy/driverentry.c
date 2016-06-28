#include <ntddk.h>

VOID DriverUnload(
	PDRIVER_OBJECT DriverObject
)
{
	DbgPrint("Dummy Driver: Unload routine\n");
	UNREFERENCED_PARAMETER(DriverObject);
}

NTSTATUS DriverEntry(
	PDRIVER_OBJECT DriverObject,
	PUNICODE_STRING RegistryPath
)
{
	DriverObject->DriverUnload = DriverUnload;
	DbgPrint("Dummy Driver: Init routine\n");
	UNREFERENCED_PARAMETER(RegistryPath);
	return STATUS_SUCCESS;
}

