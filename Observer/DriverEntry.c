#include <ntddk.h>

#include "Log\Log.h"
#include "RegistryFilter.h"
#include "DeviceIO\DeviceIO.h"
#include "Process.h"
#include "Notification/NotificationQueue.h"
#include "Image\ImageLoadNotify.h"
#include "Util\Processcache.h"


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
	DEBUG_LOG("DriverEntry: Starting the driver");

	DriverObject->DriverUnload = ObserverUnload;

	g_RegistryFilterContext = NULL;

	Status = ProcessCacheInitialize();
	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("DriverEntry: ProcessCacheInitialize failed with error 0x%.8X", Status);
		return Status;
	}

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


	Status = InitImageLoadNotifications();
	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("DriverEntry: InitImageLoadNotifications failed with error 0x%.8X", Status);
		return Status;
	}


#ifdef DBG
	{
		const wchar_t DrvPath[] = L"\\System32\\drivers\\";
		struct
		{
			OBSERVER_DRIVER_LOAD_RULE Rule;
			WCHAR Buffer[1024];
		} Drv;
		OBSERVER_RULE_HANDLE RuleHandle;
		Drv.Rule.Action = ACTION_REPORT;
		Drv.Rule.DriverLoadCheckFlags = DRIVER_LOAD_CHECK_PATH_NOT_CONTAINS | DRIVER_LOAD_CHECK_CASE_INSENSITIVE;
		Drv.Rule.PathLength = (sizeof(DrvPath) / sizeof(wchar_t)) - 1;
		RtlCopyMemory(Drv.Rule.Path, DrvPath, sizeof(DrvPath));
		Status = DriverLoadAddRule(&Drv.Rule, &RuleHandle);
		if (!NT_SUCCESS(Status))
		{
			DEBUG_LOG("DriverEntry: DriverLoadAddRule failed with error 0x%.8X", Status);
		}
	}
	{
		const wchar_t TempProcess[] = L"\\Windows\\Temp\\";
		const wchar_t AppDataProcess[] = L"\\AppData\\";
		const wchar_t AppDataTempProcess[] = L"\\AppData\\Local\\Temp\\";
		struct
		{
			OBSERVER_PROCESS_CREATION_RULE Rule;
			WCHAR Buffer[1024];
		} Proc;
		OBSERVER_RULE_HANDLE RuleHandle;
		Proc.Rule.Action = ACTION_REPORT;
		Proc.Rule.ProcessRuleCheckFlags = PROCESS_CREATION_CHECK_NAME_CONTAINS;
		RtlCopyMemory(&Proc.Rule.ParentProcessName[0], AppDataProcess, sizeof(AppDataProcess) - sizeof(wchar_t));
		Proc.Rule.ParentProcessNameLength = (sizeof(AppDataProcess) / 2) - 1;
		Status = ProcessObserverAddRule(&Proc.Rule, &RuleHandle);
		if (!NT_SUCCESS(Status))
		{
			DEBUG_LOG("DriverEntry: ProcessObserverAddRule failed with error 0x%.8X", Status);
		}

		Proc.Rule.Action = ACTION_REPORT | ACTION_BLOCK;
		Proc.Rule.ProcessRuleCheckFlags = PROCESS_CREATION_CHECK_NAME_CONTAINS;
		RtlCopyMemory(&Proc.Rule.ParentProcessName[0], TempProcess, sizeof(TempProcess) - sizeof(wchar_t));
		Proc.Rule.ParentProcessNameLength = (sizeof(TempProcess) / 2) - 1;
		Status = ProcessObserverAddRule(&Proc.Rule, &RuleHandle);
		if (!NT_SUCCESS(Status))
		{
			DEBUG_LOG("DriverEntry: ProcessObserverAddRule failed with error 0x%.8X", Status);
		}

		Proc.Rule.Action = ACTION_REPORT | ACTION_BLOCK;
		Proc.Rule.ProcessRuleCheckFlags = PROCESS_CREATION_CHECK_NAME_CONTAINS;
		RtlCopyMemory(&Proc.Rule.ParentProcessName[0], AppDataTempProcess, sizeof(AppDataTempProcess) - sizeof(wchar_t));
		Proc.Rule.ParentProcessNameLength = (sizeof(AppDataTempProcess) / 2) - 1;
		Status = ProcessObserverAddRule(&Proc.Rule, &RuleHandle);
		if (!NT_SUCCESS(Status))
		{
			DEBUG_LOG("DriverEntry: ProcessObserverAddRule failed with error 0x%.8X", Status);
		}

	}
	{
		const wchar_t RegPath[] = L"\\Microsoft\\Windows\\CurrentVersion\\Run";
		struct
		{
			OBSERVER_REGISTRY_RULE Rule;
			WCHAR Buffer[1024];
		} Reg;
		OBSERVER_RULE_HANDLE RegRuleHandle;
		RtlSecureZeroMemory(&Reg, sizeof(Reg));
		Reg.Rule.Type = REGISTRY_TYPE_SET_VALUE;
		Reg.Rule.Action = ACTION_REPORT | ACTION_DBGPRINT;
		Reg.Rule.KeyMatch = REGISTRY_MATCH_CONTAINS | REGISTRY_MATCH_IGNORE_CASE;
		Reg.Rule.ValueMatch = 0;
		RtlCopyMemory(&Reg.Rule.Path[0], RegPath, sizeof(RegPath) - sizeof(wchar_t));
		Reg.Rule.PathLength = sizeof(RegPath) / 2 - 1;
		Status = RegistryFilterAddRule(&Reg.Rule, &RegRuleHandle);
		if (!NT_SUCCESS(Status))
		{
			DEBUG_LOG("DriverEntry: RegistryFilterAddRule failed with error 0x%.8X", Status);
		}
	}
#endif //DBG
	UNREFERENCED_PARAMETER(RegistryPath);
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
VOID ObserverUnload(
	PDRIVER_OBJECT DriverObject
)
{
	NTSTATUS Status;

	UnloadImageNotifications();

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

	Status = ProcessCacheUnload();
	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("ObserverUnload: ProcessCacheUnload failed with error 0x%.8X", Status);
	}


	DEBUG_LOG("ObserverUnload: Stopping the driver");

	UNREFERENCED_PARAMETER(DriverObject);
	return;
}
