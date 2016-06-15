#include "Includes.h"

#include "../Log/Log.h"
#include "../Util/Util.h"

_Use_decl_annotations_
PLIST_ENTRY NextRegistryFilterRuleListEntry(
	PLIST_ENTRY CurrentEntry,
	BOOLEAN ReleaseCurrent
)
{
	PLIST_ENTRY pNextEntry;
	ExAcquireFastMutex(&RegistryFilterRuleListMutex);
	if (ReleaseCurrent)
	{
		PREGISTRY_FILTER_RULE_ENTRY pCurrent = CONTAINING_RECORD(CurrentEntry, REGISTRY_FILTER_RULE_ENTRY, ListEntry);
		ExReleaseRundownProtection(&pCurrent->RundownProtection);
	}
TryAgain:
	pNextEntry = CurrentEntry->Flink;
	if (pNextEntry != &RegistryFilterRuleList)
	{
		PREGISTRY_FILTER_RULE_ENTRY pNext = CONTAINING_RECORD(pNextEntry, REGISTRY_FILTER_RULE_ENTRY, ListEntry);
		if (!ExAcquireRundownProtection(&pNext->RundownProtection))
		{
			CurrentEntry = pNextEntry;
			goto TryAgain;
		}
		ExReleaseFastMutex(&RegistryFilterRuleListMutex);
		return pNextEntry;
	}
	ExReleaseFastMutex(&RegistryFilterRuleListMutex);
	return NULL;
}



_Use_decl_annotations_
BOOLEAN IsFilteredRegistryKey(
	PUNICODE_STRING KeyPath,
	PREGISTRY_FILTER_RULE_ENTRY* EntryOut
)
{
	PLIST_ENTRY pEntry;
	for (
		pEntry = NextRegistryFilterRuleListEntry(&RegistryFilterRuleList, FALSE);
		pEntry != NULL;
		pEntry = NextRegistryFilterRuleListEntry(pEntry, TRUE)
	)
	{
		BOOLEAN ShouldDoAction = FALSE;
		LONG Result = 0;
		PREGISTRY_FILTER_RULE_ENTRY CurrentEntry;
		BOOLEAN CaseInsensitive = FALSE;
		CurrentEntry = CONTAINING_RECORD(pEntry, REGISTRY_FILTER_RULE_ENTRY, ListEntry);
		if (CurrentEntry->Rule.MatchFlags & REGISTRY_MATCH_IGNORE_CASE)
		{
			CaseInsensitive = TRUE;
		}

		Result = -1;

		if (CurrentEntry->Rule.MatchFlags & REGISTRY_MATCH_EQUALS)
		{
			if (RtlCompareUnicodeString(
				&CurrentEntry->Path,
				KeyPath,
				CaseInsensitive
			) == 0) 
				ShouldDoAction = TRUE;
		}
		else if (CurrentEntry->Rule.MatchFlags & REGISTRY_MATCH_POSTFIX)
		{
			ShouldDoAction = RtlSuffixUnicodeString(
				&CurrentEntry->Path,
				KeyPath,
				CaseInsensitive
			);
		}
		else if (CurrentEntry->Rule.MatchFlags & REGISTRY_MATCH_CONTAINS)
		{
			ShouldDoAction = UtilUnicodeStringContains(
				KeyPath,
				&CurrentEntry->Path,
				CaseInsensitive
			);
		}

		if (ShouldDoAction)
		{
			DEBUG_LOG("IsFilteredRegistryKey: Action: %d for %wZ", CurrentEntry->Rule.Action, KeyPath);
			if (EntryOut != NULL)
			{
				*EntryOut = CurrentEntry;
				return TRUE;
			}
			ExReleaseRundownProtection(&CurrentEntry->RundownProtection);
			return TRUE;
		}
	}
	UNREFERENCED_PARAMETER(KeyPath);
	UNREFERENCED_PARAMETER(EntryOut);
	return FALSE;
}