#include "Processcache.h"
#include "../Log/Log.h"


NTSYSAPI NTSTATUS NTAPI PsReferenceProcessFilePointer(
	_In_ PEPROCESS 	Process,
	_Out_ PFILE_OBJECT *FileObject
);
NTSYSAPI NTSTATUS NTAPI PsLookupProcessByProcessId(
	_In_  HANDLE    ProcessId,
	_Out_ PEPROCESS *Process
);
NTSYSAPI NTSTATUS NTAPI IoQueryFileDosDeviceName(
	_In_  PFILE_OBJECT             FileObject,
	_Out_ POBJECT_NAME_INFORMATION *ObjectNameInformation
);

#define PROCESS_CACHE_CHAINS_SHIFT 6
#define PROCESS_CACHE_CHAINS (1 << PROCESS_CACHE_CHAINS_SHIFT)

#define PROCESS_CACHE_TAG 'CPbO'
#define PROCESS_CACHE_ALLOCATE(size, pool) ExAllocatePoolWithTag(pool, size, PROCESS_CACHE_TAG)
#define PROCESS_CACHE_FREE(ptr) ExFreePoolWithTag(ptr, PROCESS_CACHE_TAG)


PROCESS_CACHE_CHAIN ProcessCache[PROCESS_CACHE_CHAINS];
static EX_RUNDOWN_REF ProcessCacheRundownProtection;


VOID ReleaseProcessCacheEntry(
	_In_ PPROCESS_CACHE_ENTRY Entry
)
{
	if (InterlockedDecrement64(&Entry->ReferenceCounter) == 0)
	{
		PROCESS_CACHE_FREE(Entry);
	}
}

static BOOLEAN ProcessCacheCreated = FALSE;

NTSTATUS ProcessCacheInitialize()
{
	NTSTATUS Status;
	PPROCESS_CACHE_CHAIN pChain;
	RtlSecureZeroMemory(ProcessCache, sizeof(ProcessCache));
	for (LONG i = 0; i < PROCESS_CACHE_CHAINS; i++)
	{
		pChain = &ProcessCache[i];
		Status = ExInitializeResourceLite(&pChain->RWLock);
		if (!NT_SUCCESS(Status))
		{
			for (i = i - 1; i >= 0; --i)
			{
				pChain = &ProcessCache[i];
				ExDeleteResourceLite(&pChain->RWLock);
			}
			return Status;
		}
		InitializeListHead(&pChain->Entries);
	}
	ExInitializeRundownProtection(&ProcessCacheRundownProtection);
	return STATUS_SUCCESS;
}
NTSTATUS ProcessCacheUnload()
{
	PPROCESS_CACHE_CHAIN pChain;
	ExWaitForRundownProtectionRelease(&ProcessCacheRundownProtection);

	for (ULONG i = 0; i < PROCESS_CACHE_CHAINS; i++)
	{
		pChain = &ProcessCache[i];
		ExEnterCriticalRegionAndAcquireResourceExclusive(&pChain->RWLock);
		{
			for (PLIST_ENTRY pEntry = pChain->Entries.Flink;
				pEntry != &pChain->Entries;
				pEntry = pEntry->Flink)
			{
				PPROCESS_CACHE_ENTRY CurrentEntry = CONTAINING_RECORD(pEntry, PROCESS_CACHE_ENTRY, ListEntry);
				ReleaseProcessCacheEntry(CurrentEntry);
			}
		}
		ExReleaseResourceAndLeaveCriticalRegion(&pChain->RWLock);
		ExDeleteResourceLite(&pChain->RWLock);
	}
	return STATUS_SUCCESS;
}

PPROCESS_CACHE_ENTRY ProcessCacheFindInCache(
	_In_ HANDLE ProcessId
)
{
	PPROCESS_CACHE_ENTRY FoundEntry = NULL;
	ULONG_PTR Key = 0, Tmp = (ULONG_PTR)ProcessId;
	PPROCESS_CACHE_CHAIN Chain;
	Key = Tmp % PROCESS_CACHE_CHAINS;
	Key ^= (Tmp >> PROCESS_CACHE_CHAINS_SHIFT) % PROCESS_CACHE_CHAINS;

	if (!ExAcquireRundownProtection(&ProcessCacheRundownProtection))
	{
		return NULL;
	}
	Chain = &ProcessCache[Key];
	ExEnterCriticalRegionAndAcquireResourceShared(&Chain->RWLock);
	{
		for (PLIST_ENTRY pEntry = Chain->Entries.Flink;
			pEntry != &Chain->Entries;
			pEntry = pEntry->Flink)
		{
			PPROCESS_CACHE_ENTRY CurrentEntry = CONTAINING_RECORD(pEntry, PROCESS_CACHE_ENTRY, ListEntry);
			if (CurrentEntry->ProcessId == ProcessId)
			{
				FoundEntry = CurrentEntry;
				InterlockedIncrement64(&CurrentEntry->ReferenceCounter);
				break;
			}
		}
	}
	ExReleaseResourceAndLeaveCriticalRegion(&Chain->RWLock);
	ExReleaseRundownProtection(&ProcessCacheRundownProtection);
	return FoundEntry;
}

