#include "ResourceList.h"

_Use_decl_annotations_
VOID InitializeResourceList(
	_Inout_ POBSERVER_RESOURCE_LIST ResourceList
)
{
	InitializeListHead(&ResourceList->ListEntry);
	ExInitializeResourceLite(&ResourceList->Resource);
}

_Use_decl_annotations_
VOID InsertResourceListHead(
	_In_ POBSERVER_RESOURCE_LIST ResourceList,
	_In_ PLIST_ENTRY NewEntry
)
{
	WLockResourceList(ResourceList);
	InsertHeadList(&ResourceList->ListEntry, NewEntry);
	WUnlockResourceList(ResourceList);
}


_Use_decl_annotations_
VOID InsertResourceListTail(
	_In_ POBSERVER_RESOURCE_LIST ResourceList,
	_In_ PLIST_ENTRY NewEntry
)
{
	WLockResourceList(ResourceList);
	InsertTailList(&ResourceList->ListEntry, NewEntry);
	WUnlockResourceList(ResourceList);
}


_Use_decl_annotations_
VOID RemoveResourceListEntry(
	_In_ POBSERVER_RESOURCE_LIST ResourceList,
	_In_ PLIST_ENTRY EntryToRemove
)
{
	WLockResourceList(ResourceList);
	RemoveEntryList(EntryToRemove);
	WUnlockResourceList(ResourceList);
}