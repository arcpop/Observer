#include "Notification.h"

_Use_decl_annotations_
NTSTATUS NotificationCopy(
	PVOID			Buffer,
	SIZE_T			BufferSize,
	KPROCESSOR_MODE RequestorMode
)
{
	NTSTATUS Status;
	PLIST_ENTRY pListEntry;
	PNOTIFICATION_ENTRY pEntry;
	if (BufferSize < sizeof(NOTIFICATION_DATA))
	{
		return STATUS_BUFFER_TOO_SMALL;
	}
	while(1)
	{
		Status = KeWaitForSingleObject(&NotificationListEvent, UserRequest, KernelMode, FALSE, NULL);
		if (!NT_SUCCESS(Status))
		{
			return Status;
		}
		ExAcquireFastMutex(&NotificationListMutex);
		if (IsListEmpty(&NotificationList))
		{
			ExReleaseFastMutex(&NotificationListMutex);
			continue;
		}
		pListEntry = RemoveHeadList(&NotificationList);
		ExReleaseFastMutex(&NotificationListMutex);
		pEntry = CONTAINING_RECORD(pListEntry, NOTIFICATION_ENTRY, ListEntry);
		break;
	}
	__try
	{
		RtlCopyMemory(Buffer, &pEntry->Data, sizeof(NOTIFICATION_DATA));
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return GetExceptionCode();
	}
	return STATUS_SUCCESS;
}

