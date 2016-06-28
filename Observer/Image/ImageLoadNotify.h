#ifndef IMAGE_LOAD_NOTIFY_H
#define IMAGE_LOAD_NOTIFY_H
#pragma once

#include <ntddk.h>
#include "../Util/Util.h"
#include "../Util/ResourceList.h"
#include "../Rule.h"

#define IMAGE_NOTIFICATION_TAG 'NIbO'
#define IMAGE_NOTIFICATION_ALLOCATE(size, pool) ExAllocatePoolWithTag(pool, size, IMAGE_NOTIFICATION_TAG)
#define IMAGE_NOTIFICATION_FREE(ptr) ExFreePoolWithTag(ptr, IMAGE_NOTIFICATION_TAG)

typedef struct _OBSERVER_DRIVER_LOAD_RULE_ENTRY
{
	LIST_ENTRY ListEntry;
	OBSERVER_RULE_HANDLE RuleHandle;
	UNICODE_STRING DriverPath;
	OBSERVER_DRIVER_LOAD_RULE Rule;
} OBSERVER_DRIVER_LOAD_RULE_ENTRY, *POBSERVER_DRIVER_LOAD_RULE_ENTRY;


extern OBSERVER_RESOURCE_LIST DriverLoadRuleList;
extern OBSERVER_RESOURCE_LIST ModuleLoadRuleList;

NTSTATUS InitImageLoadNotifications();
VOID UnloadImageNotifications();

VOID HandleDriverLoad(
	_In_opt_ PUNICODE_STRING FullImageName,
	_In_ PIMAGE_INFO ImageInfo
);

NTSTATUS DriverLoadAddRule(
	_In_ POBSERVER_DRIVER_LOAD_RULE Rule,
	_In_ POBSERVER_RULE_HANDLE RuleHandle
);

NTSTATUS DriverLoadRemoveRule(
	_In_ POBSERVER_RULE_HANDLE RuleHandle
);

#endif //IMAGE_LOAD_NOTIFY_H
