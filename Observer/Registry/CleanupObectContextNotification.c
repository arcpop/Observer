#include "Includes.h"

#include "../Log/Log.h"

_Use_decl_annotations_
NTSTATUS RegistryFilterCleanupObjectContext(
	PREGISTRY_FILTER_CONTEXT pContext, 
	PREG_CALLBACK_CONTEXT_CLEANUP_INFORMATION Info
)
{
	PREGISTRY_FILTER_OBJECT_CONTEXT pObjectContext = (PREGISTRY_FILTER_OBJECT_CONTEXT)Info->ObjectContext;

	if (pObjectContext == NULL)
	{
		DEBUG_LOG("RegistryFilterCleanupObjectContext: ObjectContext is NULL");
		return STATUS_INVALID_PARAMETER;
	}
	
	CleanupObjectContext(pObjectContext);

	UNREFERENCED_PARAMETER(pContext);

	return STATUS_SUCCESS;
}

_Use_decl_annotations_
VOID CleanupObjectContext(
	PVOID ObjectContext
)
{
	PREGISTRY_FILTER_OBJECT_CONTEXT pObjectContext = (PREGISTRY_FILTER_OBJECT_CONTEXT)ObjectContext;
	if (ObjectContext == NULL)
	{
		return;
	}

	for (ULONG i = 0; i < pObjectContext->NumberOfRules; i++)
	{
		ExReleaseRundownProtection(&pObjectContext->RuleEntries[i]->RundownProtection);
	}

	REGISTRY_FILTER_FREE(pObjectContext);
}