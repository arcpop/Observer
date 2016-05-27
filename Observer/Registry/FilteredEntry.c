#include "Includes.h"

#include "../Log/Log.h"
#include "../Util/Util.h"

_Use_decl_annotations_
BOOLEAN IsFilteredRegistryKey(
	PUNICODE_STRING KeyPath,
	PREGISTRY_FILTER_RULE_ENTRY* EntryOut
)
{
	PRESOURCE_LIST_ENTRY pEntry;

	for (
		pEntry = NextListEntry(
			&RegistryFilterRuleList,
			&RegistryFilterRuleList.Entry,
			FALSE);
		pEntry != &RegistryFilterRuleList.Entry;
		pEntry = NextListEntry(
			&RegistryFilterRuleList,
			pEntry,
			TRUE)
	)
	{
		BOOLEAN ShouldDoAction = FALSE;
		LONG Result = 0;
		PREGISTRY_FILTER_RULE_ENTRY CurrentEntry;
		CurrentEntry = CONTAINING_RECORD(pEntry, REGISTRY_FILTER_RULE_ENTRY, ListEntry);

		Result = -1;

		if (CurrentEntry->Rule.MatchFlags & REGISTRY_MATCH_EQUALS)
		{
			if (RtlCompareUnicodeString(
				&CurrentEntry->Path,
				KeyPath,
				CurrentEntry->Rule.MatchFlags & REGISTRY_MATCH_IGNORE_CASE
			) == 0)
				ShouldDoAction = TRUE;
		}
		else if (CurrentEntry->Rule.MatchFlags & REGISTRY_MATCH_CONTAINS)
		{
			ShouldDoAction = UtilUnicodeStringContains(
				KeyPath,
				&CurrentEntry->Path,
				CurrentEntry->Rule.MatchFlags & REGISTRY_MATCH_IGNORE_CASE
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
			if (ReleaseListEntry(pEntry))
			{
				REGISTRY_FILTER_FREE(CurrentEntry);
			}
			return TRUE;
		}
	}
	return FALSE;
}