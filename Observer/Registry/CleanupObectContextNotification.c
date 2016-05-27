#include "Includes.h"

#include "../Log/Log.h"

_Use_decl_annotations_
NTSTATUS RegistryFilterCleanupObjectContext(
	PREGISTRY_FILTER_CONTEXT pContext, 
	PREG_CALLBACK_CONTEXT_CLEANUP_INFORMATION Info
)
{
	PREGISTRY_FILTER_OBJECT_CONTEXT pObjectContext;
	pObjectContext = (PREGISTRY_FILTER_OBJECT_CONTEXT)Info->ObjectContext;

	if (pObjectContext == NULL)
	{
		DEBUG_LOG("RegistryFilterCleanupObjectContext: ObjectContext is NULL");
		return STATUS_INVALID_PARAMETER;
	}

	ReleaseRegistryFilterFilteredKeyEntry(pObjectContext->RuleEntry);

	REGISTRY_FILTER_FREE(pObjectContext);

	UNREFERENCED_PARAMETER(pContext);

	return STATUS_SUCCESS;
}

_Use_decl_annotations_
VOID ReleaseRegistryFilterFilteredKeyEntry(
	PREGISTRY_FILTER_RULE_ENTRY RuleEntry
)
{
	if (ReleaseListEntry(&RuleEntry->ListEntry))
	{
		REGISTRY_FILTER_FREE(RuleEntry);
	}
}