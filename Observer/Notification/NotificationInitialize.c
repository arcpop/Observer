#include "Notification.h"

_Use_decl_annotations_
NTSTATUS NotificationInitialize()
{
	InitializeListHead(&NotificationList);
	KeInitializeEvent(&NotificationListEvent, NotificationEvent, FALSE);
	KeInitializeMutex(&NotificationListMutex, 0);
	return STATUS_SUCCESS;
}