#ifndef NOTIFICATION_H
#define NOTIFICATION_H
#pragma once

#include <ntddk.h>

#include "../NotificationData.h"

typedef struct _NOTIFICATION_ENTRY
{
	LIST_ENTRY			ListEntry;
	NOTIFICATION_DATA	Data;
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

extern FAST_MUTEX	NotificationListMutex;
extern LIST_ENTRY	NotificationList;
extern KEVENT		NotificationListEvent;

#endif // !NOTIFICATION_H
