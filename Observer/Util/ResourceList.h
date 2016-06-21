#ifndef RESOURCE_LIST
#define RESOURCE_LIST

#pragma once

#include <ntddk.h>

typedef struct _OBSERVER_RESOURCE_LIST
{
	LIST_ENTRY ListEntry;
	ERESOURCE Resource;
} OBSERVER_RESOURCE_LIST, *POBSERVER_RESOURCE_LIST;

#define RLockResourceList(ol) ExEnterCriticalRegionAndAcquireResourceShared(&((ol)->Resource))
#define RUnlockResourceList(ol) ExReleaseResourceAndLeaveCriticalRegion(&((ol)->Resource))

#define WLockResourceList(ol) ExEnterCriticalRegionAndAcquireResourceExclusive(&((ol)->Resource))
#define WUnlockResourceList(ol) ExReleaseResourceAndLeaveCriticalRegion(&((ol)->Resource))

VOID InitializeResourceList(
	_Inout_ POBSERVER_RESOURCE_LIST ResourceList
);

VOID InsertResourceListHead(
	_In_ POBSERVER_RESOURCE_LIST ResourceList,
	_In_ PLIST_ENTRY NewEntry
);
VOID InsertResourceListTail(
	_In_ POBSERVER_RESOURCE_LIST ResourceList,
	_In_ PLIST_ENTRY NewEntry
);

VOID RemoveResourceListEntry(
	_In_ POBSERVER_RESOURCE_LIST ResourceList,
	_In_ PLIST_ENTRY EntryToRemove
);


#endif //RESOURCE_LIST