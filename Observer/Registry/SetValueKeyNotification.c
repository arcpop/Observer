#include "Includes.h"
#include "ActionFlags.h"
#include "../Log/Log.h"


_Use_decl_annotations_
NTSTATUS RegistryFilterApplyObjectContext(
	PREGISTRY_FILTER_CONTEXT Context,
	PVOID Object,
	PREGISTRY_FILTER_FILTERED_KEY_ENTRY RuleEntry
)
{
	PREGISTRY_FILTER_OBJECT_CONTEXT pObjectContext;
	PVOID pOldContext;
	NTSTATUS Status;

	pObjectContext = REGISTRY_FILTER_ALLOCATE(
		sizeof(REGISTRY_FILTER_OBJECT_CONTEXT),
		PagedPool
	);

	if (pObjectContext == NULL)
	{
		DEBUG_LOG("RegistryFilterApplyObjectContext: Out of memory");
		return STATUS_NO_MEMORY;
	}

	pObjectContext->RuleEntry = RuleEntry;

	pOldContext = NULL;

	Status = CmSetCallbackObjectContext(
		Object,
		&Context->FilterContextCookie,
		(PVOID)pObjectContext,
		&pOldContext
	);

	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("RegistryFilterApplyObjectContext: CmSetCallbackObjectContext failed with error 0x%.8X", Status);
		REGISTRY_FILTER_FREE(pObjectContext);
		return Status;
	}
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS RegistryFilterPreSetValueKey(
	PREGISTRY_FILTER_CONTEXT pContext, 
	PREG_SET_VALUE_KEY_INFORMATION Info
)
{
	if (Info->ObjectContext != NULL)
	{
		PREGISTRY_FILTER_OBJECT_CONTEXT pEntry;
		pEntry = (PREGISTRY_FILTER_OBJECT_CONTEXT)(Info->ObjectContext);

		if (pEntry->RuleEntry->ActionFlags == ACTIONFLAG_BLOCK)
		{
			DEBUG_LOG("RegistryFilterPreSetValueKey: Value set blocked");
			return STATUS_ACCESS_DENIED;
		}

		if (pEntry->RuleEntry->ActionFlags == ACTIONFLAG_NOTIFY)
		{
			DEBUG_LOG("RegistryFilterPreSetValueKey: Value set notification");
			return STATUS_SUCCESS;
		}
		DEBUG_LOG("RegistryFilterPreSetValueKey: Unknown action");
		return STATUS_SUCCESS;
	}
	UNREFERENCED_PARAMETER(pContext);
	return STATUS_SUCCESS;
}