NTSTATUS ProcessCacheInsert(
	_In_ HANDLE ProcessId,
	_In_ PPROCESS_CACHE_ENTRY NewEntry
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG_PTR Key = 0, Tmp = (ULONG_PTR)ProcessId;
	PPROCESS_CACHE_CHAIN Chain;
	Key = Tmp % PROCESS_CACHE_CHAINS;
	Key ^= (Tmp >> PROCESS_CACHE_CHAINS_SHIFT) % PROCESS_CACHE_CHAINS;

	if (!ExAcquireRundownProtection(&ProcessCacheRundownProtection))
	{
		return STATUS_SHUTDOWN_IN_PROGRESS;
	}
	Chain = &ProcessCache[Key];
	ExEnterCriticalRegionAndAcquireResourceExclusive(&Chain->RWLock);
	{
		for (PLIST_ENTRY pEntry = Chain->Entries.Flink;
			pEntry != &Chain->Entries;
			pEntry = pEntry->Flink)
		{
			PPROCESS_CACHE_ENTRY CurrentEntry = CONTAINING_RECORD(pEntry, PROCESS_CACHE_ENTRY, ListEntry);
			if (CurrentEntry->ProcessId == ProcessId)
			{
				Status = STATUS_ALREADY_REGISTERED;
				break;
			}
		}
		if (Status != STATUS_ALREADY_REGISTERED)
		{
			InterlockedIncrement64(&NewEntry->ReferenceCounter);
			InsertHeadList(&Chain->Entries, &NewEntry->ListEntry);
		}
	}
	ExReleaseResourceAndLeaveCriticalRegion(&Chain->RWLock);
	ExReleaseRundownProtection(&ProcessCacheRundownProtection);
	return Status;
}

PPROCESS_CACHE_ENTRY ProcessCacheLookupProcessById(
	_In_ HANDLE ProcessId
)
{
	PPROCESS_CACHE_ENTRY Entry = NULL;
	NTSTATUS Status;
	PEPROCESS Process = NULL;
	PFILE_OBJECT ProcessFileObject = NULL;
	POBJECT_NAME_INFORMATION ProcessNameInfo = NULL;

	Entry = ProcessCacheFindInCache(ProcessId);
	if (Entry != NULL)
	{
		return Entry;
	}

	Status = PsLookupProcessByProcessId(ProcessId, &Process);
	if (!NT_SUCCESS(Status))
	{
		DEBUG_LOG("ProcessCacheLookupProcessById: Process %d not found (0x%.8X)", ProcessId, Status);
		return NULL;
	}

	Status = PsReferenceProcessFilePointer(Process, &ProcessFileObject);
	if (!NT_SUCCESS(Status))
	{
		ObDereferenceObject(Process);
		DEBUG_LOG("ProcessCacheLookupProcessById: Process file object for %d not found (0x%.8X)", ProcessId, Status);
		return NULL;
	}

	Status = IoQueryFileDosDeviceName(ProcessFileObject, &ProcessNameInfo);

	ObDereferenceObject(Process);
	ObDereferenceObject(ProcessFileObject);

	if (!NT_SUCCESS(Status) || (ProcessNameInfo == NULL))
	{
		DEBUG_LOG("IoQueryFileDosDeviceName: Process file name for %d not found (0x%.8X)", ProcessId, Status);
		return NULL;
	}

	Entry = PROCESS_CACHE_ALLOCATE(sizeof(PROCESS_CACHE_ENTRY), NonPagedPool);
	if (Entry == NULL)
	{
		ExFreePool(ProcessNameInfo);
		DEBUG_LOG("ProcessCacheLookupProcessById: Out of memory");
		return NULL;
	}

	Entry->DosExeFileName = ProcessNameInfo;
	Entry->ProcessId = ProcessId;
	Entry->ReferenceCounter = 0;
	Entry->ProcessType = PROCESS_TYPE_NORMAL;

	Status = ProcessCacheInsert(ProcessId, Entry);
	if (NT_SUCCESS(Status))
	{
		return Entry;
	}

	ExFreePool(ProcessNameInfo);
	PROCESS_CACHE_FREE(Entry);

	DEBUG_LOG("ProcessCacheLookupProcessById: ProcessCacheInsert returned 0x%.8X", Status);

	//Try again, maybe someone else was faster...
	return ProcessCacheFindInCache(ProcessId);
}

BOOLEAN IsSystemProcess(
	_In_ HANDLE ProcessId
)
{
	PPROCESS_CACHE_ENTRY Entry;
	BOOLEAN SystemProcess;

	Entry = ProcessCacheLookupProcessById(ProcessId);
	if (Entry == NULL)
	{
		return FALSE;
	}
	SystemProcess = Entry->ProcessType != PROCESS_TYPE_NORMAL;
	ReleaseProcessCacheEntry(Entry);
	return SystemProcess;
}


