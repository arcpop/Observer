#include "Includes.h"

#include "../Log/Log.h"

_Use_decl_annotations_
NTSTATUS RegistryFilterPreSetValueKey(
	PREGISTRY_FILTER_CONTEXT pContext, 
	PREG_SET_VALUE_KEY_INFORMATION Info
)
{
	if (Info->ObjectContext != NULL)
	{
		DEBUG_LOG("RegistryFilterPreSetValueKey: Denying access");
		return STATUS_ACCESS_DENIED;
	}
	DEBUG_LOG("RegistryFilterPreSetValueKey: Allowing access");
	UNREFERENCED_PARAMETER(pContext);
	return STATUS_SUCCESS;
}