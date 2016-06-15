#ifndef REGISTRY_FILTER_H
#define REGISTRY_FILTER_H
#pragma once


#include <ntddk.h>

#include "../Rule.h"
#include "../Log/Log.h"

#define REGISTRY_FILTER_TAG 'FRbO'
#define REGISTRY_FILTER_ALLOCATE(size, pool) MyAllocatePoolWithTag(pool, size, REGISTRY_FILTER_TAG)
#define REGISTRY_FILTER_FREE(ptr) MyFreePoolWithTag(ptr, REGISTRY_FILTER_TAG)

static PVOID MyAllocatePoolWithTag(POOL_TYPE pool, SIZE_T size, ULONG tag)
{
	PVOID p = ExAllocatePoolWithTag(pool, size, tag);
	//DEBUG_LOG("Alloc: %p\n", p);
	return p;
}
static void MyFreePoolWithTag(PVOID ptr, ULONG tag)
{
	UNREFERENCED_PARAMETER(tag);
	//DEBUG_LOG("Free: %p\n", ptr);
	ExFreePoolWithTag(ptr, tag);
}

EX_CALLBACK_FUNCTION RegistryFilterCallback;

typedef struct _REGISTRY_FILTER_RULE_ENTRY {
	LIST_ENTRY				ListEntry;
	EX_RUNDOWN_REF			RundownProtection;
	OBSERVER_RULE_HANDLE	RuleHandle;
	UNICODE_STRING			Path;
	OBSERVER_REGISTRY_RULE	Rule;
} REGISTRY_FILTER_RULE_ENTRY, *PREGISTRY_FILTER_RULE_ENTRY;

typedef struct _REGISTRY_FILTER_CONTEXT {
	PDRIVER_OBJECT DriverObject;
	LARGE_INTEGER FilterContextCookie;
} REGISTRY_FILTER_CONTEXT, *PREGISTRY_FILTER_CONTEXT;


#define MAX_OBJECT_CONTEXT_RULES 16
typedef struct _REGISTRY_FILTER_OBJECT_CONTEXT {
	ULONG NumberOfRules;
	PREGISTRY_FILTER_RULE_ENTRY RuleEntries[MAX_OBJECT_CONTEXT_RULES];
} REGISTRY_FILTER_OBJECT_CONTEXT, *PREGISTRY_FILTER_OBJECT_CONTEXT;

NTSTATUS RegistryFilterPostCreateKey(
	_In_ PREGISTRY_FILTER_CONTEXT pContext,
	_In_ PREG_POST_CREATE_KEY_INFORMATION Info
);

NTSTATUS RegistryFilterPostCreateKeyEx(
	_In_ PREGISTRY_FILTER_CONTEXT pContext,
	_In_ PREG_POST_OPERATION_INFORMATION Info
);

NTSTATUS RegistryFilterPostOpenKey(
	_In_ PREGISTRY_FILTER_CONTEXT pContext,
	_In_ PREG_POST_OPEN_KEY_INFORMATION Info
);

NTSTATUS RegistryFilterPostOpenKeyEx(
	_In_ PREGISTRY_FILTER_CONTEXT pContext,
	_In_ PREG_POST_OPERATION_INFORMATION Info
);

NTSTATUS RegistryFilterCleanupObjectContext(
	_In_ PREGISTRY_FILTER_CONTEXT pContext,
	_In_ PREG_CALLBACK_CONTEXT_CLEANUP_INFORMATION Info
);

NTSTATUS RegistryFilterPreSetValueKey(
	_In_ PREGISTRY_FILTER_CONTEXT pContext,
	_In_ PREG_SET_VALUE_KEY_INFORMATION Info
);

PLIST_ENTRY NextRegistryFilterRuleListEntry(
	_In_ PLIST_ENTRY CurrentEntry, 
	_In_ BOOLEAN ReleaseCurrent
);

BOOLEAN RegistryMatchStrings(
	_In_ PCUNICODE_STRING RuleString,
	_In_ PCUNICODE_STRING RegString,
	_In_ ULONG	MatchType
);

VOID CleanupObjectContext(
	_In_ PVOID ObjectContext
);

extern LIST_ENTRY RegistryFilterRuleList;
extern FAST_MUTEX RegistryFilterRuleListMutex;

#endif // !REGISTRY_FILTER_H
