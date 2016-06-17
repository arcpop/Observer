#include "ProcessObserver.h"

#include "../Util/Util.h"
#include "../Log/Log.h"
#include "../Notification/NotificationQueue.h"

LIST_ENTRY ProcessRuleList;
FAST_MUTEX ProcessRuleListMutex;

_Use_decl_annotations_
NTSTATUS ProcessObserverUnload()
{
	NTSTATUS Status;
	Status = PsSetCreateProcessNotifyRoutineEx(ProcessNotifyRoutine, TRUE);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}
	DEBUG_LOG("ProcessObserverUnload completed");
	return Status;
}
_Use_decl_annotations_
NTSTATUS ProcessObserverInitialize()
{
	NTSTATUS Status;
	InitializeListHead(&ProcessRuleList);
	ExInitializeFastMutex(&ProcessRuleListMutex);
	Status = PsSetCreateProcessNotifyRoutineEx(ProcessNotifyRoutine, FALSE);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}
	DEBUG_LOG("ProcessObserverInitialize completed");
	return Status;
}

_Use_decl_annotations_
NTSTATUS ProcessObserverAddRule(
	POBSERVER_PROCESS_CREATION_RULE Rule,
	POBSERVER_RULE_HANDLE			RuleHandle
)
{
	static LONG64 RuleCounter = 0;
	PPROCESS_RULE_LIST_ENTRY pEntry;
	ULONG Length = FIELD_OFFSET(PROCESS_RULE_LIST_ENTRY, Rule.ParentProcessName) +
		((Rule->ParentProcessNameLength + 1) * sizeof(WCHAR));

	pEntry = PROCESS_OBSERVER_ALLOCATE(
		Length,
		NonPagedPool);

	if (pEntry == NULL)
	{
		DEBUG_LOG("ProcessObserverAddRule: Out of memory");
		return STATUS_NO_MEMORY;
	}

	RtlCopyMemory(
		&pEntry->Rule,
		Rule,
		sizeof(OBSERVER_PROCESS_CREATION_RULE)
	);

	RtlCopyMemory(
		pEntry->Rule.ParentProcessName,
		Rule->ParentProcessName,
		Rule->ParentProcessNameLength * sizeof(WCHAR)
	);
	pEntry->Rule.ParentProcessName[Rule->ParentProcessNameLength] = L'\0';

	RuleHandle->RuleHandle = pEntry->RuleHandle.RuleHandle = InterlockedIncrement64(&RuleCounter);
	RuleHandle->RuleType = pEntry->RuleHandle.RuleType = RULE_TYPE_CREATE_PROCESS;

	ExInitializeRundownProtection(&pEntry->RundownProtection);

	ExAcquireFastMutex(&ProcessRuleListMutex);
	InsertHeadList(&ProcessRuleList, &pEntry->ListEntry);
	ExReleaseFastMutex(&ProcessRuleListMutex);

	return STATUS_SUCCESS;
}

PLIST_ENTRY NextProcessRuleListEntry(
	_In_ PLIST_ENTRY CurrentEntry, 
	_In_ BOOLEAN ReleaseCurrent
)
{
	PLIST_ENTRY pNextEntry;
	ExAcquireFastMutex(&ProcessRuleListMutex);
	if (ReleaseCurrent)
	{
		PPROCESS_RULE_LIST_ENTRY pCurrentEntry = CONTAINING_RECORD(CurrentEntry, PROCESS_RULE_LIST_ENTRY, ListEntry);
		ExReleaseRundownProtection(&pCurrentEntry->RundownProtection);
	}
TryAgain:
	pNextEntry = CurrentEntry->Flink;
	if (pNextEntry != &ProcessRuleList)
	{
		PPROCESS_RULE_LIST_ENTRY pNext = CONTAINING_RECORD(pNextEntry, PROCESS_RULE_LIST_ENTRY, ListEntry);
		if (ExAcquireRundownProtection(&pNext->RundownProtection))
		{
			ExReleaseFastMutex(&ProcessRuleListMutex);
			return pNextEntry;
		}
		CurrentEntry = pNextEntry;
		goto TryAgain;
	}
	ExReleaseFastMutex(&ProcessRuleListMutex);
	return NULL;
}

_Use_decl_annotations_
NTSTATUS ProcessObserverRemoveRule(
	POBSERVER_RULE_HANDLE RuleHandle
)
{
	PLIST_ENTRY pEntry;
	for (
		pEntry = NextProcessRuleListEntry(&ProcessRuleList, FALSE); 
		pEntry != NULL; 
		pEntry = NextProcessRuleListEntry(pEntry, TRUE)
	)
	{
		PPROCESS_RULE_LIST_ENTRY pCurrentEntry = CONTAINING_RECORD(pEntry, PROCESS_RULE_LIST_ENTRY, ListEntry);
		if (RuleHandle->RuleHandle == pCurrentEntry->RuleHandle.RuleHandle)
		{
			ExAcquireFastMutex(&ProcessRuleListMutex);
			if (pEntry->Blink == NULL)
			{
				//Somebody is removing us at the moment.
				ExReleaseFastMutex(&ProcessRuleListMutex);
				ExReleaseRundownProtection(&pCurrentEntry->RundownProtection);
				return STATUS_NOT_FOUND;
			}
			RemoveHeadList(pEntry->Blink);
			pEntry->Flink = pEntry->Blink = NULL;
			ExReleaseFastMutex(&ProcessRuleListMutex);
			ExReleaseRundownProtection(&pCurrentEntry->RundownProtection);
			ExWaitForRundownProtectionRelease(&pCurrentEntry->RundownProtection);
			PROCESS_OBSERVER_FREE(pCurrentEntry);
			return STATUS_SUCCESS;
		}
	}
	return STATUS_NOT_FOUND;
}

