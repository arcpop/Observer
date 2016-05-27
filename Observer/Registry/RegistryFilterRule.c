#include "Includes.h"
#include "../Rule.h"
#include "../Log/Log.h"


_Use_decl_annotations_
NTSTATUS RegistryFilterAddRule(
	PVOID pContext,
	POBSERVER_REGISTRY_RULE Rule,
	POBSERVER_RULE_HANDLE RuleHandle
)
{
	static LONG64 RuleCounter = 0;
	PREGISTRY_FILTER_CONTEXT Context = (PREGISTRY_FILTER_CONTEXT)pContext;
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

	InsertListEntry(
		&Context->FilteredRegistryKeysList,
		&RuleEntry->ListEntry
	);

	ReleaseRegistryFilterFilteredKeyEntry(RuleEntry);
	return STATUS_SUCCESS;
}


_Use_decl_annotations_
NTSTATUS RegistryFilterRemoveRule(
	PVOID pContext,
	POBSERVER_RULE_HANDLE RuleHandle
)
{
	//PREGISTRY_FILTER_CONTEXT Context = (PREGISTRY_FILTER_CONTEXT)pContext;
	UNREFERENCED_PARAMETER(pContext);
	UNREFERENCED_PARAMETER(RuleHandle);
	return STATUS_NOT_IMPLEMENTED;
}

