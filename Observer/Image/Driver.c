#include "ImageLoadNotify.h"
#include "../Log/Log.h"
#include "../Notification/NotificationQueue.h"
#include <ntimage.h>
#include <intrin.h>


OBSERVER_RESOURCE_LIST DriverLoadRuleList;


NTSTATUS DriverLoadAddRule(
	POBSERVER_DRIVER_LOAD_RULE Rule,
	POBSERVER_RULE_HANDLE RuleHandle
)
{
	static LONG64 DriverLoadRuleCounter = 0;
	POBSERVER_DRIVER_LOAD_RULE_ENTRY pEntry;
	ULONG Length = FIELD_OFFSET(OBSERVER_DRIVER_LOAD_RULE_ENTRY, Rule.Path)
		+ ((1 + Rule->PathLength) * sizeof(WCHAR));

	pEntry = IMAGE_NOTIFICATION_ALLOCATE(Length, NonPagedPool);
	
	if (!pEntry)
	{
		return STATUS_NO_MEMORY;
	}

	RtlSecureZeroMemory(pEntry, Length);
	pEntry->Rule.PathLength = Rule->PathLength;
	RtlCopyMemory(&(pEntry->Rule.Path[0]), Rule->Path, Rule->PathLength * sizeof(WCHAR));
	RtlInitUnicodeString(&pEntry->DriverPath, pEntry->Rule.Path);
	pEntry->Rule.Action = Rule->Action;
	pEntry->Rule.DriverLoadCheckFlags = Rule->DriverLoadCheckFlags;
	pEntry->RuleHandle.RuleHandle = RuleHandle->RuleHandle = InterlockedIncrement64(&DriverLoadRuleCounter);
	pEntry->RuleHandle.RuleType = RuleHandle->RuleType = RULE_TYPE_DRIVER_LOAD;

	InsertResourceListHead(&DriverLoadRuleList, &pEntry->ListEntry);
	return STATUS_SUCCESS;
}


NTSTATUS DriverLoadRemoveRule(
	POBSERVER_RULE_HANDLE RuleHandle
)
{
	PLIST_ENTRY pEntry;
	WLockResourceList(&DriverLoadRuleList);
	for (pEntry = DriverLoadRuleList.ListEntry.Flink; pEntry != &DriverLoadRuleList.ListEntry; pEntry = pEntry->Flink)
	{
		POBSERVER_DRIVER_LOAD_RULE_ENTRY CurrentEntry = CONTAINING_RECORD(pEntry, OBSERVER_DRIVER_LOAD_RULE_ENTRY, ListEntry);
		if (CurrentEntry->RuleHandle.RuleHandle == RuleHandle->RuleHandle)
		{
			RemoveEntryList(&CurrentEntry->ListEntry);
			IMAGE_NOTIFICATION_FREE(CurrentEntry);
			WUnlockResourceList(&DriverLoadRuleList);
			return STATUS_SUCCESS;
		}
	}
	WUnlockResourceList(&DriverLoadRuleList);
	return STATUS_NOT_FOUND;
}

BOOLEAN BlockDriver(PIMAGE_INFO ImageInfo);

_Use_decl_annotations_
VOID HandleDriverLoad(
	PUNICODE_STRING FullImageName,
	PIMAGE_INFO ImageInfo
)
{
	PLIST_ENTRY pEntry;
	BOOLEAN Block = FALSE;

	if (FullImageName == NULL)
	{
		DEBUG_LOG("No valid FullImageName, ignoring");
		return;
	}

	RLockResourceList(&DriverLoadRuleList);
	for (pEntry = DriverLoadRuleList.ListEntry.Flink; pEntry != &DriverLoadRuleList.ListEntry; pEntry = pEntry->Flink)
	{
		POBSERVER_DRIVER_LOAD_RULE_ENTRY CurrentEntry = CONTAINING_RECORD(pEntry, OBSERVER_DRIVER_LOAD_RULE_ENTRY, ListEntry);
		BOOLEAN ApplyAction = FALSE;
		if (CurrentEntry->Rule.DriverLoadCheckFlags & DRIVER_LOAD_CHECK_SIGNED)
		{
			if (ImageInfo->ImageSignatureType == SeImageSignatureNone)
				ApplyAction = TRUE;
		}
		if (!ApplyAction)
		{
			BOOLEAN CaseInsensitive = CurrentEntry->Rule.DriverLoadCheckFlags & DRIVER_LOAD_CHECK_CASE_INSENSITIVE;
			if (CurrentEntry->Rule.DriverLoadCheckFlags & DRIVER_LOAD_CHECK_PATH_CONTAINS)
			{
				if (UtilUnicodeStringContains(FullImageName, &CurrentEntry->DriverPath, CaseInsensitive))
				{
					ApplyAction = TRUE;
				}
			}
			else if (CurrentEntry->Rule.DriverLoadCheckFlags & DRIVER_LOAD_CHECK_PATH_NOT_CONTAINS)
			{
				if (!UtilUnicodeStringContains(FullImageName, &CurrentEntry->DriverPath, CaseInsensitive))
				{
					ApplyAction = TRUE;
				}
			}
		}

		if (ApplyAction)
		{
			if (CurrentEntry->Rule.Action & ACTION_BLOCK)
			{
				Block = TRUE;
			}
			if (CurrentEntry->Rule.Action & ACTION_DBGPRINT)
			{
				DbgPrint("Driver loaded: %wZ at 0x%p", FullImageName, ImageInfo->ImageBase);
			}
			if (CurrentEntry->Rule.Action & ACTION_REPORT)
			{
				PNOTIFICATION_ENTRY pNotification = NotificationCreate(RULE_TYPE_DRIVER_LOAD);
				if (pNotification != NULL)
				{
					ULONG Length;
					pNotification->Data.Reaction = CurrentEntry->Rule.Action;
					pNotification->Data.Types.DriverLoaded.ImageBase = (UINT64)ImageInfo->ImageBase;
					pNotification->Data.Types.DriverLoaded.ImageSigned = ImageInfo->ImageSignatureType;
					pNotification->Data.Types.DriverLoaded.ImageSize = (UINT64)ImageInfo->ImageSize;
					Length = FullImageName->Length / 2;
					if (Length >= NOTIFICATION_STRING_BUFFER_SIZE)
					{
						pNotification->Data.Types.DriverLoaded.Truncated = (UINT16)Length - NOTIFICATION_STRING_BUFFER_SIZE - 1;
						Length = 999;
					}
					RtlCopyMemory(pNotification->Data.Types.DriverLoaded.ImageName, FullImageName->Buffer, Length * sizeof(WCHAR));
					NotificationSend(pNotification);
				}
			}
		}
	}
	RUnlockResourceList(&DriverLoadRuleList);
	if (Block)
	{
		if (BlockDriver(ImageInfo))
		{
			DEBUG_LOG("HandleDriverLoad: Blocked driver load %wZ", FullImageName);
		}
	}
}

