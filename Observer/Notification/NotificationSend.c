#include "Notification.h"

FAST_MUTEX	NotificationListMutex;
LIST_ENTRY	NotificationList;
KEVENT		NotificationListEvent;

_Use_decl_annotations_
VOID NotificationSend(
	PNOTIFICATION_ENTRY Notification
)
{
	ExAcquireFastMutex(&NotificationListMutex);
	InsertTailList(&NotificationList, &Notification->ListEntry);
	KeSetEvent(&NotificationListEvent, 0, FALSE);
	ExReleaseFastMutex(&NotificationListMutex);
}



