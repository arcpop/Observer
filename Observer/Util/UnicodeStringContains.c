#include "../Log/Log.h"

_Use_decl_annotations_
BOOLEAN UtilUnicodeStringContains(
	PCUNICODE_STRING	String,
	PCUNICODE_STRING	SubString,
	BOOLEAN			IgnoreCase
)
{
	UNICODE_STRING Temp;
	USHORT StringCount = String->Length >> 1;
	USHORT SubStringCount = SubString->Length >> 1;
	WCHAR SubstringFirstChar;
	if (SubStringCount == 0)
	{
		return TRUE;
	}

	SubstringFirstChar = SubString->Buffer[0];
	if (IgnoreCase)
	{
		SubstringFirstChar = towlower(SubstringFirstChar);
	}

	for (USHORT Index = 0; SubStringCount <= StringCount; Index++, StringCount--)
	{
		WCHAR FirstChar = String->Buffer[Index];
		if (IgnoreCase)
		{
			FirstChar = towlower(FirstChar);
		}
		if (FirstChar != SubstringFirstChar)
		{
			continue;
		}

		Temp.Buffer = &String->Buffer[Index];
		Temp.Length = Temp.MaximumLength = StringCount;
		if (RtlPrefixUnicodeString(
			SubString,
			&Temp,
			IgnoreCase
		) == TRUE)
			return TRUE;
	}
	return FALSE;
}
