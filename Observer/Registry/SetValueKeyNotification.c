#include "Includes.h"
#include "ActionFlags.h"
#include "../Log/Log.h"


_Use_decl_annotations_
NTSTATUS RegistryFilterPreSetValueKey(
	PREGISTRY_FILTER_CONTEXT pContext, 
	PREG_SET_VALUE_KEY_INFORMATION Info
)
{
	if (Info->ObjectContext != NULL)
	{
		PREGISTRY_FILTER_FILTERED_KEY_ENTRY pEntry;
		pEntry = (PREGISTRY_FILTER_FILTERED_KEY_ENTRY)Info->ObjectContext;

		if (pEntry->ActionFlags == ACTIONFLAG_BLOCK)
		{
			DEBUG_LOG("RegistryFilterPreSetValueKey: Value set blocked");
			return STATUS_ACCESS_DENIED;
		}

		if (pEntry->ActionFlags == ACTIONFLAG_BLOCK)
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