#include "Includes.h"

#include "../Log/Log.h"


_Use_decl_annotations_
NTSTATUS RegistryFilterAdd(
	PVOID pContext,
	PUNICODE_STRING KeyPath,
	LONG ActionFlags,
	PVOID Reserved
)
{
	PREGISTRY_FILTER_FILTERED_KEY_ENTRY Entry;
	PREGISTRY_FILTER_CONTEXT Context = (PREGISTRY_FILTER_CONTEXT)pContext;
	Entry = NULL;
	__try 
	{
		Entry = REGISTRY_FILTER_ALLOCATE(
			sizeof(REGISTRY_FILTER_FILTERED_KEY_ENTRY) + KeyPath->Length, 
			NonPagedPool
		);

		if (Entry == NULL)
		{
			DEBUG_LOG("RegistryFilterAdd: Out of memory");
			return STATUS_NO_MEMORY;
		}

		Entry->ActionFlags = ActionFlags;

		RtlCopyMemory(
			Entry->FullRegistryKeyPathBuffer, 
			KeyPath->Buffer, 
			KeyPath->Length
		);

		Entry->FullRegistryKeyPath.Buffer = Entry->FullRegistryKeyPathBuffer;
		Entry->FullRegistryKeyPath.Length = 
			Entry->FullRegistryKeyPath.MaximumLength = KeyPath->Length;

		Entry->Reserved = Reserved;

		InsertListEntry(
			&Context->FilteredRegistryKeysList, 
			&Entry->ListEntry
		);

		ReleaseRegistryFilterFilteredKeyEntry(Entry);

		return STATUS_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		if (Entry != NULL)
		{
			REGISTRY_FILTER_FREE(Entry);
		}
		return STATUS_ACCESS_VIOLATION;
	}
}