#ifdef _WIN64
/*
0:  48 31 c0                xor    rax,rax
3:  0d 01 00 00 c0          or     eax,0xc0000001	# STATUS_UNSUCCESSFUL
8:  c3						retn					# Args are in rcx and rdx
*/
const UCHAR Shellcode[] = { 0x48, 0x31, 0xC0, 0x0D, 0x01, 0x00, 0x00, 0xC0, 0xC3 };
#else //!_WIN64
/*
0:  31 c0                   xor    eax,eax
2:  0d 01 00 00 c0          or     eax,0xc0000001	# STATUS_UNSUCCESSFUL
7:  c2 08 00                ret    0x8				# Args are on the stack
*/
const UCHAR Shellcode[] = { 0x31, 0xC0, 0x0D, 0x01, 0x00, 0x00, 0xC0, 0xC2, 0x08, 0x00 };
#endif

const SIZE_T ShellcodeLength = sizeof(Shellcode);

BOOLEAN BlockDriver(PIMAGE_INFO ImageInfo)
{
	PIMAGE_NT_HEADERS NtHeaders;
	PIMAGE_DOS_HEADER DosHeader;
	ULONG_PTR ImageBase = (ULONG_PTR)ImageInfo->ImageBase;
	SIZE_T Length;
	PUCHAR EntryPoint;
	PMDL pMdl;
	BOOLEAN Locked;

	DosHeader = (PIMAGE_DOS_HEADER)ImageBase;

	if (ImageInfo->ImageSize < sizeof(IMAGE_DOS_HEADER))
	{
		DEBUG_LOG("BlockDriver: Image smaller than DOS header");
		return FALSE;
	}

	if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		DEBUG_LOG("BlockDriver: Image with invalid DOS signature");
		return FALSE;
	}

	NtHeaders = (PIMAGE_NT_HEADERS)(ImageBase + DosHeader->e_lfanew);
	Length = (SIZE_T)NtHeaders;
	Length -= ImageBase;
	Length += sizeof(IMAGE_NT_HEADERS);

	if (ImageInfo->ImageSize < Length)
	{
		DEBUG_LOG("BlockDriver: Image smaller than NT headers");
		return FALSE;
	}

	if (NtHeaders->OptionalHeader.AddressOfEntryPoint == 0)
	{
		DEBUG_LOG("BlockDriver: Image has no entry point...");
		return FALSE;
	}

	if (ImageInfo->ImageSize < ((SIZE_T)NtHeaders->OptionalHeader.AddressOfEntryPoint + ShellcodeLength))
	{
		DEBUG_LOG("BlockDriver: Entry point outside of image");
		return FALSE;
	}
	
	EntryPoint = (PUCHAR)(ImageBase + NtHeaders->OptionalHeader.AddressOfEntryPoint);
	
	pMdl = IoAllocateMdl(EntryPoint, (ULONG)ShellcodeLength, FALSE, FALSE, NULL);
	if (pMdl == NULL)
	{
		DEBUG_LOG("BlockDriver: IoAllocateMdl failed to allocate the MDL");
		return FALSE;
	}
	DEBUG_LOG("Attempting to patch at %p", EntryPoint);
	Locked = FALSE;
	__try
	{
		ULONG_PTR Cr0;
		MmProbeAndLockPages(pMdl, KernelMode, IoModifyAccess);
		Locked = TRUE;

		//Disable interrupts
		_disable();
		//Disable write protection (WP-bit, 16th Bit of CR0) 
		Cr0 = __readcr0();
		__writecr0(Cr0 & ~0x10000);

		//Patch the entry point to return unsuccessful status.
		//This makes the Io Manager to unload the driver.
		memcpy(EntryPoint, Shellcode, ShellcodeLength);

		//Enable write protection (WP-bit, 16th Bit of CR0)
		Cr0 = __readcr0();
		__writecr0(Cr0 | 0x10000);
		//Reenable interrupts
		_enable();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DEBUG_LOG("BlockDriver: Failed to patch (0x%.8X)", GetExceptionCode());
	}
	if (Locked)
	{
		MmUnlockPages(pMdl);
	}
	IoFreeMdl(pMdl);
	DEBUG_LOG("BlockDriver: Patched DriverEntry");
	return TRUE;
}

