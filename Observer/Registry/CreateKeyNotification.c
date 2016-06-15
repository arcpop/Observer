#include "Includes.h"

#include "../Log/Log.h"
#include "../Notification/NotificationQueue.h"



VOID HandleCreateKey(
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
	for (
		pEntry = NextRegistryFilterRuleListEntry(&RegistryFilterRuleList, FALSE);
		pEntry != NULL;
		pEntry = NextRegistryFilterRuleListEntry(pEntry, TRUE)
		)
	{
		PREGISTRY_FILTER_RULE_ENTRY CurrentEntry = CONTAINING_RECORD(pEntry, REGISTRY_FILTER_RULE_ENTRY, ListEntry);

		if (CurrentEntry->Rule.Type == REGISTRY_TYPE_CREATE_KEY)
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
				DbgPrint("Create Key Rule Match -> %wZ\n", KeyName);
			}
			if (CurrentEntry->Rule.Action & ACTION_REPORT)
			{
				PNOTIFICATION_ENTRY pNotification = NotificationCreate(RULE_TYPE_REGISTRY);
				if (pNotification != NULL)
				{
					USHORT CopyLength = (NOTIFICATION_STRING_BUFFER_SIZE - 1) * sizeof(WCHAR);
					pNotification->Data.Types.Registry.RegistryAction = NOTIFICATION_REGISTRY_CREATE_KEY;
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
				ExAcquireRundownProtection(&CurrentEntry->RundownProtection);
				ObjCtx.RuleEntries[ObjCtx.NumberOfRules] = CurrentEntry;
				ObjCtx.NumberOfRules++;
			}
			else
			{
				DEBUG_LOG("HandleCreateKey: Too many followup rules");
			}
		}
	}
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
			DEBUG_LOG("HandleCreateKey: Out of memory");
			*ShouldBlock = TRUE;
			return;
		}
		RtlCopyMemory(pCtx, &ObjCtx, sizeof(REGISTRY_FILTER_OBJECT_CONTEXT));
		Status = CmSetCallbackObjectContext(Object, Cookie, pCtx, &OldCtx);
		if (!NT_SUCCESS(Status))
		{
			DEBUG_LOG("HandleCreateKey: CmSetCallbackObjectContext returned error 0x%.8X", Status);
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
NTSTATUS RegistryFilterPostCreateKey(
	PREGISTRY_FILTER_CONTEXT pContext, 
	PREG_POST_CREATE_KEY_INFORMATION Info
)
{
	BOOLEAN ShouldBlock = FALSE;

	if (Info->CompleteName == NULL)
	{
		DEBUG_LOG("RegistryFilterPostCreateKey: CompleteName is NULL");
		return STATUS_INVALID_PARAMETER;
	}
	
	HandleCreateKey(
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
NTSTATUS RegistryFilterPostCreateKeyEx(
	PREGISTRY_FILTER_CONTEXT pContext, 
	PREG_POST_OPERATION_INFORMATION Info
)
{
	PUNICODE_STRING ReportName;
	PCUNICODE_STRING cuRootName;
	UNICODE_STRING FullKeyName;
	PREG_CREATE_KEY_INFORMATION PreInfo;
	NTSTATUS Status;
	ULONG TotalUnicodeLength;
	USHORT Count;
	BOOLEAN ShouldBlock = FALSE;

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

		TotalUnicodeLength = cuRootName->Length;
		TotalUnicodeLength += sizeof(wchar_t);
		TotalUnicodeLength += PreInfo->CompleteName->Length;
	
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


	HandleCreateKey(
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
