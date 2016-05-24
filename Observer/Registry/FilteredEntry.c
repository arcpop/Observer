#include "Includes.h"

_Use_decl_annotations_
BOOLEAN IsFilteredRegistryKey(
	PUNICODE_STRING KeyPath,
	PREGISTRY_FILTER_CONTEXT Context,
	PREGISTRY_FILTER_FILTERED_KEY_ENTRY* EntryOut
)
{
	PRESOURCE_LIST_ENTRY pEntry;

	for (
		pEntry = NextListEntry(
			&Context->FilteredRegistryKeysList,
			&Context->FilteredRegistryKeysList.Entry,
			FALSE);
		pEntry != &Context->FilteredRegistryKeysList.Entry;
		pEntry = NextListEntry(
			&Context->FilteredRegistryKeysList,
			pEntry,
			TRUE)
	)
	{
		PREGISTRY_FILTER_FILTERED_KEY_ENTRY CurrentEntry;
		CurrentEntry = CONTAINING_RECORD(pEntry, REGISTRY_FILTER_FILTERED_KEY_ENTRY, ListEntry);
		if (RtlCompareUnicodeString(KeyPath, &CurrentEntry->FullRegistryKeyPath, TRUE) == 0)
		{
			if (EntryOut != NULL)
			{
				*EntryOut = CurrentEntry;
				return TRUE;
			}
			if (ReleaseListEntry(&pEntry))
			{
				REGISTRY_FILTER_FREE(CurrentEntry);
			}
			return TRUE;
		}
	}
	return FALSE;
}