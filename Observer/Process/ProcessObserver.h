#ifndef PROCESS_OBSERVER_H
#define PROCESS_OBSERVER_H
#pragma once

#include "../Process.h"
#include "../ResourceList.h"


#define PROCESS_OBSERVER_TAG 'sPbO'
#define PROCESS_OBSERVER_ALLOCATE(size, type) ExAllocatePoolWithTag(type, size, PROCESS_OBSERVER_TAG)
#define PROCESS_OBSERVER_FREE(ptr) ExFreePoolWithTag(ptr, PROCESS_OBSERVER_TAG)

typedef struct _PROCESS_RULE_LIST_ENTRY
{
	RESOURCE_LIST_ENTRY				ListEntry;
	OBSERVER_RULE_HANDLE			RuleHandle;
	OBSERVER_PROCESS_CREATION_RULE	Rule;
} PROCESS_RULE_LIST_ENTRY, *PPROCESS_RULE_LIST_ENTRY;


extern RESOURCE_LIST_ENTRY_HEAD	ProcessRuleList;

VOID ProcessNotifyRoutine(
	_Inout_	 PEPROCESS Process,
	_In_     HANDLE ProcessId,
	_In_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
);


#endif // !PROCESS_OBSERVER_H
