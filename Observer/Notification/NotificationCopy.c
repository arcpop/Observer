#include "NotificationQueue.h"

_Use_decl_annotations_
NTSTATUS NotificationCopy(
	PVOID			Buffer,
	ULONG			BufferSize,
	PULONG			BytesRead
)
{
	NTSTATUS Status;
	PLIST_ENTRY pListEntry;
	PNOTIFICATION_ENTRY pEntry;
	if (BufferSize < sizeof(OBSERVER_NOTIFICATION))
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
	RtlCopyMemory(Buffer, &pEntry->Data, sizeof(OBSERVER_NOTIFICATION));
	*BytesRead = sizeof(OBSERVER_NOTIFICATION);
	NOTIFICATION_FREE(pEntry);
	return STATUS_SUCCESS;
}

