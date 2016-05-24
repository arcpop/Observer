#include "../ResourceList.h"

_Use_decl_annotations_
VOID InitializeListEntryHead(
	PRESOURCE_LIST_ENTRY_HEAD Head
)
{
	InitializeListHead(&Head->Entry);
	KeInitializeSpinLock(&Head->SpinLock);
}

_Use_decl_annotations_
VOID InsertListEntry(
	PRESOURCE_LIST_ENTRY_HEAD Head,
	PRESOURCE_LIST_ENTRY Entry
)
{
	KIRQL OldIrql;

	//One refcount for the list and one for the caller
	Entry->RefCount = 1 + 1;
	KeAcquireSpinLock(&Head->SpinLock, &OldIrql);
	InsertHeadList(&Head->Entry.ListEntry, &Entry->ListEntry);
	KeReleaseSpinLock(&Head->SpinLock, OldIrql);
}

_Use_decl_annotations_
PRESOURCE_LIST_ENTRY RemoveListEntry(
	PRESOURCE_LIST_ENTRY_HEAD Head,
	PRESOURCE_LIST_ENTRY Entry
)
{
	KIRQL OldIrql;
	PLIST_ENTRY pRemovedEntry;

	KeAcquireSpinLock(&Head->SpinLock, &OldIrql);
	pRemovedEntry = RemoveHeadList(&Entry->ListEntry, &Head->SpinLock);
	if (pRemovedEntry == &Entry->ListEntry)
	{
		//Only decrease Refcount if the entry was in our list!
		InterlockedAdd(&Entry->RefCount, -1);
		KeReleaseSpinLock(&Head->SpinLock, OldIrql);
		return Entry;
	}
	KeReleaseSpinLock(&Head->SpinLock, OldIrql);
	return NULL;
}

_Use_decl_annotations_
PRESOURCE_LIST_ENTRY NextListEntry(
	PRESOURCE_LIST_ENTRY_HEAD Head,
	PRESOURCE_LIST_ENTRY CurrentEntry,
	BOOLEAN ReleaseCurrent
)
{
	PRESOURCE_LIST_ENTRY pEntry;
	KIRQL OldIrql;

	KeAcquireSpinLock(&Head->SpinLock, &OldIrql);
	pEntry = CurrentEntry->ListEntry.Flink;
	InterlockedAdd(&pEntry->RefCount, 1);
	if (ReleaseCurrent)
	{
		InterlockedAdd(&CurrentEntry->RefCount, -1);
	}
	KeReleaseSpinLock(&Head->SpinLock, OldIrql);
	return pEntry;
}

_Use_decl_annotations_
BOOLEAN ReleaseListEntry(
	PRESOURCE_LIST_ENTRY pEntry
)
{
	LONG CurrentCount;

	CurrentCount = InterlockedAdd(&pEntry->RefCount, -1);
	return CurrentCount == 0;
}


