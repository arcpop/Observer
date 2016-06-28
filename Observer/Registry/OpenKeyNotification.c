#include "Includes.h"

#include "../Log/Log.h"
#include "../Notification/NotificationQueue.h"

VOID HandleOpenKey(
	PLARGE_INTEGER Cookie,
	PUNICODE_STRING KeyName,
	PVOID Object,
	PBOOLEAN ShouldBlock
)
{
	BOOLEAN Block = FALSE;
	REGISTRY_FILTER_OBJECT_CONTEXT ObjCtx;
	PLIST_ENTRY pEntry;
	ObjCtx.NumberOfRules = 0;

	RLockResourceList(&RegistryFilterRuleList);
	for (
		pEntry = RegistryFilterRuleList.ListEntry.Flink;
		pEntry != &RegistryFilterRuleList.ListEntry;
		pEntry = pEntry->Flink
		)
	{
		PREGISTRY_FILTER_RULE_ENTRY CurrentEntry = CONTAINING_RECORD(pEntry, REGISTRY_FILTER_RULE_ENTRY, ListEntry);

		if (CurrentEntry->Rule.Type == REGISTRY_TYPE_OPEN_KEY)
		{
			if (!RegistryMatchStrings(&CurrentEntry->Path, KeyName, CurrentEntry->Rule.KeyMatch))
			{
				continue;
			}
			if (CurrentEntry->Rule.Action & ACTION_BLOCK)
			{
				Block = TRUE;
			}
			if (CurrentEntry->Rule.Action & ACTION_DBGPRINT)
			{
				DbgPrint("Open Key Rule Match -> %wZ\n", KeyName);
			}
			if (CurrentEntry->Rule.Action & ACTION_REPORT)
			{
				PNOTIFICATION_ENTRY pNotification = NotificationCreate(RULE_TYPE_REGISTRY);
				if (pNotification != NULL)
				{
					USHORT CopyLength = (NOTIFICATION_STRING_BUFFER_SIZE - 1) * sizeof(WCHAR);
					pNotification->Data.Types.Registry.RegistryAction = NOTIFICATION_REGISTRY_OPEN_KEY;
					if (KeyName->Length > CopyLength)
					{
						pNotification->Data.Types.Registry.Truncated =
							(KeyName->Length - CopyLength) >> 1;
					}
					else
					{
						CopyLength = KeyName->Length;
					}
					RtlCopyMemory(
						pNotification->Data.Types.Registry.RegistryPath,
						KeyName->Buffer,
						CopyLength
					);
					pNotification->Data.Types.Registry.RegistryPath[CopyLength] = L'\0';
					pNotification->Data.Reaction = CurrentEntry->Rule.Action;
					NotificationSend(pNotification);
				}
			}
		}
		else if ((CurrentEntry->Rule.Type == REGISTRY_TYPE_ENUMERATE_SUBKEYS) ||
			(CurrentEntry->Rule.Type == REGISTRY_TYPE_QUERY_VALUE) ||
			(CurrentEntry->Rule.Type == REGISTRY_TYPE_SET_VALUE))
		{
			if (!RegistryMatchStrings(&CurrentEntry->Path, KeyName, CurrentEntry->Rule.KeyMatch))
			{
				continue;
			}
			if (ObjCtx.NumberOfRules < MAX_OBJECT_CONTEXT_RULES)
			{
				InterlockedIncrement(&CurrentEntry->Refcount);
				ObjCtx.RuleEntries[ObjCtx.NumberOfRules] = CurrentEntry;
				ObjCtx.NumberOfRules++;
			}
			else
			{
				DEBUG_LOG("HandleOpenKey: Too many followup rules");
			}
		}
	}
	RUnlockResourceList(&RegistryFilterRuleList);
	if (Block)
	{
		*ShouldBlock = TRUE;
		return;
	}
	*ShouldBlock = FALSE;
	if (ObjCtx.NumberOfRules > 0)
	{
		PVOID OldCtx = NULL;
		NTSTATUS Status;
		PREGISTRY_FILTER_OBJECT_CONTEXT pCtx = REGISTRY_FILTER_ALLOCATE(sizeof(REGISTRY_FILTER_OBJECT_CONTEXT), NonPagedPool);
		if (pCtx == NULL)
		{
			DEBUG_LOG("HandleOpenKey: Out of memory");
			*ShouldBlock = TRUE;
			return;
		}
		RtlCopyMemory(pCtx, &ObjCtx, sizeof(REGISTRY_FILTER_OBJECT_CONTEXT));
		Status = CmSetCallbackObjectContext(Object, Cookie, pCtx, &OldCtx);
		if (!NT_SUCCESS(Status))
		{
			DEBUG_LOG("HandleOpenKey: CmSetCallbackObjectContext returned error 0x%.8X", Status);
			*ShouldBlock = TRUE;
			REGISTRY_FILTER_FREE(pCtx);
		}
		if (OldCtx != NULL)
		{
			CleanupObjectContext(OldCtx);
		}
	}
}



