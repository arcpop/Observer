#include "NotificationQueue.h"

#include "../Log/Log.h"

_Use_decl_annotations_
PNOTIFICATION_ENTRY NotificationCreate(
	ULONG NotificationType
)
{
	PNOTIFICATION_ENTRY pNotification;

	pNotification = NOTIFICATION_ALLOCATE(sizeof(NOTIFICATION_ENTRY), NonPagedPool);

	if (pNotification == NULL)
	{
		DEBUG_LOG("NotificationCreate: Out of memory");
		return NULL;
	}

	pNotification->Data.CurrentProcessID = (UINT64)PsGetCurrentProcessId();
	pNotification->Data.CurrentThreadID = (UINT64)PsGetCurrentThreadId();
	pNotification->Data.NotificationType = NotificationType;

	return pNotification; 
}