#include "Includes.h"

#include "../Log/Log.h"
#include "../Notification/NotificationQueue.h"

_Use_decl_annotations_
NTSTATUS RegistryFilterPostCreateKey(
	PREGISTRY_FILTER_CONTEXT pContext, 
	PREG_POST_CREATE_KEY_INFORMATION Info
)
{
	NTSTATUS Status;
	PREGISTRY_FILTER_RULE_ENTRY RuleEntry = NULL;

	if (Info->CompleteName == NULL)
	{
		DEBUG_LOG("RegistryFilterPostCreateKey: CompleteName is NULL");
		return STATUS_INVALID_PARAMETER;
	}
	
	if (!IsFilteredRegistryKey(Info->CompleteName, &RuleEntry))
	{
		return STATUS_SUCCESS;
	}

	if ((RuleEntry->Rule.Action == ACTION_REPORT) ||
		(RuleEntry->Rule.Action == ACTION_BLOCK))
	{
		PNOTIFICATION_ENTRY pNotification = NotificationCreate(RULE_TYPE_REGISTRY);
		if (pNotification != NULL)
		{
			pNotification->Data.Types.Registry.RegistryAction = NOTIFICATION_REGISTRY_ACTION_SET_VALUE;
			if (Info->CompleteName != NULL)
			{
				UINT16 CopyLength = (NOTIFICATION_STRING_BUFFER_SIZE - 1) * sizeof(WCHAR);
				if (Info->CompleteName->Length > CopyLength)
				{
					pNotification->Data.Types.Registry.Truncated =
						(Info->CompleteName->Length - CopyLength) >> 1;
				}
				else
				{
					CopyLength = Info->CompleteName->Length;
				}
				RtlCopyMemory(
					pNotification->Data.Types.Registry.RegistryPath,
					Info->CompleteName->Buffer,
					CopyLength
				);
				pNotification->Data.Types.Registry.RegistryPath[CopyLength] = L'\0';
			}
			else
			{
				pNotification->Data.Types.Registry.RegistryPath[0] = L'\0';
			}
			pNotification->Data.Reaction = RuleEntry->Rule.Action;
			NotificationSend(pNotification);
		}

	}
	if (RuleEntry->Rule.Action == ACTION_BLOCK)
	{
		return STATUS_ACCESS_DENIED;
	}

	if (RuleEntry->Rule.Action == ACTION_DBGPRINT)
	{
		DEBUG_LOG("KeyCreate notification");
	}
	else
	{
		DEBUG_LOG("RegistryFilterPostCreateKey: Unknown action");
	}


	Status = RegistryFilterApplyObjectContext(
		pContext,
		Info->Object,
		RuleEntry
	);

	if (!NT_SUCCESS(Status))
	{
		ExReleaseRundownProtection(&RuleEntry->RundownProtection);
		return Status;
	}
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS RegistryFilterPostCreateKeyEx(
	PREGISTRY_FILTER_CONTEXT pContext, 
	PREG_POST_OPERATION_INFORMATION Info
)
{
	PUNICODE_STRING ReportName;
	PCUNICODE_STRING cuRootName;
	UNICODE_STRING FullKeyName;
	PREG_CREATE_KEY_INFORMATION PreInfo;
	PREGISTRY_FILTER_RULE_ENTRY RuleEntry;
	NTSTATUS Status;
	ULONG TotalUnicodeLength;
	USHORT Count;

	PreInfo = (PREG_CREATE_KEY_INFORMATION)Info->PreInformation;

	if (PreInfo == NULL)
	{
		DEBUG_LOG("RegistryFilterPostCreateKeyEx: PreInformation is NULL");
		return STATUS_INVALID_PARAMETER;
	}

	if (PreInfo->CompleteName == NULL)
	{
		DEBUG_LOG("RegistryFilterPostCreateKeyEx: PreInformation->CompleteName is NULL");
		return STATUS_INVALID_PARAMETER;
	}

	//Check if Complete name is a relative path
	if ((PreInfo->CompleteName->Length == 0) ||
		(PreInfo->CompleteName->Buffer[0] != '\\')) {

		Status = CmCallbackGetKeyObjectID(
			&pContext->FilterContextCookie,
			PreInfo->RootObject,
			NULL,
			&cuRootName
		);

		if (!NT_SUCCESS(Status))
		{
			DEBUG_LOG("RegistryFilterPostCreateKeyEx: CmCallbackGetKeyObjectID failed with error 0x%.8X", Status);
			return Status;
		}

		TotalUnicodeLength = cuRootName->Length + 2 + PreInfo->CompleteName->Length;
	
		if (TotalUnicodeLength >= 0xFFFF)
		{
			DEBUG_LOG("RegistryFilterPostCreateKeyEx: TotalUnicodeLength >= 0xFFFF");
			return STATUS_NO_MEMORY;
		}

		FullKeyName.Buffer = REGISTRY_FILTER_ALLOCATE(TotalUnicodeLength, NonPagedPool);

		if (FullKeyName.Buffer == NULL)
		{
			DEBUG_LOG("RegistryFilterPostCreateKeyEx: Out of memory");
			return STATUS_NO_MEMORY;
		}

		FullKeyName.Length = (USHORT)TotalUnicodeLength;
		FullKeyName.MaximumLength = (USHORT)TotalUnicodeLength;
		Count = cuRootName->Length >> 1;
		RtlCopyMemory(FullKeyName.Buffer, cuRootName->Buffer, cuRootName->Length);
		FullKeyName.Buffer[Count] = '\\';
		RtlCopyMemory(FullKeyName.Buffer + Count + 1, PreInfo->CompleteName->Buffer, PreInfo->CompleteName->Length);


		if (!IsFilteredRegistryKey(&FullKeyName, &RuleEntry))
		{
			REGISTRY_FILTER_FREE(FullKeyName.Buffer);
			return STATUS_SUCCESS;
		}
		ReportName = &FullKeyName;
	}
	else
	{
		if (!IsFilteredRegistryKey(PreInfo->CompleteName, &RuleEntry))
		{
			return STATUS_SUCCESS;
		}
		ReportName = PreInfo->CompleteName;
	}


	if ((RuleEntry->Rule.Action == ACTION_REPORT) ||
		(RuleEntry->Rule.Action == ACTION_BLOCK))
	{
		PNOTIFICATION_ENTRY pNotification = NotificationCreate(RULE_TYPE_REGISTRY);
		if (pNotification != NULL)
		{
			pNotification->Data.Types.Registry.RegistryAction = NOTIFICATION_REGISTRY_ACTION_SET_VALUE;
			if (ReportName != NULL)
			{
				UINT16 CopyLength = (NOTIFICATION_STRING_BUFFER_SIZE - 1) * sizeof(WCHAR);
				if (ReportName->Length > CopyLength)
				{
					pNotification->Data.Types.Registry.Truncated =
						(ReportName->Length - CopyLength) >> 1;
				}
				else
				{
					CopyLength = ReportName->Length;
				}
				RtlCopyMemory(
					pNotification->Data.Types.Registry.RegistryPath,
					ReportName->Buffer,
					CopyLength
				);
				pNotification->Data.Types.Registry.RegistryPath[CopyLength] = L'\0';
			}
			else
			{
				pNotification->Data.Types.Registry.RegistryPath[0] = L'\0';
			}
			pNotification->Data.Reaction = RuleEntry->Rule.Action;
			NotificationSend(pNotification);
		}

	}

	if (ReportName == &FullKeyName)
	{
		REGISTRY_FILTER_FREE(FullKeyName.Buffer);
	}

	if (RuleEntry->Rule.Action == ACTION_BLOCK)
	{
		return STATUS_ACCESS_DENIED;
	}

	if (RuleEntry->Rule.Action == ACTION_DBGPRINT)
	{
		DEBUG_LOG("KeyCreateEx Notification");
	}
	else
	{
		DEBUG_LOG("RegistryFilterPostCreateKeyEx: Unknown action");
	}

	Status = RegistryFilterApplyObjectContext(
		pContext,
		Info->Object,
		RuleEntry
	);

	if (!NT_SUCCESS(Status))
	{
		ExReleaseRundownProtection(&RuleEntry->RundownProtection);
		return Status;
	}
	return STATUS_SUCCESS;
}
