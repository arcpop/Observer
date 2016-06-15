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

BOOLEAN RegistryMatchStrings(
	PCUNICODE_STRING RuleString,
	PCUNICODE_STRING RegString,
	ULONG MatchType
)
{
	BOOLEAN IgnoreCase = (MatchType >> 16) & 1;
	switch (MatchType & 0xFFFF)
	{
	case REGISTRY_MATCH_EQUALS:
		return RtlEqualUnicodeString(
			RuleString,
			RegString,
			IgnoreCase
		);
	case REGISTRY_MATCH_SUFFIX:
		return RtlSuffixUnicodeString(
			RuleString,
			RegString,
			IgnoreCase
		);
	case REGISTRY_MATCH_PREFIX:
		return RtlPrefixUnicodeString(
			RuleString,
			RegString,
			IgnoreCase
		);
	case REGISTRY_MATCH_CONTAINS:
		return UtilUnicodeStringContains(
			RegString,
			RuleString,
			IgnoreCase
		);
	}
	return FALSE;
}

