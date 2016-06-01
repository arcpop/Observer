#include "Includes.h"

#include "../Log/Log.h"

_Use_decl_annotations_
NTSTATUS RegistryFilterInitialize(
	PDRIVER_OBJECT pDriverObject,
	PVOID* ppContext
)
{
	NTSTATUS Status;
	PREGISTRY_FILTER_CONTEXT pContext;
	UNICODE_STRING uAltitude;

	RtlInitUnicodeString(&uAltitude, L"260005");

	pContext = REGISTRY_FILTER_ALLOCATE(sizeof(REGISTRY_FILTER_CONTEXT), NonPagedPool);
	if (pContext == NULL)
	{
		DEBUG_LOG("RegistryFilter: ExAllocatePoolWithTag out of memory");
		return STATUS_NO_MEMORY;
	}

	InitializeListHead(&RegistryFilterRuleList);
	ExInitializeFastMutex(&RegistryFilterRuleListMutex);

	pContext->DriverObject = pDriverObject;

	pContext->FilterContextCookie.QuadPart = 0;

	Status = CmRegisterCallbackEx(
		&RegistryFilterCallback,
		&uAltitude,
		&pDriverObject,
		(PVOID)pContext,
		&pContext->FilterContextCookie,
		NULL
	);

	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("RegistryFilter: CmRegisterCallbackEx returned error 0x%.8X", Status);
		REGISTRY_FILTER_FREE(pContext);
		return Status;
	}

	*ppContext = pContext;
	return STATUS_SUCCESS;
}
