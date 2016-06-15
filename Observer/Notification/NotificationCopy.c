#include "NotificationQueue.h"
#include "../Log/Log.h"

_Use_decl_annotations_
NTSTATUS NotificationCopy(
	PVOID			Buffer,
	ULONG			BufferSize,
	PULONG			BytesRead
)
{
	PLIST_ENTRY pListEntry;
	PNOTIFICATION_ENTRY pEntry;
	if (BufferSize < sizeof(OBSERVER_NOTIFICATION))
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	ExAcquireFastMutex(&NotificationListMutex);
	if (IsListEmpty(&NotificationList))
	{
		ExReleaseFastMutex(&NotificationListMutex);
		*BytesRead = 0;
		return STATUS_MORE_PROCESSING_REQUIRED;
	}
	pListEntry = RemoveHeadList(&NotificationList);
	ExReleaseFastMutex(&NotificationListMutex);
	pEntry = CONTAINING_RECORD(pListEntry, NOTIFICATION_ENTRY, ListEntry);
	RtlCopyMemory(Buffer, &pEntry->Data, sizeof(OBSERVER_NOTIFICATION));
	*BytesRead = sizeof(OBSERVER_NOTIFICATION);
	NOTIFICATION_FREE(pEntry);
	return STATUS_SUCCESS;
}

