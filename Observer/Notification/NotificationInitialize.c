#include "Notification.h"

_Use_decl_annotations_
NTSTATUS NotificationInitialize()
{
	InitializeListHead(&NotificationList);
	KeInitializeEvent(&NotificationListEvent, NotificationEvent, FALSE);
	ExInitializeFastMutex(&NotificationListMutex);
	return STATUS_SUCCESS;
}