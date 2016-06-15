#include "NotificationQueue.h"

_Use_decl_annotations_
NTSTATUS NotificationInitialize()
{
	InitializeListHead(&NotificationList);
	ExInitializeFastMutex(&NotificationListMutex);
	return STATUS_SUCCESS;
}