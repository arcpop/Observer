#include "Includes.h"
#include "../Log/Log.h"
#include "../Notification/NotificationQueue.h"


_Use_decl_annotations_
NTSTATUS RegistryFilterPreSetValueKey(
	PREGISTRY_FILTER_CONTEXT pContext, 
	PREG_SET_VALUE_KEY_INFORMATION Info
)
{
	const UNICODE_STRING DefaultKeyName = RTL_CONSTANT_STRING(L"(Default)");
	BOOLEAN Block = FALSE;
	PCUNICODE_STRING RegString;
	if (Info->ObjectContext != NULL)
	{
		PREGISTRY_FILTER_OBJECT_CONTEXT pObjCtx = (PREGISTRY_FILTER_OBJECT_CONTEXT)(Info->ObjectContext);

		for (ULONG i = 0; i < pObjCtx->NumberOfRules; i++)
		{
			PREGISTRY_FILTER_RULE_ENTRY pEntry = pObjCtx->RuleEntries[i];
			if (pEntry->Rule.Type != REGISTRY_TYPE_SET_VALUE)
			{
				continue;
			}
			//If valuename is NULL check if we match for the default value of this key
			if (Info->ValueName == NULL)
			{
				if ((pEntry->Rule.ValueMatch & 0xFFFF) != REGISTRY_MATCH_DEFAULT_VALUE_KEY)
				{
					continue;
				}
				RegString = &DefaultKeyName;
			}
			else
			{
				UNICODE_STRING ValueName;
				RtlInitUnicodeString(&ValueName, pEntry->Rule.ValueName);
				if (!RegistryMatchStrings(&ValueName, Info->ValueName, pEntry->Rule.ValueMatch))
				{
					continue;
				}
				RegString = Info->ValueName;
			}
			if (pEntry->Rule.Action & ACTION_DBGPRINT)
			{
				DbgPrint("SetValueKey Matched %wZ", RegString);
			}
			if (pEntry->Rule.Action & ACTION_BLOCK)
			{
				Block = TRUE;
			}
			if (pEntry->Rule.Action & ACTION_REPORT)
			{
				PNOTIFICATION_ENTRY pNotification = NotificationCreate(RULE_TYPE_REGISTRY);
				if (pNotification != NULL)
				{
					USHORT CopyLength = (NOTIFICATION_STRING_BUFFER_SIZE - 1) * sizeof(WCHAR);
					pNotification->Data.Types.Registry.RegistryAction = NOTIFICATION_REGISTRY_SET_VALUE;
					if (RegString->Length > CopyLength)
					{
						pNotification->Data.Types.Registry.Truncated =
							(RegString->Length - CopyLength) >> 1;
					}
					else
					{
						CopyLength = RegString->Length;
					}
					RtlCopyMemory(
						pNotification->Data.Types.Registry.RegistryPath,
						RegString->Buffer,
						CopyLength
					);
					pNotification->Data.Types.Registry.RegistryPath[CopyLength] = L'\0';
					pNotification->Data.Reaction = pEntry->Rule.Action;
					NotificationSend(pNotification);
				}
			}
		}
	}
	if (Block)
	{
		return STATUS_ACCESS_DENIED;
	}
	return STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(pContext);
}