_Use_decl_annotations_
NTSTATUS RegistryFilterPostOpenKey(
	PREGISTRY_FILTER_CONTEXT pContext,
	PREG_POST_OPEN_KEY_INFORMATION Info
)
{
	BOOLEAN ShouldBlock = FALSE;
	
	if (Info->Status != STATUS_SUCCESS)
	{
		return STATUS_SUCCESS;
	}
	if (Info->CompleteName == NULL)
	{
		DEBUG_LOG("RegistryFilterPostOpenKey: CompleteName is NULL");
		return STATUS_INVALID_PARAMETER;
	}

	HandleOpenKey(
		&pContext->FilterContextCookie,
		Info->CompleteName,
		Info->Object,
		&ShouldBlock
	);

	if (ShouldBlock)
	{
		return STATUS_ACCESS_DENIED;
	}
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS RegistryFilterPostOpenKeyEx(
	PREGISTRY_FILTER_CONTEXT pContext,
	PREG_POST_OPERATION_INFORMATION Info
)
{
	PCUNICODE_STRING cuRootName;
	UNICODE_STRING FullKeyName;
	PUNICODE_STRING ReportName;
	PREG_OPEN_KEY_INFORMATION PreInfo;
	NTSTATUS Status;
	ULONG TotalUnicodeLength;
	USHORT Count;
	BOOLEAN ShouldBlock = FALSE;

	PreInfo = (PREG_OPEN_KEY_INFORMATION)Info->PreInformation;

	if (PreInfo == NULL)
	{
		DEBUG_LOG("RegistryFilterPostOpenKeyEx: PreInformation is NULL");
		return STATUS_INVALID_PARAMETER;
	}

	if (PreInfo->CompleteName == NULL)
	{
		DEBUG_LOG("RegistryFilterPostOpenKeyEx: PreInformation->CompleteName is NULL");
		return STATUS_INVALID_PARAMETER;
	}

	if (Info->Status != STATUS_SUCCESS)
	{
		//DEBUG_LOG("RegistryFilterPostOpenKeyEx: Status 0x%.8X, Object is invalid, returning", Info->Status);
		return STATUS_SUCCESS;
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
			DEBUG_LOG("RegistryFilterPostOpenKeyEx: CmCallbackGetKeyObjectID failed with error 0x%.8X", Status);
			return Status;
		}

		TotalUnicodeLength = cuRootName->Length;
		TotalUnicodeLength += sizeof(wchar_t);
		TotalUnicodeLength += PreInfo->CompleteName->Length;

		if (TotalUnicodeLength >= 0xFFFF)
		{
			DEBUG_LOG("RegistryFilterPostOpenKeyEx: TotalUnicodeLength >= 0xFFFF");
			return STATUS_NO_MEMORY;
		}

		FullKeyName.Buffer = REGISTRY_FILTER_ALLOCATE(TotalUnicodeLength, NonPagedPool);

		if (FullKeyName.Buffer == NULL)
		{
			DEBUG_LOG("RegistryFilterPostOpenKeyEx: Out of memory");
			return STATUS_NO_MEMORY;
		}

		FullKeyName.Length = (USHORT)TotalUnicodeLength;
		FullKeyName.MaximumLength = (USHORT)TotalUnicodeLength;
		Count = cuRootName->Length / 2;
		RtlCopyMemory(FullKeyName.Buffer, cuRootName->Buffer, cuRootName->Length);
		FullKeyName.Buffer[Count] = '\\';
		RtlCopyMemory(FullKeyName.Buffer + Count + 1, PreInfo->CompleteName->Buffer, PreInfo->CompleteName->Length);
		
		
		ReportName = &FullKeyName;
	} 
	else
	{
		ReportName = PreInfo->CompleteName;
	}



	HandleOpenKey(
		&pContext->FilterContextCookie,
		ReportName,
		Info->Object,
		&ShouldBlock
	);

	if (ReportName == &FullKeyName)
	{
		REGISTRY_FILTER_FREE(FullKeyName.Buffer);
		FullKeyName.Buffer = NULL;
	}

	if (ShouldBlock)
	{
		return STATUS_ACCESS_DENIED;
	}
	return STATUS_SUCCESS;
}

