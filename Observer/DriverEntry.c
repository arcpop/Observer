#include <ntddk.h>

#include "Log\Log.h"
#include "RegistryFilter.h"
#include "DeviceIO\DeviceIO.h"
#include "Process.h"
#include "Notification/NotificationQueue.h"

const wchar_t RegPath[] = L"Windows\\CurrentVersion\\Run";
const wchar_t CmdProcess[] = L"\\cmd.exe";
const wchar_t DllhostProcess[] = L"dllhost";

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
	struct 
	{
		OBSERVER_PROCESS_CREATION_RULE Rule;
		WCHAR Buffer[1024];
	} Proc;
	struct 
	{
		OBSERVER_REGISTRY_RULE Rule;
		WCHAR Buffer[1024];
	} Reg;
	OBSERVER_RULE_HANDLE RegRuleHandle;
	OBSERVER_RULE_HANDLE RuleHandle;

	DEBUG_LOG("DriverEntry: Starting the driver");

	DriverObject->DriverUnload = ObserverUnload;

	g_RegistryFilterContext = NULL;

	Status = NotificationInitialize();

	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("DriverEntry: NotificationInitialize failed with error 0x%.8X", Status);
		return Status;
	}
	Status = RegistryFilterInitialize(
		DriverObject, 
		&g_RegistryFilterContext
	);

	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("DriverEntry: RegistryFilterInitialize failed with error 0x%.8X", Status);
		return Status;
	}
	
	Status = ProcessObserverInitialize();

	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("DriverEntry: ProcessObserverInitialize failed with error 0x%.8X", Status);
		return Status;
	}
	
	
	Status = DeviceIOInitialize(
		DriverObject
	);

	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("DriverEntry: DeviceIOInitialize failed with error 0x%.8X", Status);
		return Status;
	}
	
	Proc.Rule.Action = ACTION_REPORT;
	Proc.Rule.ProcessRuleCheckFlags = PROCESS_CREATION_CHECK_PARENT_NAME_CONTAINS;
	RtlCopyMemory(&Proc.Rule.ParentProcessName[0], DllhostProcess, sizeof(DllhostProcess) - sizeof(wchar_t));
	Proc.Rule.ParentProcessNameLength = (sizeof(DllhostProcess) / 2) - 1;
	Status = ProcessObserverAddRule(&Proc.Rule, &RuleHandle);
	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("DriverEntry: ProcessObserverAddRule failed with error 0x%.8X", Status);
	}

	Proc.Rule.Action = ACTION_BLOCK;
	Proc.Rule.ProcessRuleCheckFlags = PROCESS_CREATION_CHECK_PARENT_NAME_ENDS_WITH;
	RtlCopyMemory(&Proc.Rule.ParentProcessName[0], CmdProcess, sizeof(CmdProcess) - sizeof(wchar_t));
	Proc.Rule.ParentProcessNameLength = (sizeof(CmdProcess) / 2) - 1;
	Status = ProcessObserverAddRule(&Proc.Rule, &RuleHandle);
	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("DriverEntry: ProcessObserverAddRule failed with error 0x%.8X", Status);
	}
	
	RtlSecureZeroMemory(&Reg, sizeof(Reg));
	Reg.Rule.Type = REGISTRY_TYPE_OPEN_KEY;
	Reg.Rule.Action = ACTION_REPORT | ACTION_BLOCK | ACTION_DBGPRINT;
	Reg.Rule.KeyMatch = REGISTRY_MATCH_CONTAINS | REGISTRY_MATCH_IGNORE_CASE;
	RtlCopyMemory(&Reg.Rule.Path[0], RegPath, sizeof(RegPath) - sizeof(wchar_t));
	Reg.Rule.PathLength = sizeof(RegPath) / 2 - 1;
	Status = RegistryFilterAddRule(&Reg.Rule, &RegRuleHandle);
	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("DriverEntry: RegistryFilterAddRule failed with error 0x%.8X", Status);
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

	Status = RegistryFilterUnload(
		g_RegistryFilterContext
	);

	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("ObserverUnload: RegistryFilterUnload failed with error 0x%.8X", Status);
	}
	
	Status = ProcessObserverUnload();

	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("ObserverUnload: ProcessObserverUnload failed with error 0x%.8X", Status);
	}
	

	Status = DeviceIOUnload(
		DriverObject
	);

	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("ObserverUnload: DeviceIOUnload failed with error 0x%.8X", Status);
	}
	DEBUG_LOG("ObserverUnload: Stopping the driver");

	UNREFERENCED_PARAMETER(DriverObject);
	return;
}
