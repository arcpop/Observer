#include "Includes.h"

#include "../Log/Log.h"
#include "../Util/Util.h"

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

