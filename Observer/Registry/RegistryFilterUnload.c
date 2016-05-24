#include "Includes.h"

#include "../Log/Log.h"

_Use_decl_annotations_
NTSTATUS RegistryFilterUnload(
	PVOID pContext
)
{
	NTSTATUS Status;
	PREGISTRY_FILTER_CONTEXT Context = (PREGISTRY_FILTER_CONTEXT)pContext;

	if (Context == NULL)
	{
		//Nothing to do...
		return STATUS_SUCCESS;
	}

	Status = CmUnRegisterCallback(Context->FilterContextCookie);

	REGISTRY_FILTER_FREE(Context);

	return Status;
}