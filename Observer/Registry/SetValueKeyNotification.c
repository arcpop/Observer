#include "Includes.h"
#include "../Log/Log.h"
#include "../Notification/NotificationQueue.h"

_Use_decl_annotations_
NTSTATUS RegistryFilterApplyObjectContext(
	PREGISTRY_FILTER_CONTEXT Context,
	PVOID Object,
	PREGISTRY_FILTER_RULE_ENTRY RuleEntry
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

		if ((pEntry->RuleEntry->Rule.Action == ACTION_REPORT) ||
			(pEntry->RuleEntry->Rule.Action == ACTION_REPORT))
		{
			PNOTIFICATION_ENTRY pNotification = NotificationCreate(RULE_TYPE_REGISTRY);
			if (pNotification == NULL)
			{
				return STATUS_SUCCESS;
			}
			pNotification->Data.Types.Registry.RegistryAction = NOTIFICATION_REGISTRY_ACTION_SET_VALUE;
			if (Info->ValueName != NULL)
			{
				USHORT CopyLength = (NOTIFICATION_STRING_BUFFER_SIZE - 1) * sizeof(WCHAR);
				if (Info->ValueName->Length > CopyLength)
				{
					pNotification->Data.Types.Registry.Truncated =
						(Info->ValueName->Length - CopyLength) >> 1;
				}
				else
				{
					CopyLength = Info->ValueName->Length;
				}
				RtlCopyMemory(
					pNotification->Data.Types.Registry.RegistryPath,
					Info->ValueName->Buffer,
					CopyLength
				);
				pNotification->Data.Types.Registry.RegistryPath[CopyLength] = L'\0';
			}
			else
			{
				pNotification->Data.Types.Registry.RegistryPath[0] = L'\0';
			} 
			pNotification->Data.Reaction = pEntry->RuleEntry->Rule.Action;
			NotificationSend(pNotification);

			if (pEntry->RuleEntry->Rule.Action == ACTION_BLOCK)
			{
				return STATUS_ACCESS_DENIED;
			}
		}


		if (pEntry->RuleEntry->Rule.Action == ACTION_DBGPRINT)
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