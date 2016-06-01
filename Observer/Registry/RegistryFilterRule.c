#include "Includes.h"
#include "../Rule.h"
#include "../Log/Log.h"

LIST_ENTRY RegistryFilterRuleList;
FAST_MUTEX RegistryFilterRuleListMutex;

_Use_decl_annotations_
NTSTATUS RegistryFilterAddRule(
	POBSERVER_REGISTRY_RULE Rule,
	POBSERVER_RULE_HANDLE RuleHandle
)
{
	static LONG64 RuleCounter = 0;
	PREGISTRY_FILTER_RULE_ENTRY RuleEntry = NULL;

	RuleEntry = REGISTRY_FILTER_ALLOCATE(
		sizeof(REGISTRY_FILTER_RULE_ENTRY) + 
		((Rule->PathLength + 1) * sizeof(WCHAR)),
		NonPagedPool
	);

	if (RuleEntry == NULL)
	{
		DEBUG_LOG("RegistryFilterAddRule: Out of memory");
		return STATUS_NO_MEMORY;
	}
	RtlCopyMemory(
		&RuleEntry->Rule,
		Rule,
		sizeof(OBSERVER_REGISTRY_RULE)
	);

	RtlCopyMemory(
		RuleEntry->Rule.Path,
		Rule->Path,
		RuleEntry->Rule.PathLength * sizeof(WCHAR)
	);

	RuleEntry->Rule.Path[RuleEntry->Rule.PathLength] = L'\0';



	RuleHandle->RuleHandle = RuleEntry->RuleHandle.RuleHandle = InterlockedIncrement64(&RuleCounter);
	RuleHandle->RuleType = RuleEntry->RuleHandle.RuleType = RULE_TYPE_REGISTRY;

	RtlInitUnicodeString(&RuleEntry->Path, RuleEntry->Rule.Path);

	ExInitializeRundownProtection(&RuleEntry->RundownProtection);

	ExAcquireFastMutex(&RegistryFilterRuleListMutex);

	InsertHeadList(
		&RegistryFilterRuleList,
		&RuleEntry->ListEntry
	);
	ExReleaseFastMutex(&RegistryFilterRuleListMutex);
	return STATUS_SUCCESS;
}


_Use_decl_annotations_
NTSTATUS RegistryFilterRemoveRule(
	POBSERVER_RULE_HANDLE RuleHandle
)
{
	PLIST_ENTRY pEntry;

	if (RuleHandle->RuleType != RULE_TYPE_REGISTRY)
	{
		return STATUS_NOT_FOUND;
	}

	for (
		pEntry = NextRegistryFilterRuleListEntry(&RegistryFilterRuleList, FALSE);
		pEntry != NULL;
		pEntry = NextRegistryFilterRuleListEntry(&RegistryFilterRuleList, TRUE)
		)
	{
		PREGISTRY_FILTER_RULE_ENTRY CurrentEntry;
		CurrentEntry = CONTAINING_RECORD(pEntry, REGISTRY_FILTER_RULE_ENTRY, ListEntry);

		if (CurrentEntry->RuleHandle.RuleHandle == RuleHandle->RuleHandle)
		{
			ExAcquireFastMutex(&RegistryFilterRuleListMutex);
			if (pEntry->Blink == NULL)
			{
				//Somebody is removing us at the moment.
				ExReleaseFastMutex(&RegistryFilterRuleListMutex);
				ExReleaseRundownProtection(&CurrentEntry->RundownProtection);
				return STATUS_NOT_FOUND;
			}
			RemoveHeadList(pEntry->Blink);
			ExReleaseFastMutex(&RegistryFilterRuleListMutex);
			ExWaitForRundownProtectionRelease(&CurrentEntry->RundownProtection);
			REGISTRY_FILTER_FREE(CurrentEntry);
			return STATUS_SUCCESS;
		}
	}
	return STATUS_NOT_FOUND;
}

