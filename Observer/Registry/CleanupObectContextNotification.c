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
	DEBUG_LOG("%.8X", pObjectContext->RuleEntry->RundownProtection.Count);

	ExReleaseRundownProtection(&pObjectContext->RuleEntry->RundownProtection);

	REGISTRY_FILTER_FREE(pObjectContext);

	UNREFERENCED_PARAMETER(pContext);

	return STATUS_SUCCESS;
}
