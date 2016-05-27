#ifndef NOTIFICATION_QUEUE_H
#define NOTIFICATION_QUEUE_H
#pragma once

#include <ntddk.h>

#include "../Notification.h"

typedef struct _NOTIFICATION_ENTRY
{
	LIST_ENTRY				ListEntry;
	OBSERVER_NOTIFICATION	Data;
} NOTIFICATION_ENTRY, *PNOTIFICATION_ENTRY;

NTSTATUS NotificationInitialize();

VOID NotificationSend(
	_In_ PNOTIFICATION_ENTRY Notification
);

NTSTATUS NotificationCopy (
	_In_ PVOID			Buffer,
	_In_ ULONG			BufferSize,
	_In_ PULONG			BytesRead
);

PNOTIFICATION_ENTRY NotificationCreate(
	_In_ ULONG NotificationType
);

extern FAST_MUTEX	NotificationListMutex;
extern LIST_ENTRY	NotificationList;
extern KEVENT		NotificationListEvent;


#define NOTIFICATION_TAG 'oNbO'
#define NOTIFICATION_ALLOCATE(size, type) ExAllocatePoolWithTag(type, size, NOTIFICATION_TAG)
#define NOTIFICATION_FREE(ptr) ExFreePoolWithTag(ptr, NOTIFICATION_TAG)

#endif // !NOTIFICATION_QUEUE_H