_Use_decl_annotations_
VOID ProcessNotifyRoutine(
	PEPROCESS Process,
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
	PLIST_ENTRY pEntry;
	UINT64 PID, TID, PPID;
	if (CreateInfo == NULL)
	{
		//Do nothing on process exits.
		return;
	}
	PID		= (UINT64)CreateInfo->CreatingThreadId.UniqueProcess;
	TID		= (UINT64)CreateInfo->CreatingThreadId.UniqueThread;
	PPID	= (UINT64)CreateInfo->ParentProcessId;

	for (
		pEntry = NextProcessRuleListEntry(&ProcessRuleList, FALSE);
		pEntry != NULL;
		pEntry = NextProcessRuleListEntry(pEntry, TRUE)
		)
	{
		PPROCESS_RULE_LIST_ENTRY pCurrentEntry = CONTAINING_RECORD(pEntry, PROCESS_RULE_LIST_ENTRY, ListEntry);
		BOOLEAN Matches = TRUE;
		if (pCurrentEntry->Rule.ProcessRuleCheckFlags & PROCESS_CREATION_CHECK_PARENT_ID)
		{
			Matches = (PPID == pCurrentEntry->Rule.ParentProcessID);
		}
		if (pCurrentEntry->Rule.ProcessRuleCheckFlags & PROCESS_CREATION_CHECK_CREATING_PROCESS)
		{
			Matches = Matches && (PID == pCurrentEntry->Rule.CreatingProcessID);
		}
		if (pCurrentEntry->Rule.ProcessRuleCheckFlags & PROCESS_CREATION_CHECK_CREATING_THREAD)
		{
			Matches = Matches && (TID == pCurrentEntry->Rule.CreatingThreadID);
		}
		if (pCurrentEntry->Rule.ProcessRuleCheckFlags & PROCESS_CREATION_CHECK_PARENT_NAME_CONTAINS)
		{
			UNICODE_STRING ParentProcessName;
			RtlInitUnicodeString(&ParentProcessName, pCurrentEntry->Rule.ParentProcessName);
			Matches = Matches && UtilUnicodeStringContains(CreateInfo->ImageFileName, &ParentProcessName, TRUE);
		}
		if (pCurrentEntry->Rule.ProcessRuleCheckFlags & PROCESS_CREATION_CHECK_PARENT_NAME_EQUALS)
		{
			UNICODE_STRING ParentProcessName;
			RtlInitUnicodeString(&ParentProcessName, pCurrentEntry->Rule.ParentProcessName);
			Matches = Matches && RtlEqualUnicodeString(CreateInfo->ImageFileName, &ParentProcessName, TRUE);
		}
		if (pCurrentEntry->Rule.ProcessRuleCheckFlags & PROCESS_CREATION_CHECK_PARENT_NAME_ENDS_WITH)
		{
			UNICODE_STRING ParentProcessName;
			RtlInitUnicodeString(&ParentProcessName, pCurrentEntry->Rule.ParentProcessName);
			Matches = Matches && RtlSuffixUnicodeString(&ParentProcessName, CreateInfo->ImageFileName, TRUE);
		}
		if (Matches)
		{
			if ((pCurrentEntry->Rule.Action & ACTION_BLOCK) ||
				(pCurrentEntry->Rule.Action & ACTION_REPORT))
			{
				PNOTIFICATION_ENTRY pNotification = NotificationCreate(RULE_TYPE_CREATE_PROCESS);
				if (pNotification != NULL)
				{
					UINT16 CopyLength = (NOTIFICATION_STRING_BUFFER_SIZE - 1) * sizeof(WCHAR);
					pNotification->Data.Reaction = pCurrentEntry->Rule.Action;
					pNotification->Data.Types.ProcessCreated.NewProcessID = (UINT64)ProcessId;
					pNotification->Data.Types.ProcessCreated.CreatingThreadID = TID;
					pNotification->Data.Types.ProcessCreated.CreatingProcessID = PID;
					pNotification->Data.Types.ProcessCreated.ParentProcessID = PPID;
					if (CreateInfo->ImageFileName->Length > CopyLength)
					{
						pNotification->Data.Types.ProcessCreated.Truncated = 
							(CreateInfo->ImageFileName->Length - CopyLength) >> 1;
					}
					else
					{
						CopyLength = CreateInfo->ImageFileName->Length;
					}
					RtlCopyMemory(
						pNotification->Data.Types.ProcessCreated.ImageNamePath,
						CreateInfo->ImageFileName->Buffer,
						CopyLength
					);
					pNotification->Data.Types.ProcessCreated.ImageNamePath[CopyLength] = L'\0';
					NotificationSend(pNotification);
				}
				if (pCurrentEntry->Rule.Action & ACTION_BLOCK)
				{
					CreateInfo->CreationStatus = STATUS_ACCESS_DENIED;
				}
			}
			else if (pCurrentEntry->Rule.Action & ACTION_DBGPRINT)
			{
				DbgPrint("ProcessNotifyRoutine: Created %wZ", CreateInfo->ImageFileName);
			}
		}
	}
	UNREFERENCED_PARAMETER(Process);
}