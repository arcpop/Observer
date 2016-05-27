#include "../Log/Log.h"

_Use_decl_annotations_
BOOLEAN UtilUnicodeStringContains(
	PUNICODE_STRING	String,
	PUNICODE_STRING	SubString,
	BOOLEAN			IgnoreCase
)
{
	UNICODE_STRING Temp;
	USHORT StringCount = String->Length >> 1;
	USHORT SubStringCount = SubString->Length >> 1;

	for (USHORT Index = 0; SubStringCount <= StringCount; Index++, StringCount--)
	{
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
