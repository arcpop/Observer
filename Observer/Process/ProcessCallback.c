#include "ProcessObserver.h"

#include "../Log/Log.h"

RESOURCE_LIST_ENTRY_HEAD	ProcessRuleList;

_Use_decl_annotations_
NTSTATUS ProcessObserverInitialize()
{
	InitializeListEntryHead(&ProcessRuleList);
	return PsSetCreateProcessNotifyRoutineEx(ProcessNotifyRoutine, FALSE);
}

_Use_decl_annotations_
NTSTATUS ProcessObserverAddRule(
	POBSERVER_PROCESS_CREATION_RULE Rule,
	POBSERVER_RULE_HANDLE			RuleHandle
)
{
	static LONG64 RuleCounter = 0;
	PPROCESS_RULE_LIST_ENTRY pEntry;

	pEntry = PROCESS_OBSERVER_ALLOCATE(sizeof(PROCESS_RULE_LIST_ENTRY), NonPagedPool);
	if (pEntry == NULL)
	{
		DEBUG_LOG("ProcessObserverAddRule: Out of memory");
		return STATUS_NO_MEMORY;
	}

	RtlCopyMemory(
		&pEntry->Rule,
		Rule,
		sizeof(PROCESS_RULE_LIST_ENTRY)
	);
	RuleHandle->RuleHandle = pEntry->RuleHandle.RuleHandle = InterlockedIncrement64(&RuleCounter);
	RuleHandle->RuleType = pEntry->RuleHandle.RuleType = RULE_TYPE_CREATE_PROCESS;

	InsertListEntry(&ProcessRuleList, &pEntry->ListEntry);
	ReleaseListEntry(&pEntry->ListEntry);
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS ProcessObserverRemoveRule(
	POBSERVER_RULE_HANDLE RuleHandle
)
{
	PRESOURCE_LIST_ENTRY pEntry;
	for (
		pEntry = NextListEntry(
			&ProcessRuleList,
			&ProcessRuleList.Entry,
			FALSE);
		pEntry != &ProcessRuleList.Entry;
		pEntry = NextListEntry(
			&ProcessRuleList,
			pEntry,
			TRUE)
		)
	{
		PPROCESS_RULE_LIST_ENTRY pCurrentEntry = CONTAINING_RECORD(pEntry, PROCESS_RULE_LIST_ENTRY, ListEntry);
		if (RuleHandle->RuleHandle == pCurrentEntry->RuleHandle.RuleHandle)
		{
			RemoveListEntry(&ProcessRuleList, pEntry);
			if (ReleaseListEntry(pEntry))
			{
				PROCESS_OBSERVER_FREE(pCurrentEntry);
			}
			return STATUS_SUCCESS;
		}
	}
	return STATUS_NOT_FOUND;
}