#include "Includes.h"

#include "../Log/Log.h"

_Use_decl_annotations_
NTSTATUS RegistryFilterPostOpenKey(
	PREGISTRY_FILTER_CONTEXT pContext,
	PREG_POST_OPEN_KEY_INFORMATION Info
)
{
	NTSTATUS Status;
	PREGISTRY_FILTER_RULE_ENTRY RuleEntry;

	if (Info->CompleteName == NULL)
	{
		DEBUG_LOG("RegistryFilterPostOpenKey: CompleteName is NULL");
		return STATUS_INVALID_PARAMETER;
	}

	if (!IsFilteredRegistryKey(Info->CompleteName, &RuleEntry))
	{
		return STATUS_SUCCESS;
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
NTSTATUS RegistryFilterPostOpenKeyEx(
	PREGISTRY_FILTER_CONTEXT pContext,
	PREG_POST_OPERATION_INFORMATION Info
)
{
	PCUNICODE_STRING cuRootName;
	UNICODE_STRING FullKeyName;
	PREG_OPEN_KEY_INFORMATION PreInfo;
	PREGISTRY_FILTER_RULE_ENTRY RuleEntry;
	NTSTATUS Status;
	ULONG TotalUnicodeLength;
	USHORT Count;

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

		TotalUnicodeLength = cuRootName->Length + 2 + PreInfo->CompleteName->Length;

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
		Count = cuRootName->Length >> 1;
		RtlCopyMemory(FullKeyName.Buffer, cuRootName->Buffer, cuRootName->Length);
		FullKeyName.Buffer[Count] = '\\';
		RtlCopyMemory(FullKeyName.Buffer + Count + 1, PreInfo->CompleteName->Buffer, PreInfo->CompleteName->Length);

		if (!IsFilteredRegistryKey(&FullKeyName, &RuleEntry))
		{
			REGISTRY_FILTER_FREE(FullKeyName.Buffer);
			return STATUS_SUCCESS;
		}
		REGISTRY_FILTER_FREE(FullKeyName.Buffer);
	} 
	else
	{
		if (!IsFilteredRegistryKey(PreInfo->CompleteName, &RuleEntry))
		{
			return STATUS_SUCCESS;
		}
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

