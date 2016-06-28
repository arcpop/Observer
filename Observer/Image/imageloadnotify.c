#include "ImageLoadNotify.h"
#include "../Log/Log.h"



OBSERVER_RESOURCE_LIST ModuleLoadRuleList;


VOID HandleExecutableImageLoad(
	_In_ HANDLE ProcessId,
	_In_ PUNICODE_STRING FullImageName,
	_In_ PIMAGE_INFO ImageInfo
)
{
	//DEBUG_LOG("ImageLoad: Loaded %wZ in %.8X", FullImageName, ProcessId);
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(FullImageName);
	UNREFERENCED_PARAMETER(ImageInfo);
}
VOID ObserverImageLoadNotify(
	_In_opt_ PUNICODE_STRING FullImageName,
	_In_ HANDLE ProcessId,
	_In_ PIMAGE_INFO ImageInfo
)
{
	if (FullImageName == NULL)
	{
		return;
	}

	if ((ImageInfo->SystemModeImage) || (ProcessId == NULL))
	{
		HandleDriverLoad(FullImageName, ImageInfo);
		return;
	}
	else
	{
		HandleExecutableImageLoad(ProcessId, FullImageName, ImageInfo);
		return;
	}
}

static BOOLEAN gLoaded = FALSE;
NTSTATUS InitImageLoadNotifications()
{
	NTSTATUS Status;
	InitializeResourceList(&DriverLoadRuleList);
	InitializeResourceList(&ModuleLoadRuleList);
	Status = PsSetLoadImageNotifyRoutine(ObserverImageLoadNotify);
	gLoaded = TRUE;
	return Status;
}

VOID UnloadImageNotifications()
{
	PLIST_ENTRY pEntry;

	PsRemoveLoadImageNotifyRoutine(ObserverImageLoadNotify);

	WLockResourceList(&DriverLoadRuleList);
	for (pEntry = DriverLoadRuleList.ListEntry.Flink; pEntry != &DriverLoadRuleList.ListEntry; )
	{
		POBSERVER_DRIVER_LOAD_RULE_ENTRY CurrentEntry = CONTAINING_RECORD(pEntry, OBSERVER_DRIVER_LOAD_RULE_ENTRY, ListEntry);
		pEntry = pEntry->Flink;
		RemoveEntryList(&CurrentEntry->ListEntry);
		IMAGE_NOTIFICATION_FREE(CurrentEntry);
	}
	WUnlockResourceList(&DriverLoadRuleList);
	ExDeleteResourceLite(&DriverLoadRuleList.Resource);

	WLockResourceList(&ModuleLoadRuleList);
	for (pEntry = ModuleLoadRuleList.ListEntry.Flink; pEntry != &ModuleLoadRuleList.ListEntry; )
	{
		POBSERVER_DRIVER_LOAD_RULE_ENTRY CurrentEntry = CONTAINING_RECORD(pEntry, OBSERVER_DRIVER_LOAD_RULE_ENTRY, ListEntry);
		pEntry = pEntry->Flink;
		RemoveEntryList(&CurrentEntry->ListEntry);
		IMAGE_NOTIFICATION_FREE(CurrentEntry);
	}
	WUnlockResourceList(&ModuleLoadRuleList);
	ExDeleteResourceLite(&ModuleLoadRuleList.Resource);
}